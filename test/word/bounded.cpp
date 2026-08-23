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
 */

#include "test/word.hh"

namespace Test { namespace Word { namespace Bounded {

  using Gecode::WordValue;
  using Gecode::WordDomainType;

  static unsigned int probe_count[4];
  static Gecode::ModEvent probe_event[4];

  struct DeltaCapture {
    unsigned int count;
    Gecode::ModEvent event;
    WordValue zero, one;
    WordValue old_minimum, old_maximum, new_minimum, new_maximum;
    WordDomainType kind;
  } delta_capture;

  class DeltaProbe : public Gecode::Propagator {
  protected:
    Gecode::Word::WordView x;
    Gecode::Council<Gecode::ViewAdvisor<Gecode::Word::WordView> > c;
    DeltaProbe(Gecode::Home home, Gecode::Word::WordView x0)
      : Gecode::Propagator(home), x(x0), c(home) {
      (void) new (home) Gecode::ViewAdvisor<Gecode::Word::WordView>(
        home,*this,c,x);
    }
    DeltaProbe(Gecode::Space& home, DeltaProbe& p)
      : Gecode::Propagator(home,p) {
      x.update(home,p.x);
      c.update(home,p.c);
    }
  public:
    static void post(Gecode::Home home, Gecode::Word::WordView x) {
      (void) new (home) DeltaProbe(home,x);
    }
    virtual Gecode::Actor* copy(Gecode::Space& home) {
      return new (home) DeltaProbe(home,*this);
    }
    virtual Gecode::PropCost cost(
      const Gecode::Space&, const Gecode::ModEventDelta&) const {
      return Gecode::PropCost::unary(Gecode::PropCost::LO);
    }
    virtual void reschedule(Gecode::Space&) {}
    virtual Gecode::ExecStatus advise(
      Gecode::Space&, Gecode::Advisor&, const Gecode::Delta& d) {
      delta_capture.count++;
      delta_capture.event=Gecode::Word::WordVarImp::modevent(d);
      delta_capture.zero=x.zero(d);
      delta_capture.one=x.one(d);
      delta_capture.kind=x.domain_type(d);
      delta_capture.old_minimum=x.old_minimum(d);
      delta_capture.old_maximum=x.old_maximum(d);
      delta_capture.new_minimum=x.new_minimum(d);
      delta_capture.new_maximum=x.new_maximum(d);
      return Gecode::ES_FIX;
    }
    virtual Gecode::ExecStatus propagate(
      Gecode::Space&, const Gecode::ModEventDelta&) {
      return Gecode::ES_FIX;
    }
    virtual size_t dispose(Gecode::Space& home) {
      c.dispose(home);
      (void) Gecode::Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  template<Gecode::PropCond pc, unsigned int slot>
  class Probe : public Gecode::UnaryPropagator<Gecode::Word::WordView,pc> {
  protected:
    Probe(Gecode::Home home, Gecode::Word::WordView x)
      : Gecode::UnaryPropagator<Gecode::Word::WordView,pc>(home,x) {}
    Probe(Gecode::Space& home, Probe& p)
      : Gecode::UnaryPropagator<Gecode::Word::WordView,pc>(home,p) {}
  public:
    static void post(Gecode::Home home, Gecode::Word::WordView x) {
      (void) new (home) Probe(home,x);
    }
    virtual Gecode::Actor* copy(Gecode::Space& home) {
      return new (home) Probe(home,*this);
    }
    virtual Gecode::ExecStatus propagate(
      Gecode::Space&, const Gecode::ModEventDelta& med) {
      probe_count[slot]++;
      probe_event[slot] = Gecode::Word::WordView::me(med);
      return Gecode::ES_FIX;
    }
  };

  static WordValue
  encode(WordDomainType kind, unsigned int width, WordValue rank) {
    return rank ^ ((kind == Gecode::WDT_SIGNED) ?
                   (WordValue(1) << (width-1)) : 0);
  }

  class DomainSpace : public Gecode::Space {
  public:
    Gecode::WordVar x;
    DomainSpace(unsigned int width, WordValue lo, WordValue hi,
                WordDomainType kind, WordValue minimum, WordValue maximum)
      : x(*this,width,lo,hi,kind,minimum,maximum) {}
    DomainSpace(DomainSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
    }
    virtual Gecode::Space* copy(void) { return new DomainSpace(*this); }
  };

  static bool
  check_domain(unsigned int width, WordValue lo, WordValue hi,
               WordDomainType kind, WordValue first, WordValue last) {
    const WordValue mask = Gecode::Word::width_mask(width);
    bool any = false;
    WordValue expected_first = mask, expected_last = 0;
    for (WordValue value=0; value<=mask; value++) {
      const WordValue rank = Gecode::Word::rank(kind,width,value);
      if (Gecode::Word::cube_contains(lo,hi,value,mask) &&
          (rank >= first) && (rank <= last)) {
        expected_first = std::min(expected_first,rank);
        expected_last = std::max(expected_last,rank);
        any = true;
      }
      if (value == mask)
        break;
    }
    try {
      DomainSpace s(width,lo,hi,kind,encode(kind,width,first),
                    encode(kind,width,last));
      if (!any || !s.x.bounded() || (s.x.domain_type() != kind) ||
          (Gecode::Word::rank(kind,width,s.x.minimum()) != expected_first) ||
          (Gecode::Word::rank(kind,width,s.x.maximum()) != expected_last)) {
        ::Test::olog << "header any=" << any << " min=" << s.x.minimum()
                     << " max=" << s.x.maximum() << " expected="
                     << expected_first << "," << expected_last << std::endl;
        return false;
      }
      for (WordValue value=0; value<=mask; value++) {
        const WordValue rank = Gecode::Word::rank(kind,width,value);
        const bool expected = Gecode::Word::cube_contains(lo,hi,value,mask) &&
          (rank >= first) && (rank <= last);
        if (s.x.in(value) != expected) {
          ::Test::olog << "membership value=" << value << " got="
                       << s.x.in(value) << " expected=" << expected
                       << " masks=" << s.x.lo() << "," << s.x.hi()
                       << std::endl;
          return false;
        }
        if (value == mask)
          break;
      }
      if (s.status() == Gecode::SS_FAILED) {
        ::Test::olog << "unexpected failed source space" << std::endl;
        return false;
      }
      DomainSpace* c = static_cast<DomainSpace*>(s.clone());
      const bool clone_ok = (c->x.domain_type() == kind) &&
        (c->x.lo() == s.x.lo()) && (c->x.hi() == s.x.hi()) &&
        (c->x.minimum() == s.x.minimum()) &&
        (c->x.maximum() == s.x.maximum());
      delete c;
      if (!clone_ok)
        ::Test::olog << "clone" << std::endl;
      return clone_ok;
    } catch (const Gecode::Word::VariableEmptyDomain&) {
      return !any;
    }
  }

  static bool
  check_tell_order(unsigned int width, WordValue lo, WordValue hi,
                   WordDomainType kind, WordValue first, WordValue last) {
    const WordValue mask = Gecode::Word::width_mask(width);
    bool any=false;
    WordValue expected_first=mask, expected_last=0;
    for (WordValue value=0; value<=mask; value++) {
      const WordValue r=Gecode::Word::rank(kind,width,value);
      if (Gecode::Word::cube_contains(lo,hi,value,mask) &&
          (r >= first) && (r <= last)) {
        any=true;
        expected_first=std::min(expected_first,r);
        expected_last=std::max(expected_last,r);
      }
      if (value == mask)
        break;
    }

    DomainSpace cube_first(width,0,mask,kind,
                           encode(kind,width,0),
                           encode(kind,width,mask));
    DomainSpace range_first(width,0,mask,kind,
                            encode(kind,width,0),
                            encode(kind,width,mask));
    Gecode::ModEvent a, b;
    if (kind == Gecode::WDT_UNSIGNED) {
      Gecode::Word::UnsignedWordView x(cube_first.x), y(range_first.x);
      a=x.narrow(cube_first,lo,hi);
      if (a != Gecode::Word::ME_WORD_FAILED)
        a=x.narrow_range(cube_first,first,last);
      b=y.narrow_range(range_first,first,last);
      if (b != Gecode::Word::ME_WORD_FAILED)
        b=y.narrow(range_first,lo,hi);
    } else {
      Gecode::Word::SignedWordView x(cube_first.x), y(range_first.x);
      a=x.narrow(cube_first,lo,hi);
      if (a != Gecode::Word::ME_WORD_FAILED)
        a=x.narrow_range(cube_first,first,last);
      b=y.narrow_range(range_first,first,last);
      if (b != Gecode::Word::ME_WORD_FAILED)
        b=y.narrow(range_first,lo,hi);
    }
    const bool failed_a=Gecode::me_failed(a);
    const bool failed_b=Gecode::me_failed(b);
    if (failed_a != !any || failed_b != !any || failed_a != failed_b) {
      ::Test::olog << "tell failures any=" << any << " a=" << failed_a
                   << " b=" << failed_b << " me=" << a << "," << b
                   << std::endl;
      return false;
    }
    if (!any) return true;
    if ((cube_first.x.lo() != range_first.x.lo()) ||
        (cube_first.x.hi() != range_first.x.hi()) ||
        (cube_first.x.minimum() != range_first.x.minimum()) ||
        (cube_first.x.maximum() != range_first.x.maximum()) ||
        (Gecode::Word::rank(kind,width,cube_first.x.minimum()) !=
         expected_first) ||
        (Gecode::Word::rank(kind,width,cube_first.x.maximum()) !=
         expected_last))
      return false;
    for (WordValue value=0; value<=mask; value++) {
      const WordValue r=Gecode::Word::rank(kind,width,value);
      const bool expected=Gecode::Word::cube_contains(lo,hi,value,mask) &&
        (r >= first) && (r <= last);
      if ((cube_first.x.in(value) != expected) ||
          (range_first.x.in(value) != expected))
        return false;
      if (value == mask)
        break;
    }
    return true;
  }

  static bool
  exhaustive(void) {
    const WordDomainType kinds[] = {
      Gecode::WDT_UNSIGNED, Gecode::WDT_SIGNED
    };
    for (unsigned int width=1; width<=8; width++) {
      const WordValue n = WordValue(1) << width;
      for (unsigned int k=0; k<2; k++) {
        // Every interval at every required small width, over the full cube.
        for (WordValue first=0; first<n; first++)
          for (WordValue last=first; last<n; last++)
            if (!check_domain(width,0,n-1,kinds[k],first,last))
              {
                ::Test::olog << "interval width=" << width << " kind=" << k
                           << " first=" << first << " last=" << last
                           << std::endl;
                return false;
              }
        // Every cube, over the full ranked interval.
        WordValue cubes = 1;
        for (unsigned int bit=0; bit<width; bit++)
          cubes *= 3;
        for (WordValue code=0; code<cubes; code++) {
          WordValue c=code, lo=0, hi=0;
          for (unsigned int bit=0; bit<width; bit++, c/=3) {
            const WordValue b = WordValue(1) << bit;
            if ((c % 3) != 0)
              hi |= b;
            if ((c % 3) == 2)
              lo |= b;
          }
          if (!check_domain(width,lo,hi,kinds[k],0,n-1))
            {
              ::Test::olog << "cube width=" << width << " kind=" << k
                         << " lo=" << lo << " hi=" << hi << std::endl;
              return false;
            }
        }
      }
    }

    // Cross cube shape and interval bounds exhaustively through width four.
    for (unsigned int width=1; width<=4; width++) {
      const WordValue n=WordValue(1) << width;
      WordValue cubes=1;
      for (unsigned int bit=0; bit<width; bit++) cubes*=3;
      for (unsigned int k=0; k<2; k++)
        for (WordValue code=0; code<cubes; code++) {
          WordValue c=code, lo=0, hi=0;
          for (unsigned int bit=0; bit<width; bit++,c/=3) {
            const WordValue b=WordValue(1) << bit;
            if ((c % 3) != 0) hi|=b;
            if ((c % 3) == 2) lo|=b;
          }
          for (WordValue first=0; first<n; first++)
            for (WordValue last=first; last<n; last++)
              if (!check_domain(width,lo,hi,kinds[k],first,last) ||
                  !check_tell_order(width,lo,hi,kinds[k],first,last)) {
                ::Test::olog << "cross width=" << width << " kind=" << k
                             << " lo=" << lo << " hi=" << hi
                             << " first=" << first << " last=" << last
                             << std::endl;
                return false;
              }
        }
    }

    // Width five uses a deterministic boundary/interior interval sample for
    // every cube, keeping the normal test runtime small.
    {
      const unsigned int width=5;
      const WordValue endpoints[] = {0,1,2,7,8,15,16,23,30,31};
      WordValue cubes=1;
      for (unsigned int bit=0; bit<width; bit++) cubes*=3;
      for (unsigned int k=0; k<2; k++)
        for (WordValue code=0; code<cubes; code++) {
          WordValue c=code, lo=0, hi=0;
          for (unsigned int bit=0; bit<width; bit++,c/=3) {
            const WordValue b=WordValue(1) << bit;
            if ((c % 3) != 0) hi|=b;
            if ((c % 3) == 2) lo|=b;
          }
          for (unsigned int i=0; i<sizeof(endpoints)/sizeof(endpoints[0]); i++)
            for (unsigned int j=i; j<sizeof(endpoints)/sizeof(endpoints[0]); j++)
              if (!check_domain(width,lo,hi,kinds[k],endpoints[i],endpoints[j]) ||
                  !check_tell_order(width,lo,hi,kinds[k],endpoints[i],endpoints[j])) {
                ::Test::olog << "sample cross width=5 kind=" << k
                             << " lo=" << lo << " hi=" << hi
                             << " first=" << endpoints[i]
                             << " last=" << endpoints[j] << std::endl;
                return false;
              }
      }
    }
    return true;
  }

  static bool
  targeted(void) {
    using namespace Gecode;
    class Empty : public Space {
    public:
      Empty(void) {}
      Empty(Empty& s) : Space(s) {}
      virtual Space* copy(void) { return new Empty(*this); }
    };
    class Mixed : public Space {
    public:
      WordVarArray x;
      Mixed(void) : x(*this,3) {
        x[0] = WordVar(*this,8);
        x[1] = WordVar(*this,8,WDT_UNSIGNED,3,240);
        x[2] = WordVar(*this,8,WDT_SIGNED,WordValue(0x80),WordValue(0x7f));
      }
      Mixed(Mixed& s) : Space(s) { x.update(*this,s.x); }
      virtual Space* copy(void) { return new Mixed(*this); }
    };
    Mixed s;
    if ((s.x[0].domain_type() != WDT_CUBE) || s.x[0].bounded() ||
        (s.x[1].minimum() != 3) || (s.x[1].maximum() != 240) ||
        (s.x[2].minimum() != WordValue(0x80)) ||
        (s.x[2].maximum() != WordValue(0x7f)))
      return false;
    Gecode::Word::SignedWordView signed_view(s.x[2]);
    if ((signed_view.rank_minimum() != 0) ||
        (signed_view.rank_maximum() != 255))
      return false;
    WordVarArgs unsigned_args(s,2,4,WDT_UNSIGNED,2,11);
    WordVarArray signed_array(s,2,4,WDT_SIGNED,WordValue(8),WordValue(7));
    if ((unsigned_args[0].minimum() != 2) ||
        (unsigned_args[1].maximum() != 11) ||
        (signed_array[0].minimum() != 8) ||
        (signed_array[1].maximum() != 7))
      return false;
    if (s.status() == SS_FAILED)
      return false;
    Mixed* c = static_cast<Mixed*>(s.clone());
    dom(*c,c->x[1],7U);
    const bool isolated = (c->status() != SS_FAILED) && c->x[1].assigned() &&
      !s.x[1].assigned() && (c->x[2].domain_type() == WDT_SIGNED);
    delete c;
    if (!isolated)
      return false;

    class Replay : public Space {
    public:
      WordVarArray x;
      Replay(void) : x(*this,3) {
        x[0]=WordVar(*this,2);
        x[1]=WordVar(*this,2,WDT_UNSIGNED,1,2);
        x[2]=WordVar(*this,2,WDT_SIGNED,3,0);
        branch(*this,x,WORD_VAR_NONE(),WORD_VAL_MSB());
      }
      Replay(Replay& s) : Space(s) { x.update(*this,s.x); }
      virtual Space* copy(void) { return new Replay(*this); }
    };
    Search::Options options;
    options.c_d=1;
    DFS<Replay> dfs(new Replay,options);
    unsigned int solutions=0;
    while (Replay* solution=dfs.next()) {
      solutions++;
      if (!solution->x[0].assigned() || !solution->x[1].assigned() ||
          !solution->x[2].assigned()) {
        delete solution;
        return false;
      }
      delete solution;
    }
    if (solutions != 16)
      return false;

    class Fallback : public Space {
    public:
      WordVar x, y, z;
      Fallback(void)
        : x(*this,4,WDT_UNSIGNED,0,12),
          y(*this,4,WDT_UNSIGNED,0,12),
          z(*this,4,WDT_UNSIGNED,0,15) {
        rel(*this,x,WOT_XOR,y,z);
      }
      Fallback(Fallback& s) : Space(s) {
        x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
      }
      virtual Space* copy(void) { return new Fallback(*this); }
    };
    Fallback fallback_source;
    if (fallback_source.status() == SS_FAILED)
      return false;
    Fallback* fallback=static_cast<Fallback*>(fallback_source.clone());
    dom(*fallback,fallback->x,3);
    dom(*fallback,fallback->y,5);
    const bool fallback_ok=(fallback->status() != SS_FAILED) &&
      fallback->z.assigned() && (fallback->z.val() == 6) &&
      fallback->z.bounded() && !fallback_source.x.assigned() &&
      !fallback_source.y.assigned() && !fallback_source.z.assigned();
    delete fallback;
    if (!fallback_ok)
      return false;

    class Wide : public Space {
    public:
      WordVar u, s;
      Wide(void)
        : u(*this,64,WDT_UNSIGNED,WordValue(1) << 63,~WordValue(0)),
          s(*this,64,WDT_SIGNED,WordValue(1) << 63,
            (WordValue(1) << 63)-1) {}
      Wide(Wide& x) : Space(x) { u.update(*this,x.u); s.update(*this,x.s); }
      virtual Space* copy(void) { return new Wide(*this); }
    };
    Wide wide;
    if (!wide.u.in(~WordValue(0)) || wide.u.in(0) ||
        !wide.s.in(WordValue(1) << 63) ||
        !wide.s.in((WordValue(1) << 63)-1))
      return false;

    Gecode::Word::WordDelta delta(1,2,WDT_SIGNED,3,8,4,7);
    if ((delta.zero() != 1) || (delta.one() != 2) ||
        (delta.domain_type() != WDT_SIGNED) ||
        (delta.old_minimum() != 3) || (delta.old_maximum() != 8) ||
        (delta.new_minimum() != 4) || (delta.new_maximum() != 7))
      return false;

    try {
      Empty home;
      WordVar x(home,4,WDT_CUBE,0,15);
      (void) x;
      return false;
    } catch (const Gecode::Word::OutOfLimits&) {}
    try {
      Empty home;
      WordVar x(home,4,WDT_SIGNED,7,8);
      (void) x;
      return false;
    } catch (const Gecode::Word::VariableEmptyDomain&) {}
    return true;
  }

  static bool
  tells(void) {
    using namespace Gecode;
    DomainSpace s(4,0,15,WDT_UNSIGNED,0,15);
    Gecode::Word::UnsignedWordView x(s.x);
    if (x.narrow_range(s,1,14) != Gecode::Word::ME_WORD_BND)
      return false;
    if ((s.x.lo() != 0) || (s.x.hi() != 15) ||
        (s.x.minimum() != 1) || (s.x.maximum() != 14))
      return false;
    // Fixing the low bit excludes the old lower endpoint and is combined.
    if (x.narrow(s,s.x.lo(),s.x.hi() & ~WordValue(1)) !=
        Gecode::Word::ME_WORD_DOM)
      return false;
    if ((s.x.minimum() != 2) || (s.x.maximum() != 14) ||
        s.x.in(3) || !s.x.in(4))
      return false;
    const WordValue old_lo=s.x.lo(), old_hi=s.x.hi();
    const WordValue old_min=s.x.minimum(), old_max=s.x.maximum();
    if (x.narrow_range(s,3,13) != Gecode::Word::ME_WORD_BND)
      return false;
    if ((s.x.lo() != old_lo) || (s.x.hi() != old_hi) ||
        (s.x.minimum() != 4) || (s.x.maximum() != 12))
      return false;
    // A failed tell is atomic.
    if (x.narrow_range(s,13,13) != Gecode::Word::ME_WORD_FAILED)
      return false;
    if (!((s.x.lo() == old_lo) && (s.x.hi() == old_hi) &&
      (old_min == 2) && (old_max == 14) &&
      (s.x.minimum() == 4) && (s.x.maximum() == 12)))
      return false;

    DomainSpace signed_space(4,0,15,WDT_SIGNED,8,7);
    Gecode::Word::SignedWordView signed_view(signed_space.x);
    if (signed_view.narrow_range(signed_space,3,12) !=
        Gecode::Word::ME_WORD_BND)
      return false;
    return (signed_view.rank_minimum() == 3) &&
      (signed_view.rank_maximum() == 12) &&
      (signed_space.x.minimum() == 11) &&
      (signed_space.x.maximum() == 4) &&
      !signed_space.x.in(10) && signed_space.x.in(11) &&
      signed_space.x.in(4) && !signed_space.x.in(5);
  }

  static bool
  events(void) {
    using namespace Gecode;
    class EventSpace : public Space {
    public:
      WordVar x;
      EventSpace(void) : x(*this,4,WDT_UNSIGNED,0,15) {
        Gecode::Word::WordView xv(x);
        Probe<Gecode::Word::PC_WORD_VAL,0>::post(*this,xv);
        Probe<Gecode::Word::PC_WORD_BITS,1>::post(*this,xv);
        Probe<Gecode::Word::PC_WORD_BND,2>::post(*this,xv);
        Probe<Gecode::Word::PC_WORD_DOM,3>::post(*this,xv);
      }
      EventSpace(EventSpace& s) : Space(s) { x.update(*this,s.x); }
      virtual Space* copy(void) { return new EventSpace(*this); }
    };
    EventSpace s;
    if (s.status() == SS_FAILED)
      return false;
    for (unsigned int i=0; i<4; i++) probe_count[i]=0;
    Gecode::Word::UnsignedWordView x(s.x);
    if (x.narrow_range(s,1,14) != Gecode::Word::ME_WORD_BND)
      return false;
    (void) s.status();
    if ((probe_count[0] != 0) || (probe_count[1] != 0) ||
        (probe_count[2] != 1) || (probe_count[3] != 1) ||
        (probe_event[2] != Gecode::Word::ME_WORD_BND) ||
        (probe_event[3] != Gecode::Word::ME_WORD_BND))
      return false;
    for (unsigned int i=0; i<4; i++) probe_count[i]=0;
    if (x.narrow(s,s.x.lo(),s.x.hi() & ~WordValue(1)) !=
        Gecode::Word::ME_WORD_DOM)
      return false;
    (void) s.status();
    if ((probe_count[0] != 0) || (probe_count[1] != 1) ||
        (probe_count[2] != 1) || (probe_count[3] != 1) ||
        (probe_event[1] != Gecode::Word::ME_WORD_DOM))
      return false;
    for (unsigned int i=0; i<4; i++) probe_count[i]=0;
    if (x.narrow_domain(s,4,4,4,4) != Gecode::Word::ME_WORD_VAL)
      return false;
    (void) s.status();
    return (probe_count[0] == 1) && (probe_count[1] == 1) &&
      (probe_count[2] == 1) && (probe_count[3] == 1) &&
      (probe_event[0] == Gecode::Word::ME_WORD_VAL);
  }

  static bool
  deltas(void) {
    using namespace Gecode;
    class DeltaSpace : public Space {
    public:
      WordVar x;
      DeltaSpace(void) : x(*this,4,WDT_UNSIGNED,0,15) {
        DeltaProbe::post(*this,Gecode::Word::WordView(x));
      }
      DeltaSpace(DeltaSpace& s) : Space(s) { x.update(*this,s.x); }
      virtual Space* copy(void) { return new DeltaSpace(*this); }
    };
    DeltaSpace s;
    Gecode::Word::UnsignedWordView x(s.x);
    delta_capture.count=0;
    if (x.narrow_range(s,1,14) != Gecode::Word::ME_WORD_BND)
      return false;
    if ((delta_capture.count != 1) ||
        (delta_capture.event != Gecode::Word::ME_WORD_BND) ||
        (delta_capture.zero != 0) || (delta_capture.one != 0) ||
        (delta_capture.kind != WDT_UNSIGNED) ||
        (delta_capture.old_minimum != 0) ||
        (delta_capture.old_maximum != 15) ||
        (delta_capture.new_minimum != 1) ||
        (delta_capture.new_maximum != 14))
      return false;
    if (x.narrow(s,s.x.lo(),s.x.hi() & ~WordValue(1)) !=
        Gecode::Word::ME_WORD_DOM)
      return false;
    if ((delta_capture.count != 2) ||
        (delta_capture.event != Gecode::Word::ME_WORD_DOM) ||
        (delta_capture.zero != 1) || (delta_capture.one != 0) ||
        (delta_capture.old_minimum != 1) ||
        (delta_capture.old_maximum != 14) ||
        (delta_capture.new_minimum != 2) ||
        (delta_capture.new_maximum != 14))
      return false;
    if (x.narrow_domain(s,4,4,4,4) != Gecode::Word::ME_WORD_VAL)
      return false;
    return (delta_capture.count == 3) &&
      (delta_capture.event == Gecode::Word::ME_WORD_VAL) &&
      (delta_capture.zero == 10) && (delta_capture.one == 4) &&
      (delta_capture.kind == WDT_UNSIGNED) &&
      (delta_capture.old_minimum == 2) &&
      (delta_capture.old_maximum == 14) &&
      (delta_capture.new_minimum == 4) &&
      (delta_capture.new_maximum == 4);
  }

  class Kernel : public Test::Base {
  public:
    Kernel(void) : Test::Base("Word::Bounded::Kernel") {}
    virtual bool run(void) {
      if (!exhaustive()) return false;
      if (!targeted()) { ::Test::olog << "targeted" << std::endl; return false; }
      if (!tells()) { ::Test::olog << "tells" << std::endl; return false; }
      if (!events()) { ::Test::olog << "events" << std::endl; return false; }
      if (!deltas()) { ::Test::olog << "deltas" << std::endl; return false; }
      return true;
    }
  } kernel;

}}}

// STATISTICS: test-word
