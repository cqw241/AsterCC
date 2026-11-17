"""Local CLI adapter. All compilation and optimization stay in AsterCC."""

import asyncio
import hashlib
import json
import os
import re
import signal
import subprocess
import time
import uuid
from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel, Field

from .ir import index_traces, parse_ssa

ROOT = Path(__file__).resolve().parents[2]
# Resolve shared build artifacts explicitly, or through the linked worktree's Git directory.
if os.environ.get("ASTERCC_ARTIFACT_ROOT"):
    ART = Path(os.environ["ASTERCC_ARTIFACT_ROOT"]).resolve()
elif (ROOT / "build/bootstrap/O2/stage3").is_file():
    ART = ROOT
else:
    ART = Path(
        subprocess.check_output(
            ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"],
            cwd=ROOT,
            text=True,
        ).strip()
    ).parent

COMPILER = Path(
    os.environ.get("ASTERCC_COMPILER", str(ART / "build/bootstrap/O2/stage3"))
).resolve()
CLANG = ART / ".toolchain/usr/lib/llvm-18/bin/clang"
JOBS_ROOT = ROOT / "build/playground/jobs"
LIMIT = 1024 * 1024
JOBS: dict[str, dict] = {}
TASKS: dict[str, asyncio.Task] = {}
PROCESSES: dict[str, asyncio.subprocess.Process] = {}


def sha(path: Path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


def read_json(path: Path, default=None):
    return json.loads(path.read_text()) if path.is_file() else default


@asynccontextmanager
async def lifespan(_app):
    yield
    for identifier in list(PROCESSES):
        terminate(identifier)
    for task in TASKS.values():
        if not task.done():
            task.cancel()
    await asyncio.gather(*TASKS.values(), return_exceptions=True)


app = FastAPI(title="AsterCC Playground API", version="0.1.0", lifespan=lifespan)


class CompileRequest(BaseModel):
    source: str = Field(min_length=1, max_length=262144)
    optimization: int = Field(default=2, ge=0, le=2)
    llvm_optimization: int = Field(default=2, ge=0, le=2)


class RunRequest(BaseModel):
    arguments: list[str] = Field(default_factory=list, max_length=64)


def require_job(identifier: str):
    if identifier not in JOBS:
        raise HTTPException(404, "任务不存在或服务已重启，请重新编译。")
    return JOBS[identifier]


def public(job):
    return {k: v for k, v in job.items() if not k.startswith("_")}


def terminate(identifier):
    process = PROCESSES.get(identifier)
    if process and process.returncode is None:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass


async def process(identifier: str, command: list[str], timeout: int) -> dict:
    job = require_job(identifier)
    env = os.environ | {
        "ASTERCC_ROOT": str(job["_directory"]),
        "ASTERCC_CLANG": str(CLANG),
        "LD_LIBRARY_PATH": ":".join(
            [
                str(ART / ".toolchain/usr/lib/x86_64-linux-gnu"),
                str(ART / ".toolchain/usr/lib/llvm-18/lib"),
                os.environ.get("LD_LIBRARY_PATH", ""),
            ]
        ),
    }
    start = time.monotonic()
    proc = await asyncio.create_subprocess_exec(
        *command,
        cwd=job["_directory"],
        env=env,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
        start_new_session=True,
        stdin=asyncio.subprocess.DEVNULL,
    )
    PROCESSES[identifier] = proc
    streams = {"stdout": bytearray(), "stderr": bytearray()}
    truncated = False

    async def consume(name, stream):
        nonlocal truncated
        while chunk := await stream.read(16384):
            remaining = LIMIT - len(streams["stdout"]) - len(streams["stderr"])
            streams[name].extend(chunk[: max(remaining, 0)])
            if len(chunk) > remaining:
                truncated = True
                terminate(identifier)

    readers = asyncio.gather(
        consume("stdout", proc.stdout), consume("stderr", proc.stderr)
    )
    timed_out = False
    try:
        await asyncio.wait_for(asyncio.shield(readers), timeout)
        await asyncio.wait_for(
            proc.wait(), max(0.01, timeout - (time.monotonic() - start))
        )
    except asyncio.TimeoutError:
        timed_out = True
        terminate(identifier)
        await readers
        await proc.wait()
    except asyncio.CancelledError:
        terminate(identifier)
        await asyncio.shield(proc.wait())
        await readers
        raise
    finally:
        PROCESSES.pop(identifier, None)
    result = {
        "command": command,
        "returncode": proc.returncode,
        "seconds": round(time.monotonic() - start, 4),
        "timed_out": timed_out,
        "truncated": truncated,
        **{k: bytes(v).decode("utf-8", errors="replace") for k, v in streams.items()},
    }
    job["commands"].append(result)
    return result


def diagnostics(stderr: str, source: str):
    results = []
    for match in re.finditer(r"^(.+?):(\d+):(\d+): error: (.+)$", stderr, re.M):
        path, line, byte_col, message = match.groups()
        line = int(line)
        col = int(byte_col)
        is_main = Path(path).name == "main.aster"
        if is_main:
            lines = source.splitlines()
            if line <= len(lines):
                prefix = (
                    lines[line - 1]
                    .encode("utf-8")[: col - 1]
                    .decode("utf-8", errors="ignore")
                )
                col = len(prefix.encode("utf-16-le")) // 2 + 1
        results.append(
            {
                "file": "main.aster" if is_main else path,
                "line": line,
                "column": col,
                "message": message,
                "editable": is_main,
            }
        )
    return results


async def compile_job(identifier: str):
    job = require_job(identifier)
    folder = job["_directory"]
    try:
        job["status"] = "compiling"
        options = [
            f"-O{job['optimization']}",
            f"--llvm-opt={job['llvm_optimization']}",
            "--verify-ir",
        ]
        command = [
            str(COMPILER),
            str(folder / "main.aster"),
            *options,
            "--trace-dir",
            str(folder / "trace"),
            "--stats-json",
            str(folder / "stats.json"),
            "-o",
            str(folder / "program"),
        ]
        result = await process(identifier, command, 30)
        job["diagnostics"] = diagnostics(result["stderr"], job["source"])
        if result["returncode"] != 0:
            job.update(
                status="failed",
                error="编译超时。"
                if result["timed_out"]
                else "编译输出超过 1 MiB。"
                if result["truncated"]
                else "编译失败，请查看诊断或终端。",
            )
            return
        job["status"] = "collecting"
        result = await process(
            identifier,
            [
                str(COMPILER),
                str(folder / "main.aster"),
                *options,
                "--emit=ast",
                "-o",
                str(folder / "ast.json"),
            ],
            30,
        )
        if result["returncode"] != 0:
            job.update(status="failed", error="AST 生成失败，请查看终端。")
            return
        stats = read_json(folder / "stats.json", [])
        functions = index_traces(folder / "trace", stats)
        (folder / "final.ssa").write_text(
            "\n".join(
                (folder / "trace" / f["snapshots"][-1]["key"]).read_text()
                for f in functions
            )
        )
        job.update(
            status="ready",
            functions=functions,
            stats=stats,
            artifacts=["ast", "ssa", "llvm", "stats", "source", "binary"],
        )
    except asyncio.CancelledError:
        job["status"] = "cancelled"
        raise
    except Exception as error:
        job.update(status="failed", error=str(error))
    finally:
        (folder / "job.json").write_text(
            json.dumps(public(job), ensure_ascii=False, indent=2)
        )


@app.get("/api/health")
def health():
    files = {
        "compiler": COMPILER,
        "runtime": ART / "build/libasterrt.a",
        "clang": CLANG,
        "stdlib": ROOT / "std",
    }
    version = (
        subprocess.run(
            [str(COMPILER), "--version"],
            capture_output=True,
            text=True,
            check=True,
            timeout=3,
        ).stdout.strip()
        if COMPILER.is_file()
        else "编译器不可用"
    )
    return {
        "ready": all(p.exists() for p in files.values()),
        "compiler": str(COMPILER),
        "compiler_sha256": sha(COMPILER),
        "artifact_root": str(ART),
        "checks": {k: p.exists() for k, p in files.items()},
        "version": version,
        "repair_command": f'cd "{ART}" && bash scripts/reproduce.sh',
    }


EXAMPLES = [
    (
        "optimization",
        "优化综合示例",
        "常量分支、重复表达式与循环不变量。输出 1290。",
        "tests/cases/optimization.aster",
        [],
    ),
    (
        "hello",
        "Hello Aster",
        "打印问候并计算 0 到 9 的和。",
        "examples/hello.aster",
        [],
    ),
    (
        "control",
        "控制流与递归",
        "递归、嵌套循环与短路求值。",
        "tests/cases/control.aster",
        [],
    ),
    (
        "data",
        "结构体与标准库",
        "记录复制、数组共享引用、哈希表及字符串。",
        "tests/cases/data.aster",
        [],
    ),
    (
        "integers",
        "整数边界",
        "回绕算术、移位与有符号除法。",
        "tests/cases/integers.aster",
        [],
    ),
    (
        "effects",
        "副作用与求值顺序",
        "短路、数组索引和不能外提的操作。",
        "tests/cases/effects.aster",
        [],
    ),
    (
        "constants",
        "SCCP 常量传播",
        "常量条件与分支删除。使用命令行参数 n。",
        "examples/benchmarks/constants.aster",
        ["10000"],
    ),
    (
        "gvn",
        "GVN 公共表达式",
        "观察支配关系下的重复计算消除。",
        "examples/benchmarks/gvn.aster",
        ["10000"],
    ),
    (
        "licm",
        "LICM 循环外提",
        "观察指令数量不变时的块间移动。",
        "examples/benchmarks/licm.aster",
        ["10000"],
    ),
    (
        "arrays",
        "数组计算",
        "带边界检查的数组循环。",
        "examples/benchmarks/arrays.aster",
        ["10000"],
    ),
]


@app.get("/api/examples")
def examples():
    return [
        {
            "id": id,
            "name": name,
            "description": description,
            "source": (ROOT / path).read_text(),
            "path": path,
            "arguments": args,
        }
        for id, name, description, path, args in EXAMPLES
    ]


@app.post("/api/jobs", status_code=202)
async def create_job(request: CompileRequest):
    if not health()["ready"]:
        raise HTTPException(503, "编译器或工具链缺失，请查看服务状态。")
    identifier = uuid.uuid4().hex
    folder = JOBS_ROOT / identifier
    folder.mkdir(parents=True)
    (folder / "main.aster").write_text(request.source)
    (folder / "std").symlink_to(ROOT / "std", target_is_directory=True)
    (folder / "build").mkdir()
    (folder / "build/libasterrt.a").symlink_to(ART / "build/libasterrt.a")
    job = {
        "id": identifier,
        "status": "queued",
        **request.model_dump(),
        "source_sha256": hashlib.sha256(request.source.encode()).hexdigest(),
        "commands": [],
        "diagnostics": [],
        "functions": [],
        "stats": [],
        "artifacts": [],
        "_directory": folder,
    }
    JOBS[identifier] = job
    TASKS[identifier] = asyncio.create_task(compile_job(identifier))
    return public(job)


@app.get("/api/jobs/{identifier}")
def get_job(identifier: str):
    return public(require_job(identifier))


async def run_job(identifier, arguments):
    job = require_job(identifier)
    job["status"] = "running"
    try:
        result = await process(
            identifier, [str(job["_directory"] / "program"), *arguments], 5
        )
        job["execution"] = result
        job["status"] = "ready"
    except asyncio.CancelledError:
        job["status"] = "cancelled"
        raise
    except Exception as error:
        job.update(status="failed", error=str(error))
    finally:
        (job["_directory"] / "job.json").write_text(
            json.dumps(public(job), ensure_ascii=False, indent=2)
        )


@app.post("/api/jobs/{identifier}/run", status_code=202)
async def run(identifier: str, request: RunRequest):
    job = require_job(identifier)
    if (
        job["status"] not in ("ready", "cancelled")
        or not (job["_directory"] / "program").exists()
    ):
        raise HTTPException(409, "请先完成编译。")
    if TASKS[identifier].done() is False:
        raise HTTPException(409, "任务仍在运行。")
    job.pop("execution", None)
    job["status"] = "running"
    TASKS[identifier] = asyncio.create_task(run_job(identifier, request.arguments))
    return public(job)


@app.post("/api/jobs/{identifier}/cancel")
async def cancel(identifier: str):
    job = require_job(identifier)
    task = TASKS[identifier]
    if not task.done():
        terminate(identifier)
        task.cancel()
        await asyncio.gather(task, return_exceptions=True)
        job["status"] = "cancelled"
    return public(job)


ARTIFACT_NAMES = {
    "ast": "ast.json",
    "ssa": "final.ssa",
    "llvm": "program.ll",
    "stats": "stats.json",
    "source": "main.aster",
    "binary": "program",
}


@app.get("/api/jobs/{identifier}/artifacts/{key}")
def artifact(identifier: str, key: str, download: bool = False):
    job = require_job(identifier)
    if key not in ARTIFACT_NAMES:
        raise HTTPException(404, "未知制品。")
    path = job["_directory"] / ARTIFACT_NAMES[key]
    if not path.is_file():
        raise HTTPException(404, "制品尚未生成。")
    return FileResponse(
        path,
        filename=path.name if download else None,
        media_type="application/octet-stream"
        if key == "binary"
        else "application/json"
        if key in ("ast", "stats")
        else "text/plain; charset=utf-8",
    )


def datasets():
    result = {
        "recorded-optimization": {
            "name": "优化实验 · kernel",
            "directory": ART / "artifacts/optimization",
            "stats": ART / "artifacts/optimization/stats.json",
        }
    }
    for level in (0, 2):
        for stage in (1, 2, 3):
            result[f"O{level}-stage{stage}"] = {
                "name": f"自举 O{level} · Stage {stage}",
                "directory": ART / f"build/bootstrap/O{level}/trace{stage}",
                "stats": ART / f"artifacts/bootstrap/O{level}-stage{stage}-stats.json",
            }
    return result


def trace_dataset(identifier):
    if identifier.startswith("job-"):
        job = require_job(identifier[4:])
        return {
            "name": "当前编译",
            "directory": job["_directory"] / "trace",
            "stats": job["_directory"] / "stats.json",
        }
    if identifier not in datasets():
        raise HTTPException(404, "追踪数据集不存在。")
    return datasets()[identifier]


@app.get("/api/traces")
def traces():
    return [
        {
            "id": id,
            "name": d["name"],
            "available": d["directory"].is_dir(),
            "source": str(d["directory"]),
        }
        for id, d in datasets().items()
    ]


@app.get("/api/traces/{identifier}/functions")
def trace_functions(identifier: str):
    dataset = trace_dataset(identifier)
    return index_traces(dataset["directory"], read_json(dataset["stats"], []))


@app.get("/api/traces/{identifier}/snapshots/{key}")
def trace_snapshot(identifier: str, key: str):
    dataset = trace_dataset(identifier)
    if not re.fullmatch(r"\d+-\d+-[a-z]+\.ssa", key):
        raise HTTPException(404, "未知快照。")
    path = dataset["directory"] / key
    if not path.is_file():
        raise HTTPException(404, "快照文件缺失。")
    text = path.read_text()
    return {"text": text, "sha256": sha(path), **parse_ssa(text)}


RECORDS = {
    "bootstrap": "bootstrap.json",
    "tests": "tests.json",
    "tests-stage3": "tests-stage3.json",
    "benchmarks": "benchmarks.json",
    "sanitizers": "sanitizers.json",
    "toolchain": "toolchain.json",
    "optimization": "optimization/checks.json",
    "trace-hashes": "trace-hashes.json",
}


@app.get("/api/records/{key}")
def record(key: str):
    if key not in RECORDS or not (ART / "artifacts" / RECORDS[key]).is_file():
        raise HTTPException(404, "实验记录缺失。")
    return FileResponse(
        ART / "artifacts" / RECORDS[key],
        filename=Path(RECORDS[key]).name,
        media_type="application/json",
    )


@app.get("/api/dashboard")
def dashboard():
    records = {}
    sources = []
    for key, name in RECORDS.items():
        path = ART / "artifacts" / name
        records[key] = read_json(path)
        sources.append(
            {
                "key": key,
                "path": str(path),
                "available": path.is_file(),
                "modified": path.stat().st_mtime if path.is_file() else None,
            }
        )
    bootstrap = records.get("bootstrap") or {}
    stages = []
    for row in bootstrap.get("stages", []):
        binary = ART / row["binary"]
        current = sha(binary)
        llvm = sha(Path(str(binary) + ".ll"))
        stages.append(
            row
            | {
                "current_binary_sha256": current,
                "current_llvm_sha256": llvm,
                "current_matches": current == row["binary_sha256"]
                and llvm == row["llvm_sha256"],
            }
        )
    provenance = [
        {"path": name, "matches": sha(ROOT / name) == expected}
        for name, expected in bootstrap.get("source_sha256", {}).items()
    ]
    return {
        "records": records,
        "sources": sources,
        "stages": stages,
        "provenance": provenance,
        "seed": {
            "path": str(ART / "build/astercc0"),
            "sha256": sha(ART / "build/astercc0"),
            "bytes": (ART / "build/astercc0").stat().st_size
            if (ART / "build/astercc0").exists()
            else None,
        },
    }


DIST = ROOT / "playground/web/dist"
if (DIST / "assets").is_dir():
    app.mount("/assets", StaticFiles(directory=DIST / "assets"), name="assets")


@app.get("/{path:path}")
def spa(path: str):
    if path.startswith("api/"):
        raise HTTPException(404, "接口不存在。")
    if path == "favicon.svg" and (DIST / path).is_file():
        return FileResponse(DIST / path)
    if (DIST / "index.html").is_file():
        return FileResponse(DIST / "index.html")
    raise HTTPException(404, "前端尚未构建，请运行展示脚本或启动 Vite。")
