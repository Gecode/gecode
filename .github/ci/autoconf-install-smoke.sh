#!/usr/bin/env bash
set -euxo pipefail

destdir="${1:?usage: autoconf-install-smoke.sh <destdir> [logical-prefix]}"
logical_prefix="${2:-/usr/local}"

case "$logical_prefix" in
  /*) ;;
  *)
    echo "logical prefix must be absolute: $logical_prefix" >&2
    exit 2
    ;;
esac

rm -rf "$destdir"
mkdir -p "$destdir"

make install DESTDIR="$destdir"

install_prefix="${destdir%/}$logical_prefix"
solver_dir="$install_prefix/share/minizinc/solvers"
solver_config="$solver_dir/gecode.msc"
mznlib_dir="$install_prefix/share/minizinc/gecode"

test -f "$install_prefix/include/gecode/support/config.hpp"
test -x "$install_prefix/bin/fzn-gecode"
test -x "$install_prefix/bin/mzn-gecode"
! grep -q "/usr/local" "$install_prefix/bin/mzn-gecode"

test -f "$solver_config"
test -f "$mznlib_dir/experimental/on_restart/fzn_on_restart_complete.mzn"

python3 - "$solver_config" "$solver_dir" <<'PY'
import json
import pathlib
import sys

config_path = pathlib.Path(sys.argv[1])
solver_dir = pathlib.Path(sys.argv[2])
config = json.loads(config_path.read_text(encoding="utf-8"))

if config.get("id") != "org.gecode.gecode":
    raise SystemExit(f"unexpected solver id: {config.get('id')!r}")
if config.get("supportsFzn") is not True:
    raise SystemExit("solver config should support FlatZinc")

for key, expected_kind in (("mznlib", "dir"), ("executable", "file")):
    value = config.get(key)
    if not isinstance(value, str) or not value:
        raise SystemExit(f"{key} must be a non-empty string")
    if value.startswith("/") or ":\\" in value:
        raise SystemExit(f"{key} should be relative, got {value!r}")
    resolved = (solver_dir / value).resolve()
    if expected_kind == "dir" and not resolved.is_dir():
        raise SystemExit(f"{key} does not resolve to a directory: {resolved}")
    if expected_kind == "file" and not resolved.is_file():
        raise SystemExit(f"{key} does not resolve to a file: {resolved}")
PY
