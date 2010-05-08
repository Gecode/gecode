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

  /// %Tests for minimal modelling constraints (linear)
   namespace MiniModelLin {

     /// Linear opcode
     enum LinOpcode {
       LO_ACE, ///< Add integer and expression
       LO_AEC, ///< Add expression and integer
       LO_AEE, ///< Add expressions
       LO_SCE, ///< Subtract integer and expression
       LO_SEC, ///< Subtract expression and integer
       LO_SEE, ///< Subtract expressions
       LO_SE,  ///< Unary subtraction
       LO_MCE, ///< Multiply constant and expression
       LO_MEC, ///< Multiply constant and expression
       LO_HLT  ///< Stop execution
     };

     /// Type for representing a linear instruction
     class LinInstr {
     public:
       LinOpcode o; ///< Which instruction to execute
       unsigned char x, y, z;  ///< Instruction arguments, \a y is destination (or \a z)
       int c; ///< Numerical constant
     };

     /// Evaluate linear instructions
     template<class Expr>
     Expr
     eval(const LinInstr* pc, Expr reg[]) {
       while (true) {
         switch (pc->o) {
         case LO_ACE: reg[pc->y] = pc->c + reg[pc->x]; break;
         case LO_AEC: reg[pc->y] = reg[pc->x] + pc->c; break;
         case LO_AEE: reg[pc->z] = reg[pc->x] + reg[pc->y]; break;
         case LO_SCE: reg[pc->y] = pc->c - reg[pc->x]; break;
         case LO_SEC: reg[pc->y] = reg[pc->x] - pc->c; break;
         case LO_SEE: reg[pc->z] = reg[pc->x] - reg[pc->y]; break;
         case LO_SE:  reg[pc->y] = -reg[pc->x]; break;
         case LO_MCE: reg[pc->y] = pc->c * reg[pc->x]; break;
         case LO_MEC: reg[pc->y] = reg[pc->x] * pc->c; break;
         case LO_HLT: return reg[pc->x];
         default: GECODE_NEVER;
         }
         pc++;
       }
       GECODE_NEVER;
     }

     /**
      * \defgroup TaskTestIntMiniModelLin Minimal modeling constraints (linear constraints)
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test linear expressions over integer variables
     class LinExprInt : public Test {
     protected:
       /// Linear instruction sequence
       const LinInstr* lis;
     public:
       /// Create and register test
       LinExprInt(const LinInstr* lis0, const std::string& s)
         : Test("MiniModel::LinExpr::Int::"+s,4,-3,3), lis(lis0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int reg[3] = {x[0],x[1],x[2]};
         return eval(lis, reg) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::LinExpr reg[3] = {x[0],x[1],x[2]};
         rel(home, x[3], IRT_EQ, Gecode::expr(home, eval(lis,reg)));
       }
     };

     /// %Test linear expressions over Boolean variables
     class LinExprBool : public Test {
     protected:
       /// Linear instruction sequence
       const LinInstr* lis;
     public:
       /// Create and register test
       LinExprBool(const LinInstr* lis0, const std::string& s)
         : Test("MiniModel::LinExpr::Bool::"+s,4,-3,3), lis(lis0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=3; i--; )
           if ((x[i] < 0) || (x[i] > 1))
             return false;
         int reg[3] = {x[0],x[1],x[2]};
         return eval(lis, reg) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::LinExpr reg[3] = {
           channel(home,x[0]),channel(home,x[1]),channel(home,x[2])
         };
         rel(home, x[3], IRT_EQ, Gecode::expr(home, eval(lis,reg)));
       }
     };

     /// %Test linear expressions over integer and Boolean variables
     class LinExprMixed : public Test {
     protected:
       /// Linear instruction sequence
       const LinInstr* lis;
     public:
       /// Create and register test
       LinExprMixed(const LinInstr* lis0, const std::string& s)
         : Test("MiniModel::LinExpr::Mixed::"+s,4,-3,3), lis(lis0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if ((x[2] < 0) || (x[2] > 1))
           return false;
         int reg[3] = {x[0],x[1],x[2]};
         return eval(lis, reg) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::LinExpr reg[3] = {
           x[0],x[1],channel(home,x[2])
         };
         rel(home, x[3], IRT_EQ, Gecode::expr(home, eval(lis,reg)));
       }
     };


     /// %Test linear relations over integer variables
     class LinRelInt : public Test {
     protected:
       /// Linear instruction sequence for left hand side
       const LinInstr* l_lis;
       /// Linear instruction sequence for right hand side
       const LinInstr* r_lis;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       LinRelInt(const LinInstr* l_lis0, const LinInstr* r_lis0,
                 Gecode::IntRelType irt0, const std::string& s)
         : Test("MiniModel::LinRel::Int::"+s+"::"+str(irt0),3,-3,3,true),
           l_lis(l_lis0), r_lis(r_lis0), irt(irt0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int l_reg[3] = {x[0],x[1],x[2]};
         int r_reg[3] = {x[0],x[1],x[2]};
         return cmp(eval(l_lis,l_reg),irt,eval(r_lis,r_reg));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::LinExpr l_reg[3] = {x[0],x[1],x[2]};
         Gecode::LinExpr r_reg[3] = {x[0],x[1],x[2]};
         switch (irt) {
         case IRT_EQ:
           {
             IntVar x = Gecode::expr(home,eval(l_lis,l_reg));
             IntVar y = Gecode::expr(home,eval(r_lis,r_reg));
             IntArgs a(2, 1,-1);
             IntVarArgs xy(2); xy[0]=x; xy[1]=y;
             Gecode::rel(home, 0 == sum(a,xy));
           }
           break;
         case IRT_NQ:
           Gecode::rel(home, eval(l_lis,l_reg) - eval(r_lis,r_reg) != 0);
           break;
         case IRT_LQ:
           Gecode::rel(home, !(eval(l_lis,l_reg) > eval(r_lis,r_reg)));
           break;
         case IRT_LE:
           Gecode::rel(home, eval(l_lis,l_reg) < eval(r_lis,r_reg));
           break;
         case IRT_GQ:
           Gecode::rel(home, eval(l_lis,l_reg) >= eval(r_lis,r_reg));
           break;
         case IRT_GR:
           Gecode::rel(home, !(eval(l_lis,l_reg) <= eval(r_lis,r_reg)));
           break;
         default: GECODE_NEVER;
         }
       }
       /// Post constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         using namespace Gecode;
         Gecode::LinExpr l_reg[3] = {x[0],x[1],x[2]};
         Gecode::LinExpr r_reg[3] = {x[0],x[1],x[2]};
         switch (irt) {
         case IRT_EQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)==eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_NQ:
           Gecode::rel(home,
                       (eval(l_lis,l_reg)!=eval(r_lis,r_reg)) == b);
           break;
         case IRT_LQ:
           Gecode::rel(home,
                       !((eval(l_lis,l_reg)<=eval(r_lis,r_reg))^b));
           break;
         case IRT_LE:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)<eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_GQ:
           Gecode::rel(home,
                       (eval(l_lis,l_reg)>=eval(r_lis,r_reg)) == b);
           break;
         case IRT_GR:
           Gecode::rel(home,
                       !((eval(l_lis,l_reg)>eval(r_lis,r_reg))^b));
           break;
         default: GECODE_NEVER;
         }
       }
     };

     /// %Test linear relations over Boolean variables
     class LinRelBool : public Test {
     protected:
       /// Linear instruction sequence for left hand side
       const LinInstr* l_lis;
       /// Linear instruction sequence for right hand side
       const LinInstr* r_lis;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       LinRelBool(const LinInstr* l_lis0, const LinInstr* r_lis0,
                  Gecode::IntRelType irt0, const std::string& s)
         : Test("MiniModel::LinRel::Bool::"+s+"::"+str(irt0),3,0,1,true),
           l_lis(l_lis0), r_lis(r_lis0), irt(irt0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int l_reg[3] = {x[0],x[1],x[2]};
         int r_reg[3] = {x[0],x[1],x[2]};
         return cmp(eval(l_lis,l_reg),irt,eval(r_lis,r_reg));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(3);
         y[0] = channel(home,x[0]); y[1] = channel(home,x[1]);
         y[2] = channel(home,x[2]);
         Gecode::LinExpr l_reg[3] = {y[0],y[1],y[2]};
         Gecode::LinExpr r_reg[3] = {y[0],y[1],y[2]};
         switch (irt) {
         case IRT_EQ:
           {
             IntVar x = Gecode::expr(home,eval(l_lis,l_reg));
             IntVar y = Gecode::expr(home,eval(r_lis,r_reg));
             IntArgs a(2, -2,2);
             IntVarArgs xy(2); xy[0]=x; xy[1]=y;
             Gecode::rel(home, 0 == sum(a,xy));
           }
           break;
         case IRT_NQ:
           Gecode::rel(home, eval(l_lis,l_reg) - eval(r_lis,r_reg) != 0);
           break;
         case IRT_LQ:
           Gecode::rel(home, !(eval(l_lis,l_reg) > eval(r_lis,r_reg)));
           break;
         case IRT_LE:
           Gecode::rel(home, eval(l_lis,l_reg) < eval(r_lis,r_reg));
           break;
         case IRT_GQ:
           Gecode::rel(home, eval(l_lis,l_reg) >= eval(r_lis,r_reg));
           break;
         case IRT_GR:
           Gecode::rel(home, !(eval(l_lis,l_reg) <= eval(r_lis,r_reg)));
           break;
         default: GECODE_NEVER;
         }
       }
       /// Post constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         using namespace Gecode;
         BoolVarArgs y(3);
         y[0] = channel(home,x[0]); y[1] = channel(home,x[1]);
         y[2] = channel(home,x[2]);
         Gecode::LinExpr l_reg[3] = {y[0],y[1],y[2]};
         Gecode::LinExpr r_reg[3] = {y[0],y[1],y[2]};
         switch (irt) {
         case IRT_EQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)==eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_NQ:
           Gecode::rel(home,
                       (eval(l_lis,l_reg)!=eval(r_lis,r_reg)) == b);
           break;
         case IRT_LQ:
           Gecode::rel(home,
                       !((eval(l_lis,l_reg)<=eval(r_lis,r_reg))^b));
           break;
         case IRT_LE:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)<eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_GQ:
           Gecode::rel(home,
                       (eval(l_lis,l_reg)>=eval(r_lis,r_reg)) == b);
           break;
         case IRT_GR:
           Gecode::rel(home,
                       !((eval(l_lis,l_reg)>eval(r_lis,r_reg))^b));
           break;
         default: GECODE_NEVER;
         }
       }
     };

     /// %Test linear relations over integer and Boolean variables
     class LinRelMixed : public Test {
     protected:
       /// Linear instruction sequence for left hand side
       const LinInstr* l_lis;
       /// Linear instruction sequence for right hand side
       const LinInstr* r_lis;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       LinRelMixed(const LinInstr* l_lis0, const LinInstr* r_lis0,
                   Gecode::IntRelType irt0, const std::string& s)
         : Test("MiniModel::LinRel::Mixed::"+s+"::"+str(irt0),6,0,1,true),
           l_lis(l_lis0), r_lis(r_lis0), irt(irt0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int l_reg[3] = {x[0],x[1],x[2]};
         int r_reg[3] = {x[3],x[4],x[5]};
         return cmp(eval(l_lis,l_reg),irt,eval(r_lis,r_reg));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::LinExpr l_reg[3] = {channel(home,x[0]),x[1],x[2]};
         Gecode::LinExpr r_reg[3] = {channel(home,x[3]),x[4],
                                     channel(home,x[5])};
         switch (irt) {
         case IRT_EQ:
           Gecode::rel(home, 0 == eval(l_lis,l_reg) - eval(r_lis,r_reg));
           break;
         case IRT_NQ:
           Gecode::rel(home, eval(l_lis,l_reg) - eval(r_lis,r_reg) != 0);
           break;
         case IRT_LQ:
           Gecode::rel(home, !(eval(l_lis,l_reg) > eval(r_lis,r_reg)));
           break;
         case IRT_LE:
           Gecode::rel(home, eval(l_lis,l_reg) < eval(r_lis,r_reg));
           break;
         case IRT_GQ:
           Gecode::rel(home, eval(l_lis,l_reg) >= eval(r_lis,r_reg));
           break;
         case IRT_GR:
           Gecode::rel(home, !(eval(l_lis,l_reg) <= eval(r_lis,r_reg)));
           break;
         default: GECODE_NEVER;
         }
       }
       /// Post constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         using namespace Gecode;
         Gecode::LinExpr l_reg[3] = {channel(home,x[0]),x[1],x[2]};
         Gecode::LinExpr r_reg[3] = {channel(home,x[3]),x[4],
                                     channel(home,x[5])};
         switch (irt) {
         case IRT_EQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)==eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_NQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)!=eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_LQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)<=eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_LE:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)<eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_GQ:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)>=eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         case IRT_GR:
           rel(home, Gecode::expr(home,
                                  (eval(l_lis,l_reg)>eval(r_lis,r_reg))),
               IRT_EQ, b);
           break;
         default: GECODE_NEVER;
         }
       }
     };

     const LinInstr li000[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li001[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li002[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li003[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li004[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li005[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li006[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li007[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li008[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li009[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li010[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li011[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li012[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li013[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li014[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li015[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li016[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li017[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li018[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li019[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li020[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li021[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li022[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li023[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li024[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li025[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li026[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li027[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li028[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li029[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li030[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li031[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li032[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li033[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li034[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li035[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li036[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li037[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li038[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li039[] = {
       {LO_AEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li040[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li041[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li042[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li043[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li044[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li045[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li046[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li047[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li048[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li049[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li050[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li051[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li052[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li053[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li054[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li055[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li056[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li057[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li058[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li059[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li060[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li061[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li062[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li063[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li064[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li065[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li066[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li067[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li068[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li069[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li070[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li071[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li072[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li073[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li074[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li075[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li076[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li077[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li078[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li079[] = {
       {LO_AEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li080[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li081[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li082[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li083[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li084[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li085[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li086[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li087[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li088[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li089[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li090[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li091[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li092[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li093[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li094[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li095[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li096[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li097[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li098[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li099[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li100[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li101[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li102[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li103[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li104[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li105[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li106[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li107[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li108[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li109[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li110[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li111[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li112[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li113[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li114[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li115[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li116[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li117[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li118[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li119[] = {
       {LO_AEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li120[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li121[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li122[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li123[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li124[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li125[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li126[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li127[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li128[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li129[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li130[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li131[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li132[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li133[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li134[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li135[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li136[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li137[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li138[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li139[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li140[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li141[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li142[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li143[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li144[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li145[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li146[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li147[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li148[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li149[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li150[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li151[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li152[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li153[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li154[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li155[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li156[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li157[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li158[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li159[] = {
       {LO_AEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li160[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li161[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li162[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li163[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li164[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li165[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li166[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li167[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li168[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li169[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li170[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li171[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li172[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li173[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li174[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li175[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li176[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li177[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li178[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li179[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li180[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li181[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li182[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li183[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li184[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li185[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li186[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li187[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li188[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li189[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li190[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li191[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li192[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li193[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li194[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li195[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li196[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li197[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li198[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li199[] = {
       {LO_AEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li200[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li201[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li202[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li203[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li204[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li205[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li206[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li207[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li208[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li209[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li210[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li211[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li212[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li213[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li214[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li215[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li216[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li217[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li218[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li219[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li220[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li221[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li222[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li223[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li224[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li225[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li226[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li227[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li228[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li229[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li230[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li231[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li232[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li233[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li234[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li235[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li236[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li237[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li238[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li239[] = {
       {LO_SEE,0,1,0, 0},{LO_AEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li240[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li241[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li242[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li243[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li244[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li245[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li246[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li247[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li248[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li249[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li250[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li251[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li252[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li253[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li254[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li255[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li256[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li257[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li258[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li259[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li260[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li261[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li262[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li263[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li264[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li265[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li266[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li267[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li268[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li269[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li270[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li271[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li272[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li273[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li274[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li275[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li276[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li277[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li278[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li279[] = {
       {LO_SEE,0,1,0, 0},{LO_SCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li280[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li281[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li282[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li283[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li284[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li285[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li286[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li287[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li288[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li289[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li290[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li291[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li292[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li293[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li294[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li295[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li296[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li297[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li298[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li299[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li300[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li301[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li302[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li303[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li304[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li305[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li306[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li307[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li308[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li309[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li310[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li311[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li312[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li313[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li314[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li315[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li316[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li317[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li318[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li319[] = {
       {LO_SEE,0,1,0, 0},{LO_SEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li320[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li321[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li322[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li323[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li324[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li325[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li326[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li327[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li328[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li329[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li330[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li331[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li332[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li333[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li334[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li335[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li336[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li337[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li338[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li339[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li340[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li341[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li342[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li343[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li344[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li345[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li346[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li347[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li348[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li349[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li350[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li351[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li352[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li353[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li354[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li355[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li356[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li357[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li358[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li359[] = {
       {LO_SEE,0,1,0, 0},{LO_MCE,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li360[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li361[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li362[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li363[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li364[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li365[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li366[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li367[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li368[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li369[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li370[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li371[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li372[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li373[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li374[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li375[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0,-1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li376[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li377[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li378[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li379[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li380[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li381[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li382[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li383[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 0},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li384[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li385[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li386[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li387[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li388[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li389[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li390[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li391[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 1},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li392[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li393[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li394[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li395[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_AEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li396[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0,-1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li397[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_ACE,0,0,0, 1},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li398[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };
     const LinInstr li399[] = {
       {LO_SEE,0,1,0, 0},{LO_MEC,0,0,0, 2},{LO_SEE,0,2,0, 0},
       {LO_SE ,0,0,0, 0},{LO_HLT,0,0,0, 0}
     };

     const LinInstr* li[] = {
       &li000[0],&li001[0],&li002[0],&li003[0],&li004[0],&li005[0],
       &li006[0],&li007[0],&li008[0],&li009[0],&li010[0],&li011[0],
       &li012[0],&li013[0],&li014[0],&li015[0],&li016[0],&li017[0],
       &li018[0],&li019[0],&li020[0],&li021[0],&li022[0],&li023[0],
       &li024[0],&li025[0],&li026[0],&li027[0],&li028[0],&li029[0],
       &li030[0],&li031[0],&li032[0],&li033[0],&li034[0],&li035[0],
       &li036[0],&li037[0],&li038[0],&li039[0],&li040[0],&li041[0],
       &li042[0],&li043[0],&li044[0],&li045[0],&li046[0],&li047[0],
       &li048[0],&li049[0],&li050[0],&li051[0],&li052[0],&li053[0],
       &li054[0],&li055[0],&li056[0],&li057[0],&li058[0],&li059[0],
       &li060[0],&li061[0],&li062[0],&li063[0],&li064[0],&li065[0],
       &li066[0],&li067[0],&li068[0],&li069[0],&li070[0],&li071[0],
       &li072[0],&li073[0],&li074[0],&li075[0],&li076[0],&li077[0],
       &li078[0],&li079[0],&li080[0],&li081[0],&li082[0],&li083[0],
       &li084[0],&li085[0],&li086[0],&li087[0],&li088[0],&li089[0],
       &li090[0],&li091[0],&li092[0],&li093[0],&li094[0],&li095[0],
       &li096[0],&li097[0],&li098[0],&li099[0],&li100[0],&li101[0],
       &li102[0],&li103[0],&li104[0],&li105[0],&li106[0],&li107[0],
       &li108[0],&li109[0],&li110[0],&li111[0],&li112[0],&li113[0],
       &li114[0],&li115[0],&li116[0],&li117[0],&li118[0],&li119[0],
       &li120[0],&li121[0],&li122[0],&li123[0],&li124[0],&li125[0],
       &li126[0],&li127[0],&li128[0],&li129[0],&li130[0],&li131[0],
       &li132[0],&li133[0],&li134[0],&li135[0],&li136[0],&li137[0],
       &li138[0],&li139[0],&li140[0],&li141[0],&li142[0],&li143[0],
       &li144[0],&li145[0],&li146[0],&li147[0],&li148[0],&li149[0],
       &li150[0],&li151[0],&li152[0],&li153[0],&li154[0],&li155[0],
       &li156[0],&li157[0],&li158[0],&li159[0],&li160[0],&li161[0],
       &li162[0],&li163[0],&li164[0],&li165[0],&li166[0],&li167[0],
       &li168[0],&li169[0],&li170[0],&li171[0],&li172[0],&li173[0],
       &li174[0],&li175[0],&li176[0],&li177[0],&li178[0],&li179[0],
       &li180[0],&li181[0],&li182[0],&li183[0],&li184[0],&li185[0],
       &li186[0],&li187[0],&li188[0],&li189[0],&li190[0],&li191[0],
       &li192[0],&li193[0],&li194[0],&li195[0],&li196[0],&li197[0],
       &li198[0],&li199[0],&li200[0],&li201[0],&li202[0],&li203[0],
       &li204[0],&li205[0],&li206[0],&li207[0],&li208[0],&li209[0],
       &li210[0],&li211[0],&li212[0],&li213[0],&li214[0],&li215[0],
       &li216[0],&li217[0],&li218[0],&li219[0],&li220[0],&li221[0],
       &li222[0],&li223[0],&li224[0],&li225[0],&li226[0],&li227[0],
       &li228[0],&li229[0],&li230[0],&li231[0],&li232[0],&li233[0],
       &li234[0],&li235[0],&li236[0],&li237[0],&li238[0],&li239[0],
       &li240[0],&li241[0],&li242[0],&li243[0],&li244[0],&li245[0],
       &li246[0],&li247[0],&li248[0],&li249[0],&li250[0],&li251[0],
       &li252[0],&li253[0],&li254[0],&li255[0],&li256[0],&li257[0],
       &li258[0],&li259[0],&li260[0],&li261[0],&li262[0],&li263[0],
       &li264[0],&li265[0],&li266[0],&li267[0],&li268[0],&li269[0],
       &li270[0],&li271[0],&li272[0],&li273[0],&li274[0],&li275[0],
       &li276[0],&li277[0],&li278[0],&li279[0],&li280[0],&li281[0],
       &li282[0],&li283[0],&li284[0],&li285[0],&li286[0],&li287[0],
       &li288[0],&li289[0],&li290[0],&li291[0],&li292[0],&li293[0],
       &li294[0],&li295[0],&li296[0],&li297[0],&li298[0],&li299[0],
       &li300[0],&li301[0],&li302[0],&li303[0],&li304[0],&li305[0],
       &li306[0],&li307[0],&li308[0],&li309[0],&li310[0],&li311[0],
       &li312[0],&li313[0],&li314[0],&li315[0],&li316[0],&li317[0],
       &li318[0],&li319[0],&li320[0],&li321[0],&li322[0],&li323[0],
       &li324[0],&li325[0],&li326[0],&li327[0],&li328[0],&li329[0],
       &li330[0],&li331[0],&li332[0],&li333[0],&li334[0],&li335[0],
       &li336[0],&li337[0],&li338[0],&li339[0],&li340[0],&li341[0],
       &li342[0],&li343[0],&li344[0],&li345[0],&li346[0],&li347[0],
       &li348[0],&li349[0],&li350[0],&li351[0],&li352[0],&li353[0],
       &li354[0],&li355[0],&li356[0],&li357[0],&li358[0],&li359[0],
       &li360[0],&li361[0],&li362[0],&li363[0],&li364[0],&li365[0],
       &li366[0],&li367[0],&li368[0],&li369[0],&li370[0],&li371[0],
       &li372[0],&li373[0],&li374[0],&li375[0],&li376[0],&li377[0],
       &li378[0],&li379[0],&li380[0],&li381[0],&li382[0],&li383[0],
       &li384[0],&li385[0],&li386[0],&li387[0],&li388[0],&li389[0],
       &li390[0],&li391[0],&li392[0],&li393[0],&li394[0],&li395[0],
       &li396[0],&li397[0],&li398[0],&li399[0],
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         int n = sizeof(li)/sizeof(LinInstr*);
         for (int i=0; i<n; i++) {
           std::string s = Test::str(i);
           if (i < 10) {
             s = "00" + s;
           } else if (i < 100) {
             s = "0" + s;
           }
           (void) new LinExprInt(li[i],s);
           (void) new LinExprBool(li[i],s);
           (void) new LinExprMixed(li[i],s);
         }
         IntRelTypes irts;
         for (int i=0; i<n/2; i++) {
           std::string s = Test::str(i);
           if (i < 10) {
             s = "00" + s;
           } else if (i < 100) {
             s = "0" + s;
           }
           (void) new LinRelInt(li[2*i],li[2*i+1],irts.irt(),s);
           (void) new LinRelBool(li[2*i],li[2*i+1],irts.irt(),s);
           (void) new LinRelMixed(li[2*i],li[2*i+1],irts.irt(),s);
           ++irts;
           if (!irts())
             irts.reset();
         }
       }
     };

     Create c;
     //@}
   }

}}

// STATISTICS: test-minimodel
