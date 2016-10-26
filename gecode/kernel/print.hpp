/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <iostream>
#include <sstream>

namespace Gecode {

  /**
   * \brief Print execution information
   * \relates ExecInfo
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const ExecInfo& ei) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    switch (ei.what()) {
    case ExecInfo::PROPAGATOR:
      s << "propagator(id:" << ei.propagator().id();
      if (ei.propagator().group().in())
        s  << ",g:" << ei.propagator().group().id();
      s << ')';
      break;
    case ExecInfo::BRANCHER:
      s << "brancher(id:" << ei.brancher().id();
      if (ei.brancher().group().in())
        s  << ",g:" << ei.brancher().group().id();
      s << ')';
      break;
    case ExecInfo::POST:
      s << "post(";
      if (ei.post().in())
        s << "g:" << ei.post().id();
      s << ')';
      break;
    case ExecInfo::OTHER:
      s << '-';
      break;
    default:
      GECODE_NEVER;
    }
    return os << s.str();
  }

}

// STATISTICS: kernel-other
