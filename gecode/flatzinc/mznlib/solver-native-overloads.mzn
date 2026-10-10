%
%  Main authors:
%     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
%
%  Copyright:
%     Mikael Zayenz Lagerkvist, 2026
%
%  This file is part of Gecode, the generic constraint
%  development environment:
%     http://www.gecode.dev
%
%  Permission is hereby granted, free of charge, to any person obtaining
%  a copy of this software and associated documentation files (the
%  "Software"), to deal in the Software without restriction, including
%  without limitation the rights to use, copy, modify, merge, publish,
%  distribute, sublicense, and/or sell copies of the Software, and to
%  permit persons to whom the Software is furnished to do so, subject to
%  the following conditions:
%
%  The above copyright notice and this permission notice shall be
%  included in all copies or substantial portions of the Software.
%
%  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
%  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
%  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
%  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
%  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
%  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
%  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
%
%

include "all_equal.mzn";
include "regular.mzn";
include "nvalue_fn.mzn";
include "gecode_fzn.mzn";

% Typed standard operators preserve MiniZinc's index-set checks and use the
% non-reified native actor only in a positive root context.
function var bool: '!='(array[$U] of var $$E: x,
    array[$U] of var $$E: y) :: promise_commutative =
  matching_index_sets(x,y) /\
  if mzn_in_root_context() then
    gecode_array_int_ne(enum2int(array1d(x)),enum2int(array1d(y)))
  else
    let {any: xx = enum2int(array1d(x)); any: yy = enum2int(array1d(y));} in
      exists(i in index_set(xx))(xx[i] != yy[i])
  endif;

function var bool: '!='(array[$U] of var bool: x,
    array[$U] of var bool: y) :: promise_commutative =
  matching_index_sets(x,y) /\
  if mzn_in_root_context() then
    gecode_array_bool_ne(array1d(x),array1d(y))
  else
    let {any: xx = array1d(x); any: yy = array1d(y);} in
      exists(i in index_set(xx))(xx[i] != yy[i])
  endif;

predicate all_equal(array[$X] of var bool: xs) =
  if length(xs) <= 1 then true else fzn_all_equal_bool(array1d(xs)) endif;

predicate fzn_all_equal_bool(array[int] of var bool: xs) =
  (if length(xs) = 0 then true else
    forall(i in index_set(xs))(xs[i] = xs[min(index_set(xs))]) endif);

predicate fzn_all_equal_bool_reif(array[int] of var bool: xs, var bool: b) =
  if is_fixed(b) /\ not fix(b) then gecode_not_all_equal_bool(xs)
  else b <-> (if length(xs) = 0 then true else
    forall(i in index_set(xs))(xs[i] = xs[min(index_set(xs))]) endif) endif;

predicate nvalue(var int: n, array[$X] of var bool: xs) =
  if mzn_in_root_context() then
    gecode_nvalues_bool_fzn(array1d(xs),1,n)
  else nvalue(n,bool2int(xs)) endif;

function var int: nvalue(array[$X] of var bool: xs) :: promise_total =
  let {
    var bool2int(length(xs) > 0)..min(2,length(xs)): n :: is_defined_var;
    constraint gecode_nvalues_bool_fzn(array1d(xs),1,n) :: defines_var(n);
  } in n;

% Boolean regular uses symbols 0 and 1. Symbols missing from the standard
% alphabet have dead transitions. The normalized alphabet also lets the
% standard reified translation represent an out-of-alphabet Boolean as false.
predicate regular(array[int] of var bool: xs, int: Q, set of int: S,
    array[int,int] of int: d, int: q0, set of int: F) =
  assert(Q > 0, "regular: 'Q' must be greater than zero") /\
  assert(card(S) > 0, "regular: 'S' must be non empty") /\
  assert(index_set_1of2(d) = 1..Q /\ index_set_2of2(d) = S,
    "regular: the transition function 'd' must be [1..Q,S]") /\
  assert(forall(v in d)(v in 0..Q),
    "regular: transition function 'd' points to states outside 0..Q") /\
  assert(q0 in 1..Q, "regular: start state 'q0' not in 1..Q") /\
  assert(F subset 1..Q, "regular: final states in 'F' contain states outside 1..Q") /\
  let {
    array[1..Q,0..1] of int: dd = array2d(1..Q,0..1,
      [if symbol in S then d[state,symbol] else 0 endif
       | state in 1..Q, symbol in 0..1]);
  } in
    if mzn_in_root_context() then gecode_regular_bool(xs,Q,array1d(dd),q0,F)
    else regular(bool2int(xs),Q,0..1,dd,q0,F) endif;

predicate regular(array[int] of var bool: xs, int: Q, int: S,
    array[int,int] of int: d, int: q0, set of int: F) =
  assert(S > 0, "regular: 'S' must be greater than zero") /\
    regular(xs,Q,1..S,d,q0,F);
