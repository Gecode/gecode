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

    enum Op { ADD, NEG, SUB };

    static Gecode::WordValue
    evaluate(Op op, Gecode::WordValue x, Gecode::WordValue y,
             Gecode::WordValue mask) {
      switch (op) {
      case ADD: return (x + y) & mask;
      case NEG: return (Gecode::WordValue(0) - x) & mask;
      case SUB: return (x - y) & mask;
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

        try {
          ArithmeticSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::add(mismatch,mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool counters(void) {
        ArithmeticSpace s;
        Gecode::add(s,s.x[0],s.x[1],s.x[2]);
        // The fixed decomposition has five intermediate WordVars and six
        // propagators; Gecode has no public variable-allocation counter.
        if (Gecode::PropagatorGroup::all.size(s) != 6)
          return false;
        Gecode::StatusStatistics statistics;
        if (s.status(statistics) == Gecode::SS_FAILED)
          return false;
        return statistics.propagate >= 6;
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

    public:
      Lifecycle(void) : Base("Word::Arithmetic::Lifecycle") {}
      virtual bool run(void) {
        return partial(ADD) && partial(NEG) && partial(SUB) &&
          boolean_parity() && constants_aliases_lifecycle() &&
          counters() && search_recomputation();
      }
    };

    Binary addition(ADD,"Add");
    Negation negation;
    Binary subtraction(SUB,"Sub");
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
