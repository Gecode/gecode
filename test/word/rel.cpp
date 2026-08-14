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

#include "test/word.hh"

namespace Test { namespace Word {

  /// Tests for word relations
  namespace Rel {

    static bool
    holds(Gecode::WordRelType wrt, Gecode::WordValue x,
          Gecode::WordValue y, unsigned int width) {
      const Gecode::WordValue sign = Gecode::WordValue(1) << (width-1);
      switch (wrt) {
      case Gecode::WRT_EQ:  return x == y;
      case Gecode::WRT_NQ:  return x != y;
      case Gecode::WRT_ULQ: return x <= y;
      case Gecode::WRT_ULE: return x < y;
      case Gecode::WRT_UGQ: return x >= y;
      case Gecode::WRT_UGR: return x > y;
      case Gecode::WRT_SLQ: return (x ^ sign) <= (y ^ sign);
      case Gecode::WRT_SLE: return (x ^ sign) < (y ^ sign);
      case Gecode::WRT_SGQ: return (x ^ sign) >= (y ^ sign);
      case Gecode::WRT_SGR: return (x ^ sign) > (y ^ sign);
      default: GECODE_NEVER;
      }
      return false;
    }

    /**
     * Equality is a direct mask-intersection propagator and enforces bit
     * consistency. Disequality is direct and excludes an assigned value when
     * that exclusion is representable by the word cube. Reified disequality
     * is a strength-preserving rewrite through reified equality and a negated
     * Boolean view. Non-strict and strict ordering are direct MSB-first lo/hi
     * propagators with word-level bound/bit consistency; greater relations
     * rewrite by swapping operands, and reified strict relations rewrite as
     * negated reversed non-strict relations. Signed ordering uses a sign-bit
     * order transform and otherwise shares the same actors.
     */
    class Variable : public Test {
    private:
      Gecode::WordRelType wrt;
    public:
      Variable(Gecode::WordRelType wrt0, const std::string& name)
        : Test("Rel::Variable::"+name,2,Domain(3,0,7),true), wrt(wrt0) {}
      virtual bool solution(const Assignment& a) const {
        return holds(wrt,a[0],a[1],a.domain().width());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::rel(home,x[0],wrt,x[1]);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x,
                        Gecode::Reify r) {
        Gecode::rel(home,x[0],wrt,x[1],r);
      }
    };

    /// Relation to an explicitly-sized constant
    class Constant : public Test {
    private:
      Gecode::WordRelType wrt;
    public:
      Constant(Gecode::WordRelType wrt0, const std::string& name)
        : Test("Rel::Constant::"+name,1,Domain(3,0,7),true), wrt(wrt0) {}
      virtual bool solution(const Assignment& a) const {
        return holds(wrt,a[0],5U,a.domain().width());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::rel(home,x[0],wrt,3,5U);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x,
                        Gecode::Reify r) {
        Gecode::rel(home,x[0],wrt,3,5U,r);
      }
    };

    /// Focused alias, partial-domain, width, and cloning checks
    class Lifecycle : public Base {
    private:
      class RelSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        Gecode::BoolVar b;
        RelSpace(void) : x(*this,2,2,0,3), b(*this,0,1) {}
        RelSpace(RelSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          b.update(*this,s.b);
        }
        virtual Gecode::Space* copy(void) {
          return new RelSpace(*this);
        }
      };

      static bool partial_sound(Gecode::WordRelType wrt) {
        for (PartialAssignment p(2,1); p.has_more(); p.next()) {
          const Domain d0=p[0], d1=p[1];
          TestSpace s(2,Domain(1,0,1));
          std::vector<Domain> ds;
          ds.push_back(d0); ds.push_back(d1);
          s.narrow(ds);
          Gecode::rel(s,s.x[0],wrt,s.x[1]);
          const bool failed = s.failed();
          bool supported = false;
          Values v0(d0);
          for (; v0(); ++v0) {
            Values v1(d1);
            for (; v1(); ++v1) {
              const bool solution = holds(wrt,v0.val(),v1.val(),1);
              if (solution) {
                supported = true;
                if (failed || !s.x[0].in(v0.val()) || !s.x[1].in(v1.val()))
                  return false;
              }
            }
          }
          if (failed == supported)
            return false;
        }
        return true;
      }

      static bool alias_reified(Gecode::WordRelType wrt,
                                Gecode::ReifyMode rm, bool b) {
        RelSpace s;
        Gecode::rel(s,s.x[0],wrt,s.x[0],Gecode::Reify(s.b,rm));
        Gecode::rel(s,s.b,Gecode::IRT_EQ,b ? 1 : 0);
        const bool solution = holds(wrt,0U,0U,2);
        bool allowed = false;
        switch (rm) {
        case Gecode::RM_EQV: allowed = (b == solution); break;
        case Gecode::RM_IMP: allowed = !b || solution; break;
        case Gecode::RM_PMI: allowed = !solution || b; break;
        default: GECODE_NEVER;
        }
        return (s.status() == Gecode::SS_FAILED) != allowed;
      }

      static bool alias_resolution(Gecode::WordRelType wrt,
                                   Gecode::ReifyMode rm) {
        RelSpace s;
        Gecode::rel(s,s.x[0],wrt,s.x[0],Gecode::Reify(s.b,rm));
        if ((s.status() == Gecode::SS_FAILED) ||
            (Gecode::PropagatorGroup::all.size(s) != 0U))
          return false;
        if (wrt == Gecode::WRT_EQ)
          return (rm == Gecode::RM_IMP) ? !s.b.assigned() : s.b.one();
        return (rm == Gecode::RM_PMI) ? !s.b.assigned() : s.b.zero();
      }

      static bool alias_clone(void) {
        RelSpace eq_source;
        Gecode::rel(eq_source,eq_source.x[0],Gecode::WRT_EQ,
                    eq_source.x[0],
                    Gecode::Reify(eq_source.b,Gecode::RM_IMP));
        if ((eq_source.status() == Gecode::SS_FAILED) ||
            eq_source.b.assigned() ||
            (Gecode::PropagatorGroup::all.size(eq_source) != 0U))
          return false;
        RelSpace* eq_clone=static_cast<RelSpace*>(eq_source.clone());
        Gecode::rel(*eq_clone,eq_clone->b,Gecode::IRT_EQ,1);
        const bool eq_ok=(eq_clone->status() != Gecode::SS_FAILED) &&
          eq_clone->b.one() && !eq_source.b.assigned();
        delete eq_clone;
        if (!eq_ok)
          return false;

        RelSpace nq_source;
        Gecode::rel(nq_source,nq_source.x[0],Gecode::WRT_NQ,
                    nq_source.x[0],
                    Gecode::Reify(nq_source.b,Gecode::RM_PMI));
        if ((nq_source.status() == Gecode::SS_FAILED) ||
            nq_source.b.assigned() ||
            (Gecode::PropagatorGroup::all.size(nq_source) != 0U))
          return false;
        RelSpace* nq_clone=static_cast<RelSpace*>(nq_source.clone());
        Gecode::rel(*nq_clone,nq_clone->b,Gecode::IRT_EQ,0);
        const bool nq_ok=(nq_clone->status() != Gecode::SS_FAILED) &&
          nq_clone->b.zero() && !nq_source.b.assigned();
        delete nq_clone;
        return nq_ok;
      }

      static bool search_recomputation(Gecode::WordRelType wrt,
                                       Gecode::ReifyMode rm) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          BoolVar b;
          SearchSpace(WordRelType wrt, ReifyMode rm)
            : x(*this,2), b(*this,0,1) {
            rel(*this,x,wrt,x,Reify(b,rm));
            branch(*this,b,BOOL_VAL_MIN());
            branch(*this,x,WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            b.update(*this,s.b);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root=new SearchSpace(wrt,rm);
        Search::Options options;
        options.c_d=1;
        options.a_d=64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (SearchSpace* solution=dfs.next()) {
          const bool relation=(wrt == WRT_EQ);
          bool allowed=false;
          switch (rm) {
          case RM_EQV: allowed=(solution->b.val() == relation); break;
          case RM_IMP: allowed=!solution->b.val() || relation; break;
          case RM_PMI: allowed=!relation || solution->b.val(); break;
          default: GECODE_NEVER;
          }
          const bool ok=allowed && solution->x.assigned() &&
            solution->b.assigned() &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        const unsigned int controls=
          ((wrt == WRT_EQ) && (rm == RM_IMP)) ||
          ((wrt == WRT_NQ) && (rm == RM_PMI)) ? 2U : 1U;
        return solutions == 4U*controls;
      }

      static bool bounded_search(Gecode::WordDomainType kind0,
                                 Gecode::WordDomainType kind1,
                                 Gecode::WordRelType wrt,
                                 bool reified, Gecode::ReifyMode rm) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x, y;
          BoolVar b;
          SearchSpace(WordDomainType kind0, WordDomainType kind1,
                      WordRelType wrt,
                      bool reified, ReifyMode rm)
            : x(*this,3,kind0), y(*this,3,kind1), b(*this,0,1) {
            if (reified) {
              rel(*this,x,wrt,y,Reify(b,rm));
              branch(*this,b,BOOL_VAL_MIN());
            } else {
              rel(*this,x,wrt,y);
            }
            branch(*this,x,WORD_VAL_LSB());
            branch(*this,y,WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); b.update(*this,s.b);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root=new SearchSpace(kind0,kind1,wrt,reified,rm);
        Search::Options options;
        options.c_d=1;
        options.a_d=64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0, expected=0;
        for (WordValue xv=0; xv<8; xv++)
          for (WordValue yv=0; yv<8; yv++) {
            const bool relation=holds(wrt,xv,yv,3);
            if (!reified) {
              expected += relation ? 1U : 0U;
            } else {
              for (int control=0; control<=1; control++) {
                bool allowed=false;
                switch (rm) {
                case RM_EQV: allowed=(control == static_cast<int>(relation));
                  break;
                case RM_IMP: allowed=(control == 0) || relation; break;
                case RM_PMI: allowed=!relation || (control == 1); break;
                default: GECODE_NEVER;
                }
                expected += allowed ? 1U : 0U;
              }
            }
          }
        while (SearchSpace* solution=dfs.next()) {
          const bool relation=holds(wrt,solution->x.val(),solution->y.val(),3);
          bool allowed=relation;
          if (reified) {
            switch (rm) {
            case RM_EQV: allowed=(solution->b.val() == relation); break;
            case RM_IMP: allowed=!solution->b.val() || relation; break;
            case RM_PMI: allowed=!relation || solution->b.val(); break;
            default: GECODE_NEVER;
            }
          }
          const bool ok=allowed && solution->x.assigned() &&
            solution->y.assigned() && (!reified || solution->b.assigned()) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == expected;
      }

      static bool bounded_strength(void) {
        using namespace Gecode;
        class PairSpace : public Space {
        public:
          WordVar x, y;
          BoolVar b;
          PairSpace(WordDomainType kind, WordValue xmin, WordValue xmax,
                    WordValue ymin, WordValue ymax)
            : x(*this,4,kind,xmin,xmax), y(*this,4,kind,ymin,ymax),
              b(*this,0,1) {}
          PairSpace(PairSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); b.update(*this,s.b);
          }
          virtual Space* copy(void) { return new PairSpace(*this); }
        };

        PairSpace cycle(WDT_UNSIGNED,0,15,0,15);
        rel(cycle,cycle.x,WRT_ULE,cycle.y);
        rel(cycle,cycle.y,WRT_ULE,cycle.x);
        if (cycle.status() != SS_FAILED)
          return false;

        PairSpace alias(WDT_SIGNED,WordValue(8),WordValue(7),
                        WordValue(8),WordValue(7));
        rel(alias,alias.x,WRT_NQ,alias.x);
        if (alias.status() != SS_FAILED)
          return false;

        PairSpace equality(WDT_UNSIGNED,2,10,6,14);
        rel(equality,equality.x,WRT_EQ,equality.y);
        if ((equality.status() == SS_FAILED) ||
            (equality.x.minimum() != 6) || (equality.x.maximum() != 10) ||
            (equality.y.minimum() != 6) || (equality.y.maximum() != 10))
          return false;

        PairSpace constant(WDT_UNSIGNED,0,15,0,15);
        rel(constant,constant.x,WRT_ULQ,4,5);
        if ((constant.status() == SS_FAILED) ||
            (constant.x.maximum() != 5))
          return false;

        PairSpace re_constant(WDT_UNSIGNED,6,10,0,15);
        rel(re_constant,re_constant.x,WRT_ULQ,4,5,
            Reify(re_constant.b,RM_EQV));
        if ((re_constant.status() == SS_FAILED) || !re_constant.b.zero() ||
            (PropagatorGroup::all.size(re_constant) != 0U))
          return false;

        PairSpace eq_constant(WDT_SIGNED,WordValue(12),3,8,7);
        rel(eq_constant,eq_constant.x,WRT_EQ,4,WordValue(15),
            Reify(eq_constant.b,RM_EQV));
        rel(eq_constant,eq_constant.b,IRT_EQ,1);
        if ((eq_constant.status() == SS_FAILED) ||
            !eq_constant.x.assigned() || (eq_constant.x.val() != 15))
          return false;

        PairSpace inference(WDT_SIGNED,8,12,0,3);
        rel(inference,inference.x,WRT_SLE,inference.y,
            Reify(inference.b,RM_EQV));
        if ((inference.status() == SS_FAILED) || !inference.b.one() ||
            (PropagatorGroup::all.size(inference) != 0U))
          return false;

        class WideSpace : public Space {
        public:
          WordVar x, y;
          WideSpace(void)
            : x(*this,64,WDT_UNSIGNED,0,~WordValue(0)),
              y(*this,64,WDT_UNSIGNED,WordValue(1) << 63,
                ~WordValue(0)) {
            rel(*this,x,WRT_ULE,y);
          }
          WideSpace(WideSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new WideSpace(*this); }
        };
        WideSpace wide;
        return (wide.status() != SS_FAILED) &&
          (wide.x.maximum() == (~WordValue(0)-1));
      }

    public:
      Lifecycle(void) : Base("Word::Rel::Lifecycle") {}
      virtual bool run(void) {
        if (!partial_sound(Gecode::WRT_EQ) ||
            !partial_sound(Gecode::WRT_NQ))
          return false;
        const Gecode::WordRelType order[] = {
          Gecode::WRT_ULQ, Gecode::WRT_ULE,
          Gecode::WRT_UGQ, Gecode::WRT_UGR,
          Gecode::WRT_SLQ, Gecode::WRT_SLE,
          Gecode::WRT_SGQ, Gecode::WRT_SGR
        };
        for (unsigned int i=0; i<8; i++)
          if (!partial_sound(order[i]))
            return false;

        RelSpace eq_alias;
        Gecode::rel(eq_alias,eq_alias.x[0],Gecode::WRT_EQ,eq_alias.x[0]);
        if (eq_alias.status() == Gecode::SS_FAILED)
          return false;
        RelSpace nq_alias;
        Gecode::rel(nq_alias,nq_alias.x[0],Gecode::WRT_NQ,nq_alias.x[0]);
        if (nq_alias.status() != Gecode::SS_FAILED)
          return false;

        const Gecode::ReifyMode modes[] = {
          Gecode::RM_EQV, Gecode::RM_IMP, Gecode::RM_PMI
        };
        for (unsigned int i=0; i<3; i++)
          for (int b=0; b<=1; b++)
            if (!alias_reified(Gecode::WRT_EQ,modes[i],b != 0) ||
                !alias_reified(Gecode::WRT_NQ,modes[i],b != 0))
              return false;
        for (unsigned int i=0; i<3; i++)
          if (!alias_resolution(Gecode::WRT_EQ,modes[i]) ||
              !alias_resolution(Gecode::WRT_NQ,modes[i]) ||
              !search_recomputation(Gecode::WRT_EQ,modes[i]) ||
              !search_recomputation(Gecode::WRT_NQ,modes[i]))
            return false;
        if (!alias_clone())
          return false;

        const Gecode::WordDomainType kinds[] = {
          Gecode::WDT_UNSIGNED, Gecode::WDT_SIGNED
        };
        const Gecode::WordRelType bounded_relations[][3] = {
          {Gecode::WRT_EQ,Gecode::WRT_ULQ,Gecode::WRT_ULE},
          {Gecode::WRT_EQ,Gecode::WRT_SLQ,Gecode::WRT_SLE}
        };
        for (unsigned int k=0; k<2; k++)
          for (unsigned int i=0; i<3; i++) {
            if (!bounded_search(kinds[k],kinds[k],bounded_relations[k][i],
                                false,Gecode::RM_EQV))
              return false;
            for (unsigned int j=0; j<3; j++)
              if (!bounded_search(kinds[k],kinds[k],
                                  bounded_relations[k][i],
                                  true,modes[j]))
                return false;
          }
        const Gecode::WordRelType all_relations[] = {
          Gecode::WRT_EQ, Gecode::WRT_NQ,
          Gecode::WRT_ULQ, Gecode::WRT_ULE,
          Gecode::WRT_UGQ, Gecode::WRT_UGR,
          Gecode::WRT_SLQ, Gecode::WRT_SLE,
          Gecode::WRT_SGQ, Gecode::WRT_SGR
        };
        for (unsigned int i=0; i<10; i++) {
          if (!bounded_search(Gecode::WDT_UNSIGNED,Gecode::WDT_SIGNED,
                              all_relations[i],false,Gecode::RM_EQV))
            return false;
          for (unsigned int j=0; j<3; j++)
            if (!bounded_search(Gecode::WDT_UNSIGNED,Gecode::WDT_SIGNED,
                                all_relations[i],true,modes[j]))
              return false;
        }
        if (!bounded_strength())
          return false;
        for (unsigned int i=0; i<8; i++)
          for (unsigned int j=0; j<3; j++)
            for (int b=0; b<=1; b++)
              if (!alias_reified(order[i],modes[j],b != 0))
                return false;

        RelSpace source;
        Gecode::rel(source,source.x[0],Gecode::WRT_EQ,source.x[1],
                    Gecode::Reify(source.b,Gecode::RM_EQV));
        if (source.status() == Gecode::SS_FAILED)
          return false;
        RelSpace* clone = static_cast<RelSpace*>(source.clone());
        Gecode::rel(*clone,clone->b,Gecode::IRT_EQ,1);
        Gecode::dom(*clone,clone->x[0],2U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[1].assigned() && (clone->x[1].val() == 2U) &&
          !source.x[0].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        RelSpace ordered_source;
        Gecode::rel(ordered_source,ordered_source.x[0],Gecode::WRT_SLQ,
                    ordered_source.x[1],
                    Gecode::Reify(ordered_source.b,Gecode::RM_EQV));
        if (ordered_source.status() == Gecode::SS_FAILED)
          return false;
        RelSpace* ordered_clone =
          static_cast<RelSpace*>(ordered_source.clone());
        Gecode::rel(*ordered_clone,ordered_clone->b,Gecode::IRT_EQ,1);
        Gecode::dom(*ordered_clone,ordered_clone->x[0],3U);
        Gecode::dom(*ordered_clone,ordered_clone->x[1],0U);
        const bool ordered_clone_ok =
          ordered_clone->status() != Gecode::SS_FAILED;
        delete ordered_clone;
        if (!ordered_clone_ok)
          return false;

        try {
          RelSpace mismatch;
          Gecode::WordVar y(mismatch,3);
          Gecode::rel(mismatch,mismatch.x[0],Gecode::WRT_EQ,y);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          RelSpace mismatch;
          Gecode::rel(mismatch,mismatch.x[0],Gecode::WRT_EQ,3,0U);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}

        return true;
      }
    };

    Variable variable_eq(Gecode::WRT_EQ,"Eq");
    Variable variable_nq(Gecode::WRT_NQ,"Nq");
    Constant constant_eq(Gecode::WRT_EQ,"Eq");
    Constant constant_nq(Gecode::WRT_NQ,"Nq");
    Variable variable_ulq(Gecode::WRT_ULQ,"UnsignedLq");
    Variable variable_ule(Gecode::WRT_ULE,"UnsignedLe");
    Variable variable_ugq(Gecode::WRT_UGQ,"UnsignedGq");
    Variable variable_ugr(Gecode::WRT_UGR,"UnsignedGr");
    Variable variable_slq(Gecode::WRT_SLQ,"SignedLq");
    Variable variable_sle(Gecode::WRT_SLE,"SignedLe");
    Variable variable_sgq(Gecode::WRT_SGQ,"SignedGq");
    Variable variable_sgr(Gecode::WRT_SGR,"SignedGr");
    Constant constant_ulq(Gecode::WRT_ULQ,"UnsignedLq");
    Constant constant_ule(Gecode::WRT_ULE,"UnsignedLe");
    Constant constant_ugq(Gecode::WRT_UGQ,"UnsignedGq");
    Constant constant_ugr(Gecode::WRT_UGR,"UnsignedGr");
    Constant constant_slq(Gecode::WRT_SLQ,"SignedLq");
    Constant constant_sle(Gecode::WRT_SLE,"SignedLe");
    Constant constant_sgq(Gecode::WRT_SGQ,"SignedGq");
    Constant constant_sgr(Gecode::WRT_SGR,"SignedGr");
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
