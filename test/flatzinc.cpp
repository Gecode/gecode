/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2014
 *
 *  Last modified:
 *     $Date: 2012-04-05 20:00:11 +1000 (Thu, 05 Apr 2012) $ by $Author: vbarichard $
 *     $Revision: 12703 $
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

  FlatZincTest::FlatZincTest(const std::string& s, bool allSolutions)
    : Base("FlatZinc::"+s), _filename(s), _allSolutions(allSolutions) {}

  bool
  FlatZincTest::run(void) {
    using namespace Gecode;
    Support::Timer t_total;
    t_total.start();
    Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
    fznopt.allSolutions(_allSolutions);
    Gecode::FlatZinc::Printer p;
    Gecode::FlatZinc::FlatZincSpace* fg = NULL;
    try {
      fg = Gecode::FlatZinc::parse("test/flatzinc/"+_filename+".fzn", p, olog);

      if (fg) {
        fg->createBranchers(fg->solveAnnotations(), fznopt.seed(), fznopt.decay(),
                            false, olog);
        fg->shrinkArrays(p);
        std::ostringstream os;
        fg->run(os, p, fznopt, t_total);
        
        std::ifstream expected_f("test/flatzinc/"+_filename+".exp");
        std::string expected((std::istreambuf_iterator<char>(expected_f)),
                              std::istreambuf_iterator<char>());
        if (expected == os.str()) {
          return true;
        } else {
          if (opt.log)
            olog << "FlatZinc produced the following output:\n" << os.str() << "\n";          
          return false;
        }
      } else {
        if (opt.log)
          olog << "Could not parse input\n";          
        return false;
      }
      delete fg;
    } catch (Gecode::FlatZinc::Error& e) {
      if (opt.log)
        olog << ind(2) << "FlatZinc error : " << e.toString() << std::endl;
      return false;
    }
    
  }

  /// Helper class to create and register tests
  class Create {
  public:

    /// Perform creation and registration
    Create(void) {
      (void) new FlatZincTest("bool_clause");
      (void) new FlatZincTest("bug232");
      (void) new FlatZincTest("bug319");
      (void) new FlatZincTest("bugfix_r6746");
      (void) new FlatZincTest("bugfix_r7854");
      (void) new FlatZincTest("empty_domain_1");
      (void) new FlatZincTest("empty_domain_2");
      (void) new FlatZincTest("int_set_as_type1");
      (void) new FlatZincTest("int_set_as_type2");
      (void) new FlatZincTest("jobshop");
      (void) new FlatZincTest("no_warn_empty_domain");
      (void) new FlatZincTest("output_test");
      (void) new FlatZincTest("queens4");
      (void) new FlatZincTest("sat_arith1");
      (void) new FlatZincTest("sat_array_bool_and");
      (void) new FlatZincTest("sat_array_bool_or");
      (void) new FlatZincTest("sat_cmp_reif");
      (void) new FlatZincTest("sat_eq_reif");
      (void) new FlatZincTest("test_approx_bnb", true);
      (void) new FlatZincTest("test_array_just_right");
      (void) new FlatZincTest("test_assigned_var_bounds_bad");
      (void) new FlatZincTest("test_flatzinc_output_anns");
      (void) new FlatZincTest("test_fzn_arith");
      (void) new FlatZincTest("test_fzn_arrays");
      (void) new FlatZincTest("test_fzn_coercions");
      (void) new FlatZincTest("test_fzn_comparison");
      (void) new FlatZincTest("test_fzn_logic");
      (void) new FlatZincTest("test_fzn_sets");
      (void) new FlatZincTest("test_int_div");
      (void) new FlatZincTest("test_int_mod");
      (void) new FlatZincTest("test_int_ranges_as_values");
      (void) new FlatZincTest("test_seq_search");      
    }
  };

  Create c;
  //@}

}}

// STATISTICS: test-other
