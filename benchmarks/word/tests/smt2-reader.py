#!/usr/bin/env python3
"""Focused reader checks. Usage: python3 smt2-reader.py PATH/word-smt2-reader."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    binary = str(Path(sys.argv[1]).resolve())
    cases = [
        ("metadata-string", '(set-info :source "punctuation (; and ""quotes"")")\n'
         '(assert true)(check-sat)', "sat"),
        ("decimal-wrap", '(declare-const x (_ BitVec 64))'
         '(assert (= x (_ bv18446744073709551617 64)))'
         '(assert (distinct x #x0000000000000001))(check-sat)', "error"),
        ("decimal-narrow", '(assert (= (_ bv257 8) #x01))(check-sat)', "error"),
        ("decimal-boundaries", '(assert (= (_ bv18446744073709551615 64)'
         ' #xffffffffffffffff))(assert (= (_ bv1 1) #b1))(check-sat)', "sat"),
        ("simultaneous-let", '(declare-fun x () Bool)(assert x)'
         '(assert (let ((x false) (y x)) y))(check-sat)', "sat"),
        ("shadow-sort", '(declare-fun x () Bool)(assert x)'
         '(assert (let ((x #b0)) (= x #b1)))(check-sat)', "unsat"),
        ("quoted-literal-name", '(declare-fun |#b0| () (_ BitVec 1))'
         '(assert (= |#b0| #b1))(check-sat)', "sat"),
        ("quoted-empty-name", '(declare-fun || () Bool)(assert ||)'
         '(check-sat)', "sat"),
        ("quoted-zero-not-constant", '(declare-fun |#b0| () (_ BitVec 1))'
         '(assert (= |#b0| #b1))(assert (bvuge #b0 |#b0|))(check-sat)', "unsat"),
        ("dead-binding", '(declare-fun x () (_ BitVec 4))'
         '(declare-fun y () (_ BitVec 4))'
         '(assert (let ((t (bvsdiv x y))) (bvuge t #x0)))(check-sat)', "sat"),
        ("shared-word", '(declare-fun x () (_ BitVec 32))'
         '(assert (let ((t (bvadd x #x00000001))) (distinct t t)))'
         '(check-sat)', "unsat"),
        ("signed-division-zero", '(assert (= (bvsdiv #b1000 #b0000) #b0001))'
         '(assert (= (bvsdiv #b0010 #b0000) #b1111))(check-sat)', "sat"),
        ("signed-overflow", '(assert (= (bvsdiv #b1000 #b1111) #b1000))'
         '(check-sat)', "sat"),
        ("signed-mod", '(assert (= (bvsmod #b1101 #b0010) #b0001))'
         '(assert (= (bvsmod #b0011 #b1110) #b1111))(check-sat)', "sat"),
        ("widths", '(assert (= ((_ zero_extend 60) ((_ extract 3 0) #xff))'
         ' #x000000000000000f))(check-sat)', "sat"),
        ("sign-extend", '(assert (= ((_ sign_extend 63) #b1)'
         ' #xffffffffffffffff))(assert (= ((_ sign_extend 0) #b1) #b1))'
         '(assert (= ((_ sign_extend 3) #b0) #x0))(check-sat)', "sat"),
        ("sign-extend-overflow", '(assert (= ((_ sign_extend 64) #b0) #b0))'
         '(check-sat)', "error"),
        ("invalid-dead-literal", '(assert (bvuge #b2 #b0))(check-sat)', "error"),
        ("invalid-hex", '(assert (= #xgg #x00))(check-sat)', "error"),
        ("signed-index", '(assert (= ((_ zero_extend +0) #b0) #b0))'
         '(check-sat)', "error"),
        ("overflow-index", '(assert (= ((_ extract 4294967296 0) #b0) #b0))'
         '(check-sat)', "error"),
        ("forward-declaration", '(assert p)(declare-fun p () Bool)'
         '(check-sat)', "error"),
        ("unterminated-string", '(set-info :source "missing quote)(check-sat)',
         "error"),
        ("quoted-backslash", '(declare-fun |a\\b| () Bool)(check-sat)', "error"),
        ("early-exit", '(exit)(assert true)(check-sat)', "error"),
        ("duplicate-binding", '(assert (let ((x true) (x false)) x))'
         '(check-sat)', "error"),
        ("bad-width", '(assert (= (_ bv1 65) (_ bv1 65)))(check-sat)', "error"),
    ]
    dag = "b18"
    for i in reversed(range(1, 19)):
        dag = f"(let ((b{i} (and b{i-1} b{i-1}))) {dag})"
    cases.append(("shared-bool", '(declare-fun x () (_ BitVec 4))'
                  f'(assert (let ((b0 (= x #x0))) {dag}))(check-sat)', "sat"))
    cases.append(("bool-associativity", '(assert (=> false true false))'
                  '(assert (xor true true true))(assert (not (xor true false true)))'
                  '(check-sat)', "sat"))
    cases.extend([
        ('definition-scope', '(declare-const x (_ BitVec 4))'
         '(define-fun next () (_ BitVec 4) (bvadd x #x1))'
         '(define-fun ok () Bool (= next #x2))'
         '(assert (= x #x1))(assert (let ((x #xf)) ok))(check-sat)', 'sat'),
        ('definition-sharing', '(declare-const x (_ BitVec 32))'
         '(define-fun next () (_ BitVec 32) (bvadd x #x00000001))'
         '(assert (distinct next next))(check-sat)', 'unsat'),
        ('definition-recursive', '(define-fun x () Bool x)(check-sat)', 'error'),
        ('definition-sort', '(define-fun x () Bool #b0)(check-sat)', 'error'),
        ('definition-parameters', '(define-fun f ((x Bool)) Bool x)(check-sat)', 'error'),
        ('integer-euclidean',
         '(assert (= (div (- 3) 2) (- 2)))'
         '(assert (= (mod (- 3) 2) 1))'
         '(assert (= (div 3 (- 2)) (- 1)))'
         '(assert (= (mod 3 (- 2)) 1))'
         '(assert (= (div (- 3) (- 2)) 2))'
         '(assert (= (mod (- 3) (- 2)) 1))(check-sat)', 'sat'),
        ('integer-operators', '(declare-const x Int)(define-fun y () Int (+ x 3))'
         '(assert (>= x (- 4)))(assert (< x 9))(assert (= y (* x (- 2))))'
         '(assert (distinct x 0))(check-sat)', 'sat'),
        ('integer-sort-mismatch', '(assert (= 1 #b1))(check-sat)', 'error'),
        ('integer-minimum', '(assert (= (- 9223372036854775808)'
         ' (- 9223372036854775808)))(check-sat)', 'sat'),
        ('integer-literal-overflow', '(assert (= 9223372036854775808 0))'
         '(check-sat)', 'error'),
        ('multiple-objectives', '(maximize 1)(minimize 0)(check-sat)', 'error'),
        ('objective-sort', '(maximize #b1)(check-sat)', 'error'),
        ('early-get-objectives', '(get-objectives)(check-sat)', 'error'),
    ])
    for width in (4, 64):
        mask = (1 << width) - 1
        def literal(value):
            return '#b' + format(value & mask, f'0{width}b')
        equations = [
            f'(= (bvnot x) {literal(~9)})',
            f'(= (bvand x y {literal(3)}) {literal(9 & 6 & 3)})',
            f'(= (bvor x y {literal(3)}) {literal(9 | 6 | 3)})',
            f'(= (bvxor x y {literal(3)}) {literal(9 ^ 6 ^ 3)})',
            f'(= (bvudiv x {literal(3)}) {literal(3)})',
            f'(= (bvurem x {literal(4)}) {literal(1)})',
            f'(= (bvudiv x {literal(0)}) {literal(mask)})',
            f'(= (bvurem x {literal(0)}) x)',
            f'(= (concat ((_ extract {width-1} {width//2}) x)'
            f' ((_ extract {width//2-1} 0) x)) x)',
        ]
        negative = (1 << (width-1)) | 1
        signed = negative - (1 << width)
        for amount in (0, 1, width-1, width, width+1, mask):
            equations.extend([
                f'(= (bvshl x {literal(amount)}) {literal(9 << amount if amount < width else 0)})',
                f'(= (bvlshr x {literal(amount)}) {literal(9 >> amount if amount < width else 0)})',
                f'(= (bvashr {literal(negative)} {literal(amount)})'
                f' {literal(signed >> amount if amount < width else -1)})',
            ])
        for amount in (0, width, width+1, 18446744073709551617):
            shift = amount % width
            left = ((9 << shift) | (9 >> (width-shift))) & mask
            right = ((9 >> shift) | (9 << (width-shift))) & mask
            equations.extend([
                f'(= ((_ rotate_left {amount}) x) {literal(left)})',
                f'(= ((_ rotate_right {amount}) x) {literal(right)})',
            ])
        source = f'(declare-const x (_ BitVec {width}))(declare-const y (_ BitVec {width}))'
        source += f'(assert (= x {literal(9)}))(assert (= y {literal(6)}))'
        source += ''.join(f'(assert {equation})' for equation in equations) + '(check-sat)'
        cases.append((f'core-operators-{width}', source, 'sat'))
    cases.extend([
        ('shift-width', '(assert (= (bvshl #x1 #b1) #x2))(check-sat)', 'error'),
        ('concat-width', '(assert (= (concat #xffffffffffffffff #b0) #b0))(check-sat)', 'error'),
        ('bitwise-sort', '(assert (= (bvand #x1 true) #x1))(check-sat)', 'error'),
        ('xor-arity', '(assert (xor true))(check-sat)', 'error'),
    ])
    checks = 0
    with tempfile.TemporaryDirectory(prefix="word-smt2-reader-") as directory:
        for name, source, expected in cases:
            path = Path(directory) / (name + ".smt2")
            path.write_text(source + "\n")
            for variant in ("cube", "unsigned", "signed"):
                for mode in ("--parse-only", "--model-only", None):
                    command = [binary, variant, str(path)] + ([mode] if mode else [])
                    run = subprocess.run(command, capture_output=True, text=True, timeout=10)
                    result = json.loads(run.stdout)
                    context = (name, variant, mode, result, run.stderr)
                    if expected == "error":
                        assert run.returncode == 1 and result["status"] == "error", context
                    else:
                        assert run.returncode == 0 and result["status"] == "ok", context
                        assert result["parse_us"] >= 0, context
                        if mode != "--parse-only":
                            assert result["model_us"] >= 0, context
                        if mode is None:
                            assert result["result"] == expected, context
                            assert result["solve_us"] >= 0, context
                            if name == "dead-binding":
                                assert result["propagations"] == 0, context
                            if name == "shared-word":
                                assert result["nodes"] <= 1, context
                    checks += 1
        objectives = [
            ('maximize-int',
             '(declare-const x Int)(assert (>= x (- 3)))(assert (<= x 5))'
             '(maximize x)(check-sat)(get-objectives)', 5),
            ('minimize-int',
             '(declare-const x Int)(assert (>= x (- 3)))(assert (<= x 5))'
             '(minimize x)(check-sat)(get-objectives)', -3),
            ('maximize-expression',
             '(declare-const x Int)(assert (>= x 0))(assert (<= x 4))'
             '(maximize (- (* x 3) 2))(check-sat)', 10),
        ]
        for name, source, expected in objectives:
            path = Path(directory) / (name + '.smt2')
            path.write_text(source + '\n')
            for variant in ('cube', 'unsigned', 'signed'):
                run = subprocess.run([binary, variant, str(path)],
                                     capture_output=True, text=True, timeout=10)
                result = json.loads(run.stdout)
                context = (name, variant, result, run.stderr)
                assert run.returncode == 0 and result['result'] == 'sat', context
                assert result['objective'] == expected and result['optimal'], context
                checks += 1
        # Resource limits are parser-wide, so one policy is sufficient here.
        limits = [
            ('nesting', '(assert ' + '(not '*100000 + 'true' + ')'*100000 + ')(check-sat)'),
            ('list-length', '(assert (and ' + 'true '*2000 + '))(check-sat)'),
            ('expansion-depth', '(assert (distinct ' + 'true '*60 + '))(check-sat)'),
            ('input-size', ';' + ' '* (16*1024*1024)),
            ('syntax-node', '(set-info :x 0)'*26000 + '(check-sat)'),
            ('evaluation nesting', '(define-fun b0 () Bool true)' + ''.join(
                f'(define-fun b{i} () Bool b{i-1})' for i in range(1, 300)) +
                '(assert b299)(check-sat)'),
        ]
        for name, source in limits:
            path = Path(directory) / 'limit.smt2'
            path.write_text(source)
            run = subprocess.run([binary, 'cube', str(path)],
                                 capture_output=True, text=True, timeout=10)
            result = json.loads(run.stdout)
            assert run.returncode == 1 and name in result['message'], (name, result)
        for mode in ('--parse-only', '--model-only', None):
            path.write_text('(check-sat)')
            run = subprocess.run([binary, 'invalid', str(path)] + ([mode] if mode else []),
                                 capture_output=True, text=True, timeout=10)
            assert run.returncode == 1 and 'variant' in json.loads(run.stdout)['message']
    print(f"Passed {checks} reader checks ({len(cases)} cases, 3 policies, 3 modes).")
    print('Passed 6 resource-limit checks and 3 invalid-variant checks.')


if __name__ == "__main__":
    main()
