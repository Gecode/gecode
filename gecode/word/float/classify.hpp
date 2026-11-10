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

#include <gecode/float-word.hh>

namespace Gecode {
  namespace {
    using TinyClassifyBase =
        MixBinaryPropagator<Word::WordView, Word::PC_WORD_DOM, Int::BoolView,
                            Int::PC_BOOL_VAL>;
    /** \brief Truth of a tiny word's membership in the encodings of a class
     *
     * Truth is decided once every remaining encoding agrees. An assigned
     * truth keeps the encodings of its outcome: exactly on a value-set word,
     * otherwise their cube and bounds, so the actor stays until the word
     * holds only encodings of that outcome.
     */
    class TinyClassify : public TinyClassifyBase {
      const Word::WordValueSet members;

      enum class Decision { failed, open, decided };
      // Filtering is idempotent, so an open decision is a fixpoint: an
      // assigned truth publishes the same set again, and an unassigned
      // truth changes nothing.
      static Decision decide(Space& home, Word::WordView x,
                             Int::BoolView truth,
                             const Word::WordValueSet& members) {
        if (truth.assigned()) {
          const Word::WordValueSet selected =
              truth.one() ? members
                          : Word::subtract_value_sets(
                                Word::full_value_set(x.width()), members);
          if (me_failed(x.narrow_values(home, selected)))
            return Decision::failed;
          return Word::is_empty_value_set(
                     Word::subtract_value_sets(x.values(), selected))
                     ? Decision::decided
                     : Decision::open;
        }
        const Word::WordValueSet values = x.values();
        if (Word::is_empty_value_set(
                Word::subtract_value_sets(values, members)))
          return me_failed(truth.one(home)) ? Decision::failed
                                            : Decision::decided;
        if (Word::is_empty_value_set(
                Word::intersect_value_sets(values, members)))
          return me_failed(truth.zero(home)) ? Decision::failed
                                             : Decision::decided;
        return Decision::open;
      }

      TinyClassify(Home home, Word::WordView x, Int::BoolView truth,
                   const Word::WordValueSet& m)
          : TinyClassifyBase(home, x, truth), members(m) {}
      TinyClassify(Space& home, TinyClassify& other)
          : TinyClassifyBase(home, other), members(other.members) {}

    public:
      /// Post unless the word or truth already decides the relation.
      static ExecStatus post(Home home, Word::WordView x, Int::BoolView truth,
                             const Word::WordValueSet& members) {
        const Decision decision = decide(home, x, truth, members);
        if (decision == Decision::failed)
          return ES_FAILED;
        if (decision == Decision::open)
          (void)new (home) TinyClassify(home, x, truth, members);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinyClassify(home, *this);
      }
      size_t dispose(Space& home) override {
        (void)TinyClassifyBase::dispose(home);
        return sizeof(*this);
      }
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        const Decision decision = decide(home, x0, x1, members);
        if (decision == Decision::failed)
          return ES_FAILED;
        return decision == Decision::decided ? home.ES_SUBSUMED(*this)
                                             : ES_FIX;
      }
    };
  } // namespace
} // namespace Gecode

// STATISTICS: word-prop
