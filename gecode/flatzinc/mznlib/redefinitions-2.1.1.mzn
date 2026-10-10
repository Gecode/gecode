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

% Set extrema are undefined for empty sets. The partial functions guard the
% extremum constraint with set nonemptiness, for use in inactive expressions.
predicate gecode_set_min(var set of int: s, var int: m);
predicate gecode_set_min_reif(var set of int: s, var int: m, var bool: b);
predicate gecode_set_min_imp(var set of int: s, var int: m, var bool: b);
predicate gecode_set_max(var set of int: s, var int: m);
predicate gecode_set_max_reif(var set of int: s, var int: m, var bool: b);
predicate gecode_set_max_imp(var set of int: s, var int: m, var bool: b);

function var $$E: min(var set of $$E: s) =
  if card(ub(enum2int(s))) = 0 then
    % An empty upper bound makes the extremum undefined at compilation.
    let {
      var set of int: si = enum2int(s);
      var min(ub(si)) .. max(ub(si)): m =
        min([e + (max(ub(si)) + 1 - e) * (1 - (e in si)) | e in ub(si)]);
    } in
      to_enum_internal(enum_of(s), m)
  else
    to_enum_internal(
      enum_of(s),
      if mzn_in_root_context() then
        gecode_set_min_value(enum2int(s))
      else
        let {
          constraint card(s) > 0;
        } in
          gecode_set_min_partial(enum2int(s))
      endif
    )
  endif;

function var int: gecode_set_min_value(var set of int: s) :: promise_total =
  let {
    var min(ub(s) union {1}) .. max(ub(s) union {1}): m :: is_defined_var;
    constraint card(s) > 0;
    constraint gecode_set_min(s, m) :: defines_var(m);
  } in m;

function var int: gecode_set_min_partial(var set of int: s) :: promise_total =
  let {
    var min(ub(s) union {1}) .. max(ub(s) union {1}): m :: is_defined_var;
    var bool: present = card(s) > 0;
    constraint gecode_set_min_imp(s, m, present) :: defines_var(m);
  } in m;

function var $$E: max(var set of $$E: s) =
  if card(ub(enum2int(s))) = 0 then
    % An empty upper bound makes the extremum undefined at compilation.
    let {
      var set of int: si = enum2int(s);
      var min(ub(si)) .. max(ub(si)): m =
        max([e + (min(ub(si)) - 1 - e) * (1 - (e in si)) | e in ub(si)]);
    } in
      to_enum_internal(enum_of(s), m)
  else
    to_enum_internal(
      enum_of(s),
      if mzn_in_root_context() then
        gecode_set_max_value(enum2int(s))
      else
        let {
          constraint card(s) > 0;
        } in
          gecode_set_max_partial(enum2int(s))
      endif
    )
  endif;

function var int: gecode_set_max_value(var set of int: s) :: promise_total =
  let {
    var min(ub(s) union {1}) .. max(ub(s) union {1}): m :: is_defined_var;
    constraint card(s) > 0;
    constraint gecode_set_max(s, m) :: defines_var(m);
  } in m;

function var int: gecode_set_max_partial(var set of int: s) :: promise_total =
  let {
    var min(ub(s) union {1}) .. max(ub(s) union {1}): m :: is_defined_var;
    var bool: present = card(s) > 0;
    constraint gecode_set_max_imp(s, m, present) :: defines_var(m);
  } in m;
