#!/usr/bin/env python3
"""Enforce Comet's compile-time module dependency boundaries."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
MODULES = ROOT / "src" / "comet"
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]comet/([^/"<>]+)(?:/[^"<>]*)?[">]', re.MULTILINE)
ALLOWED = {
    "core": {"core"},
    "data": {"core", "data"},
    "runtime": {"core", "data", "runtime"},
    "platform": {"core", "platform"},
    "render": {"core", "data", "runtime", "platform", "render"},
    "engine": {"core", "data", "runtime", "platform", "render", "engine"},
}
EXTENSIONS = {".h", ".hpp", ".cc", ".cpp", ".cxx"}
errors = []
for module, allowed in ALLOWED.items():
    folder = MODULES / module
    for path in folder.rglob("*"):
        if not path.is_file() or path.suffix not in EXTENSIONS:
            continue
        content = path.read_text(encoding="utf-8")
        for match in INCLUDE.finditer(content):
            dependency = match.group(1)
            if dependency in ALLOWED and dependency not in allowed:
                line = content.count("\n", 0, match.start()) + 1
                errors.append(f"{path.relative_to(ROOT)}:{line}: {module} must not include {dependency}")
for path in (ROOT / "src" / "tool" / "assetc").rglob("*"):
    if not path.is_file() or path.suffix not in EXTENSIONS:
        continue
    content = path.read_text(encoding="utf-8")
    for match in INCLUDE.finditer(content):
        dependency = match.group(1)
        if dependency in ALLOWED and dependency not in {"core", "data"}:
            line = content.count("\n", 0, match.start()) + 1
            errors.append(f"{path.relative_to(ROOT)}:{line}: assetc must not include {dependency}")
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("Comet dependency boundaries: OK")
