/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include "test/int.hh"

#include <gecode/minimodel.hh>

namespace Test { namespace Int {

  /// %Tests for minimal modelling constraints (Boolean)
  namespace MiniModelBoolean {

     /// Boolean opcode
     enum BoolOpcode {
       BO_NOT, ///< Negation
       BO_AND, ///< Conjunction
       BO_OR,  ///< Disjunction
       BO_IMP, ///< Implication
       BO_XOR, ///< Exclusive or
       BO_EQV, ///< Equivalence
       BO_HLT  ///< Stop execution
     };

     /// Type for representing a Boolean instruction
     class BoolInstr {
     public:
       BoolOpcode o; ///< Which instruction to execute
       unsigned char x, y, z;  ///< Instruction arguments, \a z is destination (or \a y for negation)
     };

     /// Executes Boolean instruction for evaluation (checking)
     int
     eval(const BoolInstr* pc, int reg[]) {
       while (true) {
         switch (pc->o) {
         case BO_NOT: reg[pc->y] = !reg[pc->x]; break;
         case BO_AND: reg[pc->z] = reg[pc->x] & reg[pc->y]; break;
         case BO_OR:  reg[pc->z] = reg[pc->x] | reg[pc->y]; break;
         case BO_IMP: reg[pc->z] = !reg[pc->x] | reg[pc->y]; break;
         case BO_XOR: reg[pc->z] = reg[pc->x] ^ reg[pc->y]; break;
         case BO_EQV: reg[pc->z] = reg[pc->x] == reg[pc->y]; break;
         case BO_HLT: return reg[pc->x];
         default: GECODE_NEVER;
         }
         pc++;
       }
       GECODE_NEVER;
     }

     /// Executes Boolean instruction for constructing Boolean expressions
     Gecode::BoolExpr
     eval(const BoolInstr* pc, Gecode::BoolExpr reg[]) {
       using namespace Gecode;
       while (true) {
         switch (pc->o) {
         case BO_NOT: reg[pc->y] = !reg[pc->x]; break;
         case BO_AND: reg[pc->z] = reg[pc->x] && reg[pc->y]; break;
         case BO_OR:  reg[pc->z] = reg[pc->x] || reg[pc->y]; break;
         case BO_IMP: reg[pc->z] = (reg[pc->x] >> reg[pc->y]); break;
         case BO_XOR: reg[pc->z] = reg[pc->x] ^ reg[pc->y]; break;
         case BO_EQV: reg[pc->z] = (reg[pc->x] == reg[pc->y]); break;
         case BO_HLT: return reg[pc->x];
         default: GECODE_NEVER;
         }
         pc++;
       }
       GECODE_NEVER;
     }

     /**
      * \defgroup TaskTestIntMiniModelBool Minimal modelling constraints (Boolean constraints)
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test Boolean expressions with integer result
     class BoolExprInt : public Test {
     protected:
       /// Boolean instruction sequence
       const BoolInstr* bis;
       /// Result of expression
       int c;
     public:
       /// Create and register test
       BoolExprInt(const BoolInstr* bis0, const std::string& s, int c0)
         : Test("MiniModel::BoolExpr::Int::"+s+"::"+str(c0),4,0,1),
           bis(bis0), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int reg[4] = {(x[0] != x[2]), x[1],
                       (x[2] > 0), x[3]};
         return eval(bis, reg) == c;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolExpr reg[4] = {
           (x[0] != x[2]),channel(home,x[1]),
           (x[2] > 0),channel(home,x[3])
         };
         if (c == 0)
           Gecode::rel(home, !(eval(bis,reg)));
         else
           Gecode::rel(home, eval(bis,reg));
       }
     };

     /// %Test posting Boolean expressions
     class BoolExprVar : public Test {
     protected:
       /// Boolean instruction sequence
       const BoolInstr* bis;
     public:
       /// Create and register test
       BoolExprVar(const BoolInstr* bis0, const std::string& s)
         : Test("MiniModel::BoolExpr::Var::"+s,5,0,1), bis(bis0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int reg[4] = {(x[0] > x[2]), x[1],
                       (x[2] != 1), x[3]};
         return eval(bis, reg) == x[4];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolExpr reg[4] = {
           (x[0] > x[2]),channel(home,x[1]),
           (channel(home,x[2]) != 1),channel(home,x[3])
         };
         rel(home, Gecode::expr(home, eval(bis,reg)), IRT_EQ,
             channel(home,x[4]));
       }
     };

    const BoolInstr bi000[] = {
      {BO_AND,0,1,0},{BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi001[] = {
      {BO_AND,0,1,0},{BO_AND,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi002[] = {
      {BO_AND,2,3,2},{BO_AND,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi003[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_AND,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi004[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi005[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi006[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi007[] = {
      {BO_AND,0,1,0},{BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi008[] = {
      {BO_AND,0,1,0},{BO_AND,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi009[] = {
      {BO_AND,2,3,2},{BO_AND,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi010[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_AND,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi011[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi012[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi013[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi014[] = {
      {BO_AND,0,1,0},{BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi015[] = {
      {BO_AND,0,1,0},{BO_AND,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi016[] = {
      {BO_AND,2,3,2},{BO_AND,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi017[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_AND,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi018[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi019[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi020[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi021[] = {
      {BO_AND,0,1,0},{BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi022[] = {
      {BO_AND,0,1,0},{BO_AND,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi023[] = {
      {BO_AND,2,3,2},{BO_AND,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi024[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_AND,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi025[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi026[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi027[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi028[] = {
      {BO_AND,0,1,0},{BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi029[] = {
      {BO_AND,0,1,0},{BO_AND,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi030[] = {
      {BO_AND,2,3,2},{BO_AND,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi031[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_AND,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi032[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi033[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi034[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi035[] = {
      {BO_AND,0,1,0},{BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi036[] = {
      {BO_AND,0,1,0},{BO_OR ,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi037[] = {
      {BO_AND,2,3,2},{BO_OR ,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi038[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_OR ,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi039[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi040[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi041[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi042[] = {
      {BO_AND,0,1,0},{BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi043[] = {
      {BO_AND,0,1,0},{BO_OR ,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi044[] = {
      {BO_AND,2,3,2},{BO_OR ,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi045[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_OR ,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi046[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi047[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi048[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi049[] = {
      {BO_AND,0,1,0},{BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi050[] = {
      {BO_AND,0,1,0},{BO_OR ,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi051[] = {
      {BO_AND,2,3,2},{BO_OR ,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi052[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_OR ,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi053[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi054[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi055[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi056[] = {
      {BO_AND,0,1,0},{BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi057[] = {
      {BO_AND,0,1,0},{BO_OR ,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi058[] = {
      {BO_AND,2,3,2},{BO_OR ,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi059[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_OR ,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi060[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi061[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi062[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi063[] = {
      {BO_AND,0,1,0},{BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi064[] = {
      {BO_AND,0,1,0},{BO_OR ,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi065[] = {
      {BO_AND,2,3,2},{BO_OR ,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi066[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_OR ,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi067[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi068[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi069[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi070[] = {
      {BO_AND,0,1,0},{BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi071[] = {
      {BO_AND,0,1,0},{BO_IMP,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi072[] = {
      {BO_AND,2,3,2},{BO_IMP,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi073[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_IMP,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi074[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi075[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi076[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi077[] = {
      {BO_AND,0,1,0},{BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi078[] = {
      {BO_AND,0,1,0},{BO_IMP,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi079[] = {
      {BO_AND,2,3,2},{BO_IMP,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi080[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_IMP,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi081[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi082[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi083[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi084[] = {
      {BO_AND,0,1,0},{BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi085[] = {
      {BO_AND,0,1,0},{BO_IMP,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi086[] = {
      {BO_AND,2,3,2},{BO_IMP,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi087[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_IMP,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi088[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi089[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi090[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi091[] = {
      {BO_AND,0,1,0},{BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi092[] = {
      {BO_AND,0,1,0},{BO_IMP,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi093[] = {
      {BO_AND,2,3,2},{BO_IMP,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi094[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_IMP,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi095[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi096[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi097[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi098[] = {
      {BO_AND,0,1,0},{BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi099[] = {
      {BO_AND,0,1,0},{BO_IMP,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi100[] = {
      {BO_AND,2,3,2},{BO_IMP,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi101[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_IMP,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi102[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi103[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi104[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi105[] = {
      {BO_AND,0,1,0},{BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi106[] = {
      {BO_AND,0,1,0},{BO_XOR,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi107[] = {
      {BO_AND,2,3,2},{BO_XOR,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi108[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_XOR,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi109[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi110[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi111[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi112[] = {
      {BO_AND,0,1,0},{BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi113[] = {
      {BO_AND,0,1,0},{BO_XOR,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi114[] = {
      {BO_AND,2,3,2},{BO_XOR,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi115[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_XOR,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi116[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi117[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi118[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi119[] = {
      {BO_AND,0,1,0},{BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi120[] = {
      {BO_AND,0,1,0},{BO_XOR,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi121[] = {
      {BO_AND,2,3,2},{BO_XOR,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi122[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_XOR,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi123[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi124[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi125[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi126[] = {
      {BO_AND,0,1,0},{BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi127[] = {
      {BO_AND,0,1,0},{BO_XOR,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi128[] = {
      {BO_AND,2,3,2},{BO_XOR,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi129[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_XOR,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi130[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi131[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi132[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi133[] = {
      {BO_AND,0,1,0},{BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi134[] = {
      {BO_AND,0,1,0},{BO_XOR,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi135[] = {
      {BO_AND,2,3,2},{BO_XOR,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi136[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_XOR,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi137[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi138[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi139[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi140[] = {
      {BO_AND,0,1,0},{BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi141[] = {
      {BO_AND,0,1,0},{BO_EQV,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi142[] = {
      {BO_AND,2,3,2},{BO_EQV,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi143[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_EQV,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi144[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi145[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi146[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi147[] = {
      {BO_AND,0,1,0},{BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi148[] = {
      {BO_AND,0,1,0},{BO_EQV,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi149[] = {
      {BO_AND,2,3,2},{BO_EQV,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi150[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_EQV,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi151[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi152[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi153[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi154[] = {
      {BO_AND,0,1,0},{BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi155[] = {
      {BO_AND,0,1,0},{BO_EQV,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi156[] = {
      {BO_AND,2,3,2},{BO_EQV,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi157[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_EQV,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi158[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi159[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi160[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi161[] = {
      {BO_AND,0,1,0},{BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi162[] = {
      {BO_AND,0,1,0},{BO_EQV,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi163[] = {
      {BO_AND,2,3,2},{BO_EQV,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi164[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_EQV,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi165[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi166[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi167[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi168[] = {
      {BO_AND,0,1,0},{BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi169[] = {
      {BO_AND,0,1,0},{BO_EQV,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi170[] = {
      {BO_AND,2,3,2},{BO_EQV,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi171[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_AND,0,1,0},{BO_EQV,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi172[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_AND,0,1,0},
      {BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi173[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi174[] = {
      {BO_AND,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi175[] = {
      {BO_OR ,0,1,0},{BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi176[] = {
      {BO_OR ,0,1,0},{BO_AND,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi177[] = {
      {BO_OR ,2,3,2},{BO_AND,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi178[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_AND,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi179[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi180[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi181[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi182[] = {
      {BO_OR ,0,1,0},{BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi183[] = {
      {BO_OR ,0,1,0},{BO_AND,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi184[] = {
      {BO_OR ,2,3,2},{BO_AND,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi185[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_AND,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi186[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi187[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi188[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi189[] = {
      {BO_OR ,0,1,0},{BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi190[] = {
      {BO_OR ,0,1,0},{BO_AND,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi191[] = {
      {BO_OR ,2,3,2},{BO_AND,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi192[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_AND,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi193[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi194[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi195[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi196[] = {
      {BO_OR ,0,1,0},{BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi197[] = {
      {BO_OR ,0,1,0},{BO_AND,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi198[] = {
      {BO_OR ,2,3,2},{BO_AND,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi199[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_AND,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi200[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi201[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi202[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi203[] = {
      {BO_OR ,0,1,0},{BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi204[] = {
      {BO_OR ,0,1,0},{BO_AND,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi205[] = {
      {BO_OR ,2,3,2},{BO_AND,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi206[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_AND,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi207[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi208[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi209[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi210[] = {
      {BO_OR ,0,1,0},{BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi211[] = {
      {BO_OR ,0,1,0},{BO_OR ,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi212[] = {
      {BO_OR ,2,3,2},{BO_OR ,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi213[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_OR ,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi214[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi215[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi216[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi217[] = {
      {BO_OR ,0,1,0},{BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi218[] = {
      {BO_OR ,0,1,0},{BO_OR ,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi219[] = {
      {BO_OR ,2,3,2},{BO_OR ,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi220[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_OR ,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi221[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi222[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi223[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi224[] = {
      {BO_OR ,0,1,0},{BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi225[] = {
      {BO_OR ,0,1,0},{BO_OR ,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi226[] = {
      {BO_OR ,2,3,2},{BO_OR ,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi227[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_OR ,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi228[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi229[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi230[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi231[] = {
      {BO_OR ,0,1,0},{BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi232[] = {
      {BO_OR ,0,1,0},{BO_OR ,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi233[] = {
      {BO_OR ,2,3,2},{BO_OR ,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi234[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_OR ,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi235[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi236[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi237[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi238[] = {
      {BO_OR ,0,1,0},{BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi239[] = {
      {BO_OR ,0,1,0},{BO_OR ,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi240[] = {
      {BO_OR ,2,3,2},{BO_OR ,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi241[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_OR ,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi242[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi243[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi244[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi245[] = {
      {BO_OR ,0,1,0},{BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi246[] = {
      {BO_OR ,0,1,0},{BO_IMP,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi247[] = {
      {BO_OR ,2,3,2},{BO_IMP,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi248[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_IMP,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi249[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi250[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi251[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi252[] = {
      {BO_OR ,0,1,0},{BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi253[] = {
      {BO_OR ,0,1,0},{BO_IMP,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi254[] = {
      {BO_OR ,2,3,2},{BO_IMP,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi255[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_IMP,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi256[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi257[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi258[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi259[] = {
      {BO_OR ,0,1,0},{BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi260[] = {
      {BO_OR ,0,1,0},{BO_IMP,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi261[] = {
      {BO_OR ,2,3,2},{BO_IMP,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi262[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_IMP,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi263[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi264[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi265[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi266[] = {
      {BO_OR ,0,1,0},{BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi267[] = {
      {BO_OR ,0,1,0},{BO_IMP,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi268[] = {
      {BO_OR ,2,3,2},{BO_IMP,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi269[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_IMP,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi270[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi271[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi272[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi273[] = {
      {BO_OR ,0,1,0},{BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi274[] = {
      {BO_OR ,0,1,0},{BO_IMP,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi275[] = {
      {BO_OR ,2,3,2},{BO_IMP,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi276[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_IMP,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi277[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi278[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi279[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi280[] = {
      {BO_OR ,0,1,0},{BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi281[] = {
      {BO_OR ,0,1,0},{BO_XOR,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi282[] = {
      {BO_OR ,2,3,2},{BO_XOR,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi283[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_XOR,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi284[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi285[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi286[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi287[] = {
      {BO_OR ,0,1,0},{BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi288[] = {
      {BO_OR ,0,1,0},{BO_XOR,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi289[] = {
      {BO_OR ,2,3,2},{BO_XOR,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi290[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_XOR,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi291[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi292[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi293[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi294[] = {
      {BO_OR ,0,1,0},{BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi295[] = {
      {BO_OR ,0,1,0},{BO_XOR,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi296[] = {
      {BO_OR ,2,3,2},{BO_XOR,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi297[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_XOR,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi298[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi299[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi300[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi301[] = {
      {BO_OR ,0,1,0},{BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi302[] = {
      {BO_OR ,0,1,0},{BO_XOR,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi303[] = {
      {BO_OR ,2,3,2},{BO_XOR,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi304[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_XOR,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi305[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi306[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi307[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi308[] = {
      {BO_OR ,0,1,0},{BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi309[] = {
      {BO_OR ,0,1,0},{BO_XOR,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi310[] = {
      {BO_OR ,2,3,2},{BO_XOR,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi311[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_XOR,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi312[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi313[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi314[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi315[] = {
      {BO_OR ,0,1,0},{BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi316[] = {
      {BO_OR ,0,1,0},{BO_EQV,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi317[] = {
      {BO_OR ,2,3,2},{BO_EQV,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi318[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_EQV,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi319[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi320[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi321[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi322[] = {
      {BO_OR ,0,1,0},{BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi323[] = {
      {BO_OR ,0,1,0},{BO_EQV,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi324[] = {
      {BO_OR ,2,3,2},{BO_EQV,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi325[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_EQV,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi326[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi327[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi328[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi329[] = {
      {BO_OR ,0,1,0},{BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi330[] = {
      {BO_OR ,0,1,0},{BO_EQV,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi331[] = {
      {BO_OR ,2,3,2},{BO_EQV,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi332[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_EQV,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi333[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi334[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi335[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi336[] = {
      {BO_OR ,0,1,0},{BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi337[] = {
      {BO_OR ,0,1,0},{BO_EQV,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi338[] = {
      {BO_OR ,2,3,2},{BO_EQV,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi339[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_EQV,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi340[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi341[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi342[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi343[] = {
      {BO_OR ,0,1,0},{BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi344[] = {
      {BO_OR ,0,1,0},{BO_EQV,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi345[] = {
      {BO_OR ,2,3,2},{BO_EQV,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi346[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_OR ,0,1,0},{BO_EQV,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi347[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_OR ,0,1,0},
      {BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi348[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi349[] = {
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi350[] = {
      {BO_IMP,0,1,0},{BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi351[] = {
      {BO_IMP,0,1,0},{BO_AND,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi352[] = {
      {BO_IMP,2,3,2},{BO_AND,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi353[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_AND,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi354[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi355[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi356[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi357[] = {
      {BO_IMP,0,1,0},{BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi358[] = {
      {BO_IMP,0,1,0},{BO_AND,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi359[] = {
      {BO_IMP,2,3,2},{BO_AND,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi360[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_AND,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi361[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi362[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi363[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi364[] = {
      {BO_IMP,0,1,0},{BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi365[] = {
      {BO_IMP,0,1,0},{BO_AND,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi366[] = {
      {BO_IMP,2,3,2},{BO_AND,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi367[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_AND,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi368[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi369[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi370[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi371[] = {
      {BO_IMP,0,1,0},{BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi372[] = {
      {BO_IMP,0,1,0},{BO_AND,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi373[] = {
      {BO_IMP,2,3,2},{BO_AND,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi374[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_AND,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi375[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi376[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi377[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi378[] = {
      {BO_IMP,0,1,0},{BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi379[] = {
      {BO_IMP,0,1,0},{BO_AND,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi380[] = {
      {BO_IMP,2,3,2},{BO_AND,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi381[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_AND,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi382[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi383[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi384[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi385[] = {
      {BO_IMP,0,1,0},{BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi386[] = {
      {BO_IMP,0,1,0},{BO_OR ,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi387[] = {
      {BO_IMP,2,3,2},{BO_OR ,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi388[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_OR ,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi389[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi390[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi391[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi392[] = {
      {BO_IMP,0,1,0},{BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi393[] = {
      {BO_IMP,0,1,0},{BO_OR ,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi394[] = {
      {BO_IMP,2,3,2},{BO_OR ,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi395[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_OR ,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi396[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi397[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi398[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi399[] = {
      {BO_IMP,0,1,0},{BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi400[] = {
      {BO_IMP,0,1,0},{BO_OR ,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi401[] = {
      {BO_IMP,2,3,2},{BO_OR ,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi402[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_OR ,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi403[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi404[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi405[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi406[] = {
      {BO_IMP,0,1,0},{BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi407[] = {
      {BO_IMP,0,1,0},{BO_OR ,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi408[] = {
      {BO_IMP,2,3,2},{BO_OR ,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi409[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_OR ,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi410[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi411[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi412[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi413[] = {
      {BO_IMP,0,1,0},{BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi414[] = {
      {BO_IMP,0,1,0},{BO_OR ,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi415[] = {
      {BO_IMP,2,3,2},{BO_OR ,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi416[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_OR ,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi417[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi418[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi419[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi420[] = {
      {BO_IMP,0,1,0},{BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi421[] = {
      {BO_IMP,0,1,0},{BO_IMP,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi422[] = {
      {BO_IMP,2,3,2},{BO_IMP,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi423[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_IMP,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi424[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi425[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi426[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi427[] = {
      {BO_IMP,0,1,0},{BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi428[] = {
      {BO_IMP,0,1,0},{BO_IMP,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi429[] = {
      {BO_IMP,2,3,2},{BO_IMP,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi430[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_IMP,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi431[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi432[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi433[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi434[] = {
      {BO_IMP,0,1,0},{BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi435[] = {
      {BO_IMP,0,1,0},{BO_IMP,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi436[] = {
      {BO_IMP,2,3,2},{BO_IMP,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi437[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_IMP,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi438[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi439[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi440[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi441[] = {
      {BO_IMP,0,1,0},{BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi442[] = {
      {BO_IMP,0,1,0},{BO_IMP,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi443[] = {
      {BO_IMP,2,3,2},{BO_IMP,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi444[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_IMP,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi445[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi446[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi447[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi448[] = {
      {BO_IMP,0,1,0},{BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi449[] = {
      {BO_IMP,0,1,0},{BO_IMP,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi450[] = {
      {BO_IMP,2,3,2},{BO_IMP,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi451[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_IMP,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi452[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi453[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi454[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi455[] = {
      {BO_IMP,0,1,0},{BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi456[] = {
      {BO_IMP,0,1,0},{BO_XOR,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi457[] = {
      {BO_IMP,2,3,2},{BO_XOR,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi458[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_XOR,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi459[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi460[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi461[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi462[] = {
      {BO_IMP,0,1,0},{BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi463[] = {
      {BO_IMP,0,1,0},{BO_XOR,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi464[] = {
      {BO_IMP,2,3,2},{BO_XOR,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi465[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_XOR,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi466[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi467[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi468[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi469[] = {
      {BO_IMP,0,1,0},{BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi470[] = {
      {BO_IMP,0,1,0},{BO_XOR,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi471[] = {
      {BO_IMP,2,3,2},{BO_XOR,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi472[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_XOR,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi473[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi474[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi475[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi476[] = {
      {BO_IMP,0,1,0},{BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi477[] = {
      {BO_IMP,0,1,0},{BO_XOR,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi478[] = {
      {BO_IMP,2,3,2},{BO_XOR,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi479[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_XOR,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi480[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi481[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi482[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi483[] = {
      {BO_IMP,0,1,0},{BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi484[] = {
      {BO_IMP,0,1,0},{BO_XOR,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi485[] = {
      {BO_IMP,2,3,2},{BO_XOR,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi486[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_XOR,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi487[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi488[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi489[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi490[] = {
      {BO_IMP,0,1,0},{BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi491[] = {
      {BO_IMP,0,1,0},{BO_EQV,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi492[] = {
      {BO_IMP,2,3,2},{BO_EQV,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi493[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_EQV,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi494[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi495[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi496[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi497[] = {
      {BO_IMP,0,1,0},{BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi498[] = {
      {BO_IMP,0,1,0},{BO_EQV,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi499[] = {
      {BO_IMP,2,3,2},{BO_EQV,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi500[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_EQV,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi501[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi502[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi503[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi504[] = {
      {BO_IMP,0,1,0},{BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi505[] = {
      {BO_IMP,0,1,0},{BO_EQV,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi506[] = {
      {BO_IMP,2,3,2},{BO_EQV,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi507[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_EQV,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi508[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi509[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi510[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi511[] = {
      {BO_IMP,0,1,0},{BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi512[] = {
      {BO_IMP,0,1,0},{BO_EQV,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi513[] = {
      {BO_IMP,2,3,2},{BO_EQV,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi514[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_EQV,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi515[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi516[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi517[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi518[] = {
      {BO_IMP,0,1,0},{BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi519[] = {
      {BO_IMP,0,1,0},{BO_EQV,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi520[] = {
      {BO_IMP,2,3,2},{BO_EQV,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi521[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_IMP,0,1,0},{BO_EQV,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi522[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_IMP,0,1,0},
      {BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi523[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi524[] = {
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi525[] = {
      {BO_XOR,0,1,0},{BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi526[] = {
      {BO_XOR,0,1,0},{BO_AND,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi527[] = {
      {BO_XOR,2,3,2},{BO_AND,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi528[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_AND,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi529[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi530[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi531[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi532[] = {
      {BO_XOR,0,1,0},{BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi533[] = {
      {BO_XOR,0,1,0},{BO_AND,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi534[] = {
      {BO_XOR,2,3,2},{BO_AND,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi535[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_AND,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi536[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi537[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi538[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi539[] = {
      {BO_XOR,0,1,0},{BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi540[] = {
      {BO_XOR,0,1,0},{BO_AND,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi541[] = {
      {BO_XOR,2,3,2},{BO_AND,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi542[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_AND,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi543[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi544[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi545[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi546[] = {
      {BO_XOR,0,1,0},{BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi547[] = {
      {BO_XOR,0,1,0},{BO_AND,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi548[] = {
      {BO_XOR,2,3,2},{BO_AND,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi549[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_AND,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi550[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi551[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi552[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi553[] = {
      {BO_XOR,0,1,0},{BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi554[] = {
      {BO_XOR,0,1,0},{BO_AND,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi555[] = {
      {BO_XOR,2,3,2},{BO_AND,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi556[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_AND,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi557[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi558[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi559[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi560[] = {
      {BO_XOR,0,1,0},{BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi561[] = {
      {BO_XOR,0,1,0},{BO_OR ,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi562[] = {
      {BO_XOR,2,3,2},{BO_OR ,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi563[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_OR ,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi564[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi565[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi566[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi567[] = {
      {BO_XOR,0,1,0},{BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi568[] = {
      {BO_XOR,0,1,0},{BO_OR ,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi569[] = {
      {BO_XOR,2,3,2},{BO_OR ,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi570[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_OR ,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi571[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi572[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi573[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi574[] = {
      {BO_XOR,0,1,0},{BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi575[] = {
      {BO_XOR,0,1,0},{BO_OR ,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi576[] = {
      {BO_XOR,2,3,2},{BO_OR ,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi577[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_OR ,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi578[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi579[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi580[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi581[] = {
      {BO_XOR,0,1,0},{BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi582[] = {
      {BO_XOR,0,1,0},{BO_OR ,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi583[] = {
      {BO_XOR,2,3,2},{BO_OR ,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi584[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_OR ,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi585[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi586[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi587[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi588[] = {
      {BO_XOR,0,1,0},{BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi589[] = {
      {BO_XOR,0,1,0},{BO_OR ,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi590[] = {
      {BO_XOR,2,3,2},{BO_OR ,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi591[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_OR ,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi592[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi593[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi594[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi595[] = {
      {BO_XOR,0,1,0},{BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi596[] = {
      {BO_XOR,0,1,0},{BO_IMP,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi597[] = {
      {BO_XOR,2,3,2},{BO_IMP,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi598[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_IMP,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi599[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi600[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi601[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi602[] = {
      {BO_XOR,0,1,0},{BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi603[] = {
      {BO_XOR,0,1,0},{BO_IMP,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi604[] = {
      {BO_XOR,2,3,2},{BO_IMP,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi605[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_IMP,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi606[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi607[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi608[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi609[] = {
      {BO_XOR,0,1,0},{BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi610[] = {
      {BO_XOR,0,1,0},{BO_IMP,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi611[] = {
      {BO_XOR,2,3,2},{BO_IMP,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi612[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_IMP,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi613[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi614[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi615[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi616[] = {
      {BO_XOR,0,1,0},{BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi617[] = {
      {BO_XOR,0,1,0},{BO_IMP,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi618[] = {
      {BO_XOR,2,3,2},{BO_IMP,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi619[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_IMP,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi620[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi621[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi622[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi623[] = {
      {BO_XOR,0,1,0},{BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi624[] = {
      {BO_XOR,0,1,0},{BO_IMP,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi625[] = {
      {BO_XOR,2,3,2},{BO_IMP,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi626[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_IMP,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi627[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi628[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi629[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi630[] = {
      {BO_XOR,0,1,0},{BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi631[] = {
      {BO_XOR,0,1,0},{BO_XOR,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi632[] = {
      {BO_XOR,2,3,2},{BO_XOR,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi633[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_XOR,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi634[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi635[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi636[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi637[] = {
      {BO_XOR,0,1,0},{BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi638[] = {
      {BO_XOR,0,1,0},{BO_XOR,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi639[] = {
      {BO_XOR,2,3,2},{BO_XOR,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi640[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_XOR,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi641[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi642[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi643[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi644[] = {
      {BO_XOR,0,1,0},{BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi645[] = {
      {BO_XOR,0,1,0},{BO_XOR,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi646[] = {
      {BO_XOR,2,3,2},{BO_XOR,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi647[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_XOR,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi648[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi649[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi650[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi651[] = {
      {BO_XOR,0,1,0},{BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi652[] = {
      {BO_XOR,0,1,0},{BO_XOR,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi653[] = {
      {BO_XOR,2,3,2},{BO_XOR,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi654[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_XOR,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi655[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi656[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi657[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi658[] = {
      {BO_XOR,0,1,0},{BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi659[] = {
      {BO_XOR,0,1,0},{BO_XOR,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi660[] = {
      {BO_XOR,2,3,2},{BO_XOR,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi661[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_XOR,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi662[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi663[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi664[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi665[] = {
      {BO_XOR,0,1,0},{BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi666[] = {
      {BO_XOR,0,1,0},{BO_EQV,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi667[] = {
      {BO_XOR,2,3,2},{BO_EQV,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi668[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_EQV,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi669[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi670[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi671[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi672[] = {
      {BO_XOR,0,1,0},{BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi673[] = {
      {BO_XOR,0,1,0},{BO_EQV,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi674[] = {
      {BO_XOR,2,3,2},{BO_EQV,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi675[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_EQV,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi676[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi677[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi678[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi679[] = {
      {BO_XOR,0,1,0},{BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi680[] = {
      {BO_XOR,0,1,0},{BO_EQV,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi681[] = {
      {BO_XOR,2,3,2},{BO_EQV,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi682[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_EQV,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi683[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi684[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi685[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi686[] = {
      {BO_XOR,0,1,0},{BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi687[] = {
      {BO_XOR,0,1,0},{BO_EQV,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi688[] = {
      {BO_XOR,2,3,2},{BO_EQV,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi689[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_EQV,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi690[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi691[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi692[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi693[] = {
      {BO_XOR,0,1,0},{BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi694[] = {
      {BO_XOR,0,1,0},{BO_EQV,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi695[] = {
      {BO_XOR,2,3,2},{BO_EQV,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi696[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_XOR,0,1,0},{BO_EQV,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi697[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_XOR,0,1,0},
      {BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi698[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi699[] = {
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi700[] = {
      {BO_EQV,0,1,0},{BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi701[] = {
      {BO_EQV,0,1,0},{BO_AND,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi702[] = {
      {BO_EQV,2,3,2},{BO_AND,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi703[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_AND,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi704[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_AND,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi705[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi706[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi707[] = {
      {BO_EQV,0,1,0},{BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi708[] = {
      {BO_EQV,0,1,0},{BO_AND,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi709[] = {
      {BO_EQV,2,3,2},{BO_AND,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi710[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_AND,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi711[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_AND,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi712[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi713[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi714[] = {
      {BO_EQV,0,1,0},{BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi715[] = {
      {BO_EQV,0,1,0},{BO_AND,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi716[] = {
      {BO_EQV,2,3,2},{BO_AND,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi717[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_AND,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi718[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_AND,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi719[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi720[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi721[] = {
      {BO_EQV,0,1,0},{BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi722[] = {
      {BO_EQV,0,1,0},{BO_AND,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi723[] = {
      {BO_EQV,2,3,2},{BO_AND,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi724[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_AND,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi725[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_AND,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi726[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi727[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi728[] = {
      {BO_EQV,0,1,0},{BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi729[] = {
      {BO_EQV,0,1,0},{BO_AND,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi730[] = {
      {BO_EQV,2,3,2},{BO_AND,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi731[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_AND,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi732[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_AND,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi733[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi734[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_AND,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi735[] = {
      {BO_EQV,0,1,0},{BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi736[] = {
      {BO_EQV,0,1,0},{BO_OR ,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi737[] = {
      {BO_EQV,2,3,2},{BO_OR ,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi738[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_OR ,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi739[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_OR ,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi740[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi741[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi742[] = {
      {BO_EQV,0,1,0},{BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi743[] = {
      {BO_EQV,0,1,0},{BO_OR ,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi744[] = {
      {BO_EQV,2,3,2},{BO_OR ,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi745[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_OR ,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi746[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_OR ,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi747[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi748[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi749[] = {
      {BO_EQV,0,1,0},{BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi750[] = {
      {BO_EQV,0,1,0},{BO_OR ,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi751[] = {
      {BO_EQV,2,3,2},{BO_OR ,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi752[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_OR ,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi753[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_OR ,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi754[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi755[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi756[] = {
      {BO_EQV,0,1,0},{BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi757[] = {
      {BO_EQV,0,1,0},{BO_OR ,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi758[] = {
      {BO_EQV,2,3,2},{BO_OR ,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi759[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_OR ,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi760[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_OR ,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi761[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi762[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi763[] = {
      {BO_EQV,0,1,0},{BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi764[] = {
      {BO_EQV,0,1,0},{BO_OR ,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi765[] = {
      {BO_EQV,2,3,2},{BO_OR ,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi766[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_OR ,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi767[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_OR ,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi768[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi769[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_OR ,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi770[] = {
      {BO_EQV,0,1,0},{BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi771[] = {
      {BO_EQV,0,1,0},{BO_IMP,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi772[] = {
      {BO_EQV,2,3,2},{BO_IMP,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi773[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_IMP,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi774[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_IMP,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi775[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi776[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi777[] = {
      {BO_EQV,0,1,0},{BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi778[] = {
      {BO_EQV,0,1,0},{BO_IMP,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi779[] = {
      {BO_EQV,2,3,2},{BO_IMP,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi780[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_IMP,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi781[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_IMP,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi782[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi783[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi784[] = {
      {BO_EQV,0,1,0},{BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi785[] = {
      {BO_EQV,0,1,0},{BO_IMP,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi786[] = {
      {BO_EQV,2,3,2},{BO_IMP,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi787[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_IMP,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi788[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_IMP,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi789[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi790[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi791[] = {
      {BO_EQV,0,1,0},{BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi792[] = {
      {BO_EQV,0,1,0},{BO_IMP,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi793[] = {
      {BO_EQV,2,3,2},{BO_IMP,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi794[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_IMP,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi795[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_IMP,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi796[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi797[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi798[] = {
      {BO_EQV,0,1,0},{BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi799[] = {
      {BO_EQV,0,1,0},{BO_IMP,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi800[] = {
      {BO_EQV,2,3,2},{BO_IMP,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi801[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_IMP,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi802[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_IMP,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi803[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi804[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_IMP,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi805[] = {
      {BO_EQV,0,1,0},{BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi806[] = {
      {BO_EQV,0,1,0},{BO_XOR,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi807[] = {
      {BO_EQV,2,3,2},{BO_XOR,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi808[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_XOR,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi809[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_XOR,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi810[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi811[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi812[] = {
      {BO_EQV,0,1,0},{BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi813[] = {
      {BO_EQV,0,1,0},{BO_XOR,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi814[] = {
      {BO_EQV,2,3,2},{BO_XOR,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi815[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_XOR,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi816[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_XOR,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi817[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi818[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi819[] = {
      {BO_EQV,0,1,0},{BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi820[] = {
      {BO_EQV,0,1,0},{BO_XOR,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi821[] = {
      {BO_EQV,2,3,2},{BO_XOR,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi822[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_XOR,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi823[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_XOR,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi824[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi825[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi826[] = {
      {BO_EQV,0,1,0},{BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi827[] = {
      {BO_EQV,0,1,0},{BO_XOR,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi828[] = {
      {BO_EQV,2,3,2},{BO_XOR,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi829[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_XOR,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi830[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_XOR,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi831[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi832[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi833[] = {
      {BO_EQV,0,1,0},{BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi834[] = {
      {BO_EQV,0,1,0},{BO_XOR,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi835[] = {
      {BO_EQV,2,3,2},{BO_XOR,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi836[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_XOR,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi837[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_XOR,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi838[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi839[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_XOR,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi840[] = {
      {BO_EQV,0,1,0},{BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi841[] = {
      {BO_EQV,0,1,0},{BO_EQV,0,2,0},{BO_AND,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi842[] = {
      {BO_EQV,2,3,2},{BO_EQV,1,2,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi843[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_EQV,2,3,1},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi844[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_EQV,2,3,1},{BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi845[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi846[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_AND,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi847[] = {
      {BO_EQV,0,1,0},{BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi848[] = {
      {BO_EQV,0,1,0},{BO_EQV,0,2,0},{BO_OR ,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi849[] = {
      {BO_EQV,2,3,2},{BO_EQV,1,2,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi850[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_EQV,2,3,1},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi851[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_EQV,2,3,1},{BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi852[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi853[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_OR ,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi854[] = {
      {BO_EQV,0,1,0},{BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi855[] = {
      {BO_EQV,0,1,0},{BO_EQV,0,2,0},{BO_IMP,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi856[] = {
      {BO_EQV,2,3,2},{BO_EQV,1,2,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi857[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_EQV,2,3,1},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi858[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_EQV,2,3,1},{BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi859[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi860[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_IMP,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi861[] = {
      {BO_EQV,0,1,0},{BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi862[] = {
      {BO_EQV,0,1,0},{BO_EQV,0,2,0},{BO_XOR,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi863[] = {
      {BO_EQV,2,3,2},{BO_EQV,1,2,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi864[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_EQV,2,3,1},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi865[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_EQV,2,3,1},{BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi866[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi867[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_XOR,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi868[] = {
      {BO_EQV,0,1,0},{BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi869[] = {
      {BO_EQV,0,1,0},{BO_EQV,0,2,0},{BO_EQV,0,3,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi870[] = {
      {BO_EQV,2,3,2},{BO_EQV,1,2,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi871[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_EQV,0,1,0},{BO_EQV,2,3,1},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi872[] = {
      {BO_NOT,0,0,0},{BO_NOT,2,2,0},{BO_NOT,0,0,0},{BO_EQV,0,1,0},
      {BO_EQV,2,3,1},{BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi873[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},
      {BO_HLT,0,0,0}
    };
    const BoolInstr bi874[] = {
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},{BO_EQV,2,3,1},{BO_NOT,1,1,0},
      {BO_EQV,0,1,0},{BO_NOT,0,0,0},
      {BO_HLT,0,0,0}
    };

    const BoolInstr* bi[] = {
      &bi000[0],&bi001[0],&bi002[0],&bi003[0],&bi004[0],&bi005[0],
      &bi006[0],&bi007[0],&bi008[0],&bi009[0],&bi010[0],&bi011[0],
      &bi012[0],&bi013[0],&bi014[0],&bi015[0],&bi016[0],&bi017[0],
      &bi018[0],&bi019[0],&bi020[0],&bi021[0],&bi022[0],&bi023[0],
      &bi024[0],&bi025[0],&bi026[0],&bi027[0],&bi028[0],&bi029[0],
      &bi030[0],&bi031[0],&bi032[0],&bi033[0],&bi034[0],&bi035[0],
      &bi036[0],&bi037[0],&bi038[0],&bi039[0],&bi040[0],&bi041[0],
      &bi042[0],&bi043[0],&bi044[0],&bi045[0],&bi046[0],&bi047[0],
      &bi048[0],&bi049[0],&bi050[0],&bi051[0],&bi052[0],&bi053[0],
      &bi054[0],&bi055[0],&bi056[0],&bi057[0],&bi058[0],&bi059[0],
      &bi060[0],&bi061[0],&bi062[0],&bi063[0],&bi064[0],&bi065[0],
      &bi066[0],&bi067[0],&bi068[0],&bi069[0],&bi070[0],&bi071[0],
      &bi072[0],&bi073[0],&bi074[0],&bi075[0],&bi076[0],&bi077[0],
      &bi078[0],&bi079[0],&bi080[0],&bi081[0],&bi082[0],&bi083[0],
      &bi084[0],&bi085[0],&bi086[0],&bi087[0],&bi088[0],&bi089[0],
      &bi090[0],&bi091[0],&bi092[0],&bi093[0],&bi094[0],&bi095[0],
      &bi096[0],&bi097[0],&bi098[0],&bi099[0],&bi100[0],&bi101[0],
      &bi102[0],&bi103[0],&bi104[0],&bi105[0],&bi106[0],&bi107[0],
      &bi108[0],&bi109[0],&bi110[0],&bi111[0],&bi112[0],&bi113[0],
      &bi114[0],&bi115[0],&bi116[0],&bi117[0],&bi118[0],&bi119[0],
      &bi120[0],&bi121[0],&bi122[0],&bi123[0],&bi124[0],&bi125[0],
      &bi126[0],&bi127[0],&bi128[0],&bi129[0],&bi130[0],&bi131[0],
      &bi132[0],&bi133[0],&bi134[0],&bi135[0],&bi136[0],&bi137[0],
      &bi138[0],&bi139[0],&bi140[0],&bi141[0],&bi142[0],&bi143[0],
      &bi144[0],&bi145[0],&bi146[0],&bi147[0],&bi148[0],&bi149[0],
      &bi150[0],&bi151[0],&bi152[0],&bi153[0],&bi154[0],&bi155[0],
      &bi156[0],&bi157[0],&bi158[0],&bi159[0],&bi160[0],&bi161[0],
      &bi162[0],&bi163[0],&bi164[0],&bi165[0],&bi166[0],&bi167[0],
      &bi168[0],&bi169[0],&bi170[0],&bi171[0],&bi172[0],&bi173[0],
      &bi174[0],&bi175[0],&bi176[0],&bi177[0],&bi178[0],&bi179[0],
      &bi180[0],&bi181[0],&bi182[0],&bi183[0],&bi184[0],&bi185[0],
      &bi186[0],&bi187[0],&bi188[0],&bi189[0],&bi190[0],&bi191[0],
      &bi192[0],&bi193[0],&bi194[0],&bi195[0],&bi196[0],&bi197[0],
      &bi198[0],&bi199[0],&bi200[0],&bi201[0],&bi202[0],&bi203[0],
      &bi204[0],&bi205[0],&bi206[0],&bi207[0],&bi208[0],&bi209[0],
      &bi210[0],&bi211[0],&bi212[0],&bi213[0],&bi214[0],&bi215[0],
      &bi216[0],&bi217[0],&bi218[0],&bi219[0],&bi220[0],&bi221[0],
      &bi222[0],&bi223[0],&bi224[0],&bi225[0],&bi226[0],&bi227[0],
      &bi228[0],&bi229[0],&bi230[0],&bi231[0],&bi232[0],&bi233[0],
      &bi234[0],&bi235[0],&bi236[0],&bi237[0],&bi238[0],&bi239[0],
      &bi240[0],&bi241[0],&bi242[0],&bi243[0],&bi244[0],&bi245[0],
      &bi246[0],&bi247[0],&bi248[0],&bi249[0],&bi250[0],&bi251[0],
      &bi252[0],&bi253[0],&bi254[0],&bi255[0],&bi256[0],&bi257[0],
      &bi258[0],&bi259[0],&bi260[0],&bi261[0],&bi262[0],&bi263[0],
      &bi264[0],&bi265[0],&bi266[0],&bi267[0],&bi268[0],&bi269[0],
      &bi270[0],&bi271[0],&bi272[0],&bi273[0],&bi274[0],&bi275[0],
      &bi276[0],&bi277[0],&bi278[0],&bi279[0],&bi280[0],&bi281[0],
      &bi282[0],&bi283[0],&bi284[0],&bi285[0],&bi286[0],&bi287[0],
      &bi288[0],&bi289[0],&bi290[0],&bi291[0],&bi292[0],&bi293[0],
      &bi294[0],&bi295[0],&bi296[0],&bi297[0],&bi298[0],&bi299[0],
      &bi300[0],&bi301[0],&bi302[0],&bi303[0],&bi304[0],&bi305[0],
      &bi306[0],&bi307[0],&bi308[0],&bi309[0],&bi310[0],&bi311[0],
      &bi312[0],&bi313[0],&bi314[0],&bi315[0],&bi316[0],&bi317[0],
      &bi318[0],&bi319[0],&bi320[0],&bi321[0],&bi322[0],&bi323[0],
      &bi324[0],&bi325[0],&bi326[0],&bi327[0],&bi328[0],&bi329[0],
      &bi330[0],&bi331[0],&bi332[0],&bi333[0],&bi334[0],&bi335[0],
      &bi336[0],&bi337[0],&bi338[0],&bi339[0],&bi340[0],&bi341[0],
      &bi342[0],&bi343[0],&bi344[0],&bi345[0],&bi346[0],&bi347[0],
      &bi348[0],&bi349[0],&bi350[0],&bi351[0],&bi352[0],&bi353[0],
      &bi354[0],&bi355[0],&bi356[0],&bi357[0],&bi358[0],&bi359[0],
      &bi360[0],&bi361[0],&bi362[0],&bi363[0],&bi364[0],&bi365[0],
      &bi366[0],&bi367[0],&bi368[0],&bi369[0],&bi370[0],&bi371[0],
      &bi372[0],&bi373[0],&bi374[0],&bi375[0],&bi376[0],&bi377[0],
      &bi378[0],&bi379[0],&bi380[0],&bi381[0],&bi382[0],&bi383[0],
      &bi384[0],&bi385[0],&bi386[0],&bi387[0],&bi388[0],&bi389[0],
      &bi390[0],&bi391[0],&bi392[0],&bi393[0],&bi394[0],&bi395[0],
      &bi396[0],&bi397[0],&bi398[0],&bi399[0],&bi400[0],&bi401[0],
      &bi402[0],&bi403[0],&bi404[0],&bi405[0],&bi406[0],&bi407[0],
      &bi408[0],&bi409[0],&bi410[0],&bi411[0],&bi412[0],&bi413[0],
      &bi414[0],&bi415[0],&bi416[0],&bi417[0],&bi418[0],&bi419[0],
      &bi420[0],&bi421[0],&bi422[0],&bi423[0],&bi424[0],&bi425[0],
      &bi426[0],&bi427[0],&bi428[0],&bi429[0],&bi430[0],&bi431[0],
      &bi432[0],&bi433[0],&bi434[0],&bi435[0],&bi436[0],&bi437[0],
      &bi438[0],&bi439[0],&bi440[0],&bi441[0],&bi442[0],&bi443[0],
      &bi444[0],&bi445[0],&bi446[0],&bi447[0],&bi448[0],&bi449[0],
      &bi450[0],&bi451[0],&bi452[0],&bi453[0],&bi454[0],&bi455[0],
      &bi456[0],&bi457[0],&bi458[0],&bi459[0],&bi460[0],&bi461[0],
      &bi462[0],&bi463[0],&bi464[0],&bi465[0],&bi466[0],&bi467[0],
      &bi468[0],&bi469[0],&bi470[0],&bi471[0],&bi472[0],&bi473[0],
      &bi474[0],&bi475[0],&bi476[0],&bi477[0],&bi478[0],&bi479[0],
      &bi480[0],&bi481[0],&bi482[0],&bi483[0],&bi484[0],&bi485[0],
      &bi486[0],&bi487[0],&bi488[0],&bi489[0],&bi490[0],&bi491[0],
      &bi492[0],&bi493[0],&bi494[0],&bi495[0],&bi496[0],&bi497[0],
      &bi498[0],&bi499[0],&bi500[0],&bi501[0],&bi502[0],&bi503[0],
      &bi504[0],&bi505[0],&bi506[0],&bi507[0],&bi508[0],&bi509[0],
      &bi510[0],&bi511[0],&bi512[0],&bi513[0],&bi514[0],&bi515[0],
      &bi516[0],&bi517[0],&bi518[0],&bi519[0],&bi520[0],&bi521[0],
      &bi522[0],&bi523[0],&bi524[0],&bi525[0],&bi526[0],&bi527[0],
      &bi528[0],&bi529[0],&bi530[0],&bi531[0],&bi532[0],&bi533[0],
      &bi534[0],&bi535[0],&bi536[0],&bi537[0],&bi538[0],&bi539[0],
      &bi540[0],&bi541[0],&bi542[0],&bi543[0],&bi544[0],&bi545[0],
      &bi546[0],&bi547[0],&bi548[0],&bi549[0],&bi550[0],&bi551[0],
      &bi552[0],&bi553[0],&bi554[0],&bi555[0],&bi556[0],&bi557[0],
      &bi558[0],&bi559[0],&bi560[0],&bi561[0],&bi562[0],&bi563[0],
      &bi564[0],&bi565[0],&bi566[0],&bi567[0],&bi568[0],&bi569[0],
      &bi570[0],&bi571[0],&bi572[0],&bi573[0],&bi574[0],&bi575[0],
      &bi576[0],&bi577[0],&bi578[0],&bi579[0],&bi580[0],&bi581[0],
      &bi582[0],&bi583[0],&bi584[0],&bi585[0],&bi586[0],&bi587[0],
      &bi588[0],&bi589[0],&bi590[0],&bi591[0],&bi592[0],&bi593[0],
      &bi594[0],&bi595[0],&bi596[0],&bi597[0],&bi598[0],&bi599[0],
      &bi600[0],&bi601[0],&bi602[0],&bi603[0],&bi604[0],&bi605[0],
      &bi606[0],&bi607[0],&bi608[0],&bi609[0],&bi610[0],&bi611[0],
      &bi612[0],&bi613[0],&bi614[0],&bi615[0],&bi616[0],&bi617[0],
      &bi618[0],&bi619[0],&bi620[0],&bi621[0],&bi622[0],&bi623[0],
      &bi624[0],&bi625[0],&bi626[0],&bi627[0],&bi628[0],&bi629[0],
      &bi630[0],&bi631[0],&bi632[0],&bi633[0],&bi634[0],&bi635[0],
      &bi636[0],&bi637[0],&bi638[0],&bi639[0],&bi640[0],&bi641[0],
      &bi642[0],&bi643[0],&bi644[0],&bi645[0],&bi646[0],&bi647[0],
      &bi648[0],&bi649[0],&bi650[0],&bi651[0],&bi652[0],&bi653[0],
      &bi654[0],&bi655[0],&bi656[0],&bi657[0],&bi658[0],&bi659[0],
      &bi660[0],&bi661[0],&bi662[0],&bi663[0],&bi664[0],&bi665[0],
      &bi666[0],&bi667[0],&bi668[0],&bi669[0],&bi670[0],&bi671[0],
      &bi672[0],&bi673[0],&bi674[0],&bi675[0],&bi676[0],&bi677[0],
      &bi678[0],&bi679[0],&bi680[0],&bi681[0],&bi682[0],&bi683[0],
      &bi684[0],&bi685[0],&bi686[0],&bi687[0],&bi688[0],&bi689[0],
      &bi690[0],&bi691[0],&bi692[0],&bi693[0],&bi694[0],&bi695[0],
      &bi696[0],&bi697[0],&bi698[0],&bi699[0],&bi700[0],&bi701[0],
      &bi702[0],&bi703[0],&bi704[0],&bi705[0],&bi706[0],&bi707[0],
      &bi708[0],&bi709[0],&bi710[0],&bi711[0],&bi712[0],&bi713[0],
      &bi714[0],&bi715[0],&bi716[0],&bi717[0],&bi718[0],&bi719[0],
      &bi720[0],&bi721[0],&bi722[0],&bi723[0],&bi724[0],&bi725[0],
      &bi726[0],&bi727[0],&bi728[0],&bi729[0],&bi730[0],&bi731[0],
      &bi732[0],&bi733[0],&bi734[0],&bi735[0],&bi736[0],&bi737[0],
      &bi738[0],&bi739[0],&bi740[0],&bi741[0],&bi742[0],&bi743[0],
      &bi744[0],&bi745[0],&bi746[0],&bi747[0],&bi748[0],&bi749[0],
      &bi750[0],&bi751[0],&bi752[0],&bi753[0],&bi754[0],&bi755[0],
      &bi756[0],&bi757[0],&bi758[0],&bi759[0],&bi760[0],&bi761[0],
      &bi762[0],&bi763[0],&bi764[0],&bi765[0],&bi766[0],&bi767[0],
      &bi768[0],&bi769[0],&bi770[0],&bi771[0],&bi772[0],&bi773[0],
      &bi774[0],&bi775[0],&bi776[0],&bi777[0],&bi778[0],&bi779[0],
      &bi780[0],&bi781[0],&bi782[0],&bi783[0],&bi784[0],&bi785[0],
      &bi786[0],&bi787[0],&bi788[0],&bi789[0],&bi790[0],&bi791[0],
      &bi792[0],&bi793[0],&bi794[0],&bi795[0],&bi796[0],&bi797[0],
      &bi798[0],&bi799[0],&bi800[0],&bi801[0],&bi802[0],&bi803[0],
      &bi804[0],&bi805[0],&bi806[0],&bi807[0],&bi808[0],&bi809[0],
      &bi810[0],&bi811[0],&bi812[0],&bi813[0],&bi814[0],&bi815[0],
      &bi816[0],&bi817[0],&bi818[0],&bi819[0],&bi820[0],&bi821[0],
      &bi822[0],&bi823[0],&bi824[0],&bi825[0],&bi826[0],&bi827[0],
      &bi828[0],&bi829[0],&bi830[0],&bi831[0],&bi832[0],&bi833[0],
      &bi834[0],&bi835[0],&bi836[0],&bi837[0],&bi838[0],&bi839[0],
      &bi840[0],&bi841[0],&bi842[0],&bi843[0],&bi844[0],&bi845[0],
      &bi846[0],&bi847[0],&bi848[0],&bi849[0],&bi850[0],&bi851[0],
      &bi852[0],&bi853[0],&bi854[0],&bi855[0],&bi856[0],&bi857[0],
      &bi858[0],&bi859[0],&bi860[0],&bi861[0],&bi862[0],&bi863[0],
      &bi864[0],&bi865[0],&bi866[0],&bi867[0],&bi868[0],&bi869[0],
      &bi870[0],&bi871[0],&bi872[0],&bi873[0],&bi874[0]
    };

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        int n = sizeof(bi)/sizeof(BoolInstr*);
        for (int i=0; i<n; i++) {
          std::string s = Test::str(i);
          if (i < 10) {
            s = "00" + s;
          } else if (i < 100) {
            s = "0" + s;
          }
          (void) new BoolExprInt(bi[i],s,0);
          (void) new BoolExprInt(bi[i],s,1);
          (void) new BoolExprVar(bi[i],s);
        }
      }
    };

    Create c;
    //@}
   }

}}

// STATISTICS: test-minimodel
