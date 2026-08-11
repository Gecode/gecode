/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
#include <gecode/word.hh>

namespace Gecode {

  StdWordTracer::StdWordTracer(std::ostream& os0) : os(os0) {}

  void
  StdWordTracer::init(const Space&, const WordTraceRecorder& t) {
    os << "trace<Word>::init(id:" << t.id();
    if (t.group().in()) os << ",g:" << t.group().id();
    os << ") slack: 100.00% (" << t.slack().initial() << " bits)"
       << std::endl;
  }

  void
  StdWordTracer::prune(const Space&, const WordTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, WordTraceDelta& d) {
    os << "trace<Word>::prune(id:" << t.id();
    if (t.group().in()) os << ",g:" << t.group().id();
    os << "): [" << i << "] = " << t[i]
       << " zero:0x" << std::hex << d.zero()
       << " one:0x" << d.one() << std::dec
       << " by " << vti << std::endl;
  }

  static void
  print_slack(std::ostream& os, const WordTraceRecorder& t) {
    const double initial = static_cast<double>(t.slack().initial());
    const double previous = static_cast<double>(t.slack().previous());
    const double current = static_cast<double>(t.slack().current());
    const double current_percent = (initial == 0.0) ? 0.0
      : 100.0 * current / initial;
    const double delta_percent = (initial == 0.0) ? 0.0
      : 100.0 * (previous-current) / initial;
    os << std::showpoint << std::setprecision(4)
       << current_percent << "% - " << delta_percent << '%';
  }

  void
  StdWordTracer::fix(const Space&, const WordTraceRecorder& t) {
    os << "trace<Word>::fix(id:" << t.id();
    if (t.group().in()) os << ",g:" << t.group().id();
    os << ") slack: "; print_slack(os,t); os << std::endl;
  }
  void
  StdWordTracer::fail(const Space&, const WordTraceRecorder& t) {
    os << "trace<Word>::fail(id:" << t.id();
    if (t.group().in()) os << ",g:" << t.group().id();
    os << ") slack: "; print_slack(os,t); os << std::endl;
  }
  void
  StdWordTracer::done(const Space&, const WordTraceRecorder& t) {
    os << "trace<Word>::done(id:" << t.id();
    if (t.group().in()) os << ",g:" << t.group().id();
    os << ") slack: 0%" << std::endl;
  }

  StdWordTracer StdWordTracer::def;

}

// STATISTICS: word-post
