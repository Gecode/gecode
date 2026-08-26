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
 */

#include "test/word.hh"
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

namespace Test { namespace Word { namespace Overflow {

  static bool
  result(Gecode::WordOverflowType op, Gecode::WordValue x,
         Gecode::WordValue y, Gecode::WordValue mask) {
    const Gecode::WordValue sign = (mask >> 1) + 1;
    const bool x_negative = (x & sign) != 0;
    const bool y_negative = (y & sign) != 0;
    switch (op) {
    case Gecode::WOF_NEG_SIGNED: return x == sign;
    case Gecode::WOF_ADD_UNSIGNED: return x > mask-y;
    case Gecode::WOF_ADD_SIGNED:
      return (x_negative == y_negative) &&
        ((((x+y) & mask) & sign) != (x & sign));
    case Gecode::WOF_MULT_UNSIGNED: return y != 0 && x > mask/y;
    case Gecode::WOF_MULT_SIGNED: {
      const Gecode::WordValue x_magnitude = x_negative ? (-x & mask) : x;
      const Gecode::WordValue y_magnitude = y_negative ? (-y & mask) : y;
      const Gecode::WordValue limit = (x_negative != y_negative) ? sign : sign-1;
      return x_magnitude != 0 && y_magnitude > limit/x_magnitude;
    }
    case Gecode::WOF_DIV_SIGNED: return x == sign && y == mask;
    default: GECODE_NEVER;
    }
    return false;
  }

  class Unary : public Test {
  private:
    int expected;
  public:
    Unary(int expected0) : Test("Overflow::Neg::"+str(expected0),1,
                               Domain(3,0,7)), expected(expected0) {}
    virtual bool solution(const Assignment& a) const {
      return result(Gecode::WOF_NEG_SIGNED,a[0],0,dom.mask()) == expected;
    }
    virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
      Gecode::BoolVar b(home,expected,expected);
      Gecode::overflow(home,x[0],Gecode::WOF_NEG_SIGNED,b);
    }
  };

  class Binary : public Test {
  private:
    Gecode::WordOverflowType op;
    int expected;
  public:
    Binary(Gecode::WordOverflowType op0, int expected0,
           const std::string& name)
      : Test("Overflow::"+name+"::"+str(expected0),2,Domain(3,0,7)),
        op(op0), expected(expected0) {}
    virtual bool solution(const Assignment& a) const {
      return result(op,a[0],a[1],dom.mask()) == expected;
    }
    virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
      Gecode::BoolVar b(home,expected,expected);
      Gecode::overflow(home,x[0],op,x[1],b);
    }
  };

  enum Flag { CARRY, BORROW };
  class Flagged : public Test {
  private:
    Flag flag;
    int expected;
  public:
    Flagged(Flag flag0, int expected0, const std::string& name)
      : Test("Overflow::"+name+"::"+str(expected0),3,Domain(3,0,7)),
        flag(flag0), expected(expected0) {}
    virtual bool solution(const Assignment& a) const {
      const bool f = flag == CARRY ? a[0] > dom.mask()-a[1] : a[0] < a[1];
      const Gecode::WordValue z = flag == CARRY ?
        (a[0]+a[1]) & dom.mask() : (a[0]-a[1]) & dom.mask();
      return (f == expected) && (a[2] == z);
    }
    virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
      Gecode::BoolVar b(home,expected,expected);
      if (flag == CARRY)
        Gecode::add(home,x[0],x[1],x[2],b);
      else
        Gecode::sub(home,x[0],x[1],x[2],b);
    }
  };

  class Lifecycle : public Base {
  private:
    class S : public Gecode::Space {
    public:
      Gecode::WordVar x, y, z;
      Gecode::BoolVar b;
      S(unsigned int width=4) : x(*this,width), y(*this,width), z(*this,width),
                                b(*this,0,1) {}
      S(S& s) : Gecode::Space(s) {
        x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
        b.update(*this,s.b);
      }
      virtual Gecode::Space* copy(void) { return new S(*this); }
    };

    static bool backward_and_clone(void) {
      S source;
      Gecode::add(source,source.x,source.y,source.z,source.b);
      if (source.status() == Gecode::SS_FAILED)
        return false;
      S* copy = static_cast<S*>(source.clone());
      Gecode::dom(*copy,copy->x,15U);
      Gecode::dom(*copy,copy->y,1U);
      Gecode::rel(*copy,copy->b,Gecode::IRT_EQ,1);
      const bool ok = copy->status() != Gecode::SS_FAILED &&
        copy->z.assigned() && copy->z.val() == 0 && !source.z.assigned();
      delete copy;
      if (!ok)
        return false;

      S borrow;
      Gecode::sub(borrow,borrow.x,borrow.y,borrow.z,borrow.b);
      Gecode::rel(borrow,borrow.b,Gecode::IRT_EQ,1);
      Gecode::dom(borrow,borrow.x,0U);
      Gecode::dom(borrow,borrow.z,15U);
      return borrow.status() != Gecode::SS_FAILED && borrow.y.assigned() &&
        borrow.y.val() == 1U;
    }

    static bool seams_alias_failure(void) {
      S wide(64);
      Gecode::dom(wide,wide.x,~Gecode::WordValue(0));
      Gecode::add(wide,wide.x,wide.x,wide.z,wide.b);
      if (wide.status() == Gecode::SS_FAILED || !wide.z.assigned() ||
          wide.z.val() != ~Gecode::WordValue(1) || !wide.b.one())
        return false;
      S wide_overflow(64);
      Gecode::dom(wide_overflow,wide_overflow.x,
                  Gecode::WordValue(1) << 63);
      Gecode::dom(wide_overflow,wide_overflow.y,
                  ~Gecode::WordValue(0));
      Gecode::overflow(wide_overflow,wide_overflow.x,
                       Gecode::WOF_MULT_SIGNED,wide_overflow.y,
                       wide_overflow.b);
      const bool expected = result(Gecode::WOF_MULT_SIGNED,
        Gecode::WordValue(1) << 63,~Gecode::WordValue(0),
        ~Gecode::WordValue(0));
      if (wide_overflow.status() == Gecode::SS_FAILED ||
          !wide_overflow.b.assigned() ||
          (wide_overflow.b.val() != static_cast<int>(expected)))
        return false;
      S one(1);
      Gecode::overflow(one,one.x,Gecode::WOF_DIV_SIGNED,one.y,one.b);
      Gecode::dom(one,one.x,1U); Gecode::dom(one,one.y,1U);
      if (one.status() == Gecode::SS_FAILED || !one.b.one())
        return false;
      S failed;
      Gecode::dom(failed,failed.x,15U); Gecode::dom(failed,failed.y,1U);
      Gecode::dom(failed,failed.z,0U); Gecode::rel(failed,failed.b,Gecode::IRT_EQ,0);
      Gecode::add(failed,failed.x,failed.y,failed.z,failed.b);
      return failed.status() == Gecode::SS_FAILED;
    }

    static bool recomputation(void) {
      using namespace Gecode;
      class SearchSpace : public Space {
      public:
        WordVar x, y, sum, difference;
        BoolVar carry, borrow;
        BoolVarArray flags;
        SearchSpace(void)
          : x(*this,2), y(*this,2), sum(*this,2), difference(*this,2),
            carry(*this,0,1), borrow(*this,0,1), flags(*this,6,0,1) {
          add(*this,x,y,sum,carry);
          sub(*this,x,y,difference,borrow);
          overflow(*this,x,WOF_NEG_SIGNED,flags[0]);
          overflow(*this,x,WOF_ADD_UNSIGNED,y,flags[1]);
          overflow(*this,x,WOF_ADD_SIGNED,y,flags[2]);
          overflow(*this,x,WOF_MULT_UNSIGNED,y,flags[3]);
          overflow(*this,x,WOF_MULT_SIGNED,y,flags[4]);
          overflow(*this,x,WOF_DIV_SIGNED,y,flags[5]);
          WordVarArgs decision = {x,y};
          branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
        }
        SearchSpace(SearchSpace& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y); sum.update(*this,s.sum);
          difference.update(*this,s.difference);
          carry.update(*this,s.carry); borrow.update(*this,s.borrow);
          flags.update(*this,s.flags);
        }
        virtual Space* copy(void) { return new SearchSpace(*this); }
      };
      SearchSpace* root = new SearchSpace;
      Search::Options options;
      options.c_d = 1;
      DFS<SearchSpace> dfs(root,options);
      delete root;
      unsigned int solutions = 0;
      while (SearchSpace* solution = dfs.next()) {
        bool ok = solution->x.assigned() && solution->y.assigned() &&
          solution->sum.assigned() && solution->difference.assigned() &&
          solution->carry.assigned() && solution->borrow.assigned();
        const WordOverflowType operations[] = {
          WOF_NEG_SIGNED, WOF_ADD_UNSIGNED, WOF_ADD_SIGNED,
          WOF_MULT_UNSIGNED, WOF_MULT_SIGNED, WOF_DIV_SIGNED
        };
        for (int i=0; ok && i<6; i++)
          ok = solution->flags[i].assigned() &&
            (solution->flags[i].val() == static_cast<int>(Overflow::result(
              operations[i],solution->x.val(),solution->y.val(),3U)));
        ok = ok && solution->carry.val() ==
          static_cast<int>(solution->x.val() > 3U-solution->y.val()) &&
          solution->borrow.val() ==
          static_cast<int>(solution->x.val() < solution->y.val());
        delete solution;
        if (!ok)
          return false;
        solutions++;
      }
      return solutions == 16;
    }

    static bool bounded(void) {
      using namespace Gecode;
      static_assert(!Gecode::Word::WordView::supports_bounds,
                    "compact Word views must remain cube-only");
      static_assert(Gecode::Word::UnsignedWordView::supports_bounds &&
                    !Gecode::Word::UnsignedWordView::signed_order,
                    "unsigned bounded views must expose unsigned ranks");
      static_assert(Gecode::Word::SignedWordView::supports_bounds &&
                    Gecode::Word::SignedWordView::signed_order,
                    "signed bounded views must expose signed ranks");
      class B : public Space {
      public:
        WordVar x,y,sum,difference;
        BoolVar carry,borrow,overflow_flag;
        B(void)
          : x(*this,4,WDT_UNSIGNED,2U,4U),
            y(*this,4,WDT_UNSIGNED,3U,5U),
            sum(*this,4,WDT_UNSIGNED),
            difference(*this,4,WDT_UNSIGNED),
            carry(*this,0,1), borrow(*this,0,1), overflow_flag(*this,0,1) {
          add(*this,x,y,sum,carry);
          sub(*this,y,x,difference,borrow);
          overflow(*this,x,WOF_ADD_UNSIGNED,y,overflow_flag);
        }
        B(B& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y);
          sum.update(*this,s.sum); difference.update(*this,s.difference);
          carry.update(*this,s.carry); borrow.update(*this,s.borrow);
          overflow_flag.update(*this,s.overflow_flag);
        }
        Space* copy(void) { return new B(*this); }
      };
      B source;
      if ((source.status() == SS_FAILED) || !source.carry.zero() ||
          !source.overflow_flag.zero() || (source.sum.minimum() != 5U) ||
          (source.sum.maximum() != 9U))
        return false;
      B* clone=static_cast<B*>(source.clone());
      dom(*clone,clone->x,4U); dom(*clone,clone->y,3U);
      const bool clone_ok=(clone->status() != SS_FAILED) &&
        clone->sum.assigned() && (clone->sum.val() == 7U) &&
        clone->difference.assigned() && (clone->difference.val() == 15U) &&
        clone->borrow.one() && !source.sum.assigned();
      delete clone;
      if (!clone_ok) return false;

      class Wrap : public Space {
      public:
        WordVar x,y,z; BoolVar carry;
        Wrap(void) : x(*this,4,WDT_UNSIGNED,15U,15U),
          y(*this,4,WDT_UNSIGNED,1U,2U), z(*this,4,WDT_UNSIGNED),
          carry(*this,0,1) { add(*this,x,y,z,carry); }
        Wrap(Wrap& s) : Space(s) {
          x.update(*this,s.x);y.update(*this,s.y);z.update(*this,s.z);
          carry.update(*this,s.carry);
        }
        Space* copy(void) { return new Wrap(*this); }
      };
      Wrap wrap;
      if ((wrap.status() == SS_FAILED) || !wrap.carry.one())
        return false;

      // Assigned terminal flags use the statically specialized bounded
      // arithmetic actor, without retaining a Bool subscription.
      class FixedTerminal : public Space {
      public:
        WordVar x,y,sum,difference;
        BoolVar carry,borrow;
        FixedTerminal(void)
          : x(*this,4,WDT_UNSIGNED,0U,15U),
            y(*this,4,WDT_UNSIGNED,1U,2U),
            sum(*this,4,WDT_UNSIGNED),
            difference(*this,4,WDT_UNSIGNED),
            carry(*this,0,0), borrow(*this,0,0) {
          add(*this,x,y,sum,carry);
          sub(*this,x,y,difference,borrow);
        }
        FixedTerminal(FixedTerminal& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y);
          sum.update(*this,s.sum); difference.update(*this,s.difference);
          carry.update(*this,s.carry); borrow.update(*this,s.borrow);
        }
        Space* copy(void) { return new FixedTerminal(*this); }
      };
      FixedTerminal fixed;
      if ((fixed.status() == SS_FAILED) || (fixed.x.minimum() != 1U) ||
          (fixed.x.maximum() != 14U) || (fixed.sum.minimum() != 2U) ||
          (fixed.sum.maximum() != 15U) ||
          (fixed.difference.minimum() != 0U) ||
          (fixed.difference.maximum() != 13U) ||
          (PropagatorGroup::all.size(fixed) != 2U))
        return false;
      FixedTerminal* fixed_clone=static_cast<FixedTerminal*>(fixed.clone());
      dom(*fixed_clone,fixed_clone->x,10U);
      dom(*fixed_clone,fixed_clone->y,2U);
      const bool fixed_clone_ok=
        (fixed_clone->status() != SS_FAILED) &&
        fixed_clone->sum.assigned() && (fixed_clone->sum.val() == 12U) &&
        fixed_clone->difference.assigned() &&
        (fixed_clone->difference.val() == 8U) &&
        (PropagatorGroup::all.size(*fixed_clone) == 0U);
      delete fixed_clone;
      if (!fixed_clone_ok) return false;

      class WideTerminal : public Space {
      public:
        WordVar x,y,z; BoolVar carry;
        WideTerminal(int terminal)
          : x(*this,64,WDT_UNSIGNED,~WordValue(2),~WordValue(0)),
            y(*this,64,WDT_UNSIGNED,1U,2U),
            z(*this,64,WDT_UNSIGNED), carry(*this,terminal,terminal) {
          add(*this,x,y,z,carry);
        }
        WideTerminal(WideTerminal& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          carry.update(*this,s.carry);
        }
        Space* copy(void) { return new WideTerminal(*this); }
      };
      WideTerminal wide_clear(0), wide_set(1);
      if ((wide_clear.status() == SS_FAILED) ||
          (wide_clear.x.maximum() != ~WordValue(0)-1U) ||
          (wide_clear.z.minimum() != ~WordValue(0)-1U) ||
          (wide_clear.z.maximum() != ~WordValue(0)) ||
          (wide_set.status() == SS_FAILED) ||
          (wide_set.x.minimum() != ~WordValue(0)-1U) ||
          (wide_set.z.minimum() != 0U) ||
          (wide_set.z.maximum() != 1U))
        return false;

      // An initially unknown flag that becomes decided rewrites to the same
      // static actor and remains sound across cloning and recomputation.
      class Rewrite : public Space {
      public:
        WordVar x,y,z; BoolVar carry;
        Rewrite(void) : x(*this,3,WDT_UNSIGNED,0U,3U),
          y(*this,3,WDT_UNSIGNED,0U,3U), z(*this,3,WDT_UNSIGNED),
          carry(*this,0,1) {
          add(*this,x,y,z,carry);
          WordVarArgs a={x,y};
          branch(*this,a,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
        }
        Rewrite(Rewrite& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          carry.update(*this,s.carry);
        }
        Space* copy(void) { return new Rewrite(*this); }
      };
      Rewrite* rewrite_root=new Rewrite;
      if ((rewrite_root->status() == SS_FAILED) ||
          !rewrite_root->carry.zero() ||
          (PropagatorGroup::all.size(*rewrite_root) != 1U)) {
        delete rewrite_root;
        return false;
      }
      Search::Options rewrite_options; rewrite_options.c_d=1;
      DFS<Rewrite> rewrite_dfs(rewrite_root,rewrite_options);
      delete rewrite_root;
      unsigned int rewrite_solutions=0;
      while (Rewrite* solution=rewrite_dfs.next()) {
        const bool ok=solution->z.assigned() && solution->carry.zero() &&
          (solution->z.val() == solution->x.val()+solution->y.val()) &&
          (PropagatorGroup::all.size(*solution) == 0U);
        delete solution;
        if (!ok) return false;
        rewrite_solutions++;
      }
      if (rewrite_solutions != 16U) return false;

      class StaticSearch : public Space {
      public:
        WordVar x,y,z; BoolVar flag;
        bool addition;
        StaticSearch(bool addition0, int flag0)
          : x(*this,3,WDT_UNSIGNED), y(*this,3,WDT_UNSIGNED),
            z(*this,3,WDT_UNSIGNED), flag(*this,flag0,flag0),
            addition(addition0) {
          if (addition)
            add(*this,x,y,z,flag);
          else
            sub(*this,x,y,z,flag);
          WordVarArgs a={x,y};
          branch(*this,a,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
        }
        StaticSearch(StaticSearch& s) : Space(s), addition(s.addition) {
          x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          flag.update(*this,s.flag);
        }
        Space* copy(void) { return new StaticSearch(*this); }
      };
      for (int operation=0; operation<2; operation++)
        for (int expected=0; expected<2; expected++) {
          StaticSearch* root=new StaticSearch(operation == 0,expected);
          Search::Options options; options.c_d=1;
          DFS<StaticSearch> dfs(root,options); delete root;
          unsigned int solutions=0;
          while (StaticSearch* solution=dfs.next()) {
            const WordValue xv=solution->x.val(), yv=solution->y.val();
            const WordValue result=(operation == 0) ?
              ((xv+yv)&7U) : ((xv-yv)&7U);
            const int terminal=(operation == 0) ?
              static_cast<int>(xv > 7U-yv) :
              static_cast<int>(xv < yv);
            const bool ok=solution->z.assigned() &&
              (solution->z.val() == result) &&
              (terminal == expected) &&
              (PropagatorGroup::all.size(*solution) == 0U);
            delete solution;
            if (!ok) return false;
            solutions++;
          }
          if (solutions != static_cast<unsigned int>(expected ? 28 : 36))
            return false;
        }

      class Replay : public Space {
      public:
        WordVar x,y,z; BoolVar carry;
        Replay(void) : x(*this,2,WDT_UNSIGNED), y(*this,2,WDT_UNSIGNED),
          z(*this,2,WDT_UNSIGNED), carry(*this,0,1) {
          add(*this,x,y,z,carry);
          WordVarArgs a={x,y}; branch(*this,a,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
        }
        Replay(Replay& s) : Space(s) {
          x.update(*this,s.x);y.update(*this,s.y);z.update(*this,s.z);
          carry.update(*this,s.carry);
        }
        Space* copy(void) { return new Replay(*this); }
      };
      Replay* root=new Replay;
      Search::Options o; o.c_d=1;
      DFS<Replay> dfs(root,o); delete root;
      unsigned int solutions=0;
      while (Replay* s=dfs.next()) {
        const WordValue total=s->x.val()+s->y.val();
        const bool ok=s->z.assigned() && s->carry.assigned() &&
          (s->z.val() == (total&3U)) &&
          (s->carry.val() == static_cast<int>(total>3U));
        delete s;
        if (!ok) return false;
        solutions++;
      }
      return solutions == 16U;
    }

    static bool minimodel(void) {
      S s;
      Gecode::BoolExpr e = Gecode::overflow(Gecode::WordExpr(s.x),
        Gecode::WOF_ADD_SIGNED,Gecode::WordExpr(s.y));
      Gecode::rel(s,e);
      Gecode::dom(s,s.x,7U); Gecode::dom(s,s.y,1U);
      return s.status() != Gecode::SS_FAILED;
    }
  public:
    Lifecycle(void) : Base("Word::Overflow::Lifecycle") {}
    virtual bool run(void) {
      return backward_and_clone() && seams_alias_failure() && minimodel() &&
        recomputation() && bounded();
    }
  };

  Unary neg0(0), neg1(1);
  Binary ua0(Gecode::WOF_ADD_UNSIGNED,0,"UnsignedAdd");
  Binary ua1(Gecode::WOF_ADD_UNSIGNED,1,"UnsignedAdd");
  Binary sa0(Gecode::WOF_ADD_SIGNED,0,"SignedAdd");
  Binary sa1(Gecode::WOF_ADD_SIGNED,1,"SignedAdd");
  Binary um0(Gecode::WOF_MULT_UNSIGNED,0,"UnsignedMult");
  Binary um1(Gecode::WOF_MULT_UNSIGNED,1,"UnsignedMult");
  Binary sm0(Gecode::WOF_MULT_SIGNED,0,"SignedMult");
  Binary sm1(Gecode::WOF_MULT_SIGNED,1,"SignedMult");
  Binary sd0(Gecode::WOF_DIV_SIGNED,0,"SignedDiv");
  Binary sd1(Gecode::WOF_DIV_SIGNED,1,"SignedDiv");
  Flagged carry0(CARRY,0,"Carry"), carry1(CARRY,1,"Carry");
  Flagged borrow0(BORROW,0,"Borrow"), borrow1(BORROW,1,"Borrow");
  Lifecycle lifecycle;

}}}

// STATISTICS: test-word
