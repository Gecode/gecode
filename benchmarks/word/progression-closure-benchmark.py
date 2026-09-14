#!/usr/bin/env -S uv run --script
"""Check bounded arithmetic-progression closure at representative widths."""
from __future__ import annotations
import argparse, json, subprocess
from pathlib import Path

WIDTHS=(16,24,32,40,64)
ACTORS=("divides","gcd","product_mod")

def main()->int:
    parser=argparse.ArgumentParser()
    parser.add_argument("--binary",type=Path,required=True)
    parser.add_argument("--timeout",type=float,default=5.0)
    args=parser.parse_args(); rows=[]
    for width in WIDTHS:
        for actor in ACTORS:
            process=subprocess.run([str(args.binary),str(width),actor],
              text=True,capture_output=True,check=True,timeout=args.timeout)
            row=json.loads(process.stdout)
            fixed=min(width//2,30)
            expected=((1<<fixed)-1)*(1<<fixed)
            if (row["minimum"],row["maximum"],row["assigned"]) != (expected,expected,True):
                raise RuntimeError(f"incorrect closure: {row}")
            if row["propagators"] != 0:
                raise RuntimeError(f"unfinished actor: {row}")
            rows.append(row)
    print(json.dumps({"widths":list(WIDTHS),"rows":rows},indent=2))
    return 0

if __name__=="__main__": raise SystemExit(main())
