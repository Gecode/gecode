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

#include <type_traits>

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
      : x(*this,width,kind,minimum,maximum,cube_bits(width,lo,hi)) {}
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
    // Repeating the current intersection, or supplying a wider one, must
    // leave the synchronized domain and its advisors unchanged.
    if ((x.narrow_domain(s,x.lo(),x.hi(),2,14) !=
         Gecode::Word::ME_WORD_NONE) ||
        (x.narrow_domain(s,0,15,0,15) != Gecode::Word::ME_WORD_NONE) ||
        (delta_capture.count != 2) || (x.lo() != 0) || (x.hi() != 14) ||
        (x.rank_minimum() != 2) || (x.rank_maximum() != 14))
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

  /**
   * Cross-family audit for publication into compact and bounded words.
   *
   * The deliberately small table complements the exhaustive arithmetic
   * oracle: every concrete tuple admitted before posting is checked after
   * propagation, and every complete tuple still admitted by the published
   * domains is checked against the independent relation.  Thus a passing
   * assigned-value check cannot hide either false failure or lost support.
   * There are eight cases in six families; this is not an exhaustive actor
   * matrix (signed domains, wide words, and family-specific corner cases stay
   * with their focused tests).
   */
  class PublicationAudit : public Test::Base {
  private:
    enum class Family { SHIFT, COUNT, CHANNEL, REIFIED, CONDITIONAL, EQUALITY };
    struct Case {
      Family family;
      bool delayed;
      bool mixed;
      bool alias;
      Gecode::ReifyMode mode;
    };

    class AuditSpace : public Gecode::Space {
    public:
      Gecode::WordVar x, y, z;
      Gecode::BoolVar control;
      Gecode::IntVar count;

      AuditSpace(const Case& c, bool actor)
        : x(*this,3,c.mixed ? Gecode::WDT_CUBE : Gecode::WDT_UNSIGNED),
          y(*this,3,Gecode::WDT_UNSIGNED),
          z(*this,3,Gecode::WDT_UNSIGNED), control(*this,0,1),
          count(*this,0,3) {
        if (c.alias)
          y=x;
        if (!c.delayed)
          tells(c);
        if (actor)
          post(c);
        if (c.delayed)
          tells(c);
      }
      AuditSpace(AuditSpace& s) : Gecode::Space(s) {
        x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
        control.update(*this,s.control); count.update(*this,s.count);
      }
      virtual Gecode::Space* copy(void) { return new AuditSpace(*this); }

      void tells(const Case& c) {
        using namespace Gecode;
        Gecode::Word::WordView xv(x), yv(y), zv(z);
        if (((x.domain_type() != WDT_CUBE) &&
             me_failed(xv.narrow_rank_range(*this,1U,6U))) ||
            me_failed(xv.narrow(*this,0U,6U)) ||
            ((Gecode::Word::WordView(x) != yv) &&
             (y.domain_type() != WDT_CUBE) &&
             me_failed(yv.narrow_rank_range(*this,0U,5U))) ||
            ((Gecode::Word::WordView(x) != yv) &&
             me_failed(yv.narrow(*this,1U,7U))) ||
            me_failed(zv.narrow_rank_range(*this,1U,6U)))
          fail();
        if (c.delayed && (c.family == Family::CONDITIONAL))
          rel(*this,control,IRT_EQ,1);
      }
      void post(const Case& c) {
        using namespace Gecode;
        switch (c.family) {
        case Family::SHIFT: shift_left(*this,x,1U,z); break;
        case Family::COUNT: popcount(*this,x,count); break;
        case Family::CHANNEL: channel(*this,x,1,control); break;
        case Family::REIFIED:
          if (c.mode == RM_IMP)
            rel(*this,y,WRT_UGQ,x,Reify(control,c.mode));
          else
            rel(*this,x,WRT_ULE,y,Reify(control,c.mode));
          break;
        case Family::CONDITIONAL: ite(*this,control,x,y,z); break;
        case Family::EQUALITY: rel(*this,x,WRT_EQ,y); break;
        }
      }
    };

    static bool truth(const Case& c, WordValue x, WordValue y,
                      WordValue z, int auxiliary) {
      switch (c.family) {
      case Family::SHIFT: return z == ((x << 1U) & 7U);
      case Family::COUNT: {
        unsigned int n=0;
        for (WordValue v=x; v != 0U; v &= v-1U) n++;
        return auxiliary == static_cast<int>(n);
      }
      case Family::CHANNEL: return auxiliary == static_cast<int>((x >> 1) & 1U);
      case Family::REIFIED: {
        const bool relation=x <= y;
        if (c.mode == Gecode::RM_EQV) return auxiliary == relation;
        if (c.mode == Gecode::RM_IMP) return !auxiliary || relation;
        return relation || auxiliary;
      }
      case Family::CONDITIONAL: return z == (auxiliary ? x : y);
      case Family::EQUALITY: return x == y;
      }
      return false;
    }

    static bool check(const Case& c, unsigned int& supported,
                      unsigned int& rejected) {
      using namespace Gecode;
      AuditSpace initial(c,false);
      AuditSpace actor(c,true);
      const bool initial_failed=initial.status() == SS_FAILED;
      const bool actor_failed=actor.status() == SS_FAILED;
      if (initial_failed || actor_failed) {
        ::Test::olog << "root failure initial=" << initial_failed
                     << " actor=" << actor_failed << std::endl;
        return false;
      }
      // Exercise cloning before the complete-assignment probes.  With a
      // recomputation distance of one in the focused family tests, these same
      // actors also cover archive/replay; this audit owns only publication.
      AuditSpace* published=static_cast<AuditSpace*>(actor.clone());
      if (published->status() == SS_FAILED) {
        ::Test::olog << "clone failure" << std::endl;
        delete published; return false;
      }

      const bool uses_y=(c.family == Family::REIFIED) ||
        (c.family == Family::CONDITIONAL) || (c.family == Family::EQUALITY);
      const bool uses_z=(c.family == Family::SHIFT) || (c.family == Family::CONDITIONAL);
      const bool uses_aux=(c.family == Family::COUNT) || (c.family == Family::CHANNEL) ||
        (c.family == Family::REIFIED) || (c.family == Family::CONDITIONAL);
      const int aux_max=(c.family == Family::COUNT) ? 3 : 1;
      for (WordValue xv=0; xv<8U; xv++)
        for (WordValue yv=0; yv<(uses_y ? 8U : 1U); yv++)
          for (WordValue zv=0; zv<(uses_z ? 8U : 1U); zv++)
            for (int av=0; av<=(uses_aux ? aux_max : 0); av++) {
              const bool admitted=initial.x.in(xv) &&
                (!uses_y || initial.y.in(yv)) &&
                (!uses_z || initial.z.in(zv)) &&
                (!uses_aux || ((c.family == Family::COUNT) ? initial.count.in(av) :
                               initial.control.in(av)));
              if (!admitted) continue;
              const bool valid=truth(c,xv,yv,zv,av);
              if (valid) {
                supported++;
                if (!published->x.in(xv) ||
                    (uses_y && !published->y.in(yv)) ||
                    (uses_z && !published->z.in(zv)) ||
                    (uses_aux && ((c.family == Family::COUNT) ?
                     !published->count.in(av) : !published->control.in(av)))) {
                  ::Test::olog << "lost support " << xv << ' ' << yv << ' '
                               << zv << ' ' << av << std::endl;
                  delete published;
                  return false;
                }
              }
              AuditSpace* assigned=static_cast<AuditSpace*>(published->clone());
              dom(*assigned,assigned->x,xv);
              if (uses_y) dom(*assigned,assigned->y,yv);
              if (uses_z) dom(*assigned,assigned->z,zv);
              if (uses_aux) {
                if (c.family == Family::COUNT) rel(*assigned,assigned->count,IRT_EQ,av);
                else rel(*assigned,assigned->control,IRT_EQ,av);
              }
              const bool survives=assigned->status() != SS_FAILED;
              delete assigned;
              if (survives != valid) {
                ::Test::olog << "assignment mismatch " << xv << ' ' << yv
                             << ' ' << zv << ' ' << av << " survives="
                             << survives << " valid=" << valid << std::endl;
                delete published; return false;
              }
              if (!valid) rejected++;
            }
      delete published;
      return true;
    }

  public:
    PublicationAudit(void) : Test::Base("Word::Bounded::PublicationAudit") {}
    virtual bool run(void) {
      const Case cases[] = {
        {Family::SHIFT,false,false,false,Gecode::RM_EQV},
        {Family::SHIFT,true,true,false,Gecode::RM_EQV},
        {Family::COUNT,true,false,false,Gecode::RM_EQV},
        {Family::CHANNEL,false,false,false,Gecode::RM_EQV},
        {Family::REIFIED,true,true,false,Gecode::RM_EQV},
        {Family::REIFIED,false,false,false,Gecode::RM_IMP},
        {Family::CONDITIONAL,true,true,false,Gecode::RM_EQV},
        {Family::EQUALITY,false,true,true,Gecode::RM_EQV}
      };
      unsigned int supported=0, rejected=0;
      unsigned int index=0;
      for (const Case& c : cases) {
        if (!check(c,supported,rejected)) {
          ::Test::olog << "publication audit case " << index << std::endl;
          return false;
        }
        index++;
      }
      // Freeze the table and its concrete coverage, not implementation detail.
      if ((supported != 47U) || (rejected != 143U)) {
        ::Test::olog << "publication audit counts " << supported << ' '
                     << rejected << std::endl;
        return false;
      }
      return true;
    }
  } publication_audit;

  /// Regression checks for synchronization across two domain representations.
  class Closure : public Test::Base {
    class Model : public Gecode::Space {
    public:
      Gecode::WordVarArray x;
      Gecode::IntVar y;
      explicit Model(int test) : x(*this,3), y(*this,0,63) {
        using namespace Gecode;
        x[2]=WordVar(*this,1,{0});
        if (test == 0) {
          x[0]=WordVar(*this,4,WDT_UNSIGNED,3,4,known_bits(0b1000,0b0000));
          x[1]=WordVar(*this,4,WDT_SIGNED,10,2,known_bits(0b0011,0b0010));
          rel(*this,x[0],WRT_SLQ,x[1]);
        } else if (test == 1) {
          x[0]=WordVar(*this,3,WDT_UNSIGNED,2,4);
          x[1]=WordVar(*this,3,WDT_UNSIGNED,3,5,known_bits(0b001,0b001));
          x[2]=WordVar(*this,6,WDT_UNSIGNED,13,33,known_bits(0b000011,0b000001));
          concat(*this,x[0],x[1],x[2]);
        } else if (test == 2) {
          x[0]=WordVar(*this,6,WDT_UNSIGNED,0,63,known_bits(0b000001,0b000000));
          x[1]=WordVar(*this,1,{0});
          int values[34];
          for (int i=0; i<31; i++) values[i]=2*i+1;
          for (int i=0; i<3; i++) values[31+i]=16*(i+1);
          y=IntVar(*this,IntSet(values,34));
          channel(*this,x[0],y,WI_UNSIGNED);
        } else {
          x[0]=WordVar(*this,3,{0},WDT_UNSIGNED);
          x[1]=WordVar(*this,3,WDT_UNSIGNED,0,3,known_bits(0b100,0b000));
          distinct(*this,WordVarArgs({x[0],x[1]}),IPL_BND);
        }
      }
      Model(Model& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
      }
      Gecode::Space* copy(void) override { return new Model(*this); }
    };
  public:
    Closure(void) : Test::Base("Word::Bounded::Closure") {}
    bool run(void) override {
      using namespace Gecode;
      Model ordering(0);
      if (ordering.status() != SS_FAILED) return false;
      Model concat(1);
      if ((concat.status() == SS_FAILED) ||
          (concat.x[2].lo() != 21U) || (concat.x[2].hi() != 29U))
        return false;
      Model channel(2);
      if ((channel.status() == SS_FAILED) ||
          (channel.y.min() != 16) || (channel.y.max() != 48) ||
          (channel.x[0].minimum() != 16U) ||
          (channel.x[0].maximum() != 48U))
        return false;
      Model distinct(3);
      if ((distinct.status() == SS_FAILED) ||
          (distinct.x[1].minimum() != 1U))
        return false;
      return true;
    }
  } closure;

  /// Every deduced bound move applies, at any width.
  class ExactNarrowing : public Test::Base {
    /// Contradictory strict orderings on two words of one width
    class Cycle : public Gecode::Space {
    public:
      Gecode::WordVar x, y;
      explicit Cycle(unsigned int width)
        : x(*this,width,Gecode::WDT_UNSIGNED),
          y(*this,width,Gecode::WDT_UNSIGNED) {
        using namespace Gecode;
        rel(*this,x,WRT_ULE,y);
        rel(*this,y,WRT_ULE,x);
        branch(*this,WordVarArgs({x,y}),WORD_VAR_NONE(),WORD_VAL_MSB());
      }
      Cycle(Cycle& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
      }
      Gecode::Space* copy(void) override { return new Cycle(*this); }
    };
    /// A 64-bit word with cube [0,hi] and the full interval of its kind
    class Cube : public Gecode::Space {
    public:
      Gecode::WordVar x;
      Cube(Gecode::WordValue hi, Gecode::WordDomainType kind)
        : x(*this,64,kind,
            (kind == Gecode::WDT_SIGNED) ? sign : 0,
            (kind == Gecode::WDT_SIGNED) ? sign-1 : ~Gecode::WordValue(0),
            cube_bits(64,0,hi)) {}
      Cube(Cube& s) : Gecode::Space(s) { x.update(*this,s.x); }
      Gecode::Space* copy(void) override { return new Cube(*this); }
    };
    /// 64-bit words of one kind
    class Words : public Gecode::Space {
    public:
      Gecode::WordVarArray x;
      explicit Words(int n) : x(*this,n) {}
      Words(Words& s) : Gecode::Space(s) { x.update(*this,s.x); }
      Gecode::Space* copy(void) override { return new Words(*this); }
    };
    static const Gecode::WordValue sign=Gecode::WordValue(1) << 63;
    /// Whether x > 0 on the cube {0,value} of \a kind assigns x to value
    static bool assigns_above_zero(Gecode::WordValue value,
                                   Gecode::WordDomainType kind) {
      using namespace Gecode;
      Cube s(value,kind);
      rel(s,s.x,(kind == WDT_SIGNED) ? WRT_SGR : WRT_UGR,0);
      return (s.status() != SS_FAILED) && s.x.assigned() &&
        (s.x.val() == value);
    }
    /// Whether x \a wrt y with y below the greatest value moves the facing
    /// endpoints of the full 64-bit x and y by one or two values
    static bool orders_exactly(Gecode::WordRelType wrt, bool is_strict) {
      using namespace Gecode;
      const bool is_signed=(wrt == WRT_SLQ) || (wrt == WRT_SLE);
      const WordDomainType kind=is_signed ? WDT_SIGNED : WDT_UNSIGNED;
      const WordValue least=is_signed ? sign : 0U;
      const WordValue greatest=is_signed ? sign-1U : ~WordValue(0);
      Words s(2);
      s.x[0]=WordVar(s,64,kind);
      s.x[1]=WordVar(s,64,kind,least,greatest-1U);
      rel(s,s.x[0],wrt,s.x[1]);
      const WordValue step=is_strict ? 1U : 0U;
      return (s.status() != SS_FAILED) &&
        (s.x[0].maximum() == greatest-1U-step) &&
        (s.x[1].minimum() == least+step);
    }
    /// Whether bounds distinct raises a third word above a two-value Hall
    /// interval at the start of a 2^63-value interval
    static bool raises_above_hall_interval(void) {
      using namespace Gecode;
      Words s(3);
      s.x[0]=WordVar(s,64,WDT_UNSIGNED,sign,sign+1U);
      s.x[1]=WordVar(s,64,WDT_UNSIGNED,sign,sign+1U);
      s.x[2]=WordVar(s,64,WDT_UNSIGNED,sign,~WordValue(0));
      distinct(s,s.x,IPL_BND);
      return (s.status() != SS_FAILED) && (s.x[2].minimum() == sign+2U);
    }
  public:
    ExactNarrowing(void) : Test::Base("Word::Bounded::ExactNarrowing") {}
    bool run(void) override {
      using namespace Gecode;
      Cycle narrow(8);
      Cycle wide(18);
      if ((narrow.status() != SS_FAILED) || (wide.status() != SS_FAILED))
        return false;
      if (!assigns_above_zero(sign,WDT_UNSIGNED) ||
          !assigns_above_zero(sign >> 1,WDT_SIGNED))
        return false;
      if (!orders_exactly(WRT_ULQ,false) || !orders_exactly(WRT_ULE,true) ||
          !orders_exactly(WRT_SLQ,false) || !orders_exactly(WRT_SLE,true))
        return false;
      if (!raises_above_hall_interval())
        return false;
      // Closure of the cube {0..3, 2^63..2^63+3} lowers the maximum to 3.
      Cube sparse(sign | 3,WDT_UNSIGNED);
      Gecode::Word::UnsignedWordView x(sparse.x);
      const ModEvent me=x.narrow_range(sparse,1,sign >> 1);
      return !me_failed(me) && (x.rank_minimum() == 1) &&
        (x.rank_maximum() == 3);
    }
  } exact_narrowing;

  /*
   * Overload checks for the construction forms. Raw lo/hi masks construct
   * no word, floating-point endpoints are rejected at compile time on every
   * integer-endpoint form, and a braced pair is a value list, since WordBits
   * comes only from known_bits.
   */
  template<class T, class... Args>
  constexpr bool constructs = std::is_constructible_v<T,Args...>;

  using Gecode::Space;
  using Gecode::WordBits;
  using Gecode::WordVar;
  using Gecode::WordVarArgs;
  using Gecode::WordVarArray;
  using Gecode::WordValueSetOption;

  static_assert(!constructs<WordVar,Space&,unsigned int,WordValue,WordValue>);
  static_assert(!constructs<WordVar,Space&,int,int,int>);
  static_assert(!constructs<WordVar,Space&,unsigned int,WordValue,WordValue,
                            WordDomainType,WordValue,WordValue>);
  static_assert(!constructs<WordVarArgs,Space&,int,unsigned int,WordValue,
                            WordValue>);
  static_assert(!constructs<WordVarArgs,Space&,int,unsigned int,WordValue,
                            WordValue,WordDomainType,WordValue,WordValue>);
  static_assert(!constructs<WordVarArray,Space&,int,unsigned int,WordValue,
                            WordValue>);
  static_assert(!constructs<WordVarArray,Space&,int,int,int,int>);
  static_assert(!constructs<WordVarArray,Space&,int,unsigned int,WordValue,
                            WordValue,WordDomainType,WordValue,WordValue>);

  static_assert(!constructs<WordVar,Space&,int,WordDomainType,double,double>);
  static_assert(!constructs<WordVar,Space&,int,WordDomainType,int,double>);
  static_assert(!constructs<WordVar,Space&,int,WordDomainType,float,int>);
  static_assert(!constructs<WordVar,Space&,int,WordDomainType,double,double,
                            WordValueSetOption>);
  static_assert(!constructs<WordVar,Space&,int,WordDomainType,double,double,
                            WordBits>);
  static_assert(!constructs<WordVar,Space&,int,WordDomainType,int,double,
                            WordBits,WordValueSetOption>);
  static_assert(!constructs<WordVarArgs,Space&,int,int,WordDomainType,double,
                            double>);
  static_assert(!constructs<WordVarArgs,Space&,int,int,WordDomainType,double,
                            int,WordBits>);
  static_assert(!constructs<WordVarArray,Space&,int,int,WordDomainType,double,
                            double>);
  static_assert(!constructs<WordVarArray,Space&,int,int,WordDomainType,int,
                            double,WordBits>);

  static_assert(constructs<WordVar,Space&,int,WordDomainType,int,int>);
  static_assert(constructs<WordVar,Space&,int,WordDomainType,int,int,WordBits>);
  static_assert(constructs<WordVarArgs,Space&,int,int,WordDomainType,int,int>);
  static_assert(constructs<WordVarArgs,Space&,int,int,WordDomainType,int,int,
                           WordBits>);
  static_assert(constructs<WordVarArray,Space&,int,int,WordDomainType,int,int,
                           WordBits>);
  static_assert(constructs<WordVar,Space&,WordDomainType,double,double>);
  static_assert(constructs<WordVar,Space&,WordDomainType,int,int>);
  static_assert(constructs<WordVarArray,Space&,int,WordDomainType,double,
                           double>);
  static_assert(constructs<WordVarArgs,Space&,int,int>);
  static_assert(constructs<WordVarArray,Space&,int,int>);

  static_assert(!std::is_constructible_v<WordBits,WordValue,WordValue>);
  static_assert(!std::is_default_constructible_v<WordBits>);

  /// Whether \a B can be list-initialized from two integers
  template<class B, class = void>
  struct pair_initializes : std::false_type {};
  template<class B>
  struct pair_initializes<B,std::void_t<decltype(B{0,1})>>
    : std::true_type {};
  static_assert(!pair_initializes<WordBits>::value);
  static_assert(pair_initializes<Gecode::WordValArgs>::value);

  /// Whether a word can be created from a braced pair
  template<class S, class = void>
  struct pair_constructs : std::false_type {};
  template<class S>
  struct pair_constructs<S,std::void_t<decltype(
    WordVar(std::declval<S&>(),30U,{0,1}))>> : std::true_type {};
  static_assert(pair_constructs<Space>::value);

  /// Construction from endpoints, value lists, and known bits
  class Construction : public Test::Base {
    /// Space holding one word
    class One : public Gecode::Space {
    public:
      WordVar x;
      One(void) {}
      One(One& s) : Gecode::Space(s) {
        x.update(*this,s.x);
      }
      virtual Gecode::Space* copy(void) { return new One(*this); }
    };
    /// Space holding one word of \a width with the values 1, 3, and 4
    class Listed : public Gecode::Space {
    public:
      WordVar x;
      Listed(unsigned int width, WordValueSetOption value_set)
        : x(*this,width,{1,3,4},Gecode::WDT_UNSIGNED,value_set) {
        Gecode::branch(*this,x,Gecode::WORD_VAL_LSB());
      }
      Listed(Listed& s) : Gecode::Space(s) {
        x.update(*this,s.x);
      }
      virtual Gecode::Space* copy(void) { return new Listed(*this); }
    };

    /// Test whether \a s accepts \a value as the value of its word
    static bool
    accepts(Listed& s, WordValue value) {
      if (s.status() == Gecode::SS_FAILED)
        return false;
      Listed* c = static_cast<Listed*>(s.clone());
      Gecode::dom(*c,c->x,value);
      const bool ok = c->status() != Gecode::SS_FAILED;
      delete c;
      return ok;
    }

    /// Endpoints in the word's interpretation, by the fit rule
    static bool
    endpoints(void) {
      using namespace Gecode;
      One s;
      const WordVar small(s,8,WDT_SIGNED,-3,5);
      if ((small.minimum() != 0xfd) || (small.maximum() != 5) ||
          small.in(0xfc) || !small.in(0xfd) || !small.in(5) || small.in(6))
        return false;
      const WordVar negative(s,4,WDT_SIGNED,-4,3);
      const WordVar pattern(s,4,WDT_SIGNED,12,3);
      if ((negative.minimum() != 12) || (pattern.minimum() != 12) ||
          (negative.maximum() != 3) || (pattern.maximum() != 3))
        return false;
      const WordVarArray array(s,2,4,WDT_SIGNED,-8,7);
      for (int i=0; i<array.size(); i++)
        if ((array[i].minimum() != 8) || (array[i].maximum() != 7) ||
            (array[i].lo() != 0) || (array[i].hi() != 15))
          return false;
      const WordVar even(s,4,WDT_UNSIGNED,6,10,known_bits(0b0001,0));
      for (WordValue v=0; v<16; v++)
        if (even.in(v) != ((v >= 6) && (v <= 10) && ((v & 1) == 0)))
          return false;
      try {
        WordVar(s,4,WDT_UNSIGNED,0,16);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        WordVar(s,4,WDT_SIGNED,-9,0);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        WordVar(s,4,WDT_UNSIGNED,6,7,known_bits(0b0110,0));
        return false;
      } catch (const Gecode::Word::VariableEmptyDomain&) {}
      try {
        WordVar(s,4,WDT_UNSIGNED,7,6);
        return false;
      } catch (const Gecode::Word::VariableEmptyDomain&) {}
      return true;
    }

    /// Value lists give exact membership
    static bool
    values(void) {
      using namespace Gecode;
      Listed exact(8,WVS_EXACT);
      if (!exact.x.has_value_set() || exact.x.in(2) || !exact.x.in(3))
        return false;
      Listed hull(8,WVS_NONE);
      if (hull.x.has_value_set() || !hull.x.in(2) || accepts(hull,2) ||
          !accepts(hull,3))
        return false;
      Listed* wide = new Listed(16,WVS_DEFAULT);
      if (!wide->x.in(2) || accepts(*wide,2) || !accepts(*wide,3)) {
        delete wide;
        return false;
      }
      DFS<Listed> search(wide);
      delete wide;
      WordValue found = 0;
      int n = 0;
      while (Listed* solution = search.next()) {
        if (solution->x.val() < 64)
          found |= WordValue(1) << solution->x.val();
        n++;
        delete solution;
      }
      if ((n != 3) || (found != 0b11010))
        return false;
      One s;
      const WordVar carry(s,30,{0,1});
      if ((carry.lo() != 0) || (carry.hi() != 1))
        return false;
      try {
        WordVar(s,8,WordValArgs());
        return false;
      } catch (const Gecode::Word::VariableEmptyDomain&) {}
      return true;
    }

    /// Known bits fix bits for construction and for dom()
    static bool
    known(void) {
      using namespace Gecode;
      try {
        (void) known_bits(0b0001,0b1001);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      One s;
      try {
        WordVar(s,4,known_bits(0b10000,0));
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      const WordVar flags(s,4,known_bits(0b1001,0b0001));
      if ((flags.lo() != 0b0001) || (flags.hi() != 0b0111))
        return false;
      s.x = WordVar(s,4);
      dom(s,s.x,known_bits(0b1001,0b0001));
      if ((s.status() == SS_FAILED) || (s.x.lo() != 0b0001) ||
          (s.x.hi() != 0b0111))
        return false;
      try {
        dom(s,s.x,known_bits(0b10000,0));
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      WordVarArgs words(s,2,4);
      dom(s,words,known_bits(0b0011,0b0010));
      if ((s.status() == SS_FAILED) || (words[0].lo() != 0b0010) ||
          (words[1].hi() != 0b1110))
        return false;
      dom(s,s.x,known_bits(0b1000,0b1000));
      return s.status() == SS_FAILED;
    }
  public:
    Construction(void) : Test::Base("Word::Bounded::Construction") {}
    bool run(void) override {
      if (!endpoints()) { ::Test::olog << "endpoints" << std::endl; return false; }
      if (!values()) { ::Test::olog << "values" << std::endl; return false; }
      if (!known()) { ::Test::olog << "known" << std::endl; return false; }
      return true;
    }
  } construction;

}}}

// STATISTICS: test-word
