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

#ifndef GECODE_WORD_VAR_IMP_HPP
#define GECODE_WORD_VAR_IMP_HPP

namespace Gecode { namespace Word {
  class WordVarImp;
  class BoundedWordVarImp;
  class ValueSetWordVarImp;

  /// Known-one and may-be-one masks in one representation
  struct WordCube {
    WordValue lo;
    WordValue hi;
  };

  /// Endpoints in a fixed word rank order
  struct WordRankInterval {
    WordValue minimum;
    WordValue maximum;
  };

  /// Encodings first to last, with first <= last as unsigned values
  struct WordEncodingRange {
    WordValue first;
    WordValue last;
  };

  /// Cube and ranked interval for a fixed width and ordering
  struct WordDomain {
    WordCube cube;
    WordRankInterval interval;
  };

  /// Admitted endpoints; the interval is usable only when is_admitted is true
  struct WordIntervalResult {
    bool is_admitted;
    WordRankInterval interval;
  };

  /// Canonical intersection; the domain is usable only when is_consistent is true
  struct WordDomainResult {
    bool is_consistent;
    WordDomain domain;
  };

  /// Largest width of a word that can carry an inline value set
  const unsigned int max_value_set_width = 8;

  /** \brief Set of the values of a word of width at most eight
   *
   * Value v is bit v % 64 of blocks[v / 64]. Bits for values of
   * \f$2^{width}\f$ and above, and blocks past the width's block count, are
   * zero.
   */
  struct WordValueSet {
    WordValue blocks[4];
  };

  /// Validated allocation domain, with ranked endpoints when bounded
  struct PreparedWordDomain {
    unsigned int width;
    WordDomainType domain_type;
    WordValue lo;
    WordValue hi;
    WordValue rank_minimum;
    WordValue rank_maximum;
  };

  /// Delta recording bits newly fixed to zero and one
  class WordDelta : public Delta {
    friend class WordVarImp;
  public:
    WordDelta(void);
    WordDelta(WordValue zero, WordValue one,
              WordDomainType domain_type=WDT_CUBE,
              WordValue old_minimum=0, WordValue old_maximum=0,
              WordValue new_minimum=0, WordValue new_maximum=0);
    WordValue zero(void) const;
    WordValue one(void) const;
    WordDomainType domain_type(void) const;
    /// Return the old internal order-rank minimum
    WordValue old_minimum(void) const;
    /// Return the old internal order-rank maximum
    WordValue old_maximum(void) const;
    /// Return the new internal order-rank minimum
    WordValue new_minimum(void) const;
    /// Return the new internal order-rank maximum
    WordValue new_maximum(void) const;
  private:
    WordValue _zero;
    WordValue _one;
    WordValue _old_minimum;
    WordValue _old_maximum;
    WordValue _new_minimum;
    WordValue _new_maximum;
    WordDomainType _domain_type;
  };
}}

#include <gecode/word/var-imp/delta.hpp>

namespace Gecode { namespace Word {
  /// Own word cube state and publish its changes to subscribers
  class WordVarImp : public WordVarImpBase {
    friend class BoundedWordVarImp;
  public:
    WordVarImp(Space& home, unsigned int width, WordValue lo, WordValue hi);

    unsigned int width(void) const;
    WordValue mask(void) const;
    WordValue lo(void) const;
    WordValue hi(void) const;
    WordDomainType domain_type(void) const;
    bool bounded(void) const;
    /// Test whether the domain is an inline value set (see ValueSetWordVarImp)
    bool has_value_set(void) const;
    /** \brief Test whether a tell may narrow the domain beyond its request
     *
     * A cube tell on a word without bounds or value set leaves exactly the
     * requested cube. Bounds and value sets close the cube further, so a
     * propagator must feed such a word's domain back before it reports a
     * fixpoint.
     */
    bool may_close_beyond_request(void) const;
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue minimum(void) const;
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue maximum(void) const;
    WordValue unknown(void) const;
    WordValue val(void) const;
    unsigned int unknown_size(void) const;
    bool assigned(void) const;
    bool in(WordValue n) const;
    /** \brief Return the exact set of values of a word of width at most eight
     *
     * Without an inline value set, the result is the values of the cube that
     * lie in the ranked interval of a bounded word.
     */
    WordValueSet values(void) const;

    ModEvent narrow(Space& home, WordValue lo, WordValue hi);
    ModEvent eq(Space& home, WordValue value);
    /** \brief Remove \a value from the domain
     *
     * The removal is exact for a word with an inline value set. Otherwise the
     * domain changes only where cube and bounds can express the removal: an
     * assigned word fails, a bounded word loses \a value when it is a ranked
     * endpoint, and a cube with one unknown bit is assigned the other value.
     * An inner value of a word without a value set stays in the domain.
     */
    ModEvent nq(Space& home, WordValue value);
    /** \brief Intersect the domain of a word of width at most eight with \a set
     *
     * The intersection is exact for a word with an inline value set. Otherwise
     * the word narrows to the cube and ranked interval of the intersection,
     * the tightest domain that cube and bounds can express.
     */
    GECODE_WORD_EXPORT ModEvent narrow_values(Space& home,
                                              const WordValueSet& set);

    GECODE_WORD_EXPORT void subscribe(Space& home, Propagator& p,
                                      PropCond pc, bool is_scheduled=true);
    GECODE_WORD_EXPORT void reschedule(Space& home, Propagator& p,
                                       PropCond pc);
    GECODE_WORD_EXPORT void subscribe(Space& home, Advisor& a,
                                      bool is_run_on_failure);

    static ModEventDelta med(ModEvent me);
    static WordValue zero(const Delta& d);
    static WordValue one(const Delta& d);
    static WordDomainType domain_type(const Delta& d);
    static WordValue old_minimum(const Delta& d);
    static WordValue old_maximum(const Delta& d);
    static WordValue new_minimum(const Delta& d);
    static WordValue new_maximum(const Delta& d);

    WordVarImp* copy(Space& home);
  protected:
    /// Width; stored in one byte so that the value-set flag fits beside it
    std::uint8_t _width;
    /// Whether this is a ValueSetWordVarImp
    bool _has_value_set;
    WordDomainType _domain_type;
    WordValue _lo;
    WordValue _hi;
    WordVarImp(Space& home, WordVarImp& x);
    WordVarImp(Space& home, unsigned int width, WordValue lo, WordValue hi,
               WordDomainType domain_type);
  private:
    GECODE_WORD_EXPORT WordVarImp* perform_copy(Space& home);
  };

  /// Word implementation with a narrowing interval in a fixed rank order
  class BoundedWordVarImp : public WordVarImp {
    friend class WordVarImp;
    friend class ValueSetWordVarImp;
  public:
    BoundedWordVarImp(Space& home, const PreparedWordDomain& domain);

    /// Return the internal order-rank minimum
    WordValue minimum(void) const;
    /// Return the internal order-rank maximum
    WordValue maximum(void) const;
    WordValue rank(WordValue value) const;

    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum);
    ModEvent narrow_range(Space& home, WordValue minimum,
                          WordValue maximum);
  protected:
    WordValue _minimum;
    WordValue _maximum;
    BoundedWordVarImp(Space& home, BoundedWordVarImp& x);
  private:
    /** \brief Close \a intersection, which differs from the domain, and
     * publish it
     *
     * It runs only when a tell changes the domain, so it is out of line.
     * Inlined into every tell, the closure and the notification make
     * unoptimized frames large enough to overflow the stack of a thread that
     * posts deeply nested expressions.
     */
    GECODE_WORD_EXPORT ModEvent close_intersection(
      Space& home, const WordDomain& intersection);
  };

  /** \brief Word implementation whose domain is an inline value set
   *
   * Words of width at most eight that are created with WVS_EXACT use this
   * class, whatever their domain type. The set has at most 256 values and is
   * stored inline. It obeys this invariant after every tell:
   *  - the set is not empty,
   *  - the cube is the bit hull of the set, and
   *  - for a bounded word, the ranked interval spans exactly the least and
   *    greatest rank of a value in the set.
   *
   * The invariant implies the closure of cube and interval that
   * BoundedWordVarImp keeps, so code that reads only cube and bounds stays
   * correct. A cube word leaves the inherited interval unused. Every tell
   * filters the set and then derives cube and interval from it; a change
   * that leaves cube and interval unchanged reports ME_WORD_HOLE.
   *
   * The tells on the set are out of line. Inlined into every caller, their
   * loops over the set's blocks make unoptimized frames large enough to
   * overflow the stack of a thread that posts deeply nested expressions.
   */
  class ValueSetWordVarImp : public BoundedWordVarImp {
    friend class WordVarImp;
    friend class BoundedWordVarImp;
  public:
    ValueSetWordVarImp(Space& home, const PreparedWordDomain& domain);
  protected:
    WordValueSet _values;
    ValueSetWordVarImp(Space& home, ValueSetWordVarImp& x);
  private:
    /// Return cube and interval of the nonempty set \a values
    WordDomain domain_of(const WordValueSet& values) const;
    /// Replace the set by its subset \a next, closing cube and interval
    ModEvent replace_values(Space& home, const WordValueSet& next);
    /// Keep the values that \a cube admits
    GECODE_WORD_EXPORT ModEvent narrow_cube(Space& home, WordCube cube);
    /// Keep the values that \a cube admits and whose rank is in \a interval
    GECODE_WORD_EXPORT ModEvent narrow_domain_values(Space& home,
                                                     WordCube cube,
                                                     WordRankInterval interval);
    /// Keep the values whose rank is in \a interval
    GECODE_WORD_EXPORT ModEvent narrow_ranks(Space& home,
                                             WordRankInterval interval);
    /// Keep only \a value, which the set contains
    GECODE_WORD_EXPORT ModEvent assign_value(Space& home, WordValue value);
    /// Remove \a value
    GECODE_WORD_EXPORT ModEvent remove_value(Space& home, WordValue value);
    /// Keep the values that are also in \a set
    GECODE_WORD_EXPORT ModEvent intersect_values(Space& home,
                                                 const WordValueSet& set);
  };
}}

#include <gecode/word/var-imp/word.hpp>

// STATISTICS: word-var

#endif
