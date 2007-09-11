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

namespace Gecode { namespace Support {

  char*
  Symbol::SO::strdup(const char* s) {
    unsigned int n = strlen(s)+1;
    char* d = static_cast<char*>(Memory::malloc(sizeof(char)*n));
    for (unsigned int i=n; i--; )
      d[i]=s[i];
    return d;
  }

  void
  Symbol::SO::append(SO* so0) {
    if (so0 == NULL)
      return;
    unsigned int n1 = strlen(s);
    unsigned int n2 = strlen(so0->s);
    char* d = static_cast<char*>(Memory::malloc(sizeof(char)*(n1+n2+1)));
    for (unsigned int i=n1; i--; )
      d[i] = s[i];
    for (unsigned int i=n2+1; i--; )
      d[n1+i] = so0->s[i];
    if (own)
      Memory::free(s);
    s = d;
    own = true;
  }

  bool
  Symbol::SO::eq(const SO* other) const {
    if (this == other)
      return true;
    if (other == NULL)
      return false;
    return (!strcmp(s, other->s));
  }
  
  Symbol::SO*
  Symbol::SO::copy(void) const {
    return new SO(s, own);
  }

  std::ostream&
  Symbol::SO::print(std::ostream& os) const {
    return os << s;
  }

  void
  Symbol::dispose(void) {
    if (so && so->cancel()) {
      if (so->own)
        Memory::free(so->s);
      delete so;
    }
  }

}}

// STATISTICS: support-any
