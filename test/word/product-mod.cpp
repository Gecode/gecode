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

    class ReifiedSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar y;
      Gecode::IntVar modulus;
      Gecode::WordVar result;
      Gecode::BoolVar b;
      ReifiedSpace(unsigned int width=4, int minimum=1, int maximum=10)
        : x(*this,width), y(*this,width),
          modulus(*this,minimum,maximum), result(*this,width), b(*this,0,1) {}
      ReifiedSpace(ReifiedSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
        modulus.update(*this,s.modulus);
        result.update(*this,s.result);
        b.update(*this,s.b);
      }
      virtual Gecode::Space* copy(void) {
        return new ReifiedSpace(*this);
      }
      void post(Gecode::ReifyMode rm) {
        Gecode::product_mod(*this,x,y,modulus,result,Gecode::Reify(b,rm));
      }
    };

    class BoundedSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar y;
      Gecode::IntVar modulus;
      Gecode::WordVar result;
      BoundedSpace(unsigned int width, Gecode::WordValue xmin,
                   Gecode::WordValue xmax, Gecode::WordValue ymin,
                   Gecode::WordValue ymax, int mmin, int mmax,
                   Gecode::WordValue rmin, Gecode::WordValue rmax)
        : x(*this,width,Gecode::WDT_UNSIGNED,xmin,xmax),
          y(*this,width,Gecode::WDT_UNSIGNED,ymin,ymax),
          modulus(*this,mmin,mmax),
          result(*this,width,Gecode::WDT_UNSIGNED,rmin,rmax) {}
      BoundedSpace(BoundedSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x); y.update(*this,s.y);
        modulus.update(*this,s.modulus); result.update(*this,s.result);
      }
      virtual Gecode::Space* copy(void) { return new BoundedSpace(*this); }
      void post(void) { Gecode::product_mod(*this,x,y,modulus,result); }
    };

    class BoundedReifiedSpace : public BoundedSpace {
    public:
      Gecode::BoolVar b;
      BoundedReifiedSpace(void)
        : BoundedSpace(9,10U,20U,10U,20U,509,509,0U,511U),
          b(*this,0,1) {}
      BoundedReifiedSpace(BoundedReifiedSpace& s) : BoundedSpace(s) {
        b.update(*this,s.b);
      }
      virtual Gecode::Space* copy(void) {
        return new BoundedReifiedSpace(*this);
      }
      void post(void) {
        Gecode::product_mod(*this,x,y,modulus,result,
                            Gecode::Reify(b,Gecode::RM_EQV));
      }
    };

    class ProgressionSpace : public Gecode::Space {
    public:
      Gecode::WordVar x, one, result;
      Gecode::IntVar modulus;
      ProgressionSpace(void)
        : x(*this,8,0U,240U,Gecode::WDT_UNSIGNED,16U,240U),
          one(*this,8,Gecode::WDT_UNSIGNED,1U,1U),
          result(*this,8,Gecode::WDT_UNSIGNED,0U,0U),
          modulus(*this,15,15) {
        Gecode::product_mod(*this,one,x,modulus,result);
      }
      ProgressionSpace(ProgressionSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x); one.update(*this,s.one);
        result.update(*this,s.result); modulus.update(*this,s.modulus);
      }
      virtual Gecode::Space* copy(void) { return new ProgressionSpace(*this); }
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

    static bool bounded_ranges(void) {
      ProgressionSpace progression;
      if ((progression.status() == Gecode::SS_FAILED) ||
          !progression.x.assigned() || (progression.x.val() != 240U) ||
          (Gecode::PropagatorGroup::all.size(progression) != 0))
        return false;

      BoundedSpace forward(9,10U,20U,10U,20U,509,509,0U,511U);
      forward.post();
      if ((forward.status() == Gecode::SS_FAILED) ||
          (forward.result.minimum() != 100U) ||
          (forward.result.maximum() != 400U))
        return false;

      BoundedSpace inverse(9,10U,20U,12U,12U,509,509,144U,144U);
      inverse.post();
      if ((inverse.status() == Gecode::SS_FAILED) ||
          !inverse.x.assigned() || (inverse.x.val() != 12U))
        return false;

      BoundedSpace refreshed(4,3U,4U,2U,2U,5,6,2U,2U);
      refreshed.post();
      if ((refreshed.status() == Gecode::SS_FAILED) ||
          !refreshed.x.assigned() || (refreshed.x.val() != 4U) ||
          (refreshed.modulus.min() != 5) || (refreshed.modulus.max() != 6))
        return false;

      BoundedSpace variable(9,20U,20U,20U,20U,1,509,1U,1U);
      variable.post();
      if ((variable.status() == Gecode::SS_FAILED) ||
          (variable.modulus.min() != 2) || (variable.modulus.max() != 399))
        return false;

      BoundedSpace zero(9,0U,0U,10U,20U,2,509,0U,511U);
      zero.post();
      if ((zero.status() == Gecode::SS_FAILED) ||
          !zero.result.assigned() || (zero.result.val() != 0U) ||
          (Gecode::PropagatorGroup::all.size(zero) != 0))
        return false;

      BoundedSpace boundary(9,10U,20U,10U,20U,20,20,20U,30U);
      boundary.post();
      if (boundary.status() != Gecode::SS_FAILED)
        return false;

      BoundedSpace invalid(9,10U,20U,10U,20U,-2,0,0U,511U);
      invalid.post();
      if (invalid.status() != Gecode::SS_FAILED)
        return false;

      const Gecode::WordValue maximum=~Gecode::WordValue(0);
      BoundedSpace wide(64,maximum,maximum,1U,1U,
                        2147483646,2147483646,0U,maximum);
      wide.post();
      return (wide.status() != Gecode::SS_FAILED) &&
        wide.result.assigned() && (wide.result.val() == 15U);
    }

    static bool bounded_alias_and_reification(void) {
      BoundedSpace alias(9,10U,20U,1U,1U,509,509,10U,20U);
      Gecode::product_mod(alias,alias.x,alias.y,alias.modulus,alias.x);
      if (alias.status() == Gecode::SS_FAILED)
        return false;

      BoundedReifiedSpace reified;
      reified.post();
      if ((reified.status() == Gecode::SS_FAILED) ||
          (reified.result.minimum() != 0U) ||
          (reified.result.maximum() != 511U))
        return false;
      Gecode::rel(reified,reified.b,Gecode::IRT_EQ,1);
      return (reified.status() != Gecode::SS_FAILED) &&
        (reified.result.minimum() == 100U) &&
        (reified.result.maximum() == 400U);
    }

    static bool bounded_oracle(void) {
      using namespace Gecode;
      class SearchSpace : public Space {
      public:
        WordVar x, y, result;
        IntVar modulus;
        SearchSpace(void)
          : x(*this,4,WDT_UNSIGNED,0U,3U),
            y(*this,4,WDT_UNSIGNED,0U,3U),
            result(*this,4,WDT_UNSIGNED,0U,15U), modulus(*this,1,7) {
          product_mod(*this,x,y,modulus,result);
          branch(*this,result,WORD_VAL_SPLIT_MIN());
          WordVarArgs operands={x,y};
          branch(*this,operands,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
          branch(*this,modulus,INT_VAL_MIN());
        }
        SearchSpace(SearchSpace& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y);
          result.update(*this,s.result); modulus.update(*this,s.modulus);
        }
        virtual Space* copy(void) { return new SearchSpace(*this); }
      };
      SearchSpace* root=new SearchSpace;
      Search::Options options; options.c_d=2; options.a_d=3;
      DFS<SearchSpace> dfs(root,options); delete root;
      unsigned int solutions=0;
      while (SearchSpace* solution=dfs.next()) {
        const WordValue expected=(solution->x.val()*solution->y.val()) %
          static_cast<WordValue>(solution->modulus.val());
        const bool ok=solution->result.val() == expected;
        delete solution;
        if (!ok) return false;
        solutions++;
      }
      return solutions == 112U;
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

    static bool reified_truth_rows(void) {
      const Gecode::ReifyMode modes[] = {
        Gecode::RM_EQV, Gecode::RM_IMP, Gecode::RM_PMI
      };
      for (unsigned int i=0; i<3; i++)
        for (int truth=0; truth<=1; truth++)
          for (int control=0; control<=1; control++) {
            ReifiedSpace s(3,5,5);
            Gecode::dom(s,s.x,3U);
            Gecode::dom(s,s.y,4U);
            Gecode::dom(s,s.result,truth ? 2U : 3U);
            Gecode::rel(s,s.b,Gecode::IRT_EQ,control);
            s.post(modes[i]);
            bool allowed=false;
            switch (modes[i]) {
            case Gecode::RM_EQV: allowed = control == truth; break;
            case Gecode::RM_IMP: allowed = !control || truth; break;
            case Gecode::RM_PMI: allowed = !truth || control; break;
            default: GECODE_NEVER;
            }
            if (((s.status() == Gecode::SS_FAILED) == allowed) ||
                (allowed && (Gecode::PropagatorGroup::all.size(s) != 0)))
              return false;
          }

      ReifiedSpace alias(3,5,5);
      Gecode::dom(alias,alias.x,2U);
      Gecode::dom(alias,alias.y,1U);
      Gecode::rel(alias,alias.b,Gecode::IRT_EQ,1);
      Gecode::product_mod(alias,alias.x,alias.y,alias.modulus,alias.x,
                          Gecode::Reify(alias.b,Gecode::RM_EQV));
      return (alias.status() != Gecode::SS_FAILED) &&
        (Gecode::PropagatorGroup::all.size(alias) == 0);
    }

    static bool reified_rewrites(void) {
      ReifiedSpace true_eqv(4,2,8);
      Gecode::dom(true_eqv,true_eqv.x,0U);
      Gecode::dom(true_eqv,true_eqv.result,0U);
      true_eqv.post(Gecode::RM_EQV);
      if ((true_eqv.status() == Gecode::SS_FAILED) ||
          !true_eqv.b.assigned() || (true_eqv.b.val() != 1) ||
          (Gecode::PropagatorGroup::all.size(true_eqv) != 0))
        return false;

      ReifiedSpace false_eqv(4,5,5);
      Gecode::dom(false_eqv,false_eqv.x,3U);
      Gecode::dom(false_eqv,false_eqv.y,4U);
      Gecode::dom(false_eqv,false_eqv.result,3U);
      false_eqv.post(Gecode::RM_EQV);
      if ((false_eqv.status() == Gecode::SS_FAILED) ||
          !false_eqv.b.assigned() || (false_eqv.b.val() != 0) ||
          (Gecode::PropagatorGroup::all.size(false_eqv) != 0))
        return false;

      ReifiedSpace direct(4,2,8);
      Gecode::rel(direct,direct.b,Gecode::IRT_EQ,1);
      direct.post(Gecode::RM_EQV);
      if ((direct.status() == Gecode::SS_FAILED) ||
          (Gecode::PropagatorGroup::all.size(direct) != 1) ||
          (direct.result.hi() != 7U))
        return false;
      Gecode::dom(direct,direct.x,15U);
      Gecode::dom(direct,direct.y,15U);
      Gecode::rel(direct,direct.modulus,Gecode::IRT_EQ,7);
      if ((direct.status() == Gecode::SS_FAILED) ||
          !direct.result.assigned() || (direct.result.val() != 1U) ||
          (Gecode::PropagatorGroup::all.size(direct) != 0))
        return false;

      ReifiedSpace negative(4,5,5);
      Gecode::rel(negative,negative.b,Gecode::IRT_EQ,0);
      negative.post(Gecode::RM_EQV);
      Gecode::dom(negative,negative.x,3U);
      Gecode::dom(negative,negative.y,4U);
      Gecode::dom(negative,negative.result,3U);
      if ((negative.status() == Gecode::SS_FAILED) ||
          (Gecode::PropagatorGroup::all.size(negative) != 0))
        return false;

      ReifiedSpace imp(4,2,8);
      Gecode::rel(imp,imp.b,Gecode::IRT_EQ,0);
      imp.post(Gecode::RM_IMP);
      ReifiedSpace pmi(4,2,8);
      Gecode::rel(pmi,pmi.b,Gecode::IRT_EQ,1);
      pmi.post(Gecode::RM_PMI);
      return (imp.status() != Gecode::SS_FAILED) &&
        (pmi.status() != Gecode::SS_FAILED) &&
        (Gecode::PropagatorGroup::all.size(imp) == 0) &&
        (Gecode::PropagatorGroup::all.size(pmi) == 0);
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

      ReifiedSpace reified_source(4,2,9);
      reified_source.post(Gecode::RM_EQV);
      if ((reified_source.status() == Gecode::SS_FAILED) ||
          (Gecode::PropagatorGroup::all.size(reified_source) != 1))
        return false;
      ReifiedSpace* reified_clone=
        static_cast<ReifiedSpace*>(reified_source.clone());
      Gecode::rel(*reified_clone,reified_clone->b,Gecode::IRT_EQ,1);
      Gecode::dom(*reified_clone,reified_clone->x,15U);
      Gecode::dom(*reified_clone,reified_clone->y,15U);
      Gecode::rel(*reified_clone,reified_clone->modulus,Gecode::IRT_EQ,7);
      const bool reified_clone_ok =
        (reified_clone->status() != Gecode::SS_FAILED) &&
        reified_clone->result.assigned() &&
        (reified_clone->result.val() == 1U) &&
        !reified_source.b.assigned();
      delete reified_clone;
      if (!reified_clone_ok)
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

    static bool reified_recomputation(void) {
      using namespace Gecode;
      class SearchSpace : public Space {
      public:
        WordVar x;
        WordVar y;
        IntVar modulus;
        WordVar result;
        BoolVar b;
        SearchSpace(void)
          : x(*this,2), y(*this,2), modulus(*this,1,3), result(*this,2),
            b(*this,0,1) {
          product_mod(*this,x,y,modulus,result,Reify(b,RM_EQV));
          branch(*this,modulus,INT_VAL_MIN());
          WordVarArgs words={x,y,result};
          branch(*this,words,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
        }
        SearchSpace(SearchSpace& s) : Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          modulus.update(*this,s.modulus);
          result.update(*this,s.result);
          b.update(*this,s.b);
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
        const WordValue expected=(solution->x.val()*solution->y.val()) %
          static_cast<WordValue>(solution->modulus.val());
        const bool ok=solution->b.assigned() &&
          (solution->b.val() == (solution->result.val() == expected)) &&
          (PropagatorGroup::all.size(*solution) == 0);
        delete solution;
        if (!ok)
          return false;
        solutions++;
      }
      return solutions == 192;
    }

  public:
    Lifecycle(void) : Base("Word::ProductMod::Lifecycle") {}
    virtual bool run(void) {
      return assigned() && partial() && contracts_and_aliases() &&
        bounded_ranges() && bounded_alias_and_reification() &&
        bounded_oracle() && reified_truth_rows() && reified_rewrites() &&
        clone_and_recomputation() && reified_recomputation();
    }
  };

  Lifecycle lifecycle;

}}}

// STATISTICS: test-word
