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

#include "test/word.hh"

namespace Test { namespace Word {

  class CaptureTracer : public Gecode::WordTracer {
  public:
    Gecode::WordValue zero;
    Gecode::WordValue one;
    int prunes;
    CaptureTracer(void) : zero(0), one(0), prunes(0) {}
    virtual void init(const Gecode::Space&,
                      const Gecode::WordTraceRecorder&) {}
    virtual void prune(const Gecode::Space&,
                       const Gecode::WordTraceRecorder&,
                       const Gecode::ViewTraceInfo&, int,
                       Gecode::WordTraceDelta& d) {
      zero |= d.zero(); one |= d.one(); prunes++;
    }
    virtual void fix(const Gecode::Space&,
                     const Gecode::WordTraceRecorder&) {}
    virtual void fail(const Gecode::Space&,
                      const Gecode::WordTraceRecorder&) {}
    virtual void done(const Gecode::Space&,
                      const Gecode::WordTraceRecorder&) {}
  };

  class TraceSpace : public Gecode::Space {
  public:
    Gecode::WordVarArray x;
    TraceSpace(CaptureTracer& tracer) : x(*this,1,4,0,15) {
      Gecode::trace(*this,x,Gecode::TraceFilter::all,Gecode::TE_PRUNE,tracer);
    }
    TraceSpace(TraceSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
    }
    virtual Gecode::Space* copy(void) { return new TraceSpace(*this); }
  };

  class Trace : public Base {
  public:
    Trace(void) : Base("Word::Trace") {}
    virtual bool run(void) {
      CaptureTracer tracer;
      TraceSpace root(tracer);
      TraceSpace* clone = static_cast<TraceSpace*>(root.clone());
      Gecode::dom(*clone,clone->x[0],2,6);
      const bool ok = (clone->status() != Gecode::SS_FAILED) &&
        (tracer.prunes == 1) && (tracer.zero == 9) && (tracer.one == 2);
      delete clone;
      return ok;
    }
  };

  namespace { Trace trace; }

}}

// STATISTICS: test-word
