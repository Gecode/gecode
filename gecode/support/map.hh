/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2007-08-09 15:30:21 +0200 (Thu, 09 Aug 2007) $ by $Author: tack $
 *     $Revision: 4790 $
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

#ifndef __GECODE_SUPPORT_MAP_HH__
#define __GECODE_SUPPORT_MAP_HH__

#include "gecode/support/string.hh"
#include "gecode/support/dynamic-array.hh"

namespace Gecode { namespace Support {
  
  template <class Key, class KeyCmp, class Value>
  class Map {
  private:
    struct Pair {
      Key k; Value v;
      Pair(const Key& k0, const Value& v0) : k(k0), v(v0) {}
    };
    Support::DynamicArray<Pair> a;
    int n;
  public:
    Map() : n(0) {}
    void put(const Key& k, Value v) { new (&a[n++]) Pair(k,v); };
    bool get(const Key& k, Value& v) {
      for (int i=n; i--;) {
        if (a[i].k == k) {
          v = a[i].v; return true;
        }
      }
      return false;
    }
    int size(void) { return n; }
    const Key& key(int i) const { return a[i].k; }
    const Key& value(int i) const { return a[i].v; }
  };
  
  template <class Value>
  class StringMap : public Map<String,StringCmp,Value> {
    
  };

  template <class N>
  class Cmp {
  public:
    int cmp(N i, N j) {
      if (i>j) return 1;
      if (i<j) return -1;
      return 0;
    }
  };

  template <class Value>
  class IntMap : public Map<int,Cmp<int>,Value> {
  };

  template <class P, class Value>
  class PtrMap : public Map<P*,Cmp<P*>,Value> {    
  };
  
}}

#endif

// STATISTICS: support-any
