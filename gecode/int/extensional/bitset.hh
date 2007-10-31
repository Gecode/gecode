/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#ifndef __GECODE_EXTENSIONAL_BITSET_HH__
#define __GECODE_EXTENSIONAL_BITSET_HH__

#include <climits>

namespace Gecode { namespace Int { namespace Extensional {

  /** \brief 
   *
   * Requires \code #include "gecode/extensional/bitset.hh" \endcode
   * \ingroup FuncExtensional
   */
  template<typename Base>
  class BitSet {
    Base* data;
    unsigned int size;
  public:
    /// Default (empty) initialization of BitSet
    BitSet(void) : data(NULL), size(0) {}

    /// BitSet with space for \a s bits. The bits are  set to \a value.
    BitSet(Space *home, int s, bool value = false) 
      : data(NULL), size(0) {
      init(home, s, value);
    }

    /// Copy BitSet \a bs.
    BitSet(Space *home, const BitSet<Base>& bs)
      : data(home->alloc(bs.size*sizeof(Base))), size(bs.size) {
      for (int i = size; i--; ) data[i] = bs.data[i];
    }
    
    /// Initialise BitSet with space for \a s bits. The bits are  set to \a value.
    void init(Space *home, int s, bool value = false);
    
    /// Access value at bit \a i.
    bool get(unsigned int i);
    /// Set value at bit \a i to \a value.
    void set(unsigned int i, bool value = true);
  };
  

  template<typename Base>
  forceinline void
  BitSet<Base>::init(Space* home, int s, bool value) {
    size = static_cast<int>(std::ceil(static_cast<double>(s)
                                      /(CHAR_BIT*sizeof(Base))));
    data = static_cast<Base*>(home->alloc(size*sizeof(Base)));
    Base ival = value ? ~0 : 0;
    for (int i = size; i--; ) data[i] = ival;
  }

  template<typename Base>
  forceinline bool
  BitSet<Base>::get(unsigned int i) {
    unsigned int pos = i / (sizeof(Base)*CHAR_BIT);
    unsigned int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    return data[pos] & ((Base)1 << bit);
  }
  template<typename Base>
  forceinline void
  BitSet<Base>::set(unsigned int i, bool value) {
    unsigned int pos = i / (sizeof(Base)*CHAR_BIT);
    unsigned int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    if (value)
      data[pos] |= ((Base)1 << bit);
    else
      data[pos] &= ~((Base)1 << bit);
  }
}}}

#endif /* __GECODE_EXTENSIONAL_BITSET_HH__ */

// STATISTICS: int-other
