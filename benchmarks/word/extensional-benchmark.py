#!/usr/bin/env python3
"""Compare native Word constraints with small add/multiply tables on SMT2."""
import argparse
import itertools
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile


def cases():
    for width in (3, 4, 5, 6):
        mask = (1 << width) - 1
        def lit(n):
            return f'(_ bv{n & mask} {width})'
        declarations = '\n'.join(f'(declare-fun {v} () (_ BitVec {width}))' for v in ('x', 'y'))
        for family in ('inverse', 'square', 'network'):
            # Compute expected decisions independently by exhaustive Python integers.
            if family == 'inverse':
                expression = '(bvmul x y)'
                evaluate = lambda x, y: x * y & mask
            elif family == 'square':
                expression = '(bvadd (bvmul x x) y)'
                evaluate = lambda x, y: (x * x + y) & mask
            else:
                expression = '(bvadd (bvmul x y) (bvxor x y))'
                evaluate = lambda x, y: (x * y + (x ^ y)) & mask
            # Bounds and cube restrictions survive alongside the replaced subset.
            domain = [(x, y) for x in range(mask + 1) for y in range(mask + 1)
                      if x <= mask // 2 and y >= mask // 2 and x & 1 == 0]
            possible = {evaluate(x, y) for x, y in domain}
            targets = [('sat', sorted(possible)[len(possible) // 2])]
            missing = sorted(set(range(mask + 1)) - possible)
            if missing:
                targets.append(('unsat', missing[len(missing) // 2]))
            else:
                # An inconsistent output pair exercises propagation across two tables.
                targets.append(('unsat', targets[0][1]))
            for expected, target in targets:
                extra = ''
                if expected == 'unsat' and not missing:
                    extra = f'(assert (= (bvadd {expression} {lit(1)}) {lit(target)}))'
                source = f'''(set-logic QF_BV)
{declarations}
(assert (bvule x {lit(mask // 2)}))
(assert (bvuge y {lit(mask // 2)}))
(assert (= (bvand x {lit(1)}) {lit(0)}))
(assert (= {expression} {lit(target)}))
{extra}
(check-sat)
'''
                yield f'{family}-{width}-{expected}', source, expected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=Path('build/bin/word-smt2-reader'))
    parser.add_argument('--z3', default='z3')
    parser.add_argument('--repetitions', type=int, default=5)
    parser.add_argument('--timeout', type=float, default=15)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.repetitions < 1 or args.timeout <= 0:
        parser.error('repetitions and timeout must be positive')
    args.output.mkdir(parents=True, exist_ok=True)
    records = []
    with tempfile.TemporaryDirectory(prefix='word-tables-') as directory:
        for name, source, expected in cases():
            path = Path(directory) / (name + '.smt2')
            path.write_text(source)
            (args.output / path.name).write_text(source)
            oracle = subprocess.run([args.z3, str(path)], capture_output=True,
                                    text=True, timeout=args.timeout, check=True)
            if oracle.stdout.strip() != expected:
                raise RuntimeError(f'{name}: oracle mismatch: {oracle.stdout}')
            for repetition in range(args.repetitions):
                pairs = list(itertools.product(('cube', 'unsigned', 'signed'), (False, True)))
                if repetition % 2:
                    pairs.reverse()
                for policy, tables in pairs:
                    command = [str(args.binary.resolve()), policy, str(path)]
                    if tables:
                        command.append('--tables')
                    record = dict(case=name, policy=policy, tables=tables, repetition=repetition)
                    try:
                        result = subprocess.run(command, capture_output=True, text=True,
                                                timeout=args.timeout)
                        record.update(json.loads(result.stdout))
                        if result.returncode or record.get('result') != expected:
                            raise RuntimeError(f'{command}: {result.stdout} {result.stderr}')
                        if tables and not record.get('table_replacements'):
                            raise RuntimeError(f'{name}: no replacements')
                    except subprocess.TimeoutExpired:
                        record.update(status='timeout')
                    records.append(record)
            print(name, flush=True)
    (args.output / 'raw.json').write_text(json.dumps(dict(
        platform=platform.platform(), binary=str(args.binary.resolve()),
        repetitions=args.repetitions, records=records), indent=2) + '\n')
    lines = ['| Case | Domain | Native model / solve µs | Table model / solve µs | Native / table nodes |',
             '| --- | --- | ---: | ---: | ---: |']
    for name, _, _ in cases():
        for policy in ('cube', 'unsigned', 'signed'):
            groups = [[r for r in records if r['case'] == name and r['policy'] == policy
                       and r['tables'] == t] for t in (False, True)]
            if any(r['status'] != 'ok' for group in groups for r in group):
                lines.append(f'| {name} | {policy} | timeout | timeout | — |')
                continue
            med = lambda group, key: round(statistics.median(r[key] for r in group))
            timings = [f"{med(g, 'model_us')} / {med(g, 'solve_us')}" for g in groups]
            nodes = f"{med(groups[0], 'nodes')} / {med(groups[1], 'nodes')}"
            lines.append(f'| {name} | {policy} | {timings[0]} | {timings[1]} | {nodes} |')
    (args.output / 'summary.md').write_text('\n'.join(lines) + '\n')


if __name__ == '__main__':
    main()
