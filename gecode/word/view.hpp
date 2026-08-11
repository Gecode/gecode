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
    WordValue unknown(void) const { return x->unknown(); }
    unsigned int unknown_size(void) const { return x->unknown_size(); }
    WordValue val(void) const { return x->val(); }
    bool in(WordValue n) const { return x->in(n); }
    ModEvent narrow(Space& home, WordValue lo, WordValue hi) {
      return x->narrow(home,lo,hi);
    }
    ModEvent eq(Space& home, WordValue n) { return x->eq(home,n); }
    WordValue zero(const Delta& d) const { return WordVarImp::zero(d); }
    WordValue one(const Delta& d) const { return WordVarImp::one(d); }
    static ModEventDelta med(ModEvent me) { return VarImpView<WordVar>::med(me); }
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

#endif
