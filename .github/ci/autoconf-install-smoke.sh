#!/usr/bin/env bash
set -euxo pipefail

destdir="${1:?usage: autoconf-install-smoke.sh <destdir> [logical-prefix] [examples-enabled] [flatzinc-enabled]}"
logical_prefix="${2:-/usr/local}"
examples_enabled="${3:-yes}"
flatzinc_enabled="${4:-yes}"

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

if grep -q '^#define GECODE_HAS_WORD_VARS' \
    "$install_prefix/include/gecode/support/config.hpp"; then
  test -f "$install_prefix/include/gecode/word.hh"
  test -f "$install_prefix/include/gecode/word/arithmetic/bounded-product-mod.hpp"
  find "$install_prefix/lib" -maxdepth 1 -name '*gecodeword*' -print -quit | \
    grep -q .
  consumer_dir="$destdir/word-consumer"
  mkdir -p "$consumer_dir"
  cat > "$consumer_dir/main.cpp" <<'EOF'
#include <gecode/word/arithmetic.hh>
class WordInstallModel : public Gecode::Space {
public:
  Gecode::WordVar x;
  WordInstallModel(void) : x(*this,4) {
    Gecode::dom(*this,x,6U);
  }
  WordInstallModel(WordInstallModel& s) : Gecode::Space(s) {
    x.update(*this,s.x);
  }
  virtual Gecode::Space* copy(void) {
    return new WordInstallModel(*this);
  }
};
int main(void) {
  WordInstallModel model;
  return model.status() == Gecode::SS_SOLVED ? 0 : 1;
}
EOF
  "${CXX:-c++}" -std=c++17 \
    -I"$install_prefix/include" -L"$install_prefix/lib" \
    "$consumer_dir/main.cpp" -lgecodeword -lgecodeint -lgecodekernel \
    -lgecodesupport -lpthread -o "$consumer_dir/word-consumer"
  env DYLD_LIBRARY_PATH="$install_prefix/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}" \
    LD_LIBRARY_PATH="$install_prefix/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    "$consumer_dir/word-consumer"
else
  test -z "$(find "$install_prefix/lib" -maxdepth 1 -name '*gecodeword*' -print -quit)"
fi

case "$examples_enabled" in
  yes)
    test -f "$install_prefix/include/examples/scowl.hpp"
    test -f "$install_prefix/include/examples/job-shop-instances.hpp"
    ;;
  no)
    test ! -e "$install_prefix/include/examples"
    ;;
  *)
    echo "examples-enabled must be yes or no: $examples_enabled" >&2
    exit 2
    ;;
esac

case "$flatzinc_enabled" in
  yes)
    test -x "$install_prefix/bin/fzn-gecode"
    test -x "$install_prefix/bin/mzn-gecode"
    ! grep -q "/usr/local" "$install_prefix/bin/mzn-gecode"
    test -f "$solver_config"
    test -f "$mznlib_dir/experimental/on_restart/fzn_on_restart_complete.mzn"
    ;;
  no)
    test ! -e "$install_prefix/bin/fzn-gecode"
    test ! -e "$install_prefix/bin/mzn-gecode"
    test ! -e "$install_prefix/share/minizinc"
    exit 0
    ;;
  *)
    echo "flatzinc-enabled must be yes or no: $flatzinc_enabled" >&2
    exit 2
    ;;
esac

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
