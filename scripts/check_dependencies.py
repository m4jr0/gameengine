#!/usr/bin/env python3
"""Check direct Comet module includes and standalone tool boundaries.

This is a source-level check, not a substitute for CMake link validation.
"""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
MODULES = ROOT / "src" / "comet"
INCLUDE = re.compile(
    r'^\s*#\s*include\s*[<"]comet/([^/"<>]+)(?:/[^"<>]*)?[">]',
    re.MULTILINE,
)
ALLOWED = {
    "core": {"core"},
    "data": {"core", "data"},
    "runtime": {"core", "data", "runtime"},
    "platform": {"core", "data", "runtime", "platform"},
    "render": {"core", "data", "runtime", "platform", "render"},
    "engine": {"core", "data", "runtime", "platform", "render", "engine"},
}
EXTENSIONS = {".h", ".hpp", ".cc", ".cpp", ".cxx"}
errors = []


def check_includes(folder: Path, owner: str, allowed: set[str]) -> None:
    for path in sorted(folder.rglob("*")):
        if not path.is_file() or path.suffix not in EXTENSIONS:
            continue

        content = path.read_text(encoding="utf-8")
        for match in INCLUDE.finditer(content):
            dependency = match.group(1).removesuffix(".h")
            if dependency not in ALLOWED or dependency in allowed:
                continue

            line = content.count("\n", 0, match.start()) + 1
            errors.append(
                f"{path.relative_to(ROOT)}:{line}: "
                f"{owner} must not include {dependency}"
            )


for module, allowed in ALLOWED.items():
    check_includes(MODULES / module, module, allowed)

check_includes(ROOT / "src" / "tool" / "assetc", "assetc", {"core", "data"})
check_includes(ROOT / "src" / "tests" / "foundation", "foundation tests", {"core", "data"})

if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)

print("Comet source dependency boundaries: OK")
