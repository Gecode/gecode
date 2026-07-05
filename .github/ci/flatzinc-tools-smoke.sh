#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: flatzinc-tools-smoke.sh <install-prefix>}"
"$prefix/bin/fzn-gecode" --help
test -x "$prefix/bin/mzn-gecode"
! grep -q "/usr/local" "$prefix/bin/mzn-gecode"
