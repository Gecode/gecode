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

#include <algorithm>

#include <limits>
#include <memory>

namespace Test { namespace Word {

  unsigned int
  Domain::unknown_size(void) const {
    Gecode::WordValue u = unknown();
    unsigned int n = 0;
    while (u != 0) {
      u &= u-1;
      n++;
    }
    return n;
  }

  Values::Values(const Domain& d0)
    : d(d0), subset(0), more(true) {}

  bool
  Values::operator()(void) const {
    return more;
  }

  void
  Values::operator++(void) {
    const Gecode::WordValue unknown = d.unknown();
    const Gecode::WordValue next = (subset - unknown) & unknown;
    if (next == 0)
      more = false;
    else
      subset = next;
  }

  Gecode::WordValue
  Values::val(void) const {
    return d.lo() | subset;
  }

  bool
  exhaustive(const Domain& d, int arity, Gecode::WordValue limit) {
    if (arity < 0)
      return false;
    Gecode::WordValue assignments = 1;
    for (int i=0; i<arity; i++) {
      for (unsigned int bit=0; bit<d.unknown_size(); bit++) {
        if (assignments > limit / 2)
          return false;
        assignments *= 2;
      }
    }
    return assignments <= limit;
  }

  std::vector<Gecode::WordValue>
  values(const Domain& d) {
    if (!exhaustive(d,1))
      throw Gecode::Word::OutOfLimits("Test::Word::values");
    std::vector<Gecode::WordValue> result;
    for (Values i(d); i(); ++i)
      result.push_back(i.val());
    return result;
  }

  std::vector<Gecode::WordValue>
  float_order_range(unsigned int width, Gecode::WordValue minimum,
                    Gecode::WordValue maximum) {
    const Gecode::WordValue sign = Gecode::WordValue(1) << (width-1);
    std::vector<Gecode::WordValue> result;
    for (Gecode::WordValue v = minimum; ; ) {
      result.push_back(v);
      if (v == maximum)
        return result;
      if ((v & sign) == 0)
        v++;
      else if (v == sign)
        v = 0;
      else
        v--;
    }
  }

  std::pair<Gecode::WordValue,Gecode::WordValue>
  float_order_extremes(unsigned int width,
                       const std::set<Gecode::WordValue>& values) {
    assert(!values.empty());
    const Gecode::WordValue sign = Gecode::WordValue(1) << (width-1);
    // Encodings with the sign bit set order by descending code, then the
    // others by ascending code.
    std::vector<Gecode::WordValue> negative, positive;
    for (Gecode::WordValue v : values)
      ((v & sign) != 0 ? negative : positive).push_back(v);
    return {negative.empty() ? positive.front() : negative.back(),
            positive.empty() ? negative.front() : positive.back()};
  }

  Gecode::WordBits
  cube_bits(unsigned int width, Gecode::WordValue lo, Gecode::WordValue hi) {
    Gecode::Word::check_domain(width,lo,hi,"Test::Word::cube_bits");
    return Gecode::known_bits(lo | (~hi & Gecode::Word::width_mask(width)),lo);
  }

  Gecode::WordVar
  float_window(Gecode::Space& home, Gecode::WordDomainType format,
               Gecode::WordValue minimum, Gecode::WordValue maximum,
               Gecode::WordValueSetOption value_set) {
    return Gecode::WordVar(home,Gecode::float_word_width(format),format,
                           minimum,maximum,value_set);
  }

  Gecode::WordVar
  float_code(Gecode::Space& home, Gecode::WordDomainType type,
                 Gecode::WordValue code) {
    return Gecode::WordVar(home,Gecode::Word::float_domain_width(type),type,
                           code,code);
  }

  CpltAssignment::CpltAssignment(int n0, const Domain& d0)
    : Assignment(n0,d0),
      vals(new Gecode::WordValue[static_cast<size_t>(n0)]), more(true) {
    assert(n0 > 0);
    for (int i=0; i<n; i++)
      vals[i] = d.lo();
  }

  void
  CpltAssignment::next(Gecode::Support::RandomGenerator&) {
    int i = n-1;
    while (true) {
      const Gecode::WordValue unknown = d.unknown();
      const Gecode::WordValue subset = vals[i] & unknown;
      const Gecode::WordValue next = (subset - unknown) & unknown;
      if (next != 0) {
        vals[i] = d.lo() | next;
        return;
      }
      vals[i] = d.lo();
      if (i == 0) {
        more = false;
        return;
      }
      --i;
    }
  }

  Gecode::WordValue
  RandomAssignment::randval(Gecode::Support::RandomGenerator& rand) const {
    Gecode::WordValue value = d.lo();
    Gecode::WordValue unknown = d.unknown();
    while (unknown != 0) {
      const Gecode::WordValue bit = unknown & (~unknown + 1);
      if (rand(2) != 0)
        value |= bit;
      unknown &= unknown-1;
    }
    return value;
  }

  RandomAssignment::RandomAssignment(
    int n0, const Domain& d0, int samples,
    Gecode::Support::RandomGenerator& rand)
    : Assignment(n0,d0), vals(new Gecode::WordValue[static_cast<size_t>(n0)]),
      remaining(samples) {
    assert((n0 > 0) && (samples >= 0));
    for (int i=0; i<n; i++)
      vals[i] = randval(rand);
  }

  void
  RandomAssignment::next(Gecode::Support::RandomGenerator& rand) {
    for (int i=0; i<n; i++)
      vals[i] = randval(rand);
    remaining--;
  }

  PartialDomains::PartialDomains(unsigned int width)
    : _width(width), state(0), end(1) {
    Gecode::Word::check_domain(width,0,Gecode::Word::width_mask(width),
                               "Test::Word::PartialDomains");
    if (width > max_width)
      throw Gecode::Word::OutOfLimits("Test::Word::PartialDomains");
    for (unsigned int i=0; i<width; i++) {
      if (end > std::numeric_limits<Gecode::WordValue>::max() / 3)
        throw Gecode::Word::OutOfLimits("Test::Word::PartialDomains");
      end *= 3;
    }
  }

  Domain
  PartialDomains::domain(void) const {
    Gecode::WordValue code = state;
    Gecode::WordValue lo = 0;
    Gecode::WordValue hi = 0;
    for (unsigned int i=0; i<_width; i++) {
      const Gecode::WordValue bit = Gecode::WordValue(1) << i;
      switch (code % 3) {
      case 0: break;
      case 1: hi |= bit; break;
      case 2: lo |= bit; hi |= bit; break;
      default: GECODE_NEVER;
      }
      code /= 3;
    }
    return Domain(_width,lo,hi);
  }

  PartialAssignment::PartialAssignment(int n0, unsigned int width)
    : n(n0), _width(width), state(0), end(1) {
    if ((n <= 0) || (width == 0) || (width > 64) ||
        (static_cast<unsigned int>(n) * width > PartialDomains::max_width))
      throw Gecode::Word::OutOfLimits("Test::Word::PartialAssignment");
    for (unsigned int i=0; i<static_cast<unsigned int>(n) * width; i++)
      end *= 3;
  }

  Domain
  PartialAssignment::operator[](int i) const {
    assert((i >= 0) && (i < n));
    Gecode::WordValue code = state;
    for (int j=0; j<i; j++)
      for (unsigned int bit=0; bit<_width; bit++)
        code /= 3;
    Gecode::WordValue lo = 0;
    Gecode::WordValue hi = 0;
    for (unsigned int bit=0; bit<_width; bit++) {
      const Gecode::WordValue mask = Gecode::WordValue(1) << bit;
      switch (code % 3) {
      case 0: break;
      case 1: hi |= mask; break;
      case 2: lo |= mask; hi |= mask; break;
      default: GECODE_NEVER;
      }
      code /= 3;
    }
    return Domain(_width,lo,hi);
  }

}}

std::ostream&
operator<<(std::ostream& os, const Test::Word::Domain& d) {
  os << d.width() << "'b";
  for (unsigned int i=d.width(); i--;) {
    const Gecode::WordValue bit = Gecode::WordValue(1) << i;
    os << ((d.lo() & bit) ? '1' : ((d.hi() & bit) ? '?' : '0'));
  }
  return os;
}

std::ostream&
operator<<(std::ostream& os, const Test::Word::Assignment& a) {
  os << "{";
  for (int i=0; i<a.size(); i++) {
    os << a.domain().width() << "'h" << std::hex << a[i] << std::dec;
    os << ((i+1 == a.size()) ? "}" : ",");
  }
  return os;
}

std::ostream&
operator<<(std::ostream& os, Gecode::WordDomainType t) {
  switch (t) {
  case Gecode::WDT_CUBE: return os << "WDT_CUBE";
  case Gecode::WDT_UNSIGNED: return os << "WDT_UNSIGNED";
  case Gecode::WDT_SIGNED: return os << "WDT_SIGNED";
  case Gecode::WDT_FP32: return os << "WDT_FP32";
  case Gecode::WDT_BF16: return os << "WDT_BF16";
  case Gecode::WDT_BINARY16: return os << "WDT_BINARY16";
  case Gecode::WDT_E4M3FN: return os << "WDT_E4M3FN";
  case Gecode::WDT_E2M1: return os << "WDT_E2M1";
  }
  GECODE_NEVER;
  return os;
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Rounding r) {
  switch (r) {
  case Gecode::FloatWordPolicy::RNE: return os << "RNE";
  case Gecode::FloatWordPolicy::RTZ: return os << "RTZ";
  case Gecode::FloatWordPolicy::RDN: return os << "RDN";
  case Gecode::FloatWordPolicy::RUP: return os << "RUP";
  }
  return os << static_cast<int>(r);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Subnormals s) {
  switch (s) {
  case Gecode::FloatWordPolicy::Preserve: return os << "Preserve";
  case Gecode::FloatWordPolicy::SignedZero: return os << "SignedZero";
  }
  return os << static_cast<int>(s);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Overflow o) {
  switch (o) {
  case Gecode::FloatWordPolicy::Format: return os << "Format";
  case Gecode::FloatWordPolicy::SaturateFinite: return os << "SaturateFinite";
  }
  return os << static_cast<int>(o);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Zeros z) {
  switch (z) {
  case Gecode::FloatWordPolicy::Signed: return os << "Signed";
  case Gecode::FloatWordPolicy::Positive: return os << "Positive";
  }
  return os << static_cast<int>(z);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Nan n) {
  switch (n) {
  case Gecode::FloatWordPolicy::SignalFirst: return os << "SignalFirst";
  case Gecode::FloatWordPolicy::OperandFirst: return os << "OperandFirst";
  case Gecode::FloatWordPolicy::Canonical: return os << "Canonical";
  }
  return os << static_cast<int>(n);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Tininess t) {
  switch (t) {
  case Gecode::FloatWordPolicy::AfterPrecision: return os << "AfterPrecision";
  case Gecode::FloatWordPolicy::BeforeRounding: return os << "BeforeRounding";
  }
  return os << static_cast<int>(t);
}

std::ostream&
operator<<(std::ostream& os, Gecode::FloatWordPolicy::Comparison c) {
  switch (c) {
  case Gecode::FloatWordPolicy::Quiet: return os << "Quiet";
  case Gecode::FloatWordPolicy::Signaling: return os << "Signaling";
  }
  return os << static_cast<int>(c);
}

namespace Test { namespace Word {

  TestSpace::TestSpace(int n, const Domain& d0, Test* t,
                       Gecode::WordValueSetOption value_set)
    : d(d0), x(*this,n,d.width(),cube_bits(d.width(),d.lo(),d.hi()),value_set),
      r(Gecode::BoolVar(*this,0,1),Gecode::RM_EQV), test(t),
      reified(false) {
    if (opt.log)
      olog << ind(2) << "Initial: x[]=" << x << std::endl;
  }

  TestSpace::TestSpace(int n, const Domain& d0, Test* t,
                       Gecode::ReifyMode rm,
                       Gecode::WordValueSetOption value_set)
    : d(d0), x(*this,n,d.width(),cube_bits(d.width(),d.lo(),d.hi()),value_set),
      r(Gecode::BoolVar(*this,0,1),rm), test(t), reified(true) {
    if (opt.log)
      olog << ind(2) << "Initial: x[]=" << x
           << " b=" << r.var() << std::endl;
  }

  TestSpace::TestSpace(TestSpace& s)
    : Gecode::Space(s), d(s.d), test(s.test), reified(s.reified) {
    x.update(*this,s.x);
    Gecode::BoolVar b;
    Gecode::BoolVar sb(s.r.var());
    b.update(*this,sb);
    r.var(b);
    r.mode(s.r.mode());
  }

  Gecode::Space*
  TestSpace::copy(void) {
    return new TestSpace(*this);
  }

  bool
  TestSpace::assigned(void) const {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        return false;
    return true;
  }

  bool
  TestSpace::match(const Assignment& a) const {
    if (a.size() != x.size())
      return false;
    for (int i=x.size(); i--; )
      if (!x[i].assigned() || (x[i].val() != a[i]))
        return false;
    return true;
  }

  void
  TestSpace::assign(const Assignment& a, bool skip,
                    Gecode::Support::RandomGenerator& rand) {
    assert(a.size() == x.size());
    const int skip_i = skip ?
      static_cast<int>(rand(static_cast<unsigned int>(a.size()))) : -1;
    for (int i=x.size(); i--; )
      if (i != skip_i) {
        Gecode::dom(*this,x[i],a[i]);
        if (Base::fixpoint(rand) && failed())
          return;
      }
  }

  void
  TestSpace::narrow(const std::vector<Domain>& domains) {
    assert(static_cast<int>(domains.size()) == x.size());
    for (int i=x.size(); i--; ) {
      assert(domains[static_cast<size_t>(i)].width() == x[i].width());
      Gecode::dom(*this,x[i],domains[static_cast<size_t>(i)].lo(),
                  domains[static_cast<size_t>(i)].hi());
    }
  }

  /// Return random bits, each set with probability \f$2^{-sparsity}\f$
  static Gecode::WordValue
  random_block(Gecode::Support::RandomGenerator& rand, unsigned int sparsity) {
    Gecode::WordValue block = ~Gecode::WordValue(0);
    for (unsigned int i=0; i<sparsity; i++) {
      Gecode::WordValue bits = 0;
      for (int j=0; j<4; j++)
        bits = (bits << 16) | rand(1U << 16);
      block &= bits;
    }
    return block;
  }

  /** \brief Return a random subset of \a values that keeps \a value
   *
   * The subset is \a values itself, \a value and one other random value,
   * or each value kept with probability 1/2 or 1/4.
   */
  static Gecode::Word::WordValueSet
  random_subset(const Gecode::Word::WordValueSet& values,
                Gecode::WordValue value, Gecode::WordValue mask,
                Gecode::Support::RandomGenerator& rand) {
    Gecode::Word::WordValueSet kept = values;
    const unsigned int choice = rand(4);
    if (choice == 0)
      return kept;
    if (choice == 1) {
      kept = Gecode::Word::WordValueSet{{0,0,0,0}};
      const Gecode::WordValue other = rand(static_cast<unsigned int>(mask)+1U);
      kept.blocks[other / 64] |= Gecode::WordValue(1) << (other % 64);
    } else {
      for (Gecode::WordValue& block : kept.blocks)
        block &= random_block(rand,choice-1);
    }
    kept.blocks[value / 64] |= Gecode::WordValue(1) << (value % 64);
    for (int i=0; i<4; i++)
      kept.blocks[i] &= values.blocks[i];
    return kept;
  }

  void
  TestSpace::remove_values(const Assignment& a,
                           Gecode::Support::RandomGenerator& rand) {
    assert(a.size() == x.size());
    for (int i=x.size(); i--; ) {
      Gecode::Word::WordView v(x[i]);
      if (!v.has_value_set())
        continue;
      const Gecode::Word::WordValueSet kept =
        random_subset(v.values(),a[i],d.mask(),rand);
      if (opt.log) {
        olog << ind(3) << "Keep x[" << i << "] in {";
        for (Gecode::WordValue value=0; value<=d.mask(); value++)
          if ((kept.blocks[value / 64] >> (value % 64)) & 1U)
            olog << value << " ";
        olog << "}" << std::endl;
      }
      if (Gecode::me_failed(v.narrow_values(*this,kept)))
        fail();
      if (Base::fixpoint(rand) && failed())
        return;
    }
  }

  void
  TestSpace::rel(bool value) {
    assert(reified);
    Gecode::rel(*this,r.var(),Gecode::IRT_EQ,value ? 1 : 0);
  }

  void
  TestSpace::post(void) {
    assert(test != nullptr);
    if (reified)
      test->post(*this,x,r);
    else
      test->post(*this,x);
  }

  bool
  TestSpace::failed(void) {
    return status() == Gecode::SS_FAILED;
  }

  unsigned int
  TestSpace::propagators(void) {
    return Gecode::PropagatorGroup::all.size(*this);
  }

  Test::Test(const std::string& name, int a, const Domain& domain,
             bool r)
    : Base("Word::"+name), arity(a), dom(domain), reified(r),
      rms((1 << Gecode::RM_EQV) | (1 << Gecode::RM_IMP) |
          (1 << Gecode::RM_PMI)) {}

  Assignment*
  Test::assignment(void) const {
    if (exhaustive(dom,arity))
      return new CpltAssignment(arity,dom);
    return new RandomAssignment(arity,dom,4096,_rand);
  }

  void
  Test::post(Gecode::Space&, Gecode::WordVarArray&, Gecode::Reify) {}

  bool
  Test::check(const Assignment& a, bool sol,
              Gecode::WordValueSetOption value_set) {
    const bool has_value_sets = (value_set == Gecode::WVS_EXACT);
    if (opt.log && has_value_sets)
      olog << ind(2) << "With value sets" << std::endl;
    const bool removes_first = has_value_sets && (_rand(2) == 0);
    TestSpace* s = new TestSpace(arity,dom,this,value_set);
    if (removes_first)
      s->remove_values(a,_rand);
    s->post();
    if (has_value_sets && !removes_first)
      s->remove_values(a,_rand);
    TestSpace* c = (s->status() == Gecode::SS_FAILED) ? nullptr :
      static_cast<TestSpace*>(s->clone());
    TestSpace* assigned = (c == nullptr) ? s : c;
    assigned->assign(a,false,_rand);
    const bool is_wrong = (assigned->failed() == sol);
    delete c;
    delete s;
    if (is_wrong)
      return false;

    const Gecode::ReifyMode modes[] = {
      Gecode::RM_EQV, Gecode::RM_IMP, Gecode::RM_PMI
    };
    for (unsigned int i=0; reified && (i<3); i++) {
      if ((rms & (1 << modes[i])) == 0)
        continue;
      for (int bv=0; bv<=1; bv++) {
        const bool b = (bv != 0);
        bool allowed = false;
        switch (modes[i]) {
        case Gecode::RM_EQV: allowed = (b == sol); break;
        case Gecode::RM_IMP: allowed = !b || sol; break;
        case Gecode::RM_PMI: allowed = !sol || b; break;
        default: GECODE_NEVER;
        }
        TestSpace* rs = new TestSpace(arity,dom,this,modes[i],value_set);
        rs->post();
        rs->rel(b);
        if (has_value_sets)
          rs->remove_values(a,_rand);
        rs->assign(a,false,_rand);
        const bool failed = rs->failed();
        delete rs;
        if (failed == allowed)
          return false;
      }
    }
    return true;
  }

  bool
  Test::run(void) {
    const bool has_value_sets =
      dom.width() <= Gecode::Word::max_value_set_width;
    std::unique_ptr<Assignment> a(assignment());
    while (a->has_more()) {
      const bool sol = solution(*a);
      if (!check(*a,sol,Gecode::WVS_NONE))
        return false;
      if (has_value_sets && !check(*a,sol,Gecode::WVS_EXACT))
        return false;
      a->next(_rand);
    }
    return true;
  }

  /// Self-test for the word testing framework
  class Framework : public Base {
  public:
    Framework(void) : Base("Word::TestFramework",TestTags::check()) {}
    virtual bool run(void) {
      Domain d(3,1,7);
      const std::vector<Gecode::WordValue> expected = {1,3,5,7};
      if (values(d) != expected)
        return false;
      if ((d.unknown_size() != 2) || !d.in(5) || d.in(2))
        return false;

      Gecode::Support::RandomGenerator unused(1);
      CpltAssignment complete(2,d);
      unsigned int assignments = 0;
      while (complete.has_more()) {
        if (!d.in(complete[0]) || !d.in(complete[1]))
          return false;
        assignments++;
        complete.next(unused);
      }
      if ((assignments != 16) || !exhaustive(d,2) ||
          exhaustive(Domain(13,0,8191),1))
        return false;

      if ((values(Domain(1,0,1)).size() != 2) ||
          (values(Domain(64,0,Gecode::WordValue(1) << 63)).size() != 2))
        return false;
      try {
        Domain invalid(0,0,0);
        (void) invalid;
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Domain invalid(65,0,0);
        (void) invalid;
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      class InvalidChannelSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::BoolVar b;
        InvalidChannelSpace(void)
          : x(*this,3), b(*this,0,1) {}
        InvalidChannelSpace(InvalidChannelSpace& s)
          : Gecode::Space(s) {
          x.update(*this,s.x);
          b.update(*this,s.b);
        }
        virtual Gecode::Space* copy(void) {
          return new InvalidChannelSpace(*this);
        }
      } invalid_channel;
      try {
        Gecode::channel(invalid_channel,invalid_channel.x,3,
                        invalid_channel.b);
        return false;
      } catch (const Gecode::Word::OutOfLimits& e) {
        if (std::string(e.what()).find("Word argument out of limits") ==
            std::string::npos)
          return false;
      }

      TestSpace dom_space(1,Domain(3,0,7));
      try {
        Gecode::dom(dom_space,dom_space.x[0],0,8);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      try {
        Gecode::dom(dom_space,dom_space.x[0],8);
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}
      Gecode::dom(dom_space,dom_space.x[0],1,6);
      if (!dom_space.failed())
        return false;

      unsigned int partial = 0;
      for (PartialDomains i(3); i(); ++i) {
        Domain p = i.domain();
        if (((p.lo() & ~p.hi()) != 0) || (p.width() != 3))
          return false;
        partial++;
      }
      if (partial != 27)
        return false;
      PartialAssignment partial_assignment(2,2);
      unsigned int partial_assignments = 0;
      while (partial_assignment.has_more()) {
        for (int i=0; i<partial_assignment.size(); i++)
          if (partial_assignment[i].width() != 2)
            return false;
        partial_assignments++;
        partial_assignment.next();
      }
      if (partial_assignments != 81)
        return false;
      try {
        PartialDomains too_large(PartialDomains::max_width+1);
        (void) too_large;
        return false;
      } catch (const Gecode::Word::OutOfLimits&) {}

      Gecode::Support::RandomGenerator r0(17), r1(17);
      RandomAssignment a0(3,d,8,r0), a1(3,d,8,r1);
      while (a0.has_more() && a1.has_more()) {
        for (int i=0; i<a0.size(); i++)
          if ((a0[i] != a1[i]) || !d.in(a0[i]))
            return false;
        a0.next(r0);
        a1.next(r1);
      }
      if (a0.has_more() || a1.has_more())
        return false;

      TestSpace* s = new TestSpace(2,d);
      TestSpace* c = static_cast<TestSpace*>(s->clone());
      CpltAssignment one(2,d);
      c->assign(one,false,unused);
      const bool clone_ok = !c->failed() && c->assigned() && c->match(one) &&
        !s->assigned();
      delete c;
      delete s;
      if (!clone_ok)
        return false;

      std::ostringstream out;
      out << d << " " << one;
      return out.str() == "3'b??1 {3'h1,3'h1}";
    }
  };

  Framework framework;

}}

// STATISTICS: test-word
