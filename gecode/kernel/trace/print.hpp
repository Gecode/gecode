/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <iostream>
#include <sstream>

namespace Gecode {

  /**
   * \brief Print view trace information
   * \relates ViewTraceInfo
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const ViewTraceInfo& vti) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    switch (vti.what()) {
    case ViewTraceInfo::PROPAGATOR:
      s << "propagator(id:" << vti.propagator().id();
      if (vti.propagator().group().in())
        s  << ",g:" << vti.propagator().group().id();
      s << ')';
      break;
    case ViewTraceInfo::BRANCHER:
      s << "brancher(id:" << vti.brancher().id();
      if (vti.brancher().group().in())
        s  << ",g:" << vti.brancher().group().id();
      s << ')';
      break;
    case ViewTraceInfo::POST:
      s << "post(";
      if (vti.post().in())
        s << "g:" << vti.post().id();
      s << ')';
      break;
    case ViewTraceInfo::OTHER:
      s << '-';
      break;
    default:
      GECODE_NEVER;
    }
    return os << s.str();
  }

  /**
   * \brief Print propagate trace information
   * \relates PropagateTraceInfo
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const PropagateTraceInfo& pti) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << "propagate(id:" << pti.id();
    if (pti.group().in())
      s  << ",g:" << pti.group().id();
    s << ",s:";
    switch (pti.status()) {
    case PropagateTraceInfo::FIX:
      s << "fix"; break;
    case PropagateTraceInfo::NOFIX:
      s << "nofix"; break;
    case PropagateTraceInfo::FAILED:
      s << "failed"; break;
    case PropagateTraceInfo::SUBSUMED:
      s << "subsumed"; break;
    default:
      GECODE_NEVER;
    }
    s << ')';
    return os << s.str();
  }

  /**
   * \brief Print commit trace information
   * \relates CommitTraceInfo
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const CommitTraceInfo& cti) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << "commit(id:" << cti.id();
    if (cti.group().in())
      s  << ",g:" << cti.group().id();
    s << ')';
    return os << s.str();
  }

  /**
   * \brief Print post trace information
   * \relates PostTraceInfo
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const PostTraceInfo& pti) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << "post(";
    if (pti.group().in())
      s  << "g:" << pti.group().id() << ",";
    s << "s:";
    switch (pti.status()) {
    case PostTraceInfo::POSTED:
      s << "posted(" << pti.propagators() << ")"; break;
    case PostTraceInfo::FAILED:
      s << "failed"; break;
    case PostTraceInfo::SUBSUMED:
      s << "subsumed"; break;
    default:
      GECODE_NEVER;
    }
    s << ')';
    return os << s.str();
  }

}

// STATISTICS: kernel-trace
