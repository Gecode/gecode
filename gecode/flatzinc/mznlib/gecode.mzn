%
%  Main authors:
%     Guido Tack <tack@gecode.org>
%
%  Copyright:
%     Guido Tack, 2007
%
%  Last modified:
%     $Date$ by $Author$
%     $Revision$
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

% This file contains predicate definitions for additional constraints available
% in the Gecode FlatZinc interpreter.

% i in z <-> exists (j in x) (i in y[j])
predicate array_set_element_union(var set of int: x, array[int] of set of int: y, var set of int: z);

% i in z <-> forall (j in x) (i in y[j])
predicate array_set_element_intersect(var set of int: x, array[int] of set of int: y, var set of int: z);

% (i in z <-> exists (j in x) (i in y[j])) /\ (i in x /\ j in x /\ i!=j -> disjoint(y[i],y[j]))
predicate array_set_element_partition(var set of int: x, array[int] of set of int: y, var set of int: z);

% z subset u /\ ( i in z <-> forall (j in x) (i in y[j]) )
predicate array_set_element_intersect_in(var set of int: x, array[int] of var set of int: y,
    var set of int: z, set of int: u);
