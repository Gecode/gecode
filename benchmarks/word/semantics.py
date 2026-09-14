"""Independent semantic checks shared by the focused Word benchmarks.

This is deliberately a small result contract, not a solver-output protocol.
Solver-specific counters and timings remain runner metadata.
"""

from __future__ import annotations

import itertools
from typing import Any, Callable

GOALS = frozenset(("first", "unsat", "unique", "enumerate"))


def xor_rotate(value: int, *, width: int, rounds: int, key: int,
               shift: int) -> int:
    mask = (1 << width) - 1
    for _ in range(rounds):
        value ^= key
        value = ((value << shift) | (value >> (width-shift))) & mask
    return value


def xor_projections(parameters: dict[str, int]) -> list[list[int]]:
    width = parameters["width"]
    fixed = ~ (parameters["target_lo"] ^ parameters["target_hi"])
    excluded = parameters.get("excluded_input")
    return [[value] for value in range(1 << width)
            if value != excluded and
            (xor_rotate(value, width=width, rounds=parameters["rounds"],
                        key=parameters["key"], shift=parameters["shift"])
             & fixed) == parameters["target_lo"]]


def product_mod(x: int, y: int, modulus: int) -> int:
    """Mathematical product modulo a positive modulus (no word pre-wrap)."""
    if modulus <= 0:
        raise ValueError("product_mod modulus must be positive")
    return (x * y) % modulus


def signed_value(value: int, width: int) -> int:
    """Interpret an unsigned width-bit encoding as two's complement."""
    value &= (1 << width)-1
    return value-(1 << width) if value & (1 << (width-1)) else value


def smt_divrem(x: int, y: int, *, width: int,
               signed: bool = False) -> tuple[int, int]:
    """SMT-LIB bv[u|s]div and bv[u|s]rem, including total edge cases."""
    mask=(1 << width)-1
    x, y = x & mask, y & mask
    if not signed:
        return (mask, x) if y == 0 else (x//y, x%y)
    sx, sy = signed_value(x,width), signed_value(y,width)
    if sy == 0: return (1 if sx < 0 else mask), x
    if sx == -(1 << (width-1)) and sy == -1: return x, 0
    q=abs(sx)//abs(sy)
    if (sx < 0) != (sy < 0): q=-q
    return q & mask, (sx-q*sy) & mask


def alu_trace(value: int, width: int) -> dict[str, int]:
    """Concrete short-ALU semantics; amount is deliberately input & 3."""
    mask=(1 << width)-1
    value &= mask
    amount=value & 3
    s1=(value << amount) & mask
    total=s1+0x1d
    s2=total & mask
    carry=int(total > mask)
    negative=int(bool(s2 & (1 << (width-1))))
    shifted=(signed_value(s2,width) >> amount) & mask
    s3=(s2 ^ (0x15 & mask)) if carry else shifted
    output=((s3+1)&mask) if negative else s3
    return {"input":value,"amount":amount,"s1":s1,"s2":s2,
            "carry":carry,"negative":negative,"s3":s3,"output":output}


def dma_decision_variables(count: int) -> list[str]:
    return (["index"] + [f"base[{i}]" for i in range(count)] +
            [f"flag[{i}]" for i in range(count)])


def _dma_length(index: int) -> int:
    return (0x20, 0x30, 0x20, 0x40, 0x30, 0x20)[index % 6]


def _dma_limit(count: int) -> int:
    return 0x100 + 0x20 + sum(_dma_length(i) for i in range(count))


def dma_projection_valid(parameters: dict[str, int], row: tuple[int, ...]) -> bool:
    count = parameters["descriptor_count"]
    if len(row) != 1 + 2*count:
        return False
    index = row[0]
    bases = row[1:1+count]
    flags = row[1+count:]
    limit = _dma_limit(count)
    if not 0 <= index < count:
        return False
    if any(base % 16 or not 0x100 <= base <= limit-0x20
           for base in bases):
        return False
    ends = tuple(base+_dma_length(i) for i, base in enumerate(bases))
    if any(end > limit for end in ends) or any(
            ends[i] > bases[i+1] for i in range(count-1)):
        return False
    if any(flag not in (1, 3, 5) for flag in flags) or any(
            flags[i] > flags[i+1] for i in range(count-1)):
        return False
    writes = tuple(bool(flag & 2) for flag in flags)
    executes = tuple(bool(flag & 4) for flag in flags)
    if sum(writes) != count//3 or sum(executes) != count//3:
        return False
    selected_limit = ends[index] + (0x10 if writes[index] else 0)
    return selected_limit <= min(0x300, limit)


def dma_projections(parameters: dict[str, int]) -> list[list[int]]:
    """Enumerate the deliberately small three-descriptor control."""
    count = parameters["descriptor_count"]
    if count != 3:
        raise ValueError("exact DMA enumeration is limited to three descriptors")
    limit = _dma_limit(count)
    bases = range(0x100, limit-0x20+1, 16)
    rows = []
    for base_values in itertools.product(bases, repeat=count):
        for flag_values in itertools.product((1, 3, 5), repeat=count):
            for index in range(count):
                row = (index, *base_values, *flag_values)
                if dma_projection_valid(parameters, row):
                    rows.append(list(row))
    return rows


def product_mod_projections(parameters: dict[str, Any]) -> list[list[int]]:
    width = parameters["width"]
    mask = (1 << width)-1
    modulus = parameters["modulus"]
    rows = []
    for x in range(parameters["x_min"], parameters["x_max"]+1):
        for y in range(parameters["y_min"], parameters["y_max"]+1):
            encoded_x, encoded_y = x & mask, y & mask
            result = product_mod(encoded_x, encoded_y, modulus)
            if parameters["result_min"] <= result <= parameters["result_max"]:
                rows.append([encoded_x, encoded_y, result])
    return rows


def nary_add_projection_valid(parameters: dict[str, int],
                              row: tuple[int, ...]) -> bool:
    count = parameters["segments"]
    return (len(row) == count and
            all(64 <= value <= 256 and value % 16 == 0 for value in row) and
            all(row[i] <= row[i+1] for i in range(count-1)) and
            sum(row) % 4096 == parameters["total"])


def nary_add_projections(parameters: dict[str, int]) -> list[list[int]]:
    count = parameters["segments"]
    values = range(64, 257, 16)
    return [list(row) for row in itertools.combinations_with_replacement(
        values, count) if sum(row) % 4096 == parameters["total"]]


def _family_checks(instance: dict[str, Any]) -> tuple[
        Callable[[tuple[int, ...]], bool], list[list[int]] | None]:
    parameters = instance["parameters"]
    kind = instance.get("kind")
    if kind == "xor-rotate":
        expected = xor_projections(parameters)
        return lambda row: list(row) in expected, expected
    if kind == "dma":
        exact = dma_projections(parameters) \
            if instance["goal"] in ("enumerate", "unsat") else None
        return lambda row: dma_projection_valid(parameters, row), exact
    if kind == "product-mod":
        expected = product_mod_projections(parameters)
        return lambda row: list(row) in expected, expected
    if kind == "nary-add":
        exact = nary_add_projections(parameters) if instance["goal"] == "unsat" else None
        return lambda row: nary_add_projection_valid(parameters, row), exact
    raise ValueError(f"unsupported semantic benchmark kind {kind!r}")


def validate_contract(instance: dict[str, Any], result: dict[str, Any]) -> None:
    goal = instance.get("goal")
    expected_status = instance.get("expected_status")
    names = instance.get("decision_variables")
    parameters = instance.get("parameters")
    if goal not in GOALS or expected_status not in ("sat", "unsat"):
        raise ValueError("fixture lacks a valid declared goal/status")
    if not isinstance(names, list) or not names or len(names) != len(set(names)):
        raise ValueError("fixture decision_variables must be ordered and unique")
    if not isinstance(parameters, dict):
        raise ValueError("fixture parameters must be an object")
    if result.get("semantic_status") != expected_status:
        raise ValueError("solver semantic status differs from fixture")
    if result.get("decision_variables") != names:
        raise ValueError("solver decision-variable projection differs from fixture")
    projections = result.get("projections")
    if not isinstance(projections, list) or any(
            not isinstance(row, list) or len(row) != len(names)
            or any(not isinstance(value, int) or isinstance(value, bool)
                   for value in row) for row in projections):
        raise ValueError("solver returned a malformed public projection")
    canonical = sorted(set(map(tuple, projections)))
    if len(canonical) != len(projections):
        raise ValueError("solver returned duplicate public projections")
    if expected_status == "unsat":
        if projections or result.get("solutions") != 0:
            raise ValueError("UNSAT result carries a witness or solution")
    elif not projections:
        raise ValueError("SAT result carries no public witness")
    witness_valid, exact = _family_checks(instance)
    if any(not witness_valid(row) for row in canonical):
        raise ValueError("solver returned an invalid public witness")
    if goal in ("enumerate", "unsat"):
        if exact is None or canonical != sorted(map(tuple, exact)):
            raise ValueError("projected solution set is incorrect")
        if result.get("solutions") != len(exact):
            raise ValueError("projected solution count is incorrect")
    elif goal in ("first", "unique"):
        if len(canonical) != 1:
            raise ValueError(f"{goal} goal must return one public projection")
        if goal == "unique" and exact is not None:
            expected = sorted(map(tuple, exact))
            if len(expected) != 1 or canonical != expected:
                raise ValueError("unique goal does not have one exact solution")
    if goal == "unsat" and expected_status != "unsat":
        raise ValueError("unsat goal must declare expected_status=unsat")
