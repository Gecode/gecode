/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

namespace Test { namespace Set {

  inline
  std::string
  SetTest::str(int i) {
    std::stringstream s;
    s << i;
    return s.str();
  }

  inline
  std::string
  SetTest::str(Gecode::SetRelType srt) {
    switch (srt) {
    case Gecode::SRT_EQ: return "Eq";
    case Gecode::SRT_LQ: return "Lq";
    case Gecode::SRT_LE: return "Le";
    case Gecode::SRT_GQ: return "Gq";
    case Gecode::SRT_GR: return "Gr";
    case Gecode::SRT_NQ: return "Nq";
    case Gecode::SRT_SUB: return "Sub";
    case Gecode::SRT_SUP: return "Sup";
    case Gecode::SRT_DISJ: return "Disj";
    case Gecode::SRT_CMPL: return "Cmpl";
    }
    return "";
  }

  inline
  std::string
  SetTest::str(Gecode::SetOpType sot) {
    switch (sot) {
    case Gecode::SOT_UNION: return "Union";
    case Gecode::SOT_DUNION: return "DUnion";
    case Gecode::SOT_INTER: return "Inter";
    case Gecode::SOT_MINUS: return "Minus";
    }
    return "";
  }

  inline
  std::string
  SetTest::str(const Gecode::IntArgs& x) {
    std::string s = "";
    for (int i=0; i<x.size()-1; i++)
      s += str(x[i]) + ",";
    return "[" + s + str(x[x.size()-1]) + "]";
  }

  inline
  SetRelTypes::SetRelTypes(void)
    : i(sizeof(srts)/sizeof(Gecode::SetRelType)-1) {}
  inline bool
  SetRelTypes::operator()(void) const {
    return i>=0;
  }
  inline void
  SetRelTypes::operator++(void) {
    i--;
  }
  inline Gecode::SetRelType
  SetRelTypes::srt(void) const {
    return srts[i];
  }

  inline
  SetOpTypes::SetOpTypes(void)
    : i(sizeof(sots)/sizeof(Gecode::SetOpType)-1) {}
  inline bool
  SetOpTypes::operator()(void) const {
    return i>=0;
  }
  inline void
  SetOpTypes::operator++(void) {
    i--;
  }
  inline Gecode::SetOpType
  SetOpTypes::sot(void) const {
    return sots[i];
  }

}}

// STATISTICS: test-set
