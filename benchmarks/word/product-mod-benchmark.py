#!/usr/bin/env -S uv run --script
"""Run the alternating fixed width-nine Word product-modulo comparison."""
from __future__ import annotations
import argparse, json, statistics, subprocess, time
from pathlib import Path

EXPECTED=(121,476747865)

def run(binary:Path,variant:str)->dict:
    start=time.perf_counter()
    process=subprocess.run([str(binary),"--variant",variant],
                           text=True,capture_output=True,check=True)
    value=json.loads(process.stdout)
    value["seconds"]=time.perf_counter()-start
    if (value["solutions"],value["checksum"]) != EXPECTED:
        raise RuntimeError(f"incorrect result: {value}")
    return value

def main()->int:
    parser=argparse.ArgumentParser()
    parser.add_argument("--binary",type=Path,required=True)
    parser.add_argument("--repetitions",type=int,default=20)
    parser.add_argument("--output",type=Path)
    args=parser.parse_args(); rows=[]
    for repetition in range(args.repetitions):
        order=("bounded","compact") if repetition%2==0 else ("compact","bounded")
        for variant in order:
            row=run(args.binary,variant); row["repetition"]=repetition
            rows.append(row)
    summary={}
    for variant in ("bounded","compact"):
        selected=[row for row in rows if row["variant"]==variant]
        summary[variant]={
          "median_seconds":statistics.median(row["seconds"] for row in selected),
          "nodes":selected[0]["nodes"],"failures":selected[0]["failures"],
          "propagations":selected[0]["propagations"]}
    report={"repetitions":args.repetitions,"summary":summary,"runs":rows}
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2,sort_keys=True)+"\n")
    print(json.dumps({"repetitions":args.repetitions,"summary":summary},
                     indent=2,sort_keys=True))
    return 0

if __name__=="__main__": raise SystemExit(main())
