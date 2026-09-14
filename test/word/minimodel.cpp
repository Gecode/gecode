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
#include <gecode/minimodel/word-post.hh>
#include <gecode/search.hh>
#include <stdexcept>
#include <vector>

namespace Test { namespace Word {

  namespace MiniModel {

    class Logic : public Test {
    public:
      Logic(void) : Test("MiniModel::Logic",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == (((a[0] & (~a[1] & 3U)) ^ a[2] ^ 1U) & 3U);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        WordExpr original = ((WordExpr(x[0]) & ~WordExpr(x[1])) ^
                             WordExpr(x[2])) ^ WordExpr(2,1U);
        WordExpr copied(original);
        WordExpr assigned(x[0]);
        assigned = copied;
        rel(home,assigned == WordExpr(x[3]));
      }
    };

    class NamedLogic : public Test {
    public:
      NamedLogic(void) : Test("MiniModel::NamedLogic",3,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        const Gecode::WordValue n0 = ~(a[0] & a[1]) & 3U;
        const Gecode::WordValue n1 = ~(a[0] | a[1]) & 3U;
        return a[2] == (~(n0 ^ n1) & 3U);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        WordExpr e = xnor(nand(WordExpr(x[0]),WordExpr(x[1])),
                          nor(WordExpr(x[0]),WordExpr(x[1])));
        rel(home,e == WordExpr(x[2]));
      }
    };

    class BooleanConditional : public Test {
    public:
      BooleanConditional(void)
        : Test("MiniModel::BooleanConditional",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == ((a[0] & 1U) ? a[1] : a[2]);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        WordExpr e = ite(bit(WordExpr(x[0]),0),WordExpr(x[1]),
                         WordExpr(x[2]));
        rel(home,e == WordExpr(x[3]));
      }
    };

    class MaskConditional : public Test {
    public:
      MaskConditional(void)
        : Test("MiniModel::MaskConditional",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == ((a[0] & a[1]) | (~a[0] & a[2] & 3U));
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        rel(home,ite(WordExpr(x[0]),WordExpr(x[1]),WordExpr(x[2])) ==
            WordExpr(x[3]));
      }
    };

    class Relation : public Test {
    public:
      Relation(void) : Test("MiniModel::Relation",2,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        const int signed_x = (a[0] & 2U) ?
          static_cast<int>(a[0])-4 : static_cast<int>(a[0]);
        const int signed_y = (a[1] & 2U) ?
          static_cast<int>(a[1])-4 : static_cast<int>(a[1]);
        return (a[0] < a[1]) && (signed_x >= signed_y) &&
          (a[0] & 1U) && !(a[0] & 2U) && (a[0] != a[1]);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        BoolExpr relation = (WordExpr(x[0]) < WordExpr(x[1])) &&
          signed_greater_equal(WordExpr(x[0]),WordExpr(x[1])) &&
          bit(WordExpr(x[0]),0) && !bit(WordExpr(x[0]),1) &&
          !(WordExpr(x[0]) == WordExpr(x[1]));
        rel(home,relation);
      }
    };

    class Reduction : public Test {
    public:
      enum Operation { OP_AND, OP_OR, OP_XOR };
    private:
      Operation op;
      int expected;

      static bool parity(Gecode::WordValue value) {
        value ^= value >> 32;
        value ^= value >> 16;
        value ^= value >> 8;
        value ^= value >> 4;
        value ^= value >> 2;
        value ^= value >> 1;
        return (value & 1U) != 0;
      }
    public:
      Reduction(Operation op0, int expected0, const std::string& name)
        : Test("MiniModel::Reduction::"+name+"::"+str(expected0),
               1,Domain(3,0,7)), op(op0), expected(expected0) {}
      virtual bool solution(const Assignment& a) const {
        bool actual = false;
        switch (op) {
        case OP_AND: actual = a[0] == 7U; break;
        case OP_OR: actual = a[0] != 0; break;
        case OP_XOR: actual = parity(a[0]); break;
        }
        return actual == (expected != 0);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        BoolExpr e;
        switch (op) {
        case OP_AND: e = reduce_and(WordExpr(x[0])); break;
        case OP_OR: e = reduce_or(WordExpr(x[0])); break;
        case OP_XOR: e = reduce_xor(WordExpr(x[0])); break;
        }
        rel(home,expected ? e : !e);
      }
    };

    class ReductionComposition : public Test {
    public:
      ReductionComposition(void)
        : Test("MiniModel::ReductionComposition",1,Domain(3,0,7)) {}
      virtual bool solution(const Assignment& a) const {
        Gecode::WordValue value = a[0];
        value ^= value >> 2;
        value ^= value >> 1;
        const bool parity = (value & 1U) != 0;
        return (a[0] != 0) && (a[0] != 7U) && parity;
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        using namespace Gecode;
        rel(home,reduce_or(WordExpr(x[0])) &&
            !reduce_and(WordExpr(x[0])) && reduce_xor(WordExpr(x[0])));
      }
    };

    class Lifecycle : public Base {
    private:
      class ParitySpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar direct_result;
        Gecode::WordVar minimodel_result;
        Gecode::BoolVar control;
        Gecode::BoolVar direct_relation;
        Gecode::BoolVar minimodel_relation;
        ParitySpace(void)
          : x(*this,2), y(*this,2), direct_result(*this,2),
            minimodel_result(), control(*this,0,1),
            direct_relation(*this,0,1), minimodel_relation() {
          using namespace Gecode;
          WordVar combined(*this,2);
          rel(*this,x,WOT_XOR,y,combined);
          ite(*this,control,combined,x,direct_result);
          rel(*this,x,WRT_SLQ,y,Reify(direct_relation,RM_EQV));

          WordExpr expression = ite(BoolExpr(control),
                                     WordExpr(x) ^ WordExpr(y),WordExpr(x));
          minimodel_result = expr(*this,expression);
          minimodel_relation =
            expr(*this,signed_less_equal(WordExpr(x),WordExpr(y)));
        }
        ParitySpace(ParitySpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          direct_result.update(*this,s.direct_result);
          minimodel_result.update(*this,s.minimodel_result);
          control.update(*this,s.control);
          direct_relation.update(*this,s.direct_relation);
          minimodel_relation.update(*this,s.minimodel_relation);
        }
        virtual Gecode::Space* copy(void) {
          return new ParitySpace(*this);
        }
      };

      static bool parity(void) {
        for (Gecode::WordValue x=0; x<4; x++)
          for (Gecode::WordValue y=0; y<4; y++)
            for (int control=0; control<2; control++) {
              ParitySpace s;
              Gecode::dom(s,s.x,x);
              Gecode::dom(s,s.y,y);
              Gecode::rel(s,s.control,Gecode::IRT_EQ,control);
              if ((s.status() == Gecode::SS_FAILED) ||
                  !s.direct_result.assigned() ||
                  !s.minimodel_result.assigned() ||
                  (s.direct_result.val() != s.minimodel_result.val()) ||
                  !s.direct_relation.assigned() ||
                  !s.minimodel_relation.assigned() ||
                  (s.direct_relation.val() != s.minimodel_relation.val()))
                return false;
            }

        ParitySpace partial;
        Gecode::dom(partial,partial.x,1U,3U);
        Gecode::dom(partial,partial.y,0U,2U);
        Gecode::rel(partial,partial.control,Gecode::IRT_EQ,1);
        if (partial.status() == Gecode::SS_FAILED)
          return false;
        return (partial.direct_result.lo() == partial.minimodel_result.lo()) &&
          (partial.direct_result.hi() == partial.minimodel_result.hi()) &&
          (partial.direct_relation.min() == partial.minimodel_relation.min()) &&
          (partial.direct_relation.max() == partial.minimodel_relation.max());
      }

      static bool invalid(void) {
        ParitySpace s;
        Gecode::WordVar narrow(s,1);
        if (Gecode::WordExpr(s.x).width() != 2)
          return false;
        try {
          (void) (Gecode::WordExpr(s.x) & Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) (Gecode::WordExpr(s.x) == Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::ite(Gecode::WordExpr(narrow),
                             Gecode::WordExpr(s.x),Gecode::WordExpr(s.y));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::bit(Gecode::WordExpr(s.x),2);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        return true;
      }

      static bool clone(void) {
        ParitySpace source;
        if (source.status() == Gecode::SS_FAILED)
          return false;
        ParitySpace* copy = static_cast<ParitySpace*>(source.clone());
        Gecode::dom(*copy,copy->x,3U);
        Gecode::dom(*copy,copy->y,1U);
        Gecode::rel(*copy,copy->control,Gecode::IRT_EQ,1);
        const bool ok = (copy->status() != Gecode::SS_FAILED) &&
          copy->minimodel_result.assigned() &&
          (copy->minimodel_result.val() == 2U) && !source.x.assigned();
        delete copy;
        return ok;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar result;
          BoolVar control;
          SearchSpace(void)
            : x(*this,2), y(*this,2), result(), control(*this,0,1) {
            result = expr(*this,ite(BoolExpr(control),
                                    WordExpr(x) ^ WordExpr(y),WordExpr(x)));
            WordVarArgs decision = {x,y};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
            branch(*this,control,BOOL_VAL_MIN());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            result.update(*this,s.result);
            control.update(*this,s.control);
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
          const WordValue expected = solution->control.val() ?
            solution->x.val() ^ solution->y.val() : solution->x.val();
          const bool ok = solution->x.assigned() && solution->y.assigned() &&
            solution->control.assigned() && solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 32;
      }

      static bool domain_policy(void) {
        using namespace Gecode;
        class PolicySpace : public Space {
        public:
          WordVar x, y, unsigned_result, signed_result, bitwise_result;
          PolicySpace(void)
            : x(*this,4), y(*this,4),
              unsigned_result((WordExpr(x)+WordExpr(y)).
                              post(*this,WDT_UNSIGNED)),
              signed_result((-WordExpr(x)).post(*this,WDT_SIGNED)),
              bitwise_result((WordExpr(x)^WordExpr(y)).post(*this,WDT_UNSIGNED)) {}
          PolicySpace(PolicySpace& s) : Space(s) {
            x.update(*this,s.x); y.update(*this,s.y);
            unsigned_result.update(*this,s.unsigned_result);
            signed_result.update(*this,s.signed_result);
            bitwise_result.update(*this,s.bitwise_result);
          }
          virtual Space* copy(void) { return new PolicySpace(*this); }
        };
        PolicySpace source;
        if (source.status() == SS_FAILED) return false;
        PolicySpace* copy=static_cast<PolicySpace*>(source.clone());
        const bool ok=(copy->unsigned_result.domain_type() == WDT_UNSIGNED) &&
          (copy->signed_result.domain_type() == WDT_SIGNED) &&
          (copy->bitwise_result.domain_type() == WDT_UNSIGNED);
        delete copy;
        return ok;
      }

    public:
      Lifecycle(void) : Base("Word::MiniModel::Lifecycle") {}
      virtual bool run(void) {
        return parity() && invalid() && clone() && search_recomputation() &&
          domain_policy();
      }
    };

    class StructuralLifecycle : public Base {
    private:
      enum Operation {
        OP_EXTRACT, OP_CONCAT, OP_REPEAT, OP_ZERO_EXTEND, OP_SIGN_EXTEND,
        OP_SHIFT_LEFT, OP_VAR_SHIFT_LEFT, OP_LOGICAL_SHIFT_RIGHT,
        OP_VAR_LOGICAL_SHIFT_RIGHT, OP_ARITHMETIC_SHIFT_RIGHT,
        OP_VAR_ARITHMETIC_SHIFT_RIGHT, OP_ROTATE_LEFT, OP_ROTATE_RIGHT,
        OP_COUNT
      };

      static unsigned int result_width(Operation op) {
        switch (op) {
        case OP_EXTRACT: return 2;
        case OP_CONCAT: case OP_REPEAT:
        case OP_ZERO_EXTEND: case OP_SIGN_EXTEND: return 8;
        default: return 4;
        }
      }

      class StructuralSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar amount;
        Gecode::WordVar direct_result;
        Gecode::WordVar minimodel_result;
        StructuralSpace(Operation op, unsigned int fixed_amount)
          : x(*this,4), amount(*this,4),
            direct_result(*this,result_width(op)), minimodel_result() {
          using namespace Gecode;
          WordExpr expression(x);
          switch (op) {
          case OP_EXTRACT:
            Gecode::extract(*this,x,1,2,direct_result);
            expression = Gecode::extract(WordExpr(x),1,2);
            break;
          case OP_CONCAT:
            Gecode::concat(*this,x,amount,direct_result);
            expression = Gecode::concat(WordExpr(x),WordExpr(amount));
            break;
          case OP_REPEAT:
            Gecode::repeat(*this,x,2,direct_result);
            expression = Gecode::repeat(WordExpr(x),2);
            break;
          case OP_ZERO_EXTEND:
            Gecode::zero_extend(*this,x,8,direct_result);
            expression = Gecode::zero_extend(WordExpr(x),8);
            break;
          case OP_SIGN_EXTEND:
            Gecode::sign_extend(*this,x,8,direct_result);
            expression = Gecode::sign_extend(WordExpr(x),8);
            break;
          case OP_SHIFT_LEFT:
            Gecode::shift_left(*this,x,fixed_amount,direct_result);
            expression = WordExpr(x) << fixed_amount;
            break;
          case OP_VAR_SHIFT_LEFT:
            Gecode::shift_left(*this,x,amount,direct_result);
            expression = WordExpr(x) << WordExpr(amount);
            break;
          case OP_LOGICAL_SHIFT_RIGHT:
            Gecode::logical_shift_right(*this,x,fixed_amount,direct_result);
            expression = Gecode::logical_shift_right(WordExpr(x),fixed_amount);
            break;
          case OP_VAR_LOGICAL_SHIFT_RIGHT:
            Gecode::logical_shift_right(*this,x,amount,direct_result);
            expression = Gecode::logical_shift_right(WordExpr(x),
                                                       WordExpr(amount));
            break;
          case OP_ARITHMETIC_SHIFT_RIGHT:
            Gecode::arithmetic_shift_right(*this,x,fixed_amount,direct_result);
            expression = Gecode::arithmetic_shift_right(WordExpr(x),
                                                         fixed_amount);
            break;
          case OP_VAR_ARITHMETIC_SHIFT_RIGHT:
            Gecode::arithmetic_shift_right(*this,x,amount,direct_result);
            expression = Gecode::arithmetic_shift_right(WordExpr(x),
                                                         WordExpr(amount));
            break;
          case OP_ROTATE_LEFT:
            Gecode::rotate_left(*this,x,fixed_amount,direct_result);
            expression = Gecode::rotate_left(WordExpr(x),fixed_amount);
            break;
          case OP_ROTATE_RIGHT:
            Gecode::rotate_right(*this,x,fixed_amount,direct_result);
            expression = Gecode::rotate_right(WordExpr(x),fixed_amount);
            break;
          default:
            GECODE_NEVER;
          }
          WordExpr copied(expression);
          WordExpr assigned(x);
          assigned = copied;
          minimodel_result = expr(*this,assigned);
        }
        StructuralSpace(StructuralSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          amount.update(*this,s.amount);
          direct_result.update(*this,s.direct_result);
          minimodel_result.update(*this,s.minimodel_result);
        }
        virtual Gecode::Space* copy(void) {
          return new StructuralSpace(*this);
        }
      };

      static bool parity(void) {
        const unsigned int fixed_amounts[] = {0,3,4,7};
        for (unsigned int op=0; op<OP_COUNT; op++)
          for (unsigned int a=0; a<4; a++)
            for (Gecode::WordValue x=0; x<16; x++)
              for (Gecode::WordValue amount=0; amount<16; amount += 5) {
                StructuralSpace s(static_cast<Operation>(op),
                                  fixed_amounts[a]);
                Gecode::dom(s,s.x,x);
                Gecode::dom(s,s.amount,amount);
                if ((s.status() == Gecode::SS_FAILED) ||
                    (s.direct_result.width() != result_width(
                       static_cast<Operation>(op))) ||
                    (s.minimodel_result.width() != result_width(
                       static_cast<Operation>(op))) ||
                    !s.direct_result.assigned() ||
                    !s.minimodel_result.assigned() ||
                    (s.direct_result.val() != s.minimodel_result.val()))
                  return false;
              }

        StructuralSpace partial(OP_CONCAT,0);
        Gecode::dom(partial,partial.x,1U,11U);
        Gecode::dom(partial,partial.amount,2U,11U);
        if (partial.status() == Gecode::SS_FAILED)
          return false;
        return (partial.direct_result.lo() == partial.minimodel_result.lo()) &&
          (partial.direct_result.hi() == partial.minimodel_result.hi());
      }

      static bool invalid(void) {
        StructuralSpace s(OP_EXTRACT,0);
        Gecode::WordVar narrow(s,3);
        Gecode::WordVar wide0(s,40);
        Gecode::WordVar wide1(s,40);
        try {
          (void) Gecode::extract(Gecode::WordExpr(s.x),0,0);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::extract(Gecode::WordExpr(s.x),4,1);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::extract(Gecode::WordExpr(s.x),3,2);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::concat(Gecode::WordExpr(wide0),
                                Gecode::WordExpr(wide1));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::repeat(Gecode::WordExpr(s.x),0);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::repeat(Gecode::WordExpr(s.x),17);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::zero_extend(Gecode::WordExpr(s.x),3);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) Gecode::sign_extend(Gecode::WordExpr(s.x),65);
          return false;
        } catch (const Gecode::Word::OutOfLimits&) {}
        try {
          (void) (Gecode::WordExpr(s.x) << Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::logical_shift_right(Gecode::WordExpr(s.x),
                                              Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::arithmetic_shift_right(Gecode::WordExpr(s.x),
                                                 Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          WordVar amount;
          WordVar result;
          SearchSpace(void)
            : x(*this,2), amount(*this,2), result() {
            WordExpr shifted = logical_shift_right(WordExpr(x),
                                                   WordExpr(amount));
            result = expr(*this,rotate_left(shifted,1));
            WordVarArgs decision = {x,amount};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            amount.update(*this,s.amount);
            result.update(*this,s.result);
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
          const WordValue shifted = (solution->amount.val() < 2) ?
            solution->x.val() >> solution->amount.val() : 0;
          const WordValue expected = ((shifted << 1) | (shifted >> 1)) & 3U;
          const bool ok = solution->x.assigned() &&
            solution->amount.assigned() && solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 16;
      }

    public:
      StructuralLifecycle(void)
        : Base("Word::MiniModel::StructuralLifecycle") {}
      virtual bool run(void) {
        return parity() && invalid() && search_recomputation();
      }
    };

    class BooleanPolicyLifecycle : public Base {
    private:
      class PolicySpace : public Gecode::Space {
      public:
        Gecode::WordVar x, y, arithmetic_result, bitwise_result;
        Gecode::BoolVar control, factory_composition, manual_composition;
        Gecode::BoolVarArray factory, manual;

        PolicySpace(Gecode::WordDomainType domain_type)
          : x(*this,3), y(*this,3), arithmetic_result(), bitwise_result(),
            control(*this,0,1), factory_composition(), manual_composition(),
            factory(*this,7,0,1), manual(*this,7,0,1) {
          using namespace Gecode;
          const bool signed_domain = domain_type == WDT_SIGNED;
          WordExpr arithmetic_expression = signed_domain ?
            -WordExpr(x) : WordExpr(x)+WordExpr(y);
          arithmetic_result=arithmetic_expression.post(*this,domain_type);
          WordExpr bitwise_expression=WordExpr(x)^WordExpr(y);
          bitwise_result=bitwise_expression.post(*this,domain_type);

          rel(*this,arithmetic_result,signed_domain ? WRT_SLQ : WRT_ULQ,y,
              Reify(manual[0],RM_EQV));
          channel(*this,arithmetic_result,0,manual[1]);
          reduce_and(*this,arithmetic_result,manual[2]);
          reduce_or(*this,arithmetic_result,manual[3]);
          reduce_xor(*this,arithmetic_result,manual[4]);
          overflow(*this,arithmetic_result,WOF_NEG_SIGNED,manual[5]);
          overflow(*this,arithmetic_result,
                   signed_domain ? WOF_ADD_SIGNED : WOF_ADD_UNSIGNED,
                   y,manual[6]);

          factory[0]=expr(*this,word_rel(arithmetic_expression,
            signed_domain ? WRT_SLQ : WRT_ULQ,WordExpr(y),domain_type));
          factory[1]=expr(*this,bit(arithmetic_expression,0,domain_type));
          factory[2]=expr(*this,reduce_and(arithmetic_expression,domain_type));
          factory[3]=expr(*this,reduce_or(arithmetic_expression,domain_type));
          factory[4]=expr(*this,reduce_xor(arithmetic_expression,domain_type));
          factory[5]=expr(*this,overflow(arithmetic_expression,
            WOF_NEG_SIGNED,domain_type));
          factory[6]=expr(*this,overflow(arithmetic_expression,
            signed_domain ? WOF_ADD_SIGNED : WOF_ADD_UNSIGNED,WordExpr(y),
            domain_type));

          BoolExpr ordinary(control);
          factory_composition=expr(*this,
            ((word_rel(arithmetic_expression,
                       signed_domain ? WRT_SLQ : WRT_ULQ,
                       WordExpr(y),domain_type) &&
              ordinary) || !bit(bitwise_expression,0,domain_type)) ==
            reduce_xor(arithmetic_expression,domain_type));
          manual_composition=expr(*this,
            ((BoolExpr(manual[0]) && ordinary) ||
             !bit(WordExpr(bitwise_result),0)) == BoolExpr(manual[4]));

          WordVarArgs decisions={x,y};
          branch(*this,decisions,WORD_VAR_NONE(),WORD_VAL_LSB());
          branch(*this,control,BOOL_VAL_MIN());
        }
        PolicySpace(PolicySpace& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y);
          arithmetic_result.update(*this,s.arithmetic_result);
          bitwise_result.update(*this,s.bitwise_result);
          control.update(*this,s.control);
          factory_composition.update(*this,s.factory_composition);
          manual_composition.update(*this,s.manual_composition);
          factory.update(*this,s.factory); manual.update(*this,s.manual);
        }
        virtual Space* copy(void) { return new PolicySpace(*this); }
      };

      static bool parity(Gecode::WordDomainType domain_type) {
        using namespace Gecode;
        PolicySpace* root=new PolicySpace(domain_type);
        if ((root->arithmetic_result.domain_type() != domain_type) ||
            (root->bitwise_result.domain_type() != domain_type)) {
          delete root;
          return false;
        }
        Search::Options options;
        options.c_d=1;
        DFS<PolicySpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (PolicySpace* solution=dfs.next()) {
          bool ok=solution->factory_composition.assigned() &&
            solution->manual_composition.assigned() &&
            (solution->factory_composition.val() ==
             solution->manual_composition.val());
          for (int i=0; i<solution->factory.size(); i++)
            ok = ok && solution->factory[i].assigned() &&
              solution->manual[i].assigned() &&
              (solution->factory[i].val() == solution->manual[i].val());
          if (!ok) {
            delete solution;
            return false;
          }
          delete solution;
          solutions++;
        }
        return solutions == 128U;
      }

      class DefaultSpace : public Gecode::Space {
      public:
        Gecode::WordVar x, y;
        DefaultSpace(bool explicit_cube) : x(*this,3), y(*this,3) {
          using namespace Gecode;
          WordExpr sum=WordExpr(x)+WordExpr(y);
          BoolExpr e=explicit_cube ?
            (word_rel(sum,WRT_ULQ,WordExpr(y),WDT_CUBE) &&
             reduce_or(sum,WDT_CUBE)) :
            ((sum <= WordExpr(y)) && reduce_or(sum));
          rel(*this,e);
        }
        DefaultSpace(DefaultSpace& s) : Space(s) {
          x.update(*this,s.x); y.update(*this,s.y);
        }
        virtual Space* copy(void) { return new DefaultSpace(*this); }
      };

      static bool default_cube_parity(void) {
        DefaultSpace old_api(false), explicit_cube(true);
        Gecode::StatusStatistics old_stats, explicit_stats;
        const Gecode::SpaceStatus old_status=old_api.status(old_stats);
        const Gecode::SpaceStatus explicit_status=
          explicit_cube.status(explicit_stats);
        return (old_status == explicit_status) &&
          (old_stats.propagate == explicit_stats.propagate) &&
          (Gecode::PropagatorGroup::all.size(old_api) ==
           Gecode::PropagatorGroup::all.size(explicit_cube));
      }
    public:
      BooleanPolicyLifecycle(void)
        : Base("Word::MiniModel::BooleanPolicyLifecycle") {}
      virtual bool run(void) {
        return parity(Gecode::WDT_UNSIGNED) && parity(Gecode::WDT_SIGNED) &&
          default_cube_parity();
      }
    };

    class ArithmeticLifecycle : public Base {
    private:
      enum Operation {
        OP_ADD, OP_NEG, OP_SUB, OP_MULT, OP_DIV, OP_MOD,
        OP_SIGNED_DIV, OP_SIGNED_REM, OP_SIGNED_MOD, OP_COUNT
      };

      class ArithmeticSpace : public Gecode::Space {
      public:
        Gecode::WordVar x;
        Gecode::WordVar y;
        Gecode::WordVar direct_result;
        Gecode::WordVar minimodel_result;
        ArithmeticSpace(Operation op)
          : x(*this,2), y(*this,2), direct_result(*this,2),
            minimodel_result() {
          using namespace Gecode;
          WordExpr expression(x);
          switch (op) {
          case OP_ADD:
            add(*this,x,y,direct_result);
            expression = WordExpr(x) + WordExpr(y);
            break;
          case OP_NEG:
            neg(*this,x,direct_result);
            expression = -WordExpr(x);
            break;
          case OP_SUB:
            sub(*this,x,y,direct_result);
            expression = WordExpr(x) - WordExpr(y);
            break;
          case OP_MULT:
            mult(*this,x,y,direct_result);
            expression = WordExpr(x) * WordExpr(y);
            break;
          case OP_DIV:
            Gecode::div(*this,x,y,direct_result,WS_SMTLIB);
            expression = Gecode::div(WordExpr(x),WordExpr(y),WS_SMTLIB);
            break;
          case OP_MOD:
            Gecode::mod(*this,x,y,direct_result,WS_SMTLIB);
            expression = Gecode::mod(WordExpr(x),WordExpr(y),WS_SMTLIB);
            break;
          case OP_SIGNED_DIV:
            signed_div(*this,x,y,direct_result,WS_SMTLIB);
            expression = signed_div(WordExpr(x),WordExpr(y),WS_SMTLIB);
            break;
          case OP_SIGNED_REM:
            signed_rem(*this,x,y,direct_result,WS_SMTLIB);
            expression = signed_rem(WordExpr(x),WordExpr(y),WS_SMTLIB);
            break;
          case OP_SIGNED_MOD:
            signed_mod(*this,x,y,direct_result,WS_SMTLIB);
            expression = signed_mod(WordExpr(x),WordExpr(y),WS_SMTLIB);
            break;
          default:
            GECODE_NEVER;
          }
          WordExpr copied(expression);
          WordExpr assigned(x);
          assigned = copied;
          minimodel_result = expr(*this,assigned);
        }
        ArithmeticSpace(ArithmeticSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          direct_result.update(*this,s.direct_result);
          minimodel_result.update(*this,s.minimodel_result);
        }
        virtual Gecode::Space* copy(void) {
          return new ArithmeticSpace(*this);
        }
      };

      static bool parity(void) {
        for (unsigned int op=0; op<OP_COUNT; op++)
          for (Gecode::WordValue x=0; x<4; x++)
            for (Gecode::WordValue y=0; y<4; y++) {
              ArithmeticSpace s(static_cast<Operation>(op));
              Gecode::dom(s,s.x,x);
              Gecode::dom(s,s.y,y);
              if ((s.status() == Gecode::SS_FAILED) ||
                  !s.direct_result.assigned() ||
                  !s.minimodel_result.assigned() ||
                  (s.direct_result.val() != s.minimodel_result.val()))
                return false;
            }
        return true;
      }

      static bool invalid(void) {
        ArithmeticSpace s(OP_ADD);
        Gecode::WordVar narrow(s,1);
        try {
          (void) (Gecode::WordExpr(s.x) + Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::div(Gecode::WordExpr(s.x),Gecode::WordExpr(narrow));
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        try {
          (void) Gecode::signed_mod(
            Gecode::WordExpr(s.x),Gecode::WordExpr(s.y),
            // One is an unnamed value within the enum's representable range.
            // Larger values would make the negative test itself undefined.
            static_cast<Gecode::WordSemantics>(1));
          return false;
        } catch (const Gecode::Word::UnknownOperation&) {}
        return true;
      }

      static bool clone(void) {
        ArithmeticSpace source(OP_SIGNED_DIV);
        if (source.status() == Gecode::SS_FAILED)
          return false;
        ArithmeticSpace* copy =
          static_cast<ArithmeticSpace*>(source.clone());
        Gecode::dom(*copy,copy->x,2U);
        Gecode::dom(*copy,copy->y,3U);
        const bool ok = (copy->status() != Gecode::SS_FAILED) &&
          copy->direct_result.assigned() && copy->minimodel_result.assigned() &&
          (copy->direct_result.val() == copy->minimodel_result.val()) &&
          !source.x.assigned();
        delete copy;
        return ok;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          WordVar x;
          WordVar y;
          WordVar result;
          SearchSpace(void)
            : x(*this,3), y(*this,3), result() {
            WordExpr sum = WordExpr(x) + WordExpr(y);
            WordExpr product = sum * WordExpr(3,3U);
            result = expr(*this,Gecode::mod(product,WordExpr(y),WS_SMTLIB));
            WordVarArgs decision = {x,y};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            x.update(*this,s.x);
            y.update(*this,s.y);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 16;
        options.a_d = 64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SearchSpace* solution = dfs.next()) {
          const WordValue product =
            ((solution->x.val()+solution->y.val()) * 3U) & 7U;
          const WordValue expected = (solution->y.val() == 0) ? product :
            product % solution->y.val();
          const bool ok = solution->x.assigned() && solution->y.assigned() &&
            solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 64;
      }

    public:
      ArithmeticLifecycle(void)
        : Base("Word::MiniModel::ArithmeticLifecycle") {}
      virtual bool run(void) {
        return parity() && invalid() && clone() && search_recomputation();
      }
    };

    class SharedLoweringLifecycle : public Base {
    private:
      static Gecode::WordExpr mixed_dag(Gecode::WordExpr e,
                                        unsigned int depth) {
        using namespace Gecode;
        for (unsigned int i=0; i<depth; i++)
          e=ite(bit(e,0),e+WordExpr(3,3U),e^WordExpr(3,6U));
        return e;
      }

      class MixedDagSpace : public Gecode::Space {
      public:
        Gecode::WordVar x, result;
        MixedDagSpace(unsigned int depth, Gecode::WordDomainType kind)
          : x(*this,3) {
          result=mixed_dag(Gecode::WordExpr(x),depth).post(*this,kind);
          Gecode::branch(*this,x,Gecode::WORD_VAL_LSB());
        }
        MixedDagSpace(MixedDagSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x); result.update(*this,s.result);
        }
        Gecode::Space* copy(void) { return new MixedDagSpace(*this); }
      };

      static bool mixed_sharing(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED}) {
          for (unsigned int depth : {4U,8U,12U}) {
            MixedDagSpace s(depth,kind);
            if (PropagatorGroup::all.size(s) > 12U*depth+4U)
              return false;
          }
          MixedDagSpace root(5,kind);
          Search::Options options; options.c_d=2; options.a_d=64;
          DFS<MixedDagSpace> dfs(&root,options);
          unsigned int count=0;
          while (MixedDagSpace* s=dfs.next()) {
            WordValue expected=s->x.val();
            for (unsigned int i=0; i<5; i++)
              expected=(expected&1U) ? (expected+3U)&7U : expected^6U;
            const bool ok=s->result.assigned() && s->result.val()==expected;
            delete s;
            if (!ok) return false;
            count++;
          }
          if (count != 8U) return false;
        }
        MixedDagSpace first(1,WDT_CUBE), second(1,WDT_CUBE);
        WordExpr e=mixed_dag(WordExpr(first.x),3);
        WordVar a=e.post(first), b=e.post(first);
        WordVar u=e.post(first,WDT_UNSIGNED), s=e.post(first,WDT_SIGNED);
        if ((Gecode::Word::WordView(a)==Gecode::Word::WordView(b)) ||
            (Gecode::Word::WordView(first.result)==
             Gecode::Word::WordView(second.result)) ||
            u.domain_type()!=WDT_UNSIGNED || s.domain_type()!=WDT_SIGNED)
          return false;
        dom(first,first.x,1U);
        return first.status()!=SS_FAILED && second.status()!=SS_FAILED &&
          !second.x.assigned() && !second.result.assigned();
      }

      static bool mixed_exception(void) {
        using namespace Gecode;
        class ThrowOnce : public BoolExpr::Misc {
          bool first=true;
        public:
          void post(Home home, BoolVar b, bool neg, const IntPropLevels&) {
            if (first) {
              first=false;
              throw std::runtime_error("mixed posting test");
            }
            rel(home,b,IRT_EQ,neg ? 0 : 1);
          }
        };
        MixedDagSpace home(1,WDT_CUBE);
        WordExpr shared=mixed_dag(WordExpr(home.x),3);
        WordExpr e=ite(bit(shared,0) && BoolExpr(new ThrowOnce),shared,~shared);
        bool caught=false;
        try { e.post(home); }
        catch (const std::runtime_error&) { caught=true; }
        WordVar result=e.post(home);
        dom(home,home.x,1U);
        return caught && home.status()!=SS_FAILED && result.assigned();
      }

      class SharedSpace : public Gecode::Space {
      public:
        Gecode::WordVar x, result;
        SharedSpace(unsigned int depth)
          : x(*this,4), result() {
          Gecode::WordExpr expression = ~Gecode::WordExpr(x);
          for (unsigned int i=0; i<depth; i++)
            expression = expression | expression;
          result = expression.post(*this);
        }
        SharedSpace(SharedSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          result.update(*this,s.result);
        }
        virtual Gecode::Space* copy(void) {
          return new SharedSpace(*this);
        }
      };

      class FanOutSpace : public Gecode::Space {
      public:
        Gecode::WordVar x, y, result;
        FanOutSpace(void) : x(*this,3), y(*this,3), result() {
          Gecode::WordExpr shared = Gecode::WordExpr(x)+Gecode::WordExpr(y);
          Gecode::WordExpr expression =
            (shared ^ Gecode::WordExpr(x)) + (shared | Gecode::WordExpr(y));
          result = expression.post(*this);
          Gecode::WordVarArgs decision = {x,y};
          Gecode::branch(*this,decision,Gecode::WORD_VAR_SIZE_MIN(),
                         Gecode::WORD_VAL_LSB());
        }
        FanOutSpace(FanOutSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          y.update(*this,s.y);
          result.update(*this,s.result);
        }
        virtual Gecode::Space* copy(void) {
          return new FanOutSpace(*this);
        }
      };

      static bool linear_actor_growth(void) {
        const unsigned int depths[] = {4,8,12};
        for (unsigned int i=0; i<3; i++) {
          SharedSpace s(depths[i]);
          if (Gecode::PropagatorGroup::all.size(s) != depths[i]+1)
            return false;
        }
        return true;
      }

      static bool posting_independence(void) {
        using namespace Gecode;
        class PolicySpace : public Space {
        public:
          WordVar x, first, second, bounded, cube;
          PolicySpace(void) : x(*this,4) {
            WordExpr expression = WordExpr(x)+WordExpr(4,1U);
            first = expression.post(*this);
            second = expression.post(*this);
            bounded = expression.post(*this,WDT_UNSIGNED);
            cube = expression.post(*this,WDT_CUBE);
          }
          PolicySpace(PolicySpace& s) : Space(s) {
            x.update(*this,s.x);
            first.update(*this,s.first);
            second.update(*this,s.second);
            bounded.update(*this,s.bounded);
            cube.update(*this,s.cube);
          }
          virtual Space* copy(void) { return new PolicySpace(*this); }
        };

        PolicySpace first_space;
        PolicySpace second_space;
        return (PropagatorGroup::all.size(first_space) == 4) &&
          (PropagatorGroup::all.size(second_space) == 4) &&
          (first_space.bounded.domain_type() == WDT_UNSIGNED) &&
          (first_space.cube.domain_type() == WDT_CUBE);
      }

      static bool fan_out_semantics_and_recomputation(void) {
        using namespace Gecode;
        FanOutSpace* root = new FanOutSpace;
        Search::Options options;
        options.c_d = 2;
        options.a_d = 64;
        DFS<FanOutSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (FanOutSpace* solution = dfs.next()) {
          const WordValue shared =
            (solution->x.val()+solution->y.val()) & 7U;
          const WordValue expected =
            ((shared ^ solution->x.val()) +
             (shared | solution->y.val())) & 7U;
          const bool ok = solution->result.assigned() &&
            (solution->result.val() == expected);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 64;
      }

    public:
      SharedLoweringLifecycle(void)
        : Base("Word::MiniModel::SharedLoweringLifecycle") {}
      virtual bool run(void) {
        if (!mixed_sharing() || !mixed_exception())
          return false;
        return linear_actor_growth() && posting_independence() &&
          fan_out_semantics_and_recomputation();
      }
    };

    class BoundedBooleanConditionalLifecycle : public Base {
    private:
      class PolicySpace : public Gecode::Space {
      public:
        Gecode::BoolVar outer, inner;
        Gecode::WordVar then_word, else_word, direct, expression, fallback;
        PolicySpace(Gecode::WordDomainType kind)
          : outer(*this,0,1), inner(*this,0,1),
            then_word(*this,4,kind,
                      kind == Gecode::WDT_SIGNED ? 12U : 2U,
                      kind == Gecode::WDT_SIGNED ? 14U : 4U),
            else_word(*this,4,kind,
                      kind == Gecode::WDT_SIGNED ? 1U : 9U,
                      kind == Gecode::WDT_SIGNED ? 3U : 11U),
            direct(*this,4,kind), expression(), fallback() {
          using namespace Gecode;
          BoolExpr control=BoolExpr(outer) && BoolExpr(inner);
          ite(*this,expr(*this,control),then_word,else_word,direct);
          WordExpr conditional=ite(control,WordExpr(then_word),
                                   WordExpr(else_word));
          expression=conditional.post(*this,kind);
          fallback=conditional.post(*this);
        }
        PolicySpace(PolicySpace& s) : Space(s) {
          outer.update(*this,s.outer); inner.update(*this,s.inner);
          then_word.update(*this,s.then_word); else_word.update(*this,s.else_word);
          direct.update(*this,s.direct); expression.update(*this,s.expression);
          fallback.update(*this,s.fallback);
        }
        virtual Space* copy(void) { return new PolicySpace(*this); }
      };

      static bool policy(Gecode::WordDomainType kind) {
        using namespace Gecode;
        PolicySpace s(kind);
        const WordValue expected_min = kind == WDT_SIGNED ? 12U : 2U;
        const WordValue expected_max = kind == WDT_SIGNED ? 3U : 11U;
        if ((s.status() == SS_FAILED) ||
            (s.expression.domain_type() != kind) ||
            (s.fallback.domain_type() != WDT_CUBE) ||
            (s.expression.minimum() != expected_min) ||
            (s.expression.maximum() != expected_max) ||
            (s.direct.minimum() != s.expression.minimum()) ||
            (s.direct.maximum() != s.expression.maximum()))
          return false;
        rel(s,s.outer,IRT_EQ,1);
        rel(s,s.inner,IRT_EQ,1);
        return (s.status() != SS_FAILED) &&
          (s.direct.minimum() == s.then_word.minimum()) &&
          (s.direct.maximum() == s.then_word.maximum()) &&
          (s.expression.minimum() == s.then_word.minimum()) &&
          (s.expression.maximum() == s.then_word.maximum());
      }

      static bool alias(void) {
        using namespace Gecode;
        class AliasSpace : public Space {
        public:
          BoolVar outer, inner;
          WordVar cube, bounded, result;
          AliasSpace(void)
            : outer(*this,1,1), inner(*this,1,1), cube(*this,4,3U,3U),
              bounded(*this,4,WDT_SIGNED,13U,13U), result() {
            WordExpr shared=WordExpr(cube)+WordExpr(bounded);
            result=ite(BoolExpr(outer) && BoolExpr(inner),shared,shared)
              .post(*this,WDT_UNSIGNED);
          }
          AliasSpace(AliasSpace& s) : Space(s) {
            outer.update(*this,s.outer); inner.update(*this,s.inner);
            cube.update(*this,s.cube); bounded.update(*this,s.bounded);
            result.update(*this,s.result);
          }
          Space* copy(void) { return new AliasSpace(*this); }
        } s;
        return (s.status() != SS_FAILED) &&
          (s.result.domain_type() == WDT_UNSIGNED) &&
          s.result.assigned() && (s.result.val() == 0U);
      }

    public:
      BoundedBooleanConditionalLifecycle(void)
        : Base("Word::MiniModel::BoundedBooleanConditionalLifecycle") {}
      virtual bool run(void) {
        return policy(Gecode::WDT_UNSIGNED) && policy(Gecode::WDT_SIGNED) &&
          alias();
      }
    };

    class RequestedPolicyLifecycle : public Base {
    private:
      enum { result_count=33 };
      class PolicySpace : public Gecode::Space {
      public:
        Gecode::WordVar x, y;
        Gecode::WordVarArray result;
        Gecode::BoolVar relation, reduction;
        bool representation_ok;
        PolicySpace(Gecode::WordDomainType kind)
          : x(*this,3), y(*this,3,Gecode::WDT_SIGNED),
            result(*this,result_count), representation_ok(true) {
          using namespace Gecode;
          WordExpr a(x), b(y), shared=a^(b+WordExpr(3,1));
          std::vector<WordExpr> expressions={
            ~a, a&b, a|b, a^b, nand(a,b), nor(a,b), xnor(a,b),
            ite(bit(a,0,kind),a,b), ite(a,b,~b),
            extract(concat(a,b),2,3), concat(a,b), repeat(a,2),
            zero_extend(a,6), sign_extend(a,6), a<<1,
            logical_shift_right(a,1), arithmetic_shift_right(a,1),
            rotate_left(a,1), rotate_right(a,1), a<<b,
            logical_shift_right(a,b), arithmetic_shift_right(a,b),
            -a, a+b, a-b, a*b, div(a,b), mod(a,b),
            signed_div(a,b), signed_rem(a,b), signed_mod(a,b),
            extract(concat(shared,~shared),1,3)+WordExpr(3,1),
            ite(word_rel(shared,WRT_EQ,a,kind) || bit(shared,0,kind),
                shared,~shared)
          };
          Gecode::MiniModel::WordPostContext context(*this);
          for (int i=0; i<result_count; i++)
            result[i]=expressions[i].post(*this,kind,&context);
          // A shared expression is the MiniModel counterpart of a let-bound
          // value: repeated word and Boolean uses must share its lowering.
          WordVar first=shared.post(*this,kind,&context);
          WordVar again=shared.post(*this,kind,&context);
          representation_ok=first.varimp()==again.varimp();
          relation=word_rel(shared,WRT_EQ,a,kind).expr(
            *this,IntPropLevels::def,&context);
          reduction=reduce_xor(shared,kind).expr(
            *this,IntPropLevels::def,&context);
          WordVar leaf=a.post(*this,kind,&context);
          WordVar signed_leaf=b.post(*this,kind,&context);
          representation_ok=representation_ok && leaf.varimp()==x.varimp() &&
            signed_leaf.varimp()==y.varimp() &&
            leaf.domain_type()==WDT_CUBE && signed_leaf.domain_type()==WDT_SIGNED;
          for (const auto& entry:context.cache) {
            WordVar value=entry.second;
            if (value.varimp()!=x.varimp() && value.varimp()!=y.varimp())
              representation_ok=representation_ok && value.domain_type()==kind;
          }
          // Default posting remains cube even when a leaf has signed bounds.
          representation_ok=representation_ok &&
            (a+b).post(*this).domain_type()==WDT_CUBE;
          WordVarArgs decisions={x,y};
          branch(*this,decisions,WORD_VAR_NONE(),WORD_VAL_LSB());
        }
        PolicySpace(PolicySpace& s) : Gecode::Space(s),
          representation_ok(s.representation_ok) {
          x.update(*this,s.x); y.update(*this,s.y);
          result.update(*this,s.result);
          relation.update(*this,s.relation); reduction.update(*this,s.reduction);
        }
        Gecode::Space* copy(void) { return new PolicySpace(*this); }
      };
      static int signed_value(unsigned int value) {
        return (value&4U) ? static_cast<int>(value)-8 : static_cast<int>(value);
      }
      static unsigned int ashr(unsigned int x, unsigned int shift) {
        if (shift>=3) return (x&4U) ? 7U : 0U;
        return ((x>>shift) | ((x&4U) ? (7U<<(3-shift)) : 0U))&7U;
      }
      static unsigned int expected(unsigned int op,unsigned int x,unsigned int y) {
        const int sx=signed_value(x),sy=signed_value(y);
        const unsigned int shared=x^((y+1U)&7U);
        switch (op) {
        case 0: return (~x)&7U;
        case 1: return x&y;
        case 2: return x|y;
        case 3: return x^y;
        case 4: return (~(x&y))&7U;
        case 5: return (~(x|y))&7U;
        case 6: return (~(x^y))&7U;
        case 7: return (x&1U) ? x : y;
        case 8: return ((x&y)|((~x)&(~y)))&7U;
        case 9: return (((x<<3)|y)>>2)&7U;
        case 10: return (x<<3)|y;
        case 11: return (x<<3)|x;
        case 12: return x;
        case 13: return (x&4U) ? (x|56U) : x;
        case 14: return (x<<1)&7U;
        case 15: return x>>1;
        case 16: return ashr(x,1);
        case 17: return ((x<<1)|(x>>2))&7U;
        case 18: return ((x>>1)|(x<<2))&7U;
        case 19: return y>=3 ? 0U : (x<<y)&7U;
        case 20: return y>=3 ? 0U : x>>y;
        case 21: return ashr(x,y);
        case 22: return (0U-x)&7U;
        case 23: return (x+y)&7U;
        case 24: return (x-y)&7U;
        case 25: return (x*y)&7U;
        case 26: return y==0 ? 7U : x/y;
        case 27: return y==0 ? x : x%y;
        case 28: return sy==0 ? (sx<0 ? 1U : 7U) :
          static_cast<unsigned int>(sx/sy)&7U;
        case 29: return sy==0 ? x : static_cast<unsigned int>(sx%sy)&7U;
        case 30: {
          if (sy==0) return x;
          int rem=sx%sy;
          if (rem!=0 && ((rem<0)!=(sy<0))) rem+=sy;
          return static_cast<unsigned int>(rem)&7U;
        }
        case 31: return (((((shared<<3)|((~shared)&7U))>>1)&7U)+1U)&7U;
        case 32: return (shared==x || (shared&1U)) ? shared : (~shared)&7U;
        default: GECODE_NEVER; return 0;
        }
      }
      static bool policy(Gecode::WordDomainType kind) {
        using namespace Gecode;
        PolicySpace* root=new PolicySpace(kind);
        if (!root->representation_ok) { delete root; return false; }
        Search::Options options; options.c_d=2; options.a_d=64;
        DFS<PolicySpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (PolicySpace* s=dfs.next()) {
          bool ok=s->representation_ok && s->x.assigned() && s->y.assigned();
          const unsigned int x=static_cast<unsigned int>(s->x.val());
          const unsigned int y=static_cast<unsigned int>(s->y.val());
          for (int i=0; i<result_count; i++)
            ok=ok && s->result[i].domain_type()==kind &&
              s->result[i].assigned() && s->result[i].val()==expected(i,x,y);
          const unsigned int shared=x^((y+1U)&7U);
          const unsigned int parity=((shared>>2)^(shared>>1)^shared)&1U;
          ok=ok && s->relation.assigned() && s->reduction.assigned() &&
            s->relation.val()==static_cast<int>(shared==x) &&
            s->reduction.val()==static_cast<int>(parity);
          delete s;
          if (!ok) return false;
          solutions++;
        }
        return solutions==64;
      }
    public:
      RequestedPolicyLifecycle(void)
        : Base("Word::MiniModel::RequestedPolicyLifecycle") {}
      bool run(void) {
        return policy(Gecode::WDT_CUBE) && policy(Gecode::WDT_UNSIGNED) &&
          policy(Gecode::WDT_SIGNED);
      }
    };

    RequestedPolicyLifecycle requested_policy_lifecycle;

    Logic logic;
    NamedLogic named_logic;
    BooleanConditional boolean_conditional;
    MaskConditional mask_conditional;
    Relation relation;
    Reduction reduction_and0(Reduction::OP_AND,0,"And");
    Reduction reduction_and1(Reduction::OP_AND,1,"And");
    Reduction reduction_or0(Reduction::OP_OR,0,"Or");
    Reduction reduction_or1(Reduction::OP_OR,1,"Or");
    Reduction reduction_xor0(Reduction::OP_XOR,0,"Xor");
    Reduction reduction_xor1(Reduction::OP_XOR,1,"Xor");
    ReductionComposition reduction_composition;
    Lifecycle lifecycle;
    StructuralLifecycle structural_lifecycle;
    ArithmeticLifecycle arithmetic_lifecycle;
    BooleanPolicyLifecycle boolean_policy_lifecycle;
    SharedLoweringLifecycle shared_lowering_lifecycle;
    BoundedBooleanConditionalLifecycle bounded_boolean_conditional_lifecycle;

  }

}}

// STATISTICS: test-word
