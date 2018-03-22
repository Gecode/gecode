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

predicate gecode_table_int(array[int] of var int: x, array[int, int] of int: t);

predicate table_int(array[int] of var int: x, array[int, int] of int: t) =
    assert (index_set_2of2(t) == index_set(x),
        "The second dimension of the table must equal the number of variables "
            ++ "in the first argument",
        gecode_table_int(x,t));

predicate table_int_reif(array[int] of var int: x, array[int, int] of int: t,
                         var bool: b) =

    let { int: n_vars = length(x) }
    in

    assert(n_vars in 1..5,
        "'table' constraints in a reified context " ++
        "are only supported for 1..5 variables.",

    if n_vars = 1 then

        x[1] in { t[it,1] | it in index_set_1of2(t) } <-> b

    else

        let { set of int: ix  = index_set(x),
              set of int: full_size = 1..product(i in ix)( dom_size(x[i]) ),
              array[full_size, 1..n_vars + 1] of int: t_b =
                    array2d(full_size, 1..n_vars + 1,

                    if n_vars = 2 then

                        [ let { array[ix] of int: tpl = [i1,i2] } in
                            (tpl ++ [bool2int(aux_is_in_table(tpl,t))])[p]
                          | i1 in dom(x[1]),
                            i2 in dom(x[2]),
                            p in 1..n_vars + 1 ]

                    else if n_vars = 3 then

                        [ let { array[ix] of int: tpl = [i1,i2,i3] } in
                            (tpl ++ [bool2int(aux_is_in_table(tpl,t))])[p]
                          | i1 in dom(x[1]),
                            i2 in dom(x[2]),
                            i3 in dom(x[3]),
                            p in 1..n_vars + 1 ]

                    else if n_vars = 4 then

                        [ let { array[ix] of int: tpl = [i1,i2,i3,i4] }
                          in
                            (tpl ++ [bool2int(aux_is_in_table(tpl,t))])[p]
                          | i1 in dom(x[1]),
                            i2 in dom(x[2]),
                            i3 in dom(x[3]),
                            i4 in dom(x[4]),
                            p in 1..n_vars + 1 ]

                    else % if n_vars = 5 then

                        [ let { array[ix] of int: tpl = [i1,i2,i3,i4,i5] } in
                            (tpl ++ [bool2int(aux_is_in_table(tpl,t))])[p]
                          | i1 in dom(x[1]),
                            i2 in dom(x[2]),
                            i3 in dom(x[3]),
                            i4 in dom(x[4]),
                            i5 in dom(x[5]),
                            p in 1..n_vars + 1 ]

                    endif endif endif ) }
        in
        table_int(x ++ [bool2int(b)], t_b)

    endif
    );

test aux_is_in_table(array[int] of int: e, array[int, int] of int: t) =
    exists(i in index_set_1of2(t))(
        forall(j in index_set(e))( t[i,j] = e[j] )
    );
