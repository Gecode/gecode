#!/usr/bin/env python3
"""Pure CMake -P JSON encoder tests; no compiler, solver, or downloads."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cmake", required=True)
    args = parser.parse_args()
    script = ROOT / "tools/flatzinc/configure-optimize-msc.cmake"
    template = ROOT / "tools/flatzinc/gecode-optimize.msc.in"
    with tempfile.TemporaryDirectory(prefix="gecode-msc-escape-") as tmp:
        output = Path(tmp)/"directory with spaces"/"solver.msc"
        cases = [
            ("6.4.0", "../../../bin/fzn-gecode-optimize", "../gecode-optimize-experimental"),
            ('version"quote', 'C:\\solver path\\a"b;ç/driver.exe', '../library space/雪;z'),
            ("v"+"".join(map(chr,range(1,32))), 'drive\\name\nline\rreturn\ttab', 'lib\bback\fform'),
            ('@GECODE_OPTIMIZE_MSC_EXECUTABLE@', '@GECODE_VERSION@/binary', '@GECODE_OPTIMIZE_MSC_MZNLIB@'),
        ]
        for version,driver,library in cases:
            command = [args.cmake, "-DTEMPLATE="+str(template), "-DVERSION="+version,
                       "-DDRIVER="+driver, "-DMZNLIB="+library, "-DOUTPUT="+str(output), "-P", str(script)]
            subprocess.run(command,check=True,capture_output=True,timeout=10)
            encoded = output.read_bytes()
            value = json.loads(encoded)
            assert value["version"] == version
            assert value["executable"] == [driver,"--minizinc"]
            assert value["mznlib"] == library
            # Generated build/install registrations retain the public controls;
            # configuring paths must not erase their types, defaults or help.
            expected_flags = json.loads(template.read_text())["extraFlags"]
            assert value["extraFlags"] == expected_flags
            assert len({flag[0] for flag in value["extraFlags"]}) == len(expected_flags) == 21
            assert value["stdFlags"] == ["-t"]
            assert not list(output.parent.glob(output.name+".*.tmp"))
        prior = output.read_bytes()
        for omitted in ("VERSION","DRIVER","MZNLIB","OUTPUT","TEMPLATE"):
            inputs = {"VERSION":"1","DRIVER":"driver","MZNLIB":"lib","OUTPUT":str(output),"TEMPLATE":str(template)}
            del inputs[omitted]
            run = subprocess.run([args.cmake,*[f"-D{k}={v}" for k,v in inputs.items()],"-P",str(script)],
                                 capture_output=True,timeout=10)
            assert run.returncode != 0 and output.read_bytes() == prior
        run = subprocess.run([args.cmake,"-DTEMPLATE="+str(Path(tmp)/"absent.in"),"-DVERSION=1",
                              "-DDRIVER=driver","-DMZNLIB=lib","-DOUTPUT="+str(output),"-P",str(script)],
                             capture_output=True,timeout=10)
        assert run.returncode != 0 and output.read_bytes() == prior
    print("MiniZinc JSON encoder: 10 cases passed")


if __name__ == "__main__":
    main()
