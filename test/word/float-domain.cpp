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

#include <gecode/search.hh>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace Test { namespace Word { namespace FloatDomain {

  using Gecode::WordValue;
  using Gecode::WordDomainType;
  using Gecode::WordValueSetOption;
  using Gecode::WDT_CUBE;
  using Gecode::WDT_UNSIGNED;
  using Gecode::WDT_SIGNED;
  using Gecode::WDT_FP32;
  using Gecode::WDT_BF16;
  using Gecode::WDT_BINARY16;
  using Gecode::WDT_E4M3FN;
  using Gecode::WDT_E2M1;
  using Gecode::WVS_DEFAULT;
  using Gecode::WVS_EXACT;
  using Gecode::WVS_NONE;
  using Gecode::Word::WordValueSet;
  using Gecode::Word::WordView;
  using Gecode::Word::FloatOrderWordView;
  using Gecode::Word::ME_WORD_FAILED;
  using Gecode::Word::ME_WORD_NONE;
  using Gecode::Word::ME_WORD_VAL;
  using Gecode::Word::ME_WORD_BITS;
  using Gecode::Word::ME_WORD_BND;
  using Gecode::Word::ME_WORD_DOM;
  using Gecode::Word::ME_WORD_HOLE;

  /// Layout of a float format, written down independently of the library
  struct Format {
    const char* name;
    WordDomainType type;
    unsigned int width;
    bool is_tiny;
    /// Encodings in strictly increasing IEEE total order
    std::vector<WordValue> ordered_samples;
  };

  static const std::vector<Format>&
  formats(void) {
    static const std::vector<Format> all = {
      {"FP32",WDT_FP32,32,false,
       {0xffffffff,0xffc00000,0xffbfffff,0xff800001,0xff800000,0xff7fffff,
        0xbf800000,0x80000001,0x80000000,0x00000000,0x00000001,0x3f800000,
        0x7f7fffff,0x7f800000,0x7f800001,0x7fbfffff,0x7fc00000,0x7fffffff}},
      {"BF16",WDT_BF16,16,false,
       {0xffff,0xffc0,0xffbf,0xff81,0xff80,0xff7f,0xbf80,0x8001,0x8000,
        0x0000,0x0001,0x3f80,0x7f7f,0x7f80,0x7f81,0x7fbf,0x7fc0,0x7fff}},
      {"BINARY16",WDT_BINARY16,16,false,
       {0xffff,0xfe00,0xfdff,0xfc01,0xfc00,0xfbff,0xbc00,0x8001,0x8000,
        0x0000,0x0001,0x3c00,0x7bff,0x7c00,0x7c01,0x7dff,0x7e00,0x7fff}},
      {"E4M3FN",WDT_E4M3FN,8,true,
       {0xff,0xfe,0xb8,0x81,0x80,0x00,0x01,0x38,0x7e,0x7f}},
      {"E2M1",WDT_E2M1,4,true,
       {0xf,0xa,0x9,0x8,0x0,0x1,0x2,0x7}}
    };
    return all;
  }

  static WordValue
  mask_of(unsigned int width) {
    return (width >= 64) ? ~WordValue(0) : ((WordValue(1) << width)-1);
  }

  static WordValue
  sign_of(unsigned int width) {
    return WordValue(1) << (width-1);
  }

  /// Return the position of \a v in the IEEE total order of a \a width format
  static WordValue
  total_order_position(unsigned int width, WordValue v) {
    // Negative encodings come first, ordered by decreasing magnitude.
    const WordValue s = sign_of(width);
    const WordValue magnitude = v & ~s;
    return ((v & s) != 0) ? (s-1-magnitude) : (s+magnitude);
  }

  /// Return the position of \a v in the order of a \a type word
  static WordValue
  order_position(WordDomainType type, unsigned int width, WordValue v) {
    switch (type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return v;
    case WDT_SIGNED:
      return v ^ sign_of(width);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return total_order_position(width,v);
    }
    GECODE_NEVER;
    return v;
  }

  static bool
  cube_admits(WordValue lo, WordValue hi, WordValue value) {
    return ((value & lo) == lo) && ((value & ~hi) == 0);
  }

  static bool
  set_contains(const WordValueSet& set, WordValue value) {
    return ((set.blocks[value / 64] >> (value % 64)) & 1U) != 0;
  }

  /// Return the values of cube \a lo, \a hi in increasing order
  static std::vector<WordValue>
  cube_values(WordValue lo, WordValue hi) {
    std::vector<WordValue> values;
    const WordValue unknown = hi & ~lo;
    WordValue subset = 0;
    do {
      values.push_back(lo | subset);
      subset = (subset - unknown) & unknown;
    } while (subset != 0);
    return values;
  }

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

  /// Test whether creating a word with \a create throws Word::OutOfLimits
  template<class Create>
  static bool
  throws_out_of_limits(Create create) {
    WordSpace home;
    try {
      create(home);
    } catch (const Gecode::Word::OutOfLimits&) {
      return true;
    }
    return false;
  }

  /// Creation, width inference, default value sets and their errors
  class Creation : public Test::Base {
  public:
    Creation(void) : Test::Base("Word::FloatDomain::Creation") {}

    virtual bool run(void) {
      for (const Format& f : formats())
        if (!create(f)) {
          olog << f.name << std::endl;
          return false;
        }
      return bounded_e2m1() && errors();
    }

  private:
    /// Test whether \a x is a full word of format \a f
    static bool full(const Format& f, const Gecode::WordVar& x,
                     bool has_value_set) {
      const WordValue mask = mask_of(f.width);
      return (x.width() == f.width) && (x.domain_type() == f.type) &&
        x.bounded() && (x.has_value_set() == has_value_set) &&
        (x.lo() == 0) && (x.hi() == mask) &&
        (x.minimum() == mask) && (x.maximum() == (mask >> 1));
    }

    static bool create(const Format& f) {
      WordSpace home;
      const Gecode::WordVar inferred(home,f.type);
      const Gecode::WordVar explicit_width(home,f.width,f.type);
      const Gecode::WordVar opted_out(home,f.type,WVS_NONE);
      const Gecode::WordVarArgs args(home,3,f.type);
      const Gecode::WordVarArray array(home,2,f.type,WVS_NONE);
      if (!full(f,inferred,f.is_tiny) || !full(f,explicit_width,f.is_tiny) ||
          !full(f,opted_out,false))
        return false;
      for (int i=0; i<args.size(); i++)
        if (!full(f,args[i],f.is_tiny))
          return false;
      for (int i=0; i<array.size(); i++)
        if (!full(f,array[i],false))
          return false;
      if (f.is_tiny) {
        const Gecode::WordVar exact(home,f.type,WVS_EXACT);
        if (!full(f,exact,true))
          return false;
      }
      // A width-inferring constructor is the same as the explicit width.
      const unsigned int wrong = (f.width == 32) ? 16 : 2*f.width;
      const Gecode::WordDomainType type = f.type;
      return
        throws_out_of_limits([wrong,type](WordSpace& s) {
          Gecode::WordVar(s,wrong,type);
        }) &&
        throws_out_of_limits([wrong,type](WordSpace& s) {
          Gecode::WordVar(s,wrong,type,0,0);
        }) &&
        throws_out_of_limits([wrong,type](WordSpace& s) {
          Gecode::WordVar(s,wrong,type,0,0,Gecode::known_bits(0,0));
        }) &&
        throws_out_of_limits([wrong,type](WordSpace& s) {
          Gecode::WordVarArgs(s,2,wrong,type);
        }) &&
        throws_out_of_limits([wrong,type](WordSpace& s) {
          Gecode::WordVarArray(s,2,wrong,type);
        }) &&
        (f.is_tiny ||
         (throws_out_of_limits([type](WordSpace& s) {
            Gecode::WordVar(s,type,WVS_EXACT);
          }) &&
          throws_out_of_limits([type](WordSpace& s) {
            Gecode::WordVarArgs(s,1,type,WVS_EXACT);
          })));
    }

    /// E2M1 bounds given as encodings, from -1.5 to +3, in total order
    static bool bounded_e2m1(void) {
      WordSpace home;
      const Gecode::WordVar with_set(home,4,WDT_E2M1,0xb,0x5);
      const Gecode::WordVar without_set(home,4,WDT_E2M1,0xb,0x5,WVS_NONE);
      const std::vector<WordValue> members = {0xb,0xa,0x9,0x8,0x0,0x1,0x2,
                                              0x3,0x4,0x5};
      for (const Gecode::WordVar& x : {with_set,without_set}) {
        if ((x.minimum() != 0xb) || (x.maximum() != 0x5))
          return false;
        for (WordValue v=0; v<16; v++)
          if (x.in(v) != (std::find(members.begin(),members.end(),v) !=
                          members.end()))
            return false;
      }
      if (!with_set.has_value_set() || without_set.has_value_set())
        return false;
      try {
        Gecode::WordVar(home,4,WDT_E2M1,0x5,0xb);
        return false;
      } catch (const Gecode::Word::VariableEmptyDomain&) {}
      return true;
    }

    static bool errors(void) {
      WordSpace home;
      const Gecode::WordVar plain(home,4,WDT_UNSIGNED);
      const Gecode::WordVar cube(home,4);
      if (plain.has_value_set() || cube.has_value_set())
        return false;
      return
        throws_out_of_limits([](WordSpace& s) {
          Gecode::WordVar(s,WDT_UNSIGNED);
        }) &&
        throws_out_of_limits([](WordSpace& s) {
          Gecode::WordVarArgs(s,1,WDT_CUBE);
        }) &&
        throws_out_of_limits([](WordSpace& s) {
          Gecode::WordVarArray(s,1,WDT_SIGNED);
        });
    }
  } creation;

  /// Rank encoding and decoding agree with the IEEE total order
  class RoundTrip : public Test::Base {
  public:
    RoundTrip(void) : Test::Base("Word::FloatDomain::RoundTrip") {}

    virtual bool run(void) {
      for (const Format& f : formats())
        if (!samples_in_order(f) || !round_trips(f)) {
          olog << f.name << std::endl;
          return false;
        }
      return true;
    }

  private:
    static bool samples_in_order(const Format& f) {
      for (std::size_t i=1; i<f.ordered_samples.size(); i++)
        if (Gecode::Word::rank(f.type,f.width,f.ordered_samples[i-1]) >=
            Gecode::Word::rank(f.type,f.width,f.ordered_samples[i]))
          return false;
      return true;
    }

    /// Test one encoding \a v, also read as the rank \a v
    static bool round_trips(const Format& f, WordValue v) {
      const WordValue mask = mask_of(f.width);
      const WordValue r = Gecode::Word::rank(f.type,f.width,v);
      const WordValue e = Gecode::Word::encode_rank(f.type,f.width,v);
      return (r <= mask) && (e <= mask) &&
        (r == total_order_position(f.width,v)) &&
        (Gecode::Word::encode_rank(f.type,f.width,r) == v) &&
        (Gecode::Word::rank(f.type,f.width,e) == v);
    }

    bool round_trips(const Format& f) {
      if (f.width <= 16) {
        for (WordValue v=0; v<=mask_of(f.width); v++)
          if (!round_trips(f,v))
            return false;
        return true;
      }
      const WordValue s = sign_of(f.width);
      const WordValue boundaries[] = {0,1,s-1,s,s+1,mask_of(f.width),
                                      mask_of(f.width)-1};
      for (WordValue v : boundaries)
        if (!round_trips(f,v))
          return false;
      for (WordValue v : f.ordered_samples)
        if (!round_trips(f,v))
          return false;
      for (unsigned int i=0; i<100000; i++) {
        const WordValue v = (WordValue(_rand(1U << 16)) << 16) |
          _rand(1U << 16);
        if (!round_trips(f,v))
          return false;
      }
      return true;
    }
  } round_trip;

  /** \brief Independent reference domain over an enumerated universe
   *
   * The universe holds every value a word can take: the values of its
   * creation cube. With a value set, membership is exact. Without one, the
   * domain stays the values of its bit hull whose rank lies between the
   * least and greatest rank, which is exactly what cube and bounds express.
   */
  struct Reference {
    unsigned int width;
    WordDomainType type;
    bool has_value_set;
    std::vector<WordValue> universe;
    std::vector<bool> member;

    WordValue mask(void) const { return mask_of(width); }
    WordValue position(WordValue v) const {
      return total_order_position(width,v);
    }
  };

  /// Hull, rank extremes and size of a reference domain
  struct Summary {
    unsigned int size;
    WordValue lo, hi;
    WordValue rank_minimum, rank_maximum;
    WordValue minimum, maximum;
  };

  static Summary
  summarize(const Reference& r) {
    Summary s = {0,~WordValue(0),0,~WordValue(0),0,0,0};
    for (std::size_t i=0; i<r.universe.size(); i++)
      if (r.member[i]) {
        const WordValue v = r.universe[i];
        s.size++;
        s.lo &= v;
        s.hi |= v;
        if (r.position(v) <= s.rank_minimum) {
          s.rank_minimum = r.position(v);
          s.minimum = v;
        }
        if (r.position(v) >= s.rank_maximum) {
          s.rank_maximum = r.position(v);
          s.maximum = v;
        }
      }
    return s;
  }

  /// Shrink a reference without a value set to what cube and bounds express
  static void
  close_reference(Reference& r) {
    if (r.has_value_set)
      return;
    const Summary s = summarize(r);
    for (std::size_t i=0; i<r.universe.size(); i++) {
      const WordValue v = r.universe[i];
      r.member[i] = (s.size != 0) && cube_admits(s.lo,s.hi,v) &&
        (r.position(v) >= s.rank_minimum) && (r.position(v) <= s.rank_maximum);
    }
  }

  /// Keep the members admitted by cube \a lo, \a hi and the rank interval
  static void
  restrict_reference(Reference& r, WordValue lo, WordValue hi,
                     WordValue minimum, WordValue maximum) {
    for (std::size_t i=0; i<r.universe.size(); i++) {
      const WordValue v = r.universe[i];
      if (!cube_admits(lo,hi,v) || (r.position(v) < minimum) ||
          (r.position(v) > maximum))
        r.member[i] = false;
    }
  }

  /// Return the event that changing \a before to its subset \a after reports
  static Gecode::ModEvent
  expected_event(const Summary& before, const Summary& after) {
    if (after.size == 0)
      return ME_WORD_FAILED;
    if (after.size == before.size)
      return ME_WORD_NONE;
    if (after.size == 1)
      return ME_WORD_VAL;
    const bool has_new_bits =
      (after.lo != before.lo) || (after.hi != before.hi);
    const bool has_new_bounds =
      (after.rank_minimum != before.rank_minimum) ||
      (after.rank_maximum != before.rank_maximum);
    if (has_new_bits && has_new_bounds)
      return ME_WORD_DOM;
    if (has_new_bits)
      return ME_WORD_BITS;
    if (has_new_bounds)
      return ME_WORD_BND;
    return ME_WORD_HOLE;
  }

  /// Test whether \a x has exactly the domain of \a r
  static bool
  agrees(const Gecode::WordVar& x, const Reference& r) {
    const Summary s = summarize(r);
    if ((x.has_value_set() != r.has_value_set) || (x.lo() != s.lo) ||
        (x.hi() != s.hi) || (x.assigned() != (s.size == 1)) ||
        (x.minimum() != s.minimum) || (x.maximum() != s.maximum))
      return false;
    const WordView v(x);
    if ((v.rank_minimum() != s.rank_minimum) ||
        (v.rank_maximum() != s.rank_maximum))
      return false;
    WordValueSet expected = {{0,0,0,0}};
    for (std::size_t i=0; i<r.universe.size(); i++) {
      if (x.in(r.universe[i]) != r.member[i])
        return false;
      if (r.member[i] && (r.width <= 8))
        expected.blocks[r.universe[i] / 64] |=
          WordValue(1) << (r.universe[i] % 64);
    }
    return (r.width > 8) ||
      Gecode::Word::same_value_sets(v.values(),expected);
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
   * Randomised tells on one float format, compared after every tell with
   * an independent reference. Clones are told independently. Tiny formats
   * run with and without a value set over all their encodings; wide
   * formats run without one, on creation cubes with at most ten unknown
   * bits that reach NaNs, infinities, zeros and subnormals.
   */
  class Domain : public Test::Base {
  public:
    Domain(const Format& f)
      : Test::Base(std::string("Word::FloatDomain::Domain::")+f.name),
        format(f) {}

    virtual bool run(void) {
      const unsigned int sequences = format.is_tiny ? 40 : 30;
      for (unsigned int i=0; i<sequences; i++) {
        const bool has_value_set = format.is_tiny && ((i % 2) == 0);
        if (!run_sequence(has_value_set)) {
          olog << "sequence " << i << std::endl;
          return false;
        }
      }
      return true;
    }

  private:
    const Format& format;

    bool run_sequence(bool has_value_set) {
      std::vector<Run> runs;
      runs.push_back(create(has_value_set));
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
        const Tell tell = choose_tell();
        const Gecode::ModEvent me = apply(tell,runs[i]);
        const Summary after = summarize(runs[i].reference);
        if (me != expected_event(before,after)) {
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

    WordValue random_word(WordValue mask) {
      return ((WordValue(_rand(1U << 16)) << 16) | _rand(1U << 16)) & mask;
    }

    /** \brief Return a creation cube of the wide format with at most ten
     * unknown bits
     *
     * The special cubes vary the sign, the lowest exponent bit, the quiet
     * bit and low fraction bits: the first fixes the other exponent bits to
     * one and reaches infinities, NaNs and the largest finite numbers, the
     * second fixes them to zero and reaches zeros, subnormals and the least
     * normal numbers.
     */
    void wide_cube(WordValue& lo, WordValue& hi) {
      const unsigned int fraction_bits =
        (format.type == WDT_FP32) ? 23 : ((format.type == WDT_BF16) ? 7 : 10);
      const WordValue mask = mask_of(format.width);
      const WordValue s = sign_of(format.width);
      const WordValue exponent = (mask & ~s) & ~mask_of(fraction_bits);
      const WordValue lowest_exponent = WordValue(1) << fraction_bits;
      const WordValue quiet = WordValue(1) << (fraction_bits-1);
      const WordValue special = s | lowest_exponent | quiet | 3;
      switch (_rand(4U)) {
      case 0:
        lo = exponent & ~lowest_exponent;
        hi = lo | special;
        return;
      case 1:
        lo = 0;
        hi = special | (s >> 1);
        return;
      default: {
        lo = random_word(mask);
        WordValue unknown = (_rand(2U) == 0) ? s : 0;
        const unsigned int count = 3 + _rand(7U);
        for (unsigned int k=0; k<count; k++)
          unknown |= WordValue(1) << _rand(format.width);
        lo &= ~unknown;
        hi = lo | unknown;
        return;
      }
      }
    }

    Run create(bool has_value_set) {
      Run run;
      run.space.reset(new WordSpace);
      WordSpace& home = *run.space;
      const unsigned int width = format.width;
      WordValue lo = 0, hi = mask_of(width);
      if (!format.is_tiny)
        wide_cube(lo,hi);
      Reference& r = run.reference;
      r = {width,format.type,has_value_set,
           format.is_tiny ? cube_values(0,mask_of(width)) : cube_values(lo,hi),
           {}};
      r.member.assign(r.universe.size(),true);
      const WordValueSetOption option = has_value_set ? WVS_DEFAULT : WVS_NONE;
      if (format.is_tiny && (_rand(3U) == 0)) {
        home.x = Gecode::WordVar(home,format.type,option);
        return run;
      }
      if (format.is_tiny && (_rand(2U) == 0)) {
        lo = 0; hi = mask_of(width);
        const WordValue bit = WordValue(1) << _rand(width);
        if (_rand(2U) == 0) lo |= bit; else hi &= ~bit;
      }
      WordValue minimum, maximum;
      random_ranks(r,minimum,maximum);
      restrict_reference(r,lo,hi,minimum,maximum);
      close_reference(r);
      try {
        home.x = Gecode::WordVar(
          home,width,format.type,
          Gecode::Word::encode_rank(format.type,width,minimum),
          Gecode::Word::encode_rank(format.type,width,maximum),
          cube_bits(width,lo,hi),option);
      } catch (const Gecode::Word::VariableEmptyDomain&) {
        if (summarize(r).size != 0)
          throw;
        r.member.assign(r.universe.size(),true);
        restrict_reference(r,lo,hi,0,r.mask());
        home.x = Gecode::WordVar(home,width,format.type,r.mask(),
                                 r.mask() >> 1,cube_bits(width,lo,hi),option);
      }
      return run;
    }

    Tell choose_tell(void) {
      const Tell tells[] = {Tell::CUBE,Tell::CUBE_AND_RANKS,Tell::RANKS,
                            Tell::RANKS,Tell::EQ,Tell::NQ,Tell::NQ,
                            Tell::VALUES};
      return tells[_rand(format.is_tiny ? 8U : 7U)];
    }

    /// Return a member of \a r most of the time, otherwise any value
    WordValue random_value(const Reference& r) {
      std::vector<WordValue> members;
      for (std::size_t i=0; i<r.universe.size(); i++)
        if (r.member[i])
          members.push_back(r.universe[i]);
      if (!members.empty() && (_rand(4U) != 0))
        return members[_rand(static_cast<unsigned int>(members.size()))];
      return r.universe[_rand(static_cast<unsigned int>(r.universe.size()))];
    }

    /// Return a cube fixing one or two bits, often the sign bit
    void random_cube(const Reference& r, WordValue& lo, WordValue& hi) {
      lo = 0;
      hi = r.mask();
      const unsigned int fixed = 1U + _rand(2U);
      for (unsigned int k=0; k<fixed; k++) {
        const WordValue bit = (_rand(3U) == 0) ? sign_of(r.width) :
          (WordValue(1) << _rand(r.width));
        if (_rand(2U) != 0)
          lo |= bit;
        else
          hi &= ~bit;
      }
    }

    /// Return a rank interval near member ranks, cut extremes, or random
    void random_ranks(const Reference& r, WordValue& minimum,
                      WordValue& maximum) {
      const WordValue mask = r.mask();
      switch (_rand(4U)) {
      case 0: {
        minimum = random_word(mask);
        maximum = random_word(mask);
        if (minimum > maximum)
          std::swap(minimum,maximum);
        return;
      }
      case 1: {
        minimum = r.position(random_value(r));
        maximum = r.position(random_value(r));
        if (minimum > maximum)
          std::swap(minimum,maximum);
        const WordValue widen = _rand(3U);
        minimum = (minimum >= widen) ? (minimum-widen) : 0;
        maximum = std::min(maximum+_rand(3U),mask);
        return;
      }
      default: {
        const Summary s = summarize(r);
        if (s.size == 0) {
          minimum = 0; maximum = mask;
          return;
        }
        const WordValue cut_minimum = _rand(3U);
        const WordValue cut_maximum = _rand(3U);
        minimum = std::min(s.rank_minimum+cut_minimum,mask);
        maximum = (s.rank_maximum >= cut_maximum) ?
          (s.rank_maximum-cut_maximum) : 0;
        return;
      }
      }
    }

    /// Apply \a tell to the word of \a run and the same filter to its reference
    Gecode::ModEvent apply(Tell tell, Run& run) {
      WordSpace& home = *run.space;
      Reference& r = run.reference;
      WordView x(home.x);
      WordValue lo, hi, minimum, maximum;
      switch (tell) {
      case Tell::CUBE:
        random_cube(r,lo,hi);
        restrict_reference(r,lo,hi,0,r.mask());
        close_reference(r);
        return x.narrow(home,lo,hi);
      case Tell::CUBE_AND_RANKS:
        random_cube(r,lo,hi);
        random_ranks(r,minimum,maximum);
        restrict_reference(r,lo,hi,minimum,maximum);
        close_reference(r);
        return FloatOrderWordView(home.x).
          narrow_domain(home,lo,hi,minimum,maximum);
      case Tell::RANKS:
        random_ranks(r,minimum,maximum);
        restrict_reference(r,0,r.mask(),minimum,maximum);
        close_reference(r);
        return x.narrow_rank_range(home,minimum,maximum);
      case Tell::EQ: {
        const WordValue v = random_value(r);
        for (std::size_t i=0; i<r.universe.size(); i++)
          if (r.universe[i] != v)
            r.member[i] = false;
        return x.eq(home,v);
      }
      case Tell::NQ: {
        const WordValue v = random_value(r);
        remove_reference_value(r,v);
        return x.nq(home,v);
      }
      case Tell::VALUES: {
        const unsigned int density = 2U + _rand(6U);
        WordValueSet candidates = {{0,0,0,0}};
        for (WordValue v=0; v<256; v++)
          if (_rand(density) != 0)
            candidates.blocks[v / 64] |= WordValue(1) << (v % 64);
        for (std::size_t i=0; i<r.universe.size(); i++)
          if (!set_contains(candidates,r.universe[i]))
            r.member[i] = false;
        close_reference(r);
        return x.narrow_values(home,candidates);
      }
      }
      GECODE_NEVER;
      return ME_WORD_FAILED;
    }

    /** \brief Remove \a v as WordVarImp::nq does
     *
     * Without a value set, only a value at a rank extreme can go.
     */
    static void remove_reference_value(Reference& r, WordValue v) {
      const Summary s = summarize(r);
      const bool is_extreme = (r.position(v) == s.rank_minimum) ||
        (r.position(v) == s.rank_maximum);
      if (!r.has_value_set && !is_extreme && (s.size > 1))
        return;
      for (std::size_t i=0; i<r.universe.size(); i++)
        if (r.universe[i] == v)
          r.member[i] = false;
      close_reference(r);
    }
  };

  Domain domain_fp32(formats()[0]);
  Domain domain_bf16(formats()[1]);
  Domain domain_binary16(formats()[2]);
  Domain domain_e4m3fn(formats()[3]);
  Domain domain_e2m1(formats()[4]);

  /// Probe counters, used only by the Events test
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

  /// Advisor delta captured by DeltaProbe, used only by the Events test
  struct DeltaCapture {
    unsigned int count;
    Gecode::ModEvent event;
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

  /// Space whose word is watched by one probe per condition and an advisor
  class ProbeSpace : public WordSpace {
  public:
    ProbeSpace(WordDomainType type) {
      x = Gecode::WordVar(*this,type);
      WordView xv(x);
      Probe<Gecode::Word::PC_WORD_VAL,0>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_BITS,1>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_BND,2>::post(*this,xv);
      Probe<Gecode::Word::PC_WORD_DOM,3>::post(*this,xv);
      DeltaProbe::post(*this,xv);
    }
    ProbeSpace(ProbeSpace& s) : WordSpace(s) {}
    virtual Gecode::Space* copy(void) { return new ProbeSpace(*this); }
  };

  /// Float bound, bit and hole events, their scheduling and deltas
  class Events : public Test::Base {
  public:
    Events(void) : Test::Base("Word::FloatDomain::Events") {}

    virtual bool run(void) {
      return bound_change() && sign_change() && bit_change() &&
        hole_change();
    }

  private:
    static void reset(void) {
      for (unsigned int i=0; i<4; i++) {
        probe_count[i] = 0;
        probe_event[i] = ME_WORD_NONE;
      }
      delta_capture = {0,ME_WORD_NONE,0,0,0,0};
    }

    /// Test which probes ran since the last reset, each seeing \a event
    static bool probes_ran(bool val, bool bits, bool bnd, bool dom,
                           Gecode::ModEvent event) {
      const bool ran[4] = {val,bits,bnd,dom};
      for (unsigned int i=0; i<4; i++) {
        if (probe_count[i] != (ran[i] ? 1U : 0U))
          return false;
        if (ran[i] && (probe_event[i] != event))
          return false;
      }
      return (delta_capture.count == 1) && (delta_capture.event == event);
    }

    /// Run \a s to its first fixpoint and reset the probes
    static bool settle(ProbeSpace& s) {
      const bool ok = s.status() != Gecode::SS_FAILED;
      reset();
      return ok;
    }

    /// Removing the least FP32 value, a negative NaN, moves only a bound
    static bool bound_change(void) {
      ProbeSpace s(WDT_FP32);
      if (!settle(s))
        return false;
      const WordValue mask = 0xffffffff;
      if (WordView(s.x).nq(s,0xffffffff) != ME_WORD_BND)
        return false;
      return (s.status() != Gecode::SS_FAILED) &&
        probes_ran(false,false,true,true,ME_WORD_BND) &&
        (delta_capture.old_minimum == 0) &&
        (delta_capture.old_maximum == mask) &&
        (delta_capture.new_minimum == 1) &&
        (delta_capture.new_maximum == mask) &&
        (s.x.minimum() == 0xfffffffe) && (s.x.maximum() == 0x7fffffff);
    }

    /// Fixing the sign bit to zero moves bits and the lower bound to +0
    static bool sign_change(void) {
      ProbeSpace s(WDT_FP32);
      if (!settle(s))
        return false;
      if (WordView(s.x).narrow(s,0,0x7fffffff) != ME_WORD_DOM)
        return false;
      return (s.status() != Gecode::SS_FAILED) &&
        probes_ran(false,true,true,true,ME_WORD_DOM) &&
        (delta_capture.new_minimum == 0x80000000) &&
        (delta_capture.new_maximum == 0xffffffff) &&
        (s.x.minimum() == 0) && (s.x.maximum() == 0x7fffffff);
    }

    /// The least and greatest FP32 values are odd, so fixing bit 0 to one
    /// keeps the bounds
    static bool bit_change(void) {
      ProbeSpace s(WDT_FP32);
      if (!settle(s))
        return false;
      if (WordView(s.x).narrow(s,1,0xffffffff) != ME_WORD_BITS)
        return false;
      return (s.status() != Gecode::SS_FAILED) &&
        probes_ran(false,true,false,true,ME_WORD_BITS) &&
        (s.x.minimum() == 0xffffffff) && (s.x.maximum() == 0x7fffffff);
    }

    /// Removing +0.5 from a full E2M1 set leaves cube and bounds
    static bool hole_change(void) {
      ProbeSpace s(WDT_E2M1);
      if (!settle(s) || !s.x.has_value_set())
        return false;
      if (WordView(s.x).nq(s,0x1) != ME_WORD_HOLE)
        return false;
      return (s.status() != Gecode::SS_FAILED) &&
        probes_ran(false,false,false,true,ME_WORD_HOLE) &&
        (delta_capture.old_minimum == 0) && (delta_capture.new_minimum == 0) &&
        (delta_capture.old_maximum == 15) &&
        (delta_capture.new_maximum == 15) && !s.x.in(0x1) &&
        (s.x.minimum() == 0xf) && (s.x.maximum() == 0x7);
    }
  } events;

  /// Propagator tells on FP32 apply every move in float rank order
  class ExactNarrowing : public Test::Base {
  public:
    ExactNarrowing(void) : Test::Base("Word::FloatDomain::ExactNarrowing") {}

    virtual bool run(void) {
      return small_moves() && sign_cube() && assignment_and_failure() &&
        exact_tells() && exact_split();
    }

  private:
    static constexpr WordValue mask = 0xffffffff;
    static constexpr WordValue sign = 0x80000000;

    static Gecode::ModEvent narrow(WordSpace& s, WordValue lo, WordValue hi,
                                   WordValue minimum, WordValue maximum) {
      return Gecode::Word::narrow_word_domain(
        s,WordView(s.x),{lo,hi},{minimum,maximum});
    }

    static bool ranks(const WordSpace& s, WordValue minimum,
                      WordValue maximum) {
      const WordView x(s.x);
      return (x.rank_minimum() == minimum) && (x.rank_maximum() == maximum);
    }

    static bool small_moves(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,WDT_FP32);
      if ((narrow(s,0,mask,1,mask) != ME_WORD_BND) || !ranks(s,1,mask))
        return false;
      if ((narrow(s,0,mask,0,mask-1) != ME_WORD_BND) || !ranks(s,1,mask-1))
        return false;
      const WordValue move = WordValue(1) << 27;
      return (narrow(s,0,mask,move,mask) == ME_WORD_BND) &&
        ranks(s,move,mask-1);
    }

    static bool sign_cube(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,WDT_FP32);
      if ((narrow(s,0,mask >> 1,0,mask) != ME_WORD_DOM) ||
          !ranks(s,sign,mask) || (s.x.minimum() != 0))
        return false;
      return (narrow(s,0,mask,sign+1,mask) == ME_WORD_BND) &&
        ranks(s,sign+1,mask);
    }

    static bool assignment_and_failure(void) {
      WordSpace assigned;
      assigned.x = Gecode::WordVar(assigned,WDT_FP32);
      if ((narrow(assigned,0,mask,sign+12345,sign+12345) != ME_WORD_VAL) ||
          !assigned.x.assigned() || (assigned.x.val() != 12345))
        return false;
      WordSpace inverted;
      inverted.x = Gecode::WordVar(inverted,WDT_FP32);
      if (narrow(inverted,0,mask,5,4) != ME_WORD_FAILED)
        return false;
      WordSpace excluded;
      excluded.x = Gecode::WordVar(excluded,WDT_FP32);
      return (narrow(excluded,0,mask >> 1,0,mask) == ME_WORD_DOM) &&
        (narrow(excluded,0,mask,0,sign-1) == ME_WORD_FAILED);
    }

    static bool exact_tells(void) {
      WordSpace s;
      s.x = Gecode::WordVar(s,WDT_FP32);
      WordView x(s.x);
      if ((x.narrow_rank_range(s,1,mask) != ME_WORD_BND) || !ranks(s,1,mask))
        return false;
      if ((x.nq(s,0x7fffffff) != ME_WORD_BND) || !ranks(s,1,mask-1))
        return false;
      Gecode::dom(s,s.x,WordValue(0x3f800000));
      return (s.status() != Gecode::SS_FAILED) && s.x.assigned() &&
        (s.x.val() == 0x3f800000);
    }

    /// The first split alternative of a full word bounds it at -0
    static bool exact_split(void) {
      WordSpace* root = new WordSpace;
      root->x = Gecode::WordVar(*root,WDT_FP32);
      Gecode::branch(*root,root->x,Gecode::WORD_VAL_SPLIT_MIN());
      if (root->status() != Gecode::SS_BRANCH) {
        delete root;
        return false;
      }
      const Gecode::Choice* choice = root->choice();
      WordSpace* lower = static_cast<WordSpace*>(root->clone());
      lower->commit(*choice,0);
      const bool ok = (lower->status() != Gecode::SS_FAILED) &&
        ranks(*lower,0,sign-1) && (lower->x.maximum() == sign);
      delete choice;
      delete lower;
      delete root;
      return ok;
    }
  } exact_narrowing;

  /// Description of one word of an integer-constraint case
  struct WordSpec {
    WordDomainType type;
    unsigned int width;
    WordValue lo, hi;
    /// Encoded endpoints, unused for WDT_CUBE
    WordValue minimum, maximum;
    WordValueSetOption value_set;
  };

  /// Test whether \a v belongs to the domain that \a w describes
  static bool
  spec_admits(const WordSpec& w, WordValue v) {
    if (!cube_admits(w.lo,w.hi,v))
      return false;
    if (w.type == WDT_CUBE)
      return true;
    const WordValue r = order_position(w.type,w.width,v);
    return (r >= order_position(w.type,w.width,w.minimum)) &&
      (r <= order_position(w.type,w.width,w.maximum));
  }

  /// Integer constraints run on float and mixed words
  enum class Constraint {
    EQ, ULE, SLE, REIFIED_SLQ, ADD, MULT, SIGNED_DIV, DIVMOD, EXTENSIONAL,
    XOR
  };

  /// Return the number of words that \a c relates
  static unsigned int
  arity(Constraint c) {
    switch (c) {
    case Constraint::EQ: case Constraint::ULE: case Constraint::SLE:
    case Constraint::REIFIED_SLQ: case Constraint::EXTENSIONAL:
      return 2;
    case Constraint::ADD: case Constraint::MULT: case Constraint::SIGNED_DIV:
    case Constraint::XOR:
      return 3;
    case Constraint::DIVMOD:
      return 4;
    }
    GECODE_NEVER;
    return 0;
  }

  static std::int64_t
  signed_value(unsigned int width, WordValue v) {
    const WordValue s = sign_of(width);
    return ((v & s) != 0) ? static_cast<std::int64_t>(v) -
      static_cast<std::int64_t>(2*s) : static_cast<std::int64_t>(v);
  }

  /// Return the SMT-LIB signed quotient of \a x by \a y
  static WordValue
  signed_quotient(unsigned int width, WordValue x, WordValue y) {
    const WordValue mask = mask_of(width);
    if (y == 0)
      return ((x & sign_of(width)) != 0) ? 1 : mask;
    const std::int64_t q = signed_value(width,x) / signed_value(width,y);
    return static_cast<WordValue>(q) & mask;
  }

  /// Test whether the table of the extensional cases holds \a x, \a y
  static bool
  table_holds(unsigned int width, WordValue x, WordValue y) {
    const WordValue mask = mask_of(width);
    return (y == (x ^ (mask >> 1))) || (y == ((x+5) & mask));
  }

  /// Bit-level oracle of \a c on the words \a v, with Boolean \a b
  static bool
  holds(Constraint c, unsigned int width, const std::vector<WordValue>& v,
        bool b) {
    const WordValue mask = mask_of(width);
    switch (c) {
    case Constraint::EQ:
      return v[0] == v[1];
    case Constraint::ULE:
      return v[0] < v[1];
    case Constraint::SLE:
      return signed_value(width,v[0]) < signed_value(width,v[1]);
    case Constraint::REIFIED_SLQ:
      return b == (signed_value(width,v[0]) <= signed_value(width,v[1]));
    case Constraint::ADD:
      return v[2] == ((v[0]+v[1]) & mask);
    case Constraint::MULT:
      return v[2] == ((v[0]*v[1]) & mask);
    case Constraint::SIGNED_DIV:
      return v[2] == signed_quotient(width,v[0],v[1]);
    case Constraint::DIVMOD:
      if (v[1] == 0)
        return (v[2] == mask) && (v[3] == v[0]);
      return (v[2] == v[0] / v[1]) && (v[3] == v[0] % v[1]);
    case Constraint::EXTENSIONAL:
      return table_holds(width,v[0],v[1]);
    case Constraint::XOR:
      return v[2] == (v[0] ^ v[1]);
    }
    GECODE_NEVER;
    return false;
  }

  /// One integer-constraint case on words of the same width
  struct IntegerCase {
    Constraint constraint;
    std::vector<WordSpec> words;
  };

  /// Space posting one integer constraint and branching on its words
  class IntegerSpace : public Gecode::Space {
  public:
    Gecode::WordVarArray x;
    Gecode::BoolVar b;
    IntegerSpace(const IntegerCase& c)
      : x(*this,static_cast<int>(c.words.size())), b(*this,0,1) {
      for (std::size_t i=0; i<c.words.size(); i++) {
        const WordSpec& w = c.words[i];
        x[static_cast<int>(i)] = (w.type == WDT_CUBE) ?
          Gecode::WordVar(*this,w.width,cube_bits(w.width,w.lo,w.hi),w.value_set) :
          Gecode::WordVar(*this,w.width,w.type,w.minimum,
                          w.maximum,cube_bits(w.width,w.lo,w.hi),w.value_set);
      }
      post(c.constraint,c.words[0].width);
      Gecode::branch(*this,x,Gecode::WORD_VAR_NONE(),Gecode::WORD_VAL_LSB());
      Gecode::branch(*this,b,Gecode::BOOL_VAL_MIN());
    }
    IntegerSpace(IntegerSpace& s) : Gecode::Space(s) {
      x.update(*this,s.x);
      b.update(*this,s.b);
    }
    virtual Gecode::Space* copy(void) { return new IntegerSpace(*this); }
  private:
    void post(Constraint c, unsigned int width) {
      using namespace Gecode;
      switch (c) {
      case Constraint::EQ: rel(*this,x[0],WRT_EQ,x[1]); return;
      case Constraint::ULE: rel(*this,x[0],WRT_ULE,x[1]); return;
      case Constraint::SLE: rel(*this,x[0],WRT_SLE,x[1]); return;
      case Constraint::REIFIED_SLQ:
        rel(*this,x[0],WRT_SLQ,x[1],Reify(b));
        return;
      case Constraint::ADD: add(*this,x[0],x[1],x[2]); return;
      case Constraint::MULT: mult(*this,x[0],x[1],x[2]); return;
      case Constraint::SIGNED_DIV: signed_div(*this,x[0],x[1],x[2]); return;
      case Constraint::DIVMOD: divmod(*this,x[0],x[1],x[2],x[3]); return;
      case Constraint::EXTENSIONAL: {
        WordTupleSet table(IntArgs({static_cast<int>(width),
                                    static_cast<int>(width)}));
        const WordValue mask = mask_of(width);
        for (WordValue v=0; v<=mask; v++) {
          table.add(WordValArgs({v,v ^ (mask >> 1)}));
          table.add(WordValArgs({v,(v+5) & mask}));
        }
        table.finalize();
        extensional(*this,WordVarArgs({x[0],x[1]}),table);
        return;
      }
      case Constraint::XOR: rel(*this,x[0],WOT_XOR,x[1],x[2]); return;
      }
    }
  };

  /// A solution: the words' values followed by the Boolean
  typedef std::vector<WordValue> Solution;

  static std::vector<Solution>
  search_solutions(const IntegerCase& c) {
    IntegerSpace* root = new IntegerSpace(c);
    Gecode::DFS<IntegerSpace> dfs(root);
    delete root;
    std::vector<Solution> found;
    while (IntegerSpace* s = dfs.next()) {
      Solution solution;
      for (int i=0; i<s->x.size(); i++)
        solution.push_back(s->x[i].val());
      solution.push_back(static_cast<WordValue>(s->b.val()));
      found.push_back(solution);
      delete s;
    }
    std::sort(found.begin(),found.end());
    return found;
  }

  /** \brief Return the oracle's solutions
   *
   * Inputs range over their spec's values; the outputs of an operation
   * follow from them and must lie in their own domains. The Boolean is
   * free unless the case is reified.
   */
  static std::vector<Solution>
  oracle_solutions(const IntegerCase& c) {
    const unsigned int width = c.words[0].width;
    const unsigned int n = arity(c.constraint);
    const unsigned int inputs = 2;
    std::vector<std::vector<WordValue> > domains(inputs);
    for (unsigned int i=0; i<inputs; i++)
      for (WordValue v : cube_values(c.words[i].lo,c.words[i].hi))
        if (spec_admits(c.words[i],v))
          domains[i].push_back(v);
    std::vector<Solution> found;
    for (WordValue a : domains[0])
      for (WordValue d : domains[1]) {
        std::vector<WordValue> v = {a,d};
        v.resize(n,0);
        if (n >= 3) {
          // Read off the outputs of the operation, then check them.
          const WordValue mask = mask_of(width);
          switch (c.constraint) {
          case Constraint::ADD: v[2] = (a+d) & mask; break;
          case Constraint::MULT: v[2] = (a*d) & mask; break;
          case Constraint::SIGNED_DIV: v[2] = signed_quotient(width,a,d); break;
          case Constraint::XOR: v[2] = a ^ d; break;
          case Constraint::DIVMOD:
            v[2] = (d == 0) ? mask : a / d;
            v[3] = (d == 0) ? a : a % d;
            break;
          default: GECODE_NEVER;
          }
        }
        bool is_admitted = true;
        for (unsigned int i=inputs; i<n; i++)
          is_admitted = is_admitted && spec_admits(c.words[i],v[i]);
        if (!is_admitted)
          continue;
        for (bool b : {false,true}) {
          if (!holds(c.constraint,width,v,b))
            continue;
          Solution solution(v.begin(),v.end());
          solution.push_back(b ? 1 : 0);
          found.push_back(solution);
        }
      }
    std::sort(found.begin(),found.end());
    return found;
  }

  /**
   * Integer Word constraints act on the bits of float words. Complete
   * solution sets over float, unsigned, signed and cube words are compared
   * with bit-level oracles. The float words have restricted total-order
   * intervals, so a rank read in another order would change the domains.
   */
  class IntegerConstraints : public Test::Base {
  public:
    IntegerConstraints(void)
      : Test::Base("Word::FloatDomain::IntegerConstraints") {}

    virtual bool run(void) {
      for (const IntegerCase& c : cases()) {
        const std::vector<Solution> found = search_solutions(c);
        const std::vector<Solution> expected = oracle_solutions(c);
        if (found != expected) {
          olog << "constraint " << static_cast<int>(c.constraint)
               << " width " << c.words[0].width << " found " << found.size()
               << " expected " << expected.size() << std::endl;
          return false;
        }
      }
      return true;
    }

  private:
    static std::vector<IntegerCase> cases(void) {
      // E2M1 from -1.5 to +3, with and without a value set.
      const WordSpec a = {WDT_E2M1,4,0,15,0xb,0x5,WVS_DEFAULT};
      const WordSpec b = {WDT_E2M1,4,0,15,0xb,0x5,WVS_NONE};
      const WordSpec u = {WDT_UNSIGNED,4,0,15,2,13,WVS_NONE};
      const WordSpec s = {WDT_SIGNED,4,0,15,0xb,0x6,WVS_NONE};
      const WordSpec c = {WDT_CUBE,4,1,15,0,0,WVS_NONE};
      // E4M3FN with four unknown bits, including the sign, from -1.625
      // (0xbd) to 136 (0x71), and small companions.
      const WordSpec e = {WDT_E4M3FN,8,0x38,0xfd,0xbd,0x71,WVS_DEFAULT};
      const WordSpec f = {WDT_E4M3FN,8,0x38,0xfd,0xbd,0x71,WVS_NONE};
      const WordSpec u8 = {WDT_UNSIGNED,8,0x30,0x3f,0x31,0x3e,WVS_NONE};
      const WordSpec s8 = {WDT_SIGNED,8,0x0a,0xcb,0xc0,0x4b,WVS_NONE};
      const WordSpec c8 = {WDT_CUBE,8,0x10,0x93,0,0,WVS_EXACT};
      const WordSpec full8 = {WDT_CUBE,8,0,0xff,0,0,WVS_NONE};
      // FP32 around 1.0 with four unknown bits, including the sign, and a
      // non-negative finite result.
      const WordSpec g = {WDT_FP32,32,0x3f800000,0xbfc00003,0xbf800002,
                          0x3fc00001,WVS_NONE};
      const WordSpec u32 = {WDT_UNSIGNED,32,0x3f800000,0x3fc00003,0x3f800000,
                            0x3fc00002,WVS_NONE};
      const WordSpec h = {WDT_FP32,32,0,0xffffffff,0x00000000,0x7f7fffff,
                          WVS_NONE};
      const WordSpec full4 = {WDT_CUBE,4,0,15,0,0,WVS_NONE};
      return {
        {Constraint::EQ,{a,u}}, {Constraint::EQ,{b,s}},
        {Constraint::ULE,{a,s}}, {Constraint::ULE,{b,a}},
        {Constraint::SLE,{b,u}}, {Constraint::SLE,{a,c}},
        {Constraint::REIFIED_SLQ,{a,s}}, {Constraint::REIFIED_SLQ,{b,c}},
        {Constraint::ADD,{a,u,b}}, {Constraint::ADD,{b,c,s}},
        {Constraint::MULT,{a,s,full4}}, {Constraint::MULT,{b,b,a}},
        {Constraint::SIGNED_DIV,{a,s,b}}, {Constraint::SIGNED_DIV,{s,b,a}},
        {Constraint::DIVMOD,{a,u,b,full4}}, {Constraint::DIVMOD,{u,b,c,a}},
        {Constraint::EXTENSIONAL,{a,b}}, {Constraint::EXTENSIONAL,{b,s}},
        {Constraint::XOR,{a,c,s}}, {Constraint::XOR,{b,u,a}},
        {Constraint::EQ,{e,f}}, {Constraint::ULE,{e,u8}},
        {Constraint::SLE,{f,s8}}, {Constraint::REIFIED_SLQ,{e,c8}},
        {Constraint::ADD,{e,c8,full8}}, {Constraint::ADD,{f,u8,e}},
        {Constraint::MULT,{e,s8,full8}}, {Constraint::SIGNED_DIV,{f,s8,e}},
        {Constraint::EXTENSIONAL,{e,c8}}, {Constraint::XOR,{e,u8,f}},
        {Constraint::ULE,{g,u32}}, {Constraint::ADD,{g,u32,h}}
      };
    }
  } integer_constraints;

}}}

// STATISTICS: test-word
