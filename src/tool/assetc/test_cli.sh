#!/usr/bin/env bash
# Run after building assetc; these checks do not write assets.
set -euo pipefail
binary="${1:?Usage: test_cli.sh /path/to/assetc}"
"$binary" --help >/dev/null
for args in '--unknown' '--assets' '--resources' '--force=true' '--force --force' '--assets='; do
  read -r -a tokens <<< "$args"
  if "$binary" "${tokens[@]}" >/dev/null 2>&1; then
    printf 'Expected failure: %s\n' "$args" >&2
    exit 1
  fi
done
printf 'assetc CLI smoke checks passed\n'
