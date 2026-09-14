#!/usr/bin/env python3
"""Focused reader checks. Usage: python3 smt2-reader.py PATH/word-smt2-reader."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


def session_checks(binary):
    cases = {
        'scoped-assertions': ('(declare-const x Bool)(assert x)(check-sat)(push 1)'
            '(assert (not x))(check-sat)(pop 1)(check-sat)', ['sat', 'unsat', 'sat']),
        'scoped-declarations': ('(check-sat)(push 1)(declare-const x Bool)(assert x)'
            '(check-sat)(pop 1)(check-sat)', ['sat', 'sat', 'sat']),
        'reused-name': ('(push 1)(declare-const x Bool)(assert x)(check-sat)(pop 1)'
            '(declare-const x (_ BitVec 1))(assert (= x #b1))(check-sat)', ['sat', 'sat']),
        'scoped-definition': ('(push 1)(define-fun x () Bool true)(check-sat)(pop 1)'
            '(define-fun x () Bool false)(assert x)(check-sat)', ['sat', 'unsat']),
        'scoped-objective': ('(push 1)(maximize 3)(check-sat)(pop 1)'
            '(minimize 2)(check-sat)', ['sat', 'sat']),
        'options-persist': ('(push 1)(set-option :timeout 7)(set-option :produce-models true)'
            '(set-option :smt.arith.solver 2)(pop 1)(check-sat)(get-model)', ['sat']),
        'option-snapshots': ('(set-option :timeout 7)(check-sat)'
            '(set-option :timeout 8)(check-sat)', ['sat', 'sat']),
        'model-too-late': ('(check-sat)(set-option :produce-models true)(get-model)', 'error'),
        'metadata-after-result': ('(set-option :produce-models true)(check-sat)'
            '(set-info :source "metadata")(get-model)', ['sat']),
        'option-after-result': ('(set-option :produce-models true)(check-sat)'
            '(set-option :timeout 8)(get-model)', ['sat']),
        'popped-unknown-symbol': ('(push 1)(assert missing)(pop 1)(check-sat)', 'error'),
        'popped-type-error': ('(push 1)(assert (= true #b0))(pop 1)(check-sat)', 'error'),
        'popped-duplicate': ('(push 1)(declare-const x Bool)(declare-const x Bool)'
            '(pop 1)(check-sat)', 'error'),
        'popped-unsupported': ('(push 1)(frobnicate)(pop 1)(check-sat)', 'error'),
        'global-declarations': ('(set-option :global-declarations true)(check-sat)', 'error'),
        'unknown-option': ('(set-option :missing 1)(check-sat)', 'error'),
        'bad-timeout': ('(set-option :timeout -1)(check-sat)', 'error'),
        'bad-option-type': ('(set-option :produce-models 1)(check-sat)', 'error'),
        'pop-underflow': ('(pop 1)(check-sat)', 'error'),
        'zero-scopes': ('(set-option :produce-models true)(check-sat)'
            '(push 0)(pop 0)(get-model)', ['sat']),
        'scope-limit': ('(push 129)(check-sat)', 'error'),
        'scope-boundary': ('(push 128)(pop 128)(check-sat)', ['sat']),
        'query-limit': ('(check-sat)' * 129, 'error'),
        'query-boundary': ('(check-sat)' * 128, ['sat'] * 128),
        'invalid-after-query': ('(check-sat)(assert missing)', 'error'),
        'exit': ('(check-sat)(exit)(assert missing)', ['sat']),
    }
    count = 0
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / 'session.smt2'
        def run(source, variant='signed', *flags):
            path.write_text(source)
            process = subprocess.run([binary, variant, str(path), *flags],
                                     text=True, capture_output=True, timeout=10)
            records = [json.loads(line) for line in process.stdout.splitlines()]
            return process, records
        for name, (source, expected) in cases.items():
            for variant in ('cube', 'unsigned', 'signed'):
                process, records = run(source, variant)
                context = (name, variant, records, process.stderr)
                if expected == 'error':
                    assert process.returncode == 1 and records[0]['status'] == 'error', context
                else:
                    assert process.returncode == 0, context
                    assert [r['result'] for r in records] == expected, context
                    assert [r['query'] for r in records] == list(range(1, len(records)+1)), context
                    assert all(r['query_count'] == len(records) for r in records), context
                    if name == 'options-persist':
                        assert records[0]['timeout_ms'] == 7 and records[0]['produce_models'], context
                        assert records[0]['ignored_options'][0]['name'] == ':smt.arith.solver', context
                        assert records[0]['model'] == {}, context
                    if name == 'option-snapshots':
                        assert [r['timeout_ms'] for r in records] == [7, 8], context
                    if name == 'scoped-objective':
                        assert [r['objective'] for r in records] == [3, 2], context
                count += 1
        source = ('(declare-const |a\tname| Bool)(assert |a\tname|)'
                  '(declare-const low Int)(assert (= low (- 9223372036854775808)))'
                  '(declare-const high (_ BitVec 64))(assert (= high #xffffffffffffffff))'
                  '(declare-const spare Int)(declare-const spareb Bool)'
                  '(declare-const sparew (_ BitVec 7))(check-sat)')
        for variant in ('cube', 'unsigned', 'signed'):
            process, records = run(source, variant, '--model')
            assert process.returncode == 0, records
            model = records[0]['model']
            assert model['a\tname'] == {'sort': 'Bool', 'value': True}, model
            assert model['low'] == {'sort': 'Int', 'value': '-9223372036854775808'}, model
            assert model['high'] == {'sort': 'BitVec', 'width': 64, 'value': '18446744073709551615'}, model
            assert -(1 << 63) <= int(model['spare']['value']) < 1 << 63, model
            assert isinstance(model['spareb']['value'], bool), model
            assert 0 <= int(model['sparew']['value']) < 128, model
            count += 1
        process, records = run('(set-option :produce-models true)(assert false)(check-sat)(get-model)')
        assert records[0]['result'] == 'unsat' and 'model_error' in records[0], records
        count += 1
        process, records = run('(set-option :timeout 1)(declare-const x Int)'
            '(assert (>= x 0))(assert (<= x 1000000))'
            '(maximize (- (* x x)))(check-sat)', 'signed', '--model')
        assert process.returncode == 0, records
        if records[0]['result'] == 'unknown':
            assert records[0]['reason_unknown'] == 'timeout', records
            if 'objective' in records[0]:
                assert records[0]['optimal'] is False, records
        else:
            # A faster solver may finish within the time limit. This formula
            # is satisfiable and has optimum zero at x=0.
            assert records[0]['result'] == 'sat', records
            assert records[0]['objective'] == 0 and records[0]['optimal'], records
        if 'objective' in records[0]:
            x = int(records[0]['model']['x']['value'])
            assert 0 <= x <= 1000000 and records[0]['objective'] == -x ** 2, records
        count += 1
        # Many alternatives used to copy a large bounds map after the range
        # analysis budget was exhausted. The asserted formula remains intact.
        disjunctions = ''.join(f'(declare-const x{i} Int)(assert (>= x{i} 0))'
            f'(assert (<= x{i} 2))(assert (or (= x{i} 0) (= x{i} 1) (= x{i} 2)))'
            for i in range(700))
        process, records = run(disjunctions + '(check-sat)', 'signed', '--model-only')
        assert process.returncode == 0 and records[0]['status'] == 'ok', records
        count += 1
        # Snapshot accounting bounds repeated copies independently of input size.
        oversized = '(assert true)' * 5000 + '(check-sat)' * 128
        process, records = run(oversized, 'signed', '--parse-only')
        assert process.returncode == 1 and 'expanded-query limit' in records[0]['message'], records
        count += 1
    print(f'Passed {count} session, options, model, timeout, and analysis-budget checks.')


def normalization_checks(binary):
    """Check native Int factoring with exact Python arithmetic, without Z3."""
    def integer(value):
        return str(value) if value >= 0 else f'(- {-value})'

    def declarations(values):
        return ''.join(f'(declare-const {name} Int)(assert (= {name} {integer(value)}))'
                       for name, value in values.items())

    # Each entry carries an independent set of allowed public assignments.
    cases = []
    bounds = '(declare-const x Int)(declare-const y Int)(declare-const z Int)' + ''.join(
        f'(assert (>= {name} {integer(low)}))(assert (<= {name} {high}))'
        for name, low, high in [('x', -3, 3), ('y', -2, 4), ('z', -2, 2)])
    for operation, sign in [('+', 1), ('-', -1)]:
        for target in [6, 1]:
            expression = f'({operation} (* 2 x y) (* 2 x z))'
            supported = {(x, y, z) for x in range(-3, 4) for y in range(-2, 5)
                         for z in range(-2, 3) if 2*x*y + sign*2*x*z == target}
            cases.append((f'factor-{operation}-{target}',
                          bounds + f'(assert (= {expression} {target}))(check-sat)',
                          ('x', 'y', 'z'), supported, True))

    def fixed(name, values, expression, expected, rewritten):
        cases.append((name, declarations(values) +
                      f'(assert (= {expression} {integer(expected)}))(check-sat)',
                      tuple(values), {tuple(values.values())}, rewritten))

    fixed('factor-corpus-shape', {'x': 5, 'y': 3},
          '(- (* 4 x y) (* 4 x))', 4*5*3-4*5, True)
    fixed('factor-reordered-products', {'x': -3, 'y': 5},
          '(+ (* 2 x) (* y x))', 2*(-3)+5*(-3), True)
    fixed('factor-repeated-variable', {'x': -3},
          '(+ (* x x) (* x 2))', (-3)*(-3)+(-3)*2, True)
    fixed('factor-empty-left-residual', {'x': -3},
          '(- x (* x 2))', (-3)-(-3)*2, True)
    fixed('factor-empty-right-residual', {'x': -3},
          '(+ (* x 2) x)', (-3)*2+(-3), True)
    fixed('factor-let-simultaneous', {'x': 2, 'y': 3, 'z': 4},
          '(let ((x y) (y x)) (+ (* x y) (* x z)))', 3*2+3*4, True)
    fixed('factor-let-shadows-bounds', {'x': -(1 << 63)},
          '(let ((x 2)) (+ (* x 3) (* x 4)))', 14, True)
    fixed('factor-separate-lexical-scopes', {},
          '(+ (let ((x 2)) (* x 3)) (let ((x 5)) (* x 7)))', 41, False)
    # The original products and sum fit. A new residual sum would overflow.
    fixed('factor-residual-add-overflow', {'x': 0},
          '(+ (* x 9223372036854775807) (* x 1))', 0, False)
    fixed('factor-residual-sub-overflow', {'x': 0},
          '(- (* x (- 9223372036854775808)) (* x 1))', 0, False)
    # |1| names a variable; the unquoted 1 is a different, constant term.
    fixed('factor-quoted-symbol-vs-literal', {'|1|': 2},
          '(+ (* |1| 3) (* 1 4))', 2*3+1*4, False)
    fixed('factor-quoted-and-simple-symbol', {'x': 2},
          '(+ (* |x| 3) (* x 4))', 14, True)
    many = '(* ' + ' '.join(['x']*17) + ')'
    fixed('factor-size-fallback', {'x': 1}, f'(+ {many} {many})', 2, False)
    cases.append(('factor-global-definition',
                  declarations({'x': 5}) +
                  '(define-fun f () Int (+ (* x 2) (* x 3)))'
                  '(assert (= (let ((x (- 7))) f) 25))(check-sat)',
                  ('x',), {(5,)}, True))
    cases.append(('factor-zero-divisor-fallback',
                  declarations({'x': 2}) + '(assert (= (div 3 0) 7))'
                  '(assert (= (+ (* x (div 3 0)) (* x 1)) 16))(check-sat)',
                  ('x',), {(2,)}, False))
    cases.append(('factor-bitvector-unchanged',
                  '(declare-const x (_ BitVec 4))(assert (= x #xf))'
                  '(assert (= (bvadd (bvmul x #x2) (bvmul x #x3)) #xb))(check-sat)',
                  ('x',), {(15,)}, False))
    count = 0
    with tempfile.TemporaryDirectory(prefix='word-normalization-') as directory:
        path = Path(directory) / 'case.smt2'
        for name, source, names, supported, rewritten in cases:
            path.write_text(source)
            for variant in ('cube', 'unsigned', 'signed'):
                run = subprocess.run([binary, variant, str(path), '--model'],
                                     capture_output=True, text=True, timeout=10)
                result = json.loads(run.stdout)
                context = (name, variant, result, run.stderr)
                assert run.returncode == 0 and result['status'] == 'ok', context
                assert result['result'] == ('sat' if supported else 'unsat'), context
                assert (result['integer_normalizations'] > 0) == rewritten, context
                if supported:
                    assignment = tuple(int(result['model'][n.strip('|')]['value']) for n in names)
                    assert assignment in supported, context
                count += 1
    print(f'Passed {count} dependency-free native normalization checks.')


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
        ("quoted-theory-constants", '(assert |true|)(assert (not |false|))'
         '(check-sat)', "sat"),
        ("theory-declaration", '(declare-const true Bool)(check-sat)', "error"),
        ("quoted-theory-declaration", '(declare-fun |false| () Bool)'
         '(check-sat)', "error"),
        ("theory-definition", '(define-fun |true| () Bool false)'
         '(check-sat)', "error"),
        ("theory-binding", '(assert (let ((|false| true)) |false|))'
         '(check-sat)', "error"),
        ("theory-integer-binding", '(assert (let ((div 1)) (= div 1)))'
         '(check-sat)', "error"),
        ("theory-word-declaration", '(declare-const |bvadd| (_ BitVec 8))'
         '(check-sat)', "error"),
        ("string-declaration", '(declare-const "x" Bool)(check-sat)', "error"),
        ("string-binding", '(assert (let (("x" true)) true))'
         '(check-sat)', "error"),
        ("malformed-empty-let", '(assert (let))(check-sat)', "error"),
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
    cases.extend([
        ('integer-div-zero-nonzero',
         '(assert (= (div 1 0) 7))(check-sat)', 'sat'),
        ('integer-mod-zero-negative',
         '(assert (= (mod 1 0) (- 3)))(check-sat)', 'sat'),
        ('integer-zero-functions-distinct',
         '(assert (= (div 1 0) 7))(assert (= (mod 1 0) 8))(check-sat)', 'sat'),
        ('integer-zero-same-argument',
         '(assert (= (div 1 0) 7))(assert (= (div 1 0) 8))(check-sat)', 'unsat'),
        ('integer-zero-different-arguments',
         '(assert (= (div 1 0) 7))(assert (= (div 2 0) 8))(check-sat)', 'sat'),
        ('integer-zero-dynamic-congruence',
         '(declare-const x Int)(declare-const y Int)'
         '(assert (>= x 0))(assert (<= x 3))(assert (>= y 0))(assert (<= y 3))'
         '(assert (= (div x 0) 7))(assert (= (div y 0) 8))'
         '(assert (= x y))(check-sat)', 'unsat'),
        ('integer-zero-expression-congruence',
         '(declare-const x Int)(declare-const y Int)'
         '(assert (>= x 0))(assert (<= x 3))(assert (>= y 1))(assert (<= y 4))'
         '(assert (= (mod (+ x 1) 0) 7))(assert (= (mod y 0) 8))'
         '(assert (= y (+ x 1)))(check-sat)', 'unsat'),
        ('integer-zero-nonzero-independent',
         '(declare-const d Int)(assert (>= d 1))(assert (<= d 2))'
         '(assert (= (div 6 d) 3))(assert (= (div 6 0) 7))(check-sat)', 'sat'),
        ('integer-range-negative-divisor',
         '(declare-const x Int)(declare-const d Int)(declare-const q Int)'
         '(assert (= x (- 19)))(assert (>= d (- 5)))(assert (<= d (- 2)))'
         '(assert (= q (div x d)))(assert (= q 10))(check-sat)', 'sat'),
        ('integer-range-checked-add',
         '(declare-const x Int)(declare-const y Int)'
         '(assert (= x 9223372036854775807))(assert (= y (+ x 1)))'
         '(assert (= y (- 9223372036854775808)))(check-sat)', 'sat'),
        ('integer-range-checked-multiply',
         '(declare-const x Int)(declare-const y Int)'
         '(assert (= x (- 9223372036854775808)))(assert (= y (* x 1)))'
         '(assert (= y x))(check-sat)', 'sat'),
        ('integer-range-modular-bv',
         '(declare-const x (_ BitVec 4))(declare-const y (_ BitVec 4))'
         '(assert (= x #xf))(assert (= y (bvadd x #x1)))'
         '(assert (= y #x0))(check-sat)', 'sat'),
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
            ('maximize-factored-expression',
             '(declare-const x Int)(assert (>= x (- 3)))(assert (<= x 5))'
             '(maximize (+ (* x 2) (* x 3)))(check-sat)', 25),
            ('maximize-zero-function',
             '(assert (>= (div 1 0) 0))(assert (<= (div 1 0) 5))'
             '(maximize (div 1 0))(check-sat)', 5),
            ('maximize-variable-zero-function',
             '(declare-const d Int)(assert (>= d 0))(assert (<= d 2))'
             '(assert (= (div 8 d) 5))(maximize d)(check-sat)', 0),
            ('maximize-self-equality',
             '(declare-const x Int)(assert (= x x))(maximize x)(check-sat)',
             (1 << 63)-1),
            ('minimize-self-equality',
             '(declare-const x Int)(assert (= x x))(minimize x)(check-sat)',
             -(1 << 63)),
            ('maximize-negation',
             '(declare-const x Int)(assert (>= x 0))(assert (<= x 10000))'
             '(maximize (- x))(check-sat)', 0),
            ('minimize-negation',
             '(declare-const x Int)(assert (>= x 0))(assert (<= x 10000))'
             '(minimize (- x))(check-sat)', -10000),
            ('maximize-negative-square',
             '(declare-const x Int)(assert (>= x (- 3)))(assert (<= x 5))'
             '(maximize (- (* x x)))(check-sat)', 0),
            ('minimize-square',
             '(declare-const x Int)(assert (>= x (- 3)))(assert (<= x 5))'
             '(minimize (* x x))(check-sat)', 0),
            ('range-guarded-ceil',
             '(declare-const x Int)(declare-const q Int)(declare-const r Int)'
             '(assert (>= x 1))(assert (<= x 100))'
             '(assert (let ((d (div x 7)))'
             ' (=> (= (- x (* 7 d)) 0) (= q d))))'
             '(assert (let ((d (div x 7)))'
             ' (=> (distinct (- x (* 7 d)) 0) (= q (+ d 1)))))'
             '(assert (=> (= (mod q 4) 0) (= r (div q 4))))'
             '(assert (=> (distinct (mod q 4) 0) (= r (+ (div q 4) 1))))'
             '(maximize r)(check-sat)', 4),
            ('range-disjunction',
             '(declare-const x Int)(declare-const y Int)'
             '(assert (>= x 1))(assert (<= x 64))'
             '(assert (or (and (>= x 16) (= y 16))'
             ' (and (not (>= x 16)) (= y x))))'
             '(maximize y)(check-sat)', 16),
            ('range-incomplete-guards',
             '(declare-const x Int)(declare-const y Int)'
             '(assert (>= x 0))(assert (<= x 5))'
             '(assert (>= y 0))(assert (<= y 20))'
             '(assert (=> (< x 3) (= y 2)))'
             '(assert (=> (>= x 4) (= y 3)))'
             '(maximize y)(check-sat)', 20),
            ('range-negative-quotient',
             '(declare-const x Int)(declare-const y Int)'
             '(assert (>= x (- 10)))(assert (<= x (- 1)))'
             '(assert (= y (div x 3)))(minimize y)(check-sat)', -4),
        ]
        shared = '(declare-const x Int)(assert (>= x 0))(assert (<= x 1))'
        shared += '(define-fun twice0 () Int x)'
        shared += ''.join(f'(define-fun twice{i} () Int (+ twice{i-1} twice{i-1}))'
                          for i in range(1, 41))
        objectives.append(('shared-affine-objective',
                           shared + '(maximize twice40)(check-sat)', 1 << 40))
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
                if name in ('maximize-negation', 'minimize-negation'):
                    # Search must follow the objective, rather than enumerate
                    # thousands of increasingly good input values.
                    assert result['nodes'] < 100, context
                checks += 1
        # Resource limits are parser-wide, so one policy is sufficient here.
        limits = [
            ('nesting', '(assert ' + '(not '*100000 + 'true' + ')'*100000 + ')(check-sat)'),
            ('list-length', '(assert (and ' + 'true '*2000 + '))(check-sat)'),
            ('expansion-depth', '(assert (distinct ' + 'true '*60 + '))(check-sat)'),
            ('input-size', ';' + ' '* (16*1024*1024)),
            ('syntax-node', '(set-info :x 0)'*26000 + '(check-sat)'),
            ('zero-divisor function', ''.join(
                f'(assert (= (div {i} 0) 0))' for i in range(257)) + '(check-sat)'),
            ('evaluation nesting', '(define-fun b0 () Bool true)' + ''.join(
                f'(define-fun b{i} () Bool b{i-1})' for i in range(1, 300)) +
                '(assert b299)(check-sat)'),
            ('evaluation nesting', '(define-fun n0 () Int 0)' + ''.join(
                f'(define-fun n{i} () Int (+ n{i-1} 1))' for i in range(1, 300)) +
                '(assert (>= n299 0))(check-sat)'),
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
    session_checks(binary)
    normalization_checks(binary)
    print(f"Passed {checks} reader checks ({len(cases)} cases, 3 policies, 3 modes).")
    print(f'Passed {len(limits)} resource-limit checks and 3 invalid-variant checks.')


if __name__ == "__main__":
    main()
