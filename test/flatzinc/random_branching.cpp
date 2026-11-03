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
#include <gecode/flatzinc/branch.hh>
#include <gecode/search.hh>
#include <memory>
#include <sstream>

namespace Test { namespace FlatZinc {

  namespace {
    /// Integer and Boolean variables share one randomized value brancher.
    class IntBoolSpace : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      Gecode::BoolVarArray b;
      IntBoolSpace(void) : x(*this,5,0,3), b(*this,3,0,1) {
        using namespace Gecode;
        Gecode::FlatZinc::branch(*this,x,b,Gecode::FlatZinc::INTBOOL_VAR_AFC_SIZE_MAX(0.99),
                                 INT_VAL_RND(Rnd(3)));
      }
      IntBoolSpace(IntBoolSpace& s) : Space(s) {
        x.update(*this,s.x);
        b.update(*this,s.b);
      }
      Space* copy(void) override { return new IntBoolSpace(*this); }
    };

    std::string solutions(unsigned int distance) {
      Gecode::Search::Options options;
      options.c_d = distance; options.a_d = distance;
      IntBoolSpace root;
      Gecode::DFS<IntBoolSpace> search(&root,options);
      std::ostringstream result;
      for (int i=0; i<40; i++) {
        std::unique_ptr<IntBoolSpace> s(search.next());
        if (!s) break;
        result << s->x << s->b << ';';
      }
      return result.str();
    }

    /// Check that random value selection does not depend on recomputation
    class RandomIntBoolBranch : public Base {
    public:
      RandomIntBoolBranch(void) : Base("FlatZinc::random_intbool_branch") {}
      bool run(void) override {
        const std::string cloned = solutions(1);
        return (cloned == solutions(4)) && (cloned == solutions(100));
      }
    } random_intbool_branch;
  }

}}

// STATISTICS: test-flatzinc
