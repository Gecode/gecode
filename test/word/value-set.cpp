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

#include <algorithm>
#include <memory>
#include <vector>

namespace Test { namespace Word { namespace ValueSet {

  using Gecode::WordValue;
  using Gecode::WordDomainType;
  using Gecode::WDT_CUBE;
  using Gecode::WDT_UNSIGNED;
  using Gecode::WDT_SIGNED;
  using Gecode::WVS_EXACT;
  using Gecode::WVS_NONE;
  using Gecode::Word::WordValueSet;
  using Gecode::Word::WordView;
  using Gecode::Word::ME_WORD_FAILED;
  using Gecode::Word::ME_WORD_NONE;
  using Gecode::Word::ME_WORD_VAL;
  using Gecode::Word::ME_WORD_BITS;
  using Gecode::Word::ME_WORD_BND;
  using Gecode::Word::ME_WORD_DOM;
  using Gecode::Word::ME_WORD_HOLE;

  /// Space holding one word
  class WordSpace : public Gecode::Space {
  public:
    Gecode::WordVar x;
    WordSpace(void) {}
    WordSpace(WordSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
    }
    virtual Gecode::Space* copy(void) { return new WordSpace(*this); }
  };

  static WordValue
  sign_flip(WordDomainType kind, unsigned int width) {
    return (kind == WDT_SIGNED) ? (WordValue(1) << (width-1)) : 0;
  }

  static bool
  cube_admits(WordValue lo, WordValue hi, WordValue value) {
    return ((value & lo) == lo) && ((value & ~hi) == 0);
  }

  static bool
  set_contains(const WordValueSet& set, WordValue value) {
    return ((set.blocks[value / 64] >> (value % 64)) & 1U) != 0;
  }

  /// Independent reference domain with one flag per value
  struct Reference {
    unsigned int width;
    WordDomainType kind;
    std::vector<bool> member;

    WordValue count(void) const { return WordValue(1) << width; }
    WordValue mask(void) const { return count()-1; }
    bool bounded(void) const { return kind != WDT_CUBE; }
    WordValue rank(WordValue value) const {
      return value ^ sign_flip(kind,width);
    }
  };

  /// Hull, rank extremes and size of a reference domain
  struct Summary {
    unsigned int size;
    WordValue lo, hi;
    WordValue rank_minimum, rank_maximum;
  };

  static Summary
  summarize(const Reference& r) {
    Summary s = {0,~WordValue(0),0,~WordValue(0),0};
    for (WordValue v=0; v<r.count(); v++)
      if (r.member[v]) {
        s.size++;
        s.lo &= v;
        s.hi |= v;
        s.rank_minimum = std::min(s.rank_minimum,r.rank(v));
        s.rank_maximum = std::max(s.rank_maximum,r.rank(v));
      }
    return s;
  }

  /// Return the event that changing \a before to its subset \a after reports
  static Gecode::ModEvent
  expected_event(const Reference& r, const Summary& before,
                 const Summary& after) {
    if (after.size == 0)
      return ME_WORD_FAILED;
    if (after.size == before.size)
      return ME_WORD_NONE;
    if (after.size == 1)
      return ME_WORD_VAL;
    const bool has_new_bits =
      (after.lo != before.lo) || (after.hi != before.hi);
    const bool has_new_bounds = r.bounded() &&
      ((after.rank_minimum != before.rank_minimum) ||
       (after.rank_maximum != before.rank_maximum));
    if (has_new_bits && has_new_bounds)
      return ME_WORD_DOM;
    if (has_new_bits)
      return ME_WORD_BITS;
    if (has_new_bounds)
      return ME_WORD_BND;
    return ME_WORD_HOLE;
  }

  static WordValueSet
  reference_set(const Reference& r) {
    WordValueSet set = {{0,0,0,0}};
    for (WordValue v=0; v<r.count(); v++)
      if (r.member[v])
        set.blocks[v / 64] |= WordValue(1) << (v % 64);
    return set;
  }

  /// Test whether \a x has exactly the domain of \a r
  static bool
  agrees(const Gecode::WordVar& x, const Reference& r) {
    const Summary s = summarize(r);
    if (!x.has_value_set() || (x.lo() != s.lo) || (x.hi() != s.hi) ||
        (x.assigned() != (s.size == 1)))
      return false;
    if (r.bounded() &&
        ((x.minimum() != r.rank(s.rank_minimum)) ||
         (x.maximum() != r.rank(s.rank_maximum))))
      return false;
    for (WordValue v=0; v<r.count(); v++)
      if (x.in(v) != r.member[v])
        return false;
    if (x.in(r.count()))
      return false;
    const WordValueSet values = WordView(x).values();
    const WordValueSet expected = reference_set(r);
    for (unsigned int i=0; i<4; i++)
      if (values.blocks[i] != expected.blocks[i])
        return false;
    return true;
  }

  /// Restrict \a r to cube \a lo, \a hi and the rank interval
  static void
  restrict_reference(Reference& r, WordValue lo, WordValue hi,
                     WordValue minimum, WordValue maximum) {
    for (WordValue v=0; v<r.count(); v++)
      if (!cube_admits(lo,hi,v) || (r.rank(v) < minimum) ||
          (r.rank(v) > maximum))
        r.member[v] = false;
  }

  static Gecode::ModEvent
  narrow_domain(Gecode::Space& home, Gecode::WordVar x, WordValue lo,
                WordValue hi, WordValue minimum, WordValue maximum) {
    if (x.domain_type() == WDT_SIGNED)
      return Gecode::Word::SignedWordView(x).
        narrow_domain(home,lo,hi,minimum,maximum);
    return Gecode::Word::UnsignedWordView(x).
      narrow_domain(home,lo,hi,minimum,maximum);
  }

  static Gecode::ModEvent
  narrow_range(Gecode::Space& home, Gecode::WordVar x,
               WordValue minimum, WordValue maximum) {
    if (x.domain_type() == WDT_SIGNED)
      return Gecode::Word::SignedWordView(x).
        narrow_range(home,minimum,maximum);
    return Gecode::Word::UnsignedWordView(x).
      narrow_range(home,minimum,maximum);
  }

  /// Tells applied by the randomised test
  enum class Tell { CUBE, CUBE_AND_RANKS, RANKS, EQ, NQ, VALUES };

  /// One word under test, with its own reference
  struct Run {
    std::unique_ptr<WordSpace> space;
    Reference reference;
  };

  /// Return a clone of the stable \a run with a copy of its reference
  static Run
  clone(const Run& run) {
    Run copy;
    copy.space.reset(static_cast<WordSpace*>(run.space->clone()));
    copy.reference = run.reference;
    return copy;
  }

  /**
   * Randomised tells on words with a value set, compared after every tell
   * with an independent reference set. Clones are told independently.
   */
  class Domain : public Test::Base {
  public:
    Domain(void) : Test::Base("Word::ValueSet::Domain") {}

    virtual bool run(void) {
      const WordDomainType kinds[] = {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED};
      const unsigned int widths[] = {1,2,3,5,6,7,8};
      for (WordDomainType kind : kinds)
        for (unsigned int width : widths)
          if (!run_sequence(kind,width)) {
            olog << "kind " << static_cast<int>(kind)
                 << " width " << width << std::endl;
            return false;
          }
      return true;
    }

  private:
    bool run_sequence(WordDomainType kind, unsigned int width) {
      std::vector<Run> runs;
      runs.push_back(create(kind,width));
      if (!agrees(runs[0].space->x,runs[0].reference)) {
        olog << "creation" << std::endl;
        return false;
      }
      for (unsigned int step=0; (step < 30) && !runs.empty(); step++) {
        const unsigned int i = _rand(static_cast<unsigned int>(runs.size()));
        if ((_rand(8U) == 0) && (runs.size() < 3)) {
          if (runs[i].space->status() == Gecode::SS_FAILED)
            return false;
          runs.push_back(clone(runs[i]));
        }
        const Summary before = summarize(runs[i].reference);
        const Tell tell = choose_tell(runs[i].reference);
        const Gecode::ModEvent me = apply(tell,runs[i]);
        const Summary after = summarize(runs[i].reference);
        if (me != expected_event(runs[i].reference,before,after)) {
          olog << "tell " << static_cast<int>(tell) << " event " << me
               << std::endl;
          return false;
        }
        if (me == ME_WORD_FAILED)
          runs.erase(runs.begin()+i);
        for (const Run& r : runs)
          if (!agrees(r.space->x,r.reference)) {
            olog << "tell " << static_cast<int>(tell) << " domain"
                 << std::endl;
            return false;
          }
      }
      return true;
    }

    /// Return a random cube that fixes one or two bits of \a width
    void random_cube(unsigned int width, WordValue& lo, WordValue& hi) {
      lo = 0;
      hi = Gecode::Word::width_mask(width);
      const unsigned int fixed = 1U + _rand(2U);
      for (unsigned int k=0; k<fixed; k++) {
        const WordValue bit = WordValue(1) << _rand(width);
        if (_rand(2U) != 0)
          lo |= bit;
        else
          hi &= ~bit;
      }
    }

    /// Return a random rank interval, mostly cutting the current extremes
    void random_ranks(const Reference& r, WordValue& minimum,
                      WordValue& maximum) {
      if (_rand(4U) == 0) {
        minimum = _rand(static_cast<unsigned int>(r.count()));
        maximum = minimum +
          _rand(static_cast<unsigned int>(r.count()-minimum));
        return;
      }
      const Summary s = summarize(r);
      const WordValue cut_minimum = _rand(3U);
      const WordValue cut_maximum = _rand(3U);
      minimum = std::min(s.rank_minimum+cut_minimum,r.mask());
      maximum = (s.rank_maximum >= cut_maximum) ?
        (s.rank_maximum-cut_maximum) : 0;
    }

    /// Return a member of \a r most of the time, otherwise any value
    WordValue random_value(const Reference& r) {
      if (_rand(4U) != 0) {
        std::vector<WordValue> members;
        for (WordValue v=0; v<r.count(); v++)
          if (r.member[v])
            members.push_back(v);
        if (!members.empty())
          return members[_rand(static_cast<unsigned int>(members.size()))];
      }
      return _rand(static_cast<unsigned int>(r.count()));
    }

    Run create(WordDomainType kind, unsigned int width) {
      Run run;
      run.space.reset(new WordSpace);
      WordSpace& home = *run.space;
      run.reference = {width,kind,
                       std::vector<bool>(WordValue(1) << width,true)};
      Reference& r = run.reference;
      WordValue lo, hi, minimum, maximum;
      random_cube(width,lo,hi);
      random_ranks(r,minimum,maximum);
      const WordValue flip = sign_flip(kind,width);
      try {
        switch (_rand(3U)) {
        case 0:
          home.x = (kind == WDT_CUBE) ?
            Gecode::WordVar(home,width,WVS_EXACT) :
            Gecode::WordVar(home,width,kind,WVS_EXACT);
          return run;
        case 1:
          if (kind == WDT_CUBE) {
            home.x = Gecode::WordVar(home,width,WDT_CUBE,WVS_EXACT);
            return run;
          }
          home.x = Gecode::WordVar(home,width,kind,minimum ^ flip,
                                   maximum ^ flip,WVS_EXACT);
          restrict_reference(r,0,r.mask(),minimum,maximum);
          return run;
        default:
          if (kind == WDT_CUBE) {
            home.x = Gecode::WordVar(home,width,cube_bits(width,lo,hi),WVS_EXACT);
            restrict_reference(r,lo,hi,0,r.mask());
            return run;
          }
          home.x = Gecode::WordVar(home,width,kind,minimum ^ flip,
                                   maximum ^ flip,cube_bits(width,lo,hi),
                                   WVS_EXACT);
          restrict_reference(r,lo,hi,minimum,maximum);
          return run;
        }
      } catch (const Gecode::Word::VariableEmptyDomain&) {
        r.member.assign(r.count(),true);
        home.x = Gecode::WordVar(home,width,kind,WVS_EXACT);
        return run;
      }
    }

    Tell choose_tell(const Reference& r) {
      const Tell cube_tells[] = {Tell::CUBE,Tell::EQ,Tell::NQ,Tell::NQ,
                                 Tell::VALUES};
      const Tell bounded_tells[] = {Tell::CUBE,Tell::CUBE_AND_RANKS,Tell::RANKS,
                                    Tell::EQ,Tell::NQ,Tell::NQ,
                                    Tell::VALUES};
      if (r.bounded())
        return bounded_tells[_rand(7U)];
      return cube_tells[_rand(5U)];
    }

    /// Apply \a tell to the word of \a run and the same filter to its reference
    Gecode::ModEvent apply(Tell tell, Run& run) {
      WordSpace& home = *run.space;
      Reference& r = run.reference;
      WordView x(home.x);
      WordValue lo, hi, minimum, maximum;
      switch (tell) {
      case Tell::CUBE:
        random_cube(r.width,lo,hi);
        restrict_reference(r,lo,hi,0,r.mask());
        return x.narrow(home,lo,hi);
      case Tell::CUBE_AND_RANKS:
        random_cube(r.width,lo,hi);
        random_ranks(r,minimum,maximum);
        restrict_reference(r,lo,hi,minimum,maximum);
        return narrow_domain(home,home.x,lo,hi,minimum,maximum);
      case Tell::RANKS:
        random_ranks(r,minimum,maximum);
        restrict_reference(r,0,r.mask(),minimum,maximum);
        return narrow_range(home,home.x,minimum,maximum);
      case Tell::EQ: {
        const WordValue v = random_value(r);
        for (WordValue u=0; u<r.count(); u++)
          if (u != v)
            r.member[u] = false;
        return x.eq(home,v);
      }
      case Tell::NQ: {
        const WordValue v = random_value(r);
        r.member[v] = false;
        return x.nq(home,v);
      }
      case Tell::VALUES: {
        // Candidates also cover values beyond the width, which must be ignored.
        const unsigned int density = 2U + _rand(6U);
        WordValueSet candidates = {{0,0,0,0}};
        for (WordValue v=0; v<256; v++)
          if (_rand(density) != 0)
            candidates.blocks[v / 64] |= WordValue(1) << (v % 64);
        for (WordValue v=0; v<r.count(); v++)
          if (!set_contains(candidates,v))
            r.member[v] = false;
        return x.narrow_values(home,candidates);
      }
      }
      GECODE_NEVER;
      return ME_WORD_FAILED;
    }
  } domain;

  /// Probe counters, used only by the Kernel test
  static unsigned int probe_count[4];
  static Gecode::ModEvent probe_event[4];

  template<Gecode::PropCond pc, unsigned int slot>
  class Probe : public Gecode::UnaryPropagator<WordView,pc> {
  protected:
    Probe(Gecode::Home home, WordView x)
      : Gecode::UnaryPropagator<WordView,pc>(home,x) {}
    Probe(Gecode::Space& home, Probe& p)
      : Gecode::UnaryPropagator<WordView,pc>(home,p) {}
  public:
    static void post(Gecode::Home home, WordView x) {
      (void) new (home) Probe(home,x);
    }
    virtual Gecode::Actor* copy(Gecode::Space& home) {
      return new (home) Probe(home,*this);
    }
    virtual Gecode::ExecStatus propagate(
      Gecode::Space&, const Gecode::ModEventDelta& med) {
      probe_count[slot]++;
      probe_event[slot] = WordView::me(med);
      return Gecode::ES_FIX;
    }
  };

  /// Advisor delta captured by DeltaProbe, used only by the Kernel test
  struct DeltaCapture {
    unsigned int count;
    Gecode::ModEvent event;
    WordValue zero, one;
    WordValue old_minimum, old_maximum, new_minimum, new_maximum;
  } delta_capture;

  class DeltaProbe : public Gecode::Propagator {
  protected:
    WordView x;
    Gecode::Council<Gecode::ViewAdvisor<WordView> > c;
    DeltaProbe(Gecode::Home home, WordView x0)
      : Gecode::Propagator(home), x(x0), c(home) {
      (void) new (home) Gecode::ViewAdvisor<WordView>(home,*this,c,x);
    }
    DeltaProbe(Gecode::Space& home, DeltaProbe& p)
      : Gecode::Propagator(home,p) {
      x.update(home,p.x);
      c.update(home,p.c);
    }
  public:
    static void post(Gecode::Home home, WordView x) {
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
      delta_capture.event = Gecode::Word::WordVarImp::modevent(d);
      delta_capture.zero = x.zero(d);
      delta_capture.one = x.one(d);
      delta_capture.old_minimum = x.old_minimum(d);
      delta_capture.old_maximum = x.old_maximum(d);
      delta_capture.new_minimum = x.new_minimum(d);
      delta_capture.new_maximum = x.new_maximum(d);
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

  static void
  reset_probes(void) {
    for (unsigned int i=0; i<4; i++) {
      probe_count[i] = 0;
      probe_event[i] = ME_WORD_NONE;
    }
  }

  /// Test which probes ran since the last reset, each seeing \a event
  static bool
  probes_ran(bool val, bool bits, bool bnd, bool dom,
             Gecode::ModEvent event) {
    const bool ran[4] = {val,bits,bnd,dom};
    for (unsigned int i=0; i<4; i++) {
      if (probe_count[i] != (ran[i] ? 1U : 0U))
        return false;
      if (ran[i] && (probe_event[i] != event))
        return false;
    }
    return true;
  }

  static WordValueSet
  value_set_of(std::initializer_list<WordValue> values) {
    WordValueSet set = {{0,0,0,0}};
    for (WordValue v : values)
      set.blocks[v / 64] |= WordValue(1) << (v % 64);
    return set;
  }

  static bool
  same_sets(const WordValueSet& a, const WordValueSet& b) {
    for (unsigned int i=0; i<4; i++)
      if (a.blocks[i] != b.blocks[i])
        return false;
    return true;
  }

  /// Space whose word is watched by one probe per propagation condition
  class ProbeSpace : public WordSpace {
  public:
    ProbeSpace(unsigned int width, WordDomainType kind) {
      x = Gecode::WordVar(*this,width,kind,WVS_EXACT);
      WordView xv(x);
      Probe<Gecode::Word::PC_WORD_VAL,0>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_BITS,1>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_BND,2>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_DOM,3>::post(*this,xv);
    }
    ProbeSpace(ProbeSpace& s) : WordSpace(s) {}
    virtual Gecode::Space* copy(void) { return new ProbeSpace(*this); }
  };

  /// Targeted checks of creation, events, deltas and words without a set
  class Kernel : public Test::Base {
  public:
    Kernel(void) : Test::Base("Word::ValueSet::Kernel") {}

    virtual bool run(void) {
      if (!creation()) { olog << "creation" << std::endl; return false; }
      if (!combination()) { olog << "combination" << std::endl; return false; }
      if (!bounded_events()) { olog << "bounded events" << std::endl; return false; }
      if (!cube_events()) { olog << "cube events" << std::endl; return false; }
      if (!deltas()) { olog << "deltas" << std::endl; return false; }
      if (!without_value_set()) { olog << "without value set" << std::endl; return false; }
      return true;
    }

  private:
    static bool creation(void) {
      const WordDomainType kinds[] = {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED};
      for (WordDomainType kind : kinds)
        for (unsigned int width=1; width<=8; width++) {
          WordSpace home;
          home.x = Gecode::WordVar(home,width,kind,WVS_EXACT);
          const WordValueSet values = WordView(home.x).values();
          if (!home.x.has_value_set() || (home.x.domain_type() != kind) ||
              !same_sets(values,Gecode::Word::full_value_set(width)))
            return false;
        }
      {
        WordSpace home;
        home.x = Gecode::WordVar(home,8,WDT_SIGNED);
        if (home.x.has_value_set())
          return false;
        home.x = Gecode::WordVar(home,4,WVS_NONE);
        if (home.x.has_value_set())
          return false;
      }
      {
        // A cube and an interval give exactly their common values.
        WordSpace home;
        home.x = Gecode::WordVar(home,4,WDT_UNSIGNED,4,12,Gecode::known_bits(0b0001,0b0001),WVS_EXACT);
        if (!same_sets(WordView(home.x).values(),
                       value_set_of({5,7,9,11})) ||
            (home.x.lo() != 1) || (home.x.minimum() != 5) ||
            (home.x.maximum() != 11))
          return false;
      }
      WordSpace home;
      try {
        Gecode::WordVar x(home,9,WVS_EXACT);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Gecode::WordVar x(home,9,WDT_UNSIGNED,0,7,WVS_EXACT);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Gecode::WordVar x(home,9,WDT_SIGNED,0,7,Gecode::known_bits(0,0),WVS_EXACT);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Gecode::WordVarArgs x(home,2,9,WDT_CUBE,WVS_EXACT);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Gecode::WordVarArray x(home,2,9,Gecode::known_bits(0,0),WVS_EXACT);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      Gecode::WordVarArgs args(home,2,8,WDT_SIGNED,WVS_EXACT);
      Gecode::WordVarArray array(home,2,3,WVS_EXACT);
      return args[0].has_value_set() && args[1].has_value_set() &&
        array[0].has_value_set() && array[1].has_value_set() &&
        (args[0].varimp() != args[1].varimp());
    }

    static bool combination(void) {
      const Gecode::ModEvent events[] = {
        ME_WORD_NONE,ME_WORD_VAL,ME_WORD_BITS,ME_WORD_BND,ME_WORD_DOM,
        ME_WORD_HOLE};
      for (Gecode::ModEvent e : events) {
        const Gecode::ModEvent expected = (e == ME_WORD_NONE) ? ME_WORD_HOLE : e;
        if ((Gecode::Word::WordVarImp::me_combine(ME_WORD_HOLE,e) != expected) ||
            (Gecode::Word::WordVarImp::me_combine(e,ME_WORD_HOLE) != expected))
          return false;
      }
      return true;
    }

    static bool bounded_events(void) {
      ProbeSpace s(4,WDT_UNSIGNED);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      WordView x(s.x);
      Gecode::Word::UnsignedWordView u(s.x);
      reset_probes();
      if ((x.nq(s,5) != ME_WORD_HOLE) || s.x.in(5))
        return false;
      (void) s.status();
      if (!probes_ran(false,false,false,true,ME_WORD_HOLE))
        return false;
      // A hole before a bound change in the same round gives the bound change.
      reset_probes();
      if ((x.nq(s,6) != ME_WORD_HOLE) ||
          (u.narrow_range(s,0,14) != ME_WORD_BND))
        return false;
      (void) s.status();
      if (!probes_ran(false,false,true,true,ME_WORD_BND))
        return false;
      // A hole after a bit change in the same round keeps the bit change.
      reset_probes();
      if ((x.narrow(s,0,14) != ME_WORD_BITS) || (x.nq(s,10) != ME_WORD_HOLE))
        return false;
      (void) s.status();
      if (!probes_ran(false,true,false,true,ME_WORD_BITS))
        return false;
      reset_probes();
      if ((x.nq(s,2) != ME_WORD_HOLE) || (x.eq(s,4) != ME_WORD_VAL))
        return false;
      (void) s.status();
      return probes_ran(true,true,true,true,ME_WORD_VAL);
    }

    static bool cube_events(void) {
      ProbeSpace s(3,WDT_CUBE);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      WordView x(s.x);
      reset_probes();
      if (x.nq(s,3) != ME_WORD_HOLE)
        return false;
      (void) s.status();
      if (!probes_ran(false,false,false,true,ME_WORD_HOLE))
        return false;
      reset_probes();
      if ((x.nq(s,5) != ME_WORD_HOLE) ||
          (x.narrow_values(s,value_set_of({0,1,2})) != ME_WORD_BITS) ||
          (s.x.lo() != 0) || (s.x.hi() != 3))
        return false;
      (void) s.status();
      return probes_ran(false,true,false,true,ME_WORD_BITS);
    }

    static bool deltas(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,4,WDT_UNSIGNED,WVS_EXACT);
      DeltaProbe::post(s,WordView(s.x));
      WordView x(s.x);
      delta_capture.count = 0;
      if (x.nq(s,7) != ME_WORD_HOLE)
        return false;
      if ((delta_capture.count != 1) ||
          (delta_capture.event != ME_WORD_HOLE) ||
          (delta_capture.zero != 0) || (delta_capture.one != 0) ||
          (delta_capture.old_minimum != 0) ||
          (delta_capture.old_maximum != 15) ||
          (delta_capture.new_minimum != 0) ||
          (delta_capture.new_maximum != 15))
        return false;
      // Removing the maximum moves the bound and reports it in the delta.
      if (x.nq(s,15) != ME_WORD_BND)
        return false;
      return (delta_capture.count == 2) &&
        (delta_capture.event == ME_WORD_BND) &&
        (delta_capture.zero == 0) && (delta_capture.one == 0) &&
        (delta_capture.old_maximum == 15) &&
        (delta_capture.new_maximum == 14);
    }

    static bool without_value_set(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,4,WDT_UNSIGNED);
      WordView x(s.x);
      if ((x.nq(s,0) != ME_WORD_BND) || (s.x.minimum() != 1) ||
          (x.nq(s,15) != ME_WORD_BND) || (s.x.maximum() != 14) ||
          (x.nq(s,7) != ME_WORD_NONE) || !s.x.in(7) ||
          (x.nq(s,0) != ME_WORD_NONE))
        return false;
      // The intersection narrows to its cube and interval, not to the set.
      if ((x.narrow_values(s,value_set_of({3,5,9})) != ME_WORD_DOM) ||
          (s.x.lo() != 1) || (s.x.hi() != 15) ||
          (s.x.minimum() != 3) || (s.x.maximum() != 9) ||
          !same_sets(x.values(),value_set_of({3,5,7,9})))
        return false;
      if ((x.eq(s,5) != ME_WORD_VAL) || (x.nq(s,5) != ME_WORD_FAILED))
        return false;

      WordSpace cube;
      cube.x = Gecode::WordVar(cube,3,Gecode::known_bits(0b110,0b010));
      WordView c(cube.x);
      if ((c.nq(cube,3) != ME_WORD_VAL) || (cube.x.val() != 2))
        return false;
      WordSpace wide;
      wide.x = Gecode::WordVar(wide,3);
      WordView w(wide.x);
      if ((w.nq(wide,3) != ME_WORD_NONE) ||
          (w.narrow_values(wide,value_set_of({1,3})) != ME_WORD_BITS) ||
          (wide.x.lo() != 1) || (wide.x.hi() != 3))
        return false;
      return w.narrow_values(wide,value_set_of({0,2})) == ME_WORD_FAILED;
    }
  } kernel;

  /** \brief Propagators reach honest fixpoints on value-set words
   *
   * They feed back cube bits that a value set closes beyond a tell, and
   * propagators that test membership wake when only values leave the set.
   */
  class Closure : public Test::Base {
    /// Words x, y, w and z, which each check replaces as it needs
    class Model : public Gecode::Space {
    public:
      Gecode::WordVar x, y, w, z;
      Model(void) : x(*this,1), y(*this,1), w(*this,1), z(*this,1) {}
      Model(Model& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        y.update(*this,s.y);
        w.update(*this,s.w);
        z.update(*this,s.z);
      }
      virtual Gecode::Space* copy(void) { return new Model(*this); }
      /// Make x a value-set word with values 0 and 3
      void sparse_x(void) {
        x = Gecode::WordVar(*this,2,WVS_EXACT);
        (void) WordView(x).narrow_values(*this,value_set_of({0,3}));
      }
      /// Return a 3-bit value-set word of \a kind with \a values
      Gecode::WordVar sparse(WordDomainType kind,
                             std::initializer_list<WordValue> values) {
        Gecode::WordVar v(*this,3,kind,WVS_EXACT);
        (void) WordView(v).narrow_values(*this,value_set_of(values));
        return v;
      }
    };
  public:
    Closure(void) : Test::Base("Word::ValueSet::Closure") {}
    bool run(void) override {
      return binary() && nary() && table() && fixed() && channel() &&
        extensional() && bounded_extensional() && shift() && negation() &&
        negation_failure(false) && negation_failure(true) && search_counts() && gcd() &&
        divides() && constant_shift();
    }
  private:
    /// x OR y = 1 with x and y in {0,3} has no solution
    static bool binary(void) {
      Model s;
      s.sparse_x();
      s.y = Gecode::WordVar(s,2,WVS_EXACT);
      (void) WordView(s.y).narrow_values(s,value_set_of({0,3}));
      s.z = Gecode::WordVar(s,2,{1});
      Gecode::rel(s,s.x,Gecode::WOT_OR,s.y,s.z);
      return s.status() == Gecode::SS_FAILED;
    }
    /// x OR y OR w = 1 with x and y in {0,3} and w in {0,2} has no solution
    static bool nary(void) {
      Model s;
      s.sparse_x();
      s.y = Gecode::WordVar(s,2,WVS_EXACT);
      (void) WordView(s.y).narrow_values(s,value_set_of({0,3}));
      s.w = Gecode::WordVar(s,2);
      s.z = Gecode::WordVar(s,2);
      Gecode::rel(s,Gecode::WOT_OR,Gecode::WordVarArgs({s.x,s.y,s.w}),s.z);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      Gecode::dom(s,s.w,0,2);
      Gecode::dom(s,s.z,1);
      return s.status() == Gecode::SS_FAILED;
    }
    /// y = ~x with x in {0,3} and then y in {0,2} gives x = 3 and y = 0
    static bool table(void) {
      Model s;
      s.sparse_x();
      s.y = Gecode::WordVar(s,2);
      Gecode::complement(s,s.x,s.y);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      Gecode::dom(s,s.y,0,2);
      return (s.status() != Gecode::SS_FAILED) && s.y.assigned() &&
        (s.y.val() == 0);
    }
    /// Zero-extending y in {0,1} to z in {0,3,12} gives z = 0 and y = 0
    static bool fixed(void) {
      Model s;
      s.y = Gecode::WordVar(s,2);
      s.z = Gecode::WordVar(s,4,WVS_EXACT);
      (void) WordView(s.z).narrow_values(s,value_set_of({0,3,12}));
      Gecode::zero_extend(s,s.y,4,s.z);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      Gecode::dom(s,s.y,0,1);
      return (s.status() != Gecode::SS_FAILED) && s.y.assigned() &&
        (s.y.val() == 0);
    }
    /// Channeling x in {1,3,4} and then y <= 3 gives x in {1,3} and y >= 1
    static bool channel(void) {
      Model s;
      s.x = Gecode::WordVar(s,3,WVS_EXACT);
      (void) WordView(s.x).narrow_values(s,value_set_of({1,3,4}));
      Gecode::IntVar y(s,0,7);
      Gecode::channel(s,s.x,y,Gecode::WI_UNSIGNED);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      Gecode::rel(s,y,Gecode::IRT_LQ,3);
      return (s.status() != Gecode::SS_FAILED) && (y.min() == 1) &&
        (y.max() == 3);
    }
    /// Rows (1,0) and (3,1) leave x = 3, so y = 1 and y = 0 fails
    static bool extensional(void) {
      Gecode::WordTupleSet rows(Gecode::IntArgs({2,1}));
      rows.add(Gecode::WordValArgs({1,0}));
      rows.add(Gecode::WordValArgs({3,1}));
      rows.finalize();
      Model s;
      s.sparse_x();
      s.y = Gecode::WordVar(s,1);
      Gecode::extensional(s,Gecode::WordVarArgs({s.x,s.y}),rows);
      if (s.status() == Gecode::SS_FAILED)
        return false;
      std::unique_ptr<Model> c(static_cast<Model*>(s.clone()));
      Gecode::dom(*c,c->y,0);
      return (c->status() == Gecode::SS_FAILED) && s.y.assigned() &&
        (s.y.val() == 1);
    }
    /// Unsigned x in {1,3,5,6} leaves row (2,1) dead, so y = 0
    static bool bounded_extensional(void) {
      Gecode::WordTupleSet rows(Gecode::IntArgs({3,1}));
      rows.add(Gecode::WordValArgs({2,1}));
      rows.add(Gecode::WordValArgs({3,0}));
      rows.add(Gecode::WordValArgs({5,0}));
      rows.add(Gecode::WordValArgs({6,0}));
      rows.finalize();
      Model s;
      s.x = Gecode::WordVar(s,3,WDT_UNSIGNED,WVS_EXACT);
      (void) WordView(s.x).narrow_values(s,value_set_of({1,3,5,6}));
      s.y = Gecode::WordVar(s,1);
      Gecode::extensional(s,Gecode::WordVarArgs({s.x,s.y}),rows);
      return (s.status() != Gecode::SS_FAILED) && s.y.assigned() &&
        (s.y.val() == 0);
    }
    /// Removing amount 0 from {0,1,2,3} leaves 1 << amount in {0,2,4}
    static bool shift(void) {
      Model s;
      s.x = Gecode::WordVar(s,3,{1});
      s.y = Gecode::WordVar(s,3,Gecode::known_bits(0b100,0b000),WVS_EXACT);
      s.z = Gecode::WordVar(s,3);
      Gecode::shift_left(s,s.x,s.y,s.z);
      if ((s.status() == Gecode::SS_FAILED) || (s.z.hi() != 7))
        return false;
      if (WordView(s.y).nq(s,0) != ME_WORD_HOLE)
        return false;
      return (s.status() != Gecode::SS_FAILED) && (s.z.lo() == 0) &&
        (s.z.hi() == 6);
    }
    /// Signed z = -x with x in {1,3,6,7} and z in {1,6} gives x = 7
    static bool negation(void) {
      Model s;
      s.x = s.sparse(WDT_SIGNED,{1,3,6,7});
      s.z = s.sparse(WDT_SIGNED,{1,6});
      Gecode::neg(s,s.x,s.z);
      return (s.status() != Gecode::SS_FAILED) && s.x.assigned() &&
        (s.x.val() == 7);
    }
    /** \brief Signed z = -x with x in {0,3} and z in {1,6} has no solution
     *
     * The bounds give x in [0,2] and z in [-2,0], which assign x = 0 and
     * z = 6. With \a is_incremental, the values are removed after posting.
     */
    static bool negation_failure(bool is_incremental) {
      Model s;
      s.x = s.sparse(WDT_SIGNED,{0,1,2,3,4,5,6,7});
      s.z = s.sparse(WDT_SIGNED,{0,1,2,3,4,5,6,7});
      if (is_incremental) {
        Gecode::neg(s,s.x,s.z);
        if (s.status() == Gecode::SS_FAILED)
          return false;
      }
      (void) WordView(s.x).narrow_values(s,value_set_of({0,3}));
      (void) WordView(s.z).narrow_values(s,value_set_of({1,6}));
      if (!is_incremental)
        Gecode::neg(s,s.x,s.z);
      return s.status() == Gecode::SS_FAILED;
    }
    /// Arithmetic relation of a search-count case
    enum class Operation { ADD, SUB, MULT, DIV, SREM, NARY_ADD, PRODUCT_MOD3,
                           ADD1, SHIFT1 };
    /// When a search-count case removes values, relative to posting
    enum class Removal { BEFORE_POST, AFTER_POST, AFTER_STATUS };
    /// Test whether \a set contains \a value
    static bool contains(const WordValueSet& set, WordValue value) {
      return ((set.blocks[value / 64] >> (value % 64)) & 1) != 0;
    }
    /// Return the signed value of a 3-bit word
    static int signed_value(WordValue value) {
      return ((value & 4) != 0) ? static_cast<int>(value) - 8 :
        static_cast<int>(value);
    }
    /// Test whether 3-bit words satisfy \a op, where only NARY_ADD uses \a w
    static bool satisfies(Operation op, WordValue x, WordValue y, WordValue w,
                          WordValue z) {
      switch (op) {
      case Operation::ADD: return ((x + y) & 7) == z;
      case Operation::SUB: return ((x - y) & 7) == z;
      case Operation::MULT: return ((x * y) & 7) == z;
      case Operation::DIV: return ((y == 0) ? 7 : x / y) == z;
      case Operation::SREM:
        if (y == 0)
          return x == z;
        return (static_cast<WordValue>(signed_value(x) % signed_value(y)) & 7) == z;
      case Operation::NARY_ADD: return ((x + y + w) & 7) == z;
      case Operation::PRODUCT_MOD3: return (x * y) % 3 == z;
      case Operation::ADD1: return ((x + 1) & 7) == z;
      case Operation::SHIFT1: return ((x << 1) & 7) == z;
      }
      GECODE_NEVER;
      return false;
    }
    /// Post \a op on the words of \a s
    static void post(Model& s, Operation op) {
      switch (op) {
      case Operation::ADD: Gecode::add(s,s.x,s.y,s.z); break;
      case Operation::SUB: Gecode::sub(s,s.x,s.y,s.z); break;
      case Operation::MULT: Gecode::mult(s,s.x,s.y,s.z); break;
      case Operation::DIV: Gecode::div(s,s.x,s.y,s.z); break;
      case Operation::SREM: Gecode::signed_rem(s,s.x,s.y,s.z); break;
      case Operation::NARY_ADD:
        Gecode::add(s,Gecode::WordVarArgs({s.x,s.y,s.w}),s.z);
        break;
      case Operation::PRODUCT_MOD3:
        Gecode::product_mod(s,s.x,s.y,Gecode::IntVar(s,3,3),s.z);
        break;
      case Operation::ADD1: Gecode::add(s,s.x,WordValue(1),s.z); break;
      case Operation::SHIFT1: Gecode::shift_left(s,s.x,1U,s.z); break;
      }
    }
    /// Remove the values outside \a sets from the words of \a s
    static void remove_values(Model& s, const WordValueSet (&sets)[4]) {
      Gecode::WordVar* words[4]={&s.x,&s.y,&s.w,&s.z};
      for (unsigned int i=0; i<4; i++)
        if (Gecode::me_failed(WordView(*words[i]).narrow_values(s,sets[i])))
          s.fail();
    }
    /** \brief Test that search finds exactly \a expected solutions of \a op
     *
     * The words are 3-bit value-set words of \a kind, restricted to \a sets
     * at \a removal. Search branches on bits, least significant first if
     * \a is_lsb, and every assignment it finds must satisfy \a op.
     */
    static bool search_finds(Operation op, WordDomainType kind,
                             const WordValueSet (&sets)[4], Removal removal,
                             bool is_lsb, unsigned int expected) {
      Model* s=new Model;
      s->x = s->sparse(kind,{0,1,2,3,4,5,6,7});
      s->y = s->sparse(kind,{0,1,2,3,4,5,6,7});
      s->w = s->sparse(kind,{0,1,2,3,4,5,6,7});
      s->z = s->sparse(kind,{0,1,2,3,4,5,6,7});
      if (removal == Removal::BEFORE_POST)
        remove_values(*s,sets);
      post(*s,op);
      if (removal == Removal::AFTER_STATUS)
        (void) s->status();
      if (removal != Removal::BEFORE_POST)
        remove_values(*s,sets);
      Gecode::branch(*s,Gecode::WordVarArgs({s->x,s->y,s->w,s->z}),
                     Gecode::WORD_VAR_NONE(),
                     is_lsb ? Gecode::WORD_VAL_LSB() : Gecode::WORD_VAL_MSB());
      Gecode::DFS<Model> e(s);
      delete s;
      unsigned int found=0;
      while (Model* solution=e.next()) {
        const bool is_solution=satisfies(op,solution->x.val(),
          solution->y.val(),solution->w.val(),solution->z.val());
        delete solution;
        if (!is_solution)
          return false;
        found++;
      }
      return found == expected;
    }
    /** \brief Compare the solutions search finds for \a op with a count
     *
     * The words are restricted to \a x, \a y, \a w and \a z before posting,
     * after posting, or after posting and propagation. Search in each case
     * must find the assignments in the value sets that satisfy \a op.
     */
    static bool search_count(Operation op, WordDomainType kind,
                             std::initializer_list<WordValue> x,
                             std::initializer_list<WordValue> y,
                             std::initializer_list<WordValue> w,
                             std::initializer_list<WordValue> z) {
      const WordValueSet sets[4]={value_set_of(x),value_set_of(y),
                                  value_set_of(w),value_set_of(z)};
      unsigned int expected=0;
      for (WordValue a=0; a<8; a++)
        for (WordValue b=0; b<8; b++)
          for (WordValue c=0; c<8; c++)
            for (WordValue d=0; d<8; d++)
              if (contains(sets[0],a) && contains(sets[1],b) &&
                  contains(sets[2],c) && contains(sets[3],d) &&
                  satisfies(op,a,b,c,d))
                expected++;
      for (Removal removal : {Removal::BEFORE_POST,Removal::AFTER_POST,
                              Removal::AFTER_STATUS})
        for (bool is_lsb : {true,false})
          if (!search_finds(op,kind,sets,removal,is_lsb,expected))
            return false;
      return true;
    }
    /** \brief Search counts that need bounded arithmetic to reach a fixpoint
     *
     * Publishing a bounds pass can close a value set further than the pass
     * requested. Each case failed while the pass returned ES_FIX instead.
     */
    static bool search_counts(void) {
      return
        search_count(Operation::ADD,WDT_SIGNED,
                     {0,1,2,3,5},{0,3,5,6},{0},{4,7}) &&
        search_count(Operation::SUB,WDT_SIGNED,
                     {0,1,2,3,5},{0,3,5,6},{0},{4,7}) &&
        search_count(Operation::MULT,WDT_SIGNED,
                     {0,1,4},{0,1,3,4,7},{0},{0,1,6}) &&
        search_count(Operation::DIV,WDT_UNSIGNED,
                     {0,3,4,7},{1,6},{0},{1,4,5,7}) &&
        search_count(Operation::SREM,WDT_SIGNED,
                     {1,3,5,7},{0,1,2,3,4,6,7},{0},{6}) &&
        search_count(Operation::NARY_ADD,WDT_SIGNED,
                     {1,3,5},{0,1,2,4,5,7},{0,3,4,7},{1,4,5,6}) &&
        search_count(Operation::PRODUCT_MOD3,WDT_UNSIGNED,
                     {0,2,3,4,5,6},{0,1,4,7},{0},{2,3,5,6,7}) &&
        search_count(Operation::ADD1,WDT_SIGNED,{4,7},{0},{0},{1,2,3,6}) &&
        search_count(Operation::SHIFT1,WDT_UNSIGNED,{0,2,3},{0},{0},{2,7});
    }
    /// gcd(x,2) = z with x in {0,4,7} and z in {0,1,4,7} gives x = 7
    static bool gcd(void) {
      Model s;
      s.x = s.sparse(WDT_UNSIGNED,{0,4,7});
      s.y = s.sparse(WDT_UNSIGNED,{2});
      s.z = s.sparse(WDT_UNSIGNED,{0,1,4,7});
      Gecode::gcd(s,s.x,s.y,s.z);
      return (s.status() != Gecode::SS_FAILED) && s.x.assigned() &&
        (s.x.val() == 7);
    }
    /// x divides y with x in {0,3} and y in {2,5,7} has no solution
    static bool divides(void) {
      Model s;
      s.x = s.sparse(WDT_UNSIGNED,{0,3});
      s.y = s.sparse(WDT_UNSIGNED,{2,5,7});
      Gecode::divides(s,s.x,s.y,Gecode::Reify(Gecode::BoolVar(s,1,1)));
      return s.status() == Gecode::SS_FAILED;
    }
    /// z = x << 1 with x in {0,3} and z in {0,4,5,7} gives z = 0
    static bool constant_shift(void) {
      Model s;
      s.x = s.sparse(WDT_UNSIGNED,{0,3});
      s.z = s.sparse(WDT_UNSIGNED,{0,4,5,7});
      Gecode::shift_left(s,s.x,1U,s.z);
      return (s.status() != Gecode::SS_FAILED) && s.z.assigned() &&
        (s.z.val() == 0);
    }
  } closure;

  /// Test whether \a value is 1, 2, 4 or 6, whose hulls admit other values
  static bool
  is_listed(WordValue value) {
    return (value == 1) || (value == 2) || (value == 4) || (value == 6);
  }

  /// Return the list 1, 2, 4 and 6, unordered and with a duplicate
  static Gecode::WordValArgs
  listed_values(void) {
    return Gecode::WordValArgs({6,1,4,2,4});
  }

  /// dom() with a list of three-bit values whose cube hull is every value
  class DomValues : public Test {
  public:
    DomValues(void) : Test("Dom::Values",1,Word::Domain(3,0,7)) {}
    bool solution(const Assignment& a) const override {
      return is_listed(a[0]);
    }
    void post(Gecode::Space& home, Gecode::WordVarArray& x) override {
      Gecode::dom(home,x[0],listed_values());
    }
  } dom_values;

  /** \brief Restricting words to a list of values
   *
   * A value-set word keeps exactly the listed values. Another word keeps
   * the hulls of the listed values and rejects other values on assignment.
   */
  class Dom : public Test::Base {
  public:
    Dom(void) : Test::Base("Word::ValueSet::Dom") {}
    bool run(void) override {
      for (WordDomainType kind : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED})
        for (Gecode::WordValueSetOption value_set : {WVS_NONE,WVS_EXACT})
          if (!restricts(kind,value_set) || !searches(kind,value_set))
            return false;
      return failures() && wide() && wide_hulls() && entailment();
    }
  private:
    /// Return a three-bit word of \a kind restricted to the listed values
    static WordSpace* restricted(WordDomainType kind,
                                 Gecode::WordValueSetOption value_set) {
      WordSpace* s = new WordSpace;
      s->x = Gecode::WordVar(*s,3,kind,value_set);
      Gecode::dom(*s,s->x,listed_values());
      return s;
    }
    /// Test the domain right after posting and propagation
    static bool restricts(WordDomainType kind,
                          Gecode::WordValueSetOption value_set) {
      std::unique_ptr<WordSpace> s(restricted(kind,value_set));
      if (s->status() == Gecode::SS_FAILED)
        return false;
      if (value_set == WVS_EXACT) {
        for (WordValue v=0; v<8; v++)
          if (s->x.in(v) != is_listed(v))
            return false;
        return Gecode::PropagatorGroup::all.size(*s) == 0U;
      }
      const WordValue flip = sign_flip(kind,3);
      WordValue first = 7, last = 0;
      for (WordValue v=0; v<8; v++)
        if (is_listed(v)) {
          first = std::min(first,v ^ flip);
          last = std::max(last,v ^ flip);
        }
      if ((s->x.lo() != 0U) || (s->x.hi() != 7U))
        return false;
      return !s->x.bounded() || ((s->x.minimum() == (first ^ flip)) &&
                                 (s->x.maximum() == (last ^ flip)));
    }
    /// Test that search finds exactly the listed values
    static bool searches(WordDomainType kind,
                         Gecode::WordValueSetOption value_set) {
      WordSpace* root = restricted(kind,value_set);
      Gecode::branch(*root,root->x,Gecode::WORD_VAL_LSB());
      Gecode::DFS<WordSpace> e(root);
      delete root;
      std::vector<WordValue> found;
      while (WordSpace* solution = e.next()) {
        std::unique_ptr<WordSpace> guard(solution);
        if (!solution->x.assigned() ||
            (Gecode::PropagatorGroup::all.size(*solution) != 0U))
          return false;
        found.push_back(solution->x.val());
      }
      std::sort(found.begin(),found.end());
      return found == std::vector<WordValue>({1,2,4,6});
    }
    /// Test empty lists, empty intersections and values outside the width
    static bool failures(void) {
      for (Gecode::WordValueSetOption value_set : {WVS_NONE,WVS_EXACT}) {
        WordSpace empty;
        empty.x = Gecode::WordVar(empty,3,value_set);
        Gecode::dom(empty,empty.x,Gecode::WordValArgs());
        if (empty.status() != Gecode::SS_FAILED)
          return false;

        WordSpace odd;
        odd.x = Gecode::WordVar(odd,3,Gecode::known_bits(0b001,0b001),value_set);
        Gecode::dom(odd,odd.x,Gecode::WordValArgs({2,4}));
        if (odd.status() != Gecode::SS_FAILED)
          return false;

        WordSpace invalid;
        invalid.x = Gecode::WordVar(invalid,3,value_set);
        try {
          Gecode::dom(invalid,invalid.x,Gecode::WordValArgs({1,8}));
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
      }
      WordSpace sparse;
      sparse.x = Gecode::WordVar(sparse,3,WVS_EXACT);
      Gecode::dom(sparse,sparse.x,Gecode::WordValArgs({0,3}));
      Gecode::dom(sparse,sparse.x,Gecode::WordValArgs({1,2}));
      return sparse.status() == Gecode::SS_FAILED;
    }
    /// Test that a word wider than eight bits rejects unlisted values
    static bool wide(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,16);
      Gecode::dom(s,s.x,Gecode::WordValArgs({3,1000}));
      if ((s.status() == Gecode::SS_FAILED) || (s.x.lo() != (3U & 1000U)) ||
          (s.x.hi() != (3U | 1000U)))
        return false;
      std::unique_ptr<WordSpace> unlisted(
        static_cast<WordSpace*>(s.clone()));
      Gecode::dom(*unlisted,unlisted->x,WordValue(1));
      Gecode::dom(s,s.x,WordValue(1000));
      return (unlisted->status() == Gecode::SS_FAILED) &&
        (s.status() != Gecode::SS_FAILED) &&
        (Gecode::PropagatorGroup::all.size(s) == 0U);
    }
    /// Test the domain of a 17-bit \a kind word in \a lo..\a hi after dom()
    static bool wide_hull(WordDomainType kind, WordValue lo, WordValue hi,
                          const Gecode::WordValArgs& values,
                          WordValue minimum, WordValue maximum,
                          WordValue cube_lo, WordValue cube_hi) {
      WordSpace s;
      s.x = Gecode::WordVar(s,17,kind,lo,hi);
      Gecode::dom(s,s.x,values);
      return (s.status() != Gecode::SS_FAILED) &&
        (s.x.minimum() == minimum) && (s.x.maximum() == maximum) &&
        (s.x.lo() == cube_lo) && (s.x.hi() == cube_hi);
    }
    /// Test that a wide bounded word gets the hulls of its listed values
    static bool wide_hulls(void) {
      const WordValue mask = (WordValue(1) << 17) - 1;
      const WordValue minus_two = mask - 1;
      return
        wide_hull(WDT_UNSIGNED,0,mask,Gecode::WordValArgs({1,mask-1}),
                  1,mask-1,0,mask) &&
        wide_hull(WDT_UNSIGNED,0,100000,Gecode::WordValArgs({5,mask-1,1}),
                  1,5,1,5) &&
        // -2 precedes 3 in signed order but follows it in encoding order
        wide_hull(WDT_SIGNED,WordValue(1) << 16,(WordValue(1) << 16) - 1,
                  Gecode::WordValArgs({3,minus_two}),minus_two,3,
                  2,mask);
    }
    /// Test that a word whose values are all listed gets no propagator
    static bool entailment(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,3,Gecode::known_bits(0b110,0b010));
      Gecode::dom(s,s.x,Gecode::WordValArgs({1,2,3}));
      return (s.status() != Gecode::SS_FAILED) &&
        (Gecode::PropagatorGroup::all.size(s) == 0U) && (s.x.lo() == 2U) &&
        (s.x.hi() == 3U);
    }
  } dom;

}}}

// STATISTICS: test-word
