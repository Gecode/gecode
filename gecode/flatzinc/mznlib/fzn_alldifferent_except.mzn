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

include "fzn_all_different_int.mzn";
include "global_cardinality_low_up.mzn";

predicate gecode_all_different_except(array[int] of var int: x, int: except_value);

predicate fzn_alldifferent_except(array[int] of var int: xs, set of int: except) =
  if length(xs) = 0 then
    true
  elseif card(except) = 1 then
    if min(except) < lb_array(xs) \/ min(except) > ub_array(xs) then
      % Drop irrelevant exceptions before FlatZinc checks integer limits.
      fzn_all_different_int(xs)
    else
      gecode_all_different_except(xs, min(except))
    endif
  elseif forall (i in index_set(xs)) (
    has_bounds(xs[i])
  ) then
    % Native distinct supports one exception; retain the standard translation
    % for larger exception sets.
    let {
      set of int: values = dom_array(xs) diff except;
    } in
      global_cardinality(xs, set2array(values), [0 | i in values], [1 | i in values])
  else
    forall (i, j in index_set(xs) where i < j) (
      (xs[i] in except /\ xs[j] in except) \/ xs[i] != xs[j]
    )
  endif;
