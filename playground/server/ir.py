"""Lossless views of AsterCC's textual SSA, never compiler transformations."""

import re
from pathlib import Path

VALUE = re.compile(r"^\s*%(\d+):(.+?) = (\w+)(?:\s+(.*))?$")
REFERENCE = re.compile(r"%(\d+)(?:@b(\d+))?")


def parse_ssa(text: str) -> dict:
    blocks = []
    name = text.splitlines()[0].removeprefix("function ") if text else ""
    for line_number, line in enumerate(text.splitlines(), 1):
        match = re.fullmatch(r"b(\d+):", line)
        if match:
            blocks.append(
                {
                    "id": "b" + match[1],
                    "line": line_number,
                    "instructions": [],
                    "terminator": "",
                    "edges": [],
                }
            )
        elif blocks and line.strip():
            block = blocks[-1]
            value = VALUE.match(line)
            if value:
                refs = [
                    {"value": "%" + m[1], "predecessor": "b" + m[2] if m[2] else None}
                    for m in REFERENCE.finditer(value[4] or "")
                ]
                # String payloads can contain percent characters. They have no SSA operands.
                if value[3] in ("string", "const", "arg"):
                    refs = []
                block["instructions"].append(
                    {
                        "id": "%" + value[1],
                        "type": value[2],
                        "op": value[3],
                        "text": line.strip(),
                        "line": line_number,
                        "uses": refs,
                    }
                )
            else:
                block["terminator"] = line.strip()
                block["terminator_line"] = line_number
                block["terminator_uses"] = [
                    "%" + m[1] for m in REFERENCE.finditer(line)
                ]
                targets = re.findall(r"\bb\d+\b", line)
                block["edges"] = [
                    {
                        "target": target,
                        "label": ("true" if i == 0 else "false")
                        if line.strip().startswith("branch ")
                        else "",
                    }
                    for i, target in enumerate(targets)
                ]
    count = sum(
        i["op"] not in ("arg", "const", "string")
        for b in blocks
        for i in b["instructions"]
    )
    return {"name": name, "blocks": blocks, "instruction_count": count}


def index_traces(directory: Path, stats: list) -> list:
    groups = {}
    for path in directory.glob("*.ssa"):
        match = re.fullmatch(r"(\d+)-(\d+)-(.+)\.ssa", path.name)
        if match:
            fid, order = int(match[1]), int(match[2])
            groups.setdefault(fid, []).append(
                {"key": path.name, "order": order, "pass": match[3]}
            )
    functions = []
    offset = 0
    for fid, snapshots in sorted(groups.items()):
        snapshots.sort(key=lambda x: x["order"])
        text = (directory / snapshots[0]["key"]).read_text()
        name = text.splitlines()[0].removeprefix("function ")
        rows = stats[offset : offset + len(snapshots) - 1]
        offset += len(snapshots) - 1
        for snapshot in snapshots:
            if snapshot["order"]:
                row = next(
                    (r for i, r in enumerate(rows, 1) if i == snapshot["order"]), None
                )
                snapshot["stats"] = row
        functions.append({"id": fid, "name": name, "snapshots": snapshots})
    return functions
