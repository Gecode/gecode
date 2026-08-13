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
        return delta_scheduling() && search_recomputation();
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
