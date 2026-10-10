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

predicate gecode_int_ite(
  var bool: b,
  var int: x,
  var int: y,
  var int: z,
);

% A two-branch conditional ends with a fixed true condition for else.
predicate fzn_if_then_else_int(
  array[int] of var bool: c,
  array[int] of int: x,
  var int: y,
) =
  if length(c) = 2 /\ index_sets_agree(c, x) /\
      (if is_fixed(c[max(index_set(c))]) then fix(c[max(index_set(c))]) else false endif) then
    gecode_int_ite(c[min(index_set(c))], x[min(index_set(c))], x[max(index_set(c))], y)
  else
    let {
      array[index_set(c)] of var bool: d;
    } in
      forall (i in index_set(c)) (
        if i > min(index_set(c)) then
          d[i] = (not c[i - 1] /\ d[i - 1])
        else
          d[i] = true
        endif
      ) /\
      forall (i in index_set(c)) (
        c[i] /\ d[i] -> y = x[i]
      )
  endif;
