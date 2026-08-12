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

namespace Test { namespace Word { namespace ProductMod {

  class Lifecycle : public Base {
  private:
    class ProductSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar y;
      Gecode::IntVar modulus;
      Gecode::WordVar result;
      ProductSpace(unsigned int width=4, int minimum=1, int maximum=10)
        : x(*this,width), y(*this,width),
          modulus(*this,minimum,maximum), result(*this,width) {}
      ProductSpace(ProductSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
        modulus.update(*this,s.modulus);
        result.update(*this,s.result);
      }
      virtual Gecode::Space* copy(void) {
        return new ProductSpace(*this);
      }
      void post(void) {
        Gecode::product_mod(*this,x,y,modulus,result);
      }
    };

    static bool assigned(void) {
      for (Gecode::WordValue x=0; x<8; x++)
        for (Gecode::WordValue y=0; y<8; y++)
          for (int modulus=1; modulus<=7; modulus++) {
            ProductSpace s(3,modulus,modulus);
            Gecode::dom(s,s.x,x);
            Gecode::dom(s,s.y,y);
            s.post();
            if ((s.status() == Gecode::SS_FAILED) ||
                !s.result.assigned() ||
                (s.result.val() != (x*y)%
                 static_cast<Gecode::WordValue>(modulus)) ||
                (Gecode::PropagatorGroup::all.size(s) != 0))
              return false;
          }

      ProductSpace wide(64,2147483646,2147483646);
      const Gecode::WordValue maximum=~Gecode::WordValue(0);
      Gecode::dom(wide,wide.x,maximum);
      Gecode::dom(wide,wide.y,maximum);
      wide.post();
      return (wide.status() != Gecode::SS_FAILED) &&
        wide.result.assigned() && (wide.result.val() == 225U);
    }

    static bool partial(void) {
      ProductSpace hull(4,1,8);
      hull.post();
      if ((hull.status() == Gecode::SS_FAILED) ||
          (hull.result.hi() != 7U))
        return false;

      ProductSpace lower(4,1,10);
      Gecode::dom(lower,lower.result,5U,7U);
      lower.post();
      if ((lower.status() == Gecode::SS_FAILED) ||
          (lower.modulus.min() != 6))
        return false;

      ProductSpace nonzero(4,7,7);
      Gecode::dom(nonzero,nonzero.x,0U,8U);
      Gecode::dom(nonzero,nonzero.y,1U);
      Gecode::dom(nonzero,nonzero.result,1U);
      nonzero.post();
      if ((nonzero.status() == Gecode::SS_FAILED) ||
          !nonzero.x.assigned() || (nonzero.x.val() != 8U))
        return false;

      ProductSpace unchanged(4,7,10);
      Gecode::dom(unchanged,unchanged.x,2U);
      Gecode::dom(unchanged,unchanged.y,3U);
      unchanged.post();
      return (unchanged.status() != Gecode::SS_FAILED) &&
        unchanged.result.assigned() && (unchanged.result.val() == 6U) &&
        (Gecode::PropagatorGroup::all.size(unchanged) == 0);
    }

    static bool contracts_and_aliases(void) {
      ProductSpace positive(4,-2,4);
      positive.post();
      if ((positive.status() == Gecode::SS_FAILED) ||
          (positive.modulus.min() != 1))
        return false;

      ProductSpace invalid(4,-2,0);
      invalid.post();
      if (invalid.status() != Gecode::SS_FAILED)
        return false;

      ProductSpace failed(4,7,7);
      Gecode::dom(failed,failed.result,7U);
      failed.post();
      if (failed.status() != Gecode::SS_FAILED)
        return false;

      ProductSpace zero(4,2,8);
      Gecode::dom(zero,zero.x,0U);
      zero.post();
      if ((zero.status() == Gecode::SS_FAILED) ||
          !zero.result.assigned() || (zero.result.val() != 0U) ||
          (Gecode::PropagatorGroup::all.size(zero) != 0))
        return false;

      ProductSpace one(4,1,1);
      one.post();
      if ((one.status() == Gecode::SS_FAILED) ||
          !one.result.assigned() || (one.result.val() != 0U) ||
          (Gecode::PropagatorGroup::all.size(one) != 0))
        return false;

      ProductSpace alias(4,5,5);
      Gecode::dom(alias,alias.x,2U);
      Gecode::dom(alias,alias.y,1U);
      Gecode::product_mod(alias,alias.x,alias.y,alias.modulus,alias.x);
      if ((alias.status() == Gecode::SS_FAILED) ||
          (Gecode::PropagatorGroup::all.size(alias) != 0))
        return false;

      try {
        ProductSpace mismatch;
        Gecode::WordVar other(mismatch,3);
        Gecode::product_mod(mismatch,mismatch.x,other,mismatch.modulus,
                            mismatch.result);
        return false;
      } catch (const Gecode::Word::WidthMismatch&) {}
      return true;
    }

    static bool clone_and_recomputation(void) {
      ProductSpace source(4,2,9);
      source.post();
      if ((source.status() == Gecode::SS_FAILED) ||
          (Gecode::PropagatorGroup::all.size(source) != 1))
        return false;
      ProductSpace* clone=static_cast<ProductSpace*>(source.clone());
      Gecode::dom(*clone,clone->x,15U);
      Gecode::dom(*clone,clone->y,15U);
      Gecode::rel(*clone,clone->modulus,Gecode::IRT_EQ,7);
      const bool clone_ok=(clone->status() != Gecode::SS_FAILED) &&
        clone->result.assigned() && (clone->result.val() == 1U) &&
        !source.result.assigned();
      delete clone;
      if (!clone_ok)
        return false;

      using namespace Gecode;
      class SearchSpace : public Space {
      public:
        WordVar x;
        WordVar y;
        IntVar modulus;
        WordVar result;
        SearchSpace(void)
          : x(*this,2), y(*this,2), modulus(*this,1,3), result(*this,2) {
          product_mod(*this,x,y,modulus,result);
          branch(*this,modulus,INT_VAL_MIN());
          WordVarArgs words={x,y};
          branch(*this,words,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
        }
        SearchSpace(SearchSpace& s) : Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          modulus.update(*this,s.modulus);
          result.update(*this,s.result);
        }
        virtual Space* copy(void) { return new SearchSpace(*this); }
      };

      SearchSpace* root=new SearchSpace;
      Search::Options options;
      options.c_d=8;
      options.a_d=64;
      DFS<SearchSpace> dfs(root,options);
      delete root;
      unsigned int solutions=0;
      while (SearchSpace* solution=dfs.next()) {
        const bool ok=solution->x.assigned() && solution->y.assigned() &&
          solution->modulus.assigned() && solution->result.assigned() &&
          (solution->result.val() ==
           (solution->x.val()*solution->y.val()) %
           static_cast<WordValue>(solution->modulus.val())) &&
          (PropagatorGroup::all.size(*solution) == 0);
        delete solution;
        if (!ok)
          return false;
        solutions++;
      }
      return solutions == 48;
    }

  public:
    Lifecycle(void) : Base("Word::ProductMod::Lifecycle") {}
    virtual bool run(void) {
      return assigned() && partial() && contracts_and_aliases() &&
        clone_and_recomputation();
    }
  };

  Lifecycle lifecycle;

}}}

// STATISTICS: test-word
