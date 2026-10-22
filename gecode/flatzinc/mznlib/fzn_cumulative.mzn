%
%  Main authors:
%     Guido Tack <tack@gecode.dev>
%     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
%
%  Copyright:
%     Guido Tack, 2007
%     Mikael Zayenz Lagerkvist, 2009
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

predicate gecode_unary_flex(array[int] of var int: s,
  array[int] of var int: d, array[int] of var int: e);
predicate gecode_cumulative_flex(array[int] of var int: s,
  array[int] of var int: d, array[int] of var int: e,
  array[int] of int: r, var int: b);
predicate gecode_cumulative_flex_optional(array[int] of var int: s,
  array[int] of var int: d, array[int] of var int: e,
  array[int] of int: r, var int: b, array[int] of var bool: present);
predicate gecode_cumulatives_flex(array[int] of var int: s,
  array[int] of var int: d, array[int] of var int: e,
  array[int] of var int: r, array[int] of var int: m,
  array[int] of int: b, bool: upper);

predicate fzn_cumulative(array[int] of var int: s,
                         array[int] of var int: d,
                         array[int] of var int: r, var int: b) =
  if length(s) = 0 then
    true
  elseif length(s) = 1 then
    (d[min(index_set(d))] > 0 -> r[min(index_set(r))] <= b)
  elseif lb(b) < 0 then
    % Retain the legacy treatment of negative capacity domains.
    gecode_cumulatives(s, d, r, b)
  else
    let {
      array[int] of var int: start = array1d(s);
      array[int] of var int: duration = array1d(d);
      array[int] of var int: usage = array1d(r);
      array[int] of var int: end = [start[i] + duration[i] | i in index_set(start)];
      bool: positive = forall(di in d)(lb(di) > 0);
      array[int] of int: heights = sort([lb(ri) | ri in r]);
      bool: disjunctive = positive /\
        (heights[1] > ub(b) div 2 \/
         (heights[2] > ub(b) div 2 /\ heights[1] + heights[2] > ub(b)));
    } in
    if disjunctive then
      forall(ri in r)(ri <= b) /\
      if is_fixed(d) then
        gecode_schedule_unary(start, fix(duration))
      else
        gecode_unary_flex(start, duration, end)
      endif
    elseif is_fixed(r) then
      if positive then
        if is_fixed(d) then
          gecode_schedule_cumulative_optional(start, fix(duration),
            fix(usage), [true | i in index_set(start)], b)
        else
          gecode_cumulative_flex(start, duration, end, fix(usage), b)
        endif
      else
        % MiniZinc's zero-duration tasks consume no capacity.
        gecode_cumulative_flex_optional(start, duration, end, fix(usage), b,
          [duration[i] > 0 | i in index_set(start)])
      endif
    elseif positive /\ is_fixed(b) then
      gecode_cumulatives_flex(start, duration, end, usage,
        [0 | i in index_set(start)], [fix(b)], true)
    else
      % With nonnegative usage the maximum load occurs at a task start.
      % Unlike a time-point decomposition, size does not depend on the horizon.
      forall(j in index_set(start)) (
        sum(i in index_set(start)) (
          bool2int(start[i] <= start[j] /\ start[j] < end[i]) * usage[i]
        ) <= b
      )
    endif
  endif;

predicate gecode_schedule_unary(array[int] of var int: s,
  array[int] of int: d);
predicate gecode_schedule_cumulative_optional(array[int] of var int: s,
  array[int] of int: d, array[int] of int: r,
  array[int] of var bool: present, var int: b);

% Retained for compatibility with existing FlatZinc and MiniZinc models.
predicate gecode_cumulatives(array[int] of var int: s,
  array[int] of var int: d, array[int] of var int: r, var int: b);
