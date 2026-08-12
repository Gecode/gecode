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
        return partial(ADD) && add_bit_consistency() &&
          partial(NEG) && neg_bit_consistency() &&
          partial(SUB) && sub_bit_consistency() &&
          boolean_parity() && constants_aliases_lifecycle() &&
          counters() && search_recomputation() &&
          neg_sub_search_recomputation();
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

    public:
      MultiplicationLifecycle(void)
        : Base("Word::Arithmetic::MultLifecycle") {}
      virtual bool run(void) {
        return partial() && assigned_small() && prefix_propagation() &&
          boolean_parity() && lifecycle() && counters() &&
          search_recomputation();
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
        Gecode::WordVar boolean_div;
        Gecode::WordVar boolean_mod;
        DifferentialSpace(void)
          : x(*this,2), y(*this,2), native_div(*this,2),
            native_mod(*this,2), boolean_div(*this,2),
            boolean_mod(*this,2) {
          Gecode::div(*this,x,y,native_div);
          Gecode::mod(*this,x,y,native_mod);

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
                !s.boolean_div.assigned() || !s.boolean_mod.assigned() ||
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
        Gecode::div(source,source.x[0],source.x[1],source.x[2]);
        Gecode::mod(source,source.x[0],source.x[1],source.x[3]);
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

        try {
          DivisionSpace mismatch(4,3);
          Gecode::WordVar other(mismatch,2);
          Gecode::div(mismatch,mismatch.x[0],other,mismatch.x[2]);
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
            div(*this,x,y,quotient);
            mod(*this,x,y,remainder);
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
          native_propagation() && search_recomputation();
      }
    };

    Binary addition(ADD,"Add");
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

  }

}}

// STATISTICS: test-word
