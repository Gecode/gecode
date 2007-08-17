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

#ifndef __GECODE_SUPPORT_STRING_HH__
#define __GECODE_SUPPORT_STRING_HH__

#include "gecode/kernel.hh"

namespace Gecode { namespace Support {
  
  class String {
  private:
    char* s;
    int n;
  public:
    String(void) : s(NULL), n(0) {}
    String(const char* s0) {
      n = strlen(s0);
      if (n>0) {
        s = static_cast<char*>(Memory::malloc(sizeof(char)*(n+1)));
        memcpy(s, s0, n);
        s[n] = 0;
      }
    }
    ~String(void) { if (n>0) Memory::free(s); }
    String(const String& s0) {
      n = s0.n;
      if (n>0) {
        s = static_cast<char*>(Memory::malloc(sizeof(char)*(n+1)));
        memcpy(s, s0.s, n);
        s[n] = 0;
      }
    }
    const String& operator=(const String& s0) {
      if (this != &s0) {
        if (n>0)
          Memory::free(s);
        n = s0.n;
        if (n>0) {
          s = static_cast<char*>(Memory::malloc(sizeof(char)*(n+1)));
          memcpy(s, s0.s, n);
          s[n] = 0;
        }
      }
      return *this;
    }
    const char*
    c_str(void) const {
      return s;
    }
    
    bool
    empty(void) const {
      return n==0;
    }
    
    String operator+(const String& s0) const {
      String ret;
      ret.n = n + s0.n;
      if (ret.n>0) {
        ret.s = static_cast<char*>(Memory::malloc(sizeof(char)*(ret.n+1)));
        memcpy(ret.s, s, n);
        memcpy(ret.s+n, s0.s, s0.n);
        ret.s[ret.n] = 0;
      }
      return ret;
    }
    
    bool operator==(const String& s0) {
      return !strcmp(s, s0.s);
    }
    
  };

  class StringCmp {
  public:
    int cmp(const String& s0, const String& s1) {
      return strcmp(s0.c_str(), s1.c_str());
    }
  };
  
}}

#endif

// STATISTICS: support-any
