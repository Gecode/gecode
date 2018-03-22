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
#include <gecode/int.hh>

namespace Gecode {

  StdIntTracer::StdIntTracer(std::ostream& os0)
    : os(os0) {}

  void
  StdIntTracer::init(const Space&, const IntTraceRecorder& t) {
    os << "trace<Int>::init(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 100.00% (" << t.slack().initial() << " values)"
       << std::endl;
  }

  void
  StdIntTracer::prune(const Space&, const IntTraceRecorder& t,
                      const ViewTraceInfo& vti, int i, IntTraceDelta& d) {
    os << "trace<Int>::prune(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << "): [" << i << "] = " << t[i] << " - {";
    os << d.min();
    if (d.width() > 1)
      os << ".." << d.max();
    ++d;
    while (d()) {
      os << ',' << d.min();
      if (d.width() > 1)
        os << ".." << d.max();
      ++d;
    }
    os << "} by " << vti << std::endl;
  }

  void
  StdIntTracer::fix(const Space&, const IntTraceRecorder& t) {
    os << "trace<Int>::fix(id:" << t.id();
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
  StdIntTracer::fail(const Space&, const IntTraceRecorder& t) {
    os << "trace<Int>::fail(id:" << t.id();
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
  StdIntTracer::done(const Space&, const IntTraceRecorder& t) {
    os << "trace<Int>::done(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 0%" << std::endl;
  }

  StdIntTracer StdIntTracer::def;



  StdBoolTracer::StdBoolTracer(std::ostream& os0)
    : os(os0) {}

  void
  StdBoolTracer::init(const Space&, const BoolTraceRecorder& t) {
    os << "trace<Bool>::init(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 100% (" << t.slack().initial() << " values)"
       << std::endl;
  }

  void
  StdBoolTracer::prune(const Space&, const BoolTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, BoolTraceDelta& d) {
    os << "trace<Bool>::prune(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << "): [" << i << "] = " << t[i] << " - {";
    os << d.min();
    if (d.width() > 1)
      os << ".." << d.max();
    ++d;
    while (d()) {
      os << ',' << d.min();
      if (d.width() > 1)
        os << ".." << d.max();
      ++d;
    }
    os << "} by " << vti << std::endl;
  }

  void
  StdBoolTracer::fix(const Space&, const BoolTraceRecorder& t) {
    os << "trace<Bool>::fix(id:" << t.id();
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
  StdBoolTracer::fail(const Space&, const BoolTraceRecorder& t) {
    os << "trace<Bool>::fail(id:" << t.id();
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
  StdBoolTracer::done(const Space&, const BoolTraceRecorder& t) {
    os << "trace<Bool>::done(id:" << t.id();
    if (t.group().in())
      os << ",g:" << t.group().id();
    os << ") slack: 0%" << std::endl;
  }

  StdBoolTracer StdBoolTracer::def;

}

// STATISTICS: int-trace
