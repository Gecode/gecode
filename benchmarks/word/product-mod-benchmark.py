#!/usr/bin/env -S uv run --script
"""Run the alternating fixed width-nine Word product-modulo comparison."""
from __future__ import annotations
import argparse, json, statistics, subprocess, time
from pathlib import Path

EXPECTED=(121,476747865)
COUNTERS=("status","solutions","checksum","nodes","failures","propagations")

def captured(value:bytes|str|None)->str:
    return value.decode(errors="replace") if isinstance(value,bytes) else value or ""

def run(binary:Path,variant:str,timeout:float)->dict:
    start=time.perf_counter()
    command=[str(binary),"--variant",variant]
    try:
        process=subprocess.run(command,text=True,capture_output=True,
                               check=False,timeout=timeout)
    except subprocess.TimeoutExpired as error:
        return {"variant":variant,"status":"timeout","seconds":timeout,
                "command":command,"returncode":None,"stdout":captured(error.stdout),
                "stderr":captured(error.stderr),"error":str(error)}
    except OSError as error:
        return {"variant":variant,"status":"error",
                "seconds":time.perf_counter()-start,"command":command,
                "returncode":None,"stdout":"","stderr":"","error":str(error)}
    seconds=time.perf_counter()-start
    try:
        value=json.loads(process.stdout)
        if process.returncode != 0:
            raise ValueError(f"exit status {process.returncode}")
        if not isinstance(value,dict):
            raise ValueError("solver output must be a JSON object")
        if value.get("status", "ok") != "ok" or \
                (value.get("solutions"),value.get("checksum")) != EXPECTED:
            raise ValueError(f"incorrect result: {value}")
    except (json.JSONDecodeError, ValueError) as error:
        return {"variant":variant,"status":"error","seconds":seconds,
                "command":command,"returncode":process.returncode,
                "stdout":process.stdout,"stderr":process.stderr,"error":str(error)}
    value.update(status="ok",seconds=seconds,command=command,returncode=process.returncode,
                 stdout=process.stdout,stderr=process.stderr,error=None)
    return value

def main()->int:
    parser=argparse.ArgumentParser()
    parser.add_argument("--binary",type=Path,required=True)
    parser.add_argument("--repetitions",type=int,default=20)
    parser.add_argument("--timeout",type=float,default=60.0)
    parser.add_argument("--output",type=Path)
    args=parser.parse_args(); rows=[]
    for repetition in range(args.repetitions):
        order=("bounded","compact") if repetition%2==0 else ("compact","bounded")
        for variant in order:
            row=run(args.binary,variant,args.timeout); row["repetition"]=repetition
            rows.append(row)
    summary={}; issues=[]
    for variant in ("bounded","compact"):
        selected=[row for row in rows if row["variant"]==variant]
        statuses={row["status"] for row in selected}
        successful=[row for row in selected if row["status"]=="ok"]
        if statuses != {"ok"}:
            issues.append(f"{variant}: statuses={sorted(statuses)}")
        if successful:
            first=successful[0]
            if any(any((field in row) != (field in first) or
                       row.get(field) != first.get(field) for field in COUNTERS)
                   for row in successful[1:]):
                issues.append(f"{variant}: unstable status/counters")
        seconds=[row["seconds"] for row in successful]
        summary[variant]={
          "statuses":{status:sum(row["status"]==status for row in selected)
                      for status in ("ok","timeout","error")},
          "median_seconds":statistics.median(seconds) if seconds else None,
          "min_seconds":min(seconds) if seconds else None,
          "max_seconds":max(seconds) if seconds else None,
          "counters":{field:successful[0].get(field) for field in COUNTERS}
                     if successful else None}
    report={"repetitions":args.repetitions,"binary":str(args.binary.resolve()),
            "summary":summary,"runs":rows,"validation":{"issues":issues}}
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2,sort_keys=True)+"\n")
    print(json.dumps({"repetitions":args.repetitions,"summary":summary,
                      "validation":{"issues":issues}},
                     indent=2,sort_keys=True))
    return 1 if issues else 0

if __name__=="__main__": raise SystemExit(main())
