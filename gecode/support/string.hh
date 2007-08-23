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

#ifndef __GECODE_SUPPORT_STRING_HH__
#define __GECODE_SUPPORT_STRING_HH__

#include "gecode/support.hh"

#include <iostream>

namespace Gecode { namespace Support {
  
  /** \brief Immutable strings
    *
    */
  class String {
  private:
    
    /** \brief Reference-counted string objects
      *
      */
    class SO {
    public:
      /// Duplicate string \a s
      GECODE_SUPPORT_EXPORT static char* strdup(const char* s);
      /// The reference count
      unsigned int use_cnt;
      /// Reference counting: cancel subscription
      bool cancel(void);
      /// Reference counting: subscribe to an SO
      void subscribe(void);

      /// The actual string
      const char* s;
      /// Whether the SO owns the string
      bool own;
      /// Next SO in the list
      SO* next;
      /// Tail of the list
      SO* tail;

      /// Construct from a given string \a s0, which is copied if \a copy is true.
      SO(const char* s0, bool copy);
      /// Append \a s0
      void append(SO* s0);
      /// Destructor
      GECODE_SUPPORT_EXPORT ~SO(void);
      /// Compare with \a other for equality
      bool eq(const SO* other) const;
      /// Return size of the string
      unsigned int size(void) const;
      /// Return a copy
      SO* copy(void) const;
      /// Hash value according to modulo \a M
      int hash(int m) const;
      /// Output to \a os
      std::ostream& print(std::ostream& os) const;
    };

    /// The string object
    SO* so;
  public:
    /// Construct empty string
    String(void);
    /// Construct string from \a s0, make a copy if \a copy is true.
    String(const char* s0, bool copy = false);
    /// Destructor
    ~String(void);
    /// Copy constructor
    String(const String& s0);
    /// Assignment operator
    const String& operator=(const String& s0);
    
    /// Test if this string is equal to \a s0
    bool operator==(const String& s0) const;
    /// Return if string is empty
    bool empty(void) const;
    
    /// Concatenate with \a s0, making \a s0 unusable!
    String& operator+(const String& s0);
    /// Make a copy
    String copy(void) const;
    /// Hash value according to modulo \a M
    int hash(int m) const;
    /// Print this string to \a os
    std::ostream& print(std::ostream& os) const;    
  };

  forceinline
  String::SO::SO(const char* s0, bool copy)
    : s(copy ? strdup(s0) : s0), own(copy), next(NULL), tail(this) {}

  forceinline bool
  String::SO::cancel(void) { return --use_cnt == 0; }

  forceinline void
  String::SO::subscribe(void) { ++use_cnt; }

  forceinline void
  String::SO::append(SO* so0) {
    assert(tail->next == NULL);
    tail->next = so0;
    so0->subscribe();
    tail = so0->tail;
  }

  forceinline String::SO*
  String::SO::copy(void) const {
    SO* head = new SO(s, own);
    head->subscribe();
    SO* last = head;
    SO* cur = next;
    while (cur != NULL) {
      last->next = new SO(cur->s, cur->own);
      last->next->subscribe();
      last = last->next;
      cur = cur->next;
    }
    if (last != head)
      head->tail = last;
    return head;
  }

  inline int
  String::SO::hash(int m) const {
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
  String::SO::size(void) const {
    unsigned int len = 0;
    const SO* cur = this;
    while (cur != NULL) {
      len += strlen(cur->s);
      cur = cur->next;
    }
    return len;
  }

  inline bool
  String::SO::eq(const SO* other) const {
    if (this == other)
      return true;
    const SO* c = this;
    const SO* oc = other;
    int cpos = 0;
    int opos = 0;
    
    while(true) {
      if (c->s[cpos] == 0) {
        c = c->next; cpos = 0;
        if (c == NULL)
          return oc->s[opos] == 0 && oc->next == 0;
      }
      if (oc->s[opos] == 0) {
        oc = oc->next; opos = 0;
        if (oc == NULL)
          return false;
      }
      if (c->s[cpos] != oc->s[opos])
        return false;
      cpos++;
      opos++;
    }
    assert(false);
    return false;
  }
  
  forceinline std::ostream&
  String::SO::print(std::ostream& os) const {
    const SO* cur = this;
    while (cur != NULL) {
      os << cur->s;
      cur = cur->next;
    }
    return os;
  }

  forceinline
  String::String(void) : so(NULL) {}

  inline
  String::String(const char* s0, bool copy) {
    so = new SO(s0, copy);
    so->subscribe();
  }

  inline
  String::~String(void) {
    if (so && so->cancel())
      delete so;
  }

  forceinline
  String::String(const String& s0) {
    so = s0.so;
    if (so)
      so->subscribe();
  }

  forceinline const String&
  String::operator=(const String& s0) {
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
  String::empty(void) const {
    return so==NULL;
  }
  
  inline String&
  String::operator+(const String& s0) {
    if (so == NULL) {
      so = s0.so;
      if (so)
        so->subscribe();
    } else if (s0.so != NULL) {
      so->append(s0.so);
    }
    return *this;
  }
  
  inline String
  String::copy(void) const {
    String ret;
    if (so)
      ret.so = so->copy();
    return ret;
  }

  inline int
  String::hash(int m) const {
    if (so == NULL)
      return 0;
    return so->hash(m);
  }

  forceinline bool
  String::operator==(const String& s0) const {
    if (so == NULL)
      return (s0.so == NULL);
    return so->eq(s0.so);
  }
  
  forceinline std::ostream&
  String::print(std::ostream& os) const {
    if (so) return so->print(os);
    return os;
  }
  
}}

forceinline
std::ostream&
operator<<(std::ostream& os, const Gecode::Support::String& x) {
  return x.print(os);
}

#endif

// STATISTICS: support-any
