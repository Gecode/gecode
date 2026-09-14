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

  /// Tests for bit channel constraints
  namespace Channel {

    /**
     * \defgroup TaskTestWordChannel Channel constraints
     * \ingroup TaskTestWord
     */
    //@{

    /// Channel a word bit to a Boolean constant
    class Constant : public Test {
    private:
      unsigned int bit;
      int value;
    public:
      /// Construct and register test
      Constant(unsigned int bit0, int value0)
        : Test("Channel::Constant::"+str(static_cast<int>(bit0))+"::"+
               str(value0),
               1,Domain(3,0,7)), bit(bit0), value(value0) {}
      /// Check whether \a a satisfies the channel
      virtual bool solution(const Assignment& a) const {
        return static_cast<int>((a[0] >> bit) & 1U) == value;
      }
      /// Post the channel
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::channel(home,x[0],bit,value);
      }
    };

    /// Channel two word bits through a shared Boolean variable
    class Shared : public Test {
    private:
      unsigned int bit;
    public:
      /// Construct and register test
      explicit Shared(unsigned int bit0)
        : Test("Channel::Shared::"+str(static_cast<int>(bit0)),
               2,Domain(3,0,7)), bit(bit0) {}
      /// Check whether the selected bits are equal
      virtual bool solution(const Assignment& a) const {
        return ((a[0] >> bit) & 1U) == ((a[1] >> bit) & 1U);
      }
      /// Post both channels through the same Boolean variable
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::BoolVar b(home,0,1);
        Gecode::channel(home,x[0],bit,b);
        Gecode::channel(home,x[1],bit,b);
      }
    };

    /// Small space for focused lifecycle checks
    class ChannelSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::BoolVar b;
      /// Construct an unconstrained three-bit word and Boolean
      ChannelSpace(void) : x(*this,3), b(*this,0,1) {}
      /// Constructor for cloning \a s
      ChannelSpace(ChannelSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        b.update(*this,s.b);
      }
      /// Copy during cloning
      virtual Gecode::Space* copy(void) {
        return new ChannelSpace(*this);
      }
    };

    /// Focused checks not expressed by the assignment oracle
    class Lifecycle : public Base {
    private:
      static bool numeric_bounds(void) {
        using namespace Gecode;
        class NumericSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          NumericSpace(WordDomainType kind, int minimum, int maximum)
            : x(*this,4), y(*this,minimum,maximum) {
            channel(*this,x,y,kind);
          }
          NumericSpace(NumericSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new NumericSpace(*this); }
        };

        NumericSpace u(WDT_UNSIGNED,8,11);
        if ((u.status() == SS_FAILED) || (u.x.lo() != 8U) ||
            (u.x.hi() != 11U) || (u.y.min() != 8) || (u.y.max() != 11))
          return false;
        NumericSpace* uc = static_cast<NumericSpace*>(u.clone());
        rel(*uc,uc->y,IRT_EQ,9);
        const bool clone_ok = (uc->status() != SS_FAILED) &&
          uc->x.assigned() && (uc->x.val() == 9U) &&
          !u.x.assigned();
        delete uc;
        if (!clone_ok)
          return false;

        NumericSpace s(WDT_SIGNED,-4,-1);
        if ((s.status() == SS_FAILED) || (s.x.lo() != 12U) ||
            (s.x.hi() != 15U) || (s.y.min() != -4) || (s.y.max() != -1))
          return false;

        class Wide : public Space {
        public:
          WordVar x;
          IntVar y;
          Wide(unsigned int width, WordDomainType kind, int value)
            : x(*this,width), y(*this,value,value) {
            channel(*this,x,y,kind);
          }
          Wide(Wide& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new Wide(*this); }
        };
        Wide unsigned64(64,WDT_UNSIGNED,Gecode::Int::Limits::max);
        if ((unsigned64.status() == SS_FAILED) || !unsigned64.x.assigned() ||
            (unsigned64.x.val() !=
             static_cast<WordValue>(Gecode::Int::Limits::max)))
          return false;
        Wide signed64(64,WDT_SIGNED,Gecode::Int::Limits::min);
        const WordValue signed64_value = ~WordValue(0) -
          static_cast<WordValue>(-static_cast<long long>(
            Gecode::Int::Limits::min)) + 1U;
        if ((signed64.status() == SS_FAILED) || !signed64.x.assigned() ||
            (signed64.x.val() != signed64_value))
          return false;
        Wide signed1(1,WDT_SIGNED,-1);
        if ((signed1.status() == SS_FAILED) || !signed1.x.assigned() ||
            (signed1.x.val() != 1U))
          return false;

        class BoundedSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          BoundedSpace(void)
            : x(*this,4,WDT_UNSIGNED,2U,13U), y(*this,6,9) {
            channel(*this,x,y,WDT_UNSIGNED);
          }
          BoundedSpace(BoundedSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new BoundedSpace(*this); }
        } bounded;
        if ((bounded.status() == SS_FAILED) ||
            (bounded.x.minimum() != 6U) || (bounded.x.maximum() != 9U))
          return false;

        class BoundedSignedSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          BoundedSignedSpace(void)
            : x(*this,12,WDT_SIGNED,3096U,1000U), y(*this,-1000,1000) {
            channel(*this,x,y,WDT_SIGNED);
          }
          BoundedSignedSpace(BoundedSignedSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new BoundedSignedSpace(*this); }
        } signed_bounded;
        if (signed_bounded.status() == SS_FAILED)
          return false;
        rel(signed_bounded,signed_bounded.y,IRT_GQ,-900);
        rel(signed_bounded,signed_bounded.y,IRT_LQ,900);
        if ((signed_bounded.status() == SS_FAILED) ||
            (signed_bounded.x.minimum() != 3196U) ||
            (signed_bounded.x.maximum() != 900U))
          return false;

        class BoundedEventSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          BoundedEventSpace(void)
            : x(*this,12,WDT_UNSIGNED,1000,2000), y(*this,1000,2000) {
            channel(*this,x,y,WDT_UNSIGNED);
          }
          BoundedEventSpace(BoundedEventSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new BoundedEventSpace(*this); }
        } event;
        if (event.status() == SS_FAILED)
          return false;
        channel(event,event.x,0,0);
        StatusStatistics neutral;
        if ((event.status(neutral) == SS_FAILED) ||
            (neutral.propagate != 0U) ||
            (event.x.minimum() != 1000U) ||
            (event.x.maximum() != 2000U))
          return false;
        rel(event,event.y,IRT_GQ,1100);
        rel(event,event.y,IRT_LQ,1900);
        StatusStatistics bounds;
        if ((event.status(bounds) == SS_FAILED) ||
            (bounds.propagate != 1U) ||
            (event.x.minimum() != 1100U) ||
            (event.x.maximum() != 1900U))
          return false;

        try {
          ChannelSpace invalid;
          IntVar y(invalid,0,7);
          channel(invalid,invalid.x,y,WDT_CUBE);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          BoundedSpace invalid;
          channel(invalid,invalid.x,invalid.y,WDT_SIGNED);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        return true;
      }

      static bool numeric_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          explicit SearchSpace(WordDomainType kind)
            : x(*this,4), y(*this,-8,15) {
            channel(*this,x,y,kind);
            branch(*this,x,WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        for (int k=0; k<2; k++) {
          const WordDomainType kind = (k == 0) ? WDT_UNSIGNED : WDT_SIGNED;
          SearchSpace* root = new SearchSpace(kind);
          Search::Options options;
          options.c_d = 1;
          options.a_d = 8;
          DFS<SearchSpace> dfs(root,options);
          delete root;
          unsigned int solutions = 0;
          unsigned int checksum = 0;
          while (SearchSpace* solution = dfs.next()) {
            const int expected = (kind == WDT_UNSIGNED) ?
              static_cast<int>(solution->x.val()) :
              ((solution->x.val() & 8U) ?
               static_cast<int>(solution->x.val())-16 :
               static_cast<int>(solution->x.val()));
            const bool ok = solution->x.assigned() &&
              solution->y.assigned() && (solution->y.val() == expected) &&
              (PropagatorGroup::all.size(*solution) == 0U);
            checksum += static_cast<unsigned int>(solution->y.val()+8);
            delete solution;
            if (!ok)
              return false;
            solutions++;
          }
          const unsigned int expected_checksum = (k == 0) ? 248U : 120U;
          if ((solutions != 16U) || (checksum != expected_checksum))
            return false;
        }
        return true;
      }

      static bool delta_scheduling(void) {
        ChannelSpace incremental;
        Gecode::channel(incremental,incremental.x,2,incremental.b);
        if (incremental.status() == Gecode::SS_FAILED)
          return false;

        Gecode::dom(incremental,incremental.x,0U,6U);
        Gecode::StatusStatistics unrelated;
        if ((incremental.status(unrelated) == Gecode::SS_FAILED) ||
            (unrelated.propagate != 0U))
          return false;

        Gecode::dom(incremental,incremental.x,4U,6U);
        Gecode::StatusStatistics relevant;
        return (incremental.status(relevant) != Gecode::SS_FAILED) &&
          (relevant.propagate == 1U) && incremental.b.one() &&
          (Gecode::PropagatorGroup::all.size(incremental) == 0U);
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          BoolVar b;
          SearchSpace(void) : x(*this,10), b(*this,0,1) {
            channel(*this,x,7,b);
            branch(*this,x,WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            b.update(*this,s.b);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SearchSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() && solution->b.assigned() &&
            (solution->b.val() ==
             static_cast<int>((solution->x.val() >> 7) & 1U)) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 1024U;
      }

    public:
      /// Construct and register test
      Lifecycle(void) : Base("Word::Channel::Lifecycle") {}
      /// Run focused checks
      virtual bool run(void) {
        ChannelSpace source;
        Gecode::channel(source,source.x,2,source.b);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        ChannelSpace* clone = static_cast<ChannelSpace*>(source.clone());
        Gecode::rel(*clone,clone->b,Gecode::IRT_EQ,1);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          ((clone->x.lo() & 4U) != 0) && ((source.x.unknown() & 4U) != 0);
        delete clone;
        if (!clone_ok)
          return false;

        ChannelSpace fixed;
        Gecode::dom(fixed,fixed.x,5U);
        Gecode::channel(fixed,fixed.x,0,fixed.b);
        if ((fixed.status() == Gecode::SS_FAILED) || !fixed.b.one())
          return false;

        ChannelSpace failed;
        Gecode::dom(failed,failed.x,5U);
        Gecode::channel(failed,failed.x,0,0);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        try {
          ChannelSpace invalid;
          Gecode::channel(invalid,invalid.x,3,invalid.b);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          ChannelSpace invalid;
          Gecode::channel(invalid,invalid.x,0,2);
          return false;
        } catch (const Gecode::Int::NotZeroOne&) {}
        return numeric_bounds() && numeric_recomputation() &&
          delta_scheduling() && search_recomputation();
      }
    };

    Constant constant00(0,0);
    Constant constant01(0,1);
    Constant constant10(1,0);
    Constant constant11(1,1);
    Constant constant20(2,0);
    Constant constant21(2,1);
    Shared shared0(0);
    Shared shared1(1);
    Shared shared2(2);
    Lifecycle lifecycle;

    //@}
  }

}}

// STATISTICS: test-word
