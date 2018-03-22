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
        (void) new FlatZincTest("sudoku",
"predicate all_different_int(array [int] of var int: x);\n\
predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var 1..9: X____00001;\n\
var 1..9: X____00002;\n\
var 1..9: X____00003;\n\
var 1..9: X____00004;\n\
var 1..9: X____00005;\n\
var 1..9: X____00006;\n\
var 1..9: X____00007;\n\
var 1..9: X____00008;\n\
var 1..9: X____00009;\n\
var 1..9: X____00010;\n\
var 1..9: X____00011;\n\
var 1..9: X____00012;\n\
var 1..9: X____00013;\n\
var 1..9: X____00014;\n\
var 1..9: X____00015;\n\
var 1..9: X____00016;\n\
var 1..9: X____00017;\n\
var 1..9: X____00018;\n\
var 1..9: X____00019;\n\
var 1..9: X____00020;\n\
var 1..9: X____00021;\n\
var 1..9: X____00022;\n\
var 1..9: X____00023;\n\
var 1..9: X____00024;\n\
var 1..9: X____00025;\n\
var 1..9: X____00026;\n\
var 1..9: X____00027;\n\
var 1..9: X____00028;\n\
var 1..9: X____00029;\n\
var 1..9: X____00030;\n\
var 1..9: X____00031;\n\
var 1..9: X____00032;\n\
var 1..9: X____00033;\n\
var 1..9: X____00034;\n\
var 1..9: X____00035;\n\
var 1..9: X____00036;\n\
var 1..9: X____00037;\n\
var 1..9: X____00038;\n\
var 1..9: X____00039;\n\
var 1..9: X____00040;\n\
var 1..9: X____00041;\n\
var 1..9: X____00042;\n\
var 1..9: X____00043;\n\
var 1..9: X____00044;\n\
var 1..9: X____00045;\n\
var 1..9: X____00046;\n\
var 1..9: X____00047;\n\
var 1..9: X____00048;\n\
var 1..9: X____00049;\n\
var 1..9: X____00050;\n\
var 1..9: X____00051;\n\
var 1..9: X____00052;\n\
var 1..9: X____00053;\n\
array [1..81] of var 1..9: puzzle :: output_array([1..9, 1..9]) = [X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009, X____00010, 6, 8, 4, X____00011, 1, X____00012, 7, X____00013, X____00014, X____00015, X____00016, X____00017, 8, 5, X____00018, 3, X____00019, X____00020, 2, 6, 8, X____00021, 9, X____00022, 4, X____00023, X____00024, X____00025, 7, X____00026, X____00027, X____00028, 9, X____00029, X____00030, X____00031, 5, X____00032, 1, X____00033, 6, 3, 2, X____00034, X____00035, 4, X____00036, 6, 1, X____00037, X____00038, X____00039, X____00040, X____00041, 3, X____00042, 2, X____00043, 7, 6, 9, X____00044, X____00045, X____00046, X____00047, X____00048, X____00049, X____00050, X____00051, X____00052, X____00053];\n\
constraint all_different_int([6, 1, X____00037, 2, X____00043, 7, X____00048, X____00049, X____00050]);\n\
constraint all_different_int([8, X____00021, 9, X____00026, X____00027, X____00028, 1, X____00033, 6]);\n\
constraint all_different_int([X____00001, X____00002, X____00003, X____00004, X____00005, X____00006, X____00007, X____00008, X____00009]);\n\
constraint all_different_int([X____00001, X____00002, X____00003, X____00010, 6, 8, X____00014, X____00015, X____00016]);\n\
constraint all_different_int([X____00001, X____00010, X____00014, X____00020, X____00024, X____00031, X____00035, X____00041, X____00045]);\n\
constraint all_different_int([X____00002, 6, X____00015, 2, X____00025, 5, 4, 3, X____00046]);\n\
constraint all_different_int([X____00003, 8, X____00016, 6, 7, X____00032, X____00036, X____00042, X____00047]);\n\
constraint all_different_int([X____00004, 4, X____00017, 8, X____00026, 1, 6, 2, X____00048]);\n\
constraint all_different_int([X____00004, X____00005, X____00006, 4, X____00011, 1, X____00017, 8, 5]);\n\
constraint all_different_int([X____00005, X____00011, 8, X____00021, X____00027, X____00033, 1, X____00043, X____00049]);\n\
constraint all_different_int([X____00006, 1, 5, 9, X____00028, 6, X____00037, 7, X____00050]);\n\
constraint all_different_int([X____00007, X____00008, X____00009, X____00012, 7, X____00013, X____00018, 3, X____00019]);\n\
constraint all_different_int([X____00007, X____00012, X____00018, X____00022, 9, 3, X____00038, 6, X____00051]);\n\
constraint all_different_int([X____00008, 7, 3, 4, X____00029, 2, X____00039, 9, X____00052]);\n\
constraint all_different_int([X____00009, X____00013, X____00019, X____00023, X____00030, X____00034, X____00040, X____00044, X____00053]);\n\
constraint all_different_int([X____00010, 6, 8, 4, X____00011, 1, X____00012, 7, X____00013]);\n\
constraint all_different_int([X____00014, X____00015, X____00016, X____00017, 8, 5, X____00018, 3, X____00019]);\n\
constraint all_different_int([X____00020, 2, 6, 8, X____00021, 9, X____00022, 4, X____00023]);\n\
constraint all_different_int([X____00020, 2, 6, X____00024, X____00025, 7, X____00031, 5, X____00032]);\n\
constraint all_different_int([X____00022, 4, X____00023, 9, X____00029, X____00030, 3, 2, X____00034]);\n\
constraint all_different_int([X____00024, X____00025, 7, X____00026, X____00027, X____00028, 9, X____00029, X____00030]);\n\
constraint all_different_int([X____00031, 5, X____00032, 1, X____00033, 6, 3, 2, X____00034]);\n\
constraint all_different_int([X____00035, 4, X____00036, 6, 1, X____00037, X____00038, X____00039, X____00040]);\n\
constraint all_different_int([X____00035, 4, X____00036, X____00041, 3, X____00042, X____00045, X____00046, X____00047]);\n\
constraint all_different_int([X____00038, X____00039, X____00040, 6, 9, X____00044, X____00051, X____00052, X____00053]);\n\
constraint all_different_int([X____00041, 3, X____00042, 2, X____00043, 7, 6, 9, X____00044]);\n\
constraint all_different_int([X____00045, X____00046, X____00047, X____00048, X____00049, X____00050, X____00051, X____00052, X____00053]);\n\
solve satisfy;\n\
", "puzzle = array2d(1..9, 1..9, [5, 9, 3, 7, 6, 2, 8, 1, 4, 2, 6, 8, 4, 3, 1, 5, 7, 9, 7, 1, 4, 9, 8, 5, 2, 3, 6, 3, 2, 6, 8, 5, 9, 1, 4, 7, 1, 8, 7, 3, 2, 4, 9, 6, 5, 4, 5, 9, 1, 7, 6, 3, 2, 8, 9, 4, 2, 6, 1, 8, 7, 5, 3, 8, 3, 5, 2, 4, 7, 6, 9, 1, 6, 7, 1, 5, 9, 3, 4, 8, 2]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
