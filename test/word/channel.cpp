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
#include <vector>

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

        // The cube of x keeps it even and y is odd below 100, so each
        // exchange moves an endpoint beyond the one sent. The channel is
        // cheap and repeats the exchange itself, in one propagation.
        class HoleWalkSpace : public Space {
        public:
          WordVar x;
          IntVar y;
          HoleWalkSpace(void)
            : x(*this,12,WDT_UNSIGNED,0U,0xffeU,known_bits(0x001,0x000)), y(*this,0,4095) {
            channel(*this,x,y,WI_UNSIGNED);
          }
          HoleWalkSpace(HoleWalkSpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          virtual Space* copy(void) { return new HoleWalkSpace(*this); }
        } walk;
        if (walk.status() == SS_FAILED)
          return false;
        IntArgs walk_values;
        for (int v=1; v<100; v+=2)
          walk_values << v;
        walk_values << 100;
        dom(walk,walk.y,IntSet(walk_values));
        StatusStatistics walk_statistics;
        if ((walk.status(walk_statistics) == SS_FAILED) ||
            (walk_statistics.propagate != 1U) ||
            !walk.x.assigned() || (walk.x.val() != 100U) ||
            !walk.y.assigned() || (walk.y.val() != 100))
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

    /** \brief A word \a x and the word \a y that link() reinterprets it as
     *
     * Clone only after link(), which initializes \a y.
     */
    class ReinterpretSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Gecode::WordVar y;
      /// Construct \a x of \a type with encoded endpoints for a bounded type
      ReinterpretSpace(unsigned int width, Gecode::WordDomainType type,
                       Gecode::WordValue minimum, Gecode::WordValue maximum,
                       Gecode::WordValueSetOption value_set)
        : x((type == Gecode::WDT_CUBE)
              ? Gecode::WordVar(*this, width, value_set)
              : Gecode::WordVar(*this, width, type, minimum, maximum,
                                value_set)) {}
      /// Construct \a x with the full domain of \a type
      ReinterpretSpace(unsigned int width, Gecode::WordDomainType type,
                       Gecode::WordValueSetOption value_set)
        : x(*this, width, type, value_set) {}
      ReinterpretSpace(ReinterpretSpace& s) : Gecode::Space(s) {
        x.update(*this, s.x);
        y.update(*this, s.y);
      }
      virtual Gecode::Space* copy(void) { return new ReinterpretSpace(*this); }
      /// Link \a y to \a x as a word of \a type
      void link(Gecode::WordDomainType type) {
        y = Gecode::reinterpret(*this, x, type);
      }
      /// Return a clone, which requires a successful status() first
      std::unique_ptr<ReinterpretSpace> cloned(void) {
        return std::unique_ptr<ReinterpretSpace>(
          static_cast<ReinterpretSpace*>(clone()));
      }
    };

    /** \brief Compare linked 4-bit words of every pair of integer domain
     * types with a bits-equal predicate
     *
     * The source word has a cube and, when bounded, a rank window; the
     * linked word is narrowed after posting by another cube and window.
     * Search must find exactly the shared values, with both words assigned
     * by propagation, and every propagation must end at a fixpoint.
     */
    class ReinterpretSolutions : public Base {
    private:
      /// A rank window and a cube
      struct Restriction {
        Gecode::WordValue rank_minimum;
        Gecode::WordValue rank_maximum;
        Gecode::WordValue lo;
        Gecode::WordValue hi;
      };

      static const unsigned int width = 4;

      /// Return the rank of \a code in the order of \a type
      static Gecode::WordValue rank_of(Gecode::WordDomainType type,
                                       Gecode::WordValue code) {
        return (type == Gecode::WDT_SIGNED) ? (code ^ 8U) : code;
      }

      /** \brief Test whether the interval of \a target lies within the hull,
       * in its order, of the codes in the interval of \a source
       */
      static bool within_hull(const Gecode::WordVar& target,
                              const Gecode::WordVar& source) {
        if (!target.bounded() || !source.bounded())
          return true;
        const Gecode::WordDomainType from = source.domain_type();
        const Gecode::WordDomainType to = target.domain_type();
        Gecode::WordValue minimum = 15, maximum = 0;
        for (Gecode::WordValue code = 0; code < 16; code++) {
          const Gecode::WordValue rank = rank_of(from, code);
          if ((rank >= rank_of(from, source.minimum())) &&
              (rank <= rank_of(from, source.maximum()))) {
            minimum = std::min(minimum, rank_of(to, code));
            maximum = std::max(maximum, rank_of(to, code));
          }
        }
        return (rank_of(to, target.minimum()) >= minimum) &&
          (rank_of(to, target.maximum()) <= maximum);
      }

      /** \brief Test whether the link in \a s is at a fixpoint
       *
       * The link then has nothing left to pass: both words have the same
       * cube, and each interval lies within the hull of the other's.
       */
      static bool at_fixpoint(const ReinterpretSpace& s) {
        return (s.x.lo() == s.y.lo()) && (s.x.hi() == s.y.hi()) &&
          within_hull(s.y, s.x) && within_hull(s.x, s.y);
      }

      /// Test whether \a code satisfies \a r in the order of \a type
      static bool admits(Gecode::WordDomainType type, const Restriction& r,
                         Gecode::WordValue code) {
        const Gecode::WordValue rank = rank_of(type, code);
        return ((code & r.lo) == r.lo) && ((code & ~r.hi) == 0) &&
          ((type == Gecode::WDT_CUBE) ||
           ((rank >= r.rank_minimum) && (rank <= r.rank_maximum)));
      }

      /// Return the rank windows to try for \a type
      static std::vector<std::pair<Gecode::WordValue, Gecode::WordValue>>
      windows(Gecode::WordDomainType type) {
        if (type == Gecode::WDT_CUBE)
          return {{0, 15}};
        return {{0, 15}, {3, 12}, {9, 15}, {2, 6}};
      }

      /// Check the link from a word of \a from to a word of \a to
      static bool check(Gecode::WordDomainType from, Gecode::WordDomainType to,
                        const Restriction& source, const Restriction& target) {
        using namespace Gecode;
        bool is_source_empty = true;
        std::set<WordValue> expected;
        for (WordValue code = 0; code < 16; code++) {
          if (admits(from, source, code))
            is_source_empty = false;
          if (admits(from, source, code) && admits(to, target, code))
            expected.insert(code);
        }

        // The signed rank map is its own inverse, so it also encodes ranks.
        ReinterpretSpace root(width, from, rank_of(from, source.rank_minimum),
                              rank_of(from, source.rank_maximum), WVS_NONE);
        dom(root, root.x, source.lo, source.hi);
        root.link(to);
        if (root.status() == SS_FAILED)
          return is_source_empty;
        if (root.y.has_value_set() || (root.y.domain_type() != to) ||
            !at_fixpoint(root))
          return false;
        dom(root, root.y, target.lo, target.hi);
        if ((to != WDT_CUBE) && !root.failed() &&
            me_failed(Gecode::Word::WordView(root.y).narrow_rank_range(
              root, target.rank_minimum, target.rank_maximum)))
          root.fail();
        if (root.status() == SS_FAILED)
          return expected.empty();
        if (!at_fixpoint(root))
          return false;
        for (WordValue code : expected)
          if (!root.x.in(code) || !root.y.in(code))
            return false;

        branch(root, root.x, WORD_VAL_LSB());
        DFS<ReinterpretSpace> dfs(&root);
        std::set<WordValue> actual;
        while (std::unique_ptr<ReinterpretSpace> solution{dfs.next()}) {
          if (!solution->y.assigned() ||
              (solution->y.val() != solution->x.val()) ||
              (PropagatorGroup::all.size(*solution) != 0U))
            return false;
          actual.insert(solution->x.val());
        }
        return actual == expected;
      }

    public:
      ReinterpretSolutions(void)
        : Base("Word::Channel::Reinterpret::Solutions") {}
      virtual bool run(void) {
        using namespace Gecode;
        const WordDomainType types[] = {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED};
        const std::pair<WordValue, WordValue> target_cubes[] = {
          {0x0, 0xf}, {0x0, 0x7}, {0x8, 0xf}, {0x1, 0xd}};
        for (WordDomainType from : types)
          for (WordDomainType to : types)
            for (PartialDomains cubes(width); cubes(); ++cubes)
              for (const auto& source_window : windows(from))
                for (const auto& target_cube : target_cubes)
                  for (const auto& target_window : windows(to)) {
                    const Domain cube = cubes.domain();
                    const Restriction source = {
                      source_window.first, source_window.second,
                      cube.lo(), cube.hi()};
                    const Restriction target = {
                      target_window.first, target_window.second,
                      target_cube.first, target_cube.second};
                    if (!check(from, to, source, target)) {
                      olog << ind(2) << "from " << from << " " << cube
                           << " ranks " << source.rank_minimum << ".."
                           << source.rank_maximum << " to " << to
                           << " cube " << target.lo << "/" << target.hi
                           << " ranks " << target.rank_minimum << ".."
                           << target.rank_maximum << std::endl;
                      return false;
                    }
                  }
        return true;
      }
    };

    /// Value-set holes and assignments pass in both directions
    class ReinterpretValueSets : public Base {
    private:
      /// Test whether \a a and \a b admit the same values (width at most 8)
      static bool same_values(const Gecode::WordVar& a,
                              const Gecode::WordVar& b) {
        for (Gecode::WordValue v = 0; v <= a.mask(); v++)
          if (a.in(v) != b.in(v))
            return false;
        return true;
      }

      /** \brief Link a value-set word of \a from and \a width to a word of
       * \a to
       *
       * The removed values 2, 3 and 5 and the assigned values 0xa and 0xc
       * lie strictly inside the cube and bounds of every pair tested, so
       * each removal is a hole-only change.
       */
      static bool holes(Gecode::WordDomainType from, unsigned int width,
                        Gecode::WordDomainType to) {
        using namespace Gecode;
        ReinterpretSpace s(width, from, WVS_EXACT);
        Gecode::Word::WordView xv(s.x);
        if (xv.nq(s, 5U) != Gecode::Word::ME_WORD_HOLE)
          return false;
        s.link(to);
        if ((s.status() == SS_FAILED) || !s.y.has_value_set() ||
            (s.y.width() != width) || (s.y.domain_type() != to) ||
            s.y.in(5U) || !same_values(s.x, s.y))
          return false;

        if (xv.nq(s, 2U) != Gecode::Word::ME_WORD_HOLE)
          return false;
        if ((s.status() == SS_FAILED) || s.y.in(2U) || !same_values(s.x, s.y))
          return false;
        Gecode::Word::WordView yv(s.y);
        if (yv.nq(s, 3U) != Gecode::Word::ME_WORD_HOLE)
          return false;
        if ((s.status() == SS_FAILED) || s.x.in(3U) || !same_values(s.x, s.y) ||
            (PropagatorGroup::all.size(s) != 1U))
          return false;

        std::unique_ptr<ReinterpretSpace> from_x = s.cloned();
        dom(*from_x, from_x->x, 0xaU);
        if ((from_x->status() == SS_FAILED) || !from_x->y.assigned() ||
            (from_x->y.val() != 0xaU) ||
            (PropagatorGroup::all.size(*from_x) != 0U))
          return false;
        std::unique_ptr<ReinterpretSpace> from_y = s.cloned();
        dom(*from_y, from_y->y, 0xcU);
        if ((from_y->status() == SS_FAILED) || !from_y->x.assigned() ||
            (from_y->x.val() != 0xcU) ||
            (PropagatorGroup::all.size(*from_y) != 0U))
          return false;
        return !s.x.assigned() && s.x.in(0xaU) && s.x.in(0xcU) &&
          s.y.in(0xaU) && s.y.in(0xcU);
      }

    public:
      ReinterpretValueSets(void)
        : Base("Word::Channel::Reinterpret::ValueSets") {}
      virtual bool run(void) {
        using namespace Gecode;
        return holes(WDT_E2M1, 4, WDT_UNSIGNED) &&
          holes(WDT_UNSIGNED, 4, WDT_E2M1) &&
          holes(WDT_SIGNED, 8, WDT_E4M3FN) &&
          holes(WDT_E4M3FN, 8, WDT_CUBE);
      }
    };

    /** \brief Without value sets, cube and float-order bounds pass in both
     * directions
     *
     * A 16-bit unsigned window from 0x10 to 0x8010 holds positive BF16
     * encodings up to 0x7fff and negative ones from -0 to 0x8010, so the
     * BF16 word ranges from 0x8010 to 0x7fff in total order.
     */
    class ReinterpretBounds : public Base {
    private:
      /// Return the unsigned window linked to a BF16 word
      static std::unique_ptr<ReinterpretSpace> linked(void) {
        using namespace Gecode;
        std::unique_ptr<ReinterpretSpace> s(
          new ReinterpretSpace(16, WDT_UNSIGNED, 0x10, 0x8010, WVS_NONE));
        s->link(WDT_BF16);
        return s;
      }

      /// The BF16 word gets the hull of the unsigned window in total order
      static bool hull(void) {
        std::unique_ptr<ReinterpretSpace> s = linked();
        return (s->status() != Gecode::SS_FAILED) && !s->y.has_value_set() &&
          (s->y.minimum() == 0x8010U) && (s->y.maximum() == 0x7fffU) &&
          (s->y.lo() == 0U) && (s->y.hi() == 0xffffU);
      }

      /// Bit-only and bound-only changes on either word pass to the other
      static bool changes(void) {
        using namespace Gecode;
        std::unique_ptr<ReinterpretSpace> s = linked();
        if (s->status() == SS_FAILED)
          return false;
        Gecode::Word::WordView xv(s->x);
        if (xv.narrow(*s, s->x.lo(), s->x.hi() & ~0x2U) !=
            Gecode::Word::ME_WORD_BITS)
          return false;
        if ((s->status() == SS_FAILED) || ((s->y.hi() & 0x2U) != 0U))
          return false;
        if (xv.narrow_rank_range(*s, 0x10, 0x8008) !=
            Gecode::Word::ME_WORD_BND)
          return false;
        if ((s->status() == SS_FAILED) || (s->y.minimum() != 0x8008U) ||
            (s->y.maximum() != 0x7ffdU))
          return false;
        // Positive BF16 values from 0x0020 to infinity, in total order.
        Gecode::Word::WordView yv(s->y);
        if (me_failed(yv.narrow_rank_range(*s, 0x8020, 0xff80)))
          return false;
        if ((s->status() == SS_FAILED) || (s->x.minimum() != 0x20U) ||
            (s->x.maximum() != 0x7f80U) || (s->x.hi() != 0x7ffdU) ||
            (s->y.minimum() != 0x20U) || (s->y.maximum() != 0x7f80U))
          return false;
        if (me_failed(yv.narrow(*s, s->y.lo() | 0x4U, s->y.hi())))
          return false;
        return (s->status() != SS_FAILED) && (s->x.lo() == 0x4U) &&
          (s->x.minimum() == 0x24U) && (s->y.minimum() == 0x24U);
      }

      /** \brief The link repeats its cheap exchange until neither word
       * changes, so narrowing one word narrows both in one propagation
       */
      static bool exchanges_to_fixpoint(void) {
        using namespace Gecode;
        std::unique_ptr<ReinterpretSpace> s = linked();
        if (s->status() == SS_FAILED)
          return false;
        // Positive BF16 values from 0x0020 to infinity, in total order.
        Gecode::Word::WordView yv(s->y);
        if (me_failed(yv.narrow_rank_range(*s, 0x8020, 0xff80)))
          return false;
        StatusStatistics statistics;
        return (s->status(statistics) != SS_FAILED) &&
          (statistics.propagate == 1U) && (s->x.minimum() == 0x20U) &&
          (s->x.maximum() == 0x7f80U);
      }

    public:
      ReinterpretBounds(void) : Base("Word::Channel::Reinterpret::Bounds") {}
      virtual bool run(void) {
        return hull() && changes() && exchanges_to_fixpoint();
      }
    };

    /// Assigned sources, clones, failed spaces and invalid calls
    class ReinterpretLifecycle : public Base {
    private:
      /// An assigned word of \a from links by assignment, without a propagator
      static bool assigned(Gecode::WordDomainType from,
                           Gecode::WordValueSetOption value_set,
                           Gecode::WordDomainType to) {
        using namespace Gecode;
        ReinterpretSpace s(8, from, value_set);
        dom(s, s.x, 0x41U);
        s.link(to);
        return (PropagatorGroup::all.size(s) == 0U) && s.y.assigned() &&
          (s.y.val() == 0x41U) && (s.y.width() == 8U) &&
          (s.y.domain_type() == to) &&
          (s.y.has_value_set() == s.x.has_value_set()) &&
          (s.status() != SS_FAILED);
      }

      /// Narrowing a clone leaves the original, which still propagates
      static bool clones(void) {
        using namespace Gecode;
        ReinterpretSpace s(16, WDT_SIGNED, WVS_NONE);
        s.link(WDT_UNSIGNED);
        if (s.status() == SS_FAILED)
          return false;
        std::unique_ptr<ReinterpretSpace> c = s.cloned();
        dom(*c, c->y, 0x1234U);
        if ((c->status() == SS_FAILED) || !c->x.assigned() ||
            (c->x.val() != 0x1234U) || s.x.assigned() ||
            (PropagatorGroup::all.size(*c) != 0U))
          return false;
        channel(s, s.y, 15, 1);
        return (s.status() != SS_FAILED) && (s.x.minimum() == 0x8000U) &&
          (s.x.maximum() == 0xffffU) && (c->x.val() == 0x1234U) &&
          (PropagatorGroup::all.size(s) == 1U);
      }

      /// A failed space returns an unlinked word of the requested type
      static bool failed(void) {
        using namespace Gecode;
        ReinterpretSpace s(4, WDT_UNSIGNED, WVS_EXACT);
        s.fail();
        s.link(WDT_E2M1);
        return (s.y.width() == 4U) && (s.y.domain_type() == WDT_E2M1) &&
          s.y.has_value_set();
      }

      /// Test that linking to \a to throws Word::OutOfLimits, posting nothing
      static bool throws(unsigned int width, Gecode::WordDomainType from,
                         Gecode::WordValueSetOption value_set,
                         Gecode::WordDomainType to) {
        using namespace Gecode;
        ReinterpretSpace s(width, from, value_set);
        try {
          s.link(to);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {
        }
        return (PropagatorGroup::all.size(s) == 0U) &&
          (s.status() != SS_FAILED);
      }

    public:
      ReinterpretLifecycle(void)
        : Base("Word::Channel::Reinterpret::Lifecycle") {}
      virtual bool run(void) {
        using namespace Gecode;
        const WordDomainType invalid = static_cast<WordDomainType>(42);
        return assigned(WDT_CUBE, WVS_NONE, WDT_SIGNED) &&
          assigned(WDT_UNSIGNED, WVS_EXACT, WDT_E4M3FN) &&
          assigned(WDT_E4M3FN, WVS_DEFAULT, WDT_UNSIGNED) &&
          assigned(WDT_SIGNED, WVS_NONE, WDT_SIGNED) && clones() &&
          failed() && throws(8, WDT_UNSIGNED, WVS_NONE, WDT_E2M1) &&
          throws(16, WDT_UNSIGNED, WVS_NONE, WDT_FP32) &&
          throws(32, WDT_FP32, WVS_NONE, WDT_BF16) &&
          throws(4, WDT_CUBE, WVS_NONE, invalid) &&
          throws(4, WDT_E2M1, WVS_DEFAULT, invalid);
      }
    };

    ReinterpretSolutions reinterpret_solutions;
    ReinterpretValueSets reinterpret_value_sets;
    ReinterpretBounds reinterpret_bounds;
    ReinterpretLifecycle reinterpret_lifecycle;

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
