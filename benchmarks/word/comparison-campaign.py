#!/usr/bin/env python3
"""Resumable, resource-bounded word-037 comparison campaign."""
from __future__ import annotations
import argparse, hashlib, importlib.util, json, math, os, platform, random, shutil, signal, statistics, subprocess, time
from collections import Counter, defaultdict
from pathlib import Path

HERE=Path(__file__).resolve().parent
FAMILIES=("dma","register","inverse","crc-xorshift","reduced-speck","bounded-alu")
CONFIGS=("gecode-baseline","gecode-candidate","z3","bitwuzla")
SEARCHES={"gecode-baseline":"lsb","gecode-candidate":"split-min"}
FINAL={"measured","timeout","unknown","error","memory-limit","unsupported","deferred"}

def imported(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    if spec is None or spec.loader is None: raise RuntimeError(f"cannot import {path}")
    module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module); return module
MIXED=imported("word037_mixed",HERE/"mixed-model-comparison.py")
BITS=imported("word037_bits",HERE/"word-bit-network-comparison.py")

def load(path): return json.loads(path.read_text())
def dump(path,value):
    path.parent.mkdir(parents=True,exist_ok=True); tmp=path.with_suffix(path.suffix+".tmp")
    tmp.write_text(json.dumps(value,indent=2,sort_keys=True)+"\n"); tmp.replace(path)
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def expected_rows(case): return BITS.evaluate(case) if case["harness"]=="bit-network" else MIXED.expected(case)

def native(case,search,args,batch=1):
    p,k=case["parameters"],case["native_kind"]
    if k=="dma": return [str(args.dma_binary),"-formulation","bounded-word","-search-control","native" if search=="lsb" else "public-min","-projection","all","-size",str(p["descriptor_count"]),"-window-slack",str(p["window_slack"]),"-selected-cap",str(p["selected_cap"]),"-measurement","batch","-batch",str(batch),"-solutions","0"]
    if k=="lookup": return [str(args.lookup_binary),"-formulation","bounded-word","-size",str(p["size"]),"-allowed-mask",str(sum(1<<i for i in p["allowed_indices"])),"-projection","all","-search-control",search,"-batch",str(batch),"-solutions","0"]
    if k=="allocation": return [str(args.allocation_binary),"register","bounds",str(p["registers_per_bank"]),str(batch),str(p["slots_per_bank"]),"projections","--search",search]
    if k=="inverse": return [str(args.inverse_binary),"--case",case["native_case"],"--search",search,"--batch",str(batch)]
    if k=="alu": return [str(args.alu_binary),"--comparison-width",str(p["width"]),"--input-min",str(p["input_min"]),"--input-max",str(p["input_max"]),"--output-min",str(p["output_min"]),"--output-max",str(p["output_max"]),"--search",search,"--batch",str(batch)]
    masks=(p.get("known_masks",[p.get("known_mask",0)])+[0]*4)[:4]; values=(p.get("known_values",[p.get("known_value",0)])+[0]*4)[:4]
    excluded=((p.get("exclude_projection") or [])+[0]*4)[:4]
    return [str(args.bit_binary),"--parameters",case["family"],str(p["rounds"]),*[str(x) for pair in zip(masks,values) for x in pair],str(p["output_mask"]),str(p["output_value"]),str(int(p.get("exclude_projection") is not None)),*[str(x) for x in excluded],"--search",search,"--batch",str(batch)]

def smt_problem(case):
    body,_=BITS.smt(case) if case["harness"]=="bit-network" else MIXED.smt_case(case)
    return "(set-logic QF_BV)\n"+"\n".join(body)+"\n(check-sat)\n"
def smt_batch(case,n): return "\n".join("(reset)\n"+smt_problem(case) for _ in range(n))

def derive(manifest):
    cases=manifest.get("cases",[]); calibration=manifest.get("calibration_cases")
    if manifest.get("schema_version")!=2 or manifest.get("campaign")!="word-037" or len(cases)!=72 or len({c["id"] for c in cases})!=72: raise ValueError("manifest must have 72 unique explicit cases")
    if not isinstance(calibration,list) or {c.get("campaign_family") for c in calibration}!=set(FAMILIES) or len({c.get("id") for c in calibration})!=len(calibration): raise ValueError("distinct calibration cases missing or duplicated")
    seen=set(); result={}
    for family in FAMILIES:
        group=[c for c in cases if c["campaign_family"]==family]
        if len(group)!=12: raise ValueError(f"{family}: expected 12 cases")
        for level in ("small","medium","large"):
            part=[c for c in group if c["level"]==level]
            if len(part)!=4 or sorted(c["expected_status"] for c in part)!=["sat","sat","unsat","unsat"]: raise ValueError(f"{family}/{level}: bad SAT split")
        maxima=[max(c["scale"]["value"] for c in group if c["level"]==x) for x in ("small","medium","large")]
        if not maxima[0]<maxima[1]<maxima[2]: raise ValueError(f"{family}: non-increasing scale")
    for case in cases:
        key=json.dumps([case["family"],case["parameters"]],sort_keys=True)
        if key in seen: raise ValueError(f"{case['id']}: duplicate parameters")
        seen.add(key); rows=expected_rows(case); status="sat" if rows else "unsat"
        if status!=case["expected_status"] or len(rows)!=case["expected_solutions"]: raise ValueError(f"{case['id']}: expected result drift")
        if case["harness"]=="mixed" and case["decision_variables"]!=MIXED.expected_names(case): raise ValueError(f"{case['id']}: projection drift")
        result[case["id"]]={"status":status,"solutions":len(rows),"projections":rows}
    measured_keys={json.dumps([c["family"],c["parameters"]],sort_keys=True) for c in cases}
    for case in calibration:
        if case["id"] in result or json.dumps([case["family"],case["parameters"]],sort_keys=True) in measured_keys: raise ValueError("calibration cases must be outside the measured matrix")
        rows=expected_rows(case)
        if not rows or case["expected_status"]!="sat" or len(rows)!=case["expected_solutions"]: raise ValueError(f"{case['id']}: calibration expectation drift")
        result[case["id"]]={"status":"sat","solutions":len(rows),"projections":rows}
    return result

def semantic_native(stdout,expect):
    try: value=json.loads(stdout)
    except json.JSONDecodeError: return False,"native output is not JSON",{}
    ok=value.get("semantic_status")==expect["status"] and sorted(value.get("projections",[]))==sorted(expect["projections"])
    return ok,"ok" if ok else "native status/witness mismatch",value
def semantic_smt(stdout,expect,batch):
    statuses=[x.strip() for x in stdout.splitlines() if x.strip() in ("sat","unsat","unknown")]
    ok=len(statuses)==batch and all(x==expect["status"] for x in statuses)
    return ok,"ok" if ok else f"SMT statuses {statuses}",{"statuses":statuses}

def validate(args):
    manifest=load(args.manifest); expect=derive(manifest); checked=[]
    if shutil.which(args.z3):
        for family in FAMILIES:
            case=next(c for c in manifest["cases"] if c["campaign_family"]==family and c["level"]=="small" and c["expected_status"]=="sat")
            done=subprocess.run([args.z3,"-smt2","-in"],input=smt_problem(case),text=True,capture_output=True,timeout=args.timeout)
            ok,detail,_=semantic_smt(done.stdout,expect[case["id"]],1)
            if done.returncode or not ok: raise ValueError(f"{case['id']}: {detail}")
            checked.append(case["id"])
    summary={"status":"pass","instances":72,"calibration_instances":len(manifest["calibration_cases"]),"cells":288,"sat":sum(c["expected_status"]=="sat" for c in manifest["cases"]),"unsat":sum(c["expected_status"]=="unsat" for c in manifest["cases"]),"representative_z3_checks":checked}
    if args.output: dump(args.output,{"summary":summary,"cases":expect})
    print(json.dumps(summary,indent=2)); return 0

def smoke(args):
    manifest=load(args.manifest); expect=derive(manifest); output=[]
    for family in FAMILIES:
        case=next(c for c in manifest["cases"] if c["campaign_family"]==family and c["level"]=="small" and c["expected_status"]=="sat"); row={"case":case["id"]}
        for config,search in SEARCHES.items():
            if config=="gecode-candidate" and family in ("crc-xorshift","reduced-speck"): search="msb"
            command=native(case,search,args,2); done=subprocess.run(command,text=True,capture_output=True,timeout=args.timeout); ok,detail,_=semantic_native(done.stdout,expect[case["id"]]); row[config]={"status":"pass" if not done.returncode and ok else "fail","detail":detail,"command":command}
        if shutil.which(args.z3):
            done=subprocess.run([args.z3,"-smt2","-in"],input=smt_batch(case,2),text=True,capture_output=True,timeout=args.timeout); ok,detail,_=semantic_smt(done.stdout,expect[case["id"]],2); row["z3"]={"status":"pass" if not done.returncode and ok else "fail","detail":detail}
        else: row["z3"]={"status":"unsupported","detail":"z3 absent"}
        row["bitwuzla"]={"status":"unsupported","detail":"no validated adapter"}; output.append(row)
    ok=all(all(row[c]["status"] in ("pass","unsupported") for c in CONFIGS) for row in output)
    print(json.dumps({"status":"pass" if ok else "fail","results":output},indent=2)); return 0 if ok else 1

def binary_hashes(args):
    paths=[Path(getattr(args,name+"_binary")) for name in ("bit","dma","lookup","allocation","inverse","alu")]+[Path(args.z3)]
    missing=[str(path) for path in paths if not path.is_file()]
    if missing: raise ValueError("missing benchmark executable(s): "+", ".join(missing))
    return {str(path.resolve()):sha(path) for path in paths}
def command_output(command):
    done=subprocess.run(command,text=True,capture_output=True,timeout=10)
    return {"command":command,"returncode":done.returncode,"stdout":done.stdout.strip(),"stderr":done.stderr.strip()}
def host_identity():
    identity={"system":platform.system(),"release":platform.release(),"machine":platform.machine(),"python":platform.python_version(),"logical_cpus":os.cpu_count()}
    if platform.system()=="Darwin":
        for key in ("machdep.cpu.brand_string","hw.model"):
            done=subprocess.run(["sysctl","-n",key],text=True,capture_output=True)
            if done.returncode==0 and done.stdout.strip(): identity[key]=done.stdout.strip()
    return identity
def frozen(args,manifest):
    head=subprocess.run(["git","rev-parse","HEAD"],text=True,capture_output=True,check=True).stdout.strip()
    root=HERE.parent.parent
    sources=[HERE/"comparison-campaign.py",HERE/"word-037-cases.json",HERE/"mixed-model-comparison.py",HERE/"word-bit-network-comparison.py",root/"benchmarks/word/distinct.cpp",root/"examples/word-bit-network-comparison.cpp",root/"examples/word-inverse-arithmetic.cpp",root/"examples/word-register-file.cpp",root/"examples/word-symbolic-alu.cpp",root/"gecode/word/arithmetic/add.hpp",root/"gecode/word/arithmetic/mult.hpp",root/"gecode/word/arithmetic/neg-sub.hpp"]
    return {"schema_version":1,"campaign":"word-037-local","git_head":head,"source_hashes":{str(p.relative_to(root)):sha(p) for p in sources},"runner":"native subprocess","host":host_identity(),"binary_hashes":binary_hashes(args),"binary_paths":{n:str(Path(getattr(args,n+"_binary")).resolve()) for n in ("bit","dma","lookup","allocation","inverse","alu")}|{"z3":str(Path(args.z3).resolve())},"solver_probes":{"z3":command_output([args.z3,"--version"])},"limits":{"wall_seconds":{"screen":30,"followup":300},"campaign_budget_seconds":args.cpu_budget,"cpu_affinity":"host default","memory":"host default; RSS not sampled"},"options":{"repeats":5,"tiny_min_seconds":0.25},"matrix":{"instances":72,"configurations":list(CONFIGS),"cells":288}}
def ensure_root(args,manifest):
    args.root.mkdir(parents=True,exist_ok=True); path=args.root/"metadata.json"; now=frozen(args,manifest)
    if path.exists() and load(path)!=now: raise ValueError("result root has a different frozen identity/options")
    if not path.exists(): dump(path,now)
    (args.root/"records").mkdir(exist_ok=True)
    plan=[{"case":c["id"],"family":c["campaign_family"],"level":c["level"],"expected_status":c["expected_status"],"config":k} for c in manifest["cases"] for k in CONFIGS]
    dump(args.root/"screen-plan.json",{"cells":len(plan),"plan":plan})
def record_path(args,phase,case,config,repeat): return args.root/"records"/f"{phase}--{case}--{config}--r{repeat}.json"
def records(args):
    out=[]
    for path in (args.root/"records").glob("*.json"):
        try: out.append(load(path))
        except (OSError,json.JSONDecodeError): pass
    return out
def ledger(args):
    return sum(float(x.get("cpu_seconds") or 0) for x in records(args))
def local_run(args,command,stdin,timeout,identity):
    del identity
    started=time.perf_counter()
    process=subprocess.Popen(command,stdin=subprocess.PIPE if stdin is not None else None,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
    try:
        stdout,stderr=process.communicate(stdin,timeout=timeout); timeout_hit=False
    except subprocess.TimeoutExpired:
        timeout_hit=True; os.killpg(process.pid,signal.SIGKILL); stdout,stderr=process.communicate()
    wall=time.perf_counter()-started
    status="timeout" if timeout_hit else ("error" if process.returncode else "measured")
    charged=timeout if timeout_hit else wall
    return {"status":status,"returncode":process.returncode,"stdout":stdout,"stderr":stderr,"wall_seconds":wall,"cpu_seconds":charged,"cpu_charge":"wall-clock campaign ledger","max_rss_kib":None,"runner":"native subprocess","command":command}

def execute(args,case,config,expect,phase,repeat,timeout,batch):
    path=record_path(args,phase,case["id"],config,repeat)
    if path.exists() and load(path).get("status") in FINAL: return load(path)
    common={"phase":phase,"case":case["id"],"family":case["campaign_family"],"level":case["level"],"expected_status":expect["status"],"config":config,"repeat":repeat,"batch":batch}
    if config=="bitwuzla":
        value={**common,"status":"unsupported","detail":"no validated word-037 adapter","cpu_seconds":0}; dump(path,value); return value
    if ledger(args)+timeout>args.cpu_budget:
        value={**common,"status":"deferred","detail":"CPU ledger cannot preauthorize timeout","cpu_seconds":0}; dump(path,value); return value
    if config.startswith("gecode"):
        search="lsb" if config=="gecode-baseline" else args.selection[case["campaign_family"]]; command=native(case,search,args,batch); stdin=None
    else: command=[args.z3,"-smt2","-in"]; stdin=smt_batch(case,batch)
    value=local_run(args,command,stdin,timeout,f"{phase}-{case['id']}-{config}-{repeat}"); value.update(common)
    if value["status"]=="measured":
        ok,detail,parsed=semantic_native(value["stdout"],expect) if config.startswith("gecode") else semantic_smt(value["stdout"],expect,batch)
        value["semantic_validation"]=detail; value["parsed"]=parsed
        if not ok: value["status"]="unknown" if config=="z3" and "unknown" in value["stdout"].splitlines() else "error"
        else: value["seconds_per_problem"]=value["cpu_seconds"]/batch
    dump(path,value); return value

def load_selection(args):
    path=args.root/"calibration.json"
    if not path.exists(): raise ValueError("run calibrate first")
    return load(path)["selection"]
def calibrate(args):
    manifest=load(args.manifest); ensure_root(args,manifest); expect=derive(manifest); args.selection={x:("msb" if x in ("crc-xorshift","reduced-speck") else "split-min") for x in FAMILIES}; samples=[]
    for case in manifest["calibration_cases"]:
        for config in SEARCHES: samples.append(execute(args,case,config,expect[case["id"]],"calibration",0,30,1))
    selection={}
    for family in FAMILIES:
        # The baseline is fixed by the task.  Calibration admits and freezes the
        # one distinct alternative; it never turns the candidate into a second
        # spelling of the baseline merely because LSB won these calibration rows.
        candidate=[r for r in samples if r["family"]==family and r["config"]=="gecode-candidate"]
        selection[family]=("msb" if family in ("crc-xorshift","reduced-speck") else "split-min") if candidate and all(r["status"]=="measured" for r in candidate) else "unsupported"
        if selection[family]=="unsupported": raise ValueError(f"{family}: alternative search failed calibration")
    dump(args.root/"calibration.json",{"selection":selection,"calibration_only_cases":[c["id"] for c in manifest["calibration_cases"]],"samples":[{"case":r["case"],"config":r["config"],"status":r["status"],"seconds":r.get("seconds_per_problem")} for r in samples]})
    print(json.dumps({"status":"pass","selection":selection},indent=2)); return 0
def batch_for(args,case,config,expect):
    batch=1
    while batch<=65536:
        probe=execute(args,case,config,expect,f"batch-probe-{batch}",0,30,batch)
        cpu=probe.get("cpu_seconds")
        if probe["status"]!="measured" or (cpu and cpu>=.25): return batch
        batch*=2
    return batch
def screen(args):
    manifest=load(args.manifest); ensure_root(args,manifest); args.selection=load_selection(args); expect=derive(manifest); result=[]
    for case in manifest["cases"]:
        for config in CONFIGS:
            batch=1 if config=="bitwuzla" else batch_for(args,case,config,expect[case["id"]]); result.append(execute(args,case,config,expect[case["id"]],"screen",0,30,batch))
    print(json.dumps({"cells":len(result),"status_counts":dict(Counter(x["status"] for x in result)),"cpu_seconds":ledger(args)},indent=2)); return 0
def followup(args):
    manifest=load(args.manifest); ensure_root(args,manifest); args.selection=load_selection(args); expect=derive(manifest); wanted=set(args.case); chosen=[c for c in manifest["cases"] if c["id"] in wanted]
    if len(chosen)!=len(wanted) or not chosen: raise ValueError("--case must name known selected cases")
    jobs=[]
    for repeat in range(1,6):
        rotated=list(CONFIGS[repeat%len(CONFIGS):]+CONFIGS[:repeat%len(CONFIGS)])
        for offset in range(len(chosen)):
            for config in rotated: jobs.append((chosen[(offset+repeat-1)%len(chosen)],config,repeat))
    result=[]
    for case,config,repeat in jobs:
        batch=1 if config=="bitwuzla" else batch_for(args,case,config,expect[case["id"]]); result.append(execute(args,case,config,expect[case["id"]],"followup",repeat,300,batch))
    print(json.dumps({"records":len(result),"status_counts":dict(Counter(x["status"] for x in result)),"cpu_seconds":ledger(args)},indent=2)); return 0

def analyze(args):
    selected=[r for r in records(args) if r.get("phase") in ("screen","followup")]; screen_rows=[r for r in selected if r["phase"]=="screen"]; followup_rows=[r for r in selected if r["phase"]=="followup"]
    cells={(r["case"],r["config"]) for r in screen_rows}; groups=defaultdict(list); by_sat=defaultdict(Counter); by_config=defaultdict(Counter)
    for r in screen_rows: by_sat[r["expected_status"]][r["status"]]+=1; by_config[r["config"]][r["status"]]+=1
    for r in screen_rows:
        if r["status"]=="measured": groups[(r["family"],r["expected_status"],r["config"])].append(r["seconds_per_problem"])
    timings=[{"family":k[0],"expected_status":k[1],"config":k[2],"n":len(v),"median_seconds":statistics.median(v),"spread_seconds":max(v)-min(v),"minimum_seconds":min(v),"maximum_seconds":max(v)} for k,v in sorted(groups.items())]
    runs=defaultdict(dict)
    for r in screen_rows:
        if r["status"]=="measured": runs[(r["phase"],r["case"],r["repeat"])][r["config"]]=r
    screen_ratios=[]
    for run in runs.values():
        for contender in ("gecode-candidate","z3"):
            if "gecode-baseline" in run and contender in run:
                baseline=run["gecode-baseline"].get("seconds_per_problem")
                other=run[contender].get("seconds_per_problem")
                if baseline is not None and other is not None and baseline>0 and other>0:
                    screen_ratios.append({"family":run[contender]["family"],"expected_status":run[contender]["expected_status"],"contender":contender,"baseline_over_contender":baseline/other})
    status_counts=Counter(r["status"] for r in screen_rows); status_counts["deferred"]+=288-len(cells)
    ratio_groups=defaultdict(list)
    for r in screen_ratios: ratio_groups[(r["family"],r["contender"])].append(r["baseline_over_contender"])
    def classification(value): return "win" if value>1 else ("loss" if value<1 else "parity")
    conclusions=[{"family":k[0],"contender":k[1],"paired_median":statistics.median(v),"range":[min(v),max(v)],"result":classification(statistics.median(v))} for k,v in sorted(ratio_groups.items())]
    followup_by_case=defaultdict(list)
    for row in followup_rows: followup_by_case[row["case"]].append(row)
    followups=[]
    for case,case_rows in sorted(followup_by_case.items()):
        measured={config:[r["seconds_per_problem"] for r in case_rows if r["config"]==config and r["status"]=="measured"] for config in ("gecode-baseline","gecode-candidate","z3")}
        pairs=defaultdict(dict)
        for row in case_rows:
            if row["status"]=="measured": pairs[row["repeat"]][row["config"]]=row["seconds_per_problem"]
        pair_summaries=[]
        for contender in ("gecode-candidate","z3"):
            values=[run["gecode-baseline"]/run[contender] for run in pairs.values() if run.get("gecode-baseline",0)>0 and run.get(contender,0)>0]
            if values: pair_summaries.append({"contender":contender,"n":len(values),"median":statistics.median(values),"range":[min(values),max(values)],"result":classification(statistics.median(values)),"stable":all(v>1 for v in values) or all(v<1 for v in values) or all(v==1 for v in values)})
        followups.append({"case":case,"family":case_rows[0]["family"],"expected_status":case_rows[0]["expected_status"],"timings":{k:{"n":len(v),"median_seconds":statistics.median(v),"range_seconds":[min(v),max(v)]} for k,v in measured.items() if v},"paired":pair_summaries})
    stable=[{"case":case["case"],"family":case["family"],**pair} for case in followups for pair in case["paired"] if pair["contender"]=="gecode-candidate" and pair["stable"] and pair["result"]!="parity"]
    stable.sort(key=lambda x:abs(math.log(x["median"])),reverse=True)
    priorities=[f"{x['case']}: candidate is a stable {x['result']} versus baseline ({x['median']:.3g}× baseline/candidate, {x['range'][0]:.3g}–{x['range'][1]:.3g})" for x in stable[:2]]
    metadata=load(args.root/"metadata.json"); calibration=load(args.root/"calibration.json")
    frozen={"git_head":metadata["git_head"],"runner":metadata["runner"],"host":metadata["host"],"limits":metadata["limits"],"options":metadata["options"],"calibration_selection":calibration["selection"],"external_root":str(args.root.resolve())}
    result={"schema_version":1,"frozen":frozen,"screen_accounting":{"expected_cells":288,"recorded_cells":len(cells),"status_counts":dict(status_counts),"sat_unsat":{k:dict(v) for k,v in by_sat.items()},"by_config":{k:dict(v) for k,v in by_config.items()}},"screen_timing_groups":timings,"screen_paired_ratios":screen_ratios,"screen_family_conclusions":conclusions,"followup_cases":followups,"priorities":priorities,"rss":{"method":"unavailable in direct local mode","maximum_by_config":{c:None for c in CONFIGS}},"timeout_aware":{c:{s:sum(r["config"]==c and r["status"]==s for r in screen_rows) for s in ("measured","timeout","memory-limit","unknown","error","unsupported","deferred")} for c in CONFIGS},"counters":{"note":"Gecode nodes/failures/propagations and SMT statuses are separate measures, not identical work","gecode":[{"case":r["case"],"config":r["config"],**{x:r.get("parsed",{}).get(x) for x in ("nodes","failures","propagations")}} for r in selected if r["config"].startswith("gecode") and r["status"]=="measured"],"smt":[{"case":r["case"],"config":r["config"],"statuses":r.get("parsed",{}).get("statuses")} for r in selected if r["config"]=="z3" and r["status"]=="measured"]},"cpu_seconds":ledger(args),"external_root":str(args.root.resolve())}
    dump(args.root/"analysis.json",result); counts=result["screen_accounting"]["status_counts"]
    lines=["# Word-037 comparison campaign","",f"External result root: `{args.root.resolve()}`",f"Git revision: `{frozen['git_head']}`",f"Runner: `{frozen['runner']}` on `{frozen['host']['system']} {frozen['host']['release']} {frozen['host']['machine']}`",f"Limits: `{json.dumps(frozen['limits'],sort_keys=True)}`",f"Options: `{json.dumps(frozen['options'],sort_keys=True)}`",f"Calibration selection: `{json.dumps(frozen['calibration_selection'],sort_keys=True)}`","",f"Screen accounting: {len(cells)}/288 cells; "+", ".join(f"{k}={v}" for k,v in sorted(counts.items()))+f". Wall-time ledger: {result['cpu_seconds']:.2f}/{metadata['limits']['campaign_budget_seconds']:.0f} seconds.","","All solver processes ran directly on the host. CPU affinity and memory were left at the macOS defaults; wall caps bound individual runs. Per-process RSS is unavailable in this mode.","","Gecode search counters and SMT statuses are retained separately; they are not treated as the same work metric.","","## Screen timing groups","","These groups use only the single screen run for each measured matrix cell.","","| Family | Status | Configuration | n | Median s | Range s |","|---|---|---|---:|---:|---:|"]
    for s in timings: lines.append(f"| {s['family']} | {s['expected_status']} | {s['config']} | {s['n']} | {s['median_seconds']:.6g} | {s['minimum_seconds']:.6g}–{s['maximum_seconds']:.6g} |")
    lines += ["","## Broad screen ranges",""]
    lines += ([f"- {c['family']}: {c['contender']} paired median {c['paired_median']:.3g}× (range {c['range'][0]:.3g}–{c['range'][1]:.3g}); {c['result']} versus baseline." for c in conclusions] or ["- No paired measurements are available; all absent screen cells are accounted for as deferred."])
    lines += ["","## Resource and completion metrics","","Per-process peak RSS is unavailable in direct local mode.","","| Configuration | Maximum RSS KiB |","|---|---:|"]
    for config in CONFIGS:
        rss=result["rss"]["maximum_by_config"][config] if result["rss"]["maximum_by_config"][config] is not None else "unavailable"
        lines.append(f"| {config} | {rss} |")
    lines += ["","Timeout-aware counts use screen rows only.","","| Configuration | Measured | Timeout | Memory limit | Unknown | Error | Unsupported | Deferred |","|---|---:|---:|---:|---:|---:|---:|---:|"]
    for config in CONFIGS:
        status=result["timeout_aware"][config]
        lines.append(f"| {config} | {status['measured']} | {status['timeout']} | {status['memory-limit']} | {status['unknown']} | {status['error']} | {status['unsupported']} | {status['deferred']} |")
    lines += ["","## Five-repeat selected cases",""]
    for case in followups:
        lines.append(f"### {case['case']} ({case['expected_status']})"); lines.append("")
        for config,timing in case["timings"].items(): lines.append(f"- {config}: n={timing['n']}, median {timing['median_seconds']:.6g}s, range {timing['range_seconds'][0]:.6g}–{timing['range_seconds'][1]:.6g}s.")
        for pair in case["paired"]: lines.append(f"- baseline/{pair['contender']}: n={pair['n']}, median {pair['median']:.3g}×, range {pair['range'][0]:.3g}–{pair['range'][1]:.3g}; {pair['result']}{' (stable)' if pair['stable'] else ''}.")
        lines.append("")
    lines += ["## Data-derived priorities",""]+( [f"- {p}" for p in priorities] if priorities else ["- No stable non-parity follow-up comparison warrants a priority."] )+[""]
    report="\n".join(lines)
    (args.root/"result.md").write_text(report)
    (HERE/"word-037-result.md").write_text(report)
    print(json.dumps(result["screen_accounting"],indent=2)); return 0

def main():
    p=argparse.ArgumentParser(); p.add_argument("command",choices=("validate","smoke","calibrate","screen","followup","analyze")); p.add_argument("--manifest",type=Path,default=HERE/"word-037-cases.json"); p.add_argument("--output",type=Path); p.add_argument("--timeout",type=float,default=30); p.add_argument("--z3",default=shutil.which("z3")); p.add_argument("--root",type=Path,default=Path("/private/tmp/gecode-word-037-campaign-local")); p.add_argument("--cpu-budget",type=float,default=21600); p.add_argument("--case",action="append",default=[])
    root=Path("build/bin")
    for name,file in (("bit","word-bit-network-comparison"),("dma","word-dma-descriptor"),("lookup","word-register-file"),("allocation","word-distinct-benchmark"),("inverse","word-inverse-arithmetic"),("alu","word-symbolic-alu")): p.add_argument(f"--{name}-binary",type=Path,default=root/file)
    args=p.parse_args()
    if not args.z3: p.error("z3 was not found; pass --z3")
    try: return {"validate":validate,"smoke":smoke,"calibrate":calibrate,"screen":screen,"followup":followup,"analyze":analyze}[args.command](args)
    except (KeyError,OSError,ValueError,subprocess.CalledProcessError,subprocess.TimeoutExpired) as error: p.error(str(error))
if __name__=="__main__": raise SystemExit(main())
