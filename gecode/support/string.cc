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

#include "gecode/support/string.hh"

namespace Gecode { namespace Support {

  char*
  String::SO::strdup(const char* s) {
    unsigned int n = strlen(s);
    //    char* d = static_cast<char*>(Memory::malloc(sizeof(char)*n));
    char* d = static_cast<char*>(::malloc(sizeof(char)*n));
    for (unsigned int i=n+1; i--; )
      d[i]=s[i];
    return d;
  }

  bool
  String::SO::eq(const SO* other) const {
    if (this == other)
      return true;
    const SO* c = this;
    const SO* oc = other;
    int cpos = 0;
    int opos = 0;
    
    while (true) {
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
  
  String::SO*
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

  std::ostream&
  String::SO::print(std::ostream& os) const {
    const SO* cur = this;
    while (cur != NULL) {
      os << cur->s;
      cur = cur->next;
    }
    return os;
  }

  String::SO::~SO(void) {
    if (own)
      ::free(const_cast<char*>(s));
    if (next && next->cancel())
      delete next;
  }

}}

// STATISTICS: support-any
