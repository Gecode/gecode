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
#include "benchmarks/word/smt2-cardinality.hpp"
#include <gecode/word/branch.hh>
#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <vector>

namespace Test { namespace Word { namespace SMT2Cardinality {

  using namespace Gecode;
  using Value=std::uint64_t;
  using Count=WordSmt2::Cardinality;

  /// Compare a production count with an independently enumerated oracle.
  bool check(unsigned int width, Value lo, Value hi, Value lower,
             Value upper, bool sign, Count expected) {
    const Value sign_bit=sign ? Value(1) << (width-1) : 0;
    const Count actual=WordSmt2::cardinality(width,lo,hi,
      lower^sign_bit,upper^sign_bit,sign);
    if (actual == expected)
      return true;
    olog << "width=" << width << " lo=" << lo << " hi=" << hi
         << " ranked lower=" << lower << " ranked upper=" << upper
         << " signed=" << sign << " expected=" << expected.high << ':'
         << expected.low << " actual=" << actual.high << ':'
         << actual.low << '\n';
    return false;
  }

  /// Enumerate every cube and numeric interval at one small width.
  class Exhaustive : public Base {
  private:
    unsigned int width;
  public:
    Exhaustive(unsigned int width0)
      : Base("Word::SMT2::Cardinality::Exhaustive::"+
             std::to_string(width0)), width(width0) {}
    virtual bool run(void) {
      const Value size=Value(1) << width;
      Value cubes=1;
      for (unsigned int bit=0; bit<width; bit++)
        cubes*=3;
      for (Value code=0; code<cubes; code++) {
        Value lo=0, hi=0, remaining=code;
        for (unsigned int bit=0; bit<width; bit++, remaining/=3) {
          if (remaining%3 == 1) {
            hi|=Value(1) << bit;
          } else if (remaining%3 == 2) {
            lo|=Value(1) << bit;
            hi|=Value(1) << bit;
          }
        }
        for (bool sign : {false,true}) {
          const Value sign_bit=sign ? size/2 : 0;
          std::vector<Value> prefix(size+1);
          // Enumerate members directly, independently of the bit-prefix
          // counting algorithm under test. Include empty intersections.
          for (Value value=0; value<size; value++)
            if ((value&lo) == lo && (value&~hi) == 0)
              prefix[(value^sign_bit)+1]++;
          for (Value value=1; value<=size; value++)
            prefix[value]+=prefix[value-1];
          for (Value lower=0; lower<size; lower++)
            for (Value upper=lower; upper<size; upper++)
              if (!check(width,lo,hi,lower,upper,sign,
                         prefix[upper+1]-prefix[lower]))
                return false;
        }
      }
      return true;
    }
  };

  /// Enumerate sparse wide cubes against deterministic random intervals.
  class Sparse : public Base {
  private:
    unsigned int width;
  public:
    Sparse(unsigned int width0)
      : Base("Word::SMT2::Cardinality::Sparse::"+std::to_string(width0)),
        width(width0) {}
    virtual bool run(void) {
      std::mt19937_64 random(20260915);
      const Value mask=width == 64 ? ~Value(0) : (Value(1)<<width)-1;
      for (unsigned int trial=0; trial<1000; trial++) {
        Value unknown=0;
        for (unsigned int bit=0; bit<10; bit++)
          unknown|=Value(1) << (random()%width);
        const Value lo=random()&mask&~unknown;
        const Value hi=lo|unknown;
        Value lower=random()&mask, upper=random()&mask;
        if (lower > upper)
          std::swap(lower,upper);
        for (bool sign : {false,true}) {
          const Value sign_bit=sign ? Value(1) << (width-1) : 0;
          Count expected=0;
          Value subset=unknown;
          for (;;) {
            const Value rank=(lo|subset)^sign_bit;
            expected+=(lower <= rank && rank <= upper);
            if (subset == 0)
              break;
            subset=(subset-1)&unknown;
          }
          if (!check(width,lo,hi,lower,upper,sign,expected))
            return false;
        }
      }
      return true;
    }
  };

  /// Exercise exact 2^64 counts and subtraction of large prefix counts.
  class WideEdges : public Base {
  public:
    WideEdges(void) : Base("Word::SMT2::Cardinality::WideEdges") {}
    virtual bool run(void) {
      for (bool sign : {false,true}) {
        if (!check(64,0,~Value(0),0,~Value(0),sign,Count::power(64)) ||
            !check(64,0,~Value(0),1,~Value(0),sign,Count::power(64)-1) ||
            !check(64,0,~Value(0),Value(1)<<63,~Value(0),sign,
                   Count::power(63)) ||
            !check(64,0,~Value(0),~Value(0),~Value(0),sign,1) ||
            !check(64,0,~Value(2),~Value(3),~Value(0),sign,2))
          return false;
      }
      return true;
    }
  } wide_edges;

  /// Cause value failures so AFC updates can be observed across clones.
  class RejectZero : public UnaryPropagator<Gecode::Word::WordView,
                                            Gecode::Word::PC_WORD_VAL> {
  private:
    using Super=UnaryPropagator<Gecode::Word::WordView,
                               Gecode::Word::PC_WORD_VAL>;
  public:
    RejectZero(Home home, WordVar x) : Super(home,Gecode::Word::WordView(x)) {}
    RejectZero(Space& home, RejectZero& p) : Super(home,p) {}
    virtual Actor* copy(Space& home) {
      return new (home) RejectZero(home,*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      return x0.val() == 0 ? ES_FAILED : home.ES_SUBSUMED(*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::unary(PropCost::LO);
    }
  };

  class BranchSpace : public Space {
  public:
    WordVarArray x;
    BranchSpace(bool unequal_counts=false)
      : x(*this,2,unequal_counts ? 4 : 1,WDT_UNSIGNED) {
      if (unequal_counts) {
        rel(*this,x[0],WRT_ULQ,4,7);
        rel(*this,x[1],WRT_ULQ,4,5);
      }
      for (int i=0; i<2; i++)
        new (*this) RejectZero(*this,x[i]);
      branch(*this,x,WordSmt2::bounded_afc(*this,x),
             unequal_counts ? WORD_VAL_SPLIT_MIN() : WORD_VAL_LSB());
    }
    BranchSpace(BranchSpace& s) : Space(s) { x.update(*this,s.x); }
    virtual Space* copy(void) { return new BranchSpace(*this); }
  };

  /// Shared AFC state must remain valid after a failing clone is destroyed.
  class CloneFailure : public Base {
  public:
    CloneFailure(void) : Base("Word::SMT2::Cardinality::CloneFailure") {}
    virtual bool run(void) {
      BranchSpace root;
      if (root.status() != SS_BRANCH || root.afc_decay() != 1.0 ||
          root.x[0].afc() != 1.0 || root.x[1].afc() != 1.0) {
        olog << "Unexpected initial branch status or AFC counters\n";
        return false;
      }
      std::unique_ptr<BranchSpace> sibling(
        static_cast<BranchSpace*>(root.clone()));
      {
        std::unique_ptr<BranchSpace> failing(
          static_cast<BranchSpace*>(root.clone()));
        rel(*failing,failing->x[1],WRT_EQ,1,0);
        if (failing->status() != SS_FAILED) {
          olog << "Assigning zero did not fail\n";
          return false;
        }
      }
      if (root.x[1].afc() != 2.0 || sibling->x[1].afc() != 2.0 ||
          root.x[0].afc() != 1.0 || sibling->x[0].afc() != 1.0) {
        olog << "Failure AFC update did not survive across clones\n";
        return false;
      }
      if (sibling->status() != SS_BRANCH)
        return false;
      std::unique_ptr<const Choice> choice(sibling->choice());
      sibling->commit(*choice,0);
      if (!sibling->x[1].assigned() || sibling->x[1].val() != 0 ||
          sibling->x[0].assigned()) {
        olog << "Selector did not prefer the variable with greater AFC\n";
        return false;
      }
      if (sibling->status() != SS_FAILED || root.x[1].afc() != 3.0) {
        olog << "Committed failure did not update shared AFC\n";
        return false;
      }
      return true;
    }
  } clone_failure;

  /// Equal AFC and unknown-bit counts must prefer the smaller actual domain.
  class ActualCount : public Base {
  public:
    ActualCount(void) : Base("Word::SMT2::Cardinality::ActualCount") {}
    virtual bool run(void) {
      BranchSpace root(true);
      if (root.status() != SS_BRANCH ||
          root.x[0].unknown_size() != root.x[1].unknown_size() ||
          root.x[0].afc() != root.x[1].afc()) {
        olog << "Test domains do not have equal initial selector metrics\n";
        return false;
      }
      std::unique_ptr<BranchSpace> child(
        static_cast<BranchSpace*>(root.clone()));
      std::unique_ptr<const Choice> choice(child->choice());
      child->commit(*choice,0);
      if (child->x[0].maximum() != 7 || child->x[1].maximum() != 2) {
        olog << "Selector did not split the domain with six admitted values\n";
        return false;
      }
      return true;
    }
  } actual_count;

  class Create {
  public:
    Create(void) {
      for (unsigned int width=1; width<=6; width++)
        (void) new Exhaustive(width);
      for (unsigned int width : {8U,16U,32U,64U})
        (void) new Sparse(width);
    }
  } create;

}}}

// STATISTICS: test-word
