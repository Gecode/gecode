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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SUPPORT_BITSET_HH__
#define __GECODE_SUPPORT_BITSET_HH__

#define GECODE_RAND_MAX

namespace Gecode { namespace Support {

  /** \brief 
   *
   * Requires \code #include "gecode/support/bitset.hh" \endcode
   * \ingroup FuncSupport
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
    size = static_cast<int>(std::ceil(static_cast<double>(s)/(8*sizeof(Base))));
    data = static_cast<Base*>(home->alloc(size*sizeof(Base)));
    Base ival = value ? ~0 : 0;
    for (int i = size; i--; ) data[i] = ival;
  }

  template<typename Base>
  forceinline bool
  BitSet<Base>::get(unsigned int i) {
    unsigned int pos = i / (sizeof(Base)*8);
    unsigned int bit = i % (sizeof(Base)*8);
    assert(pos < size);
    return data[pos] & (1 << bit);
  }
  template<typename Base>
  forceinline void
  BitSet<Base>::set(unsigned int i, bool value) {
    unsigned int pos = i / (sizeof(Base)*8);
    unsigned int bit = i % (sizeof(Base)*8);
    assert(pos < size);
    if (value)
      data[pos] |= (1 << bit);
    else
      data[pos] &= ~(1 << bit);
  }
}}

#endif /* __GECODE_SUPPORT_BITSET_HH__ */

// STATISTICS: support-any
