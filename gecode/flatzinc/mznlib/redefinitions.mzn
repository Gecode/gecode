%
%  Main authors:
%     Guido Tack <tack@gecode.org>
%
%  Copyright:
%     Guido Tack, 2007
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

% Sums over Boolean variables
predicate bool_lin_eq(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_ne(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_le(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_lt(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_ge(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_gt(array[int] of int: a, array[int] of var bool: x,
                      var int: c);

predicate bool_sum_eq(array[int] of var bool: x, var int: c) =
    bool_lin_eq([1 | i in index_set(x)],x,c);
predicate bool_sum_ne(array[int] of var bool: x, var int: c) =
    bool_lin_ne([1 | i in index_set(x)],x,c);
predicate bool_sum_le(array[int] of var bool: x, var int: c) =
    bool_lin_le([1 | i in index_set(x)],x,c);
predicate bool_sum_lt(array[int] of var bool: x, var int: c) =
    bool_lin_lt([1 | i in index_set(x)],x,c);
predicate bool_sum_ge(array[int] of var bool: x, var int: c) =
    bool_lin_ge([1 | i in index_set(x)],x,c);
predicate bool_sum_gt(array[int] of var bool: x, var int: c) =
    bool_lin_gt([1 | i in index_set(x)],x,c);

predicate float_sinh(var float: a, var float: b) =
    b == (exp(a)-exp(-a))/2.0;

predicate float_cosh(var float: a, var float: b) =
    b == (exp(a)+exp(-a))/2.0;

predicate float_tanh(var float: a, var float: b) =
    let { var float: e2a = exp(2.0*a) } in
    b == (e2a-1.0)/(e2a+1.0);

predicate float_ne_reif(var float: a, var float: b, var bool: c) =
    not c <-> a==b;

predicate array_float_element(var int: b, array[int] of float: as, var float: c) =
    b in index_set(as) /\
    forall(d in index_set(as))( b = d -> as[d] = c );

predicate array_var_float_element(var int: b, array[int] of var float: as, var float: c) =
    b in index_set(as) /\
    forall(d in index_set(as))( b = d -> as[d] = c );

predicate gecode_int_pow(var int: x, int: y, var int: z);

predicate int_pow(var int: x, var int: y, var int: z) =
    if is_fixed(y) then gecode_int_pow(x,fix(y),z)
    else let {
        array[lb(y)..ub(y)] of var int: pow_x = array1d(lb(y)..ub(y),
          [if y=i then x else 0 endif | i in lb(y)..ub(y)]);
        array[lb(y)..ub(y)] of var int: pow_table = array1d(lb(y)..ub(y),
          [pow(pow_x[i],i) | i in lb(y)..ub(y)]);
    } in z=pow_table[y]
    endif;

predicate array_bool_and_imp(array [int] of var bool: as, var bool: r);
predicate array_bool_or_imp(array [int] of var bool: as, var bool: r);
predicate array_bool_xor_imp(array [int] of var bool: as, var bool: r);

predicate bool_and_imp(var bool: a, var bool: b, var bool: r);
predicate bool_clause_imp(array [int] of var bool: as,
                          array [int] of var bool: bs, var bool: r);
predicate bool_ge_imp(var bool: a, var bool: b, var bool: r);
predicate bool_gt_imp(var bool: a, var bool: b, var bool: r);
predicate bool_le_imp(var bool: a, var bool: b, var bool: r);
predicate bool_lt_imp(var bool: a, var bool: b, var bool: r);
predicate bool_ne_imp(var bool: a, var bool: b, var bool: r);
predicate bool_ne_imp(var bool: a, var bool: b, var bool: r);
predicate bool_or_imp(var bool: a, var bool: b, var bool: r);
predicate bool_xor_imp(var bool: a, var bool: b, var bool: r);

predicate bool_lin_eq_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);
predicate bool_lin_ge_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);
predicate bool_lin_gt_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);
predicate bool_lin_le_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);
predicate bool_lin_lt_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);
predicate bool_lin_ne_imp(array [int] of int: as, array [int] of var bool: bs,
                          var int: c, var bool: r);

predicate int_eq_imp(var int: a, var int: b, var bool: r);
predicate int_ge_imp(var int: a, var int: b, var bool: r);
predicate int_gt_imp(var int: a, var int: b, var bool: r);
predicate int_le_imp(var int: a, var int: b, var bool: r);
predicate int_lt_imp(var int: a, var int: b, var bool: r);
predicate int_ne_imp(var int: a, var int: b, var bool: r);

predicate int_lin_eq_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);
predicate int_lin_ge_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);
predicate int_lin_gt_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);
predicate int_lin_le_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);
predicate int_lin_lt_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);
predicate int_lin_ne_imp(array [int] of int: as, array [int] of var int: bs,
                         int: c, var bool: r);

predicate set_in_imp(var int: x, set of int: S, var bool: r);
