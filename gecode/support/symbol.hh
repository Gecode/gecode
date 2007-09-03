/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *     Christian Schulte, 2007
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

#ifndef __GECODE_SUPPORT_SYMBOL_HH__
#define __GECODE_SUPPORT_SYMBOL_HH__

#include "gecode/support.hh"

#include <iostream>

namespace Gecode { namespace Support {
  
  /** \brief Immutable Symbols
    *
    */
  class Symbol {
  private:
    
    /** \brief Reference-counted Symbol objects
      *
      */
    class SO {
    public:
      /// Duplicate Symbol \a s
      GECODE_SUPPORT_EXPORT static char* strdup(const char* s);
      /// The reference count
      unsigned int use_cnt;
      /// Reference counting: cancel subscription
      bool cancel(void);
      /// Reference counting: subscribe to an SO
      void subscribe(void);

      /// The actual Symbol
      const char* s;
      /// Whether the SO owns the Symbol
      bool own;
      /// Next SO in the list
      SO* next;
      /// Tail of the list
      SO* tail;

      /// Construct from a given Symbol \a s0, which is copied if \a copy is true.
      SO(const char* s0, bool copy);
      /// Append \a s0
      void append(SO* s0);
      /// Destructor
      GECODE_SUPPORT_EXPORT ~SO(void);
      /// Compare with \a other for equality
      GECODE_SUPPORT_EXPORT bool eq(const SO* other) const;
      /// Return size of the Symbol
      unsigned int size(void) const;
      /// Return a copy
      GECODE_SUPPORT_EXPORT SO* copy(void) const;
      /// Hash value according to modulo \a M
      int hash(int m) const;
      /// Output to \a os
      GECODE_SUPPORT_EXPORT std::ostream& print(std::ostream& os) const;
    };

    /// The Symbol object
    SO* so;
  public:
    /// Construct empty Symbol
    Symbol(void);
    /// Construct Symbol from \a s0, make a copy if \a copy is true.
    Symbol(const char* s0, bool copy = false);
    /// Destructor
    ~Symbol(void);
    /// Copy constructor
    Symbol(const Symbol& s0);
    /// Assignment operator
    const Symbol& operator=(const Symbol& s0);
    
    /// Test if this Symbol is equal to \a s0
    bool operator==(const Symbol& s0) const;
    /// Return if Symbol is empty
    bool empty(void) const;
    
    /// Concatenate with \a s0, making \a s0 unusable!
    Symbol& operator+(const Symbol& s0);
    /// Make a copy
    GECODE_SUPPORT_EXPORT Symbol copy(void) const;
    /// Hash value according to modulo \a M
    int hash(int m) const;
    /// Print this Symbol to \a os
    GECODE_SUPPORT_EXPORT std::ostream& print(std::ostream& os) const;    
  };

  forceinline
  Symbol::SO::SO(const char* s0, bool copy)
    : use_cnt(0),
      s(copy ? strdup(s0) : s0), own(copy), next(NULL), tail(this) {}

  forceinline bool
  Symbol::SO::cancel(void) { return --use_cnt == 0; }

  forceinline void
  Symbol::SO::subscribe(void) { ++use_cnt; }

  forceinline void
  Symbol::SO::append(SO* so0) {
    assert(tail->next == NULL);
    tail->next = so0;
    so0->subscribe();
    tail = so0->tail;
  }

  forceinline int
  Symbol::SO::hash(int m) const {
    int h = 0;
    const SO* cur = this;
    int pos = 0;
    while (true) {
      if (cur->s[pos] == 0) {
        cur = cur->next;
        pos = 0;
        if (cur == NULL)
          break;
      }
      h = (127 * h + cur->s[pos++]) % m;
    }
    return h;
  }

  inline unsigned int
  Symbol::SO::size(void) const {
    unsigned int len = 0;
    const SO* cur = this;
    while (cur != NULL) {
      len += strlen(cur->s);
      cur = cur->next;
    }
    return len;
  }

  forceinline
  Symbol::Symbol(void) : so(NULL) {}

  forceinline
  Symbol::Symbol(const char* s0, bool copy) {
    so = new SO(s0, copy);
    so->subscribe();
  }

  forceinline
  Symbol::~Symbol(void) {
    if (so && so->cancel())
      delete so;
  }

  forceinline
  Symbol::Symbol(const Symbol& s0) {
    so = s0.so;
    if (so)
      so->subscribe();
  }

  forceinline const Symbol&
  Symbol::operator=(const Symbol& s0) {
    if (this != &s0) {
      if (so && so->cancel())
        delete so;
      so = s0.so;
      if (so)
        so->subscribe();
    }
    return *this;
  }
  
  forceinline bool
  Symbol::empty(void) const {
    return so==NULL;
  }
  
  forceinline Symbol&
  Symbol::operator+(const Symbol& s0) {
    if (so == NULL) {
      so = s0.so;
      if (so)
        so->subscribe();
    } else if (s0.so != NULL) {
      so->append(s0.so);
    }
    return *this;
  }
  
  forceinline Symbol
  Symbol::copy(void) const {
    Symbol ret;
    if (so)
      ret.so = so->copy();
    return ret;
  }

  forceinline int
  Symbol::hash(int m) const {
    if (so == NULL)
      return 0;
    return so->hash(m);
  }

  forceinline bool
  Symbol::operator==(const Symbol& s0) const {
    if (so == NULL)
      return (s0.so == NULL);
    return so->eq(s0.so);
  }
  
  forceinline std::ostream&
  Symbol::print(std::ostream& os) const {
    if (so) return so->print(os);
    return os;
  }
  
}}

forceinline
std::ostream&
operator<<(std::ostream& os, const Gecode::Support::Symbol& x) {
  return x.print(os);
}

#endif

// STATISTICS: support-any
