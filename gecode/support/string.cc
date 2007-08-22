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

  int
  String::SO::cmp(SO* other) const {
    if (s && other->s)
      return (s==other->s ? 0 : strcmp(s,other->s));
    SOIter me(this); SOIter notme(other);
    for (; me() && notme(); ++me, ++notme)
      if (me.c() != notme.c())
        return me.c() < notme.c() ? -1 : 1;
    if (me())
      return -1;
    if (notme())
      return 1;
    return 0;
  }

  String::SO::~SO(void) {
    if (own)
      ::free(const_cast<char*>(s));
    if (left && left->cancel())
      delete left;
    if (right && right->cancel())
      delete right;
  }

  String::SOIter::SOIter(const String::SO* so0) : n(0) {
    if (so0->s) {
      so = so0; size = static_cast<int>(strlen(so->s)); 
      left = NULL; right = NULL;
    } else {
      so = NULL;
      left  = new SOIter(so0->left);
      right = new SOIter(so0->right);
    }
  }

  bool
  String::SOIter::operator()(void) {
    if (so)
      return n < size;
    return (*left)() || (*right)();
  }
  void
  String::SOIter::operator++(void) {
    if (so) {
      n++;
      return;
    }
    if ((*left)()) {
      ++(*left);
      return;
    }
    ++(*right);
  }
  char
  String::SOIter::c(void) const {
    if (so)
      return so->s[n];
    if ((*left)())
      return left->c();
    return right->c();
  }

  String::SOIter::~SOIter(void) {
    delete left;
    delete right;
  }

}}

// STATISTICS: support-any
