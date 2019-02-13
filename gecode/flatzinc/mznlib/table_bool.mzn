%
%  Main authors:
%     Christian Schulte <schulte@gecode.org>
%     Guido Tack <tack@gecode.org>
%
%  Copyright:
%     Christian Schulte, 2018
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

predicate gecode_table_bool(array[int] of var bool: x,
                            array[int] of int: t);

predicate gecode_table_bool_reif(array[int] of var bool: x,
                                 array[int] of int: t, var bool: b);

predicate gecode_table_bool_imp(array[int] of var bool: x,
                                array[int] of int: t, var bool: b);

predicate table_bool(array[int] of var bool: x, array[int, int] of int: t) =
    assert (index_set_2of2(t) == index_set(x),
        "The second dimension of the table must equal the number of variables "
            ++ "in the first argument",
        gecode_table_bool(x,array1d(t)));

predicate table_bool_reif(array[int] of var bool: x, array[int, int] of int: t,
                          var bool: b) =
    assert (index_set_2of2(t) == index_set(x),
        "The second dimension of the table must equal the number of variables "
            ++ "in the first argument",
        gecode_table_bool_reif(x,array1d(t),b));

predicate table_bool_imp(array[int] of var bool: x, array[int, int] of int: t,
                         var bool: b) =
    assert (index_set_2of2(t) == index_set(x),
        "The second dimension of the table must equal the number of variables "
            ++ "in the first argument",
        gecode_table_bool_imp(x,array1d(t),b));
