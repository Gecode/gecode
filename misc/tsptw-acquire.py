#!/usr/bin/env python3
"""Acquire the frozen TSPTW selection explicitly, checking manifest hashes."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import runpy
import sys
import tarfile
from urllib.request import urlopen

CHECKER = runpy.run_path(str(Path(__file__).with_name("tsptw-check.py")))
ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "examples/data/tsptw/manifest.json"


def window_variant(content, variant):
    tokens = " ".join(line.split("#", 1)[0] for line in
                      content.decode().splitlines()).split()
    n = int(tokens[0])
    from fractions import Fraction
    for i in range(1, n):
        ready = Fraction(tokens[1+n*n+2*i])
        due_index = 1+n*n+2*i+1
        due = Fraction(tokens[due_index])
        due = due+100 if variant == "customer-due-plus-100" else ready+(due-ready)/2
        tokens[due_index] = CHECKER["exact_text"](due)
    rows = [str(n)]
    rows += [" ".join(tokens[1+i*n:1+(i+1)*n]) for i in range(n)]
    rows += [" ".join(tokens[1+n*n+2*i:1+n*n+2*i+2]) for i in range(n)]
    return ("\n".join(rows)+"\n").encode()


def verify_hash(content, expected, name):
    if hashlib.sha256(content).hexdigest() != expected:
        raise ValueError(f"content hash mismatch: {name}")


def acquire(destination, archive_directory=None):
    manifest = json.loads(MANIFEST.read_text())
    archives = {}
    for family, source in manifest["sources"].items():
        archive_name = source["archive_url"].rsplit("/", 1)[1]
        content = ((archive_directory/archive_name).read_bytes() if archive_directory
                   else urlopen(source["archive_url"], timeout=60).read())
        verify_hash(content, source["archive_sha256"], archive_name)
        archives[family] = tarfile.open(fileobj=io.BytesIO(content), mode="r:gz")
    try:
        for case in manifest["cases"]:
            content = archives[case["family"]].extractfile(case["member"]).read()
            verify_hash(content, case["source_sha256"], case["member"])
            if "variant" in case:
                content = window_variant(content, case["variant"])
            verify_hash(content, case["sha256"], case["id"])
            path = destination/case["path"]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(content)
            CHECKER["read_instance"](path)
    finally:
        for archive in archives.values():
            archive.close()
    print(f"acquired and validated {len(manifest['cases'])} selected cases in {destination}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--archives", type=Path, help="use already downloaded archives offline")
    args = parser.parse_args()
    try:
        acquire(args.destination, args.archives)
    except (OSError, ValueError, tarfile.TarError) as error:
        print(str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
