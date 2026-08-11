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

    /**
     * Equality is a direct mask-intersection propagator and enforces bit
     * consistency. Disequality is direct and excludes an assigned value when
     * that exclusion is representable by the word cube. Reified disequality
     * is a strength-preserving rewrite through reified equality and a negated
     * Boolean view.
     */
    class Variable : public Test {
    private:
      Gecode::WordRelType wrt;
    public:
      Variable(Gecode::WordRelType wrt0, const std::string& name)
        : Test("Rel::Variable::"+name,2,Domain(3,0,7),true), wrt(wrt0) {}
      virtual bool solution(const Assignment& a) const {
        return (wrt == Gecode::WRT_EQ) ? (a[0] == a[1]) : (a[0] != a[1]);
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
        return (wrt == Gecode::WRT_EQ) ? (a[0] == 5U) : (a[0] != 5U);
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
              const bool solution = (wrt == Gecode::WRT_EQ) ?
                (v0.val() == v1.val()) : (v0.val() != v1.val());
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
        const bool solution = (wrt == Gecode::WRT_EQ);
        bool allowed = false;
        switch (rm) {
        case Gecode::RM_EQV: allowed = (b == solution); break;
        case Gecode::RM_IMP: allowed = !b || solution; break;
        case Gecode::RM_PMI: allowed = !solution || b; break;
        default: GECODE_NEVER;
        }
        return (s.status() == Gecode::SS_FAILED) != allowed;
      }

    public:
      Lifecycle(void) : Base("Word::Rel::Lifecycle") {}
      virtual bool run(void) {
        if (!partial_sound(Gecode::WRT_EQ) ||
            !partial_sound(Gecode::WRT_NQ))
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
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
