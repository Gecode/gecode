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

#include <iomanip>
#include <gecode/set.hh>

namespace Gecode {

  StdSetTracer::StdSetTracer(std::ostream& os0)
    : os(os0) {}

  void
  StdSetTracer::init(const Space&, const SetTraceRecorder& t) {
    os << "trace<Set>::init(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 100.00% (" << t.slack().initial() << " values)"
       << std::endl;
  }

  void
  StdSetTracer::prune(const Space&, const SetTraceRecorder& t,
                      const ViewTraceInfo& vti, int i, SetTraceDelta& d) {
    os << "trace<Set>::prune(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << "): [" << i << "] = " << t[i] << " + {";
    {
      SetTraceDelta::Glb glb(d.glb());
      if (glb()) {
        os << glb.min() << ".." << glb.max();
        ++glb;
        while (glb()) {
          os << "," << glb.min() << ".." << glb.max();
          ++glb;
        }
      }
    }
    os << "} - {";
    {
      SetTraceDelta::Lub lub(d.lub());
      if (lub()) {
        os << lub.min() << ".." << lub.max();
        ++lub;
        while (lub()) {
          os << "," << lub.min() << ".." << lub.max();
          ++lub;
        }
      }
    }
    os << "} by " << vti << std::endl;
  }

  void
  StdSetTracer::fix(const Space&, const SetTraceRecorder& t) {
    os << "trace<Set>::fix(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: ";
    double sl_i = static_cast<double>(t.slack().initial());
    double sl_p = static_cast<double>(t.slack().previous());
    double sl_c = static_cast<double>(t.slack().current());
    double p_c = 100.0 * (sl_c / sl_i);
    double p_d = 100.0 * (sl_p / sl_i) - p_c;
    os << std::showpoint << std::setprecision(4)
       << p_c << "% - "
       << std::showpoint << std::setprecision(4)
       << p_d << '%'
       << std::endl;
  }

  void
  StdSetTracer::fail(const Space&, const SetTraceRecorder& t) {
    os << "trace<Set>::fail(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: ";
    double sl_i = static_cast<double>(t.slack().initial());
    double sl_p = static_cast<double>(t.slack().previous());
    double sl_c = static_cast<double>(t.slack().current());
    double p_c = 100.0 * (sl_c / sl_i);
    double p_d = 100.0 * (sl_p / sl_i) - p_c;
    os << std::showpoint << std::setprecision(4)
       << p_c << "% - "
       << std::showpoint << std::setprecision(4)
       << p_d << '%'
       << std::endl;
  }

  void
  StdSetTracer::done(const Space&, const SetTraceRecorder& t) {
    os << "trace<Set>::done(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 0%" << std::endl;
  }

  StdSetTracer StdSetTracer::def;

}

// STATISTICS: set-trace
