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

#include <algorithm>
#include <memory>
#include <set>
#include <utility>

namespace Test { namespace Word {

  /// Tests for channel constraints
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
          NumericSpace(WordInterpretation interpretation, int minimum, int maximum)
            : x(*this,4), y(*this,minimum,maximum) {
            channel(*this,x,y,interpretation);
          }
          NumericSpace(NumericSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new NumericSpace(*this); }
        };

        NumericSpace u(WI_UNSIGNED,8,11);
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

        NumericSpace s(WI_SIGNED,-4,-1);
        if ((s.status() == SS_FAILED) || (s.x.lo() != 12U) ||
            (s.x.hi() != 15U) || (s.y.min() != -4) || (s.y.max() != -1))
          return false;

        class Wide : public Space {
        public:
          WordVar x;
          IntVar y;
          Wide(unsigned int width, WordInterpretation interpretation, int value)
            : x(*this,width), y(*this,value,value) {
            channel(*this,x,y,interpretation);
          }
          Wide(Wide& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new Wide(*this); }
        };
        Wide unsigned64(64,WI_UNSIGNED,Gecode::Int::Limits::max);
        if ((unsigned64.status() == SS_FAILED) || !unsigned64.x.assigned() ||
            (unsigned64.x.val() !=
             static_cast<WordValue>(Gecode::Int::Limits::max)))
          return false;
        Wide signed64(64,WI_SIGNED,Gecode::Int::Limits::min);
        const WordValue signed64_value = ~WordValue(0) -
          static_cast<WordValue>(-static_cast<long long>(
            Gecode::Int::Limits::min)) + 1U;
        if ((signed64.status() == SS_FAILED) || !signed64.x.assigned() ||
            (signed64.x.val() != signed64_value))
          return false;
        Wide signed1(1,WI_SIGNED,-1);
        if ((signed1.status() == SS_FAILED) || !signed1.x.assigned() ||
            (signed1.x.val() != 1U))
          return false;

        class BoundedSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          BoundedSpace(void)
            : x(*this,4,WDT_UNSIGNED,2U,13U), y(*this,6,9) {
            channel(*this,x,y,WI_UNSIGNED);
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
            channel(*this,x,y,WI_SIGNED);
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
            channel(*this,x,y,WI_UNSIGNED);
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
          BoundedSpace invalid;
          channel(invalid,invalid.x,invalid.y,WI_SIGNED);
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
          explicit SearchSpace(WordInterpretation interpretation)
            : x(*this,4), y(*this,-8,15) {
            channel(*this,x,y,interpretation);
            branch(*this,x,WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        for (int k=0; k<2; k++) {
          const WordInterpretation interpretation = (k == 0) ? WI_UNSIGNED : WI_SIGNED;
          SearchSpace* root = new SearchSpace(interpretation);
          Search::Options options;
          options.c_d = 1;
          options.a_d = 8;
          DFS<SearchSpace> dfs(root,options);
          delete root;
          unsigned int solutions = 0;
          unsigned int checksum = 0;
          while (SearchSpace* solution = dfs.next()) {
            const int expected = (interpretation == WI_UNSIGNED) ?
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
      Lifecycle(void) : Base("Word::Channel::Lifecycle",TestTags::check()) {}
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

    /// A word and integer whose restrictions can be posted before the channel
    class EncodingSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::IntVar y;
      EncodingSpace(unsigned int width, Gecode::WordDomainType kind,
                    Gecode::WordValue minimum, Gecode::WordValue maximum,
                    const Gecode::IntSet& integers)
          : x((kind == Gecode::WDT_CUBE)
                  ? Gecode::WordVar(*this, width)
                  : Gecode::WordVar(*this, width, kind, minimum, maximum)),
            y(*this, integers) {}
      EncodingSpace(EncodingSpace& s) : Gecode::Space(s) {
        x.update(*this, s.x);
        y.update(*this, s.y);
      }
      virtual Gecode::Space*
      copy(void) {
        return new EncodingSpace(*this);
      }
    };

    /// Compare complete encoding pairs with a concrete mask/interval oracle
    class EncodingSolutions : public Base {
    public:
      EncodingSolutions(void) : Base("Word::Channel::Encoding::Solutions") {}
      virtual bool
      run(void) {
        using namespace Gecode;
        const WordDomainType kinds[] = {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED};
        for (unsigned int width = 1; width <= 4; width++) {
          const int limit = 1 << width;
          const int mask = limit - 1;
          for (WordDomainType kind : kinds)
            for (int interval = 0; interval < 2; interval++) {
              const int first = (interval == 0) ? 0 : mask / 3;
              const int last = mask - first;
              const int sign = (kind == WDT_SIGNED) ? limit / 2 : 0;
              for (PartialDomains domains(width); domains(); ++domains) {
                const Domain bits = domains.domain();
                for (int subset = 0; subset < 3; subset++) {
                  std::vector<int> integers;
                  for (int value = -1; value <= limit; value++)
                    if ((subset == 0) ||
                        ((subset == 1) && (value % 3 != 1)) ||
                        ((subset == 2) && ((value < 0) || (value >= limit))))
                      integers.push_back(value);
                  std::set<std::pair<WordValue, int>> expected;
                  for (int code = 0; code < limit; code++) {
                    const int numeric =
                        ((kind == WDT_SIGNED) && (code >= limit / 2))
                            ? code - limit
                            : code;
                    if (((static_cast<WordValue>(code) & bits.lo()) ==
                         bits.lo()) &&
                        ((static_cast<WordValue>(code) & ~bits.hi()) == 0) &&
                        ((kind == WDT_CUBE) || ((numeric >= first - sign) &&
                                                (numeric <= last - sign))) &&
                        (std::find(integers.begin(), integers.end(), code) !=
                         integers.end()))
                      expected.insert(std::make_pair(WordValue(code), code));
                  }

                  EncodingSpace root(
                      width, kind, WordValue(first ^ sign),
                      WordValue(last ^ sign),
                      IntSet(integers.data(),
                             static_cast<int>(integers.size())));
                  dom(root, root.x, bits.lo(), bits.hi());
                  channel_encoding(root, root.x, root.y);
                  const bool failed = root.status() == SS_FAILED;
                  if (failed != expected.empty())
                    return false;
                  if (!failed) {
                    if (root.y.size() != expected.size())
                      return false;
                    for (const auto& pair : expected)
                      if (!root.y.in(pair.second))
                        return false;
                  }
                  branch(root, root.x, WORD_VAL_LSB());
                  branch(root, root.y, INT_VAL_MIN());
                  Search::Options options;
                  options.c_d = 3;
                  options.a_d = 8;
                  DFS<EncodingSpace> dfs(&root, options);
                  std::set<std::pair<WordValue, int>> actual;
                  while (
                      std::unique_ptr<EncodingSpace> solution{dfs.next()}) {
                    if (!solution->x.assigned() || !solution->y.assigned() ||
                        (PropagatorGroup::all.size(*solution) != 0U))
                      return false;
                    actual.insert(
                        std::make_pair(solution->x.val(), solution->y.val()));
                  }
                  if (actual != expected)
                    return false;
                }
              }
            }
        }
        return true;
      }
    };

    /// Domain-only events, signed order, aliases, assignment, and cloning
    class EncodingLifecycle : public Base {
    private:
      static bool
      events(void) {
        using namespace Gecode;
        const int holes[] = {0, 1, 254};
        EncodingSpace interior(8, WDT_UNSIGNED, 0, 255, IntSet(holes, 3));
        channel_encoding(interior, interior.x, interior.y);
        if (interior.status() == SS_FAILED)
          return false;
        rel(interior, interior.y, IRT_NQ, 1);
        if ((interior.y.min() != 0) || (interior.y.max() != 254) ||
            (interior.status() == SS_FAILED) || (interior.x.hi() != 254U))
          return false;

        EncodingSpace bits(8, WDT_UNSIGNED, 0, 254, IntSet(0, 255));
        channel_encoding(bits, bits.x, bits.y);
        if (bits.status() == SS_FAILED)
          return false;
        channel(bits, bits.x, 0, 0);
        if ((bits.x.minimum() != 0U) || (bits.x.maximum() != 254U) ||
            (bits.status() == SS_FAILED) || (bits.y.size() != 128U))
          return false;
        for (int code = 0; code < 256; code++)
          if (bits.y.in(code) != ((code % 2) == 0))
            return false;

        EncodingSpace bounds(8, WDT_UNSIGNED, 0, 255, IntSet(0, 255));
        channel_encoding(bounds, bounds.x, bounds.y);
        if (bounds.status() == SS_FAILED)
          return false;
        if (Gecode::Word::WordView(bounds.x).narrow_rank_range(
                bounds, 1, 254) != Gecode::Word::ME_WORD_BND)
          return false;
        if ((bounds.x.lo() != 0U) || (bounds.x.hi() != 255U) ||
            (bounds.status() == SS_FAILED) || (bounds.y.min() != 1) ||
            (bounds.y.max() != 254))
          return false;

        EncodingSpace signed_bounds(8, WDT_SIGNED, 254, 1, IntSet(0, 255));
        channel_encoding(signed_bounds, signed_bounds.x, signed_bounds.y);
        if ((signed_bounds.status() == SS_FAILED) ||
            (signed_bounds.y.size() != 4U) || !signed_bounds.y.in(0) ||
            !signed_bounds.y.in(1) || !signed_bounds.y.in(254) ||
            !signed_bounds.y.in(255))
          return false;
        rel(signed_bounds, signed_bounds.y, IRT_NQ, 254);
        rel(signed_bounds, signed_bounds.y, IRT_NQ, 1);
        return (signed_bounds.status() != SS_FAILED) &&
               (signed_bounds.x.minimum() == 255U) &&
               (signed_bounds.x.maximum() == 0U) &&
               (signed_bounds.y.size() == 2U);
      }

      static bool
      assignment_and_failure(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED})
          for (int from_word = 0; from_word < 2; from_word++)
            for (int delayed = 0; delayed < 2; delayed++) {
              EncodingSpace s(8, kind, (kind == WDT_SIGNED) ? 128 : 0,
                              (kind == WDT_SIGNED) ? 127 : 255,
                              IntSet(0, 255));
              if (delayed) {
                channel_encoding(s, s.x, s.y);
                if (s.status() == SS_FAILED)
                  return false;
              }
              if (from_word)
                dom(s, s.x, 193U);
              else
                rel(s, s.y, IRT_EQ, 193);
              if (!delayed)
                channel_encoding(s, s.x, s.y);
              if ((s.status() == SS_FAILED) || !s.x.assigned() ||
                  !s.y.assigned() || (s.x.val() != 193U) ||
                  (s.y.val() != 193) || (PropagatorGroup::all.size(s) != 0U))
                return false;
            }
        EncodingSpace empty(8, WDT_CUBE, 0, 255, IntSet(1, 1));
        channel(empty, empty.x, 0, 0);
        channel_encoding(empty, empty.x, empty.y);
        if (empty.status() != SS_FAILED)
          return false;
        const int holes[] = {0, 3, 5};
        EncodingSpace delayed_empty(8, WDT_CUBE, 0, 255, IntSet(holes, 3));
        channel_encoding(delayed_empty, delayed_empty.x, delayed_empty.y);
        if (delayed_empty.status() == SS_FAILED)
          return false;
        dom(delayed_empty, delayed_empty.x, 1U);
        if (delayed_empty.status() != SS_FAILED)
          return false;
        for (unsigned int width : {9U, 64U}) {
          EncodingSpace invalid(width, WDT_CUBE, 0, 0, IntSet(0, 255));
          try {
            channel_encoding(invalid, invalid.x, invalid.y);
            return false;
          } catch (const Gecode::Word::OutOfLimits&) {
          }
          invalid.fail();
          try {
            channel_encoding(invalid, invalid.x, invalid.y);
            return false;
          } catch (const Gecode::Word::OutOfLimits&) {
          }
        }
        return true;
      }

      static bool
      aliases_and_clone(void) {
        using namespace Gecode;
        const int holes[] = {0, 1, 254};
        EncodingSpace source(8, WDT_UNSIGNED, 0, 255, IntSet(holes, 3));
        WordVar alias = source.x;
        IntVar shared = source.y;
        WordVar other(source, 8);
        channel_encoding(source, source.x, source.y);
        channel_encoding(source, alias, shared);
        channel_encoding(source, other, shared);
        if (source.status() == SS_FAILED)
          return false;
        std::unique_ptr<EncodingSpace> clone{
            static_cast<EncodingSpace*>(source.clone())};
        rel(*clone, clone->y, IRT_NQ, 1);
        if ((clone->status() == SS_FAILED) || (clone->x.hi() != 254U) ||
            !source.y.in(1) || (source.x.hi() != 255U))
          return false;
        rel(source, shared, IRT_EQ, 1);
        if ((source.status() == SS_FAILED) || !alias.assigned() ||
            (alias.val() != 1U) || !other.assigned() || (other.val() != 1U) ||
            (PropagatorGroup::all.size(source) != 0U) ||
            (clone->y.size() != 2U))
          return false;
        dom(*clone, clone->x, 254U);
        return (clone->status() != SS_FAILED) && clone->y.assigned() &&
               (clone->y.val() == 254) &&
               (PropagatorGroup::all.size(*clone) == 0U);
      }

    public:
      EncodingLifecycle(void) : Base("Word::Channel::Encoding::Lifecycle") {}
      virtual bool
      run(void) {
        return events() && assignment_and_failure() && aliases_and_clone();
      }
    };

    EncodingSolutions encoding_solutions;
    EncodingLifecycle encoding_lifecycle;

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
