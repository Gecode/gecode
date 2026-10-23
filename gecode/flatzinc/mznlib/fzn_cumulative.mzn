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

predicate gecode_unary_flex(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: e,
);
predicate gecode_cumulative_flex(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: e,
  array[int] of int: r,
  var int: b,
);
predicate gecode_cumulative_flex_optional(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: e,
  array[int] of int: r,
  var int: b,
  array[int] of var bool: present,
);
predicate gecode_cumulatives_flex(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: e,
  array[int] of var int: r,
  array[int] of var int: m,
  array[int] of int: b,
  bool: upper,
);

predicate fzn_cumulative(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: r,
  var int: b,
) =
  if length(s) = 0 then
    true
  elseif length(s) = 1 then
    (d[min(index_set(d))] > 0 -> r[min(index_set(r))] <= b)
  elseif lb(b) < 0 then
    % Retain the legacy treatment of negative capacity domains.
    gecode_cumulatives(s, d, r, b)
  else
    let {
      % Model-level task bounds must fit Gecode's implicit integer domain.
      int: int_limit = 2147483646;
      array[int] of var -int_limit .. int_limit: start = array1d(s);
      array[int] of var -int_limit .. int_limit: duration = array1d(d);
      array[int] of var -int_limit .. int_limit: usage = array1d(r);
      var -int_limit .. int_limit: capacity = b;
      array[int] of var -int_limit .. int_limit: end = [start[i] + duration[i] | i in index_set(start)];
      bool: positive_durations = forall (di in duration) (
        lb(di) > 0
      );
      int: earliest_start = min([lb(si) | si in start]);
      int: latest_end = max([ub(ei) | ei in end]);
      array[int] of int: min_usages = sort([lb(ri) | ri in usage]);
      bool: is_disjunctive = positive_durations /\
      (min_usages[1] > ub(capacity) div 2 \/ (min_usages[2] > ub(capacity) div 2 /\
      min_usages[1] + min_usages[2] > ub(capacity)));
    } in
    if is_disjunctive then
      forall (ri in usage) (
        ri <= capacity
      ) /\
      if is_fixed(duration) then
        gecode_schedule_unary(start, fix(duration))
      else
        gecode_unary_flex(start, duration, end)
      endif
    elseif is_fixed(usage) then
      if positive_durations then
        if is_fixed(duration) then
          gecode_schedule_cumulative_optional(
            start,
            fix(duration),
            fix(usage),
            [true | i in index_set(start)],
            capacity,
          )
        else
          gecode_cumulative_flex(start, duration, end, fix(usage), capacity)
        endif
      else
        % MiniZinc's zero-duration tasks consume no capacity.
        gecode_cumulative_flex_optional(
          start,
          duration,
          end,
          fix(usage),
          capacity,
          [duration[i] > 0 | i in index_set(start)],
        )
      endif
    elseif positive_durations /\ is_fixed(capacity) then
      gecode_cumulatives_flex(
        start,
        duration,
        end,
        usage,
        [0 | i in index_set(start)],
        [fix(capacity)],
        true,
      )
    elseif latest_end - earliest_start <= int_limit /\ sum (ri in usage) (ub(ri)) <= int_limit then
      forall (i in index_set(start)) (
        duration[i] > 0 -> usage[i] <= capacity
      ) /\
      % Put zero-duration tasks on a separate machine. A task spanning the
      % horizon with usage -capacity turns the variable bound into a fixed zero bound.
      % The second bound is immaterial: only empty tasks use that machine.
      % The usage-sum guard keeps the native integer profile representable.
      gecode_cumulatives_flex(
        start ++ [earliest_start],
        duration ++ [latest_end - earliest_start],
        end ++ [latest_end],
        usage ++ [-capacity],
        [bool2int(duration[i] = 0) | i in index_set(start)] ++ [0],
        [0, max([ub(ri) | ri in usage])],
        true,
      )
    else
      % Avoid an unrepresentable spanning duration or resource profile.
      % With nonnegative usage the maximum load occurs at a task start.
      % The number of constraints depends only on task count.
      forall (j in index_set(start)) (
        sum (i in index_set(start)) (
          bool2int(start[i] <= start[j] /\ start[j] < end[i]) * usage[i]
        ) <= capacity
      )
    endif
  endif;

predicate gecode_schedule_unary(
  array[int] of var int: s,
  array[int] of int: d,
);
predicate gecode_schedule_cumulative_optional(
  array[int] of var int: s,
  array[int] of int: d,
  array[int] of int: r,
  array[int] of var bool: present,
  var int: b,
);

% Retained for compatibility with existing FlatZinc and MiniZinc models.
predicate gecode_cumulatives(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: r,
  var int: b,
);
