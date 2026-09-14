#!/usr/bin/env -S uv run --script
"""Run the interleaved Release comparison for Word n-ary Add."""
from __future__ import annotations
import argparse, json, statistics, subprocess, time
from pathlib import Path
from semantics import validate_contract
EXPECTED={4:(86,156576),6:(676,2627648),8:(3788,25471040)}
CASES=tuple({"schema_version":1,"id":f"segments-{n}","kind":"nary-add",
             "goal":"first","expected_status":"sat",
             "decision_variables":[f"length[{i}]" for i in range(n)],
             "parameters":{"segments":n,"total":160*n},
             "expected":EXPECTED[n]} for n in EXPECTED) + (
       {"schema_version":1,"id":"segments-4-unsat","kind":"nary-add",
        "goal":"unsat","expected_status":"unsat",
        "decision_variables":[f"length[{i}]" for i in range(4)],
        "parameters":{"segments":4,"total":1},"expected":(0,0)},)
COUNTERS=("status","semantic_status","solutions","checksum","nodes","failures",
          "propagations","decision_variables","projections")
def captured(value:bytes|str|None)->str:
    return value.decode(errors="replace") if isinstance(value,bytes) else value or ""
def run(binary:Path,case:dict,variant:str,timeout:float)->dict:
    n=case["parameters"]["segments"]
    total=case["parameters"]["total"]
    start=time.perf_counter()
    command=[str(binary),"--segments",str(n),"--variant",variant,
             "--total",str(total)]
    try:
        p=subprocess.run(command,text=True,capture_output=True,check=False,
                         timeout=timeout)
    except subprocess.TimeoutExpired as error:
        return {"segments":n,"variant":variant,"status":"timeout",
                "seconds":timeout,"command":command,"returncode":None,
                "stdout":captured(error.stdout),"stderr":captured(error.stderr),
                "error":str(error)}
    except OSError as error:
        return {"segments":n,"variant":variant,"status":"error",
                "seconds":time.perf_counter()-start,"command":command,
                "returncode":None,"stdout":"","stderr":"","error":str(error)}
    seconds=time.perf_counter()-start
    try:
        value=json.loads(p.stdout)
        if p.returncode != 0:
            raise ValueError(f"exit status {p.returncode}")
        if not isinstance(value,dict):
            raise ValueError("solver output must be a JSON object")
        if value.get("status") != "ok" or \
                (value.get("solutions"),value.get("checksum")) != case["expected"]:
            raise ValueError(f"incorrect result: {value}")
        validate_contract(case,value)
    except (json.JSONDecodeError, ValueError) as error:
        return {"segments":n,"variant":variant,"status":"error",
                "seconds":seconds,"command":command,"returncode":p.returncode,
                "stdout":p.stdout,"stderr":p.stderr,"error":str(error)}
    value.update(case_id=case["id"],status="ok",seconds=seconds,command=command,returncode=p.returncode,
                 stdout=p.stdout,stderr=p.stderr,error=None)
    return value
def main()->int:
    ap=argparse.ArgumentParser(); ap.add_argument("--binary",type=Path,required=True)
    ap.add_argument("--repetitions",type=int,default=20)
    ap.add_argument("--timeout",type=float,default=60.0)
    ap.add_argument("--output",type=Path); args=ap.parse_args(); rows=[]
    for rep in range(args.repetitions):
        order=("bounded","compact") if rep%2==0 else ("compact","bounded")
        for case in CASES:
            for variant in order:
                row=run(args.binary,case,variant,args.timeout)
                row.update(case_id=case["id"],repetition=rep); rows.append(row)
    summary={}; issues=[]
    for case in CASES:
        key=case["id"]
        summary[key]={}
        for variant in ("bounded","compact"):
            selected=[r for r in rows if r["case_id"]==key and r["variant"]==variant]
            statuses={row["status"] for row in selected}
            successful=[row for row in selected if row["status"]=="ok"]
            if statuses != {"ok"}:
                issues.append(f"{key}/{variant}: statuses={sorted(statuses)}")
            if successful:
                first=successful[0]
                if any(any((field in row) != (field in first) or
                           row.get(field) != first.get(field) for field in COUNTERS)
                       for row in successful[1:]):
                    issues.append(f"{key}/{variant}: unstable status/counters")
            seconds=[row["seconds"] for row in successful]
            summary[key][variant]={
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
                      "validation":{"issues":issues}},indent=2,sort_keys=True))
    return 1 if issues else 0
if __name__=="__main__": raise SystemExit(main())
