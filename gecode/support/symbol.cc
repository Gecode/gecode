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

#include "gecode/support.hh"
#include <sstream>

namespace Gecode { namespace Support {

  forceinline void* 
  Symbol::SO::operator new(size_t s) {
    return heap.alloc(s);
  }
  forceinline void
  Symbol::SO::operator delete(void* p) {
    heap.free(p);
  }
  forceinline bool
  Symbol::SO::cancel(void) { return --use_cnt == 0; }

  forceinline void
  Symbol::SO::subscribe(void) { ++use_cnt; }

  forceinline unsigned int
  Symbol::SO::size(void) const {
    return strlen(s);
  }

  forceinline char*
  Symbol::SO::strdup(const char* s) {
    unsigned int n = strlen(s)+1;
    char* d = static_cast<char*>(heap.alloc(sizeof(char)*n));
    for (unsigned int i=n; i--; )
      d[i]=s[i];
    return d;
  }

  forceinline 
  Symbol::SO::SO(const char* s0, bool copy)
    : use_cnt(0),
      s(copy ? strdup(s0) : const_cast<char*>(s0)), own(copy) {}

  forceinline int
  Symbol::SO::hash(int m) const {
    int h = 0;
    int pos = 0;
    while (s[pos] != 0) {
      h = (127 * h + s[pos++]) % m;
    }
    return h;
  }

  forceinline void
  Symbol::SO::append(SO* so0) {
    if (so0 == NULL)
      return;
    unsigned int n1 = strlen(s);
    unsigned int n2 = strlen(so0->s);
    char* d = static_cast<char*>(heap.alloc(sizeof(char)*(n1+n2+1)));
    for (unsigned int i=n1; i--; )
      d[i] = s[i];
    for (unsigned int i=n2+1; i--; )
      d[n1+i] = so0->s[i];
    if (own)
      heap.free(s);
    s = d;
    own = true;
  }

  forceinline bool
  Symbol::SO::eq(const SO* other) const {
    if (this == other)
      return true;
    if (other == NULL)
      return false;
    return (!strcmp(s, other->s));
  }

  forceinline bool
  Symbol::SO::eq(const char* other) const {
    return (!strcmp(s, other));
  }
  
  std::ostream&
  Symbol::SO::print(std::ostream& os) const {
    return os << s;
  }


  Symbol::Symbol(void) : so(NULL) {}

  Symbol::Symbol(const char* s0, bool copy)
    : so(new SO(s0, copy)) {
    so->subscribe();
  }

  Symbol::Symbol(int i) {
    std::stringstream s;
    s << i;
    so = new SO(s.str().c_str(), true);
    so->subscribe();
  }

  Symbol::Symbol(unsigned int i) {
    std::stringstream s;
    s << i;
    so = new SO(s.str().c_str(), true);
    so->subscribe();
  }

  Symbol::Symbol(const Symbol& s0) {
    so = s0.so;
    if (so)
      so->subscribe();
  }

  const Symbol&
  Symbol::operator=(const Symbol& s0) {
    if (this != &s0) {
      if (so && so->cancel()) {
        if (so->own)
          heap.free(so->s);
        delete so;
      }
      so = s0.so;
      if (so)
        so->subscribe();
    }
    return *this;
  }
  
  bool
  Symbol::empty(void) const {
    return so==NULL;
  }
  
  Symbol
  Symbol::copy(void) const {
    Symbol ret;
    if (so == NULL) {
      ret.so = NULL;
    } else {
      ret.so = new SO(so->s, true);
      ret.so->subscribe();
    }
    return ret;
  }

  Symbol&
  Symbol::operator+=(const Symbol& s0) {
    if (so == NULL) {
      so = s0.so;
      if (so)
        so->subscribe();
    } else if (s0.so != NULL) {
      so->append(s0.so);
    }
    return *this;
  }
  
  int
  Symbol::hash(int m) const {
    if (so == NULL)
      return 0;
    return so->hash(m);
  }

  bool
  Symbol::operator==(const Symbol& s0) const {
    if (so == NULL)
      return (s0.so == NULL);
    return so->eq(s0.so);
  }

  bool
  Symbol::operator==(const char* s0) const {
    if (so==NULL)
      return s0[0] == 0;
    return so->eq(s0);
  }
  
  std::ostream&
  Symbol::print(std::ostream& os) const {
    if (so) return so->print(os);
    return os;
  }

  std::string
  Symbol::toString(void) const {
    if (so) return so->s;
    return "";
  }
  
  Symbol::~Symbol(void) {
    if ((so != NULL) && so->cancel()) {
      if (so->own)
        heap.free(so->s);
      delete so;
    }
  }

}}

// STATISTICS: support-any
