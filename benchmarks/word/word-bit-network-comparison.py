#!/usr/bin/env -S python3
"""Exact CRC, xorshift32, and reduced-Speck public-projection checks."""

from __future__ import annotations

import argparse
import copy
import itertools
import json
import selectors
import shutil
import subprocess
import time
from pathlib import Path
from typing import Any, Callable


def rol(value: int, amount: int, width: int) -> int:
    mask = (1 << width) - 1
    return ((value << amount) | (value >> (width - amount))) & mask


def ror(value: int, amount: int, width: int) -> int:
    return rol(value, width - amount, width)


def crc16(message: int, width: int, rounds: int) -> int:
    state = 0x1D0F
    for index in range(rounds):
        feedback = ((state >> 15) ^ (message >> (width - 1 - index))) & 1
        state = ((state << 1) & 0xFFFF) ^ (0x1021 if feedback else 0)
    return state


def xorshift32(state: int, rounds: int) -> int:
    for _ in range(rounds):
        state ^= (state << 13) & 0xFFFFFFFF
        state ^= state >> 17
        state ^= (state << 5) & 0xFFFFFFFF
        state &= 0xFFFFFFFF
    return state


def speck32_64(keys: list[int], rounds: int) -> int:
    round_keys, l = [keys[0]], list(keys[1:])
    for index in range(rounds - 1):
        next_l = ((ror(l[index], 7, 16) + round_keys[index]) & 0xFFFF) ^ index
        l.append(next_l)
        round_keys.append(rol(round_keys[index], 2, 16) ^ next_l)
    x, y = 0x6574, 0x694C
    for key in round_keys:
        x = ((ror(x, 7, 16) + y) & 0xFFFF) ^ key
        y = rol(y, 2, 16) ^ x
    return (x << 16) | y


def candidates(mask: int, value: int, width: int) -> list[int]:
    unknown = [bit for bit in range(width) if not (mask >> bit) & 1]
    base = value & mask
    return [base | sum(((choice >> offset) & 1) << bit
                       for offset, bit in enumerate(unknown))
            for choice in range(1 << len(unknown))]


def evaluate(case: dict[str, Any]) -> list[list[int]]:
    family, p = case["family"], case["parameters"]
    if family == "crc16":
        inputs = ([value] for value in candidates(p["known_mask"], p["known_value"], p["width"]))
        output: Callable[[list[int]], int] = lambda row: crc16(row[0], p["width"], p["rounds"])
    elif family == "xorshift32":
        inputs = ([value] for value in candidates(p["known_mask"], p["known_value"], 32))
        output = lambda row: xorshift32(row[0], p["rounds"])
    else:
        pools = [candidates(mask, value, 16) for mask, value in
                 zip(p["known_masks"], p["known_values"])]
        inputs = (list(values) for values in itertools.product(*pools))
        output = lambda row: speck32_64(row, p["rounds"])
    blocked = p.get("exclude_projection")
    return sorted(row for row in inputs
                  if output(row) & p["output_mask"] == p["output_value"] & p["output_mask"]
                  and row != blocked)


def gf2_rank(rows: list[int], columns: int) -> int:
    rank = 0
    for column in range(columns):
        pivot = next((i for i in range(rank, len(rows)) if rows[i] >> column & 1), None)
        if pivot is None:
            continue
        rows[rank], rows[pivot] = rows[pivot], rows[rank]
        for index in range(len(rows)):
            if index != rank and rows[index] >> column & 1:
                rows[index] ^= rows[rank]
        rank += 1
    return rank


def gf2(case: dict[str, Any]) -> dict[str, Any] | None:
    if case["family"] == "speck32_64":
        return None
    p = case["parameters"]
    width = p.get("width", 32)
    known_mask, known_value = p["known_mask"], p["known_value"]
    unknown = [bit for bit in range(width) if not known_mask >> bit & 1]
    function = ((lambda value: crc16(value, width, p["rounds"]))
                if case["family"] == "crc16" else
                (lambda value: xorshift32(value, p["rounds"])))
    base_output = function(known_value & known_mask)
    equations = []
    for out_bit in range(16 if case["family"] == "crc16" else 32):
        if not p["output_mask"] >> out_bit & 1:
            continue
        coefficients = sum((((function((known_value & known_mask) | (1 << bit)) ^ base_output)
                             >> out_bit) & 1) << index
                           for index, bit in enumerate(unknown))
        rhs = ((p["output_value"] ^ base_output) >> out_bit) & 1
        equations.append(coefficients | (rhs << len(unknown)))
    rank = gf2_rank([row & ((1 << len(unknown)) - 1) for row in equations], len(unknown))
    augmented = gf2_rank(list(equations), len(unknown) + 1)
    consistent = rank == augmented
    return {"unknown_bits": len(unknown), "rank": rank,
            "nullity": len(unknown) - rank if consistent else None,
            "consistent": consistent}


def bv(value: int, width: int) -> str:
    return f"(_ bv{value} {width})"


def smt(case: dict[str, Any]) -> tuple[list[str], list[tuple[str, int]]]:
    family, p = case["family"], case["parameters"]
    lines: list[str] = []
    if family == "crc16":
        names = [("message", p["width"])]
        lines.append(f"(declare-fun message () (_ BitVec {p['width']}))")
        lines.append(f"(assert (= (bvand message {bv(p['known_mask'],p['width'])}) {bv(p['known_value'] & p['known_mask'],p['width'])}))")
        state = bv(0x1D0F, 16)
        for index in range(p["rounds"]):
            bit = f"((_ extract {p['width']-1-index} {p['width']-1-index}) message)"
            feedback = f"(bvxor ((_ extract 15 15) {state}) {bit})"
            state = f"(bvxor (bvshl {state} {bv(1,16)}) (ite (= {feedback} #b1) {bv(0x1021,16)} {bv(0,16)}))"
        lines.append(f"(assert (= (bvand {state} {bv(p['output_mask'],16)}) {bv(p['output_value'] & p['output_mask'],16)}))")
    elif family == "xorshift32":
        names = [("state", 32)]; lines.append("(declare-fun state () (_ BitVec 32))")
        lines.append(f"(assert (= (bvand state {bv(p['known_mask'],32)}) {bv(p['known_value'] & p['known_mask'],32)}))")
        result = "state"
        for _ in range(p["rounds"]):
            result = f"(bvxor {result} (bvshl {result} {bv(13,32)}))"
            result = f"(bvxor {result} (bvlshr {result} {bv(17,32)}))"
            result = f"(bvxor {result} (bvshl {result} {bv(5,32)}))"
        lines.append(f"(assert (= (bvand {result} {bv(p['output_mask'],32)}) {bv(p['output_value'] & p['output_mask'],32)}))")
    else:
        names = [(f"key{i}",16) for i in range(4)]
        for i, (mask, value) in enumerate(zip(p["known_masks"],p["known_values"])):
            lines += [f"(declare-fun key{i} () (_ BitVec 16))",
                      f"(assert (= (bvand key{i} {bv(mask,16)}) {bv(value & mask,16)}))"]
        keys, ls = ["key0"], ["key1","key2","key3"]
        for index in range(p["rounds"] - 1):
            nxt = f"(bvxor (bvadd ((_ rotate_right 7) {ls[index]}) {keys[index]}) {bv(index,16)})"
            ls.append(nxt); keys.append(f"(bvxor ((_ rotate_left 2) {keys[index]}) {nxt})")
        x, y = bv(0x6574,16), bv(0x694C,16)
        for key in keys:
            x = f"(bvxor (bvadd ((_ rotate_right 7) {x}) {y}) {key})"
            y = f"(bvxor ((_ rotate_left 2) {y}) {x})"
        result = f"(concat {x} {y})"
        lines.append(f"(assert (= (bvand {result} {bv(p['output_mask'],32)}) {bv(p['output_value'] & p['output_mask'],32)}))")
    if p.get("exclude_projection") is not None:
        lines.append("(assert (not (and " + " ".join(
            f"(= {name} {bv(value,width)})" for (name,width),value in
            zip(names,p["exclude_projection"])) + ")))" )
    return lines, names


def solver_info(name: str, executable: str | None) -> dict[str, Any]:
    if executable is None:
        return {"solver":name,"status":"missing","command":None,"options":[],"version":None}
    options = ["-smt2","-in"] if name == "z3" else ["--lang","smt2","--produce-models"]
    try:
        done=subprocess.run([executable,"--version"],capture_output=True,text=True,timeout=2)
        if done.returncode: raise RuntimeError("version probe failed")
        return {"solver":name,"status":"ready","command":executable,"options":options,
                "version":done.stdout.strip().splitlines()[0]}
    except (OSError,subprocess.TimeoutExpired,RuntimeError,IndexError) as error:
        return {"solver":name,"status":"unsupported","command":executable,"options":options,
                "version":None,"error":str(error)}


def validate_scale_axes(cases: list[dict[str, Any]]) -> None:
    """Require each named scale case to change only its declared axis."""
    by_id = {case["id"]: case for case in cases}
    for family in ("crc", "xorshift", "speck"):
        base = by_id[f"{family}-base"]["parameters"]
        unknown = by_id[f"{family}-unknown-bits"]["parameters"]
        observation = by_id[f"{family}-observation"]["parameters"]
        rounds = by_id[f"{family}-rounds"]["parameters"]
        known_fields = (("known_masks", "known_values") if family == "speck"
                        else ("known_mask", "known_value"))
        changed = lambda variant: {name for name in set(base) | set(variant)
                                   if variant.get(name) != base.get(name)}

        if not changed(unknown) or not changed(unknown) <= set(known_fields):
            raise ValueError(f"{family}-unknown-bits changes a non-unknown-bit axis")
        if not changed(observation) or not changed(observation) <= {"output_mask", "output_value"}:
            raise ValueError(f"{family}-observation changes a non-observation axis")
        if observation["output_mask"] == base["output_mask"]:
            raise ValueError(f"{family}-observation does not change its mask")
        if not changed(rounds) or not changed(rounds) <= {"rounds", "output_value"}:
            raise ValueError(f"{family}-rounds changes a non-round axis")
        if rounds["rounds"] == base["rounds"]:
            raise ValueError(f"{family}-rounds does not change its axis")


def parse_values(text: str) -> list[int]:
    import re
    return [int(token[2:],16) if token.startswith("#x") else int(token[2:],2)
            for token in re.findall(r"#x[0-9a-fA-F]+|#b[01]+",text)]


def run_solver(info: dict[str, Any], case: dict[str, Any], expected: list[list[int]], timeout: float) -> dict[str, Any]:
    row={**info,"case":case["id"],"decision_variables":case["decision_variables"],"projections":[]}
    if info["status"] != "ready": return row
    body,names=smt(case); started=time.monotonic(); process=None
    try:
        process=subprocess.Popen([info["command"],*info["options"]],stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,bufsize=1)
        assert process.stdin and process.stdout
        process.stdin.write("(set-option :produce-models true)\n(set-logic QF_BV)\n"+"\n".join(body)+"\n");process.stdin.flush()
        selector=selectors.DefaultSelector();selector.register(process.stdout,selectors.EVENT_READ)
        def response() -> str:
            remaining=timeout-(time.monotonic()-started)
            if remaining <= 0 or not selector.select(remaining): raise subprocess.TimeoutExpired(info["command"],timeout)
            text=process.stdout.readline().strip(); balance=text.count("(")-text.count(")")
            while balance>0:
                more=process.stdout.readline().strip();text+=" "+more;balance+=more.count("(")-more.count(")")
            return text
        projections=[]
        while True:
            process.stdin.write("(check-sat)\n");process.stdin.flush();status=response()
            if status=="unsat":break
            if status!="sat":raise RuntimeError(f"unexpected response {status!r}")
            process.stdin.write("(get-value ("+" ".join(name for name,_ in names)+"))\n");process.stdin.flush()
            values=parse_values(response())
            if len(values)!=len(names):raise RuntimeError("could not parse model")
            projections.append(values)
            process.stdin.write("(assert (not (and "+" ".join(f"(= {name} {bv(value,width)})" for (name,width),value in zip(names,values))+")))\n");process.stdin.flush()
        row.update(status="ok",semantic_status="sat" if projections else "unsat",
                   solutions=len(projections),projections=sorted(projections))
        if sorted(projections)!=expected:row.update(status="wrong-result",error="projection mismatch")
    except subprocess.TimeoutExpired:row.update(status="timeout",error=f"exceeded {timeout:g}s")
    except (OSError,RuntimeError) as error:row.update(status="error",error=str(error))
    finally:
        if process:process.kill();process.wait()
    row["elapsed_seconds"]=time.monotonic()-started
    return row


def run_gecode(binary: Path | None, case: dict[str, Any], expected: list[list[int]], timeout: float) -> dict[str, Any]:
    if binary is None:return {"case":case["id"],"status":"missing"}
    try:
        done=subprocess.run([str(binary),"--case",case["id"]],capture_output=True,text=True,timeout=timeout)
        value=json.loads(done.stdout) if done.returncode==0 else {}
        ok=(value.get("decision_variables")==case["decision_variables"] and
            sorted(value.get("projections",[]))==expected)
        return {"case":case["id"],"status":"ok" if ok else "wrong-result",**value}
    except subprocess.TimeoutExpired:return {"case":case["id"],"status":"timeout"}
    except (OSError,json.JSONDecodeError) as error:return {"case":case["id"],"status":"error","error":str(error)}


def main() -> int:
    parser=argparse.ArgumentParser(); parser.add_argument("--cases",type=Path,default=Path(__file__).with_name("word-bit-network-cases.json"))
    parser.add_argument("--binary",type=Path);parser.add_argument("--z3",default=shutil.which("z3"));parser.add_argument("--bitwuzla",default=shutil.which("bitwuzla"))
    parser.add_argument("--timeout",type=float,default=10);parser.add_argument("--output",type=Path);args=parser.parse_args()
    if not 0<args.timeout<=10:parser.error("--timeout must be in (0,10]")
    cases=json.loads(args.cases.read_text())["cases"]
    validate_scale_axes(cases)
    if speck32_64([0x0100,0x0908,0x1110,0x1918],22)!=0xA86842F2:raise RuntimeError("Speck32/64 official vector failed")
    infos=[solver_info("z3",args.z3),solver_info("bitwuzla",args.bitwuzla)]
    independent={};solver_rows=[];native=[];linear={}
    for case in cases:
        rows=evaluate(case);p=case["parameters"]
        unique_before_exclusion=None
        if p.get("exclude_projection") is not None:
            unblocked=copy.deepcopy(case);unblocked["parameters"]["exclude_projection"]=None
            unblocked_rows=evaluate(unblocked)
            unique_before_exclusion=(len(unblocked_rows)==1 and
                                     unblocked_rows[0]==p["exclude_projection"])
            if not unique_before_exclusion:
                raise ValueError(f"{case['id']}: excluded projection was not independently unique")
        status="sat" if rows else "unsat"
        if status!=case["expected_status"] or (case["goal"]=="unique" and len(rows)!=1) or (case["goal"]=="unsat")!=(not rows):raise ValueError(f"{case['id']}: evaluator contradicts goal")
        unknown=(sum(16-mask.bit_count() for mask in p["known_masks"]) if case["family"]=="speck32_64" else (p.get("width",32)-p["known_mask"].bit_count()))
        independent[case["id"]]={"semantic_status":status,"solutions":len(rows),"projections":rows,
          "unique_before_exclusion":unique_before_exclusion,
          "scale":{"unknown_bits":unknown,"observation_bits":p["output_mask"].bit_count(),"rounds":p["rounds"]}}
        solver_rows += [run_solver(info,case,rows,args.timeout) for info in infos]
        native_row=run_gecode(args.binary,case,rows,args.timeout);native.append(native_row)
        linear[case["id"]]=gf2(case)
        if linear[case["id"]] and native_row.get("status")=="ok":
            root_fixed=native_row["root"]["fixed_public_bits"][0]
            linear[case["id"]]["gecode_root_fixed_public_bits"]=root_fixed
            known_fixed=p.get("known_mask",0).bit_count()
            linear[case["id"]]["correlation_loss"]=(linear[case["id"]]["rank"]>0 and root_fixed==known_fixed)
    failures=[row for row in solver_rows if row.get("status") in ("wrong-result","error","timeout")]
    failures += [row for row in native if row.get("status") not in ("ok","missing")]
    artifact={"schema_version":1,"comparison":"exact public decision projections","cases":cases,
      "recurrences":{"crc16":"init=0x1d0f, poly=0x1021, MSB feedback","xorshift32":"x^=x<<13; x^=x>>17; x^=x<<5","speck32_64":"16-bit words, alpha=7, beta=2; vector key=[0100,0908,1110,1918], plaintext=6574:694c, ciphertext=a868:42f2"},
      "solver_inventory":infos,"independent":independent,"gf2":linear,"solver_results":solver_rows,"gecode_results":native,
      "validation":{"status":"fail" if failures else "pass","scale_axes":"pass","failures":failures}}
    rendered=json.dumps(artifact,indent=2,sort_keys=True)+"\n"
    if args.output:args.output.write_text(rendered)
    print(rendered,end="");return 1 if failures else 0


if __name__ == "__main__": raise SystemExit(main())
