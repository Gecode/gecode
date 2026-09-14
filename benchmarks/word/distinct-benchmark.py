#!/usr/bin/env -S uv run --script
"""Measure the shipped Word bounds-distinct actor and its controls."""
from __future__ import annotations
import argparse, json, statistics, subprocess
from pathlib import Path

FORMULATIONS=("value","bounds","int-channel")

def run(binary:Path,kind:str,formulation:str,size:int,iterations:int,
        timeout:float)->dict:
    command=[str(binary),kind,formulation,str(size),str(iterations)]
    process=subprocess.run(command,text=True,capture_output=True,
                           timeout=timeout,check=False)
    if process.returncode:
        raise RuntimeError(f"failed ({process.returncode}): {command}\n{process.stderr}")
    row=json.loads(process.stdout)
    if row.get("status") != "ok": raise RuntimeError(f"bad result: {row}")
    return row

def main()->int:
    parser=argparse.ArgumentParser()
    parser.add_argument("--binary",type=Path,required=True)
    parser.add_argument("--repetitions",type=int,default=20)
    parser.add_argument("--batch",type=int,default=20)
    parser.add_argument("--large-size",type=int,default=4096)
    parser.add_argument("--timeout",type=float,default=120.0)
    parser.add_argument("--revision",required=True,
      help="exact source revision represented by the binary")
    parser.add_argument("--output",type=Path)
    args=parser.parse_args()
    if args.repetitions < 1 or args.batch < 1: parser.error("counts must be positive")
    cases=[("register",f,n,args.batch) for n in (2,3,4)
           for f in FORMULATIONS]
    cases += [("wide",f,n,args.batch) for n in (8,16,32,64) for f in FORMULATIONS]
    cases += [("wide","bounds",args.large_size,max(1,args.batch//4))]
    rows=[]
    for repetition in range(args.repetitions):
        order=cases[repetition%len(cases):]+cases[:repetition%len(cases)]
        if repetition%2: order=list(reversed(order))
        for kind,formulation,size,iterations in order:
            row=run(args.binary,kind,formulation,size,iterations,args.timeout)
            row.update(kind=kind,formulation=formulation,size=size,
                       repetition=repetition+1)
            rows.append(row)

    summary=[]
    for case in cases:
        kind,formulation,size,iterations=case
        selected=[r for r in rows if (r["kind"],r["formulation"],r["size"]) == case[:3]]
        counters=("solutions","checksum","nodes","failures","propagations","iterations")
        baseline={k:selected[0][k] for k in counters}
        if any({k:r[k] for k in counters} != baseline for r in selected[1:]):
            raise RuntimeError(f"unstable counters for {case}")
        samples=[r["seconds"] for r in selected]
        summary.append({"kind":kind,"formulation":formulation,"size":size,
          "median_seconds":statistics.median(samples),"min_seconds":min(samples),
          "max_seconds":max(samples),"counters":baseline})
    for size in (2,3,4):
        register=[r for r in summary
                  if r["kind"] == "register" and r["size"] == size]
        projections={(r["counters"]["solutions"],r["counters"]["checksum"])
                     for r in register}
        if len(projections) != 1:
            raise RuntimeError(f"register projections differ at size {size}")
    artifact={"schema_version":1,"revision":args.revision,
      "subject":"shipped Word::Distinct::Bnd actor (not the word-016 prototype)",
      "order":"rotated and reversed interleaving","repetitions":args.repetitions,
      "memory":"not measured; no memory claim is made","summary":summary,"runs":rows}
    text=json.dumps(artifact,indent=2,sort_keys=True)+"\n"
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(text)
    print(json.dumps({k:artifact[k] for k in ("revision","subject","summary")},indent=2))
    return 0
if __name__ == "__main__": raise SystemExit(main())
