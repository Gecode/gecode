/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2014
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "test/flatzinc.hh"

namespace Test { namespace FlatZinc {

  namespace {
    /// Helper class to create and register tests
    class Create {
    public:

      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("eq20",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
array [1..7] of var 0..10: x :: output_array([0..6]);\n\
constraint int_lin_eq([-85698, 29958, 57308, 48789, -78219, 4657, 34539], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 249912);\n\
constraint int_lin_eq([-76706, 98205, 23445, 67921, 24111, -48614, -41906], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 821228);\n\
constraint int_lin_eq([-67456, 84750, -51553, 21239, 81675, -99395, -4254], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 277271);\n\
constraint int_lin_eq([-64919, 80460, 90840, -59624, -75542, 25145, -47935], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 18465);\n\
constraint int_lin_eq([-60301, 31227, 93951, 73889, 81526, -72702, 68026], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 1410723);\n\
constraint int_lin_eq([-60113, 29475, 34421, -76870, 62646, 29278, -15212], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 251591);\n\
constraint int_lin_eq([-45086, 51830, -4578, 96120, 21231, 97919, 65651], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 1198280);\n\
constraint int_lin_eq([-43277, 43525, 92298, 58630, 92590, -9372, -60227], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 1503588);\n\
constraint int_lin_eq([-16835, 47385, 97715, -12640, 69028, 76212, -81102], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 1244857);\n\
constraint int_lin_eq([-16105, 62397, -6704, 43340, 95100, -68610, 58301], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 876370);\n\
constraint int_lin_eq([-10343, 87758, -11782, 19346, 70072, -36991, 44529], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 740061);\n\
constraint int_lin_eq([1671, -34121, 10763, 80609, 42532, 93520, -33488], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 915683);\n\
constraint int_lin_eq([8874, -58412, 73947, 17147, 62335, 16005, 8632], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 752447);\n\
constraint int_lin_eq([49149, 52871, -7132, 56728, -33576, -49530, -62089], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 146074);\n\
constraint int_lin_eq([51637, 67761, 95951, 3834, -96722, 59190, 15280], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 533909);\n\
constraint int_lin_eq([71202, -11119, 73017, -38875, -14413, -29234, 72370], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 129768);\n\
constraint int_lin_eq([85176, -95332, -1268, 57898, 15883, 50547, 83287], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 373854);\n\
constraint int_lin_eq([85268, 54180, -18810, -48219, 6013, 78169, -79785], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 90614);\n\
constraint int_lin_eq([87059, -29101, -5513, -21219, 22128, 7276, 57308], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 22167);\n\
constraint int_lin_eq([94016, -82071, 35961, 66597, -30705, -44404, -38304], [x[1], x[2], x[3], x[4], x[5], x[6], x[7]], 25334);\n\
solve satisfy;\n\
", "x = array1d(0..6, [1, 4, 6, 6, 6, 3, 1]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
