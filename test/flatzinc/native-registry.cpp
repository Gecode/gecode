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

#include <memory>

namespace Test { namespace FlatZinc {
  namespace {
    // Annotations preserve both consistency and algorithm flags.
    class AlgorithmAnnotations : public Base {
    public:
      AlgorithmAnnotations(void)
        : Base("FlatZinc::NativeRegistry::AlgorithmAnnotations") {}
      virtual bool run(void) {
        const char* names[] = {"gecode_basic_propagation",
          "gecode_advanced_propagation", "gecode_full_propagation"};
        const Gecode::IntPropLevel levels[] = {
          Gecode::IPL_BASIC, Gecode::IPL_ADVANCED, Gecode::IPL_FULL
        };
        for (int i=0; i<3; i++) {
          Gecode::FlatZinc::FlatZincSpace space;
          Gecode::FlatZinc::AST::Array ann;
          ann.a.push_back(new Gecode::FlatZinc::AST::Atom(names[i]));
          ann.a.push_back(new Gecode::FlatZinc::AST::Atom("domain"));
          if (space.ann2ipl(&ann) != (Gecode::IPL_DOM | levels[i]))
            return false;
        }
        return true;
      }
    } algorithm_annotations;

    /// Advanced cumulative propagation detects an overload before search
    class CumulativeAlgorithmAnnotations : public Base {
    public:
      CumulativeAlgorithmAnnotations(void)
        : Base("FlatZinc::NativeRegistry::CumulativeAlgorithmAnnotations",
               TestTags::check()) {}
      virtual bool run(void) {
        const char* levels[] = {"gecode_basic_propagation",
          "gecode_advanced_propagation", "gecode_full_propagation"};
        for (int i=0; i<3; i++) {
          std::stringstream source;
          source << "var 1..4:a; var 0..6:b; var 2..6:c; var 0..1:e; "
                    "constraint gecode_cumulatives([a,b,c,0,e],"
                    "[3,1,1,2,4],[3,3,1,2,1],3) :: " << levels[i]
                 << "; solve satisfy;";
          Gecode::FlatZinc::Printer printer;
          std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> space(
            Gecode::FlatZinc::parse(source, printer, olog));
          if (!space)
            return false;
          const bool failed = space->status() == Gecode::SS_FAILED;
          if (failed != (i != 0)) {
            olog << "Unexpected root propagation for " << levels[i]
                 << std::endl;
            return false;
          }
        }
        return true;
      }
    } cumulative_algorithm_annotations;

    class Create {
    public:
      Create(void) {
        const std::string sat = "----------\n";
        const std::string unsat = "=====UNSATISFIABLE=====\n";
        // Explicit ends retain native scheduling semantics, including shared
        // starts, empty tasks, and negative resource usage.
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::UnaryFlexSharedStart",
          "var 0..1:s; var 1..2:e; "
          "constraint int_plus(s,1,e); "
          "constraint gecode_unary_flex([s,s],[1,1],[e,e]); solve satisfy;", unsat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativeFlexSharedStart",
          "var 0..1:s; var 1..2:e; "
          "constraint int_plus(s,1,e); "
          "constraint gecode_cumulative_flex([s,s],[1,1],[e,e],[1,1],1); "
          "solve satisfy;", unsat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativeFlexZeroDuration",
          "constraint gecode_cumulative_flex([0,1],[2,0],[2,1],[1,1],1); "
          "solve satisfy;", unsat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativesFlexNegativeUsage",
          "constraint gecode_cumulatives_flex([0,0],[2,2],[2,2],"
          "[3,-2],[0,0],[1],true); solve satisfy;", sat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativesFlexLowerBound",
          "constraint gecode_cumulatives_flex([0,0],[2,2],[2,2],"
          "[3,-2],[0,0],[2],false); solve satisfy;", unsat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativeSingletonZeroDuration",
          "var 0..1:s; var 2..3:r; "
          "constraint gecode_cumulatives([s],[0],[r],1); solve satisfy;", sat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativeSingletonPositiveDuration",
          "var 0..1:s; var 2..3:r; "
          "constraint gecode_cumulatives([s],[1],[r],1); solve satisfy;", unsat);
        (void) new FlatZincTest(TestTags::check(),
          "NativeRegistry::CumulativeSingletonVariableDuration",
          "var 0..1:s; var 0..1:d :: output_var; var 2..3:r; "
          "constraint gecode_cumulatives([s],[d],[r],1); solve satisfy;",
          "d = 0;\n----------\n");
        (void) new FlatZincTest("NativeRegistry::DistanceShared",
          "var 0..2:x; constraint gecode_inter_distance([x,x],1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::DistanceNegativeEmpty",
          "constraint gecode_inter_distance_fzn([],-1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::DistanceNegativeSingleton",
          "constraint gecode_inter_distance_fzn([0],-1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::NotAllEqual",
          "constraint gecode_not_all_equal_int([1,1,2]); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::CardinalityHoles",
          "var {0,3}: count; constraint gecode_global_cardinality_closed([1,1],[1],[count]); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::SortOrientation",
          "constraint gecode_sort_permutation([3,1,2],[1,2,3],[2,3,1],1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::CountMatchesUnbounded",
          "var int:x :: output_var; var int:y :: output_var; "
          "constraint gecode_count_matches_fzn([x],[3],1,1); "
          "constraint gecode_count_matches_fzn([y],[-3],1,1); solve satisfy;",
          "x = 3;\ny = -3;\n----------\n");
        (void) new FlatZincTest("NativeRegistry::CountMatchesRelations",
          "var int:a :: output_var; var 3..4:b :: output_var; "
          "var 3..4:c :: output_var; var int:d :: output_var; "
          "var int:e :: output_var; "
          "constraint gecode_count_matches_fzn([a],[3],2,0); "
          "constraint gecode_count_matches_fzn([b],[3],3,0); "
          "constraint gecode_count_matches_fzn([c],[3],4,1); "
          "constraint gecode_count_matches_fzn([d],[3],5,1); "
          "constraint gecode_count_matches_fzn([e],[3],6,0); solve satisfy;",
          "a = 3;\nb = 4;\nc = 4;\nd = 3;\ne = 3;\n----------\n");
        (void) new FlatZincTest("NativeRegistry::CountMatchesLimits",
          "var -10..2147483646:x :: output_var; "
          "var -2147483646..10:y :: output_var; var -10..10:z; "
          "constraint gecode_count_matches_fzn([x],[2147483646],1,1); "
          "constraint gecode_count_matches_fzn([y],[-2147483646],1,1); "
          "constraint gecode_count_matches_fzn([z,z],[-2147483646,2147483646],1,0); solve satisfy;",
          "x = 2147483646;\ny = -2147483646;\n----------\n");
        (void) new FlatZincTest("NativeRegistry::ArgMinShared",
          "var 1..2:x; constraint gecode_arg_min_int([x,x],1,x,false); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::InvalidModulusReification",
          "constraint gecode_product_mod_var_fzn_reif([3],0,3,false); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::MinimumDistanceRepeatedSite",
          "constraint gecode_minimum_distance_fzn([0,0,2],3,[0,2,4,2,0,3,4,3,0],1); solve satisfy;", unsat);
        (void) new FlatZincTest("NativeRegistry::NondeterministicPrefix",
          "constraint gecode_regular_nfa_int_fzn([0,1],3,0,2,[{1,2},{},{},{3},{},{}],1,{3}); solve satisfy;", sat);
        (void) new FlatZincErrorTest("NativeRegistry::SortSize",
          "constraint gecode_sort_permutation([1,2],[1],[1,2],1); solve satisfy;",
          {}, "sort arrays must have equal lengths");
#ifdef GECODE_HAS_SET_VARS
        (void) new FlatZincTest("NativeRegistry::SetSequenceEmpty",
          "constraint gecode_array_set_seq([]); solve satisfy;", sat);
        (void) new FlatZincTest("NativeRegistry::SetSequenceEmptyUnion",
          "constraint gecode_array_set_seq_union([],{1}); solve satisfy;", unsat);
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
