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

#include <sstream>

namespace Test { namespace Word {

  class CaptureTracer : public Gecode::WordTracer {
  public:
    Gecode::WordValue zero;
    Gecode::WordValue one;
    int prunes;
    Gecode::WordDomainType kind;
    Gecode::WordValue old_rank_minimum, old_rank_maximum;
    Gecode::WordValue new_rank_minimum, new_rank_maximum;
    bool bits_changed, bounds_changed;
    Gecode::Word::WordValueSet new_holes;
    CaptureTracer(void)
      : zero(0), one(0), prunes(0), kind(Gecode::WDT_CUBE),
        old_rank_minimum(0), old_rank_maximum(0), new_rank_minimum(0), new_rank_maximum(0),
        bits_changed(false), bounds_changed(false), new_holes{{0,0,0,0}} {}
    virtual void init(const Gecode::Space&,
                      const Gecode::WordTraceRecorder&) {}
    virtual void prune(const Gecode::Space&,
                       const Gecode::WordTraceRecorder&,
                       const Gecode::ViewTraceInfo&, int,
                       Gecode::WordTraceDelta& d) {
      zero |= d.zero(); one |= d.one(); prunes++;
      kind=d.domain_type();
      bits_changed=d.bits_changed(); bounds_changed=d.bounds_changed();
      new_holes=d.new_holes();
      if (d.bounded()) {
        old_rank_minimum=d.old_rank_minimum(); old_rank_maximum=d.old_rank_maximum();
        new_rank_minimum=d.new_rank_minimum(); new_rank_maximum=d.new_rank_maximum();
      }
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
    TraceSpace(CaptureTracer& tracer) : x(*this,1,4) {
      Gecode::trace(*this,x,Gecode::TraceFilter::all,Gecode::TE_PRUNE,tracer);
    }
    TraceSpace(TraceSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
    }
    virtual Gecode::Space* copy(void) { return new TraceSpace(*this); }
  };

  class Trace : public Base {
  public:
    Trace(void) : Base("Word::Trace",TestTags::check()) {}
    virtual bool run(void) {
      CaptureTracer tracer;
      TraceSpace root(tracer);
      if (root.status() == Gecode::SS_FAILED) return false;
      TraceSpace* clone = static_cast<TraceSpace*>(root.clone());
      Gecode::dom(*clone,clone->x[0],2,6);
      const bool ok = (clone->status() != Gecode::SS_FAILED) &&
        (tracer.prunes == 1) && (tracer.zero == 9) && (tracer.one == 2);
      delete clone;
      return ok && bounded();
    }

    bool bounded(void) const {
      using namespace Gecode;
      CaptureTracer tracer;
      class BoundedTraceSpace : public Space {
      public:
        WordVar x;
        BoundedTraceSpace(CaptureTracer& t)
          : x(*this,8,WDT_UNSIGNED,3,240) {
          WordVarArgs xs(1); xs[0]=x;
          trace(*this,xs,TraceFilter::all,TE_PRUNE,t);
        }
        BoundedTraceSpace(BoundedTraceSpace& s) : Space(s) {
          x.update(*this,s.x);
        }
        virtual Space* copy(void) { return new BoundedTraceSpace(*this); }
        void range(WordValue minimum, WordValue maximum) {
          Gecode::Word::UnsignedWordView v(x);
          (void) v.narrow_range(*this,minimum,maximum);
        }
      };
      BoundedTraceSpace root(tracer);
      if (root.status() == SS_FAILED) return false;
      BoundedTraceSpace* clone=static_cast<BoundedTraceSpace*>(root.clone());
      clone->range(17,200);
      const bool ok=(clone->status() != SS_FAILED) &&
        (tracer.prunes == 1) && (tracer.kind == WDT_UNSIGNED) &&
        (tracer.zero == 0) && (tracer.one == 0) &&
        !tracer.bits_changed && tracer.bounds_changed &&
        (tracer.old_rank_minimum == 3) && (tracer.old_rank_maximum == 240) &&
        (tracer.new_rank_minimum == 17) && (tracer.new_rank_maximum == 200);
      std::ostringstream out; out << clone->x;
      delete clone;
      return ok && (out.str().find("[u:0x11..0xc8]") != std::string::npos) &&
        slack() && value_set();
    }

    bool value_set(void) const {
      using namespace Gecode;
      CaptureTracer capture;
      std::ostringstream log;
      StdWordTracer standard(log);
      class ValueSetTraceSpace : public Space {
      public:
        WordVar x;
        ValueSetTraceSpace(WordDomainType kind, WordTracer& t)
          : x(*this,4,kind,WVS_EXACT) {
          WordVarArgs xs(1); xs[0]=x;
          trace(*this,xs,TraceFilter::all,TE_PRUNE,t);
        }
        ValueSetTraceSpace(ValueSetTraceSpace& s) : Space(s) {
          x.update(*this,s.x);
        }
        virtual Space* copy(void) { return new ValueSetTraceSpace(*this); }
        void remove(WordValue n) {
          (void) Gecode::Word::WordView(x).nq(*this,n);
        }
      };

      // Removing 5 changes neither the cube nor the bounds.
      ValueSetTraceSpace captured(WDT_UNSIGNED,capture);
      ValueSetTraceSpace logged(WDT_UNSIGNED,standard);
      if ((captured.status() == SS_FAILED) || (logged.status() == SS_FAILED))
        return false;
      captured.remove(5); logged.remove(5);
      if ((captured.status() == SS_FAILED) || (logged.status() == SS_FAILED))
        return false;
      const Gecode::Word::WordValueSet five = {{WordValue(1) << 5,0,0,0}};
      const bool captured_ok = (capture.prunes == 1) &&
        (capture.zero == 0) && (capture.one == 0) &&
        !capture.bits_changed && !capture.bounds_changed &&
        Gecode::Word::same_value_sets(capture.new_holes,five);
      std::ostringstream unsigned_out; unsigned_out << logged.x;

      // Signed holes print in rank order, so -1..1 is one range.
      ValueSetTraceSpace holed(WDT_SIGNED,capture);
      holed.remove(15); holed.remove(0); holed.remove(1);
      std::ostringstream signed_out; signed_out << holed.x;

      return captured_ok &&
        (log.str().find(": [0] = 4'b????[u:0x0..0xf]\\{0x5} zero:0x0 one:0x0"
                        " rank:[0x0..0xf]->[0x0..0xf] holes:+{0x5} by ") !=
         std::string::npos) &&
        (unsigned_out.str() == "4'b????[u:0x0..0xf]\\{0x5}") &&
        (signed_out.str() == "4'b????[s:0x8..0x7]\\{0xf..0x1}");
    }

    bool slack(void) const {
      using namespace Gecode;
      class SlackSpace : public Space {
      public:
        WordVar x;
        SlackSpace(unsigned int width) : x(*this,width) {}
        SlackSpace(unsigned int width, WordDomainType kind,
                   WordValue minimum, WordValue maximum)
          : x(*this,width,kind,minimum,maximum) {}
        SlackSpace(SlackSpace& s) : Space(s) { x.update(*this,s.x); }
        virtual Space* copy(void) { return new SlackSpace(*this); }
        unsigned long long int slack(void) const {
          return Gecode::Word::WordTraceView::slack(Gecode::Word::WordView(x));
        }
      };

      SlackSpace compact(8);
      SlackSpace unsigned_one(1,WDT_UNSIGNED,0,1);
      SlackSpace signed_one(1,WDT_SIGNED,1,0);
      SlackSpace unsigned_wide(64,WDT_UNSIGNED,0,~WordValue(0));
      SlackSpace signed_wide(64,WDT_SIGNED,WordValue(1) << 63,
                             (WordValue(1) << 63)-1);
      SlackSpace assigned(64,WDT_UNSIGNED,42,42);
      SlackSpace unsigned_bits(8,WDT_UNSIGNED,64,192);
      Gecode::Word::WordView unsigned_bits_view(unsigned_bits.x);
      (void) unsigned_bits_view.narrow(unsigned_bits,64,255);
      SlackSpace unsigned_combined(8,WDT_UNSIGNED,64,192);
      Gecode::Word::WordView unsigned_combined_view(unsigned_combined.x);
      (void) unsigned_combined_view.narrow(unsigned_combined,64,255);
      (void) unsigned_combined_view.narrow_rank_range(unsigned_combined,64,127);
      SlackSpace signed_bits(8,WDT_SIGNED,192,64);
      Gecode::Word::WordView signed_bits_view(signed_bits.x);
      (void) signed_bits_view.narrow(signed_bits,64,255);
      SlackSpace signed_combined(8,WDT_SIGNED,192,64);
      Gecode::Word::WordView signed_combined_view(signed_combined.x);
      (void) signed_combined_view.narrow(signed_combined,64,255);
      (void) signed_combined_view.narrow_rank_range(signed_combined,64,127);

      std::ostringstream output;
      StdWordTracer tracer(output);
      class ProgressSpace : public Space {
      public:
        WordVar x;
        ProgressSpace(WordTracer& t) : x(*this,8,WDT_UNSIGNED,3,240) {
          WordVarArgs xs(1); xs[0]=x;
          trace(*this,xs,TraceFilter::all,TE_INIT | TE_FIX,t);
        }
        ProgressSpace(ProgressSpace& s) : Space(s) { x.update(*this,s.x); }
        virtual Space* copy(void) { return new ProgressSpace(*this); }
        void range(WordValue minimum, WordValue maximum) {
          Gecode::Word::UnsignedWordView v(x);
          (void) v.narrow_range(*this,minimum,maximum);
        }
      } progress(tracer);
      if (progress.status() == SS_FAILED)
        return false;
      progress.range(17,100);
      if (progress.status() == SS_FAILED)
        return false;

      return (compact.slack() == 8) &&
        (unsigned_one.slack() == 2) && (signed_one.slack() == 2) &&
        (unsigned_wide.slack() == 128) && (signed_wide.slack() == 128) &&
        (assigned.slack() == 0) &&
        (unsigned_bits.slack() == 15) && (signed_bits.slack() == 15) &&
        (unsigned_combined.slack() == 12) &&
        (signed_combined.slack() == 12) &&
        (output.str().find("87.50% - 12.50%") != std::string::npos);
    }
  };

  /// Float words print their format and total-order endpoints and holes
  class FloatFormat : public Base {
  public:
    FloatFormat(void) : Base("Word::Trace::FloatFormat") {}

    /// Space with one traced float word
    class FloatTraceSpace : public Gecode::Space {
    public:
      Gecode::WordVar x;
      FloatTraceSpace(Gecode::WordDomainType type, Gecode::WordTracer& t)
        : x(*this,type) {
        Gecode::WordVarArgs xs(1); xs[0]=x;
        Gecode::trace(*this,xs,Gecode::TraceFilter::all,Gecode::TE_PRUNE,t);
      }
      FloatTraceSpace(FloatTraceSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x);
      }
      virtual Gecode::Space* copy(void) { return new FloatTraceSpace(*this); }
      void remove(Gecode::WordValue n) {
        (void) Gecode::Word::WordView(x).nq(*this,n);
      }
    };

    static std::string print(const Gecode::WordVar& x) {
      std::ostringstream out; out << x;
      return out.str();
    }

    virtual bool run(void) {
      using namespace Gecode;
      CaptureTracer capture;
      std::ostringstream log;
      StdWordTracer standard(log);

      FloatTraceSpace fp32(WDT_FP32,capture);
      const std::string fp32_out = print(fp32.x);
      const std::string fp32_tail = "[fp32:0xffffffff..0x7fffffff]";
      const bool fp32_ok = (fp32_out.size() == 36+fp32_tail.size()) &&
        (fp32_out.compare(36,std::string::npos,fp32_tail) == 0);

      // Removing -6 (0xf), the least E2M1 value, moves the lower rank.
      FloatTraceSpace captured(WDT_E2M1,capture);
      FloatTraceSpace logged(WDT_E2M1,standard);
      const std::string full_out = print(captured.x);
      if ((captured.status() == SS_FAILED) || (logged.status() == SS_FAILED))
        return false;
      captured.remove(0xf); logged.remove(0xf);
      if ((captured.status() == SS_FAILED) || (logged.status() == SS_FAILED))
        return false;
      const bool captured_ok = (capture.prunes == 1) &&
        (capture.kind == WDT_E2M1) && !capture.bits_changed &&
        capture.bounds_changed &&
        (capture.old_rank_minimum == 0) && (capture.old_rank_maximum == 15) &&
        (capture.new_rank_minimum == 1) && (capture.new_rank_maximum == 15);

      // -0, +0 and +0.5 are adjacent in the total order.
      FloatTraceSpace holed(WDT_E2M1,capture);
      holed.remove(0x8); holed.remove(0x0); holed.remove(0x1);

      return fp32_ok && captured_ok &&
        (full_out == "4'b????[e2m1:0xf..0x7]") &&
        (print(captured.x) == "4'b????[e2m1:0xe..0x7]") &&
        (log.str().find(": [0] = 4'b????[e2m1:0xe..0x7] zero:0x0 one:0x0"
                        " rank:[0x0..0xf]->[0x1..0xf] by ") !=
         std::string::npos) &&
        (print(holed.x) == "4'b????[e2m1:0xf..0x7]\\{0x8..0x1}");
    }
  };

  namespace {
    Trace trace;
    FloatFormat float_format;
  }

}}

// STATISTICS: test-word
