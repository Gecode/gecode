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

  namespace Conditional {

    static Gecode::WordValue
    mask_ite(Gecode::WordValue control, Gecode::WordValue then_value,
             Gecode::WordValue else_value, Gecode::WordValue mask) {
      return ((control & then_value) | (~control & else_value)) & mask;
    }

    /**
     * The mask form is a direct per-bit truth-table actor and enforces bit
     * consistency across the control, branches, and result.
     */
    class Mask : public Test {
    public:
      Mask(void) : Test("Conditional::Mask",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == mask_ite(a[0],a[1],a[2],dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::ite(home,x[0],x[1],x[2],x[3]);
      }
    };

    /**
     * The Boolean form is a word-level decomposition through channeling and
     * sign extension into the same direct mask actor.
     */
    class Boolean : public Test {
    public:
      Boolean(void) : Test("Conditional::Boolean",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == ((a[0] & 1U) ? a[1] : a[2]);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::BoolVar control(home,0,1);
        Gecode::channel(home,x[0],0,control);
        Gecode::ite(home,control,x[1],x[2],x[3]);
      }
    };

    class Lifecycle : public Base {
    private:
      class ConditionalSpace : public Gecode::Space {
      public:
        Gecode::BoolVar control;
        Gecode::WordVarArray x;
        ConditionalSpace(void)
          : control(*this,0,1), x(*this,3,2,0,3) {}
        ConditionalSpace(ConditionalSpace& s) : Gecode::Space(s) {
          control.update(*this,s.control);
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new ConditionalSpace(*this);
        }
      };

      static bool partial_mask(void) {
        for (PartialAssignment p(4,1); p.has_more(); p.next()) {
          TestSpace s(4,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<4; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::ite(s,s.x[0],s.x[1],s.x[2],s.x[3]);
          const bool failed = s.failed();
          bool supported = false;
          bool bit_support[4][2] = {
            {false,false},{false,false},{false,false},{false,false}
          };
          for (Values c(p[0]); c(); ++c)
            for (Values t(p[1]); t(); ++t)
              for (Values e(p[2]); e(); ++e)
                for (Values r(p[3]); r(); ++r)
                  if (r.val() == mask_ite(c.val(),t.val(),e.val(),1U)) {
                    supported = true;
                    const Gecode::WordValue values[] = {
                      c.val(),t.val(),e.val(),r.val()
                    };
                    for (int i=0; i<4; i++)
                      bit_support[i][values[i]] = true;
                  }
          if (failed == supported)
            return false;
          if (!failed)
            for (int i=0; i<4; i++)
              for (int value=0; value<2; value++)
                if (s.x[i].in(static_cast<Gecode::WordValue>(value)) !=
                    bit_support[i][value])
                  return false;
        }
        return true;
      }

    public:
      Lifecycle(void) : Base("Word::Conditional::Lifecycle") {}
      virtual bool run(void) {
        if (!partial_mask())
          return false;

        ConditionalSpace fixed;
        Gecode::dom(fixed,fixed.x[0],1U);
        Gecode::dom(fixed,fixed.x[1],2U);
        Gecode::dom(fixed,fixed.x[2],1U);
        Gecode::rel(fixed,fixed.control,Gecode::IRT_EQ,1);
        Gecode::ite(fixed,fixed.control,fixed.x[0],fixed.x[1],fixed.x[2]);
        if ((fixed.status() == Gecode::SS_FAILED) ||
            !fixed.x[2].assigned() || (fixed.x[2].val() != 1U))
          return false;

        ConditionalSpace equal;
        Gecode::dom(equal,equal.x[0],1U,3U);
        Gecode::ite(equal,equal.control,equal.x[0],equal.x[0],equal.x[1]);
        if ((equal.status() == Gecode::SS_FAILED) ||
            (equal.x[1].lo() != 1U) || (equal.x[1].hi() != 3U))
          return false;

        ConditionalSpace constants;
        Gecode::dom(constants,constants.x[0],2U);
        Gecode::dom(constants,constants.x[1],1U);
        Gecode::ite(constants,constants.control,2,3U,
                    constants.x[0],constants.x[2]);
        Gecode::rel(constants,constants.control,Gecode::IRT_EQ,1);
        if ((constants.status() == Gecode::SS_FAILED) ||
            !constants.x[2].assigned() || (constants.x[2].val() != 3U))
          return false;

        ConditionalSpace alias;
        Gecode::dom(alias,alias.x[1],3U);
        Gecode::dom(alias,alias.x[2],0U);
        Gecode::ite(alias,alias.x[0],alias.x[1],alias.x[2],alias.x[0]);
        if (alias.status() == Gecode::SS_FAILED)
          return false;

        ConditionalSpace failed;
        Gecode::dom(failed,failed.x[0],2U);
        Gecode::dom(failed,failed.x[1],1U);
        Gecode::dom(failed,failed.x[2],1U);
        Gecode::rel(failed,failed.control,Gecode::IRT_EQ,1);
        Gecode::ite(failed,failed.control,
                    failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        ConditionalSpace source;
        Gecode::ite(source,source.control,
                    source.x[0],source.x[1],source.x[2]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        ConditionalSpace* clone =
          static_cast<ConditionalSpace*>(source.clone());
        Gecode::rel(*clone,clone->control,Gecode::IRT_EQ,0);
        Gecode::dom(*clone,clone->x[1],2U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 2U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          ConditionalSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::ite(mismatch,mismatch.control,
                      mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }
    };

    Mask mask;
    Boolean boolean;
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
