#!/usr/bin/env -S uv run --script
"""Run the interleaved Release comparison for Word n-ary Add."""
from __future__ import annotations
import argparse, json, statistics, subprocess, time
from pathlib import Path
EXPECTED={4:(86,156576),6:(676,2627648),8:(3788,25471040)}
def run(binary:Path,n:int,variant:str)->dict:
    start=time.perf_counter()
    p=subprocess.run([str(binary),"--segments",str(n),"--variant",variant],
                     text=True,capture_output=True,check=True)
    value=json.loads(p.stdout); value["seconds"]=time.perf_counter()-start
    if (value["solutions"],value["checksum"]) != EXPECTED[n]:
        raise RuntimeError(f"incorrect result: {value}")
    return value
def main()->int:
    ap=argparse.ArgumentParser(); ap.add_argument("--binary",type=Path,required=True)
    ap.add_argument("--repetitions",type=int,default=20)
    ap.add_argument("--output",type=Path); args=ap.parse_args(); rows=[]
    for rep in range(args.repetitions):
        order=("bounded","compact") if rep%2==0 else ("compact","bounded")
        for n in EXPECTED:
            for variant in order:
                row=run(args.binary,n,variant); row["repetition"]=rep; rows.append(row)
    summary={}
    for n in EXPECTED:
        summary[str(n)]={}
        for variant in ("bounded","compact"):
            selected=[r for r in rows if r["segments"]==n and r["variant"]==variant]
            summary[str(n)][variant]={
              "median_seconds":statistics.median(r["seconds"] for r in selected),
              "nodes":selected[0]["nodes"],"failures":selected[0]["failures"],
              "propagations":selected[0]["propagations"]}
    report={"repetitions":args.repetitions,"summary":summary,"runs":rows}
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2,sort_keys=True)+"\n")
    print(json.dumps({"repetitions":args.repetitions,"summary":summary},indent=2,sort_keys=True))
    return 0
if __name__=="__main__": raise SystemExit(main())
