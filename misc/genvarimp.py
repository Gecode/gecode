#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///

"""Generate kernel var-type.hpp or var-imp.hpp from variable implementation specs."""

import re
import sys
from typing import Dict, List, Optional


HEADER_TEXT = """/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the following files instead:
"""

COPYRIGHT_TEXT = """ *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Kris Coester <kris.coester@sap.com>
 *     Alexander Shepil <alexander.shepil@sap.com>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *     Kris Coester, 2024
 *     Alexander Shepil, 2024
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

"""


def parse_args(argv: List[str]):
    i = 0
    gen_header = False
    gen_typehpp = False
    while i < len(argv) and re.match(r"^-", argv[i]):
        arg = argv[i]
        i += 1
        if arg == "-header":
            gen_header = True
        elif arg == "-typehpp":
            gen_typehpp = True
    files = argv[i:]
    return gen_header, gen_typehpp, files


def parse_vis_file(path: str) -> Dict:
    try:
        with open(path, "r", encoding="utf-8") as fh:
            lines = fh.readlines()
    except OSError:
        raise SystemExit(f"Could not open {path}")

    data: Dict = {
        "name": "",
        "vti": "",
        "ifdef": "",
        "endif": "",
        "dispose": 0,
        "namespace": "",
        "free_bits": 0,
        "mehdr": "",
        "meftr": "",
        "pchdr": "",
        "pcftr": "",
        "men": [],
        "meh": [],
        "me_subscribe": "",
        "mespecial": {},
        "mec": {},
        "me_assigned": "",
        "me_failed": "",
        "pcn": [],
        "pch": [],
        "pcspecial": {},
        "mepc": {},
        "pc_assigned": "",
        "pc_none": "",
    }

    idx = 0
    current: Optional[str] = None

    while True:
        if current is None:
            if idx >= len(lines):
                break
            current = lines[idx]
            idx += 1

        l = current
        current = None

        if re.match(r"^\#", l):
            continue
        if re.match(r"^\[End\]", l, re.I):
            break

        if re.match(r"^\[General\]", l, re.I):
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                m = re.match(r"^Name:\s*(\w+)", l, re.I)
                if m:
                    data["name"] = m.group(1)
                    data["vti"] = m.group(1).upper()
                    continue
                m = re.match(r"^Ifdef:\s*(\w+)", l, re.I)
                if m:
                    data["ifdef"] = f"#ifdef {m.group(1)}\n"
                    data["endif"] = "#endif\n"
                    continue
                if re.match(r"^Dispose:\s*true", l, re.I):
                    data["dispose"] = 1
                    continue
                m = re.match(r"^Namespace:\s*([^ \t\r\n]+)", l, re.I)
                if m:
                    data["namespace"] = m.group(1)
                    continue
                m = re.match(r"^Bits:\s*([^ \t\r\n]+)", l, re.I)
                if m:
                    data["free_bits"] = int(m.group(1))
                    continue
            continue

        if re.match(r"^\[ModEventHeader\]", l, re.I):
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                data["mehdr"] += l
            continue

        if re.match(r"^\[ModEvent\]", l, re.I):
            n = ""
            h = ""
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                m = re.match(r"^Name:\s*(\w+)\s*=\s*(\w+)", l, re.I)
                if m:
                    lhs, rhs = m.group(1), m.group(2)
                    if rhs not in ("FAILED", "NONE", "ASSIGNED", "SUBSCRIBE"):
                        raise SystemExit(f"Unknown special modification event: {rhs}")
                    if rhs == "SUBSCRIBE":
                        data["me_subscribe"] = f"ME_{data['vti']}_{lhs}"
                    else:
                        data["mespecial"][lhs] = rhs
                        if rhs == "ASSIGNED":
                            data["me_assigned"] = f"ME_{data['vti']}_{lhs}"
                        elif rhs == "FAILED":
                            data["me_failed"] = f"ME_{data['vti']}_{lhs}"
                    n = lhs
                    continue
                m = re.match(r"^Name:\s*(\w+)", l, re.I)
                if m:
                    n = m.group(1)
                    continue
                m = re.match(r"^Combine:\s*(.+)", l, re.I)
                if m:
                    combines = m.group(1)
                    for mm in re.finditer(r"(\w+)\s*=(\w+)", combines):
                        a, b = mm.group(1), mm.group(2)
                        data["mec"].setdefault(n, {})[a] = b
                        data["mec"].setdefault(a, {})[n] = b
                    continue
                h += l

            data["men"].append(n)
            data["meh"].append(h)
            data["mec"].setdefault("NONE", {})[n] = n
            data["mec"].setdefault(n, {})["NONE"] = n
            continue

        if re.match(r"^\[ModEventFooter\]", l, re.I):
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                data["meftr"] += l
            continue

        if re.match(r"^\[PropCondHeader\]", l, re.I):
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                data["pchdr"] += l
            continue

        if re.match(r"^\[PropCond\]", l, re.I):
            n = ""
            h = ""
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                m = re.match(r"^Name:\s*(\w+)\s*=\s*(\w+)", l, re.I)
                if m:
                    lhs, rhs = m.group(1), m.group(2)
                    if rhs not in ("ASSIGNED", "NONE"):
                        raise SystemExit(f"Unknown special propagation condition: {rhs}")
                    data["pcspecial"][lhs] = rhs
                    if rhs == "ASSIGNED":
                        data["pc_assigned"] = f"PC_{data['vti']}_{lhs}"
                    if rhs == "NONE":
                        data["pc_none"] = f"PC_{data['vti']}_{lhs}"
                    n = lhs
                    continue
                m = re.match(r"^Name:\s*(\w+)", l, re.I)
                if m:
                    n = m.group(1)
                    continue
                m = re.match(r"^ScheduledBy:\s*(.+)", l, re.I)
                if m:
                    events = m.group(1)
                    for mm in re.finditer(r"(\w+)", events):
                        ev = mm.group(1)
                        data["mepc"].setdefault(ev, {})[n] = 1
                    continue
                h += l

            data["pcn"].append(n)
            data["pch"].append(h)
            continue

        if re.match(r"^\[PropCondFooter\]", l, re.I):
            while idx < len(lines):
                l = lines[idx]
                idx += 1
                if re.match(r"^\[", l):
                    current = l
                    break
                if re.match(r"^\#", l):
                    continue
                data["pcftr"] += l
            continue

    data["maxpc"] = f"PC_{data['vti']}_{data['pcn'][-1]}"
    data["class"] = f"{data['name']}VarImpBase"
    data["conf"] = f"{data['name']}VarImpConf"
    data["base"] = f"Gecode::VarImp<{data['namespace']}::{data['conf']}>"

    hdr = ""
    ftr = ""
    for ns in data["namespace"].split("::"):
        hdr = f"{hdr}namespace {ns} {{ "
        ftr = f"{ftr}}}"
    hdr = re.sub(r" $", "", hdr)
    data["hdr"] = hdr + "\n"
    data["ftr"] = ftr + "\n"

    me_n = len(data["men"])
    if me_n == 3:
        data["me_subscribe"] = data["me_assigned"]

    if not data["me_subscribe"]:
        raise SystemExit("Missing special event specification = SUBSCRIBE")

    val2me: List[str] = ["", ""]
    o = 2
    for n in data["men"]:
        spec = data["mespecial"].get(n, "")
        if spec == "NONE":
            if len(val2me) < 1:
                val2me.extend([""] * (1 - len(val2me)))
            val2me[0] = n
        elif spec == "ASSIGNED":
            if len(val2me) < 2:
                val2me.extend([""] * (2 - len(val2me)))
            val2me[1] = n
        elif spec == "":
            if len(val2me) <= o:
                val2me.extend([""] * (o + 1 - len(val2me)))
            val2me[o] = n
            o += 1

    data["val2me"] = val2me[:o]
    data["me_max"] = f"ME_{data['vti']}_{data['val2me'][o-1]}+1"
    data["me_max_n"] = o

    me_a: Dict[str, str] = {}
    length = 0
    for n in data["val2me"]:
        me_a[n] = f"ME_{data['vti']}_{n}"
        if len(me_a[n]) > length:
            length = len(me_a[n])
    for n in data["val2me"]:
        me_a[n] = me_a[n] + (" " * (length - len(me_a[n])))
    data["me_a"] = me_a

    b = 1
    while (1 << b) < data["me_max_n"]:
        b += 1
    data["bits"] = b

    return data


def generate_typehpp(files: List[Dict], out: List[str]) -> None:
    n_files = len(files)

    for f in range(n_files):
        d = files[f]
        out.append(d["ifdef"])
        out.append(d["hdr"])
        out.append(d["mehdr"])
        o = 1
        for i, n in enumerate(d["men"]):
            out.append(d["meh"][i])
            out.append(f"  const Gecode::ModEvent ME_{d['vti']}_{n} = ")
            if d["mespecial"].get(n):
                out.append(f"Gecode::ME_GEN_{d['mespecial'][n]}")
            else:
                out.append(f"Gecode::ME_GEN_ASSIGNED + {o}")
                o += 1
            out.append(";\n")

        out.append(d["meftr"])
        out.append(d["pchdr"])

        o = 1
        for i, n in enumerate(d["pcn"]):
            out.append(d["pch"][i])
            out.append(f"  const Gecode::PropCond PC_{d['vti']}_{n} = ")
            if d["pcspecial"].get(n):
                out.append(f"Gecode::PC_GEN_{d['pcspecial'][n]}")
            else:
                out.append(f"Gecode::PC_GEN_ASSIGNED + {o}")
                o += 1
            out.append(";\n")
        out.append(d["pcftr"])
        out.append(d["ftr"])
        out.append(d["endif"])

    for f in range(n_files):
        d = files[f]
        out.append(d["ifdef"])
        if d["dispose"]:
            out.append("\n#ifndef GECODE_HAS_VAR_DISPOSE\n")
            out.append("#define GECODE_HAS_VAR_DISPOSE 1\n")
            out.append("#endif\n\n")
        out.append(d["hdr"])
        out.append(f"  /// Configuration for {d['name']}-variable implementations\n")
        out.append(f"  class {d['conf']} {{\n")
        out.append("  public:\n")
        out.append("    /// Index for cloning\n")
        out.append("    static const int idx_c = ")
        if f == 0:
            out.append("0;\n")
        else:
            prev = files[f - 1]
            out.append(f"{prev['namespace']}::{prev['conf']}::idx_c+1;\n")
        out.append("    /// Index for disposal\n")
        out.append("    static const int idx_d = ")
        if d["dispose"]:
            if f == 0:
                out.append("0;\n")
            else:
                prev = files[f - 1]
                out.append(f"{prev['namespace']}::{prev['conf']}::idx_d+1;\n")
        else:
            if f == 0:
                out.append("-1;\n")
            else:
                prev = files[f - 1]
                out.append(f"{prev['namespace']}::{prev['conf']}::idx_d;\n")
        out.append("    /// Maximal propagation condition\n")
        out.append(f"    static const Gecode::PropCond pc_max = {d['maxpc']};\n")
        out.append("    /// Freely available bits\n")
        out.append(f"    static const int free_bits = {d['free_bits']};\n")
        out.append("    /// Start of bits for modification event delta\n")
        out.append("    static const int med_fst = ")
        if f == 0:
            out.append("0;\n")
        else:
            prev = files[f - 1]
            out.append(f"{prev['namespace']}::{prev['conf']}::med_lst;\n")
        out.append("    /// End of bits for modification event delta\n")
        out.append(f"    static const int med_lst = med_fst + {d['bits']};\n")
        out.append("    /// Bitmask for modification event delta\n")
        out.append(f"    static const int med_mask = ((1 << {d['bits']}) - 1) << med_fst;\n")
        out.append("    /// Combine modification events \\a me1 and \\a me2\n")
        out.append("    static Gecode::ModEvent me_combine(Gecode::ModEvent me1, Gecode::ModEvent me2);\n")
        out.append("    /// Update modification even delta \\a med by \\a me, return true on change\n")
        out.append("    static bool med_update(Gecode::ModEventDelta& med, Gecode::ModEvent me);\n")
        out.append("  };\n")
        out.append(d["ftr"])

        if d["ifdef"] != "":
            out.append("#else\n")
            out.append(d["hdr"])
            out.append(f"  /// Dummy configuration for {d['name']}-variable implementations\n")
            out.append(f"  class {d['conf']} {{\n")
            out.append("  public:\n")
            out.append("    /// Index for cloning\n")
            out.append("    static const int idx_c = ")
            if f == 0:
                out.append("-1;\n")
            else:
                prev = files[f - 1]
                out.append(f"{prev['namespace']}::{prev['conf']}::idx_c;\n")
            out.append("    /// Index for disposal\n")
            out.append("    static const int idx_d = ")
            if f == 0:
                out.append("-1;\n")
            else:
                prev = files[f - 1]
                out.append(f"{prev['namespace']}::{prev['conf']}::idx_d;\n")
            out.append("    /// End of bits for modification event delta\n")
            out.append("    static const int med_lst = ")
            if f == 0:
                out.append("0;\n")
            else:
                prev = files[f - 1]
                out.append(f"{prev['namespace']}::{prev['conf']}::med_lst;\n")
            out.append("  };\n")
            out.append(d["ftr"])

        out.append(d["endif"])

    out.append("\n")
    out.append("namespace Gecode {\n\n")
    out.append("  /// Configuration for all variable implementations\n")
    out.append("  class AllVarConf {\n")
    out.append("  public:\n")
    out.append("    /// Index for cloning\n")
    last = files[n_files - 1]
    out.append(f"    static const int idx_c = {last['namespace']}::{last['conf']}::idx_c+1;\n")
    out.append("    /// Index for dispose\n")
    out.append(f"    static const int idx_d = {last['namespace']}::{last['conf']}::idx_d+1;\n")
    out.append("    /// Combine modification event delta \\a med1 with \\a med2\n")
    out.append("    static ModEventDelta med_combine(ModEventDelta med1, ModEventDelta med2);\n")
    out.append("  };\n\n}\n\n")

    for f in range(n_files):
        d = files[f]
        out.append(d["ifdef"])
        out.append(d["hdr"])
        out.append("  forceinline Gecode::ModEvent\n")
        out.append(f"  {d['conf']}::me_combine(Gecode::ModEvent me1, Gecode::ModEvent me2) {{\n")

        if d["me_max_n"] == 2:
            out.append("    return me1 | me2;\n")
        elif d["me_max_n"] <= 4:
            out.append("    static const Gecode::ModEvent me_c = (\n")
            for i, n1 in enumerate(d["val2me"]):
                out.append("      (\n")
                for j, n2 in enumerate(d["val2me"]):
                    n3 = d["mec"][n1][n2]
                    shift = (i << 3) + (j << 1)
                    shift_s = f" {shift}" if shift < 10 else f"{shift}"
                    out.append(f"        ({d['me_a'][n3]} << {shift_s})")
                    if j + 1 == d["me_max_n"]:
                        out.append("   ")
                    else:
                        out.append(" | ")
                    out.append(f" // [{d['me_a'][n1]}][{d['me_a'][n2]}]\n")
                if i + 1 == d["me_max_n"]:
                    out.append("      )\n")
                else:
                    out.append("      ) |\n")
            out.append("    );\n")
            out.append("    return ((me_c >> (me2 << 3)) >> (me1 << 1)) & 3;\n")
        else:
            out.append(f"    static const Gecode::ModEvent me_c[{d['me_max']}][{d['me_max']}] = {{\n")
            for i, n1 in enumerate(d["val2me"]):
                out.append("      {\n")
                for j, n2 in enumerate(d["val2me"]):
                    n3 = d["mec"][n1][n2]
                    out.append(f"        {d['me_a'][n3]}")
                    if j + 1 == d["me_max_n"]:
                        out.append(" ")
                    else:
                        out.append(",")
                    out.append(f" // [{d['me_a'][n1]}][{d['me_a'][n2]}]\n")
                out.append("      }")
                if i + 1 == d["me_max_n"]:
                    out.append("\n")
                else:
                    out.append(",\n")
            out.append("    };\n")
            out.append("    return me_c[me1][me2];\n")
        out.append("  }\n")
        out.append("  forceinline bool\n")
        out.append(f"  {d['conf']}::med_update(Gecode::ModEventDelta& med, Gecode::ModEvent me) {{\n")

        out.append("    switch (me) {\n")
        if d["me_max_n"] == 2:
            me_none = f"ME_{d['vti']}_NONE"
            if d["val2me"][0] == "NONE":
                me_assigned = f"ME_{d['vti']}_{d['val2me'][1]}"
            else:
                me_assigned = f"ME_{d['vti']}_{d['val2me'][0]}"
            out.append(f"    case {me_none}:\n")
            out.append("      return false;\n")
            out.append(f"    case {me_assigned}:\n")
            out.append(f"      if ((med & ({me_assigned} << med_fst)) != 0)\n")
            out.append("        return false;\n")
            out.append(f"      med |= {me_assigned} << med_fst;\n")
            out.append("      break;\n")
        else:
            lvti = d["vti"].lower()
            for n1 in d["val2me"]:
                weak = 1
                for n2 in d["val2me"]:
                    if n2 != "NONE" and n2 != n1 and d["mec"][n1][n2] != n2:
                        weak = 0
                out.append(f"    case ME_{d['vti']}_{n1}:\n")
                if n1 == "NONE":
                    out.append("      return false;\n")
                elif d["mespecial"].get(n1) == "ASSIGNED":
                    out.append("      {\n")
                    out.append(f"        Gecode::ModEventDelta med_{lvti} = med & med_mask;\n")
                    out.append(f"        if (med_{lvti} == (ME_{d['vti']}_{n1} << med_fst))\n")
                    out.append("          return false;\n")
                    out.append(f"        med ^= med_{lvti};\n")
                    out.append(f"        med ^= ME_{d['vti']}_{n1} << med_fst;\n")
                    out.append("        break;\n")
                    out.append("      }\n")
                elif weak:
                    out.append("      {\n")
                    out.append(f"        Gecode::ModEventDelta med_{lvti} = med & med_mask;\n")
                    out.append(f"        if (med_{lvti} != 0)\n")
                    out.append("          return false;\n")
                    out.append(f"        med |= ME_{d['vti']}_{n1} << med_fst;\n")
                    out.append("        break;\n")
                    out.append("      }\n")
                else:
                    out.append("      {\n")
                    if d["me_max_n"] <= 8:
                        out.append("        static const Gecode::ModEvent me_c = (\n")
                        for j, n2 in enumerate(d["val2me"]):
                            n3 = d["mec"][n1][n2]
                            shift = j << 2
                            shift_s = f" {shift}" if shift < 10 else f"{shift}"
                            out.append(f"          (({d['me_a'][n2]} ^ {d['me_a'][n3]}) << {shift_s})")
                            if j + 1 != d["me_max_n"]:
                                out.append(" |\n")
                        out.append("\n        );\n")
                        out.append("        Gecode::ModEvent me_o = (med & med_mask) >> med_fst;\n")
                        out.append("        Gecode::ModEvent me_n = (me_c >> (me_o << 2)) & (med_mask >> med_fst);\n")
                        out.append("        if (me_n == 0)\n")
                        out.append("          return false;\n")
                        out.append("        med ^= me_n << med_fst;\n")
                        out.append("        break;\n")
                    else:
                        out.append(f"        static const Gecode::ModEventDelta me_c[{d['me_max']}] = {{\n")
                        for j, n2 in enumerate(d["val2me"]):
                            n3 = d["mec"][n1][n2]
                            out.append(f"          ({d['me_a'][n2]} ^ {d['me_a'][n3]}) << med_fst")
                            if j + 1 != d["me_max_n"]:
                                out.append(",\n")
                        out.append("\n        };\n")
                        out.append("        Gecode::ModEvent me_o = (med & med_mask) >> med_fst;\n")
                        out.append("        Gecode::ModEventDelta med_n = me_c[me_o];\n")
                        out.append("        if (med_n == 0)\n")
                        out.append("          return false;\n")
                        out.append("        med ^= med_n;\n")
                        out.append("        break;\n")
                    out.append("      }\n")

        out.append("    default: GECODE_NEVER;\n")
        out.append("    }\n")
        out.append("    return true;\n")
        out.append("  }\n\n")
        out.append(d["ftr"])
        out.append(d["endif"])

    out.append("namespace Gecode {\n")
    out.append("  forceinline ModEventDelta\n")
    out.append("  AllVarConf::med_combine(ModEventDelta med1, ModEventDelta med2) {\n")
    for d in files:
        vic = f"{d['namespace']}::{d['conf']}"
        out.append(d["ifdef"])
        out.append(f"    (void) {vic}::med_update(med1,(med2 & {vic}::med_mask) >> {vic}::med_fst);\n")
        out.append(d["endif"])
    out.append("    return med1;\n")
    out.append("  }\n}\n\n")


def generate_header(files: List[Dict], out: List[str]) -> None:
    n_files = len(files)

    for d in files:
        out.append(d["ifdef"])
        out.append(d["hdr"])
        out.append(f"  /// Base-class for {d['name']}-variable implementations\n")
        out.append(f"  class {d['class']} : public {d['base']} {{\n")
        if d["dispose"]:
            out.append(
                f"""  private:
    /// Link to next variable, used for disposal
    {d['class']}* _next_d;
"""
            )

        out.append(
            f"""  protected:
    /// Constructor for cloning \\a x
    {d['class']}(Gecode::Space& home, {d['class']}& x);
  public:
    /// Constructor for creating static instance of variable
    {d['class']}(void);
    /// Constructor for creating variable
    {d['class']}(Gecode::Space& home);
    /// \\name Dependencies
    //@{{
    /** \\brief Subscribe propagator \\a p with propagation condition \\a pc
     *
     * In case \\a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     *
     * In case the variable is assigned (that is, \\a assigned is
     * true), the subscribing propagator is scheduled for execution.
     * Otherwise, the propagator subscribes and is scheduled for execution
     * with modification event \\a me provided that \\a pc is different
     * from \\a {d['pc_assigned']}.
     */
    void subscribe(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned, bool schedule);
    /// Subscribe advisor \\a a if \\a assigned is false.
    void subscribe(Gecode::Space& home, Gecode::Advisor& a, bool assigned, bool failed);
    /// Notify that variable implementation has been modified with modification event \\a me and delta information \\a d
    Gecode::ModEvent notify(Gecode::Space& home, Gecode::ModEvent me, Gecode::Delta& d);
    /// \\brief Schedule propagator \\a p
    static void schedule(Gecode::Space& home, Gecode::Propagator& p, Gecode::ModEvent me);
    /** \\brief Re-schedule propagator \\a p
     *
     * In case the variable is assigned (that is, \\a assigned is
     * true), the propagator is scheduled for execution.
     * Otherwise, the propagator is scheduled for execution
     * with modification event \\a me provided that \\a pc is different
     * from \\a {d['pc_assigned']}.
     */
    void reschedule(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned);
    //@}}
"""
        )

        if d["dispose"]:
            out.append(
                f"""    /// Return link to next variable to be disposed
    {d['class']}* next_d(void) const;
"""
            )

        out.append("  };\n")
        out.append(d["ftr"])
        out.append(d["endif"])

    for d in files:
        out.append(d["ifdef"])
        out.append(d["hdr"])

        if d["dispose"]:
            out.append(
                f"""
  forceinline
  {d['class']}::{d['class']}(void) {{}}

  forceinline
  {d['class']}::{d['class']}(Gecode::Space& home)
    : {d['base']}(home) {{
     _next_d = static_cast<{d['class']}*>(vars_d(home)); vars_d(home,this);
  }}

  forceinline
  {d['class']}::{d['class']}(Gecode::Space& home, {d['class']}& x)
    : {d['base']}(home,x) {{
     _next_d = static_cast<{d['class']}*>(vars_d(home)); vars_d(home,this);
  }}

  forceinline {d['class']}*
  {d['class']}::next_d(void) const {{
    return _next_d;
  }}

"""
            )
        else:
            out.append(
                f"""
  forceinline
  {d['class']}::{d['class']}(void) {{}}

  forceinline
  {d['class']}::{d['class']}(Gecode::Space& home)
    : {d['base']}(home) {{}}

  forceinline
  {d['class']}::{d['class']}(Gecode::Space& home, {d['class']}& x)
    : {d['base']}(home,x) {{}}
"""
            )

        out.append(
            f"""
  forceinline void
  {d['class']}::subscribe(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned, bool schedule) {{
    {d['base']}::subscribe(home,p,pc,assigned,{d['me_subscribe']},schedule);
  }}
  forceinline void
  {d['class']}::subscribe(Gecode::Space& home, Gecode::Advisor& a, bool assigned, bool failed) {{
    {d['base']}::subscribe(home,a,assigned,failed);
  }}

  forceinline void
  {d['class']}::schedule(Gecode::Space& home, Gecode::Propagator& p, Gecode::ModEvent me) {{
    {d['base']}::schedule(home,p,me);
  }}
  forceinline void
  {d['class']}::reschedule(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned) {{
    {d['base']}::reschedule(home,p,pc,assigned,{d['me_subscribe']});
  }}

"""
        )

        if d["me_max_n"] == 2:
            out.append(
                f"""  forceinline Gecode::ModEvent
  {d['class']}::notify(Gecode::Space& home, Gecode::ModEvent, Gecode::Delta& d) {{
    {d['base']}::schedule(home,{d['pc_assigned']},{d['pc_assigned']},{d['me_assigned']});
    if (!{d['base']}::advise(home,{d['me_assigned']},d))
      return {d['me_failed']};
    cancel(home);
    return {d['me_assigned']};
  }}

"""
            )
        else:
            out.append(
                f"""  forceinline Gecode::ModEvent
  {d['class']}::notify(Gecode::Space& home, Gecode::ModEvent me, Gecode::Delta& d) {{
    switch (me) {{
"""
            )

            val2pc: List[str] = []
            for n in d["pcn"]:
                if d["pcspecial"].get(n) == "ASSIGNED":
                    if len(val2pc) < 1:
                        val2pc.extend([""])
                    val2pc[0] = n
            o = 1
            for n in d["pcn"]:
                sp = d["pcspecial"].get(n)
                if sp != "ASSIGNED" and sp != "NONE":
                    if len(val2pc) <= o:
                        val2pc.extend([""] * (o + 1 - len(val2pc)))
                    val2pc[o] = n
                    o += 1

            for n in d["men"]:
                sp = d["mespecial"].get(n)
                if sp != "NONE" and sp != "FAILED":
                    out.append(f"    case ME_{d['vti']}_{n}:\n")
                    out.append("      // Conditions: ")
                    fst = 1
                    for pcn in d["pcn"]:
                        if d["mepc"].get(n, {}).get(pcn):
                            if fst:
                                fst = 0
                            else:
                                out.append(", ")
                            out.append(pcn)
                    out.append("\n")

                    j = 0
                    while j < len(d["pcn"]):
                        n2 = val2pc[j] if j < len(val2pc) else None
                        if n2 is not None and d["mepc"].get(n, {}).get(n2):
                            out.append(f"      {d['base']}::schedule(home,PC_{d['vti']}_{n2},")
                            while True:
                                n3 = val2pc[j + 1] if (j + 1) < len(val2pc) else None
                                if n3 is not None and d["mepc"].get(n, {}).get(n3):
                                    j += 1
                                else:
                                    break
                            n_last = val2pc[j] if j < len(val2pc) else ""
                            out.append(f"PC_{d['vti']}_{n_last},ME_{d['vti']}_{n});\n")
                        j += 1

                    out.append(f"      if (!{d['base']}::advise(home,ME_{d['vti']}_{n},d))\n")
                    out.append(f"        return {d['me_failed']};\n")
                    if d["mespecial"].get(n) == "ASSIGNED":
                        out.append("      cancel(home);\n")
                    out.append("      break;\n")

            out.append(
                """    default: GECODE_NEVER;
    }
    return me;
  }

"""
            )

        out.append(d["ftr"])
        out.append(d["endif"])

    out.append(
        """namespace Gecode {

  forceinline void
  Space::recover_noidx(void) {
    VarImp<NoIdxVarImpConf>* x =
      static_cast<VarImp<NoIdxVarImpConf>*>(pc.c.vars_noidx);
    while (x != nullptr) {
      VarImp<NoIdxVarImpConf>* n = x->next();
      x->b.base = nullptr; x->u.idx[0] = 0;
      if (sizeof(ActorLink**) > sizeof(unsigned int))
        *(1+&x->u.idx[0]) = 0;
      x = n;
    }
  }

  forceinline void
  Space::update(ActorLink** sub) {
"""
    )

    for d in files:
        out.append(d["ifdef"])
        out.append(f"    {d['base']}::update(*this,sub);\n")
        out.append(d["endif"])

    out.append(
        """  }

  forceinline void
  Space::recover(Space& source) {
    ActorLink* clone_lists[2] = {&pl, &bl};
    for (int i=0; i<2; i++) {
      ActorLink* l = clone_lists[i];
      ActorLink* c = l->next();
      while (c != l) {
        ActorLink* n = c->next();
        (void) Actor::cast(c)->dispose(*this);
        c = n;
      }
      l->init();
    }
    while (pc.c.local != nullptr) {
      LocalObject* l = pc.c.local;
      ActorLink* n = l->next();
      pc.c.local = (n == nullptr) ? nullptr : LocalObject::cast(n);
      ActorLink* fwd = l->prev();
      if (fwd != nullptr) {
        (void) Actor::cast(fwd)->dispose(*this);
        l->prev(nullptr);
      }
    }

    ActorLink* lists[2] = {&source.pl, &source.bl};
    for (int i=0; i<2; i++) {
      ActorLink* l = lists[i];
      ActorLink* p_a = l;
      ActorLink* c_a = p_a->next();
      while (c_a != l) {
        if (i == 0) {
          Propagator* p = Propagator::cast(c_a);
          if (p->u.advisors != nullptr) {
            ActorLink* a = p->u.advisors;
            p->u.advisors = nullptr;
            do {
              a->prev(p); a = a->next();
            } while (a != nullptr);
          }
        }
        ActorLink* fwd = c_a->prev();
        if (fwd != p_a) {
          c_a->prev(p_a);
        }
        p_a = c_a; c_a = c_a->next();
      }
    }

    ActorLink** sub = static_cast<ActorLink**>(mm.subscriptions());
    recover_noidx();
"""
    )

    for d in files:
        out.append(d["ifdef"])
        out.append(f"    {d['base']}::recover(*this,sub);\n")
        out.append(d["endif"])

    out.append(
        """  }
}
"""
    )


def main() -> int:
    gen_header, gen_typehpp, files = parse_args(sys.argv[1:])

    out: List[str] = []
    out.append(HEADER_TEXT)
    for f in files:
        out.append(f" *     - {f}\n")
    out.append(COPYRIGHT_TEXT)

    parsed = [parse_vis_file(f) for f in files]

    if gen_typehpp:
        generate_typehpp(parsed, out)

    if gen_header:
        generate_header(parsed, out)

    out.append("// STATISTICS: kernel-var\n")
    sys.stdout.write("".join(out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
