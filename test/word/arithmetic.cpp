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

#include <gecode/search.hh>
#include <gecode/word/arithmetic.hh>

namespace Test { namespace Word {

  namespace Arithmetic {

    enum Op { ADD, NEG, SUB, MULT, DIV, MOD, SIGNED_DIV, SIGNED_REM,
              SIGNED_MOD };

    static int
    signed_value(Gecode::WordValue value, Gecode::WordValue mask) {
      const Gecode::WordValue sign = (mask >> 1) + 1;
      return (value & sign) ?
        static_cast<int>(value) - static_cast<int>(mask) - 1 :
        static_cast<int>(value);
    }

    static Gecode::WordValue
    evaluate(Op op, Gecode::WordValue x, Gecode::WordValue y,
             Gecode::WordValue mask) {
      switch (op) {
      case ADD: return (x + y) & mask;
      case NEG: return (Gecode::WordValue(0) - x) & mask;
      case SUB: return (x - y) & mask;
      case MULT: return (x * y) & mask;
      case DIV: return (y == 0) ? mask : x / y;
      case MOD: return (y == 0) ? x : x % y;
      case SIGNED_DIV: {
        const int sx = signed_value(x,mask);
        const int sy = signed_value(y,mask);
        if (sy == 0)
          return sx < 0 ? 1U : mask;
        const int minimum = -static_cast<int>((mask >> 1) + 1);
        if ((sx == minimum) && (sy == -1))
          return x;
        return static_cast<Gecode::WordValue>(sx / sy) & mask;
      }
      case SIGNED_REM: {
        const int sx = signed_value(x,mask);
        const int sy = signed_value(y,mask);
        if (sy == 0)
          return x;
        const int minimum = -static_cast<int>((mask >> 1) + 1);
        if ((sx == minimum) && (sy == -1))
          return 0;
        return static_cast<Gecode::WordValue>(sx % sy) & mask;
      }
      case SIGNED_MOD: {
        const int sx = signed_value(x,mask);
        const int sy = signed_value(y,mask);
        if (sy == 0)
          return x;
        const int minimum = -static_cast<int>((mask >> 1) + 1);
        int r = ((sx == minimum) && (sy == -1)) ? 0 : sx % sy;
        if ((r != 0) && ((r < 0) != (sy < 0)))
          r += sy;
        return static_cast<Gecode::WordValue>(r) & mask;
      }
      default: GECODE_NEVER;
      }
      return 0;
    }

    static void
    post(Op op, Gecode::Home home, Gecode::WordVar x,
         Gecode::WordVar y, Gecode::WordVar result) {
      switch (op) {
      case ADD: Gecode::add(home,x,y,result); break;
      case NEG: Gecode::neg(home,x,result); break;
      case SUB: Gecode::sub(home,x,y,result); break;
      case MULT: Gecode::mult(home,x,y,result); break;
      case DIV: Gecode::div(home,x,y,result); break;
      case MOD: Gecode::mod(home,x,y,result); break;
      case SIGNED_DIV: Gecode::signed_div(home,x,y,result); break;
      case SIGNED_REM: Gecode::signed_rem(home,x,y,result); break;
      case SIGNED_MOD: Gecode::signed_mod(home,x,y,result); break;
      default: GECODE_NEVER;
      }
    }

    /// Assigned-value oracle for modular binary arithmetic
    class Binary : public Test {
    private:
      Op op;
    public:
      Binary(Op op0, const std::string& name)
        : Test("Arithmetic::"+name,3,Domain(3,0,7)), op(op0) {}
      virtual bool solution(const Assignment& a) const {
        return a[2] == evaluate(op,a[0],a[1],dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Arithmetic::post(op,home,x[0],x[1],x[2]);
      }
    };

    /// Assigned-value oracle for two's-complement modular negation
    class Negation : public Test {
    public:
      Negation(void) : Test("Arithmetic::Neg",2,Domain(3,0,7)) {}
      virtual bool solution(const Assignment& a) const {
        return a[1] == evaluate(NEG,a[0],0,dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::neg(home,x[0],x[1]);
      }
    };

    /// Assigned-value oracle for native n-ary modular addition
    class NaryAddition : public Test {
    public:
      NaryAddition(void) : Test("Arithmetic::NaryAdd",4,Domain(3,0,7)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == ((a[0]+a[1]+a[2]) & dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::WordVarArgs args = {x[0],x[1],x[2]};
        Gecode::add(home,args,x[3]);
      }
    };

    class Lifecycle : public Base {
    private:
      class ArithmeticSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        ArithmeticSpace(int n=3, unsigned int width=4)
          : x(*this,n,width,0,Gecode::Word::width_mask(width)) {}
        ArithmeticSpace(ArithmeticSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new ArithmeticSpace(*this);
        }
      };

      static bool partial(Op op) {
        const int arity = (op == NEG) ? 2 : 3;
        for (PartialAssignment p(arity,1); p.has_more(); p.next()) {
          TestSpace s(arity,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<arity; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          if (op == NEG)
            Gecode::neg(s,s.x[0],s.x[1]);
          else
            Arithmetic::post(op,s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();
          bool supported = false;
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y) {
              if (op == NEG) {
                if (y.val() == evaluate(NEG,x.val(),0,1U)) {
                  supported = true;
                  if (failed || !s.x[0].in(x.val()) ||
                      !s.x[1].in(y.val()))
                    return false;
                }
              } else {
                for (Values z(p[2]); z(); ++z)
                  if (z.val() == evaluate(op,x.val(),y.val(),1U)) {
                    supported = true;
                    if (failed || !s.x[0].in(x.val()) ||
                        !s.x[1].in(y.val()) || !s.x[2].in(z.val()))
                      return false;
                  }
              }
            }
          if (failed == supported)
            return false;
        }
        return true;
      }

      static bool nary_add_partial(void) {
        for (PartialAssignment p(4,1); p.has_more(); p.next()) {
          TestSpace s(4,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<4; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::WordVarArgs args = {s.x[0],s.x[1],s.x[2]};
          Gecode::add(s,args,s.x[3]);
          const bool failed=s.failed();
          bool supported=false;
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y)
              for (Values z(p[2]); z(); ++z) {
                const Gecode::WordValue result=
                  (x.val()+y.val()+z.val())&1U;
                if (!p[3].in(result))
                  continue;
                supported=true;
                const Gecode::WordValue tuple[4] = {
                  x.val(),y.val(),z.val(),result
                };
                for (int i=0; i<4; i++)
                  if (failed || !s.x[i].in(tuple[i]))
                    return false;
              }
          if (failed == supported)
            return false;
        }
        return true;
      }

      class PostingSpace : public Gecode::Space {
      public:
        PostingSpace(void) {}
        PostingSpace(PostingSpace& s) : Gecode::Space(s) {}
        virtual Gecode::Space* copy(void) {
          return new PostingSpace(*this);
        }
      };

      static bool fallback_publication(void) {
        using namespace Gecode;
        {
          PostingSpace s;
          WordVar a(s,4,4,14,WDT_SIGNED,12,4);
          WordVar b(s,4,WDT_SIGNED,10,10);
          WordVar r(s,4,0,10,WDT_SIGNED,10,2);
          add(s,a,b,r);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,0,7,WDT_UNSIGNED,3,7);
          WordVar b(s,4,WDT_UNSIGNED,5,5);
          WordVar r(s,4,3,15,WDT_UNSIGNED,7,11);
          sub(s,a,b,r);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,10,15,WDT_SIGNED,10,14);
          WordVar b(s,4,9,15,WDT_SIGNED,11,15);
          WordVar r(s,4,WDT_SIGNED,11,11);
          mult(s,a,b,r);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,12,15,WDT_UNSIGNED,13,14);
          WordVar b(s,4,WDT_UNSIGNED,1,1);
          WordVar r(s,4,0,13,WDT_UNSIGNED,5,12);
          signed_div(s,a,b,r);
          if (s.status() != SS_FAILED)
            return false;
        }
        // Nearby exact tuples exercise the same mixed-domain fallback paths.
        for (Op op : {ADD,SUB,MULT,SIGNED_DIV}) {
          PostingSpace s;
          const WordValue av = (op == ADD) ? 14U :
            (op == SUB) ? 4U : (op == MULT) ? 11U : 13U;
          const WordValue bv = (op == ADD) ? 10U :
            (op == SUB) ? 5U : (op == MULT) ? 13U : 1U;
          const WordValue rv=evaluate(op,av,bv,15U);
          WordVar a(s,4,WDT_SIGNED,av,av);
          WordVar b(s,4,bv,bv);
          WordVar r(s,4,WDT_UNSIGNED,rv,rv);
          Arithmetic::post(op,s,a,b,r);
          if (s.status() == SS_FAILED)
            return false;
        }
        return true;
      }

      static bool fallback_scheduled_publication(void) {
        using namespace Gecode;
        {
          PostingSpace s;
          WordVar a(s,4,4,14,WDT_SIGNED,12,4);
          WordVar r(s,4,0,15,WDT_UNSIGNED,0,15);
          neg(s,a,r);
          dom(s,a,3U); dom(s,r,12U);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,4,14,WDT_SIGNED,12,4), b(s,4,10,10);
          WordVar r(s,4,WDT_UNSIGNED,0,15);
          BoolVar carry(s,0,0);
          add(s,a,b,r,carry);
          dom(s,a,14U); dom(s,r,8U);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,WDT_UNSIGNED,0,7), b(s,4,5,5);
          WordVar r(s,4,0,15,WDT_SIGNED,8,7);
          BoolVar borrow(s,0,0);
          sub(s,a,b,r,borrow);
          dom(s,a,4U); dom(s,r,15U);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,4,14,WDT_SIGNED,12,4), b(s,4,10,10);
          WordVar c(s,4,WDT_UNSIGNED,1,1), r(s,4,0,15);
          WordVarArgs input = {a,b,c};
          add(s,input,r);
          dom(s,a,14U); dom(s,r,8U);
          if (s.status() != SS_FAILED)
            return false;
        }
        {
          PostingSpace s;
          WordVar a(s,4,4,14,WDT_SIGNED,12,4), b(s,4,0,15);
          add(s,a,b,a);
          dom(s,b,1U);
          if (s.status() != SS_FAILED)
            return false;
        }
        return true;
      }

      /** Exhaustive width-two support hull for the native carry actor. */
      static bool add_bit_consistency(void) {
        const unsigned int width = 2;
        const Gecode::WordValue mask = 3U;
        for (PartialAssignment p(3,width); p.has_more(); p.next()) {
          TestSpace s(3,Domain(width,0,mask));
          std::vector<Domain> domains;
          for (int i=0; i<3; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::add(s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();

          bool supported = false;
          Gecode::WordValue support_lo[3] = {mask,mask,mask};
          Gecode::WordValue support_hi[3] = {0,0,0};
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y) {
              const Gecode::WordValue z = (x.val()+y.val()) & mask;
              if (!p[2].in(z))
                continue;
              supported = true;
              const Gecode::WordValue tuple[3] = {x.val(),y.val(),z};
              for (int i=0; i<3; i++) {
                support_lo[i] &= tuple[i];
                support_hi[i] |= tuple[i];
              }
            }
          if (!supported) {
            if (!failed)
              return false;
          } else {
            if (failed)
              return false;
            for (int i=0; i<3; i++)
              if ((s.x[i].lo() != support_lo[i]) ||
                  (s.x[i].hi() != support_hi[i]))
                return false;
          }
        }
        return true;
      }

      /** Exhaustive width-two support hull for native negation. */
      static bool neg_bit_consistency(void) {
        const unsigned int width = 2;
        const Gecode::WordValue mask = 3U;
        for (PartialAssignment p(2,width); p.has_more(); p.next()) {
          TestSpace s(2,Domain(width,0,mask));
          std::vector<Domain> domains;
          domains.push_back(p[0]);
          domains.push_back(p[1]);
          s.narrow(domains);
          Gecode::neg(s,s.x[0],s.x[1]);
          const bool failed = s.failed();

          bool supported = false;
          Gecode::WordValue support_lo[2] = {mask,mask};
          Gecode::WordValue support_hi[2] = {0,0};
          for (Values x(p[0]); x(); ++x) {
            const Gecode::WordValue z =
              (Gecode::WordValue(0)-x.val()) & mask;
            if (!p[1].in(z))
              continue;
            supported = true;
            const Gecode::WordValue tuple[2] = {x.val(),z};
            for (int i=0; i<2; i++) {
              support_lo[i] &= tuple[i];
              support_hi[i] |= tuple[i];
            }
          }
          if (!supported) {
            if (!failed)
              return false;
          } else {
            if (failed)
              return false;
            for (int i=0; i<2; i++)
              if ((s.x[i].lo() != support_lo[i]) ||
                  (s.x[i].hi() != support_hi[i]))
                return false;
          }
        }
        return true;
      }

      /** Exhaustive width-two support hull for native subtraction. */
      static bool sub_bit_consistency(void) {
        const unsigned int width = 2;
        const Gecode::WordValue mask = 3U;
        for (PartialAssignment p(3,width); p.has_more(); p.next()) {
          TestSpace s(3,Domain(width,0,mask));
          std::vector<Domain> domains;
          for (int i=0; i<3; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::sub(s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();

          bool supported = false;
          Gecode::WordValue support_lo[3] = {mask,mask,mask};
          Gecode::WordValue support_hi[3] = {0,0,0};
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y) {
              const Gecode::WordValue z = (x.val()-y.val()) & mask;
              if (!p[2].in(z))
                continue;
              supported = true;
              const Gecode::WordValue tuple[3] = {x.val(),y.val(),z};
              for (int i=0; i<3; i++) {
                support_lo[i] &= tuple[i];
                support_hi[i] |= tuple[i];
              }
            }
          if (!supported) {
            if (!failed)
              return false;
          } else {
            if (failed)
              return false;
            for (int i=0; i<3; i++)
              if ((s.x[i].lo() != support_lo[i]) ||
                  (s.x[i].hi() != support_hi[i]))
                return false;
          }
        }
        return true;
      }

      /** A tiny ordinary Boolean full-adder decomposition for comparison. */
      class DifferentialSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar native_result;
        Gecode::WordVar boolean_result;
        DifferentialSpace(unsigned int width)
          : x(*this,width), y(*this,width), native_result(*this,width),
            boolean_result(*this,width) {
          Gecode::add(*this,x,y,native_result);
          Gecode::BoolVarArray carry(*this,width+1,0,1);
          Gecode::rel(*this,carry[0],Gecode::IRT_EQ,0);
          for (unsigned int bit=0; bit<width; bit++) {
            Gecode::BoolVar x_bit(*this,0,1), y_bit(*this,0,1);
            Gecode::BoolVar result_bit(*this,0,1);
            Gecode::channel(*this,x,bit,x_bit);
            Gecode::channel(*this,y,bit,y_bit);
            Gecode::channel(*this,boolean_result,bit,result_bit);

            Gecode::BoolVar xor_xy(*this,0,1);
            Gecode::rel(*this,x_bit,Gecode::BOT_XOR,y_bit,xor_xy);
            Gecode::rel(*this,xor_xy,Gecode::BOT_XOR,
                        carry[bit],result_bit);
            Gecode::BoolVar both_one(*this,0,1), carry_one(*this,0,1);
            Gecode::rel(*this,x_bit,Gecode::BOT_AND,y_bit,both_one);
            Gecode::rel(*this,carry[bit],Gecode::BOT_AND,
                        xor_xy,carry_one);
            Gecode::rel(*this,both_one,Gecode::BOT_OR,
                        carry_one,carry[bit+1]);
          }
        }
        DifferentialSpace(DifferentialSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          native_result.update(*this,s.native_result);
          boolean_result.update(*this,s.boolean_result);
        }
        virtual Gecode::Space* copy(void) {
          return new DifferentialSpace(*this);
        }
      };

      static bool boolean_parity(void) {
        for (Gecode::WordValue x=0; x<8; x++)
          for (Gecode::WordValue y=0; y<8; y++) {
            DifferentialSpace s(3);
            Gecode::dom(s,s.x,x);
            Gecode::dom(s,s.y,y);
            if ((s.status() == Gecode::SS_FAILED) ||
                !s.native_result.assigned() ||
                !s.boolean_result.assigned() ||
                (s.native_result.val() != s.boolean_result.val()))
              return false;
          }
        return true;
      }

      static bool constants_aliases_lifecycle(void) {
        ArithmeticSpace add_constant;
        Gecode::dom(add_constant,add_constant.x[0],15U);
        Gecode::add(add_constant,add_constant.x[0],4,1U,
                    add_constant.x[2]);
        if ((add_constant.status() == Gecode::SS_FAILED) ||
            !add_constant.x[2].assigned() ||
            (add_constant.x[2].val() != 0U))
          return false;

        ArithmeticSpace neg_constant;
        Gecode::neg(neg_constant,4,1U,neg_constant.x[2]);
        if ((neg_constant.status() == Gecode::SS_FAILED) ||
            !neg_constant.x[2].assigned() ||
            (neg_constant.x[2].val() != 15U))
          return false;

        ArithmeticSpace sub_constants;
        Gecode::dom(sub_constants,sub_constants.x[0],0U);
        Gecode::sub(sub_constants,sub_constants.x[0],4,1U,
                    sub_constants.x[1]);
        Gecode::sub(sub_constants,4,0U,sub_constants.x[0],
                    sub_constants.x[2]);
        if ((sub_constants.status() == Gecode::SS_FAILED) ||
            (sub_constants.x[1].val() != 15U) ||
            (sub_constants.x[2].val() != 0U))
          return false;

        ArithmeticSpace carry;
        Gecode::dom(carry,carry.x[0],7U);
        Gecode::dom(carry,carry.x[1],1U);
        Gecode::add(carry,carry.x[0],carry.x[1],carry.x[2]);
        if ((carry.status() == Gecode::SS_FAILED) ||
            (carry.x[2].val() != 8U))
          return false;

        ArithmeticSpace wide(3,64);
        Gecode::dom(wide,wide.x[0],~Gecode::WordValue(0));
        Gecode::dom(wide,wide.x[1],1U);
        Gecode::add(wide,wide.x[0],wide.x[1],wide.x[2]);
        if ((wide.status() == Gecode::SS_FAILED) ||
            !wide.x[2].assigned() || (wide.x[2].val() != 0U))
          return false;

        ArithmeticSpace wide_neg_sub(3,64);
        Gecode::dom(wide_neg_sub,wide_neg_sub.x[0],2U);
        Gecode::dom(wide_neg_sub,wide_neg_sub.x[1],1U);
        Gecode::neg(wide_neg_sub,wide_neg_sub.x[0],wide_neg_sub.x[2]);
        Gecode::sub(wide_neg_sub,wide_neg_sub.x[0],wide_neg_sub.x[1],
                    wide_neg_sub.x[1]);
        if ((wide_neg_sub.status() == Gecode::SS_FAILED) ||
            !wide_neg_sub.x[2].assigned() ||
            (wide_neg_sub.x[2].val() !=
             (~Gecode::WordValue(0)-Gecode::WordValue(1))) ||
            !wide_neg_sub.x[1].assigned() ||
            (wide_neg_sub.x[1].val() != 1U))
          return false;

        ArithmeticSpace aliases;
        Gecode::dom(aliases,aliases.x[0],8U);
        Gecode::dom(aliases,aliases.x[1],3U);
        Gecode::neg(aliases,aliases.x[0],aliases.x[0]);
        Gecode::sub(aliases,aliases.x[1],aliases.x[1],aliases.x[2]);
        if ((aliases.status() == Gecode::SS_FAILED) ||
            !aliases.x[2].assigned() || (aliases.x[2].val() != 0U))
          return false;

        ArithmeticSpace add_alias;
        Gecode::dom(add_alias,add_alias.x[0],5U);
        Gecode::add(add_alias,add_alias.x[0],add_alias.x[1],
                    add_alias.x[0]);
        if ((add_alias.status() == Gecode::SS_FAILED) ||
            !add_alias.x[1].assigned() || (add_alias.x[1].val() != 0U))
          return false;

        ArithmeticSpace add_repeated;
        Gecode::dom(add_repeated,add_repeated.x[2],6U);
        Gecode::add(add_repeated,add_repeated.x[0],add_repeated.x[0],
                    add_repeated.x[2]);
        if ((add_repeated.status() == Gecode::SS_FAILED) ||
            (add_repeated.x[0].lo() != 3U) ||
            (add_repeated.x[0].hi() != 11U))
          return false;

        ArithmeticSpace failed;
        Gecode::dom(failed,failed.x[0],1U);
        Gecode::dom(failed,failed.x[1],1U);
        Gecode::dom(failed,failed.x[2],3U);
        Gecode::add(failed,failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        ArithmeticSpace source;
        Gecode::add(source,source.x[0],source.x[1],source.x[2]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        ArithmeticSpace* clone =
          static_cast<ArithmeticSpace*>(source.clone());
        Gecode::dom(*clone,clone->x[0],15U);
        Gecode::dom(*clone,clone->x[1],1U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 0U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        ArithmeticSpace neg_source;
        Gecode::neg(neg_source,neg_source.x[0],neg_source.x[1]);
        if (neg_source.status() == Gecode::SS_FAILED)
          return false;
        ArithmeticSpace* neg_clone =
          static_cast<ArithmeticSpace*>(neg_source.clone());
        Gecode::dom(*neg_clone,neg_clone->x[0],1U);
        const bool neg_clone_ok =
          (neg_clone->status() != Gecode::SS_FAILED) &&
          neg_clone->x[1].assigned() && (neg_clone->x[1].val() == 15U) &&
          !neg_source.x[1].assigned();
        delete neg_clone;
        if (!neg_clone_ok)
          return false;

        ArithmeticSpace sub_source;
        Gecode::sub(sub_source,sub_source.x[0],sub_source.x[1],
                    sub_source.x[2]);
        if (sub_source.status() == Gecode::SS_FAILED)
          return false;
        ArithmeticSpace* sub_clone =
          static_cast<ArithmeticSpace*>(sub_source.clone());
        Gecode::dom(*sub_clone,sub_clone->x[0],1U);
        Gecode::dom(*sub_clone,sub_clone->x[1],2U);
        const bool sub_clone_ok =
          (sub_clone->status() != Gecode::SS_FAILED) &&
          sub_clone->x[2].assigned() && (sub_clone->x[2].val() == 15U) &&
          !sub_source.x[2].assigned();
        delete sub_clone;
        if (!sub_clone_ok)
          return false;

        try {
          ArithmeticSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::add(mismatch,mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool counters(void) {
        ArithmeticSpace add_space;
        Gecode::add(add_space,add_space.x[0],add_space.x[1],add_space.x[2]);
        // Each operation is one native actor with no model-level state.
        if (Gecode::PropagatorGroup::all.size(add_space) != 1)
          return false;
        Gecode::StatusStatistics statistics;
        if (add_space.status(statistics) == Gecode::SS_FAILED)
          return false;
        if (statistics.propagate < 1)
          return false;

        ArithmeticSpace neg_space;
        Gecode::neg(neg_space,neg_space.x[0],neg_space.x[1]);
        if (Gecode::PropagatorGroup::all.size(neg_space) != 1)
          return false;

        ArithmeticSpace sub_space;
        Gecode::sub(sub_space,sub_space.x[0],sub_space.x[1],sub_space.x[2]);
        return Gecode::PropagatorGroup::all.size(sub_space) == 1;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class AddSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar result;
          AddSpace(void) : x(*this,3), y(*this,3), result(*this,3) {
            add(*this,x,y,result);
            WordVarArgs decision(2);
            decision[0] = x;
            decision[1] = y;
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          AddSpace(AddSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new AddSpace(*this); }
        };

        AddSpace* root = new AddSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<AddSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (AddSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() &&
            solution->y.assigned() && solution->result.assigned() &&
            (solution->result.val() ==
             ((solution->x.val() + solution->y.val()) & 7U)) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 64;
      }

      static bool nary_add_lifecycle(void) {
        using namespace Gecode;
        ArithmeticSpace empty(1,4);
        WordVarArgs no_args;
        add(empty,no_args,empty.x[0]);
        if ((empty.status() == SS_FAILED) || !empty.x[0].assigned() ||
            (empty.x[0].val() != 0U) ||
            (PropagatorGroup::all.size(empty) != 0U))
          return false;

        ArithmeticSpace singleton(2,4);
        WordVarArgs one_arg = {singleton.x[0]};
        add(singleton,one_arg,singleton.x[1]);
        dom(singleton,singleton.x[0],9U);
        if ((singleton.status() == SS_FAILED) ||
            !singleton.x[1].assigned() || (singleton.x[1].val() != 9U))
          return false;

        ArithmeticSpace inverse(4,4);
        WordVarArgs inverse_args = {
          inverse.x[0],inverse.x[1],inverse.x[2]
        };
        add(inverse,inverse_args,inverse.x[3]);
        dom(inverse,inverse.x[0],7U);
        dom(inverse,inverse.x[1],1U);
        dom(inverse,inverse.x[3],9U);
        if ((inverse.status() == SS_FAILED) ||
            !inverse.x[2].assigned() || (inverse.x[2].val() != 1U))
          return false;

        ArithmeticSpace repeated(3,4);
        WordVarArgs repeated_args = {
          repeated.x[0],repeated.x[0],repeated.x[1]
        };
        add(repeated,repeated_args,repeated.x[2]);
        dom(repeated,repeated.x[0],3U);
        dom(repeated,repeated.x[1],1U);
        if ((repeated.status() == SS_FAILED) ||
            !repeated.x[2].assigned() || (repeated.x[2].val() != 7U))
          return false;

        ArithmeticSpace result_alias(3,4);
        WordVarArgs alias_args = {
          result_alias.x[0],result_alias.x[1],result_alias.x[2]
        };
        add(result_alias,alias_args,result_alias.x[0]);
        dom(result_alias,result_alias.x[1],1U);
        dom(result_alias,result_alias.x[0],6U);
        if ((result_alias.status() == SS_FAILED) ||
            !result_alias.x[2].assigned() ||
            (result_alias.x[2].val() != 15U))
          return false;

        ArithmeticSpace wide(4,64);
        WordVarArgs wide_args = {wide.x[0],wide.x[1],wide.x[2]};
        dom(wide,wide.x[0],~WordValue(0));
        dom(wide,wide.x[1],~WordValue(0));
        dom(wide,wide.x[2],2U);
        add(wide,wide_args,wide.x[3]);
        if ((wide.status() == SS_FAILED) || !wide.x[3].assigned() ||
            (wide.x[3].val() != 0U))
          return false;

        ArithmeticSpace failed(4,4);
        WordVarArgs failed_args = {failed.x[0],failed.x[1],failed.x[2]};
        dom(failed,failed.x[0],1U);
        dom(failed,failed.x[1],2U);
        dom(failed,failed.x[2],3U);
        dom(failed,failed.x[3],7U);
        add(failed,failed_args,failed.x[3]);
        if (failed.status() != SS_FAILED)
          return false;

        ArithmeticSpace source(4,4);
        WordVarArgs source_args = {source.x[0],source.x[1],source.x[2]};
        add(source,source_args,source.x[3]);
        if (source.status() == SS_FAILED)
          return false;
        ArithmeticSpace* clone=static_cast<ArithmeticSpace*>(source.clone());
        dom(*clone,clone->x[0],15U);
        dom(*clone,clone->x[1],1U);
        dom(*clone,clone->x[2],2U);
        const bool clone_ok=(clone->status() != SS_FAILED) &&
          clone->x[3].assigned() && (clone->x[3].val() == 2U) &&
          (PropagatorGroup::all.size(*clone) == 0U) &&
          !source.x[3].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          ArithmeticSpace mismatch(4,4);
          WordVar other(mismatch,3);
          WordVarArgs mismatch_args = {mismatch.x[0],other,mismatch.x[1]};
          add(mismatch,mismatch_args,mismatch.x[3]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}

        class SearchSpace : public Space {
        public:
          WordVarArray x;
          WordVar result;
          SearchSpace(void) : x(*this,3,2,0,3), result(*this,2) {
            WordVarArgs args = {x[0],x[1],x[2]};
            add(*this,args,result);
            branch(*this,x,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };
        SearchSpace* root=new SearchSpace;
        Search::Options options;
        options.c_d=8;
        options.a_d=64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (SearchSpace* solution=dfs.next()) {
          const WordValue expected=(solution->x[0].val()+
            solution->x[1].val()+solution->x[2].val())&3U;
          const bool ok=solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 64U;
      }

      static bool neg_sub_search_recomputation(void) {
        using namespace Gecode;
        class NegSubSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar negative;
          WordVar difference;
          NegSubSpace(void) : x(*this,3), y(*this,3), negative(*this,3),
            difference(*this,3) {
            neg(*this,x,negative);
            sub(*this,x,y,difference);
            WordVarArgs decision = {x,y};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          NegSubSpace(NegSubSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            negative.update(*this,s.negative);
            difference.update(*this,s.difference);
          }
          virtual Space* copy(void) { return new NegSubSpace(*this); }
        };

        NegSubSpace* root = new NegSubSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<NegSubSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (NegSubSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() && solution->y.assigned() &&
            solution->negative.assigned() &&
            solution->difference.assigned() &&
            (solution->negative.val() ==
             ((WordValue(0)-solution->x.val()) & 7U)) &&
            (solution->difference.val() ==
             ((solution->x.val()-solution->y.val()) & 7U)) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 64;
      }

    public:
      Lifecycle(void) : Base("Word::Arithmetic::Lifecycle") {}
      virtual bool run(void) {
        return partial(ADD) && add_bit_consistency() && nary_add_partial() &&
          fallback_publication() &&
          fallback_scheduled_publication() &&
          partial(NEG) && neg_bit_consistency() &&
          partial(SUB) && sub_bit_consistency() &&
          boolean_parity() && constants_aliases_lifecycle() &&
          counters() && search_recomputation() &&
          neg_sub_search_recomputation() && nary_add_lifecycle();
      }
    };

    class MultiplicationLifecycle : public Base {
    private:
      class MultiplicationSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        MultiplicationSpace(int n=3, unsigned int width=4)
          : x(*this,n,width,0,Gecode::Word::width_mask(width)) {}
        MultiplicationSpace(MultiplicationSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new MultiplicationSpace(*this);
        }
      };

      static bool partial(void) {
        for (PartialAssignment p(3,1); p.has_more(); p.next()) {
          TestSpace s(3,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<3; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::mult(s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();
          bool supported = false;
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y)
              for (Values z(p[2]); z(); ++z)
                if (z.val() == evaluate(MULT,x.val(),y.val(),1U)) {
                  supported = true;
                  if (failed || !s.x[0].in(x.val()) ||
                      !s.x[1].in(y.val()) || !s.x[2].in(z.val()))
                    return false;
                }
          if (failed == supported)
            return false;
        }
        return true;
      }

      /** Tiny ordinary Boolean schoolbook multiplication for comparison. */
      class DifferentialSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar native_result;
        Gecode::WordVar boolean_result;
        DifferentialSpace(unsigned int width)
          : x(*this,width), y(*this,width), native_result(*this,width),
            boolean_result(*this,width) {
          Gecode::mult(*this,x,y,native_result);
          Gecode::BoolVarArray x_bits(*this,width,0,1);
          Gecode::BoolVarArray y_bits(*this,width,0,1);
          Gecode::BoolVarArray result_bits(*this,width,0,1);
          for (unsigned int bit=0; bit<width; bit++) {
            Gecode::channel(*this,x,bit,x_bits[bit]);
            Gecode::channel(*this,y,bit,y_bits[bit]);
            Gecode::channel(*this,boolean_result,bit,result_bits[bit]);
          }

          Gecode::BoolVarArray accumulator(*this,width,0,0);
          for (unsigned int row=0; row<width; row++) {
            Gecode::BoolVarArray term(*this,width,0,1);
            for (unsigned int bit=0; bit<width; bit++) {
              if (bit < row)
                Gecode::rel(*this,term[bit],Gecode::IRT_EQ,0);
              else
                Gecode::rel(*this,x_bits[bit-row],Gecode::BOT_AND,
                            y_bits[row],term[bit]);
            }

            Gecode::BoolVarArray next(*this,width,0,1);
            Gecode::BoolVarArray carry(*this,width+1,0,1);
            Gecode::rel(*this,carry[0],Gecode::IRT_EQ,0);
            for (unsigned int bit=0; bit<width; bit++) {
              Gecode::BoolVar xor_xy(*this,0,1);
              Gecode::rel(*this,accumulator[bit],Gecode::BOT_XOR,
                          term[bit],xor_xy);
              Gecode::rel(*this,xor_xy,Gecode::BOT_XOR,
                          carry[bit],next[bit]);
              Gecode::BoolVar both_one(*this,0,1), carry_one(*this,0,1);
              Gecode::rel(*this,accumulator[bit],Gecode::BOT_AND,
                          term[bit],both_one);
              Gecode::rel(*this,carry[bit],Gecode::BOT_AND,
                          xor_xy,carry_one);
              Gecode::rel(*this,both_one,Gecode::BOT_OR,
                          carry_one,carry[bit+1]);
            }
            accumulator = next;
          }
          for (unsigned int bit=0; bit<width; bit++)
            Gecode::rel(*this,accumulator[bit],Gecode::IRT_EQ,
                        result_bits[bit]);
        }
        DifferentialSpace(DifferentialSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          native_result.update(*this,s.native_result);
          boolean_result.update(*this,s.boolean_result);
        }
        virtual Gecode::Space* copy(void) {
          return new DifferentialSpace(*this);
        }
      };

      static bool boolean_parity(void) {
        for (Gecode::WordValue x=0; x<8; x++)
          for (Gecode::WordValue y=0; y<8; y++) {
            DifferentialSpace s(3);
            Gecode::dom(s,s.x,x);
            Gecode::dom(s,s.y,y);
            if ((s.status() == Gecode::SS_FAILED) ||
                !s.native_result.assigned() ||
                !s.boolean_result.assigned() ||
                (s.native_result.val() != s.boolean_result.val()))
              return false;
          }
        return true;
      }

      static bool assigned(Gecode::WordValue x, Gecode::WordValue y,
                           Gecode::WordValue result) {
        MultiplicationSpace s;
        Gecode::dom(s,s.x[0],x);
        Gecode::dom(s,s.x[1],y);
        Gecode::mult(s,s.x[0],s.x[1],s.x[2]);
        return (s.status() != Gecode::SS_FAILED) && s.x[2].assigned() &&
          (s.x[2].val() == result);
      }

      static bool assigned_small(void) {
        for (unsigned int width=1; width<=4; width++) {
          const Gecode::WordValue mask = Gecode::Word::width_mask(width);
          for (Gecode::WordValue x=0; x<=mask; x++)
            for (Gecode::WordValue y=0; y<=mask; y++) {
              MultiplicationSpace s(3,width);
              Gecode::dom(s,s.x[0],x);
              Gecode::dom(s,s.x[1],y);
              Gecode::mult(s,s.x[0],s.x[1],s.x[2]);
              if ((s.status() == Gecode::SS_FAILED) ||
                  !s.x[2].assigned() ||
                  (s.x[2].val() != ((x*y) & mask)))
                return false;
            }
        }
        return true;
      }

      static bool prefix_propagation(void) {
        // Fixed low operand prefixes determine the same product prefix.
        MultiplicationSpace forward;
        Gecode::dom(forward,forward.x[0],3U,15U);
        Gecode::dom(forward,forward.x[1],2U,14U);
        Gecode::mult(forward,forward.x[0],forward.x[1],forward.x[2]);
        if ((forward.status() == Gecode::SS_FAILED) ||
            ((forward.x[2].lo() & 3U) != 2U) ||
            ((forward.x[2].hi() & 3U) != 2U))
          return false;

        // Odd factors are invertible modulo 2^width.
        MultiplicationSpace inverse;
        Gecode::dom(inverse,inverse.x[0],3U);
        Gecode::dom(inverse,inverse.x[2],9U);
        Gecode::mult(inverse,inverse.x[0],inverse.x[1],inverse.x[2]);
        if ((inverse.status() == Gecode::SS_FAILED) ||
            !inverse.x[1].assigned() || (inverse.x[1].val() != 3U))
          return false;

        // Even factors determine the low prefix left after their power of two.
        MultiplicationSpace even;
        Gecode::dom(even,even.x[0],4U);
        Gecode::dom(even,even.x[2],12U);
        Gecode::mult(even,even.x[0],even.x[1],even.x[2]);
        if ((even.status() == Gecode::SS_FAILED) ||
            ((even.x[1].lo() & 3U) != 3U) ||
            ((even.x[1].hi() & 3U) != 3U))
          return false;

        MultiplicationSpace zeros;
        Gecode::dom(zeros,zeros.x[0],0U,12U);
        Gecode::dom(zeros,zeros.x[1],0U,14U);
        Gecode::mult(zeros,zeros.x[0],zeros.x[1],zeros.x[2]);
        if ((zeros.status() == Gecode::SS_FAILED) ||
            ((zeros.x[2].hi() & 7U) != 0U))
          return false;

        MultiplicationSpace incompatible;
        Gecode::dom(incompatible,incompatible.x[0],4U);
        Gecode::dom(incompatible,incompatible.x[2],2U);
        Gecode::mult(incompatible,incompatible.x[0],incompatible.x[1],
                     incompatible.x[2]);
        return incompatible.status() == Gecode::SS_FAILED;
      }

      static bool fixed_product_inverse(void) {
        // A non-wrapping fixed product projects exact small factor supports.
        MultiplicationSpace factors(3,8);
        Gecode::dom(factors,factors.x[0],8U,15U);
        Gecode::dom(factors,factors.x[1],8U,15U);
        Gecode::dom(factors,factors.x[2],143U);
        Gecode::mult(factors,factors.x[0],factors.x[1],factors.x[2]);
        if ((factors.status() == Gecode::SS_FAILED) ||
            ((factors.x[0].lo() & 1U) == 0) ||
            ((factors.x[1].lo() & 1U) == 0))
          return false;

        MultiplicationSpace impossible(3,8);
        Gecode::dom(impossible,impossible.x[0],8U,15U);
        Gecode::dom(impossible,impossible.x[1],8U,15U);
        Gecode::dom(impossible,impossible.x[2],127U);
        Gecode::mult(impossible,impossible.x[0],impossible.x[1],
                     impossible.x[2]);
        if (impossible.status() != Gecode::SS_FAILED)
          return false;

        MultiplicationSpace square(3,4);
        Gecode::dom(square,square.x[0],0U,3U);
        Gecode::dom(square,square.x[2],9U);
        Gecode::mult(square,square.x[0],square.x[0],square.x[2]);
        if ((square.status() == Gecode::SS_FAILED) ||
            !square.x[0].assigned() || (square.x[0].val() != 3U))
          return false;

        MultiplicationSpace wide(3,64);
        Gecode::dom(wide,wide.x[0],8U,15U);
        Gecode::dom(wide,wide.x[1],8U,15U);
        Gecode::dom(wide,wide.x[2],143U);
        Gecode::mult(wide,wide.x[0],wide.x[1],wide.x[2]);
        return (wide.status() != Gecode::SS_FAILED) &&
          ((wide.x[0].lo() & 1U) != 0) &&
          ((wide.x[1].lo() & 1U) != 0);
      }

      static bool lifecycle(void) {
        if (!assigned(0U,15U,0U) || !assigned(1U,15U,15U) ||
            !assigned(15U,15U,1U) || !assigned(2U,4U,8U))
          return false;

        MultiplicationSpace constant;
        Gecode::dom(constant,constant.x[0],5U);
        Gecode::mult(constant,constant.x[0],4,3U,constant.x[2]);
        if ((constant.status() == Gecode::SS_FAILED) ||
            !constant.x[2].assigned() || (constant.x[2].val() != 15U))
          return false;

        MultiplicationSpace aliases;
        Gecode::dom(aliases,aliases.x[0],3U);
        Gecode::mult(aliases,aliases.x[0],aliases.x[0],aliases.x[2]);
        Gecode::mult(aliases,aliases.x[0],4,1U,aliases.x[0]);
        if ((aliases.status() == Gecode::SS_FAILED) ||
            !aliases.x[2].assigned() || (aliases.x[2].val() != 9U))
          return false;

        MultiplicationSpace failed;
        Gecode::dom(failed,failed.x[0],3U);
        Gecode::dom(failed,failed.x[1],3U);
        Gecode::dom(failed,failed.x[2],8U);
        Gecode::mult(failed,failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        MultiplicationSpace source;
        Gecode::mult(source,source.x[0],source.x[1],source.x[2]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        MultiplicationSpace* clone =
          static_cast<MultiplicationSpace*>(source.clone());
        Gecode::dom(*clone,clone->x[0],3U);
        Gecode::dom(*clone,clone->x[1],5U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 15U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          MultiplicationSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::mult(mismatch,mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool counters(void) {
        MultiplicationSpace s(3,3);
        Gecode::mult(s,s.x[0],s.x[1],s.x[2]);
        // Multiplication is one actor and allocates no model-level terms.
        if (Gecode::PropagatorGroup::all.size(s) != 1U)
          return false;
        Gecode::StatusStatistics statistics;
        if (s.status(statistics) == Gecode::SS_FAILED)
          return false;
        return statistics.propagate >= 1U;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class MultSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar result;
          MultSpace(void) : x(*this,2), y(*this,2), result(*this,2) {
            mult(*this,x,y,result);
            WordVarArgs decision(2);
            decision[0] = x;
            decision[1] = y;
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          MultSpace(MultSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new MultSpace(*this); }
        };

        MultSpace* root = new MultSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<MultSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (MultSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() &&
            solution->y.assigned() && solution->result.assigned() &&
            (solution->result.val() ==
             ((solution->x.val() * solution->y.val()) & 3U)) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 16;
      }

      static bool inverse_search_recomputation(void) {
        using namespace Gecode;
        class FactorSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar result;
          FactorSpace(void)
            : x(*this,8,8U,15U), y(*this,8,8U,15U),
              result(*this,8,143U,143U) {
            mult(*this,x,y,result);
            WordVarArgs decision={x,y};
            branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_LSB());
          }
          FactorSpace(FactorSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new FactorSpace(*this); }
        };

        FactorSpace* root = new FactorSpace;
        Search::Options options;
        options.c_d = 1;
        options.a_d = 64;
        DFS<FactorSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (FactorSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() &&
            solution->y.assigned() && solution->result.assigned() &&
            (solution->x.val()*solution->y.val() == 143U) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 2;
      }

    public:
      MultiplicationLifecycle(void)
        : Base("Word::Arithmetic::MultLifecycle") {}
      virtual bool run(void) {
        return partial() && assigned_small() && prefix_propagation() &&
          fixed_product_inverse() && boolean_parity() && lifecycle() &&
          counters() && search_recomputation() &&
          inverse_search_recomputation();
      }
    };

    class DivisionLifecycle : public Base {
    private:
      class DivisionSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        DivisionSpace(int n=4, unsigned int width=4)
          : x(*this,n,width,0,Gecode::Word::width_mask(width)) {}
        DivisionSpace(DivisionSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new DivisionSpace(*this);
        }
      };

      static bool partial(Op op) {
        for (PartialAssignment p(3,1); p.has_more(); p.next()) {
          TestSpace s(3,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<3; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Arithmetic::post(op,s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y)
              for (Values z(p[2]); z(); ++z)
                if (z.val() == evaluate(op,x.val(),y.val(),1U)) {
                  if (failed || !s.x[0].in(x.val()) ||
                      !s.x[1].in(y.val()) || !s.x[2].in(z.val()))
                    return false;
                  TestSpace* tuple = static_cast<TestSpace*>(s.clone());
                  Gecode::dom(*tuple,tuple->x[0],x.val());
                  Gecode::dom(*tuple,tuple->x[1],y.val());
                  Gecode::dom(*tuple,tuple->x[2],z.val());
                  const bool feasible = !tuple->failed();
                  delete tuple;
                  if (!feasible)
                    return false;
                }
        }
        return true;
      }

      /** Tiny ordinary Boolean extensional reference for both results. */
      class DifferentialSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar native_div;
        Gecode::WordVar native_mod;
        Gecode::WordVar separate_div;
        Gecode::WordVar separate_mod;
        Gecode::WordVar boolean_div;
        Gecode::WordVar boolean_mod;
        DifferentialSpace(void)
          : x(*this,2), y(*this,2), native_div(*this,2),
            native_mod(*this,2), separate_div(*this,2),
            separate_mod(*this,2), boolean_div(*this,2),
            boolean_mod(*this,2) {
          Gecode::divmod(*this,x,y,native_div,native_mod);
          Gecode::div(*this,x,y,separate_div);
          Gecode::mod(*this,x,y,separate_mod);

          Gecode::BoolVarArray bits(*this,8,0,1);
          for (unsigned int bit=0; bit<2; bit++) {
            Gecode::channel(*this,x,bit,bits[bit]);
            Gecode::channel(*this,y,bit,bits[2+bit]);
            Gecode::channel(*this,boolean_div,bit,bits[4+bit]);
            Gecode::channel(*this,boolean_mod,bit,bits[6+bit]);
          }
          Gecode::TupleSet tuples(8);
          for (int xv=0; xv<4; xv++)
            for (int yv=0; yv<4; yv++) {
              const int q = (yv == 0) ? 3 : xv/yv;
              const int r = (yv == 0) ? xv : xv%yv;
              tuples.add(Gecode::IntArgs({xv & 1, (xv >> 1) & 1,
                                          yv & 1, (yv >> 1) & 1,
                                          q & 1, (q >> 1) & 1,
                                          r & 1, (r >> 1) & 1}));
            }
          tuples.finalize();
          Gecode::extensional(*this,bits,tuples);
        }
        DifferentialSpace(DifferentialSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          native_div.update(*this,s.native_div);
          native_mod.update(*this,s.native_mod);
          separate_div.update(*this,s.separate_div);
          separate_mod.update(*this,s.separate_mod);
          boolean_div.update(*this,s.boolean_div);
          boolean_mod.update(*this,s.boolean_mod);
        }
        virtual Gecode::Space* copy(void) {
          return new DifferentialSpace(*this);
        }
      };

      static bool boolean_parity(void) {
        for (Gecode::WordValue x=0; x<4; x++)
          for (Gecode::WordValue y=0; y<4; y++) {
            DifferentialSpace s;
            Gecode::dom(s,s.x,x);
            Gecode::dom(s,s.y,y);
            if ((s.status() == Gecode::SS_FAILED) ||
                !s.native_div.assigned() || !s.native_mod.assigned() ||
                !s.separate_div.assigned() || !s.separate_mod.assigned() ||
                !s.boolean_div.assigned() || !s.boolean_mod.assigned() ||
                (s.native_div.val() != s.separate_div.val()) ||
                (s.native_mod.val() != s.separate_mod.val()) ||
                (s.native_div.val() != s.boolean_div.val()) ||
                (s.native_mod.val() != s.boolean_mod.val()))
              return false;
          }
        return true;
      }

      static bool policy_constants_aliases_lifecycle(void) {
        DivisionSpace policy(4,3);
        Gecode::dom(policy,policy.x[0],5U);
        Gecode::dom(policy,policy.x[1],0U);
        Gecode::div(policy,policy.x[0],policy.x[1],policy.x[2]);
        Gecode::mod(policy,policy.x[0],policy.x[1],policy.x[3],
                    Gecode::WS_SMTLIB);
        if ((policy.status() == Gecode::SS_FAILED) ||
            !policy.x[2].assigned() || (policy.x[2].val() != 7U) ||
            !policy.x[3].assigned() || (policy.x[3].val() != 5U))
          return false;

        DivisionSpace constants(4,3);
        Gecode::dom(constants,constants.x[0],6U);
        Gecode::div(constants,constants.x[0],3,2U,constants.x[1]);
        Gecode::mod(constants,3,5U,constants.x[0],constants.x[2]);
        Gecode::div(constants,3,6U,constants.x[0],constants.x[3]);
        if ((constants.status() == Gecode::SS_FAILED) ||
            (constants.x[1].val() != 3U) ||
            (constants.x[2].val() != 5U) ||
            (constants.x[3].val() != 1U))
          return false;

        DivisionSpace right_mod(4,3);
        Gecode::dom(right_mod,right_mod.x[0],6U);
        Gecode::mod(right_mod,right_mod.x[0],3,4U,right_mod.x[1]);
        if ((right_mod.status() == Gecode::SS_FAILED) ||
            !right_mod.x[1].assigned() || (right_mod.x[1].val() != 2U))
          return false;

        DivisionSpace div_alias(4,3);
        Gecode::dom(div_alias,div_alias.x[0],6U);
        Gecode::dom(div_alias,div_alias.x[1],1U);
        Gecode::div(div_alias,div_alias.x[0],div_alias.x[1],
                    div_alias.x[0]);
        if ((div_alias.status() == Gecode::SS_FAILED) ||
            (div_alias.x[0].val() != 6U))
          return false;

        DivisionSpace mod_alias(4,3);
        Gecode::dom(mod_alias,mod_alias.x[0],3U);
        Gecode::dom(mod_alias,mod_alias.x[1],4U);
        Gecode::mod(mod_alias,mod_alias.x[0],mod_alias.x[1],
                    mod_alias.x[0]);
        if ((mod_alias.status() == Gecode::SS_FAILED) ||
            (mod_alias.x[0].val() != 3U))
          return false;

        DivisionSpace failed(4,3);
        Gecode::dom(failed,failed.x[0],6U);
        Gecode::dom(failed,failed.x[1],2U);
        Gecode::dom(failed,failed.x[2],2U);
        Gecode::div(failed,failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        DivisionSpace source(4,3);
        Gecode::divmod(source,source.x[0],source.x[1],source.x[2],
                       source.x[3]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        DivisionSpace* clone = static_cast<DivisionSpace*>(source.clone());
        Gecode::dom(*clone,clone->x[0],7U);
        Gecode::dom(*clone,clone->x[1],2U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 3U) &&
          clone->x[3].assigned() && (clone->x[3].val() == 1U) &&
          !source.x[2].assigned() && !source.x[3].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        DivisionSpace combined_alias(4,3);
        Gecode::dom(combined_alias,combined_alias.x[1],0U);
        Gecode::divmod(combined_alias,combined_alias.x[0],
                       combined_alias.x[1],combined_alias.x[2],
                       combined_alias.x[0]);
        if ((combined_alias.status() == Gecode::SS_FAILED) ||
            !combined_alias.x[2].assigned() ||
            (combined_alias.x[2].val() != 7U) ||
            (Gecode::PropagatorGroup::all.size(combined_alias) != 0U))
          return false;

        DivisionSpace combined_failed(4,3);
        Gecode::dom(combined_failed,combined_failed.x[0],6U);
        Gecode::dom(combined_failed,combined_failed.x[1],2U);
        Gecode::dom(combined_failed,combined_failed.x[2],2U);
        Gecode::dom(combined_failed,combined_failed.x[3],0U);
        Gecode::divmod(combined_failed,combined_failed.x[0],
                       combined_failed.x[1],combined_failed.x[2],
                       combined_failed.x[3]);
        if (combined_failed.status() != Gecode::SS_FAILED)
          return false;

        try {
          DivisionSpace mismatch(4,3);
          Gecode::WordVar other(mismatch,2);
          Gecode::divmod(mismatch,mismatch.x[0],other,mismatch.x[2],
                         mismatch.x[3]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool native_propagation(void) {
        using namespace Gecode;
        DivisionSpace quotient_inverse(3,4);
        dom(quotient_inverse,quotient_inverse.x[1],2U);
        dom(quotient_inverse,quotient_inverse.x[2],3U);
        div(quotient_inverse,quotient_inverse.x[0],
            quotient_inverse.x[1],quotient_inverse.x[2]);
        if ((quotient_inverse.status() == SS_FAILED) ||
            (quotient_inverse.x[0].lo() != 6U) ||
            (quotient_inverse.x[0].hi() != 7U) ||
            (PropagatorGroup::all.size(quotient_inverse) != 1U))
          return false;

        DivisionSpace divisor_inverse(3,4);
        dom(divisor_inverse,divisor_inverse.x[0],6U);
        dom(divisor_inverse,divisor_inverse.x[2],3U);
        div(divisor_inverse,divisor_inverse.x[0],divisor_inverse.x[1],
            divisor_inverse.x[2]);
        if ((divisor_inverse.status() == SS_FAILED) ||
            !divisor_inverse.x[1].assigned() ||
            (divisor_inverse.x[1].val() != 2U))
          return false;

        DivisionSpace remainder_inverse(3,4);
        dom(remainder_inverse,remainder_inverse.x[1],4U);
        dom(remainder_inverse,remainder_inverse.x[2],2U);
        mod(remainder_inverse,remainder_inverse.x[0],
            remainder_inverse.x[1],remainder_inverse.x[2]);
        if ((remainder_inverse.status() == SS_FAILED) ||
            (remainder_inverse.x[0].lo() != 2U) ||
            (remainder_inverse.x[0].hi() != 14U) ||
            (PropagatorGroup::all.size(remainder_inverse) != 1U))
          return false;

        DivisionSpace combined_inverse(4,4);
        dom(combined_inverse,combined_inverse.x[1],3U);
        dom(combined_inverse,combined_inverse.x[2],2U);
        dom(combined_inverse,combined_inverse.x[3],1U);
        divmod(combined_inverse,combined_inverse.x[0],
               combined_inverse.x[1],combined_inverse.x[2],
               combined_inverse.x[3]);
        if ((combined_inverse.status() == SS_FAILED) ||
            !combined_inverse.x[0].assigned() ||
            (combined_inverse.x[0].val() != 7U) ||
            (PropagatorGroup::all.size(combined_inverse) != 0U))
          return false;

        DivisionSpace zero_divisor_div(3,4);
        dom(zero_divisor_div,zero_divisor_div.x[1],0U);
        dom(zero_divisor_div,zero_divisor_div.x[2],15U);
        div(zero_divisor_div,zero_divisor_div.x[0],
            zero_divisor_div.x[1],zero_divisor_div.x[2]);
        if ((zero_divisor_div.status() == SS_FAILED) ||
            zero_divisor_div.x[0].assigned() ||
            (PropagatorGroup::all.size(zero_divisor_div) != 0U))
          return false;

        DivisionSpace zero_divisor_mod(2,4);
        dom(zero_divisor_mod,zero_divisor_mod.x[1],0U);
        mod(zero_divisor_mod,zero_divisor_mod.x[0],
            zero_divisor_mod.x[1],zero_divisor_mod.x[0]);
        if ((zero_divisor_mod.status() == SS_FAILED) ||
            zero_divisor_mod.x[0].assigned() ||
            (PropagatorGroup::all.size(zero_divisor_mod) != 0U))
          return false;

        DivisionSpace zero_inverse(3,4);
        dom(zero_inverse,zero_inverse.x[1],0U);
        dom(zero_inverse,zero_inverse.x[2],5U);
        mod(zero_inverse,zero_inverse.x[0],zero_inverse.x[1],
            zero_inverse.x[2]);
        return (zero_inverse.status() != SS_FAILED) &&
          zero_inverse.x[0].assigned() &&
          (zero_inverse.x[0].val() == 5U) &&
          (PropagatorGroup::all.size(zero_inverse) == 0U);
      }

      static bool self_identities(void) {
        using namespace Gecode;
        const unsigned int widths[] = {1U,64U};
        for (unsigned int width : widths) {
          DivisionSpace self_mod(2,width);
          mod(self_mod,self_mod.x[0],self_mod.x[0],self_mod.x[1]);
          if ((self_mod.status() == SS_FAILED) ||
              self_mod.x[0].assigned() || !self_mod.x[1].assigned() ||
              (self_mod.x[1].val() != 0) ||
              (PropagatorGroup::all.size(self_mod) != 0U))
            return false;
          DivisionSpace* copy =
            static_cast<DivisionSpace*>(self_mod.clone());
          const bool copy_ok = (copy->status() != SS_FAILED) &&
            !copy->x[0].assigned() && copy->x[1].assigned() &&
            (copy->x[1].val() == 0) &&
            (PropagatorGroup::all.size(*copy) == 0U);
          delete copy;
          if (!copy_ok)
            return false;
        }

        DivisionSpace mod_result_alias(1,4);
        mod(mod_result_alias,mod_result_alias.x[0],mod_result_alias.x[0],
            mod_result_alias.x[0]);
        if ((mod_result_alias.status() == SS_FAILED) ||
            !mod_result_alias.x[0].assigned() ||
            (mod_result_alias.x[0].val() != 0U) ||
            (PropagatorGroup::all.size(mod_result_alias) != 0U))
          return false;

        DivisionSpace width_one(3,1);
        divmod(width_one,width_one.x[0],width_one.x[0],width_one.x[1],
               width_one.x[2]);
        if ((width_one.status() == SS_FAILED) ||
            !width_one.x[1].assigned() || (width_one.x[1].val() != 1U) ||
            !width_one.x[2].assigned() || (width_one.x[2].val() != 0U) ||
            (PropagatorGroup::all.size(width_one) != 0U))
          return false;

        DivisionSpace combined(3,64);
        divmod(combined,combined.x[0],combined.x[0],combined.x[1],
               combined.x[2]);
        if ((combined.status() == SS_FAILED) || combined.x[0].assigned() ||
            (combined.x[1].lo() != 1U) ||
            (combined.x[1].hi() != Gecode::Word::width_mask(64)) ||
            !combined.x[2].assigned() || (combined.x[2].val() != 0U) ||
            (PropagatorGroup::all.size(combined) != 1U))
          return false;
        DivisionSpace* combined_copy =
          static_cast<DivisionSpace*>(combined.clone());
        dom(*combined_copy,combined_copy->x[0],0U);
        const bool combined_copy_ok =
          (combined_copy->status() != SS_FAILED) &&
          combined_copy->x[1].assigned() &&
          (combined_copy->x[1].val() == Gecode::Word::width_mask(64)) &&
          (PropagatorGroup::all.size(*combined_copy) == 0U) &&
          !combined.x[0].assigned() && !combined.x[1].assigned();
        delete combined_copy;
        if (!combined_copy_ok)
          return false;

        DivisionSpace remainder_alias(2,4);
        divmod(remainder_alias,remainder_alias.x[0],remainder_alias.x[0],
               remainder_alias.x[1],remainder_alias.x[0]);
        if ((remainder_alias.status() == SS_FAILED) ||
            !remainder_alias.x[0].assigned() ||
            (remainder_alias.x[0].val() != 0U) ||
            !remainder_alias.x[1].assigned() ||
            (remainder_alias.x[1].val() != 15U) ||
            (PropagatorGroup::all.size(remainder_alias) != 0U))
          return false;

        DivisionSpace quotient_alias(2,4);
        divmod(quotient_alias,quotient_alias.x[0],quotient_alias.x[0],
               quotient_alias.x[0],quotient_alias.x[1]);
        if ((quotient_alias.status() == SS_FAILED) ||
            !quotient_alias.x[0].assigned() ||
            (quotient_alias.x[0].val() != 1U) ||
            !quotient_alias.x[1].assigned() ||
            (quotient_alias.x[1].val() != 0U) ||
            (PropagatorGroup::all.size(quotient_alias) != 0U))
          return false;

        DivisionSpace quotient_remainder_alias(2,4);
        divmod(quotient_remainder_alias,quotient_remainder_alias.x[0],
               quotient_remainder_alias.x[0],
               quotient_remainder_alias.x[1],
               quotient_remainder_alias.x[1]);
        if (quotient_remainder_alias.status() != SS_FAILED)
          return false;

        DivisionSpace all_alias(1,4);
        divmod(all_alias,all_alias.x[0],all_alias.x[0],all_alias.x[0],
               all_alias.x[0]);
        if (all_alias.status() != SS_FAILED)
          return false;

        DivisionSpace failed(2,4);
        dom(failed,failed.x[1],1U);
        mod(failed,failed.x[0],failed.x[0],failed.x[1]);
        return failed.status() == SS_FAILED;
      }

      static bool self_search_recomputation(void) {
        using namespace Gecode;
        class SelfSpace : public Space {
        public:
          WordVar x;
          WordVar quotient;
          WordVar remainder;
          SelfSpace(void)
            : x(*this,3), quotient(*this,3), remainder(*this,3) {
            divmod(*this,x,x,quotient,remainder);
            branch(*this,x,WORD_VAL_LSB());
          }
          SelfSpace(SelfSpace& s) : Space(s) {
            x.update(*this,s.x);
            quotient.update(*this,s.quotient);
            remainder.update(*this,s.remainder);
          }
          virtual Space* copy(void) { return new SelfSpace(*this); }
        };

        SelfSpace* root = new SelfSpace;
        Search::Options options;
        options.c_d = 1;
        options.a_d = 64;
        DFS<SelfSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SelfSpace* solution = dfs.next()) {
          const WordValue expected = (solution->x.val() == 0) ? 7U : 1U;
          const bool ok = solution->x.assigned() &&
            solution->quotient.assigned() &&
            (solution->quotient.val() == expected) &&
            solution->remainder.assigned() &&
            (solution->remainder.val() == 0U) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class DivSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar quotient;
          WordVar remainder;
          DivSpace(void) : x(*this,2), y(*this,2), quotient(*this,2),
            remainder(*this,2) {
            divmod(*this,x,y,quotient,remainder);
            WordVarArgs decision(2);
            decision[0] = x;
            decision[1] = y;
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          DivSpace(DivSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            quotient.update(*this,s.quotient);
            remainder.update(*this,s.remainder);
          }
          virtual Space* copy(void) { return new DivSpace(*this); }
        };

        DivSpace* root = new DivSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<DivSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (DivSpace* solution = dfs.next()) {
          const WordValue expected_div = solution->y.val() == 0 ?
            3U : solution->x.val() / solution->y.val();
          const WordValue expected_mod = solution->y.val() == 0 ?
            solution->x.val() : solution->x.val() % solution->y.val();
          const bool ok = solution->x.assigned() &&
            solution->y.assigned() && solution->quotient.assigned() &&
            solution->remainder.assigned() &&
            (solution->quotient.val() == expected_div) &&
            (solution->remainder.val() == expected_mod) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 16;
      }

    public:
      DivisionLifecycle(void)
        : Base("Word::Arithmetic::DivisionLifecycle") {}
      virtual bool run(void) {
        return partial(DIV) && partial(MOD) && boolean_parity() &&
          policy_constants_aliases_lifecycle() && native_propagation() &&
          self_identities() && self_search_recomputation() &&
          search_recomputation();
      }
    };

    class SignedDivisionLifecycle : public Base {
    private:
      class SignedSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        SignedSpace(int n=5, unsigned int width=4)
          : x(*this,n,width,0,Gecode::Word::width_mask(width)) {}
        SignedSpace(SignedSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new SignedSpace(*this);
        }
      };

      static bool partial(Op op) {
        for (PartialAssignment p(3,1); p.has_more(); p.next()) {
          TestSpace s(3,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<3; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Arithmetic::post(op,s,s.x[0],s.x[1],s.x[2]);
          const bool failed = s.failed();
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y)
              for (Values z(p[2]); z(); ++z)
                if (z.val() == evaluate(op,x.val(),y.val(),1U)) {
                  if (failed || !s.x[0].in(x.val()) ||
                      !s.x[1].in(y.val()) || !s.x[2].in(z.val()))
                    return false;
                  TestSpace* tuple = static_cast<TestSpace*>(s.clone());
                  Gecode::dom(*tuple,tuple->x[0],x.val());
                  Gecode::dom(*tuple,tuple->x[1],y.val());
                  Gecode::dom(*tuple,tuple->x[2],z.val());
                  const bool feasible = !tuple->failed();
                  delete tuple;
                  if (!feasible)
                    return false;
                }
        }
        return true;
      }

      /** Tiny Boolean extensional reference for the three signed results. */
      class DifferentialSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar native_div;
        Gecode::WordVar native_rem;
        Gecode::WordVar native_mod;
        Gecode::WordVar boolean_div;
        Gecode::WordVar boolean_rem;
        Gecode::WordVar boolean_mod;
        DifferentialSpace(void)
          : x(*this,2), y(*this,2), native_div(*this,2),
            native_rem(*this,2), native_mod(*this,2),
            boolean_div(*this,2), boolean_rem(*this,2),
            boolean_mod(*this,2) {
          Gecode::signed_div(*this,x,y,native_div);
          Gecode::signed_rem(*this,x,y,native_rem);
          Gecode::signed_mod(*this,x,y,native_mod);

          Gecode::BoolVarArray bits(*this,10,0,1);
          Gecode::WordVarArgs words = {x, y, boolean_div, boolean_rem,
                                       boolean_mod};
          for (int word=0; word<words.size(); word++)
            for (unsigned int bit=0; bit<2; bit++)
              Gecode::channel(*this,words[word],bit,bits[2*word+bit]);

          Gecode::TupleSet tuples(10);
          for (int xv=0; xv<4; xv++)
            for (int yv=0; yv<4; yv++) {
              const int q = static_cast<int>(
                evaluate(SIGNED_DIV,xv,yv,3U));
              const int r = static_cast<int>(
                evaluate(SIGNED_REM,xv,yv,3U));
              const int m = static_cast<int>(
                evaluate(SIGNED_MOD,xv,yv,3U));
              tuples.add(Gecode::IntArgs({xv & 1, (xv >> 1) & 1,
                                          yv & 1, (yv >> 1) & 1,
                                          q & 1, (q >> 1) & 1,
                                          r & 1, (r >> 1) & 1,
                                          m & 1, (m >> 1) & 1}));
            }
          tuples.finalize();
          Gecode::extensional(*this,bits,tuples);
        }
        DifferentialSpace(DifferentialSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          native_div.update(*this,s.native_div);
          native_rem.update(*this,s.native_rem);
          native_mod.update(*this,s.native_mod);
          boolean_div.update(*this,s.boolean_div);
          boolean_rem.update(*this,s.boolean_rem);
          boolean_mod.update(*this,s.boolean_mod);
        }
        virtual Gecode::Space* copy(void) {
          return new DifferentialSpace(*this);
        }
      };

      static bool boolean_parity(void) {
        for (Gecode::WordValue x=0; x<4; x++)
          for (Gecode::WordValue y=0; y<4; y++) {
            DifferentialSpace s;
            Gecode::dom(s,s.x,x);
            Gecode::dom(s,s.y,y);
            if ((s.status() == Gecode::SS_FAILED) ||
                !s.native_div.assigned() || !s.native_rem.assigned() ||
                !s.native_mod.assigned() || !s.boolean_div.assigned() ||
                !s.boolean_rem.assigned() || !s.boolean_mod.assigned() ||
                (s.native_div.val() != s.boolean_div.val()) ||
                (s.native_rem.val() != s.boolean_rem.val()) ||
                (s.native_mod.val() != s.boolean_mod.val()))
              return false;
          }
        return true;
      }

      static bool constant(Op op, bool left, Gecode::WordValue variable,
                           Gecode::WordValue value,
                           Gecode::WordValue expected) {
        SignedSpace s(2,4);
        Gecode::dom(s,s.x[0],variable);
        if (left) {
          switch (op) {
          case SIGNED_DIV:
            Gecode::signed_div(s,4,value,s.x[0],s.x[1]); break;
          case SIGNED_REM:
            Gecode::signed_rem(s,4,value,s.x[0],s.x[1]); break;
          case SIGNED_MOD:
            Gecode::signed_mod(s,4,value,s.x[0],s.x[1]); break;
          default: GECODE_NEVER;
          }
        } else {
          switch (op) {
          case SIGNED_DIV:
            Gecode::signed_div(s,s.x[0],4,value,s.x[1]); break;
          case SIGNED_REM:
            Gecode::signed_rem(s,s.x[0],4,value,s.x[1]); break;
          case SIGNED_MOD:
            Gecode::signed_mod(s,s.x[0],4,value,s.x[1]); break;
          default: GECODE_NEVER;
          }
        }
        return (s.status() != Gecode::SS_FAILED) && s.x[1].assigned() &&
          (s.x[1].val() == expected);
      }

      static bool lifecycle(void) {
        if (!constant(SIGNED_DIV,false,10U,2U,13U) ||
            !constant(SIGNED_DIV,true,14U,6U,13U) ||
            !constant(SIGNED_REM,false,9U,3U,15U) ||
            !constant(SIGNED_REM,true,13U,7U,1U) ||
            !constant(SIGNED_MOD,false,9U,3U,2U) ||
            !constant(SIGNED_MOD,true,13U,7U,14U))
          return false;

        SignedSpace edge(5,4);
        Gecode::dom(edge,edge.x[0],8U);
        Gecode::dom(edge,edge.x[1],15U);
        Gecode::signed_div(edge,edge.x[0],edge.x[1],edge.x[2]);
        Gecode::signed_rem(edge,edge.x[0],edge.x[1],edge.x[3]);
        Gecode::signed_mod(edge,edge.x[0],edge.x[1],edge.x[4],
                           Gecode::WS_SMTLIB);
        if ((edge.status() == Gecode::SS_FAILED) ||
            (edge.x[2].val() != 8U) || (edge.x[3].val() != 0U) ||
            (edge.x[4].val() != 0U))
          return false;

        SignedSpace zero(5,4);
        Gecode::dom(zero,zero.x[0],10U);
        Gecode::dom(zero,zero.x[1],0U);
        Gecode::signed_div(zero,zero.x[0],zero.x[1],zero.x[2]);
        Gecode::signed_rem(zero,zero.x[0],zero.x[1],zero.x[3]);
        Gecode::signed_mod(zero,zero.x[0],zero.x[1],zero.x[4]);
        if ((zero.status() == Gecode::SS_FAILED) ||
            (zero.x[2].val() != 1U) || (zero.x[3].val() != 10U) ||
            (zero.x[4].val() != 10U))
          return false;

        SignedSpace distinct(5,4);
        Gecode::dom(distinct,distinct.x[0],9U);
        Gecode::dom(distinct,distinct.x[1],3U);
        Gecode::signed_rem(distinct,distinct.x[0],distinct.x[1],
                           distinct.x[2]);
        Gecode::signed_mod(distinct,distinct.x[0],distinct.x[1],
                           distinct.x[3]);
        if ((distinct.status() == Gecode::SS_FAILED) ||
            (distinct.x[2].val() != 15U) ||
            (distinct.x[3].val() != 2U))
          return false;

        SignedSpace alias(5,4);
        Gecode::dom(alias,alias.x[0],10U);
        Gecode::dom(alias,alias.x[1],1U);
        Gecode::signed_div(alias,alias.x[0],alias.x[1],alias.x[0]);
        if ((alias.status() == Gecode::SS_FAILED) ||
            (alias.x[0].val() != 10U))
          return false;

        SignedSpace failed(5,4);
        Gecode::dom(failed,failed.x[0],9U);
        Gecode::dom(failed,failed.x[1],3U);
        Gecode::dom(failed,failed.x[2],13U);
        Gecode::signed_div(failed,failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        SignedSpace source(5,3);
        Gecode::signed_div(source,source.x[0],source.x[1],source.x[2]);
        Gecode::signed_rem(source,source.x[0],source.x[1],source.x[3]);
        Gecode::signed_mod(source,source.x[0],source.x[1],source.x[4]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        SignedSpace* clone = static_cast<SignedSpace*>(source.clone());
        Gecode::dom(*clone,clone->x[0],5U);
        Gecode::dom(*clone,clone->x[1],3U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 7U) &&
          clone->x[3].assigned() && (clone->x[3].val() == 0U) &&
          clone->x[4].assigned() && (clone->x[4].val() == 0U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          SignedSpace mismatch(5,3);
          Gecode::WordVar other(mismatch,2);
          Gecode::signed_div(mismatch,mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool native_propagation(void) {
        SignedSpace compact_div(3,64);
        Gecode::signed_div(compact_div,compact_div.x[0],compact_div.x[1],
                           compact_div.x[2]);
        if ((compact_div.status() == Gecode::SS_FAILED) ||
            (Gecode::PropagatorGroup::all.size(compact_div) != 1U))
          return false;

        SignedSpace positive_rem(3,4);
        Gecode::dom(positive_rem,positive_rem.x[0],0U,7U);
        Gecode::dom(positive_rem,positive_rem.x[1],3U);
        Gecode::signed_rem(positive_rem,positive_rem.x[0],
                           positive_rem.x[1],positive_rem.x[2]);
        if ((positive_rem.status() == Gecode::SS_FAILED) ||
            ((positive_rem.x[2].hi() & 8U) != 0))
          return false;

        SignedSpace negative_mod(3,4);
        Gecode::dom(negative_mod,negative_mod.x[1],8U,15U);
        Gecode::dom(negative_mod,negative_mod.x[2],1U,15U);
        Gecode::signed_mod(negative_mod,negative_mod.x[0],negative_mod.x[1],
                           negative_mod.x[2]);
        if ((negative_mod.status() == Gecode::SS_FAILED) ||
            ((negative_mod.x[2].lo() & 8U) == 0))
          return false;

        SignedSpace zero_negative(3,4);
        Gecode::dom(zero_negative,zero_negative.x[0],8U,15U);
        Gecode::dom(zero_negative,zero_negative.x[1],0U);
        Gecode::signed_div(zero_negative,zero_negative.x[0],
                           zero_negative.x[1],zero_negative.x[2]);
        if ((zero_negative.status() == Gecode::SS_FAILED) ||
            !zero_negative.x[2].assigned() ||
            (zero_negative.x[2].val() != 1U) ||
            (Gecode::PropagatorGroup::all.size(zero_negative) != 0U))
          return false;

        SignedSpace minus_one(3,4);
        Gecode::dom(minus_one,minus_one.x[1],15U);
        Gecode::signed_div(minus_one,minus_one.x[0],minus_one.x[1],
                           minus_one.x[2]);
        return (minus_one.status() != Gecode::SS_FAILED) &&
          (Gecode::PropagatorGroup::all.size(minus_one) == 1U);
      }

      static bool self_identities(void) {
        using namespace Gecode;
        const unsigned int widths[] = {1U,64U};
        for (unsigned int width : widths) {
          SignedSpace self_rem(2,width);
          signed_rem(self_rem,self_rem.x[0],self_rem.x[0],self_rem.x[1]);
          if ((self_rem.status() == SS_FAILED) ||
              self_rem.x[0].assigned() || !self_rem.x[1].assigned() ||
              (self_rem.x[1].val() != 0U) ||
              (PropagatorGroup::all.size(self_rem) != 0U))
            return false;
          SignedSpace* copy = static_cast<SignedSpace*>(self_rem.clone());
          const bool copy_ok = (copy->status() != SS_FAILED) &&
            !copy->x[0].assigned() && copy->x[1].assigned() &&
            (copy->x[1].val() == 0U) &&
            (PropagatorGroup::all.size(*copy) == 0U);
          delete copy;
          if (!copy_ok)
            return false;

          SignedSpace self_mod(2,width);
          signed_mod(self_mod,self_mod.x[0],self_mod.x[0],self_mod.x[1]);
          if ((self_mod.status() == SS_FAILED) ||
              self_mod.x[0].assigned() || !self_mod.x[1].assigned() ||
              (self_mod.x[1].val() != 0U) ||
              (PropagatorGroup::all.size(self_mod) != 0U))
            return false;
        }

        SignedSpace rem_alias(1,4);
        signed_rem(rem_alias,rem_alias.x[0],rem_alias.x[0],rem_alias.x[0]);
        if ((rem_alias.status() == SS_FAILED) ||
            !rem_alias.x[0].assigned() || (rem_alias.x[0].val() != 0U) ||
            (PropagatorGroup::all.size(rem_alias) != 0U))
          return false;

        SignedSpace mod_alias(1,4);
        signed_mod(mod_alias,mod_alias.x[0],mod_alias.x[0],mod_alias.x[0]);
        if ((mod_alias.status() == SS_FAILED) ||
            !mod_alias.x[0].assigned() || (mod_alias.x[0].val() != 0U) ||
            (PropagatorGroup::all.size(mod_alias) != 0U))
          return false;

        SignedSpace failed_rem(2,4);
        dom(failed_rem,failed_rem.x[1],1U);
        signed_rem(failed_rem,failed_rem.x[0],failed_rem.x[0],
                   failed_rem.x[1]);
        if (failed_rem.status() != SS_FAILED)
          return false;

        SignedSpace failed_mod(2,4);
        dom(failed_mod,failed_mod.x[1],1U);
        signed_mod(failed_mod,failed_mod.x[0],failed_mod.x[0],
                   failed_mod.x[1]);
        return failed_mod.status() == SS_FAILED;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar quotient;
          WordVar remainder;
          WordVar modulus;
          SearchSpace(void) : x(*this,2), y(*this,2), quotient(*this,2),
            remainder(*this,2), modulus(*this,2) {
            signed_div(*this,x,y,quotient);
            signed_rem(*this,x,y,remainder);
            signed_mod(*this,x,y,modulus);
            WordVarArgs decision = {x,y};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            quotient.update(*this,s.quotient);
            remainder.update(*this,s.remainder);
            modulus.update(*this,s.modulus);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SearchSpace* solution = dfs.next()) {
          const bool ok = solution->x.assigned() && solution->y.assigned() &&
            solution->quotient.assigned() && solution->remainder.assigned() &&
            solution->modulus.assigned() &&
            (solution->quotient.val() ==
             evaluate(SIGNED_DIV,solution->x.val(),solution->y.val(),3U)) &&
            (solution->remainder.val() ==
             evaluate(SIGNED_REM,solution->x.val(),solution->y.val(),3U)) &&
            (solution->modulus.val() ==
             evaluate(SIGNED_MOD,solution->x.val(),solution->y.val(),3U)) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 16;
      }

    public:
      SignedDivisionLifecycle(void)
        : Base("Word::Arithmetic::SignedDivisionLifecycle") {}
      virtual bool run(void) {
        return partial(SIGNED_DIV) && partial(SIGNED_REM) &&
          partial(SIGNED_MOD) && boolean_parity() && lifecycle() &&
          native_propagation() && self_identities() &&
          search_recomputation();
      }
    };

    class BoundedLifecycle : public Base {
    private:
      struct OracleDomain {
        Gecode::WordValue lo, hi, minimum, maximum;
      };

      static std::vector<OracleDomain>
      oracle_domains(unsigned int width, Gecode::WordDomainType kind) {
        using namespace Gecode;
        const WordValue mask=Gecode::Word::width_mask(width);
        const WordValue middle=WordValue(1) << (width-1);
        std::vector<OracleDomain> candidates = {
          {0U,mask,0U,mask},
          {0U,mask,0U,middle-1U}, {0U,mask,middle,mask},
          {0U,mask,1U,mask-1U},
          {0U,mask,0U,0U}, {0U,mask,1U,1U},
          {0U,mask,mask,mask},
          {0U,mask & ~WordValue(1),0U,mask},
          {1U,mask,0U,mask},
          {0U,mask & ~middle,0U,mask},
          {middle,mask,0U,mask},
          {WordValue(0x5) & mask,WordValue(0x5) & mask,0U,mask}
        };
        std::vector<OracleDomain> rows;
        for (OracleDomain d : candidates) {
          if (!Gecode::Word::synchronize_domain(width,kind,d.lo,d.hi,
                                        d.minimum,d.maximum))
            continue;
          bool duplicate=false;
          for (const OracleDomain& old : rows)
            duplicate |= (d.lo == old.lo) && (d.hi == old.hi) &&
              (d.minimum == old.minimum) && (d.maximum == old.maximum);
          if (!duplicate)
            rows.push_back(d);
        }
        return rows;
      }

      static bool oracle_in(const OracleDomain& d,
                            Gecode::WordDomainType kind,
                            unsigned int width, Gecode::WordValue value) {
        const Gecode::WordValue rank=Gecode::Word::rank(kind,width,value);
        return ((value & d.lo) == d.lo) && ((value & ~d.hi) == 0U) &&
          (rank >= d.minimum) && (rank <= d.maximum);
      }

      class OracleSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        int role[3];
        OracleSpace(unsigned int width, Gecode::WordDomainType kind,
                    const OracleDomain (&d)[3], const int (&map)[3])
          : x(*this,3,width,kind) {
          for (int i=0; i<3; i++) role[i]=map[i];
          for (int i=0; i<3; i++) {
            Gecode::Word::WordView v(x[role[i]]);
            if (Gecode::me_failed(v.narrow_rank_range(
                                  *this,d[i].minimum,d[i].maximum)) ||
                Gecode::me_failed(v.narrow(*this,d[i].lo,d[i].hi)))
              fail();
          }
        }
        OracleSpace(OracleSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          for (int i=0; i<3; i++) role[i]=s.role[i];
        }
        Gecode::WordVar at(int i) const { return x[role[i]]; }
        Gecode::Space* copy(void) { return new OracleSpace(*this); }
      };

      static bool oracle_selected(OracleSpace& s, Op op,
                                  Gecode::WordDomainType kind) {
        using namespace Gecode;
        if (s.failed()) return false;
        if (op == NEG)
          return (kind == WDT_SIGNED) &&
            Gecode::Word::Arithmetic::BoundNeg<
              Gecode::Word::SignedWordView>::numeric_regime(
                Gecode::Word::SignedWordView(s.at(0)));
        if (kind == WDT_UNSIGNED) {
          Gecode::Word::UnsignedWordView x(s.at(0)), y(s.at(1));
          if (op == ADD)
            return Gecode::Word::Arithmetic::BoundArithmetic<
              Gecode::Word::UnsignedWordView,
              Gecode::Word::Arithmetic::BA_ADD>::numeric_regime(x,y);
          if (op == SUB)
            return Gecode::Word::Arithmetic::BoundArithmetic<
              Gecode::Word::UnsignedWordView,
              Gecode::Word::Arithmetic::BA_SUB>::numeric_regime(x,y);
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::UnsignedWordView,
            Gecode::Word::Arithmetic::BA_MULT>::numeric_regime(x,y);
        }
        Gecode::Word::SignedWordView x(s.at(0)), y(s.at(1));
        if (op == ADD)
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::SignedWordView,
            Gecode::Word::Arithmetic::BA_ADD>::numeric_regime(x,y);
        if (op == SUB)
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::SignedWordView,
            Gecode::Word::Arithmetic::BA_SUB>::numeric_regime(x,y);
        return Gecode::Word::Arithmetic::BoundArithmetic<
          Gecode::Word::SignedWordView,
          Gecode::Word::Arithmetic::BA_MULT>::numeric_regime(x,y);
      }

      static bool oracle_terminal_selected(OracleSpace& s,
                                           Op op,
                                           Gecode::WordDomainType kind,
                                           int terminal) {
        using namespace Gecode;
        if (s.failed() || (kind != WDT_UNSIGNED) ||
            ((terminal != 0) && (terminal != 1)))
          return false;
        Gecode::Word::UnsignedWordView x(s.at(0)), y(s.at(1));
        if ((op == ADD) && (terminal == 0))
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::UnsignedWordView,
            Gecode::Word::Arithmetic::BA_ADD,
            Gecode::Word::Arithmetic::BT_CLEAR>::numeric_regime(x,y);
        if (op == ADD)
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::UnsignedWordView,
            Gecode::Word::Arithmetic::BA_ADD,
            Gecode::Word::Arithmetic::BT_SET>::numeric_regime(x,y);
        if (terminal == 0)
          return Gecode::Word::Arithmetic::BoundArithmetic<
            Gecode::Word::UnsignedWordView,
            Gecode::Word::Arithmetic::BA_SUB,
            Gecode::Word::Arithmetic::BT_CLEAR>::numeric_regime(x,y);
        return Gecode::Word::Arithmetic::BoundArithmetic<
          Gecode::Word::UnsignedWordView,
          Gecode::Word::Arithmetic::BA_SUB,
          Gecode::Word::Arithmetic::BT_SET>::numeric_regime(x,y);
      }

      static bool oracle_case(Op op, unsigned int width,
                              Gecode::WordDomainType kind,
                              const OracleDomain (&d)[3],
                              const int (&map)[3], int terminal,
                              unsigned long& cases) {
        using namespace Gecode;
        OracleSpace s(width,kind,d,map);
        if (terminal < 0) {
          if (!oracle_selected(s,op,kind))
            return true;
          if (op == NEG) neg(s,s.at(0),s.at(2));
          else if (op == ADD) add(s,s.at(0),s.at(1),s.at(2));
          else if (op == SUB) sub(s,s.at(0),s.at(1),s.at(2));
          else mult(s,s.at(0),s.at(1),s.at(2));
        } else {
          if (!oracle_terminal_selected(s,op,kind,terminal))
            return false;
          BoolVar flag(s,terminal,terminal);
          if (op == ADD) add(s,s.at(0),s.at(1),s.at(2),flag);
          else sub(s,s.at(0),s.at(1),s.at(2),flag);
        }
        cases++;
        const bool failed=s.status() == SS_FAILED;
        const WordValue mask=Gecode::Word::width_mask(width);
        bool supported=false;
        if (op == NEG) {
          for (WordValue xv=0; xv<=mask; xv++) {
            const WordValue zv=evaluate(NEG,xv,0U,mask);
            if (!oracle_in(d[0],kind,width,xv) ||
                !oracle_in(d[2],kind,width,zv) ||
                ((map[0] == map[2]) && (xv != zv)))
              continue;
            supported=true;
            if (failed || !s.at(0).in(xv) || !s.at(2).in(zv)) {
              ::Test::olog << "oracle pruned negation support width="
                           << width << " kind=" << kind << " tuple="
                           << xv << ',' << zv << std::endl;
              return false;
            }
          }
          return !failed || !supported;
        }
        for (WordValue xv=0; xv<=mask; xv++)
          for (WordValue yv=0; yv<=mask; yv++) {
              const WordValue zv=evaluate(op,xv,yv,mask);
              const bool terminal_ok = terminal < 0 ? true : op == ADD ?
                terminal == static_cast<int>(xv > mask-yv) :
                terminal == static_cast<int>(xv < yv);
              if (!terminal_ok ||
                  !oracle_in(d[0],kind,width,xv) ||
                  !oracle_in(d[1],kind,width,yv) ||
                  !oracle_in(d[2],kind,width,zv) ||
                  ((map[0] == map[1]) && (xv != yv)) ||
                  ((map[0] == map[2]) && (xv != zv)) ||
                  ((map[1] == map[2]) && (yv != zv)))
                continue;
              supported=true;
              if (failed || !s.at(0).in(xv) || !s.at(1).in(yv) ||
                  !s.at(2).in(zv)) {
                ::Test::olog << "oracle pruned support op=" << op
                             << " width=" << width << " kind=" << kind
                             << " tuple=" << xv << ',' << yv << ',' << zv
                             << std::endl;
                return false;
              }
          }
        return !failed || !supported;
      }

      static bool negation_oracle(unsigned int width,
                                  Gecode::WordDomainType kind,
                                  const std::vector<OracleDomain>& rows,
                                  unsigned long& cases) {
        using namespace Gecode;
        const int distinct[3]={0,1,2};
        const int aliased[3]={0,1,0};
        for (const OracleDomain& x : rows)
          for (const OracleDomain& z : rows) {
            const OracleDomain d[3]={x,rows[0],z};
            if (kind == WDT_UNSIGNED) {
              OracleSpace s(width,kind,d,distinct);
              if (oracle_selected(s,NEG,kind))
                return false;
            } else if (!oracle_case(NEG,width,kind,d,distinct,-1,cases)) {
              return false;
            }
          }
        if (kind == WDT_SIGNED)
          for (const OracleDomain& row : rows) {
            const OracleDomain d[3]={row,rows[0],row};
            if (!oracle_case(NEG,width,kind,d,aliased,-1,cases))
              return false;
          }
        return true;
      }

      static bool partial_domain_oracle(void) {
        using namespace Gecode;
        unsigned long cases=0;
        const int distinct[3]={0,1,2};
        const int aliases[4][3]={{0,0,2},{0,1,0},{0,1,1},{0,0,0}};
        for (unsigned int width : {2U,3U,4U})
          for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED}) {
            const std::vector<OracleDomain> rows=oracle_domains(width,kind);
            for (Op op : {ADD,SUB,MULT}) {
              for (const OracleDomain& x : rows)
                for (const OracleDomain& y : rows)
                  for (const OracleDomain& z : rows) {
                    const OracleDomain d[3]={x,y,z};
                    if (!oracle_case(op,width,kind,d,distinct,-1,cases))
                      return false;
                  }
              for (const auto& map : aliases)
                for (const OracleDomain& row : rows)
                  for (unsigned int other=0;
                       other<((map[0] == map[1]) &&
                              (map[1] == map[2]) ? 1U : 3U); other++) {
                    const OracleDomain d[3]={
                      map[0] == map[1] || map[0] == map[2] ? row : rows[other],
                      map[1] == map[0] || map[1] == map[2] ? row : rows[other],
                      map[2] == map[0] || map[2] == map[1] ? row : rows[other]
                    };
                    if (!oracle_case(op,width,kind,d,map,-1,cases))
                      return false;
                  }
            }
            if (!negation_oracle(width,kind,rows,cases))
              return false;
            if (kind == WDT_UNSIGNED) {
              const unsigned int terminal_rows[6]={0U,1U,2U,4U,5U,6U};
              for (Op op : {ADD,SUB})
                for (int terminal : {0,1})
                  for (unsigned int i : terminal_rows)
                    for (unsigned int j : terminal_rows)
                      for (unsigned int k : terminal_rows) {
                        const OracleDomain d[3]={rows[i],rows[j],rows[k]};
                        if (!oracle_case(op,width,kind,d,distinct,terminal,cases))
                          return false;
                      }
            }
          }
        // Keep changes to row normalization or actor selection intentional.
        return cases == 7463U;
      }

      static bool division_truth(void) {
        using namespace Gecode;
        class D : public Space {
        public:
          WordVar x,y,z,q,r;
          D(unsigned int width, WordDomainType kind, WordValue xv,
            WordValue yv, Op op)
            : x(*this,width,kind,xv,xv), y(*this,width,kind,yv,yv),
              z(*this,width,kind), q(*this,width,kind), r(*this,width,kind) {
            switch (op) {
            case DIV: div(*this,x,y,z); break;
            case MOD: mod(*this,x,y,z); break;
            case SIGNED_DIV: signed_div(*this,x,y,z); break;
            case SIGNED_REM: signed_rem(*this,x,y,z); break;
            case SIGNED_MOD: signed_mod(*this,x,y,z); break;
            default: assert(false);
            }
          }
          D(unsigned int width, WordValue xv, WordValue yv)
            : x(*this,width,WDT_UNSIGNED,xv,xv),
              y(*this,width,WDT_UNSIGNED,yv,yv),
              z(*this,width,WDT_UNSIGNED), q(*this,width,WDT_UNSIGNED),
              r(*this,width,WDT_UNSIGNED) { divmod(*this,x,y,q,r); }
          D(D& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
            q.update(*this,s.q); r.update(*this,s.r);
          }
          Space* copy(void) { return new D(*this); }
        };
        for (unsigned int width=1; width<=4; width++) {
          const WordValue mask=Gecode::Word::width_mask(width);
          for (WordValue xv=0; xv<=mask; xv++)
            for (WordValue yv=0; yv<=mask; yv++) {
              for (Op op : {DIV,MOD,SIGNED_DIV,SIGNED_REM,SIGNED_MOD}) {
                const WordDomainType kind=(op == DIV || op == MOD) ?
                  WDT_UNSIGNED : WDT_SIGNED;
                D d(width,kind,xv,yv,op);
                const WordValue expected=Arithmetic::evaluate(op,xv,yv,mask);
                if ((d.status() == SS_FAILED) || !d.z.assigned() ||
                    (d.z.val() != expected) ||
                    (PropagatorGroup::all.size(d) != 0U))
                  return false;
              }
              D both(width,xv,yv);
              const WordValue expected_q=(yv == 0) ? mask : xv/yv;
              const WordValue expected_r=(yv == 0) ? xv : xv%yv;
              if ((both.status() == SS_FAILED) || !both.q.assigned() ||
                  !both.r.assigned() || (both.q.val() != expected_q) ||
                  (both.r.val() != expected_r) ||
                  (PropagatorGroup::all.size(both) != 0U))
                return false;
            }
        }
        return true;
      }

      static bool division_propagation(void) {
        using namespace Gecode;
        class U : public Space {
        public:
          WordVar x,y,q,r;
          U(void) : x(*this,6,WDT_UNSIGNED,20U,40U),
            y(*this,6,WDT_UNSIGNED,4U,5U),
            q(*this,6,WDT_UNSIGNED), r(*this,6,WDT_UNSIGNED) {
            divmod(*this,x,y,q,r);
          }
          U(U& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
            q.update(*this,s.q); r.update(*this,s.r);
          }
          Space* copy(void) { return new U(*this); }
        };
        U u;
        if ((u.status() == SS_FAILED) || (u.q.minimum() != 4U) ||
            (u.q.maximum() != 10U) || (u.r.maximum() != 4U))
          return false;
        U* clone=static_cast<U*>(u.clone());
        dom(*clone,clone->x,37U); dom(*clone,clone->y,5U);
        const bool clone_ok=(clone->status() != SS_FAILED) &&
          clone->q.assigned() && (clone->q.val() == 7U) &&
          clone->r.assigned() && (clone->r.val() == 2U) &&
          !u.q.assigned() && (PropagatorGroup::all.size(*clone) == 0U);
        delete clone;
        if (!clone_ok) return false;

        class S : public Space {
        public:
          WordVar x,y,q,rem,modulus;
          S(void) : x(*this,4,WDT_SIGNED,8U,12U),
            y(*this,4,WDT_SIGNED,2U,2U), q(*this,4,WDT_SIGNED),
            rem(*this,4,WDT_SIGNED), modulus(*this,4,WDT_SIGNED) {
            signed_div(*this,x,y,q); signed_rem(*this,x,y,rem);
            signed_mod(*this,x,y,modulus);
          }
          S(S& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); q.update(*this,s.q);
            rem.update(*this,s.rem); modulus.update(*this,s.modulus);
          }
          Space* copy(void) { return new S(*this); }
        };
        S s;
        if ((s.status() == SS_FAILED) || (s.q.minimum() != 12U) ||
            (s.q.maximum() != 14U) || (s.rem.minimum() != 15U) ||
            (s.rem.maximum() != 0U) || (s.modulus.minimum() != 0U) ||
            (s.modulus.maximum() != 1U))
          return false;

        class Exceptional : public Space {
        public:
          WordVar x,y,q,r,m;
          Exceptional(unsigned int width, WordValue xv, WordValue yv)
            : x(*this,width,WDT_SIGNED,xv,xv),
              y(*this,width,WDT_SIGNED,yv,yv), q(*this,width,WDT_SIGNED),
              r(*this,width,WDT_SIGNED), m(*this,width,WDT_SIGNED) {
            signed_div(*this,x,y,q); signed_rem(*this,x,y,r);
            signed_mod(*this,x,y,m);
          }
          Exceptional(Exceptional& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); q.update(*this,s.q);
            r.update(*this,s.r); m.update(*this,s.m);
          }
          Space* copy(void) { return new Exceptional(*this); }
        };
        for (unsigned int width : {1U,64U}) {
          const WordValue minimum=WordValue(1) << (width-1);
          const WordValue minus_one=Gecode::Word::width_mask(width);
          Exceptional e(width,minimum,minus_one);
          if ((e.status() == SS_FAILED) || !e.q.assigned() ||
              (e.q.val() != minimum) || !e.r.assigned() ||
              (e.r.val() != 0U) || !e.m.assigned() || (e.m.val() != 0U))
            return false;
        }

        class Constant : public Space {
        public:
          WordVar x,q,r;
          Constant(void) : x(*this,6,WDT_UNSIGNED,20U,40U),
            q(*this,6,WDT_UNSIGNED), r(*this,6,WDT_UNSIGNED) {
            div(*this,x,6,5U,q); mod(*this,x,6,5U,r);
          }
          Constant(Constant& s) : Space(s) {
            x.update(*this,s.x); q.update(*this,s.q); r.update(*this,s.r);
          }
          Space* copy(void) { return new Constant(*this); }
        };
        Constant constant;
        if ((constant.status() == SS_FAILED) ||
            (constant.q.minimum() != 4U) || (constant.q.maximum() != 8U) ||
            (constant.r.maximum() != 4U))
          return false;

        class Alias : public Space {
        public:
          WordVar x,y;
          Alias(bool quotient) : x(*this,4,WDT_UNSIGNED,1U,7U),
            y(*this,4,WDT_UNSIGNED,1U,7U) {
            if (quotient) div(*this,x,y,x); else mod(*this,x,y,x);
          }
          Alias(Alias& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          Space* copy(void) { return new Alias(*this); }
        };
        Alias div_alias(true); dom(div_alias,div_alias.x,2U);
        dom(div_alias,div_alias.y,1U);
        if ((div_alias.status() == SS_FAILED) ||
            !div_alias.x.assigned() || (div_alias.x.val() != 2U))
          return false;
        Alias mod_alias(false); dom(mod_alias,mod_alias.x,2U);
        dom(mod_alias,mod_alias.y,3U);
        if ((mod_alias.status() == SS_FAILED) ||
            !mod_alias.x.assigned() || (mod_alias.x.val() != 2U))
          return false;

        class Failure : public Space {
        public:
          WordVar x,y,q;
          Failure(void) : x(*this,4,WDT_UNSIGNED,10U,10U),
            y(*this,4,WDT_UNSIGNED,3U,3U),
            q(*this,4,WDT_UNSIGNED,2U,2U) { div(*this,x,y,q); }
          Failure(Failure& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); q.update(*this,s.q);
          }
          Space* copy(void) { return new Failure(*this); }
        };
        Failure failure;
        if (failure.status() != SS_FAILED) return false;

        class Inverse : public Space {
        public:
          WordVar ux,uy,uq,ua,ub,ur,sx,sy,sq,sb,sr,sm;
          Inverse(void)
            : ux(*this,6,WDT_UNSIGNED),
              uy(*this,6,WDT_UNSIGNED,4U,5U),
              uq(*this,6,WDT_UNSIGNED,8U,8U),
              ua(*this,4,WDT_UNSIGNED), ub(*this,4,WDT_UNSIGNED,4U,4U),
              ur(*this,4,WDT_UNSIGNED,2U,2U),
              sx(*this,4,WDT_SIGNED), sy(*this,4,WDT_SIGNED,3U,3U),
              sq(*this,4,WDT_SIGNED,15U,15U),
              sb(*this,4,WDT_SIGNED,1U,7U),
              sr(*this,4,WDT_SIGNED,3U,3U),
              sm(*this,4,WDT_SIGNED,3U,3U) {
            div(*this,ux,uy,uq); mod(*this,ua,ub,ur);
            signed_div(*this,sx,sy,sq);
            signed_rem(*this,WordVar(*this,4,WDT_SIGNED),sb,sr);
            signed_mod(*this,WordVar(*this,4,WDT_SIGNED),sb,sm);
          }
          Inverse(Inverse& s) : Space(s) {
            ux.update(*this,s.ux); uy.update(*this,s.uy);
            uq.update(*this,s.uq); ua.update(*this,s.ua);
            ub.update(*this,s.ub); ur.update(*this,s.ur);
            sx.update(*this,s.sx); sy.update(*this,s.sy);
            sq.update(*this,s.sq); sb.update(*this,s.sb);
            sr.update(*this,s.sr); sm.update(*this,s.sm);
          }
          Space* copy(void) { return new Inverse(*this); }
        };
        Inverse inverse;
        if ((inverse.status() == SS_FAILED) ||
            (inverse.ux.minimum() != 32U) ||
            (inverse.ux.maximum() != 44U) ||
            (inverse.ua.lo() != 2U) || (inverse.ua.hi() != 14U) ||
            (inverse.sx.minimum() != 11U) ||
            (inverse.sx.maximum() != 13U) ||
            (inverse.sb.minimum() != 4U))
          return false;

        class Fallback : public Space {
        public:
          WordVar cube,bounded,result,sx,sy,sq;
          Fallback(void)
            : cube(*this,3,6U,6U),
              bounded(*this,3,WDT_UNSIGNED,2U,2U),
              result(*this,3,WDT_UNSIGNED),
              sx(*this,3,WDT_SIGNED), sy(*this,3,WDT_SIGNED),
              sq(*this,3,WDT_SIGNED) {
            div(*this,cube,bounded,result);
            signed_div(*this,sx,sy,sq);
          }
          Fallback(Fallback& s) : Space(s) {
            cube.update(*this,s.cube); bounded.update(*this,s.bounded);
            result.update(*this,s.result); sx.update(*this,s.sx);
            sy.update(*this,s.sy); sq.update(*this,s.sq);
          }
          Space* copy(void) { return new Fallback(*this); }
        };
        Fallback fallback;
        if ((fallback.status() == SS_FAILED) ||
            !fallback.result.assigned() || (fallback.result.val() != 3U))
          return false;
        Fallback* fallback_clone=static_cast<Fallback*>(fallback.clone());
        dom(*fallback_clone,fallback_clone->sx,4U);
        dom(*fallback_clone,fallback_clone->sy,7U);
        const bool fallback_ok=(fallback_clone->status() != SS_FAILED) &&
          fallback_clone->sq.assigned() && (fallback_clone->sq.val() == 4U);
        delete fallback_clone;
        if (!fallback_ok) return false;

        class UnsignedSelf : public Space {
        public:
          WordVar x,dq,mr,bq,br;
          UnsignedSelf(unsigned int width, WordValue minimum,
                       WordValue maximum)
            : x(*this,width,WDT_UNSIGNED,minimum,maximum),
              dq(*this,width,WDT_UNSIGNED), mr(*this,width,WDT_UNSIGNED),
              bq(*this,width,WDT_UNSIGNED), br(*this,width,WDT_UNSIGNED) {
            div(*this,x,x,dq); mod(*this,x,x,mr);
            divmod(*this,x,x,bq,br);
          }
          UnsignedSelf(UnsignedSelf& s) : Space(s) {
            x.update(*this,s.x); dq.update(*this,s.dq);
            mr.update(*this,s.mr); bq.update(*this,s.bq);
            br.update(*this,s.br);
          }
          Space* copy(void) { return new UnsignedSelf(*this); }
        };
        class SignedSelf : public Space {
        public:
          WordVar x,q,r,m;
          SignedSelf(unsigned int width, WordValue minimum,
                     WordValue maximum)
            : x(*this,width,WDT_SIGNED,minimum,maximum),
              q(*this,width,WDT_SIGNED), r(*this,width,WDT_SIGNED),
              m(*this,width,WDT_SIGNED) {
            signed_div(*this,x,x,q); signed_rem(*this,x,x,r);
            signed_mod(*this,x,x,m);
          }
          SignedSelf(SignedSelf& s) : Space(s) {
            x.update(*this,s.x); q.update(*this,s.q);
            r.update(*this,s.r); m.update(*this,s.m);
          }
          Space* copy(void) { return new SignedSelf(*this); }
        };
        for (unsigned int width : {1U,64U}) {
          const WordValue mask=Gecode::Word::width_mask(width);
          UnsignedSelf un(width,1U,(width == 1) ? 1U : 3U);
          if ((un.status() == SS_FAILED) || !un.dq.assigned() ||
              (un.dq.val() != 1U) || !un.mr.assigned() ||
              (un.mr.val() != 0U) || !un.bq.assigned() ||
              (un.bq.val() != 1U) || !un.br.assigned() ||
              (un.br.val() != 0U) ||
              (PropagatorGroup::all.size(un) != 0U))
            return false;
          UnsignedSelf uz(width,0U,0U);
          if ((uz.status() == SS_FAILED) || !uz.dq.assigned() ||
              (uz.dq.val() != mask) || !uz.mr.assigned() ||
              (uz.mr.val() != 0U) || !uz.bq.assigned() ||
              (uz.bq.val() != mask) || !uz.br.assigned() ||
              (uz.br.val() != 0U) ||
              (PropagatorGroup::all.size(uz) != 0U))
            return false;
          SignedSelf sn(width,mask-((width == 1) ? 0U : 2U),mask);
          if ((sn.status() == SS_FAILED) || !sn.q.assigned() ||
              (sn.q.val() != 1U) || !sn.r.assigned() ||
              (sn.r.val() != 0U) || !sn.m.assigned() ||
              (sn.m.val() != 0U) ||
              (PropagatorGroup::all.size(sn) != 0U))
            return false;
          SignedSelf sz(width,0U,0U);
          if ((sz.status() == SS_FAILED) || !sz.q.assigned() ||
              (sz.q.val() != mask) || !sz.r.assigned() ||
              (sz.r.val() != 0U) || !sz.m.assigned() ||
              (sz.m.val() != 0U) ||
              (PropagatorGroup::all.size(sz) != 0U))
            return false;
        }
        UnsignedSelf possible(3,0U,7U);
        if ((possible.status() == SS_FAILED) ||
            !possible.mr.assigned() || (possible.mr.val() != 0U) ||
            !possible.br.assigned() || (possible.br.val() != 0U) ||
            (PropagatorGroup::all.size(possible) != 2U))
          return false;
        UnsignedSelf* possible_clone=
          static_cast<UnsignedSelf*>(possible.clone());
        dom(*possible_clone,possible_clone->x,0U);
        const bool possible_ok=(possible_clone->status() != SS_FAILED) &&
          possible_clone->dq.assigned() && (possible_clone->dq.val() == 7U) &&
          possible_clone->bq.assigned() && (possible_clone->bq.val() == 7U) &&
          (PropagatorGroup::all.size(*possible_clone) == 0U);
        delete possible_clone;
        if (!possible_ok) return false;
        return true;
      }

      static bool division_replay(void) {
        using namespace Gecode;
        class R : public Space {
        public:
          WordVar x,y,q,r;
          R(void) : x(*this,3,WDT_UNSIGNED),
            y(*this,3,WDT_UNSIGNED,1U,7U), q(*this,3,WDT_UNSIGNED),
            r(*this,3,WDT_UNSIGNED) {
            divmod(*this,x,y,q,r);
            WordVarArgs decision={x,y};
            branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
          }
          R(R& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
            q.update(*this,s.q); r.update(*this,s.r);
          }
          Space* copy(void) { return new R(*this); }
        };
        Search::Options o; o.c_d=1;
        R* root=new R; DFS<R> dfs(root,o); delete root;
        unsigned int solutions=0;
        while (R* s=dfs.next()) {
          const bool ok=s->x.assigned() && s->y.assigned() &&
            s->q.assigned() && s->r.assigned() &&
            (s->q.val() == s->x.val()/s->y.val()) &&
            (s->r.val() == s->x.val()%s->y.val()) &&
            (PropagatorGroup::all.size(*s) == 0U);
          delete s;
          if (!ok) return false;
          solutions++;
        }
        if (solutions != 56U) return false;
        class SelfReplay : public Space {
        public:
          WordVar x,q,r;
          SelfReplay(void) : x(*this,3,WDT_UNSIGNED,1U,7U),
            q(*this,3,WDT_UNSIGNED), r(*this,3,WDT_UNSIGNED) {
            divmod(*this,x,x,q,r);
            branch(*this,x,WORD_VAL_SPLIT_MIN());
          }
          SelfReplay(SelfReplay& s) : Space(s) {
            x.update(*this,s.x); q.update(*this,s.q); r.update(*this,s.r);
          }
          Space* copy(void) { return new SelfReplay(*this); }
        };
        SelfReplay* self_root=new SelfReplay;
        DFS<SelfReplay> self_dfs(self_root,o); delete self_root;
        unsigned int self_solutions=0;
        while (SelfReplay* s=self_dfs.next()) {
          const bool ok=s->x.assigned() && s->q.assigned() &&
            (s->q.val() == 1U) && s->r.assigned() &&
            (s->r.val() == 0U) &&
            (PropagatorGroup::all.size(*s) == 0U);
          delete s;
          if (!ok) return false;
          self_solutions++;
        }
        return self_solutions == 7U;
      }

      static bool propagation(void) {
        using namespace Gecode;
        class UAdd : public Space {
        public:
          WordVar x,y,z;
          UAdd(void)
            : x(*this,4,WDT_UNSIGNED,2,4),
              y(*this,4,WDT_UNSIGNED,3,5),
              z(*this,4,WDT_UNSIGNED) { add(*this,x,y,z); }
          UAdd(UAdd& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          }
          Space* copy(void) { return new UAdd(*this); }
        };
        UAdd ua;
        if ((ua.status() == SS_FAILED) || (ua.z.minimum() != 5) ||
            (ua.z.maximum() != 9))
          return false;
        UAdd* clone=static_cast<UAdd*>(ua.clone());
        dom(*clone,clone->x,4U); dom(*clone,clone->y,5U);
        const bool clone_ok=(clone->status() != SS_FAILED) &&
          clone->z.assigned() && (clone->z.val() == 9U) &&
          !ua.z.assigned() && (PropagatorGroup::all.size(*clone) == 0U);
        delete clone;
        if (!clone_ok) return false;

        class SAdd : public Space {
        public:
          WordVar x,y,z;
          SAdd(void)
            : x(*this,4,WDT_SIGNED,13U,15U),
              y(*this,4,WDT_SIGNED,1U,2U),
              z(*this,4,WDT_SIGNED) { add(*this,x,y,z); }
          SAdd(SAdd& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          }
          Space* copy(void) { return new SAdd(*this); }
        };
        SAdd sa;
        if ((sa.status() == SS_FAILED) || (sa.z.minimum() != 14U) ||
            (sa.z.maximum() != 1U))
          return false;

        class Ops : public Space {
        public:
          WordVar sx,sy,sz,nx,nz,ux,uy,uz;
          Ops(void)
            : sx(*this,6,WDT_SIGNED,61U,63U),
              sy(*this,6,WDT_SIGNED,2U,4U),
              sz(*this,6,WDT_SIGNED),
              nx(*this,4,WDT_SIGNED,13U,15U),
              nz(*this,4,WDT_SIGNED),
              ux(*this,6,WDT_UNSIGNED,2U,4U),
              uy(*this,6,WDT_UNSIGNED,3U,5U),
              uz(*this,6,WDT_UNSIGNED) {
            mult(*this,sx,sy,sz); neg(*this,nx,nz); mult(*this,ux,uy,uz);
          }
          Ops(Ops& s) : Space(s) {
            sx.update(*this,s.sx); sy.update(*this,s.sy);
            sz.update(*this,s.sz); nx.update(*this,s.nx);
            nz.update(*this,s.nz); ux.update(*this,s.ux);
            uy.update(*this,s.uy); uz.update(*this,s.uz);
          }
          Space* copy(void) { return new Ops(*this); }
        };
        Ops ops;
        if ((ops.status() == SS_FAILED) ||
            (ops.sz.minimum() != 52U) || (ops.sz.maximum() != 62U) ||
            (ops.nz.minimum() != 1U) || (ops.nz.maximum() != 3U) ||
            (ops.uz.minimum() != 6U) || (ops.uz.maximum() != 20U))
          return false;

        class USub : public Space {
        public:
          WordVar x,y,z;
          USub(void) : x(*this,4,WDT_UNSIGNED,8U,12U),
            y(*this,4,WDT_UNSIGNED,2U,4U), z(*this,4,WDT_UNSIGNED) {
            sub(*this,x,y,z);
          }
          USub(USub& s) : Space(s) {
            x.update(*this,s.x);y.update(*this,s.y);z.update(*this,s.z);
          }
          Space* copy(void) { return new USub(*this); }
        };
        USub us;
        if ((us.status() == SS_FAILED) || (us.z.minimum() != 4U) ||
            (us.z.maximum() != 10U))
          return false;

        class Backward : public Space {
        public:
          WordVar ux,uy,uz,sx,sy,sz,dx,dy,dz,alias,twice;
          WordVar signed_alias,signed_twice;
          Backward(void)
            : ux(*this,5,WDT_UNSIGNED,0U,10U),
              uy(*this,5,WDT_UNSIGNED,0U,10U),
              uz(*this,5,WDT_UNSIGNED,15U,15U),
              sx(*this,5,WDT_SIGNED,0U,5U),
              sy(*this,5,WDT_SIGNED,0U,5U),
              sz(*this,5,WDT_SIGNED,8U,8U),
              dx(*this,5,WDT_SIGNED,5U,10U),
              dy(*this,5,WDT_SIGNED,0U,5U),
              dz(*this,5,WDT_SIGNED,2U,2U),
              alias(*this,5,WDT_UNSIGNED,0U,10U),
              twice(*this,5,WDT_UNSIGNED,10U,10U),
              signed_alias(*this,5,WDT_SIGNED,0U,5U),
              signed_twice(*this,5,WDT_SIGNED,8U,8U) {
            add(*this,ux,uy,uz);
            add(*this,sx,sy,sz);
            sub(*this,dx,dy,dz);
            add(*this,alias,alias,twice);
            add(*this,signed_alias,signed_alias,signed_twice);
          }
          Backward(Backward& s) : Space(s) {
            ux.update(*this,s.ux); uy.update(*this,s.uy);
            uz.update(*this,s.uz); sx.update(*this,s.sx);
            sy.update(*this,s.sy); sz.update(*this,s.sz);
            dx.update(*this,s.dx); dy.update(*this,s.dy);
            dz.update(*this,s.dz); alias.update(*this,s.alias);
            twice.update(*this,s.twice);
            signed_alias.update(*this,s.signed_alias);
            signed_twice.update(*this,s.signed_twice);
          }
          Space* copy(void) { return new Backward(*this); }
        };
        Backward backward;
        return (backward.status() != SS_FAILED) &&
          (backward.ux.minimum() == 5U) &&
          (backward.uy.minimum() == 5U) &&
          (backward.sx.minimum() == 3U) &&
          (backward.sy.minimum() == 3U) &&
          (backward.dx.maximum() == 7U) &&
          (backward.dy.minimum() == 3U) &&
          backward.alias.assigned() && (backward.alias.val() == 5U) &&
          backward.signed_alias.assigned() &&
          (backward.signed_alias.val() == 4U);
      }

      static bool boundaries_aliases(void) {
        using namespace Gecode;
        class ActiveAlias : public Space {
        public:
          WordVar x,z;
          ActiveAlias(void) : x(*this,4,WDT_UNSIGNED,1U,3U),
            z(*this,4,WDT_UNSIGNED) { add(*this,x,x,z); }
          ActiveAlias(ActiveAlias& s) : Space(s) {
            x.update(*this,s.x); z.update(*this,s.z);
          }
          Space* copy(void) { return new ActiveAlias(*this); }
        };
        ActiveAlias active_alias;
        if ((active_alias.status() == SS_FAILED) ||
            (active_alias.z.minimum() != 2U) ||
            (active_alias.z.maximum() != 6U))
          return false;

        class ActiveResultAlias : public Space {
        public:
          WordVar x,y;
          ActiveResultAlias(void) : x(*this,4,WDT_UNSIGNED,1U,7U),
            y(*this,4,WDT_UNSIGNED,0U,0U) { sub(*this,x,y,x); }
          ActiveResultAlias(ActiveResultAlias& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
          }
          Space* copy(void) { return new ActiveResultAlias(*this); }
        };
        ActiveResultAlias active_result_alias;
        if (active_result_alias.status() == SS_FAILED)
          return false;

        class ActiveWide : public Space {
        public:
          WordVar x,y,z;
          ActiveWide(void)
            : x(*this,64,WDT_UNSIGNED,WordValue(1) << 63,
                (WordValue(1) << 63)+2U),
              y(*this,64,WDT_UNSIGNED,1U,1U),
              z(*this,64,WDT_UNSIGNED) { add(*this,x,y,z); }
          ActiveWide(ActiveWide& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          }
          Space* copy(void) { return new ActiveWide(*this); }
        };
        ActiveWide active_wide;
        if ((active_wide.status() == SS_FAILED) ||
            (active_wide.z.minimum() != (WordValue(1) << 63)+1U) ||
            (active_wide.z.maximum() != (WordValue(1) << 63)+3U))
          return false;

        class SignedExtreme : public Space {
        public:
          WordVar x,y,z;
          SignedExtreme(void) : x(*this,4,WDT_SIGNED,8U,8U),
            y(*this,4,WDT_SIGNED,1U,1U), z(*this,4,WDT_SIGNED) {
            add(*this,x,y,z);
          }
          SignedExtreme(SignedExtreme& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          }
          Space* copy(void) { return new SignedExtreme(*this); }
        };
        SignedExtreme signed_extreme;
        if ((signed_extreme.status() == SS_FAILED) ||
            !signed_extreme.z.assigned() || (signed_extreme.z.val() != 9U))
          return false;

        class B : public Space {
        public:
          WordVar x,y,z;
          B(unsigned int width, WordDomainType kind)
            : x(*this,width,kind), y(*this,width,kind), z(*this,width,kind) {}
          B(B& s) : Space(s) {
            x.update(*this,s.x);y.update(*this,s.y);z.update(*this,s.z);
          }
          Space* copy(void) { return new B(*this); }
        };
        B alias(4,WDT_UNSIGNED);
        add(alias,alias.x,alias.x,alias.z);
        dom(alias,alias.x,3U);
        if ((alias.status() == SS_FAILED) || !alias.z.assigned() ||
            (alias.z.val() != 6U))
          return false;
        B result_alias(4,WDT_UNSIGNED);
        sub(result_alias,result_alias.x,result_alias.y,result_alias.x);
        dom(result_alias,result_alias.y,0U);
        if (result_alias.status() == SS_FAILED)
          return false;
        B one(1,WDT_SIGNED);
        neg(one,one.x,one.z); dom(one,one.x,1U);
        if ((one.status() == SS_FAILED) || !one.z.assigned() ||
            (one.z.val() != 1U))
          return false;
        B wide(64,WDT_UNSIGNED);
        add(wide,wide.x,wide.y,wide.z);
        dom(wide,wide.x,WordValue(1) << 63);
        dom(wide,wide.y,WordValue(1));
        if ((wide.status() == SS_FAILED) || !wide.z.assigned() ||
            (wide.z.val() != ((WordValue(1) << 63)+1)))
          return false;
        B wrap(4,WDT_UNSIGNED);
        add(wrap,wrap.x,wrap.y,wrap.z);
        dom(wrap,wrap.x,15U); dom(wrap,wrap.y,1U);
        return (wrap.status() != SS_FAILED) && wrap.z.assigned() &&
          (wrap.z.val() == 0U);
      }

      static bool replay(void) {
        using namespace Gecode;
        class S : public Space {
        public:
          WordVar x,y,z;
          S(WordDomainType kind)
            : x(*this,3,kind), y(*this,3,kind), z(*this,3,kind) {
            add(*this,x,y,z);
            WordVarArgs a={x,y};
            branch(*this,a,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
          }
          S(S& s) : Space(s) {
            x.update(*this,s.x);y.update(*this,s.y);z.update(*this,s.z);
          }
          Space* copy(void) { return new S(*this); }
        };
        for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED}) {
          Search::Options o; o.c_d=1;
          S* root=new S(kind);
          DFS<S> dfs(root,o);
          delete root;
          unsigned int solutions=0;
          while (S* s=dfs.next()) {
            const bool ok=s->x.assigned() && s->y.assigned() &&
              s->z.assigned() && (s->z.val() == ((s->x.val()+s->y.val())&7U)) &&
              (PropagatorGroup::all.size(*s) == 0U);
            delete s;
            if (!ok) return false;
            solutions++;
          }
          if (solutions != 64U) return false;
        }
        return true;
      }

      static bool staged_propagation(void) {
        using namespace Gecode;
        class S : public Space {
        public:
          WordVar x,y,z;
          S(void) : x(*this,8,WDT_UNSIGNED,8U,15U),
            y(*this,8,WDT_UNSIGNED,9U,9U),
            z(*this,8,WDT_UNSIGNED) {
            mult(*this,x,y,z);
          }
          S(S& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z);
          }
          Space* copy(void) { return new S(*this); }
        };

        // A bound-only update that creates no new cube bits stays in the
        // cheap numeric phase.
        S bounds_only;
        if (bounds_only.status() == SS_FAILED) return false;
        Gecode::Word::UnsignedWordView bx(bounds_only.x);
        if (bx.narrow_range(bounds_only,9U,15U) !=
            Gecode::Word::ME_WORD_BND)
          return false;
        StatusStatistics bounds_statistics;
        if ((bounds_only.status(bounds_statistics) == SS_FAILED) ||
            (bounds_statistics.propagate != 1U) ||
            (bounds_only.z.minimum() != 81U) ||
            (bounds_only.z.maximum() != 135U) ||
            (bounds_only.z.lo() != 0U) ||
            (bounds_only.z.hi() != 255U))
          return false;

        // Here the numeric phase fixes the high result bits. It requests one
        // separately costed cube phase through ES_NOFIX_PARTIAL.
        S staged;
        if (staged.status() == SS_FAILED) return false;
        Gecode::Word::UnsignedWordView sx(staged.x);
        if (sx.narrow_range(staged,8U,14U) !=
            Gecode::Word::ME_WORD_BND)
          return false;
        StatusStatistics staged_statistics;
        return (staged.status(staged_statistics) != SS_FAILED) &&
          (staged_statistics.propagate == 2U) &&
          (staged.z.minimum() == 72U) &&
          (staged.z.maximum() == 126U) &&
          (staged.z.lo() == 64U) && (staged.z.hi() == 127U);
      }

      static bool nary_propagation(void) {
        using namespace Gecode;
        class Nary : public Space {
        public:
          WordVarArray x;
          WordVar z;
          Nary(WordDomainType kind, bool alias=false)
            : x(*this,alias ? 2 : 3,5,kind,0U,10U),
              z(*this,5,kind,alias ? 10U : 15U,alias ? 10U : 15U) {
            WordVarArgs a;
            if (alias) a << x[0] << x[0];
            else a << x[0] << x[1] << x[2];
            add(*this,a,z);
          }
          Nary(Nary& s) : Space(s) { x.update(*this,s.x); z.update(*this,s.z); }
          Space* copy(void) { return new Nary(*this); }
        };
        Nary u(WDT_UNSIGNED);
        if ((u.status() == SS_FAILED) || (u.x[0].minimum() != 0U) ||
            (u.x[0].maximum() != 10U)) return false;
        dom(u,u.x[1],10U); dom(u,u.x[2],0U);
        if ((u.status() == SS_FAILED) || (u.x[0].minimum() != 5U) ||
            (u.x[0].maximum() != 5U)) return false;
        Nary a(WDT_UNSIGNED,true);
        if (a.status() == SS_FAILED) return false;
        class Signed : public Space {
        public:
          WordVarArray x; WordVar z;
          Signed(void) : x(*this,3,5,WDT_SIGNED,30U,2U),
            z(*this,5,WDT_SIGNED,29U,3U) {
            WordVarArgs a={x[0],x[1],x[2]}; add(*this,a,z);
          }
          Signed(Signed& s) : Space(s) { x.update(*this,s.x); z.update(*this,s.z); }
          Space* copy(void) { return new Signed(*this); }
        };
        Signed s;
        if (s.status() == SS_FAILED) return false;
        class Fallback : public Space {
        public:
          WordVarArray x; WordVar z;
          Fallback(void) : x(*this,3,3,WDT_UNSIGNED,4U,7U),
            z(*this,3,WDT_UNSIGNED) {
            WordVarArgs a={x[0],x[1],x[2]}; add(*this,a,z);
            for (int i=0; i<3; i++) dom(*this,x[i],7U);
          }
          Fallback(Fallback& s) : Space(s) {
            x.update(*this,s.x); z.update(*this,s.z);
          }
          Space* copy(void) { return new Fallback(*this); }
        };
        Fallback fallback;
        if ((fallback.status() == SS_FAILED) || !fallback.z.assigned() ||
            (fallback.z.val() != 5U)) return false;
        class Wide : public Space {
        public:
          WordVar x,c,z;
          Wide(void)
            : x(*this,64,WDT_UNSIGNED,WordValue(1)<<63,
                (WordValue(1)<<63)+1U),
              c(*this,64,WDT_UNSIGNED,1U,1U),
              z(*this,64,WDT_UNSIGNED) {
            WordVarArgs a={x,c}; add(*this,a,z);
          }
          Wide(Wide& s) : Space(s) {
            x.update(*this,s.x); c.update(*this,s.c); z.update(*this,s.z);
          }
          Space* copy(void) { return new Wide(*this); }
        };
        Wide w;
        return (w.status() != SS_FAILED) &&
          (w.z.minimum() == (WordValue(1)<<63)+1U) &&
          (w.z.maximum() == (WordValue(1)<<63)+2U);
      }
    public:
      BoundedLifecycle(void) : Base("Word::Arithmetic::BoundedLifecycle") {}
      virtual bool run(void) {
        return partial_domain_oracle() && division_truth() && division_propagation() &&
          division_replay() && propagation() && boundaries_aliases() &&
          replay() && staged_propagation() && nary_propagation();
      }
    };

    Binary addition(ADD,"Add");
    NaryAddition nary_addition;
    Negation negation;
    Binary subtraction(SUB,"Sub");
    Binary multiplication(MULT,"Mult");
    Binary division(DIV,"Div");
    Binary remainder(MOD,"Mod");
    Binary signed_division(SIGNED_DIV,"SignedDiv");
    Binary signed_remainder(SIGNED_REM,"SignedRem");
    Binary signed_modulus(SIGNED_MOD,"SignedMod");
    Lifecycle lifecycle;
    MultiplicationLifecycle multiplication_lifecycle;
    DivisionLifecycle division_lifecycle;
    SignedDivisionLifecycle signed_division_lifecycle;
    BoundedLifecycle bounded_lifecycle;

  }

}}

// STATISTICS: test-word
