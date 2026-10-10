/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
#include <gecode/flatzinc/ast.hh>

namespace Test { namespace FlatZinc {
  namespace {
    // The algorithm annotation must select the native filtering stage.
    class AlgorithmAnnotations : public Base {
    public:
      AlgorithmAnnotations(void) : Base("FlatZinc::NativeRegistry::AlgorithmAnnotations") {}
      virtual bool run(void) {
        const char* names[] = {"gecode_basic_propagation",
          "gecode_advanced_propagation", "gecode_full_propagation"};
        for (int i=0; i<3; i++) {
          Gecode::FlatZinc::FlatZincSpace space;
          Gecode::FlatZinc::AST::Array ann;
          ann.a.push_back(new Gecode::FlatZinc::AST::Atom(names[i]));
          ann.a.push_back(new Gecode::FlatZinc::AST::Atom("domain"));
          Gecode::IntVarArgs x(3);
          x[0] = Gecode::IntVar(space,2,6);
          x[1] = Gecode::IntVar(space,10,14);
          x[2] = Gecode::IntVar(space,4,15);
          Gecode::IntVar distance(space,6,8);
          Gecode::inter_distance(space,x,distance,space.ann2ipl(&ann));
          if ((space.status() == Gecode::SS_FAILED) || !distance.assigned() ||
              (distance.val() != 6))
            return false;
          if (i == 0) {
            if ((x[0].min() != 2) || (x[0].max() != 6)) return false;
          } else if (!x.assigned() || (x[0].val() != 2) ||
                     (x[1].val() != 14) || (x[2].val() != 8)) {
            return false;
          }
        }
        return true;
      }
    } algorithm_annotations;

    class Create {
    public:
      Create(void) {
        const std::string sat = "----------\n";
        const std::string unsat = "=====UNSATISFIABLE=====\n";
        (void) new FlatZincTest("NativeRegistry::DistanceShared",
          "var 0..2:x; constraint gecode_inter_distance([x,x],1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::NotAllEqual",
          "constraint gecode_not_all_equal_int([1,1,2]); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::CardinalityHoles",
          "constraint gecode_global_cardinality_sets([1,1],[1],[{0,3}]); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::SortOrientation",
          "constraint gecode_sort_permutation([3,1,2],[1,2,3],[2,3,1],1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::ArgMinShared",
          "var 1..2:x; constraint gecode_arg_min_int([x,x],1,x,false); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::DivmodNegative",
          "constraint gecode_int_divmod(-7,4,-1,-3); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::PathCostSentinel",
          "constraint gecode_path_cost_array([0,4,9,1,0,5,7,2,0],0,[1,2,3],0,2,[4,5,0],9); solve satisfy;", sat);
        (void) new FlatZincErrorTest("NativeRegistry::SortSize",
          "constraint gecode_sort_permutation([1,2],[1],[1,2],1); solve satisfy;",
          {}, "sort arrays must have equal lengths");
#ifdef GECODE_HAS_SET_VARS
        (void) new FlatZincTest("NativeRegistry::AtmostOneSingletonCardinality",
          "constraint gecode_set_atmost_one([{1}],2); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::AtmostOneUnitCardinality",
          "constraint gecode_set_atmost_one([{1},{1}],1); solve satisfy;", sat);
        (void) new FlatZincErrorTest("NativeRegistry::SetMatrixSize",
          "constraint gecode_set_element2d([{1}],0,0,0,1,{1}); solve satisfy;",
          {}, "invalid set matrix dimensions");
        (void) new FlatZincTest("NativeRegistry::EmptySetMinReification",
          "constraint gecode_set_min_reif({},1,false); solve satisfy;", sat);
#endif
      }
    };
    Create c;
  }
}}

// STATISTICS: test-flatzinc
