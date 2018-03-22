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
        (void) new FlatZincTest("steiner_triples",
"predicate bool_lin_ge(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_gt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_lt(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
predicate bool_lin_ne(array [int] of int: a, array [int] of var bool: x, var int: c);\n\
var 0..1: INT____00002 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00004 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00006 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00008 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00010 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00012 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00014 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00016 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00018 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00020 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00022 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00024 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00026 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00028 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00030 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00032 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00034 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00036 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00038 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00040 :: is_defined_var :: var_is_introduced;\n\
var 0..1: INT____00042 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00001 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00003 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00005 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00007 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00009 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00011 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00013 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00015 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00017 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00019 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00021 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00023 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00025 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00027 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00029 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00031 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00033 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00035 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00037 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00039 :: is_defined_var :: var_is_introduced;\n\
var set of 1..7: SET____00041 :: is_defined_var :: var_is_introduced;\n\
array [1..7] of var set of 1..7: sets :: output_array([1..7]);\n\
constraint set_card(SET____00001, INT____00002) :: defines_var(INT____00002);\n\
constraint set_card(SET____00003, INT____00004) :: defines_var(INT____00004);\n\
constraint set_card(SET____00005, INT____00006) :: defines_var(INT____00006);\n\
constraint set_card(SET____00007, INT____00008) :: defines_var(INT____00008);\n\
constraint set_card(SET____00009, INT____00010) :: defines_var(INT____00010);\n\
constraint set_card(SET____00011, INT____00012) :: defines_var(INT____00012);\n\
constraint set_card(SET____00013, INT____00014) :: defines_var(INT____00014);\n\
constraint set_card(SET____00015, INT____00016) :: defines_var(INT____00016);\n\
constraint set_card(SET____00017, INT____00018) :: defines_var(INT____00018);\n\
constraint set_card(SET____00019, INT____00020) :: defines_var(INT____00020);\n\
constraint set_card(SET____00021, INT____00022) :: defines_var(INT____00022);\n\
constraint set_card(SET____00023, INT____00024) :: defines_var(INT____00024);\n\
constraint set_card(SET____00025, INT____00026) :: defines_var(INT____00026);\n\
constraint set_card(SET____00027, INT____00028) :: defines_var(INT____00028);\n\
constraint set_card(SET____00029, INT____00030) :: defines_var(INT____00030);\n\
constraint set_card(SET____00031, INT____00032) :: defines_var(INT____00032);\n\
constraint set_card(SET____00033, INT____00034) :: defines_var(INT____00034);\n\
constraint set_card(SET____00035, INT____00036) :: defines_var(INT____00036);\n\
constraint set_card(SET____00037, INT____00038) :: defines_var(INT____00038);\n\
constraint set_card(SET____00039, INT____00040) :: defines_var(INT____00040);\n\
constraint set_card(SET____00041, INT____00042) :: defines_var(INT____00042);\n\
constraint set_card(sets[1], 3);\n\
constraint set_card(sets[2], 3);\n\
constraint set_card(sets[3], 3);\n\
constraint set_card(sets[4], 3);\n\
constraint set_card(sets[5], 3);\n\
constraint set_card(sets[6], 3);\n\
constraint set_card(sets[7], 3);\n\
constraint set_intersect(sets[1], sets[2], SET____00001) :: defines_var(SET____00001);\n\
constraint set_intersect(sets[1], sets[3], SET____00003) :: defines_var(SET____00003);\n\
constraint set_intersect(sets[1], sets[4], SET____00005) :: defines_var(SET____00005);\n\
constraint set_intersect(sets[1], sets[5], SET____00007) :: defines_var(SET____00007);\n\
constraint set_intersect(sets[1], sets[6], SET____00009) :: defines_var(SET____00009);\n\
constraint set_intersect(sets[1], sets[7], SET____00011) :: defines_var(SET____00011);\n\
constraint set_intersect(sets[2], sets[3], SET____00013) :: defines_var(SET____00013);\n\
constraint set_intersect(sets[2], sets[4], SET____00015) :: defines_var(SET____00015);\n\
constraint set_intersect(sets[2], sets[5], SET____00017) :: defines_var(SET____00017);\n\
constraint set_intersect(sets[2], sets[6], SET____00019) :: defines_var(SET____00019);\n\
constraint set_intersect(sets[2], sets[7], SET____00021) :: defines_var(SET____00021);\n\
constraint set_intersect(sets[3], sets[4], SET____00023) :: defines_var(SET____00023);\n\
constraint set_intersect(sets[3], sets[5], SET____00025) :: defines_var(SET____00025);\n\
constraint set_intersect(sets[3], sets[6], SET____00027) :: defines_var(SET____00027);\n\
constraint set_intersect(sets[3], sets[7], SET____00029) :: defines_var(SET____00029);\n\
constraint set_intersect(sets[4], sets[5], SET____00031) :: defines_var(SET____00031);\n\
constraint set_intersect(sets[4], sets[6], SET____00033) :: defines_var(SET____00033);\n\
constraint set_intersect(sets[4], sets[7], SET____00035) :: defines_var(SET____00035);\n\
constraint set_intersect(sets[5], sets[6], SET____00037) :: defines_var(SET____00037);\n\
constraint set_intersect(sets[5], sets[7], SET____00039) :: defines_var(SET____00039);\n\
constraint set_intersect(sets[6], sets[7], SET____00041) :: defines_var(SET____00041);\n\
constraint set_le(sets[2], sets[1]);\n\
constraint set_le(sets[3], sets[2]);\n\
constraint set_le(sets[4], sets[3]);\n\
constraint set_le(sets[5], sets[4]);\n\
constraint set_le(sets[6], sets[5]);\n\
constraint set_le(sets[7], sets[6]);\n\
solve  :: set_search(sets, input_order, indomain_min, complete) satisfy;\n\
", "sets = array1d(1..7, [1..3, {1, 4, 5}, {1, 6, 7}, {2, 4, 6}, {2, 5, 7}, {3, 4, 7}, {3, 5, 6}]);\n\
----------\n\
");
      }
    };

    Create c;
  }

}}

// STATISTICS: test-flatzinc
