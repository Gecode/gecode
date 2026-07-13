#!/usr/bin/env bash
set -euxo pipefail

source_dir="${1:?usage: autoconf-install-layout-smoke.sh <source-dir> <build-dir> <destdir> <examples-enabled> <flatzinc-enabled>}"
build_dir="${2:?}"
destdir="${3:?}"
examples_enabled="${4:?}"
flatzinc_enabled="${5:?}"

rm -rf "$build_dir" "$destdir"
mkdir -p "$build_dir"

case "$examples_enabled:$flatzinc_enabled" in
  yes:yes|yes:no|no:yes|no:no) ;;
  *)
    echo "feature values must be yes or no" >&2
    exit 2
    ;;
esac

(
  cd "$build_dir"
  "$source_dir/configure" \
    --disable-float-vars \
    --disable-set-vars \
    --disable-gist \
    "--enable-examples=$examples_enabled" \
    "--enable-flatzinc=$flatzinc_enabled"

  # Example data remains part of documentation/statistics inputs even when it
  # is deliberately omitted from the installed header set.
  make --no-print-directory -f Makefile -f - check-documentation-inputs <<'MAKE'
.PHONY: check-documentation-inputs
check-documentation-inputs:
	@case " $(ALLGECODEHDR) " in *" examples/scowl.hpp "*) ;; *) exit 1;; esac
	@case " $(ALLGECODEHDR) " in *" examples/job-shop-instances.hpp "*) ;; *) exit 1;; esac
MAKE

  make -j4
  "$source_dir/.github/ci/autoconf-install-smoke.sh" \
    "$destdir" /usr/local "$examples_enabled" "$flatzinc_enabled"
)
