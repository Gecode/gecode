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

  /// %Tests for minimal modelling constraints (%Set)
  namespace MiniModelSet {

    /// Set opcode
    enum SetOpcode {
      SO_CMPL,   ///< Complement
      SO_UNION,  ///< Union
      SO_DUNION, ///< Disjoint union
      SO_INTER,  ///< Intersection
      SO_MINUS,  ///< Difference
      SO_HLT     ///< Stop execution
    };
    
    /// Type for representing a set instruction
    class SetInstr {
    public:
      SetOpcode o; ///< Which instruction to execute
      unsigned char x, y, z;  ///< Instruction arguments, \a z is destination (or \a y for complement)
    };
    
    /// Executes set instruction for evaluation (checking)
    int
    eval(const SetInstr* pc, int reg[], bool& failed) {
      failed = false;
      while (true) {
        switch (pc->o) {
        case SO_CMPL: reg[pc->y] = !reg[pc->x]; break;
        case SO_INTER: reg[pc->z] = reg[pc->x] & reg[pc->y]; break;
        case SO_UNION:  reg[pc->z] = reg[pc->x] | reg[pc->y]; break;
        case SO_DUNION: 
          if (reg[pc->x] && reg[pc->y])
            failed = true;
          reg[pc->z] = reg[pc->x] | reg[pc->y]; break;
        case SO_MINUS: reg[pc->z] = reg[pc->x] & (!reg[pc->y]); break;
        case SO_HLT: return reg[pc->x];
        default: GECODE_NEVER;
        }
        pc++;
      }
      GECODE_NEVER;
    }
    
    /// Executes set instruction for constructing set expressions
    Gecode::SetExpr
    eval(const SetInstr* pc, Gecode::SetExpr reg[]) {
      using namespace Gecode;
      while (true) {
        switch (pc->o) {
        case SO_CMPL:   reg[pc->y] = ((-reg[pc->x]) & singleton(1)); break;
        case SO_INTER:  reg[pc->z] = (reg[pc->x] & reg[pc->y]); break;
        case SO_UNION:  reg[pc->z] = (reg[pc->x] | reg[pc->y]); break;
        case SO_DUNION: reg[pc->z] = reg[pc->x] + reg[pc->y]; break;
        case SO_MINUS:  reg[pc->z] = reg[pc->x] - reg[pc->y]; break;
        case SO_HLT: return reg[pc->x];
        default: GECODE_NEVER;
        }
        pc++;
      }
      GECODE_NEVER;
    }
    
    bool
    simpleReifiedSemantics(const SetInstr* pc) {
      while (pc->o != SO_HLT) {
        if (pc->o == SO_DUNION)
          return false;
        pc++;
      }
      return true;
    }
    
    /**
     * \defgroup TaskTestSetMiniModelSet Minimal modelling constraints (%Set constraints)
     * \ingroup TaskTestSet
     */
    //@{
    /// %Test set expressions with constant result
    class SetExprConst : public Test {
    protected:
      /// %Set instruction sequence
      const SetInstr* bis;
      /// Result of expression
      int c;
      /// %Set relation
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      SetExprConst(const SetInstr* bis0, const std::string& s, 
                   Gecode::SetRelType srt0, int c0)
        : Test("MiniModel::SetExpr::Const::"+s+"::"+str(srt0)+"::"+str(c0),
               4,0,1,simpleReifiedSemantics(bis0)),
          bis(bis0), c(c0), srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int reg[4] = {(x[0] != x[2]), x[1],
                      (x[2] > 0), x[3]};
        bool failed;
        int ret = eval(bis, reg, failed);
        if (failed)
          return false;
        switch (srt) {
          case Gecode::SRT_EQ: return ret == c;
          case Gecode::SRT_NQ: return ret != c;
          case Gecode::SRT_SUB: return ret <= c;
          case Gecode::SRT_SUP: return ret >= c;
          case Gecode::SRT_DISJ: return ret+c != 2;
          case Gecode::SRT_CMPL: return ret != c;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        using namespace Gecode;
        SetVarArgs s(home,4,IntSet::empty,1,1);
        Gecode::rel(home, (singleton(1) == s[0]) == (x[0] != x[2]));
        Gecode::rel(home, (singleton(1) == s[1]) == (x[1] == 1));
        Gecode::rel(home, (singleton(1) == s[2]) == (x[2] > 0));
        Gecode::rel(home, (singleton(1) == s[3]) == (x[3] == 1));
        Gecode::SetExpr reg[4] = {s[0],s[1],s[2],s[3]};
        Gecode::SetExpr res = (c==0) ? IntSet::empty : singleton(1);
        Gecode::SetExpr e = eval(bis,reg);
        switch (srt) {
          case Gecode::SRT_EQ: Gecode::rel(home, e == res); break;
          case Gecode::SRT_NQ: Gecode::rel(home, e != res); break;
          case Gecode::SRT_SUB: Gecode::rel(home, e <= res); break;
          case Gecode::SRT_SUP: Gecode::rel(home, e >= res); break;
          case Gecode::SRT_DISJ: Gecode::rel(home, e || res); break;
          case Gecode::SRT_CMPL: Gecode::rel(home, e == -res); break;
        }
      }
      /// Post reified constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x, 
                        Gecode::BoolVar b) {
        using namespace Gecode;
        SetVarArgs s(home,4,IntSet::empty,1,1);
        Gecode::rel(home, (singleton(1) == s[0]) == (x[0] != x[2]));
        Gecode::rel(home, (singleton(1) == s[1]) == (x[1] == 1));
        Gecode::rel(home, (singleton(1) == s[2]) == (x[2] > 0));
        Gecode::rel(home, (singleton(1) == s[3]) == (x[3] == 1));
        Gecode::SetExpr reg[4] = {s[0],s[1],s[2],s[3]};
        Gecode::SetExpr res = (c==0) ? IntSet::empty : singleton(1);
        Gecode::SetExpr e = eval(bis,reg);
        switch (srt) {
          case Gecode::SRT_EQ: Gecode::rel(home, (e == res)==b); break;
          case Gecode::SRT_NQ: Gecode::rel(home, (e != res)==b); break;
          case Gecode::SRT_SUB: Gecode::rel(home, (e <= res)==b); break;
          case Gecode::SRT_SUP: Gecode::rel(home, (e >= res)==b); break;
          case Gecode::SRT_DISJ: Gecode::rel(home, (e || res)==b); break;
          case Gecode::SRT_CMPL: Gecode::rel(home, (e == -res)==b); break;
        }
      }
    };
    
    /// %Test set expressions with expression result
    class SetExprExpr : public Test {
    protected:
      /// %First set instruction sequence
      const SetInstr* bis0;
      /// %Second set instruction sequence
      const SetInstr* bis1;
      /// %Set relation
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      SetExprExpr(const SetInstr* bis00, const SetInstr* bis10,
                  const std::string& s, Gecode::SetRelType srt0)
        : Test("MiniModel::SetExpr::Expr::"+s+"::"+str(srt0),
               8,0,1,
               simpleReifiedSemantics(bis00) && 
               simpleReifiedSemantics(bis10)),
          bis0(bis00), bis1(bis10), srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int reg0[4] = {(x[0] != x[2]), x[1],
                       (x[2] > 0), x[3]};
        bool failed0;
        int ret0 = eval(bis0, reg0, failed0);
        if (failed0)
          return false;
    
        int reg1[4] = {(x[4] != x[6]), x[5],
                       (x[6] > 0), x[7]};
        bool failed1;
        int ret1 = eval(bis1, reg1, failed1);
    
        if (failed1)
          return false;
    
        switch (srt) {
          case Gecode::SRT_EQ: return ret0 == ret1;
          case Gecode::SRT_NQ: return ret0 != ret1;
          case Gecode::SRT_SUB: return ret0 <= ret1;
          case Gecode::SRT_SUP: return ret0 >= ret1;
          case Gecode::SRT_DISJ: return ret0+ret1 != 2;
          case Gecode::SRT_CMPL: return ret0 != ret1;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        using namespace Gecode;
        SetVarArgs s(home,8,IntSet::empty,1,1);
        Gecode::rel(home, (singleton(1) == s[0]) == (x[0] != x[2]));
        Gecode::rel(home, (singleton(1) == s[1]) == (x[1] == 1));
        Gecode::rel(home, (singleton(1) == s[2]) == (x[2] > 0));
        Gecode::rel(home, (singleton(1) == s[3]) == (x[3] == 1));
    
        Gecode::rel(home, (singleton(1) == s[4]) == (x[4] != x[6]));
        Gecode::rel(home, (singleton(1) == s[5]) == (x[5] == 1));
        Gecode::rel(home, (singleton(1) == s[6]) == (x[6] > 0));
        Gecode::rel(home, (singleton(1) == s[7]) == (x[7] == 1));
    
        Gecode::SetExpr reg0[4] = {s[0],s[1],s[2],s[3]};
        Gecode::SetExpr e0 = eval(bis0,reg0);
    
        Gecode::SetExpr reg1[4] = {s[4],s[5],s[6],s[7]};
        Gecode::SetExpr e1 = eval(bis1,reg1);
    
        switch (srt) {
          case Gecode::SRT_EQ: Gecode::rel(home, e0 == e1); break;
          case Gecode::SRT_NQ: Gecode::rel(home, e0 != e1); break;
          case Gecode::SRT_SUB: Gecode::rel(home, e0 <= e1); break;
          case Gecode::SRT_SUP: Gecode::rel(home, e0 >= e1); break;
          case Gecode::SRT_DISJ: Gecode::rel(home, e0 || e1); break;
          case Gecode::SRT_CMPL: Gecode::rel(home, e0 == -e1); break;
        }
      }
      /// Post reified constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                        Gecode::BoolVar b) {
        using namespace Gecode;
        SetVarArgs s(home,8,IntSet::empty,1,1);
        Gecode::rel(home, (singleton(1) == s[0]) == (x[0] != x[2]));
        Gecode::rel(home, (singleton(1) == s[1]) == (x[1] == 1));
        Gecode::rel(home, (singleton(1) == s[2]) == (x[2] > 0));
        Gecode::rel(home, (singleton(1) == s[3]) == (x[3] == 1));
    
        Gecode::rel(home, (singleton(1) == s[4]) == (x[4] != x[6]));
        Gecode::rel(home, (singleton(1) == s[5]) == (x[5] == 1));
        Gecode::rel(home, (singleton(1) == s[6]) == (x[6] > 0));
        Gecode::rel(home, (singleton(1) == s[7]) == (x[7] == 1));
    
        Gecode::SetExpr reg0[4] = {s[0],s[1],s[2],s[3]};
        Gecode::SetExpr e0 = eval(bis0,reg0);
    
        Gecode::SetExpr reg1[4] = {s[4],s[5],s[6],s[7]};
        Gecode::SetExpr e1 = eval(bis1,reg1);
    
        switch (srt) {
          case Gecode::SRT_EQ: Gecode::rel(home, (e0 == e1)==b); break;
          case Gecode::SRT_NQ: Gecode::rel(home, (e0 != e1)==b); break;
          case Gecode::SRT_SUB: Gecode::rel(home, (e0 <= e1)==b); break;
          case Gecode::SRT_SUP: Gecode::rel(home, (e0 >= e1)==b); break;
          case Gecode::SRT_DISJ: Gecode::rel(home, (e0 || e1)==b); break;
          case Gecode::SRT_CMPL: Gecode::rel(home, (e0 == -e1)==b); break;
        }
      }
    };

    const SetInstr si000[] = {
      {SO_INTER,0,1,0},{SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si001[] = {
      {SO_INTER,0,1,0},{SO_INTER,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si002[] = {
      {SO_INTER,2,3,2},{SO_INTER,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si003[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_INTER,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si004[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si005[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si006[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si007[] = {
      {SO_INTER,0,1,0},{SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si008[] = {
      {SO_INTER,0,1,0},{SO_INTER,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si009[] = {
      {SO_INTER,2,3,2},{SO_INTER,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si010[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si011[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si012[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si013[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si014[] = {
      {SO_INTER,0,1,0},{SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si015[] = {
      {SO_INTER,0,1,0},{SO_INTER,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si016[] = {
      {SO_INTER,2,3,2},{SO_INTER,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si017[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si018[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si019[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si020[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si021[] = {
      {SO_INTER,0,1,0},{SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si022[] = {
      {SO_INTER,0,1,0},{SO_INTER,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si023[] = {
      {SO_INTER,2,3,2},{SO_INTER,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si024[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_INTER,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si025[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si026[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si027[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si028[] = {
      {SO_INTER,0,1,0},{SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si029[] = {
      {SO_INTER,0,1,0},{SO_INTER,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si030[] = {
      {SO_INTER,2,3,2},{SO_INTER,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si031[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_INTER,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si032[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si033[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si034[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si035[] = {
      {SO_INTER,0,1,0},{SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si036[] = {
      {SO_INTER,0,1,0},{SO_UNION ,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si037[] = {
      {SO_INTER,2,3,2},{SO_UNION ,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si038[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION ,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si039[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si040[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si041[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si042[] = {
      {SO_INTER,0,1,0},{SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si043[] = {
      {SO_INTER,0,1,0},{SO_UNION ,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si044[] = {
      {SO_INTER,2,3,2},{SO_UNION ,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si045[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si046[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si047[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si048[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si049[] = {
      {SO_INTER,0,1,0},{SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si050[] = {
      {SO_INTER,0,1,0},{SO_UNION ,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si051[] = {
      {SO_INTER,2,3,2},{SO_UNION ,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si052[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si053[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si054[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si055[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si056[] = {
      {SO_INTER,0,1,0},{SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si057[] = {
      {SO_INTER,0,1,0},{SO_UNION ,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si058[] = {
      {SO_INTER,2,3,2},{SO_UNION ,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si059[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION ,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si060[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si061[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si062[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si063[] = {
      {SO_INTER,0,1,0},{SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si064[] = {
      {SO_INTER,0,1,0},{SO_UNION ,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si065[] = {
      {SO_INTER,2,3,2},{SO_UNION ,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si066[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION ,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si067[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si068[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si069[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si070[] = {
      {SO_INTER,0,1,0},{SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si071[] = {
      {SO_INTER,0,1,0},{SO_UNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si072[] = {
      {SO_INTER,2,3,2},{SO_UNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si073[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si074[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si075[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si076[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si077[] = {
      {SO_INTER,0,1,0},{SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si078[] = {
      {SO_INTER,0,1,0},{SO_UNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si079[] = {
      {SO_INTER,2,3,2},{SO_UNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si080[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si081[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si082[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si083[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si084[] = {
      {SO_INTER,0,1,0},{SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si085[] = {
      {SO_INTER,0,1,0},{SO_UNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si086[] = {
      {SO_INTER,2,3,2},{SO_UNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si087[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si088[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si089[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si090[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si091[] = {
      {SO_INTER,0,1,0},{SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si092[] = {
      {SO_INTER,0,1,0},{SO_UNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si093[] = {
      {SO_INTER,2,3,2},{SO_UNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si094[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si095[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si096[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si097[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si098[] = {
      {SO_INTER,0,1,0},{SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si099[] = {
      {SO_INTER,0,1,0},{SO_UNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si100[] = {
      {SO_INTER,2,3,2},{SO_UNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si101[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_UNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si102[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si103[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si104[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si105[] = {
      {SO_INTER,0,1,0},{SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si106[] = {
      {SO_INTER,0,1,0},{SO_DUNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si107[] = {
      {SO_INTER,2,3,2},{SO_DUNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si108[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_DUNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si109[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si110[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si111[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si112[] = {
      {SO_INTER,0,1,0},{SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si113[] = {
      {SO_INTER,0,1,0},{SO_DUNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si114[] = {
      {SO_INTER,2,3,2},{SO_DUNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si115[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si116[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si117[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si118[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si119[] = {
      {SO_INTER,0,1,0},{SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si120[] = {
      {SO_INTER,0,1,0},{SO_DUNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si121[] = {
      {SO_INTER,2,3,2},{SO_DUNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si122[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si123[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si124[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si125[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si126[] = {
      {SO_INTER,0,1,0},{SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si127[] = {
      {SO_INTER,0,1,0},{SO_DUNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si128[] = {
      {SO_INTER,2,3,2},{SO_DUNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si129[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_DUNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si130[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si131[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si132[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si133[] = {
      {SO_INTER,0,1,0},{SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si134[] = {
      {SO_INTER,0,1,0},{SO_DUNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si135[] = {
      {SO_INTER,2,3,2},{SO_DUNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si136[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_DUNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si137[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si138[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si139[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si140[] = {
      {SO_INTER,0,1,0},{SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si141[] = {
      {SO_INTER,0,1,0},{SO_MINUS,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si142[] = {
      {SO_INTER,2,3,2},{SO_MINUS,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si143[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_MINUS,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si144[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si145[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si146[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si147[] = {
      {SO_INTER,0,1,0},{SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si148[] = {
      {SO_INTER,0,1,0},{SO_MINUS,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si149[] = {
      {SO_INTER,2,3,2},{SO_MINUS,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si150[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si151[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si152[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si153[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si154[] = {
      {SO_INTER,0,1,0},{SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si155[] = {
      {SO_INTER,0,1,0},{SO_MINUS,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si156[] = {
      {SO_INTER,2,3,2},{SO_MINUS,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si157[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si158[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si159[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si160[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si161[] = {
      {SO_INTER,0,1,0},{SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si162[] = {
      {SO_INTER,0,1,0},{SO_MINUS,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si163[] = {
      {SO_INTER,2,3,2},{SO_MINUS,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si164[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_MINUS,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si165[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si166[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si167[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si168[] = {
      {SO_INTER,0,1,0},{SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si169[] = {
      {SO_INTER,0,1,0},{SO_MINUS,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si170[] = {
      {SO_INTER,2,3,2},{SO_MINUS,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si171[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_INTER,0,1,0},{SO_MINUS,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si172[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_INTER,0,1,0},
      {SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si173[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si174[] = {
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si175[] = {
      {SO_UNION ,0,1,0},{SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si176[] = {
      {SO_UNION ,0,1,0},{SO_INTER,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si177[] = {
      {SO_UNION ,2,3,2},{SO_INTER,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si178[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_INTER,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si179[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si180[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si181[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si182[] = {
      {SO_UNION ,0,1,0},{SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si183[] = {
      {SO_UNION ,0,1,0},{SO_INTER,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si184[] = {
      {SO_UNION ,2,3,2},{SO_INTER,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si185[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si186[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si187[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si188[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si189[] = {
      {SO_UNION ,0,1,0},{SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si190[] = {
      {SO_UNION ,0,1,0},{SO_INTER,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si191[] = {
      {SO_UNION ,2,3,2},{SO_INTER,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si192[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si193[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si194[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si195[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si196[] = {
      {SO_UNION ,0,1,0},{SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si197[] = {
      {SO_UNION ,0,1,0},{SO_INTER,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si198[] = {
      {SO_UNION ,2,3,2},{SO_INTER,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si199[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_INTER,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si200[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si201[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si202[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si203[] = {
      {SO_UNION ,0,1,0},{SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si204[] = {
      {SO_UNION ,0,1,0},{SO_INTER,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si205[] = {
      {SO_UNION ,2,3,2},{SO_INTER,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si206[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_INTER,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si207[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si208[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si209[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si210[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si211[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si212[] = {
      {SO_UNION ,2,3,2},{SO_UNION ,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si213[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION ,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si214[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si215[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si216[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si217[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si218[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si219[] = {
      {SO_UNION ,2,3,2},{SO_UNION ,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si220[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si221[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si222[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si223[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si224[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si225[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si226[] = {
      {SO_UNION ,2,3,2},{SO_UNION ,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si227[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si228[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si229[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si230[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si231[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si232[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si233[] = {
      {SO_UNION ,2,3,2},{SO_UNION ,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si234[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION ,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si235[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si236[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si237[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si238[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si239[] = {
      {SO_UNION ,0,1,0},{SO_UNION ,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si240[] = {
      {SO_UNION ,2,3,2},{SO_UNION ,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si241[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION ,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si242[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si243[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si244[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si245[] = {
      {SO_UNION ,0,1,0},{SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si246[] = {
      {SO_UNION ,0,1,0},{SO_UNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si247[] = {
      {SO_UNION ,2,3,2},{SO_UNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si248[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si249[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si250[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si251[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si252[] = {
      {SO_UNION ,0,1,0},{SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si253[] = {
      {SO_UNION ,0,1,0},{SO_UNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si254[] = {
      {SO_UNION ,2,3,2},{SO_UNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si255[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si256[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si257[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si258[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si259[] = {
      {SO_UNION ,0,1,0},{SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si260[] = {
      {SO_UNION ,0,1,0},{SO_UNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si261[] = {
      {SO_UNION ,2,3,2},{SO_UNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si262[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si263[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si264[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si265[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si266[] = {
      {SO_UNION ,0,1,0},{SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si267[] = {
      {SO_UNION ,0,1,0},{SO_UNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si268[] = {
      {SO_UNION ,2,3,2},{SO_UNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si269[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si270[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si271[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si272[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si273[] = {
      {SO_UNION ,0,1,0},{SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si274[] = {
      {SO_UNION ,0,1,0},{SO_UNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si275[] = {
      {SO_UNION ,2,3,2},{SO_UNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si276[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_UNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si277[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si278[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si279[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si280[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si281[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si282[] = {
      {SO_UNION ,2,3,2},{SO_DUNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si283[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_DUNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si284[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si285[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si286[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si287[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si288[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si289[] = {
      {SO_UNION ,2,3,2},{SO_DUNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si290[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si291[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si292[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si293[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si294[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si295[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si296[] = {
      {SO_UNION ,2,3,2},{SO_DUNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si297[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si298[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si299[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si300[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si301[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si302[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si303[] = {
      {SO_UNION ,2,3,2},{SO_DUNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si304[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_DUNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si305[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si306[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si307[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si308[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si309[] = {
      {SO_UNION ,0,1,0},{SO_DUNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si310[] = {
      {SO_UNION ,2,3,2},{SO_DUNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si311[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_DUNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si312[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si313[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si314[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si315[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si316[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si317[] = {
      {SO_UNION ,2,3,2},{SO_MINUS,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si318[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_MINUS,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si319[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si320[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si321[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si322[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si323[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si324[] = {
      {SO_UNION ,2,3,2},{SO_MINUS,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si325[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si326[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si327[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si328[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si329[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si330[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si331[] = {
      {SO_UNION ,2,3,2},{SO_MINUS,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si332[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si333[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si334[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si335[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si336[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si337[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si338[] = {
      {SO_UNION ,2,3,2},{SO_MINUS,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si339[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_MINUS,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si340[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si341[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si342[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si343[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si344[] = {
      {SO_UNION ,0,1,0},{SO_MINUS,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si345[] = {
      {SO_UNION ,2,3,2},{SO_MINUS,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si346[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION ,0,1,0},{SO_MINUS,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si347[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION ,0,1,0},
      {SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si348[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si349[] = {
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si350[] = {
      {SO_UNION,0,1,0},{SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si351[] = {
      {SO_UNION,0,1,0},{SO_INTER,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si352[] = {
      {SO_UNION,2,3,2},{SO_INTER,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si353[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_INTER,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si354[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si355[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si356[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si357[] = {
      {SO_UNION,0,1,0},{SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si358[] = {
      {SO_UNION,0,1,0},{SO_INTER,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si359[] = {
      {SO_UNION,2,3,2},{SO_INTER,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si360[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si361[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si362[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si363[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si364[] = {
      {SO_UNION,0,1,0},{SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si365[] = {
      {SO_UNION,0,1,0},{SO_INTER,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si366[] = {
      {SO_UNION,2,3,2},{SO_INTER,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si367[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si368[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si369[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si370[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si371[] = {
      {SO_UNION,0,1,0},{SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si372[] = {
      {SO_UNION,0,1,0},{SO_INTER,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si373[] = {
      {SO_UNION,2,3,2},{SO_INTER,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si374[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_INTER,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si375[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si376[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si377[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si378[] = {
      {SO_UNION,0,1,0},{SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si379[] = {
      {SO_UNION,0,1,0},{SO_INTER,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si380[] = {
      {SO_UNION,2,3,2},{SO_INTER,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si381[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_INTER,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si382[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si383[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si384[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si385[] = {
      {SO_UNION,0,1,0},{SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si386[] = {
      {SO_UNION,0,1,0},{SO_UNION ,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si387[] = {
      {SO_UNION,2,3,2},{SO_UNION ,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si388[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si389[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si390[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si391[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si392[] = {
      {SO_UNION,0,1,0},{SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si393[] = {
      {SO_UNION,0,1,0},{SO_UNION ,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si394[] = {
      {SO_UNION,2,3,2},{SO_UNION ,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si395[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si396[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si397[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si398[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si399[] = {
      {SO_UNION,0,1,0},{SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si400[] = {
      {SO_UNION,0,1,0},{SO_UNION ,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si401[] = {
      {SO_UNION,2,3,2},{SO_UNION ,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si402[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si403[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si404[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si405[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si406[] = {
      {SO_UNION,0,1,0},{SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si407[] = {
      {SO_UNION,0,1,0},{SO_UNION ,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si408[] = {
      {SO_UNION,2,3,2},{SO_UNION ,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si409[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si410[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si411[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si412[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si413[] = {
      {SO_UNION,0,1,0},{SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si414[] = {
      {SO_UNION,0,1,0},{SO_UNION ,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si415[] = {
      {SO_UNION,2,3,2},{SO_UNION ,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si416[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si417[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si418[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si419[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si420[] = {
      {SO_UNION,0,1,0},{SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si421[] = {
      {SO_UNION,0,1,0},{SO_UNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si422[] = {
      {SO_UNION,2,3,2},{SO_UNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si423[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si424[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si425[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si426[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si427[] = {
      {SO_UNION,0,1,0},{SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si428[] = {
      {SO_UNION,0,1,0},{SO_UNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si429[] = {
      {SO_UNION,2,3,2},{SO_UNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si430[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si431[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si432[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si433[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si434[] = {
      {SO_UNION,0,1,0},{SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si435[] = {
      {SO_UNION,0,1,0},{SO_UNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si436[] = {
      {SO_UNION,2,3,2},{SO_UNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si437[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si438[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si439[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si440[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si441[] = {
      {SO_UNION,0,1,0},{SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si442[] = {
      {SO_UNION,0,1,0},{SO_UNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si443[] = {
      {SO_UNION,2,3,2},{SO_UNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si444[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si445[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si446[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si447[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si448[] = {
      {SO_UNION,0,1,0},{SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si449[] = {
      {SO_UNION,0,1,0},{SO_UNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si450[] = {
      {SO_UNION,2,3,2},{SO_UNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si451[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_UNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si452[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si453[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si454[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si455[] = {
      {SO_UNION,0,1,0},{SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si456[] = {
      {SO_UNION,0,1,0},{SO_DUNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si457[] = {
      {SO_UNION,2,3,2},{SO_DUNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si458[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si459[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si460[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si461[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si462[] = {
      {SO_UNION,0,1,0},{SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si463[] = {
      {SO_UNION,0,1,0},{SO_DUNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si464[] = {
      {SO_UNION,2,3,2},{SO_DUNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si465[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si466[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si467[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si468[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si469[] = {
      {SO_UNION,0,1,0},{SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si470[] = {
      {SO_UNION,0,1,0},{SO_DUNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si471[] = {
      {SO_UNION,2,3,2},{SO_DUNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si472[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si473[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si474[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si475[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si476[] = {
      {SO_UNION,0,1,0},{SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si477[] = {
      {SO_UNION,0,1,0},{SO_DUNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si478[] = {
      {SO_UNION,2,3,2},{SO_DUNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si479[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si480[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si481[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si482[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si483[] = {
      {SO_UNION,0,1,0},{SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si484[] = {
      {SO_UNION,0,1,0},{SO_DUNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si485[] = {
      {SO_UNION,2,3,2},{SO_DUNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si486[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si487[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si488[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si489[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si490[] = {
      {SO_UNION,0,1,0},{SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si491[] = {
      {SO_UNION,0,1,0},{SO_MINUS,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si492[] = {
      {SO_UNION,2,3,2},{SO_MINUS,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si493[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si494[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si495[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si496[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si497[] = {
      {SO_UNION,0,1,0},{SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si498[] = {
      {SO_UNION,0,1,0},{SO_MINUS,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si499[] = {
      {SO_UNION,2,3,2},{SO_MINUS,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si500[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si501[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si502[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si503[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si504[] = {
      {SO_UNION,0,1,0},{SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si505[] = {
      {SO_UNION,0,1,0},{SO_MINUS,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si506[] = {
      {SO_UNION,2,3,2},{SO_MINUS,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si507[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si508[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si509[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si510[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si511[] = {
      {SO_UNION,0,1,0},{SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si512[] = {
      {SO_UNION,0,1,0},{SO_MINUS,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si513[] = {
      {SO_UNION,2,3,2},{SO_MINUS,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si514[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si515[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si516[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si517[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si518[] = {
      {SO_UNION,0,1,0},{SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si519[] = {
      {SO_UNION,0,1,0},{SO_MINUS,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si520[] = {
      {SO_UNION,2,3,2},{SO_MINUS,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si521[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_UNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si522[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_UNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si523[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si524[] = {
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si525[] = {
      {SO_DUNION,0,1,0},{SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si526[] = {
      {SO_DUNION,0,1,0},{SO_INTER,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si527[] = {
      {SO_DUNION,2,3,2},{SO_INTER,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si528[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_INTER,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si529[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si530[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si531[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si532[] = {
      {SO_DUNION,0,1,0},{SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si533[] = {
      {SO_DUNION,0,1,0},{SO_INTER,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si534[] = {
      {SO_DUNION,2,3,2},{SO_INTER,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si535[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si536[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si537[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si538[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si539[] = {
      {SO_DUNION,0,1,0},{SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si540[] = {
      {SO_DUNION,0,1,0},{SO_INTER,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si541[] = {
      {SO_DUNION,2,3,2},{SO_INTER,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si542[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si543[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si544[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si545[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si546[] = {
      {SO_DUNION,0,1,0},{SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si547[] = {
      {SO_DUNION,0,1,0},{SO_INTER,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si548[] = {
      {SO_DUNION,2,3,2},{SO_INTER,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si549[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_INTER,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si550[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si551[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si552[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si553[] = {
      {SO_DUNION,0,1,0},{SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si554[] = {
      {SO_DUNION,0,1,0},{SO_INTER,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si555[] = {
      {SO_DUNION,2,3,2},{SO_INTER,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si556[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_INTER,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si557[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si558[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si559[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si560[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si561[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si562[] = {
      {SO_DUNION,2,3,2},{SO_UNION ,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si563[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si564[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si565[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si566[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si567[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si568[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si569[] = {
      {SO_DUNION,2,3,2},{SO_UNION ,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si570[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si571[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si572[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si573[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si574[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si575[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si576[] = {
      {SO_DUNION,2,3,2},{SO_UNION ,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si577[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si578[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si579[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si580[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si581[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si582[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si583[] = {
      {SO_DUNION,2,3,2},{SO_UNION ,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si584[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si585[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si586[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si587[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si588[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si589[] = {
      {SO_DUNION,0,1,0},{SO_UNION ,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si590[] = {
      {SO_DUNION,2,3,2},{SO_UNION ,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si591[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION ,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si592[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si593[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si594[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si595[] = {
      {SO_DUNION,0,1,0},{SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si596[] = {
      {SO_DUNION,0,1,0},{SO_UNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si597[] = {
      {SO_DUNION,2,3,2},{SO_UNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si598[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si599[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si600[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si601[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si602[] = {
      {SO_DUNION,0,1,0},{SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si603[] = {
      {SO_DUNION,0,1,0},{SO_UNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si604[] = {
      {SO_DUNION,2,3,2},{SO_UNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si605[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si606[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si607[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si608[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si609[] = {
      {SO_DUNION,0,1,0},{SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si610[] = {
      {SO_DUNION,0,1,0},{SO_UNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si611[] = {
      {SO_DUNION,2,3,2},{SO_UNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si612[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si613[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si614[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si615[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si616[] = {
      {SO_DUNION,0,1,0},{SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si617[] = {
      {SO_DUNION,0,1,0},{SO_UNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si618[] = {
      {SO_DUNION,2,3,2},{SO_UNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si619[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si620[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si621[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si622[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si623[] = {
      {SO_DUNION,0,1,0},{SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si624[] = {
      {SO_DUNION,0,1,0},{SO_UNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si625[] = {
      {SO_DUNION,2,3,2},{SO_UNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si626[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_UNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si627[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si628[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si629[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si630[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si631[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si632[] = {
      {SO_DUNION,2,3,2},{SO_DUNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si633[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si634[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si635[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si636[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si637[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si638[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si639[] = {
      {SO_DUNION,2,3,2},{SO_DUNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si640[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si641[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si642[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si643[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si644[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si645[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si646[] = {
      {SO_DUNION,2,3,2},{SO_DUNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si647[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si648[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si649[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si650[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si651[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si652[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si653[] = {
      {SO_DUNION,2,3,2},{SO_DUNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si654[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si655[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si656[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si657[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si658[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si659[] = {
      {SO_DUNION,0,1,0},{SO_DUNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si660[] = {
      {SO_DUNION,2,3,2},{SO_DUNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si661[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_DUNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si662[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si663[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si664[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si665[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si666[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si667[] = {
      {SO_DUNION,2,3,2},{SO_MINUS,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si668[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si669[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si670[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si671[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si672[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si673[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si674[] = {
      {SO_DUNION,2,3,2},{SO_MINUS,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si675[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si676[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si677[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si678[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si679[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si680[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si681[] = {
      {SO_DUNION,2,3,2},{SO_MINUS,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si682[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si683[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si684[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si685[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si686[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si687[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si688[] = {
      {SO_DUNION,2,3,2},{SO_MINUS,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si689[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si690[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si691[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si692[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si693[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si694[] = {
      {SO_DUNION,0,1,0},{SO_MINUS,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si695[] = {
      {SO_DUNION,2,3,2},{SO_MINUS,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si696[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_DUNION,0,1,0},{SO_MINUS,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si697[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_DUNION,0,1,0},
      {SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si698[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si699[] = {
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si700[] = {
      {SO_MINUS,0,1,0},{SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si701[] = {
      {SO_MINUS,0,1,0},{SO_INTER,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si702[] = {
      {SO_MINUS,2,3,2},{SO_INTER,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si703[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_INTER,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si704[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_INTER,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si705[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si706[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si707[] = {
      {SO_MINUS,0,1,0},{SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si708[] = {
      {SO_MINUS,0,1,0},{SO_INTER,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si709[] = {
      {SO_MINUS,2,3,2},{SO_INTER,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si710[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si711[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si712[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si713[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si714[] = {
      {SO_MINUS,0,1,0},{SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si715[] = {
      {SO_MINUS,0,1,0},{SO_INTER,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si716[] = {
      {SO_MINUS,2,3,2},{SO_INTER,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si717[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_INTER,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si718[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_INTER,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si719[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si720[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si721[] = {
      {SO_MINUS,0,1,0},{SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si722[] = {
      {SO_MINUS,0,1,0},{SO_INTER,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si723[] = {
      {SO_MINUS,2,3,2},{SO_INTER,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si724[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_INTER,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si725[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_INTER,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si726[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si727[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si728[] = {
      {SO_MINUS,0,1,0},{SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si729[] = {
      {SO_MINUS,0,1,0},{SO_INTER,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si730[] = {
      {SO_MINUS,2,3,2},{SO_INTER,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si731[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_INTER,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si732[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_INTER,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si733[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si734[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_INTER,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si735[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si736[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si737[] = {
      {SO_MINUS,2,3,2},{SO_UNION ,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si738[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION ,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si739[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION ,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si740[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si741[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si742[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si743[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si744[] = {
      {SO_MINUS,2,3,2},{SO_UNION ,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si745[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si746[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si747[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si748[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si749[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si750[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si751[] = {
      {SO_MINUS,2,3,2},{SO_UNION ,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si752[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION ,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si753[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION ,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si754[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si755[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si756[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si757[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si758[] = {
      {SO_MINUS,2,3,2},{SO_UNION ,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si759[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION ,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si760[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION ,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si761[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si762[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si763[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si764[] = {
      {SO_MINUS,0,1,0},{SO_UNION ,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si765[] = {
      {SO_MINUS,2,3,2},{SO_UNION ,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si766[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION ,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si767[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION ,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si768[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si769[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION ,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si770[] = {
      {SO_MINUS,0,1,0},{SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si771[] = {
      {SO_MINUS,0,1,0},{SO_UNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si772[] = {
      {SO_MINUS,2,3,2},{SO_UNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si773[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si774[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si775[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si776[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si777[] = {
      {SO_MINUS,0,1,0},{SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si778[] = {
      {SO_MINUS,0,1,0},{SO_UNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si779[] = {
      {SO_MINUS,2,3,2},{SO_UNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si780[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si781[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si782[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si783[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si784[] = {
      {SO_MINUS,0,1,0},{SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si785[] = {
      {SO_MINUS,0,1,0},{SO_UNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si786[] = {
      {SO_MINUS,2,3,2},{SO_UNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si787[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si788[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si789[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si790[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si791[] = {
      {SO_MINUS,0,1,0},{SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si792[] = {
      {SO_MINUS,0,1,0},{SO_UNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si793[] = {
      {SO_MINUS,2,3,2},{SO_UNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si794[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si795[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si796[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si797[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si798[] = {
      {SO_MINUS,0,1,0},{SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si799[] = {
      {SO_MINUS,0,1,0},{SO_UNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si800[] = {
      {SO_MINUS,2,3,2},{SO_UNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si801[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_UNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si802[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_UNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si803[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si804[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_UNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si805[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si806[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si807[] = {
      {SO_MINUS,2,3,2},{SO_DUNION,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si808[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_DUNION,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si809[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_DUNION,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si810[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si811[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si812[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si813[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si814[] = {
      {SO_MINUS,2,3,2},{SO_DUNION,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si815[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si816[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si817[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si818[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si819[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si820[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si821[] = {
      {SO_MINUS,2,3,2},{SO_DUNION,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si822[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_DUNION,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si823[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_DUNION,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si824[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si825[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si826[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si827[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si828[] = {
      {SO_MINUS,2,3,2},{SO_DUNION,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si829[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_DUNION,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si830[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_DUNION,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si831[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si832[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si833[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si834[] = {
      {SO_MINUS,0,1,0},{SO_DUNION,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si835[] = {
      {SO_MINUS,2,3,2},{SO_DUNION,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si836[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_DUNION,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si837[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_DUNION,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si838[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si839[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_DUNION,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si840[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si841[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,0,2,0},{SO_INTER,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si842[] = {
      {SO_MINUS,2,3,2},{SO_MINUS,1,2,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si843[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_MINUS,2,3,1},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si844[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_MINUS,2,3,1},{SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si845[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si846[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_INTER,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si847[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si848[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,0,2,0},{SO_UNION ,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si849[] = {
      {SO_MINUS,2,3,2},{SO_MINUS,1,2,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si850[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si851[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si852[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si853[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION ,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si854[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si855[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,0,2,0},{SO_UNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si856[] = {
      {SO_MINUS,2,3,2},{SO_MINUS,1,2,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si857[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_MINUS,2,3,1},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si858[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_MINUS,2,3,1},{SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si859[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si860[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_UNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si861[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si862[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,0,2,0},{SO_DUNION,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si863[] = {
      {SO_MINUS,2,3,2},{SO_MINUS,1,2,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si864[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_MINUS,2,3,1},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si865[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_MINUS,2,3,1},{SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si866[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si867[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_DUNION,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si868[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si869[] = {
      {SO_MINUS,0,1,0},{SO_MINUS,0,2,0},{SO_MINUS,0,3,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si870[] = {
      {SO_MINUS,2,3,2},{SO_MINUS,1,2,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si871[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_MINUS,0,1,0},{SO_MINUS,2,3,1},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si872[] = {
      {SO_CMPL,0,0,0},{SO_CMPL,2,2,0},{SO_CMPL,0,0,0},{SO_MINUS,0,1,0},
      {SO_MINUS,2,3,1},{SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si873[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si874[] = {
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},{SO_MINUS,2,3,1},{SO_CMPL,1,1,0},
      {SO_MINUS,0,1,0},{SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si875[] = {
      {SO_CMPL,0,0,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si876[] = {
      {SO_INTER,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si877[] = {
      {SO_UNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si878[] = {
      {SO_DUNION,0,1,0},
      {SO_HLT,0,0,0}
    };
    const SetInstr si879[] = {
      {SO_MINUS,0,1,0},
      {SO_HLT,0,0,0}
    };
    
    
    
    const SetInstr* si[] = {
      &si000[0],&si001[0],&si002[0],&si003[0],&si004[0],&si005[0],
      &si006[0],&si007[0],&si008[0],&si009[0],&si010[0],&si011[0],
      &si012[0],&si013[0],&si014[0],&si015[0],&si016[0],&si017[0],
      &si018[0],&si019[0],&si020[0],&si021[0],&si022[0],&si023[0],
      &si024[0],&si025[0],&si026[0],&si027[0],&si028[0],&si029[0],
      &si030[0],&si031[0],&si032[0],&si033[0],&si034[0],&si035[0],
      &si036[0],&si037[0],&si038[0],&si039[0],&si040[0],&si041[0],
      &si042[0],&si043[0],&si044[0],&si045[0],&si046[0],&si047[0],
      &si048[0],&si049[0],&si050[0],&si051[0],&si052[0],&si053[0],
      &si054[0],&si055[0],&si056[0],&si057[0],&si058[0],&si059[0],
      &si060[0],&si061[0],&si062[0],&si063[0],&si064[0],&si065[0],
      &si066[0],&si067[0],&si068[0],&si069[0],&si070[0],&si071[0],
      &si072[0],&si073[0],&si074[0],&si075[0],&si076[0],&si077[0],
      &si078[0],&si079[0],&si080[0],&si081[0],&si082[0],&si083[0],
      &si084[0],&si085[0],&si086[0],&si087[0],&si088[0],&si089[0],
      &si090[0],&si091[0],&si092[0],&si093[0],&si094[0],&si095[0],
      &si096[0],&si097[0],&si098[0],&si099[0],&si100[0],&si101[0],
      &si102[0],&si103[0],&si104[0],&si105[0],&si106[0],&si107[0],
      &si108[0],&si109[0],&si110[0],&si111[0],&si112[0],&si113[0],
      &si114[0],&si115[0],&si116[0],&si117[0],&si118[0],&si119[0],
      &si120[0],&si121[0],&si122[0],&si123[0],&si124[0],&si125[0],
      &si126[0],&si127[0],&si128[0],&si129[0],&si130[0],&si131[0],
      &si132[0],&si133[0],&si134[0],&si135[0],&si136[0],&si137[0],
      &si138[0],&si139[0],&si140[0],&si141[0],&si142[0],&si143[0],
      &si144[0],&si145[0],&si146[0],&si147[0],&si148[0],&si149[0],
      &si150[0],&si151[0],&si152[0],&si153[0],&si154[0],&si155[0],
      &si156[0],&si157[0],&si158[0],&si159[0],&si160[0],&si161[0],
      &si162[0],&si163[0],&si164[0],&si165[0],&si166[0],&si167[0],
      &si168[0],&si169[0],&si170[0],&si171[0],&si172[0],&si173[0],
      &si174[0],&si175[0],&si176[0],&si177[0],&si178[0],&si179[0],
      &si180[0],&si181[0],&si182[0],&si183[0],&si184[0],&si185[0],
      &si186[0],&si187[0],&si188[0],&si189[0],&si190[0],&si191[0],
      &si192[0],&si193[0],&si194[0],&si195[0],&si196[0],&si197[0],
      &si198[0],&si199[0],&si200[0],&si201[0],&si202[0],&si203[0],
      &si204[0],&si205[0],&si206[0],&si207[0],&si208[0],&si209[0],
      &si210[0],&si211[0],&si212[0],&si213[0],&si214[0],&si215[0],
      &si216[0],&si217[0],&si218[0],&si219[0],&si220[0],&si221[0],
      &si222[0],&si223[0],&si224[0],&si225[0],&si226[0],&si227[0],
      &si228[0],&si229[0],&si230[0],&si231[0],&si232[0],&si233[0],
      &si234[0],&si235[0],&si236[0],&si237[0],&si238[0],&si239[0],
      &si240[0],&si241[0],&si242[0],&si243[0],&si244[0],&si245[0],
      &si246[0],&si247[0],&si248[0],&si249[0],&si250[0],&si251[0],
      &si252[0],&si253[0],&si254[0],&si255[0],&si256[0],&si257[0],
      &si258[0],&si259[0],&si260[0],&si261[0],&si262[0],&si263[0],
      &si264[0],&si265[0],&si266[0],&si267[0],&si268[0],&si269[0],
      &si270[0],&si271[0],&si272[0],&si273[0],&si274[0],&si275[0],
      &si276[0],&si277[0],&si278[0],&si279[0],&si280[0],&si281[0],
      &si282[0],&si283[0],&si284[0],&si285[0],&si286[0],&si287[0],
      &si288[0],&si289[0],&si290[0],&si291[0],&si292[0],&si293[0],
      &si294[0],&si295[0],&si296[0],&si297[0],&si298[0],&si299[0],
      &si300[0],&si301[0],&si302[0],&si303[0],&si304[0],&si305[0],
      &si306[0],&si307[0],&si308[0],&si309[0],&si310[0],&si311[0],
      &si312[0],&si313[0],&si314[0],&si315[0],&si316[0],&si317[0],
      &si318[0],&si319[0],&si320[0],&si321[0],&si322[0],&si323[0],
      &si324[0],&si325[0],&si326[0],&si327[0],&si328[0],&si329[0],
      &si330[0],&si331[0],&si332[0],&si333[0],&si334[0],&si335[0],
      &si336[0],&si337[0],&si338[0],&si339[0],&si340[0],&si341[0],
      &si342[0],&si343[0],&si344[0],&si345[0],&si346[0],&si347[0],
      &si348[0],&si349[0],&si350[0],&si351[0],&si352[0],&si353[0],
      &si354[0],&si355[0],&si356[0],&si357[0],&si358[0],&si359[0],
      &si360[0],&si361[0],&si362[0],&si363[0],&si364[0],&si365[0],
      &si366[0],&si367[0],&si368[0],&si369[0],&si370[0],&si371[0],
      &si372[0],&si373[0],&si374[0],&si375[0],&si376[0],&si377[0],
      &si378[0],&si379[0],&si380[0],&si381[0],&si382[0],&si383[0],
      &si384[0],&si385[0],&si386[0],&si387[0],&si388[0],&si389[0],
      &si390[0],&si391[0],&si392[0],&si393[0],&si394[0],&si395[0],
      &si396[0],&si397[0],&si398[0],&si399[0],&si400[0],&si401[0],
      &si402[0],&si403[0],&si404[0],&si405[0],&si406[0],&si407[0],
      &si408[0],&si409[0],&si410[0],&si411[0],&si412[0],&si413[0],
      &si414[0],&si415[0],&si416[0],&si417[0],&si418[0],&si419[0],
      &si420[0],&si421[0],&si422[0],&si423[0],&si424[0],&si425[0],
      &si426[0],&si427[0],&si428[0],&si429[0],&si430[0],&si431[0],
      &si432[0],&si433[0],&si434[0],&si435[0],&si436[0],&si437[0],
      &si438[0],&si439[0],&si440[0],&si441[0],&si442[0],&si443[0],
      &si444[0],&si445[0],&si446[0],&si447[0],&si448[0],&si449[0],
      &si450[0],&si451[0],&si452[0],&si453[0],&si454[0],&si455[0],
      &si456[0],&si457[0],&si458[0],&si459[0],&si460[0],&si461[0],
      &si462[0],&si463[0],&si464[0],&si465[0],&si466[0],&si467[0],
      &si468[0],&si469[0],&si470[0],&si471[0],&si472[0],&si473[0],
      &si474[0],&si475[0],&si476[0],&si477[0],&si478[0],&si479[0],
      &si480[0],&si481[0],&si482[0],&si483[0],&si484[0],&si485[0],
      &si486[0],&si487[0],&si488[0],&si489[0],&si490[0],&si491[0],
      &si492[0],&si493[0],&si494[0],&si495[0],&si496[0],&si497[0],
      &si498[0],&si499[0],&si500[0],&si501[0],&si502[0],&si503[0],
      &si504[0],&si505[0],&si506[0],&si507[0],&si508[0],&si509[0],
      &si510[0],&si511[0],&si512[0],&si513[0],&si514[0],&si515[0],
      &si516[0],&si517[0],&si518[0],&si519[0],&si520[0],&si521[0],
      &si522[0],&si523[0],&si524[0],&si525[0],&si526[0],&si527[0],
      &si528[0],&si529[0],&si530[0],&si531[0],&si532[0],&si533[0],
      &si534[0],&si535[0],&si536[0],&si537[0],&si538[0],&si539[0],
      &si540[0],&si541[0],&si542[0],&si543[0],&si544[0],&si545[0],
      &si546[0],&si547[0],&si548[0],&si549[0],&si550[0],&si551[0],
      &si552[0],&si553[0],&si554[0],&si555[0],&si556[0],&si557[0],
      &si558[0],&si559[0],&si560[0],&si561[0],&si562[0],&si563[0],
      &si564[0],&si565[0],&si566[0],&si567[0],&si568[0],&si569[0],
      &si570[0],&si571[0],&si572[0],&si573[0],&si574[0],&si575[0],
      &si576[0],&si577[0],&si578[0],&si579[0],&si580[0],&si581[0],
      &si582[0],&si583[0],&si584[0],&si585[0],&si586[0],&si587[0],
      &si588[0],&si589[0],&si590[0],&si591[0],&si592[0],&si593[0],
      &si594[0],&si595[0],&si596[0],&si597[0],&si598[0],&si599[0],
      &si600[0],&si601[0],&si602[0],&si603[0],&si604[0],&si605[0],
      &si606[0],&si607[0],&si608[0],&si609[0],&si610[0],&si611[0],
      &si612[0],&si613[0],&si614[0],&si615[0],&si616[0],&si617[0],
      &si618[0],&si619[0],&si620[0],&si621[0],&si622[0],&si623[0],
      &si624[0],&si625[0],&si626[0],&si627[0],&si628[0],&si629[0],
      &si630[0],&si631[0],&si632[0],&si633[0],&si634[0],&si635[0],
      &si636[0],&si637[0],&si638[0],&si639[0],&si640[0],&si641[0],
      &si642[0],&si643[0],&si644[0],&si645[0],&si646[0],&si647[0],
      &si648[0],&si649[0],&si650[0],&si651[0],&si652[0],&si653[0],
      &si654[0],&si655[0],&si656[0],&si657[0],&si658[0],&si659[0],
      &si660[0],&si661[0],&si662[0],&si663[0],&si664[0],&si665[0],
      &si666[0],&si667[0],&si668[0],&si669[0],&si670[0],&si671[0],
      &si672[0],&si673[0],&si674[0],&si675[0],&si676[0],&si677[0],
      &si678[0],&si679[0],&si680[0],&si681[0],&si682[0],&si683[0],
      &si684[0],&si685[0],&si686[0],&si687[0],&si688[0],&si689[0],
      &si690[0],&si691[0],&si692[0],&si693[0],&si694[0],&si695[0],
      &si696[0],&si697[0],&si698[0],&si699[0],&si700[0],&si701[0],
      &si702[0],&si703[0],&si704[0],&si705[0],&si706[0],&si707[0],
      &si708[0],&si709[0],&si710[0],&si711[0],&si712[0],&si713[0],
      &si714[0],&si715[0],&si716[0],&si717[0],&si718[0],&si719[0],
      &si720[0],&si721[0],&si722[0],&si723[0],&si724[0],&si725[0],
      &si726[0],&si727[0],&si728[0],&si729[0],&si730[0],&si731[0],
      &si732[0],&si733[0],&si734[0],&si735[0],&si736[0],&si737[0],
      &si738[0],&si739[0],&si740[0],&si741[0],&si742[0],&si743[0],
      &si744[0],&si745[0],&si746[0],&si747[0],&si748[0],&si749[0],
      &si750[0],&si751[0],&si752[0],&si753[0],&si754[0],&si755[0],
      &si756[0],&si757[0],&si758[0],&si759[0],&si760[0],&si761[0],
      &si762[0],&si763[0],&si764[0],&si765[0],&si766[0],&si767[0],
      &si768[0],&si769[0],&si770[0],&si771[0],&si772[0],&si773[0],
      &si774[0],&si775[0],&si776[0],&si777[0],&si778[0],&si779[0],
      &si780[0],&si781[0],&si782[0],&si783[0],&si784[0],&si785[0],
      &si786[0],&si787[0],&si788[0],&si789[0],&si790[0],&si791[0],
      &si792[0],&si793[0],&si794[0],&si795[0],&si796[0],&si797[0],
      &si798[0],&si799[0],&si800[0],&si801[0],&si802[0],&si803[0],
      &si804[0],&si805[0],&si806[0],&si807[0],&si808[0],&si809[0],
      &si810[0],&si811[0],&si812[0],&si813[0],&si814[0],&si815[0],
      &si816[0],&si817[0],&si818[0],&si819[0],&si820[0],&si821[0],
      &si822[0],&si823[0],&si824[0],&si825[0],&si826[0],&si827[0],
      &si828[0],&si829[0],&si830[0],&si831[0],&si832[0],&si833[0],
      &si834[0],&si835[0],&si836[0],&si837[0],&si838[0],&si839[0],
      &si840[0],&si841[0],&si842[0],&si843[0],&si844[0],&si845[0],
      &si846[0],&si847[0],&si848[0],&si849[0],&si850[0],&si851[0],
      &si852[0],&si853[0],&si854[0],&si855[0],&si856[0],&si857[0],
      &si858[0],&si859[0],&si860[0],&si861[0],&si862[0],&si863[0],
      &si864[0],&si865[0],&si866[0],&si867[0],&si868[0],&si869[0],
      &si870[0],&si871[0],&si872[0],&si873[0],&si874[0],&si875[0],
      &si876[0],&si877[0],&si878[0],&si879[0]
    };

    
    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        int n = sizeof(si)/sizeof(SetInstr*);
        for (int i=0; i<n; i++) {
          std::string s = Test::str(i);
          if (i < 10) {
            s = "00" + s;
          } else if (i < 100) {
            s = "0" + s;
          }
          (void) new SetExprConst(si[i],s,Gecode::SRT_EQ,0);
          (void) new SetExprConst(si[i],s,Gecode::SRT_EQ,1);
          (void) new SetExprConst(si[i],s,Gecode::SRT_NQ,0);
          (void) new SetExprConst(si[i],s,Gecode::SRT_NQ,1);
          (void) new SetExprConst(si[i],s,Gecode::SRT_SUB,0);
          (void) new SetExprConst(si[i],s,Gecode::SRT_SUB,1);
          (void) new SetExprConst(si[i],s,Gecode::SRT_SUP,0);
          (void) new SetExprConst(si[i],s,Gecode::SRT_SUP,1);
          (void) new SetExprConst(si[i],s,Gecode::SRT_DISJ,0);
          (void) new SetExprConst(si[i],s,Gecode::SRT_DISJ,1);
          
          if ( (i % 31) == 0) {
          
            for (int j=0; j<n; j++) {
              if ( (j % 37) == 0) {
                std::string ss = Test::str(j);
                if (j < 10) {
                  ss = "00" + ss;
                } else if (j < 100) {
                  ss = "0" + ss;
                }
                ss=s+"::"+ss;
                (void) new SetExprExpr(si[i],si[j],ss,Gecode::SRT_EQ);
                (void) new SetExprExpr(si[i],si[j],ss,Gecode::SRT_NQ);
                (void) new SetExprExpr(si[i],si[j],ss,Gecode::SRT_SUB);
                (void) new SetExprExpr(si[i],si[j],ss,Gecode::SRT_SUP);
                (void) new SetExprExpr(si[i],si[j],ss,Gecode::SRT_DISJ);
              }
            }
          }
        }
      }
    };

    Create c;
    //@}
   }

}}

// STATISTICS: test-minimodel
