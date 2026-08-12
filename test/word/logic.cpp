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

  /// Tests for word logical constraints
  namespace Logic {

    static Gecode::WordValue
    evaluate(Gecode::WordOpType wot, Gecode::WordValue x,
             Gecode::WordValue y, Gecode::WordValue mask) {
      switch (wot) {
      case Gecode::WOT_AND:  return x & y;
      case Gecode::WOT_OR:   return x | y;
      case Gecode::WOT_XOR:  return x ^ y;
      case Gecode::WOT_NAND: return ~(x & y) & mask;
      case Gecode::WOT_NOR:  return ~(x | y) & mask;
      case Gecode::WOT_XNOR: return ~(x ^ y) & mask;
      default: GECODE_NEVER;
      }
      return 0;
    }

    /**
     * The direct table actor performs bounded native-word work and enforces
     * bit consistency for each independent logical bit.
     */
    class Binary : public Test {
    private:
      Gecode::WordOpType wot;
    public:
      Binary(Gecode::WordOpType wot0, const std::string& name)
        : Test("Logic::Binary::"+name,3,Domain(3,0,7)), wot(wot0) {}
      virtual bool solution(const Assignment& a) const {
        return a[2] == evaluate(wot,a[0],a[1],dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::rel(home,x[0],wot,x[1],x[2]);
      }
    };

    /// Direct bit-consistent complement propagator
    class Complement : public Test {
    public:
      Complement(void) : Test("Logic::Complement",2,Domain(3,0,7)) {}
      virtual bool solution(const Assignment& a) const {
        return a[1] == ((~a[0]) & dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::complement(home,x[0],x[1]);
      }
    };

    /// N-ary operators use bit-consistent primitive global actors
    class Nary : public Test {
    private:
      Gecode::WordOpType wot;
    public:
      Nary(Gecode::WordOpType wot0, const std::string& name)
        : Test("Logic::Nary::"+name,4,Domain(2,0,3)), wot(wot0) {}
      virtual bool solution(const Assignment& a) const {
        Gecode::WordOpType base;
        bool negate;
        switch (wot) {
        case Gecode::WOT_AND: case Gecode::WOT_NAND:
          base=Gecode::WOT_AND; break;
        case Gecode::WOT_OR: case Gecode::WOT_NOR:
          base=Gecode::WOT_OR; break;
        default: base=Gecode::WOT_XOR; break;
        }
        negate = (wot == Gecode::WOT_NAND) ||
          (wot == Gecode::WOT_NOR) || (wot == Gecode::WOT_XNOR);
        Gecode::WordValue result = evaluate(base,a[0],a[1],dom.mask());
        result = evaluate(base,result,a[2],dom.mask());
        if (negate)
          result = ~result & dom.mask();
        return a[3] == result;
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::WordVarArgs args(3);
        for (int i=0; i<3; i++)
          args[i]=x[i];
        Gecode::rel(home,wot,args,x[3]);
      }
    };

    /// Focused partial-domain, alias, constant, identity, and width checks
    class Lifecycle : public Base {
    private:
      class LogicSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        LogicSpace(int n=3, unsigned int width=2)
          : x(*this,n,width,0,Gecode::Word::width_mask(width)) {}
        LogicSpace(LogicSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) { return new LogicSpace(*this); }
      };

      static bool partial_binary(Gecode::WordOpType wot) {
        for (PartialAssignment p(3,1); p.has_more(); p.next()) {
          TestSpace s(3,Domain(1,0,1));
          std::vector<Domain> ds;
          for (int i=0; i<3; i++) ds.push_back(p[i]);
          s.narrow(ds);
          Gecode::rel(s,s.x[0],wot,s.x[1],s.x[2]);
          const bool failed = s.failed();
          bool supported = false;
          bool bit_support[3][2] = {{false,false},{false,false},{false,false}};
          for (Values x0(p[0]); x0(); ++x0)
            for (Values x1(p[1]); x1(); ++x1)
              for (Values z(p[2]); z(); ++z)
                if (z.val() == evaluate(wot,x0.val(),x1.val(),1U)) {
                  supported = true;
                  if (failed || !s.x[0].in(x0.val()) ||
                      !s.x[1].in(x1.val()) || !s.x[2].in(z.val()))
                    return false;
                  bit_support[0][x0.val()] = true;
                  bit_support[1][x1.val()] = true;
                  bit_support[2][z.val()] = true;
                }
          if (failed == supported)
            return false;
          if (!failed)
            for (int i=0; i<3; i++)
              for (int b=0; b<2; b++)
                if (s.x[i].in(static_cast<Gecode::WordValue>(b)) !=
                    bit_support[i][b])
                  return false;
        }
        return true;
      }

      static bool partial_complement(void) {
        for (PartialAssignment p(2,1); p.has_more(); p.next()) {
          TestSpace s(2,Domain(1,0,1));
          std::vector<Domain> ds; ds.push_back(p[0]); ds.push_back(p[1]);
          s.narrow(ds);
          Gecode::complement(s,s.x[0],s.x[1]);
          const bool failed = s.failed();
          bool supported = false;
          for (Values x(p[0]); x(); ++x)
            for (Values y(p[1]); y(); ++y)
              if (y.val() == (1U ^ x.val())) {
                supported = true;
                if (failed || !s.x[0].in(x.val()) || !s.x[1].in(y.val()))
                  return false;
              }
          if (failed == supported)
            return false;
        }
        return true;
      }

      static Gecode::WordValue identity(Gecode::WordOpType wot,
                                        Gecode::WordValue mask) {
        switch (wot) {
        case Gecode::WOT_AND: case Gecode::WOT_NOR: case Gecode::WOT_XNOR:
          return mask;
        default: return 0;
        }
      }

      static bool native_nary(void) {
        const Gecode::WordOpType primitive[] = {
          Gecode::WOT_AND,Gecode::WOT_OR,Gecode::WOT_XOR
        };
        for (unsigned int op=0; op<3; op++) {
          LogicSpace compact(5,64);
          Gecode::WordVarArgs args(4);
          for (int i=0; i<4; i++) args[i]=compact.x[i];
          Gecode::rel(compact,primitive[op],args,compact.x[4]);
          if ((compact.status() == Gecode::SS_FAILED) ||
              (Gecode::PropagatorGroup::all.size(compact) != 1U))
            return false;
        }

        LogicSpace and_backward(4,4);
        Gecode::dom(and_backward,and_backward.x[3],1U,15U);
        Gecode::WordVarArgs and_args = {
          and_backward.x[0],and_backward.x[1],and_backward.x[2]
        };
        Gecode::rel(and_backward,Gecode::WOT_AND,and_args,and_backward.x[3]);
        if (and_backward.status() == Gecode::SS_FAILED)
          return false;
        for (int i=0; i<3; i++)
          if ((and_backward.x[i].lo() & 1U) == 0)
            return false;

        LogicSpace or_backward(4,4);
        Gecode::dom(or_backward,or_backward.x[0],0U,14U);
        Gecode::dom(or_backward,or_backward.x[1],0U,14U);
        Gecode::dom(or_backward,or_backward.x[3],1U,15U);
        Gecode::WordVarArgs or_args = {
          or_backward.x[0],or_backward.x[1],or_backward.x[2]
        };
        Gecode::rel(or_backward,Gecode::WOT_OR,or_args,or_backward.x[3]);
        if ((or_backward.status() == Gecode::SS_FAILED) ||
            ((or_backward.x[2].lo() & 1U) == 0))
          return false;

        LogicSpace xor_backward(4,4);
        Gecode::dom(xor_backward,xor_backward.x[0],1U,15U);
        Gecode::dom(xor_backward,xor_backward.x[1],0U,14U);
        Gecode::dom(xor_backward,xor_backward.x[3],0U,14U);
        Gecode::WordVarArgs xor_args = {
          xor_backward.x[0],xor_backward.x[1],xor_backward.x[2]
        };
        Gecode::rel(xor_backward,Gecode::WOT_XOR,xor_args,
                    xor_backward.x[3]);
        if ((xor_backward.status() == Gecode::SS_FAILED) ||
            ((xor_backward.x[2].lo() & 1U) == 0))
          return false;

        LogicSpace duplicates(3,4);
        Gecode::WordVarArgs duplicate_args = {
          duplicates.x[0],duplicates.x[0],duplicates.x[1]
        };
        Gecode::rel(duplicates,Gecode::WOT_XOR,duplicate_args,
                    duplicates.x[2]);
        Gecode::dom(duplicates,duplicates.x[1],6U);
        if ((duplicates.status() == Gecode::SS_FAILED) ||
            !duplicates.x[2].assigned() ||
            (duplicates.x[2].val() != 6U))
          return false;

        LogicSpace result_alias(2,4);
        Gecode::WordVarArgs alias_args = {
          result_alias.x[0],result_alias.x[1]
        };
        Gecode::rel(result_alias,Gecode::WOT_XOR,alias_args,
                    result_alias.x[0]);
        if ((result_alias.status() == Gecode::SS_FAILED) ||
            !result_alias.x[1].assigned() ||
            (result_alias.x[1].val() != 0U) ||
            result_alias.x[0].assigned() ||
            (Gecode::PropagatorGroup::all.size(result_alias) != 0U))
          return false;

        LogicSpace absorbed(5,4);
        Gecode::WordVarArgs absorbed_args = {
          absorbed.x[0],absorbed.x[1],absorbed.x[2],absorbed.x[3]
        };
        Gecode::rel(absorbed,Gecode::WOT_AND,absorbed_args,absorbed.x[4]);
        Gecode::dom(absorbed,absorbed.x[0],0U);
        return (absorbed.status() != Gecode::SS_FAILED) &&
          absorbed.x[4].assigned() && (absorbed.x[4].val() == 0U) &&
          (Gecode::PropagatorGroup::all.size(absorbed) == 0U);
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVarArray x;
          WordVar result;
          SearchSpace(void) : x(*this,3,1,0,1), result(*this,1) {
            WordVarArgs args = {x[0],x[1],x[2]};
            rel(*this,WOT_XOR,args,result);
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
          const WordValue expected=solution->x[0].val()^
            solution->x[1].val()^solution->x[2].val();
          const bool ok=solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8U;
      }

    public:
      Lifecycle(void) : Base("Word::Logic::Lifecycle") {}
      virtual bool run(void) {
        const Gecode::WordOpType ops[] = {
          Gecode::WOT_AND, Gecode::WOT_OR, Gecode::WOT_XOR,
          Gecode::WOT_NAND, Gecode::WOT_NOR, Gecode::WOT_XNOR
        };
        if (!partial_complement())
          return false;
        for (unsigned int i=0; i<6; i++) {
          if (!partial_binary(ops[i]))
            return false;

          LogicSpace empty(1);
          Gecode::WordVarArgs no_args;
          Gecode::rel(empty,ops[i],no_args,empty.x[0]);
          if ((empty.status() == Gecode::SS_FAILED) ||
              !empty.x[0].assigned() ||
              (empty.x[0].val() != identity(ops[i],3U)))
            return false;

          LogicSpace single(2);
          Gecode::dom(single,single.x[0],1U);
          Gecode::WordVarArgs one_arg(1);
          one_arg[0]=single.x[0];
          Gecode::rel(single,ops[i],one_arg,single.x[1]);
          const Gecode::WordValue single_value =
            ((ops[i] == Gecode::WOT_NAND) ||
             (ops[i] == Gecode::WOT_NOR) ||
             (ops[i] == Gecode::WOT_XNOR)) ? 2U : 1U;
          if ((single.status() == Gecode::SS_FAILED) ||
              !single.x[1].assigned() ||
              (single.x[1].val() != single_value))
            return false;

          for (Gecode::WordValue value=0; value<4; value++) {
            LogicSpace alias(2);
            Gecode::dom(alias,alias.x[0],value);
            Gecode::rel(alias,alias.x[0],ops[i],alias.x[0],alias.x[1]);
            if ((alias.status() == Gecode::SS_FAILED) ||
                !alias.x[1].assigned() ||
                (alias.x[1].val() != evaluate(ops[i],value,value,3U)))
              return false;

            LogicSpace constant_operand(2);
            Gecode::dom(constant_operand,constant_operand.x[0],value);
            Gecode::rel(constant_operand,constant_operand.x[0],ops[i],
                        2,1U,constant_operand.x[1]);
            if ((constant_operand.status() == Gecode::SS_FAILED) ||
                !constant_operand.x[1].assigned() ||
                (constant_operand.x[1].val() !=
                 evaluate(ops[i],value,1U,3U)))
              return false;
          }

          LogicSpace constant_result(2);
          Gecode::dom(constant_result,constant_result.x[0],1U);
          Gecode::dom(constant_result,constant_result.x[1],2U);
          const Gecode::WordValue result = evaluate(ops[i],1U,2U,3U);
          Gecode::rel(constant_result,constant_result.x[0],ops[i],
                      constant_result.x[1],2,result);
          if (constant_result.status() == Gecode::SS_FAILED)
            return false;
        }

        LogicSpace constants(2);
        Gecode::complement(constants,2,1U,constants.x[0]);
        Gecode::complement(constants,constants.x[1],2,2U);
        if ((constants.status() == Gecode::SS_FAILED) ||
            !constants.x[0].assigned() || (constants.x[0].val() != 2U) ||
            !constants.x[1].assigned() || (constants.x[1].val() != 1U))
          return false;

        LogicSpace complement_alias(1);
        Gecode::complement(complement_alias,complement_alias.x[0],
                           complement_alias.x[0]);
        if (complement_alias.status() != Gecode::SS_FAILED)
          return false;

        LogicSpace source;
        Gecode::rel(source,source.x[0],Gecode::WOT_XOR,
                    source.x[1],source.x[2]);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        LogicSpace* clone = static_cast<LogicSpace*>(source.clone());
        Gecode::dom(*clone,clone->x[0],1U);
        Gecode::dom(*clone,clone->x[1],2U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 3U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          LogicSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::complement(mismatch,mismatch.x[0],other);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return native_nary() && search_recomputation();
      }
    };

    Complement complement_test;
    Binary binary_and(Gecode::WOT_AND,"And");
    Binary binary_or(Gecode::WOT_OR,"Or");
    Binary binary_xor(Gecode::WOT_XOR,"Xor");
    Binary binary_nand(Gecode::WOT_NAND,"Nand");
    Binary binary_nor(Gecode::WOT_NOR,"Nor");
    Binary binary_xnor(Gecode::WOT_XNOR,"Xnor");
    Nary nary_and(Gecode::WOT_AND,"And");
    Nary nary_or(Gecode::WOT_OR,"Or");
    Nary nary_xor(Gecode::WOT_XOR,"Xor");
    Nary nary_nand(Gecode::WOT_NAND,"Nand");
    Nary nary_nor(Gecode::WOT_NOR,"Nor");
    Nary nary_xnor(Gecode::WOT_XNOR,"Xnor");
    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
