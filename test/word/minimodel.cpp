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

    public:
      Lifecycle(void) : Base("Word::MiniModel::Lifecycle") {}
      virtual bool run(void) {
        return parity() && invalid() && clone() && search_recomputation();
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

    Logic logic;
    NamedLogic named_logic;
    BooleanConditional boolean_conditional;
    MaskConditional mask_conditional;
    Relation relation;
    Lifecycle lifecycle;
    StructuralLifecycle structural_lifecycle;

  }

}}

// STATISTICS: test-word
