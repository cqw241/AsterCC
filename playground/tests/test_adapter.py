"""Behavioral checks against the real Stage 3, not mocked compiler responses."""

import json
import time

import pytest
from fastapi.testclient import TestClient

from playground.server.app import ART, ROOT, app


@pytest.fixture(scope="module")
def client():
    with TestClient(app) as c:
        yield c


def wait(client, job):
    until = time.monotonic() + 35
    while job["status"] in ("queued", "compiling", "collecting", "running"):
        assert time.monotonic() < until, job
        time.sleep(0.03)
        job = client.get("/api/jobs/" + job["id"]).json()
    return job


def compile(client, source, level=2):
    response = client.post(
        "/api/jobs",
        json={"source": source, "optimization": level, "llvm_optimization": 0},
    )
    assert response.status_code == 202, response.text
    return wait(client, response.json())


@pytest.mark.parametrize("level", [0, 1, 2])
def test_real_native_pipeline(client, level):
    source = (ROOT / "examples/hello.aster").read_text()
    job = compile(client, source, level)
    assert job["status"] == "ready", job
    for key in ("ast", "ssa", "llvm"):
        response = client.get(f"/api/jobs/{job['id']}/artifacts/{key}")
        assert response.status_code == 200
        assert len(response.content) > 10
    ast = client.get(f"/api/jobs/{job['id']}/artifacts/ast").json()
    assert ast["functions"][0]["name"] == "main::main"
    job = wait(
        client, client.post(f"/api/jobs/{job['id']}/run", json={"arguments": []}).json()
    )
    assert job["execution"]["stdout"] == "Hello from AsterCC\n45\n"
    assert job["execution"]["returncode"] == 0
    assert len(job["stats"]) == (0 if level == 0 else 4 if level == 1 else 10)


def test_stdlib_and_unicode_diagnostic(client):
    source = (ROOT / "tests/cases/data.aster").read_text()
    job = compile(client, source)
    assert job["status"] == "ready", job
    job = wait(
        client, client.post(f"/api/jobs/{job['id']}/run", json={"arguments": []}).json()
    )
    assert "你好🌟\n10\n" in job["execution"]["stdout"]
    bad = 'fn main()->i64 { println("你好🌟");return missing; }'
    job = compile(client, bad)
    assert job["status"] == "failed"
    diagnostic = job["diagnostics"][0]
    assert diagnostic["editable"]
    assert (
        diagnostic["column"]
        == len(bad[: bad.index("missing")].encode("utf-16-le")) // 2 + 1
    )


def test_real_passes_and_cfg(client):
    job = compile(client, (ROOT / "tests/cases/optimization.aster").read_text())
    assert job["status"] == "ready", job
    fs = client.get(f"/api/traces/job-{job['id']}/functions").json()
    kernel = fs[0]
    assert [s["order"] for s in kernel["snapshots"]] == list(range(11))
    assert kernel["snapshots"][10]["pass"] == "dce"
    prefix = f"/api/traces/job-{job['id']}/snapshots/"
    snapshots = [client.get(prefix + s["key"]).json() for s in kernel["snapshots"]]
    assert any(
        i["op"] == "phi" for b in snapshots[0]["blocks"] for i in b["instructions"]
    )
    assert any(e["label"] == "true" for b in snapshots[0]["blocks"] for e in b["edges"])
    for name in ("sccp", "gvn"):
        assert any(
            s.get("stats", {}).get("after", 0) < s.get("stats", {}).get("before", 0)
            for s in kernel["snapshots"]
            if s["pass"] == name
        )
    before = {
        i["id"]: b["id"] for b in snapshots[5]["blocks"] for i in b["instructions"]
    }
    after = {
        i["id"]: b["id"] for b in snapshots[6]["blocks"] for i in b["instructions"]
    }
    assert before["%14"] == "b5" and after["%14"] == "b0"
    assert snapshots[6]["instruction_count"] == kernel["snapshots"][6]["stats"]["after"]


def test_exit_cancel_and_timeout(client):
    job = compile(client, "fn main()->i64 {return 7;}")
    job = wait(client, client.post(f"/api/jobs/{job['id']}/run", json={}).json())
    assert job["execution"]["returncode"] == 7
    job = compile(client, "fn main()->i64 {while true {} return 0;}")
    assert job["status"] == "ready", job
    response = client.post(f"/api/jobs/{job['id']}/run", json={})
    assert response.status_code == 202
    time.sleep(0.08)
    stopped = client.post(f"/api/jobs/{job['id']}/cancel").json()
    assert stopped["status"] == "cancelled"
    job = wait(client, client.post(f"/api/jobs/{job['id']}/run", json={}).json())
    assert job["execution"]["timed_out"] is True
    assert job["execution"]["returncode"] != 0


def test_dashboard_evidence(client):
    data = client.get("/api/dashboard").json()
    record = json.loads((ART / "artifacts/bootstrap.json").read_text())
    assert len(data["stages"]) == len(record["stages"]) == 6
    assert all(s["current_matches"] for s in data["stages"])
    assert all(p["matches"] for p in data["provenance"])
    assert data["records"]["tests-stage3"]["passed"] == 32
    assert len(data["records"]["benchmarks"]["rows"]) == 12
    assert client.get("/api/records/bootstrap").json() == record
    assert (
        client.get("/api/traces/recorded-optimization/functions").json()[0]["name"]
        == "main::kernel"
    )
