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

  namespace Count {

    enum Operation { OP_POPCOUNT, OP_LEADING, OP_TRAILING };

    static unsigned int
    count(Operation op, Gecode::WordValue value, unsigned int width) {
      if (op == OP_POPCOUNT) {
        unsigned int result = 0;
        while (value != 0) {
          value &= value-1;
          result++;
        }
        return result;
      }
      if (value == 0)
        return width;
      unsigned int result = 0;
      if (op == OP_LEADING) {
        Gecode::WordValue bit = Gecode::WordValue(1) << (width-1);
        while ((value & bit) == 0) {
          result++;
          bit >>= 1;
        }
      } else {
        Gecode::WordValue bit = 1;
        while ((value & bit) == 0) {
          result++;
          bit <<= 1;
        }
      }
      return result;
    }

    static void
    post(Gecode::Space& home, Operation op, Gecode::WordVar x,
         Gecode::IntVar result) {
      switch (op) {
      case OP_POPCOUNT: Gecode::popcount(home,x,result); break;
      case OP_LEADING: Gecode::count_leading_zeros(home,x,result); break;
      case OP_TRAILING: Gecode::count_trailing_zeros(home,x,result); break;
      default: GECODE_NEVER;
      }
    }

    class Assigned : public Test {
    private:
      Operation op;
      unsigned int expected;
    public:
      Assigned(Operation op0, unsigned int expected0, const std::string& name,
               unsigned int width=3)
        : Test("Count::Assigned::"+name+"::"+
               str(static_cast<int>(expected0)),
               1,Domain(width,0,Gecode::Word::width_mask(width))),
          op(op0), expected(expected0) {}
      virtual bool solution(const Assignment& a) const {
        return count(op,a[0],dom.width()) == expected;
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::IntVar result(home,static_cast<int>(expected),
                             static_cast<int>(expected));
        Count::post(home,op,x[0],result);
      }
    };

    class Lifecycle : public Base {
    private:
      class CountSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::IntVar result;
        CountSpace(unsigned int width, Gecode::WordValue lo,
                   Gecode::WordValue hi)
          : x(*this,width,lo,hi), result(*this,-1,65) {}
        CountSpace(CountSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          result.update(*this,s.result);
        }
        virtual Gecode::Space* copy(void) { return new CountSpace(*this); }
      };

      static bool population(void) {
        CountSpace bounds(4,5U,15U);
        Gecode::popcount(bounds,bounds.x,bounds.result);
        if ((bounds.status() == Gecode::SS_FAILED) ||
            (bounds.result.min() != 2) || (bounds.result.max() != 4))
          return false;
        Gecode::rel(bounds,bounds.result,Gecode::IRT_LQ,2);
        if ((bounds.status() == Gecode::SS_FAILED) ||
            !bounds.x.assigned() || (bounds.x.val() != 5U))
          return false;

        CountSpace upper(4,5U,15U);
        Gecode::popcount(upper,upper.x,upper.result);
        Gecode::rel(upper,upper.result,Gecode::IRT_GQ,4);
        if ((upper.status() == Gecode::SS_FAILED) ||
            !upper.x.assigned() || (upper.x.val() != 15U))
          return false;

        CountSpace failed(4,5U,5U);
        Gecode::popcount(failed,failed.x,failed.result);
        Gecode::rel(failed,failed.result,Gecode::IRT_EQ,1);
        return failed.status() == Gecode::SS_FAILED;
      }

      static bool zero_counts(void) {
        CountSpace leading(4,0U,3U);
        Gecode::count_leading_zeros(leading,leading.x,leading.result);
        if ((leading.status() == Gecode::SS_FAILED) ||
            (leading.result.min() != 2) || (leading.result.max() != 4))
          return false;
        Gecode::rel(leading,leading.result,Gecode::IRT_EQ,2);
        if ((leading.status() == Gecode::SS_FAILED) ||
            ((leading.x.lo() & 2U) == 0) || ((leading.x.hi() & 12U) != 0))
          return false;

        CountSpace trailing(4,0U,12U);
        Gecode::count_trailing_zeros(trailing,trailing.x,trailing.result);
        Gecode::rel(trailing,trailing.result,Gecode::IRT_EQ,2);
        if ((trailing.status() == Gecode::SS_FAILED) ||
            ((trailing.x.lo() & 4U) == 0) || ((trailing.x.hi() & 3U) != 0))
          return false;

        CountSpace prefix(4,0U,15U);
        Gecode::count_leading_zeros(prefix,prefix.x,prefix.result);
        Gecode::rel(prefix,prefix.result,Gecode::IRT_GQ,3);
        if ((prefix.status() == Gecode::SS_FAILED) ||
            ((prefix.x.hi() & 14U) != 0))
          return false;

        CountSpace zero(4,0U,0U);
        Gecode::count_trailing_zeros(zero,zero.x,zero.result);
        return (zero.status() != Gecode::SS_FAILED) &&
          zero.result.assigned() && (zero.result.val() == 4) &&
          (Gecode::PropagatorGroup::all.size(zero) == 0);
      }

      static bool widths_and_clone(void) {
        CountSpace one(1,0U,1U);
        Gecode::count_leading_zeros(one,one.x,one.result);
        Gecode::rel(one,one.result,Gecode::IRT_EQ,0);
        if ((one.status() == Gecode::SS_FAILED) || !one.x.assigned() ||
            (one.x.val() != 1U))
          return false;

        const Gecode::WordValue high = Gecode::WordValue(1) << 63;
        CountSpace leading_wide(64,high,high);
        Gecode::count_leading_zeros(leading_wide,leading_wide.x,
                                    leading_wide.result);
        if ((leading_wide.status() == Gecode::SS_FAILED) ||
            !leading_wide.result.assigned() ||
            (leading_wide.result.val() != 0))
          return false;
        CountSpace trailing_wide(64,high,high);
        Gecode::count_trailing_zeros(trailing_wide,trailing_wide.x,
                                     trailing_wide.result);
        if ((trailing_wide.status() == Gecode::SS_FAILED) ||
            !trailing_wide.result.assigned() ||
            (trailing_wide.result.val() != 63))
          return false;

        CountSpace wide(64,high,high | 1U);
        Gecode::popcount(wide,wide.x,wide.result);
        if (wide.status() == Gecode::SS_FAILED)
          return false;
        CountSpace* clone = static_cast<CountSpace*>(wide.clone());
        Gecode::rel(*clone,clone->result,Gecode::IRT_EQ,2);
        const bool ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x.assigned() && (clone->x.val() == (high | 1U)) &&
          !wide.x.assigned();
        delete clone;
        return ok;
      }

      static bool recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          IntVar population;
          IntVar leading;
          IntVar trailing;
          SearchSpace(void)
            : x(*this,3), population(*this,0,3),
              leading(*this,0,3), trailing(*this,0,3) {
            popcount(*this,x,population);
            count_leading_zeros(*this,x,leading);
            count_trailing_zeros(*this,x,trailing);
            WordVarArgs words = {x};
            branch(*this,words,WORD_VAR_NONE(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            population.update(*this,s.population);
            leading.update(*this,s.leading);
            trailing.update(*this,s.trailing);
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
          const WordValue value = solution->x.val();
          const bool ok = solution->population.assigned() &&
            solution->leading.assigned() && solution->trailing.assigned() &&
            (solution->population.val() ==
             static_cast<int>(count(OP_POPCOUNT,value,3))) &&
            (solution->leading.val() ==
             static_cast<int>(count(OP_LEADING,value,3))) &&
            (solution->trailing.val() ==
             static_cast<int>(count(OP_TRAILING,value,3))) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8;
      }

    public:
      Lifecycle(void) : Base("Word::Count::Lifecycle") {}
      virtual bool run(void) {
        return population() && zero_counts() && widths_and_clone() &&
          recomputation();
      }
    };

    Assigned pop0(OP_POPCOUNT,0,"Popcount");
    Assigned pop1(OP_POPCOUNT,1,"Popcount");
    Assigned pop2(OP_POPCOUNT,2,"Popcount");
    Assigned pop3(OP_POPCOUNT,3,"Popcount");
    Assigned leading0(OP_LEADING,0,"Leading");
    Assigned leading1(OP_LEADING,1,"Leading");
    Assigned leading2(OP_LEADING,2,"Leading");
    Assigned leading3(OP_LEADING,3,"Leading");
    Assigned trailing0(OP_TRAILING,0,"Trailing");
    Assigned trailing1(OP_TRAILING,1,"Trailing");
    Assigned trailing2(OP_TRAILING,2,"Trailing");
    Assigned trailing3(OP_TRAILING,3,"Trailing");
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
