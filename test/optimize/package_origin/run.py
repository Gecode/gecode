#!/usr/bin/env python3
"""Configure-only package ownership regressions; no solver builds or benchmarks.

Creates and installs interface-only export fixtures using the actual standalone
package config template. Repeated discovery must succeed; foreign combined,
standalone-prefix, or alias targets must produce the explicit ownership error.
--check-native-boundary also checks the real standalone Native=ON rejection.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]


def run(arguments, expected_error=None):
    result = subprocess.run(arguments, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=120)
    if expected_error is None:
        if result.returncode:
            raise RuntimeError("Command failed: " + repr(arguments) + "\n" + result.stdout)
    elif result.returncode == 0 or expected_error not in " ".join(result.stdout.split()):
        raise RuntimeError("Expected explicit rejection: " + expected_error + "\n" + result.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--generator")
    parser.add_argument("--work-root", type=Path)
    parser.add_argument("--source-root", type=Path, default=ROOT)
    parser.add_argument("--check-native-boundary", action="store_true")
    args = parser.parse_args()
    args.source_root = args.source_root.resolve(strict=True)
    if args.work_root:
        args.work_root.mkdir(parents=True, exist_ok=True)
    generator = ["-G", args.generator] if args.generator else []
    with tempfile.TemporaryDirectory(prefix="gecode-optimize-origin-", dir=args.work_root) as temporary:
        work = Path(temporary)
        prefixes = {}
        for name, kind in (("standalone", "standalone"), ("second", "standalone"), ("combined", "combined")):
            prefixes[name] = work / (name + "-install")
            build = work / (name + "-build")
            run([args.cmake, "-S", str(HERE / "producer"), "-B", str(build), *generator,
                 "-DPACKAGE_KIND=" + kind,
                 "-DOPTIMIZE_CONFIG_TEMPLATE=" + str(args.source_root / "gecode/optimize/GecodeOptimizeConfig.cmake.in"),
                 "-DCMAKE_INSTALL_PREFIX=" + str(prefixes[name]), "-DCMAKE_INSTALL_LIBDIR=lib"])
            run([args.cmake, "--install", str(build), "--config", "Release"])
        rejection = "already belongs to a different optimization package"
        for mode in ("repeat", "combined-first", "different-prefix", "alias-first", "c-alias-first", "c-target-first"):
            run([args.cmake, "-S", str(HERE / "consumer"), "-B", str(work / mode), *generator,
                 "-DIMPORT_MODE=" + mode, "-DSTANDALONE_PREFIX=" + str(prefixes["standalone"]),
                 "-DSECOND_STANDALONE_PREFIX=" + str(prefixes["second"]),
                 "-DCOMBINED_PREFIX=" + str(prefixes["combined"])],
                None if mode == "repeat" else rejection)
            print("PASS package origin: " + mode, flush=True)
        if args.check_native_boundary:
            run([args.cmake, "-S", str(args.source_root / "gecode/optimize"),
                 "-B", str(work / "unsupported-native"), *generator,
                 "-DGECODE_OPTIMIZE_WITH_NATIVE=ON", "-DGECODE_OPTIMIZE_WITH_HIGHS=OFF",
                 "-DGECODE_OPTIMIZE_BUILD_TESTS=OFF"],
                "Native optimization bridge requires a top-level Gecode build")
            print("PASS standalone native support boundary", flush=True)


if __name__ == "__main__":
    main()
