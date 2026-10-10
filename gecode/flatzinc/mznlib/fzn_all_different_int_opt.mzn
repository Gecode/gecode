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

predicate gecode_all_different_optional(
  array[int] of var int: x,
  array[int] of var bool: present,
);

predicate fzn_all_different_int_opt(array[int] of var opt int: xs) =
  if length(xs) = 0 then
    true
  else
    % Fixed absent values have no deopt value; their integer is ignored.
    gecode_all_different_optional(
      [if is_fixed(x) /\ absent(fix(x)) then 0 else deopt(x) endif | x in xs],
      [occurs(x) | x in xs],
    )
  endif;

% Reification has no native optional-distinct propagator.
predicate fzn_all_different_int_opt_reif(
  array[int] of var opt int: xs,
  var bool: b,
) =
  let {
    array[index_set(xs)] of var int: values = array1d(
      index_set(xs),
      [if is_fixed(x) /\ absent(fix(x)) then 0 else deopt(x) endif | x in xs],
    );
  } in
    b <-> forall (i, j in index_set(xs) where i < j) (
      (occurs(xs[i]) /\ occurs(xs[j])) -> values[i] != values[j]
    );
