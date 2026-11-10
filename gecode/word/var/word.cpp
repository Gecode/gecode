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

#include <gecode/word.hh>

#include <algorithm>

namespace Gecode {
  namespace {
    /// Return the set of \a values, which fit a width of at most eight
    Word::WordValueSet value_set_of(const WordValArgs& values) {
      Word::WordValueSet set = {{0,0,0,0}};
      for (WordValue value : values)
        set = Word::with_value(set,value);
      return set;
    }

    /// Post a unary table that rejects every value of \a x outside \a values
    void post_value_table(Home home, WordVar x, const WordValArgs& values) {
      WordTupleSet table(IntArgs({static_cast<int>(x.width())}));
      for (WordValue value : values)
        table.add(WordValArgs({value}));
      table.finalize();
      extensional(home,WordVarArgs({x}),table);
    }

    /** \brief Narrow \a x to the cube and rank hulls of the \a values it
     * admits
     *
     * Returns whether \a x remains a candidate for a further table, that is,
     * whether it is neither failed nor assigned.
     */
    bool narrow_to_listed_hull(Home home, Word::WordView x,
                               const WordValArgs& values) {
      WordValue lo = x.mask(), hi = 0;
      WordValue rank_minimum = x.mask(), rank_maximum = 0;
      bool is_admitted = false;
      for (WordValue value : values)
        if (x.in(value)) {
          is_admitted = true;
          lo &= value;
          hi |= value;
          const WordValue rank = Word::rank(x.domain_type(),x.width(),value);
          rank_minimum = std::min(rank_minimum,rank);
          rank_maximum = std::max(rank_maximum,rank);
        }
      if (!is_admitted) {
        home.fail();
        return false;
      }
      const ModEvent me = x.bounded()
        ? static_cast<Word::BoundedWordVarImp*>(x.varimp())->
            narrow_domain(home,lo,hi,rank_minimum,rank_maximum)
        : x.narrow(home,lo,hi);
      if (me_failed(me)) {
        home.fail();
        return false;
      }
      return !x.assigned();
    }

    /** \brief Restrict \a x to the nonempty list \a values of in-width
     * values
     *
     * A word with a value set keeps exactly its listed values. Another word
     * is narrowed to the hulls of its listed values and, unless that
     * decides it, gets a table that rejects the unlisted values.
     */
    void restrict_to_values(Home home, WordVar x, const WordValArgs& values) {
      if (x.width() <= Word::max_value_set_width) {
        Word::WordView xv(x);
        const Word::WordValueSet allowed = value_set_of(values);
        GECODE_ME_FAIL(xv.narrow_values(home,allowed));
        const bool is_entailed =
          Word::is_empty_value_set(Word::subtract_value_sets(xv.values(),allowed));
        if (is_entailed)
          return;
      } else if (!narrow_to_listed_hull(home,Word::WordView(x),values)) {
        return;
      }
      post_value_table(home,x,values);
    }
  }

  void WordVar::_init(Space& home, const Word::PreparedWordDomain& domain,
                      WordValueSetOption value_set,
                      const WordValArgs& values) {
    _init(home,domain,value_set);
    restrict_to_values(home,*this,values);
  }

  WordVar::WordVar(Space& home, unsigned int width,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    if ((width == 0U) || (width > 64U))
      throw Word::OutOfLimits("WordVar::WordVar");
    _init(home,width,0,Word::width_mask(width),
          Word::checked_value_set(width,WDT_CUBE,value_set,
                                  "WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, unsigned int width, WordBits known,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::WordCube cube =
      Word::known_cube(width,known,"WordVar::WordVar");
    _init(home,width,cube.lo,cube.hi,
          Word::checked_value_set(width,WDT_CUBE,value_set,
                                  "WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, unsigned int width,
                   const WordValArgs& values, WordDomainType domain_type,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVar::WordVar");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVar::WordVar");
    _init(home,domain,resolved,
          Word::prepare_values(width,values,"WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, unsigned int width,
                   WordDomainType domain_type,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVar::WordVar");
    _init(home,domain,Word::checked_value_set(domain.width,domain.domain_type,
                                              value_set,"WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, WordDomainType format,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::PreparedWordDomain domain =
      Word::prepare_float_domain(format,"WordVar::WordVar");
    _init(home,domain,Word::checked_value_set(domain.width,domain.domain_type,
                                              value_set,"WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, unsigned int width,
                   WordDomainType domain_type,
                   WordValue minimum, WordValue maximum,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,0,Word::width_mask(width),domain_type,
      Word::fit_to_width(width,minimum,"WordVar::WordVar"),
      Word::fit_to_width(width,maximum,"WordVar::WordVar"),
      "WordVar::WordVar");
    _init(home,domain,Word::checked_value_set(domain.width,domain.domain_type,
                                              value_set,"WordVar::WordVar"));
  }
  WordVar::WordVar(Space& home, unsigned int width,
                   WordDomainType domain_type,
                   WordValue minimum, WordValue maximum, WordBits known,
                   WordValueSetOption value_set)
    : VarImpVar<Word::WordVarImp>(nullptr) {
    const Word::WordCube cube =
      Word::known_cube(width,known,"WordVar::WordVar");
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,cube.lo,cube.hi,domain_type,
      Word::fit_to_width(width,minimum,"WordVar::WordVar"),
      Word::fit_to_width(width,maximum,"WordVar::WordVar"),
      "WordVar::WordVar");
    _init(home,domain,Word::checked_value_set(domain.width,domain.domain_type,
                                              value_set,"WordVar::WordVar"));
  }

  void dom(Home home, WordVar x, WordValue lo, WordValue hi) {
    if (((lo | hi) & ~Word::width_mask(x.width())) != 0)
      throw Word::OutOfLimits("Word::dom");
    GECODE_POST;
    Word::WordView xv(x);
    GECODE_ME_FAIL(xv.narrow(home,lo,hi));
  }
  void dom(Home home, WordVar x, WordValue value) {
    if ((value & ~Word::width_mask(x.width())) != 0)
      throw Word::OutOfLimits("Word::dom");
    GECODE_POST;
    Word::WordView xv(x);
    GECODE_ME_FAIL(xv.eq(home,value));
  }
  void dom(Home home, WordVar x, const WordValArgs& values) {
    const WordValue mask = Word::width_mask(x.width());
    for (WordValue value : values)
      if ((value & ~mask) != 0)
        throw Word::OutOfLimits("Word::dom");
    GECODE_POST;
    if (values.size() == 0) {
      home.fail();
      return;
    }
    restrict_to_values(home,x,values);
  }
  void dom(Home home, WordVar x, WordBits known) {
    const Word::WordCube cube = Word::known_cube(x.width(),known,"Word::dom");
    GECODE_POST;
    Word::WordView xv(x);
    GECODE_ME_FAIL(xv.narrow(home,cube.lo,cube.hi));
  }
  void dom(Home home, const WordVarArgs& x, WordBits known) {
    // Check every word before changing any.
    for (int i = 0; i < x.size(); i++)
      (void) Word::known_cube(x[i].width(),known,"Word::dom");
    GECODE_POST;
    for (int i = 0; i < x.size(); i++) {
      const Word::WordCube cube = Word::known_cube(x[i].width(),known,"Word::dom");
      Word::WordView xv(x[i]);
      GECODE_ME_FAIL(xv.narrow(home,cube.lo,cube.hi));
    }
  }
}

// STATISTICS: word-var
