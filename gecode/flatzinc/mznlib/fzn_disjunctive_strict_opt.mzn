%
%  Main authors:
%     Guido Tack <tack@gecode.org>
%     Mikael lagerkvist <lagerkvist@gecode.org>
%
%  Copyright:
%     Guido Tack, 2007
%     Mikael Lagerkvist, 2009
%
%  This file is part of Gecode, the generic constraint
%  development environment:
%     http://www.gecode.org
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

predicate gecode_schedule_unary_optional(array[int] of var int: x,
    array[int] of int: p, array[int] of var bool: m);

include "fzn_disjunctive_strict_opt_decomp.mzn";

predicate fzn_disjunctive_strict_opt(array[int] of var opt int: s,
                                     array[int] of var int: d) =
    forall (i in index_set(d)) (d[i] >= 0) /\
    if forall (i in index_set(d)) (is_fixed(d[i])) then
      gecode_schedule_unary_optional(
        [ if is_fixed(si) /\ absent(fix(si)) then 0 else deopt(si) endif | si in s],
        [ fix(di) | di in d],
        [ occurs(si) | si in s ]
      )
    else fzn_disjunctive_strict_opt_decomp(s,d) endif;
