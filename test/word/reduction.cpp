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
 */

#include "test/word.hh"

#include <gecode/search.hh>

namespace Test { namespace Word {

  namespace Reduction {

    enum Operation { OP_AND, OP_OR, OP_XOR };

    static bool
    result(Operation op, Gecode::WordValue value,
           Gecode::WordValue mask) {
      switch (op) {
      case OP_AND: return value == mask;
      case OP_OR:  return value != 0;
      case OP_XOR:
        value ^= value >> 32;
        value ^= value >> 16;
        value ^= value >> 8;
        value ^= value >> 4;
        value ^= value >> 2;
        value ^= value >> 1;
        return (value & 1U) != 0;
      default: GECODE_NEVER;
      }
      return false;
    }

    class Assigned : public Test {
    private:
      Operation op;
      int expected;
    public:
      Assigned(Operation op0, int expected0, const std::string& name,
               unsigned int width=3)
        : Test("Reduction::Assigned::"+name+"::"+str(expected0),
               1,Domain(width,0,Gecode::Word::width_mask(width))),
          op(op0), expected(expected0) {}
      virtual bool solution(const Assignment& a) const {
        return static_cast<int>(result(op,a[0],dom.mask())) == expected;
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::BoolVar b(home,expected,expected);
        switch (op) {
        case OP_AND: Gecode::reduce_and(home,x[0],b); break;
        case OP_OR:  Gecode::reduce_or(home,x[0],b); break;
        case OP_XOR: Gecode::reduce_xor(home,x[0],b); break;
        }
      }
    };

    class Lifecycle : public Base {
    private:
      class ReductionSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::BoolVar b;
        ReductionSpace(unsigned int width, Gecode::WordValue lo,
                       Gecode::WordValue hi)
          : x(*this,width,lo,hi), b(*this,0,1) {}
        ReductionSpace(ReductionSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          b.update(*this,s.b);
        }
        virtual Gecode::Space* copy(void) { return new ReductionSpace(*this); }
      };

      static bool seams(void) {
        ReductionSpace a(3,3U,7U);
        Gecode::reduce_and(a,a.x,a.b);
        Gecode::rel(a,a.b,Gecode::IRT_EQ,0);
        if ((a.status() == Gecode::SS_FAILED) || !a.x.assigned() ||
            (a.x.val() != 3U))
          return false;

        ReductionSpace o(3,0U,4U);
        Gecode::reduce_or(o,o.x,o.b);
        Gecode::rel(o,o.b,Gecode::IRT_EQ,1);
        if ((o.status() == Gecode::SS_FAILED) || !o.x.assigned() ||
            (o.x.val() != 4U))
          return false;

        ReductionSpace p(3,1U,5U);
        Gecode::reduce_xor(p,p.x,p.b);
        Gecode::rel(p,p.b,Gecode::IRT_EQ,0);
        return (p.status() != Gecode::SS_FAILED) && p.x.assigned() &&
          (p.x.val() == 5U);
      }

      static bool width_and_clone(void) {
        ReductionSpace wide(64,Gecode::WordValue(1) << 63,
                            (Gecode::WordValue(1) << 63) | 1U);
        Gecode::reduce_xor(wide,wide.x,wide.b);
        if (wide.status() == Gecode::SS_FAILED)
          return false;
        ReductionSpace* clone = static_cast<ReductionSpace*>(wide.clone());
        Gecode::rel(*clone,clone->b,Gecode::IRT_EQ,0);
        const bool ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x.assigned() &&
          (clone->x.val() == ((Gecode::WordValue(1) << 63) | 1U)) &&
          !wide.x.assigned();
        delete clone;
        return ok;
      }

      static bool failure_and_subsumption(void) {
        ReductionSpace failed(1,1U,1U);
        Gecode::reduce_and(failed,failed.x,failed.b);
        Gecode::rel(failed,failed.b,Gecode::IRT_EQ,0);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        ReductionSpace fixed(1,0U,0U);
        Gecode::reduce_or(fixed,fixed.x,fixed.b);
        return (fixed.status() != Gecode::SS_FAILED) && fixed.b.zero() &&
          (Gecode::PropagatorGroup::all.size(fixed) == 0);
      }

      static bool recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          BoolVar b;
          SearchSpace(void) : x(*this,3), b(*this,0,1) {
            reduce_xor(*this,x,b);
            WordVarArgs words = {x};
            branch(*this,words,WORD_VAR_NONE(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            b.update(*this,s.b);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 1;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SearchSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() && solution->b.assigned() &&
            (solution->b.val() == static_cast<int>(result(
              OP_XOR,solution->x.val(),7U))) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8;
      }

    public:
      Lifecycle(void) : Base("Word::Reduction::Lifecycle") {}
      virtual bool run(void) {
        return seams() && width_and_clone() && failure_and_subsumption() &&
          recomputation();
      }
    };

    Assigned and0(OP_AND,0,"And");
    Assigned and1(OP_AND,1,"And");
    Assigned or0(OP_OR,0,"Or");
    Assigned or1(OP_OR,1,"Or");
    Assigned xor0(OP_XOR,0,"Xor");
    Assigned xor1(OP_XOR,1,"Xor");
    Assigned width1(OP_XOR,1,"Width1",1);
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
