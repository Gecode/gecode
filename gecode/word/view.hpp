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

#ifndef GECODE_WORD_VIEW_HPP
#define GECODE_WORD_VIEW_HPP

namespace Gecode { namespace Word {
  /** \brief View of a word variable
   *  \ingroup TaskModelWordVars
   */
  class WordView : public VarImpView<WordVar> {
  protected:
    using VarImpView<WordVar>::x;
  public:
    WordView(void) {}
    WordView(const WordVar& y) : VarImpView<WordVar>(y.varimp()) {}
    WordView(WordVarImp* y) : VarImpView<WordVar>(y) {}

    unsigned int width(void) const { return x->width(); }
    WordValue mask(void) const { return x->mask(); }
    WordValue lo(void) const { return x->lo(); }
    WordValue hi(void) const { return x->hi(); }
    WordDomainType domain_type(void) const { return x->domain_type(); }
    bool bounded(void) const { return x->bounded(); }
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue minimum(void) const { return x->minimum(); }
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue maximum(void) const { return x->maximum(); }
    /// Return the canonical minimum in the variable's internal order
    WordValue rank_minimum(void) const {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->minimum();
    }
    /// Return the canonical maximum in the variable's internal order
    WordValue rank_maximum(void) const {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->maximum();
    }
    /// Convert internal order rank \a n to its encoded Word value
    WordValue encode_rank(WordValue n) const {
      assert(bounded());
      return Word::rank(domain_type(),width(),n);
    }
    WordValue unknown(void) const { return x->unknown(); }
    unsigned int unknown_size(void) const { return x->unknown_size(); }
    WordValue val(void) const { return x->val(); }
    bool in(WordValue n) const { return x->in(n); }
    ModEvent narrow(Space& home, WordValue lo, WordValue hi) {
      return x->narrow(home,lo,hi);
    }
    ModEvent eq(Space& home, WordValue n) { return x->eq(home,n); }
    /// Intersect the ranked interval of a bounded view
    ModEvent narrow_rank_range(Space& home, WordValue minimum,
                               WordValue maximum) {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->
        narrow_range(home,minimum,maximum);
    }
    WordValue zero(const Delta& d) const { return WordVarImp::zero(d); }
    WordValue one(const Delta& d) const { return WordVarImp::one(d); }
    WordDomainType domain_type(const Delta& d) const {
      return WordVarImp::domain_type(d);
    }
    /// Return the old internal order-rank minimum from \a d
    WordValue old_minimum(const Delta& d) const {
      return WordVarImp::old_minimum(d);
    }
    /// Return the old internal order-rank maximum from \a d
    WordValue old_maximum(const Delta& d) const {
      return WordVarImp::old_maximum(d);
    }
    /// Return the new internal order-rank minimum from \a d
    WordValue new_minimum(const Delta& d) const {
      return WordVarImp::new_minimum(d);
    }
    /// Return the new internal order-rank maximum from \a d
    WordValue new_maximum(const Delta& d) const {
      return WordVarImp::new_maximum(d);
    }
    static ModEventDelta med(ModEvent me) { return VarImpView<WordVar>::med(me); }
  };

  /// Typed view of an unsigned-bounded word
  class UnsignedWordView : public WordView {
  private:
    BoundedWordVarImp* imp(void) const {
      return static_cast<BoundedWordVarImp*>(x);
    }
  public:
    UnsignedWordView(void) {}
    UnsignedWordView(const WordVar& y) : WordView(y) {
      assert(domain_type() == WDT_UNSIGNED);
    }
    UnsignedWordView(WordVarImp* y) : WordView(y) {
      assert(domain_type() == WDT_UNSIGNED);
    }
    /// Return the canonical minimum unsigned rank
    WordValue minimum(void) const { return imp()->minimum(); }
    /// Return the canonical maximum unsigned rank
    WordValue maximum(void) const { return imp()->maximum(); }
    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum) {
      return imp()->narrow_domain(home,lo,hi,minimum,maximum);
    }
    ModEvent narrow_range(Space& home, WordValue minimum,
                          WordValue maximum) {
      return imp()->narrow_range(home,minimum,maximum);
    }
  };

  /// Typed view of a signed-bounded word
  class SignedWordView : public WordView {
  private:
    BoundedWordVarImp* imp(void) const {
      return static_cast<BoundedWordVarImp*>(x);
    }
  public:
    SignedWordView(void) {}
    SignedWordView(const WordVar& y) : WordView(y) {
      assert(domain_type() == WDT_SIGNED);
    }
    SignedWordView(WordVarImp* y) : WordView(y) {
      assert(domain_type() == WDT_SIGNED);
    }
    /// Return the canonical minimum signed-order rank
    WordValue minimum(void) const { return imp()->minimum(); }
    /// Return the canonical maximum signed-order rank
    WordValue maximum(void) const { return imp()->maximum(); }
    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum) {
      return imp()->narrow_domain(home,lo,hi,minimum,maximum);
    }
    ModEvent narrow_range(Space& home, WordValue minimum,
                          WordValue maximum) {
      return imp()->narrow_range(home,minimum,maximum);
    }
  };

  /**
   * \brief Constant view of an explicitly-sized word value
   * \ingroup TaskModelWordVars
   */
  class ConstWordView : public ConstView<WordView> {
  private:
    unsigned int _width;
    WordValue _value;
  public:
    /// Default constructor
    ConstWordView(void) : _width(1), _value(0) {}
    /// Construct the constant \a value with explicit \a width
    ConstWordView(unsigned int width, WordValue value)
      : _width(width), _value(value) {
      check_domain(width,value,value,"ConstWordView::ConstWordView");
    }

    /// Return word width
    unsigned int width(void) const { return _width; }
    /// Return width mask
    WordValue mask(void) const { return width_mask(_width); }
    /// Return known-one mask
    WordValue lo(void) const { return _value; }
    /// Return may-be-one mask
    WordValue hi(void) const { return _value; }
    WordDomainType domain_type(void) const { return WDT_CUBE; }
    bool bounded(void) const { return false; }
    /// Return unknown mask
    WordValue unknown(void) const { return 0; }
    /// Return number of unknown bits
    unsigned int unknown_size(void) const { return 0; }
    /// Return assigned value
    WordValue val(void) const { return _value; }
    /// Test whether \a value is contained
    bool in(WordValue value) const { return value == _value; }

    /// Restrict to the cube described by \a lo and \a hi
    ModEvent narrow(Space&, WordValue lo, WordValue hi) {
      const WordValue m = mask();
      if ((((lo | hi) & ~m) != 0) || ((lo & ~hi) != 0))
        return ME_WORD_FAILED;
      return (((_value & lo) == lo) && ((_value & ~hi) == 0))
        ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    /// Restrict to \a value
    ModEvent eq(Space&, WordValue value) {
      return (_value == value) ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    /// Return newly fixed-zero bits from a delta (none for constants)
    WordValue zero(const Delta&) const { return 0; }
    /// Return newly fixed-one bits from a delta (none for constants)
    WordValue one(const Delta&) const { return 0; }

    /// Update this view during cloning
    void update(Space& home, ConstWordView& y) {
      ConstView<WordView>::update(home,y);
      _width = y._width;
      _value = y._value;
    }
    /// Arbitrary stable ordering between constant views
    bool operator <(const ConstWordView& y) const {
      return (_width < y._width) ||
        ((_width == y._width) && (_value < y._value));
    }
  };
}}

// STATISTICS: word-var

#endif
