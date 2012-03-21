/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *     Mikael Lagerkvist, 2009
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

#include <gecode/flatzinc/registry.hh>
#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#include <gecode/flatzinc.hh>

namespace Gecode { namespace FlatZinc {

  Registry& registry(void) {
    static Registry r;
    return r;
  }

  void
  Registry::post(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
    std::map<std::string,poster>::iterator i = r.find(ce.id);
    if (i == r.end()) {
      throw FlatZinc::Error("Registry",
        std::string("Constraint ")+ce.id+" not found");
    }
    i->second(s, ce, ann);
  }

  void
  Registry::add(const std::string& id, poster p) {
    r[id] = p;
  }

  namespace {
  
    IntConLevel ann2icl(AST::Node* ann) {
      if (ann) {
        if (ann->hasAtom("val"))
          return ICL_VAL;
        if (ann->hasAtom("domain"))
          return ICL_DOM;
        if (ann->hasAtom("bounds") ||
            ann->hasAtom("boundsR") ||
            ann->hasAtom("boundsD") ||
            ann->hasAtom("boundsZ"))
          return ICL_BND;
      }
      return ICL_DEF;
    }
  
    inline IntRelType
    swap(IntRelType irt) {
      switch (irt) {
      case IRT_LQ: return IRT_GQ;
      case IRT_LE: return IRT_GR;
      case IRT_GQ: return IRT_LQ;
      case IRT_GR: return IRT_LE;
      default:     return irt;
      }
    }

    inline IntRelType
    neg(IntRelType irt) {
      switch (irt) {
      case IRT_EQ: return IRT_NQ;
      case IRT_NQ: return IRT_EQ;
      case IRT_LQ: return IRT_GR;
      case IRT_LE: return IRT_GQ;
      case IRT_GQ: return IRT_LE;
      case IRT_GR:
      default:
        assert(irt == IRT_GR);
      }
      return IRT_LQ;
    }

    inline IntArgs arg2intargs(AST::Node* arg, int offset = 0) {
      AST::Array* a = arg->getArray();
      IntArgs ia(a->a.size()+offset);
      for (int i=offset; i--;)
        ia[i] = 0;
      for (int i=a->a.size(); i--;)
        ia[i+offset] = a->a[i]->getInt();
      return ia;
    }

    inline IntArgs arg2boolargs(AST::Node* arg, int offset = 0) {
      AST::Array* a = arg->getArray();
      IntArgs ia(a->a.size()+offset);
      for (int i=offset; i--;)
        ia[i] = 0;
      for (int i=a->a.size(); i--;)
        ia[i+offset] = a->a[i]->getBool();
      return ia;
    }

    inline IntSet arg2intset(FlatZincSpace& s, AST::Node* n) {
      AST::SetLit* sl = n->getSet();
      IntSet d;
      if (sl->interval) {
        d = IntSet(sl->min, sl->max);
      } else {
        Region re(s);
        int* is = re.alloc<int>(static_cast<unsigned long int>(sl->s.size()));
        for (int i=sl->s.size(); i--; )
          is[i] = sl->s[i];
        d = IntSet(is, sl->s.size());
      }
      return d;
    }

    inline IntSetArgs arg2intsetargs(FlatZincSpace& s,
                                     AST::Node* arg, int offset = 0) {
      AST::Array* a = arg->getArray();
      if (a->a.size() == 0) {
        IntSetArgs emptyIa(0);
        return emptyIa;
      }
      IntSetArgs ia(a->a.size()+offset);      
      for (int i=offset; i--;)
        ia[i] = IntSet::empty;
      for (int i=a->a.size(); i--;) {
        ia[i+offset] = arg2intset(s, a->a[i]);
      }
      return ia;
    }
  
    inline IntVarArgs arg2intvarargs(FlatZincSpace& s, AST::Node* arg,
                                     int offset = 0) {
      AST::Array* a = arg->getArray();
      if (a->a.size() == 0) {
        IntVarArgs emptyIa(0);
        return emptyIa;
      }
      IntVarArgs ia(a->a.size()+offset);
      for (int i=offset; i--;)
        ia[i] = IntVar(s, 0, 0);
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isIntVar()) {
          ia[i+offset] = s.iv[a->a[i]->getIntVar()];        
        } else {
          int value = a->a[i]->getInt();
          IntVar iv(s, value, value);
          ia[i+offset] = iv;        
        }
      }
      return ia;
    }

    inline BoolVarArgs arg2boolvarargs(FlatZincSpace& s, AST::Node* arg,
                                       int offset = 0, int siv=-1) {
      AST::Array* a = arg->getArray();
      if (a->a.size() == 0) {
        BoolVarArgs emptyIa(0);
        return emptyIa;
      }
      BoolVarArgs ia(a->a.size()+offset-(siv==-1?0:1));
      for (int i=offset; i--;)
        ia[i] = BoolVar(s, 0, 0);
      for (int i=0; i<static_cast<int>(a->a.size()); i++) {
        if (i==siv)
          continue;
        if (a->a[i]->isBool()) {
          bool value = a->a[i]->getBool();
          BoolVar iv(s, value, value);
          ia[offset++] = iv;
        } else if (a->a[i]->isIntVar() &&
                   s.aliasBool2Int(a->a[i]->getIntVar()) != -1) {
          ia[offset++] = s.bv[s.aliasBool2Int(a->a[i]->getIntVar())];
        } else {
          ia[offset++] = s.bv[a->a[i]->getBoolVar()];
        }
      }
      return ia;
    }

#ifdef GECODE_HAS_SET_VARS
    SetVar getSetVar(FlatZincSpace& s, AST::Node* n) {
      SetVar x0;
      if (!n->isSetVar()) {
        IntSet d = arg2intset(s,n);
        x0 = SetVar(s, d, d);        
      } else {
        x0 = s.sv[n->getSetVar()];
      }
      return x0;
    }

    inline SetVarArgs arg2setvarargs(FlatZincSpace& s, AST::Node* arg,
                                     int offset = 0, int doffset = 0,
                                     const IntSet& od=IntSet::empty) {
      AST::Array* a = arg->getArray();
      SetVarArgs ia(a->a.size()+offset);
      for (int i=offset; i--;) {
        IntSet d = i<doffset ? od : IntSet::empty;
        ia[i] = SetVar(s, d, d);
      }
      for (int i=a->a.size(); i--;) {
        ia[i+offset] = getSetVar(s, a->a[i]);
      }
      return ia;
    }
#endif

    BoolVar getBoolVar(FlatZincSpace& s, AST::Node* n) {
      BoolVar x0;
      if (n->isBool()) {
        x0 = BoolVar(s, n->getBool(), n->getBool());
      }
      else {
        x0 = s.bv[n->getBoolVar()];
      }
      return x0;
    }

    IntVar getIntVar(FlatZincSpace& s, AST::Node* n) {
      IntVar x0;
      if (n->isIntVar()) {
        x0 = s.iv[n->getIntVar()];
      } else {
        x0 = IntVar(s, n->getInt(), n->getInt());            
      }
      return x0;
    }

    bool isBoolArray(FlatZincSpace& s, AST::Node* b, int& singleInt) {
      AST::Array* a = b->getArray();
      singleInt = -1;
      if (a->a.size() == 0)
        return true;
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isBoolVar() || a->a[i]->isBool()) {
        } else if (a->a[i]->isIntVar()) {
          if (s.aliasBool2Int(a->a[i]->getIntVar()) == -1) {
            if (singleInt != -1) {
              return false;
            }
            singleInt = i;
          }
        } else {
          return false;
        }
      }
      return singleInt==-1 || a->a.size() > 1;
    }

    void p_distinct(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = arg2intvarargs(s, ce[0]);
      IntConLevel icl = ann2icl(ann);
      distinct(s, va, icl == ICL_DEF ? ICL_BND : icl);
    }
    void p_distinctOffset(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = arg2intvarargs(s, ce[1]);
      AST::Array* offs = ce.args->a[0]->getArray();
      IntArgs oa(offs->a.size());
      for (int i=offs->a.size(); i--; ) {
        oa[i] = offs->a[i]->getInt();    
      }
      IntConLevel icl = ann2icl(ann);
      distinct(s, oa, va, icl == ICL_DEF ? ICL_BND : icl);
    }

    void p_all_equal(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = arg2intvarargs(s, ce[0]);
      rel(s, va, IRT_EQ, ann2icl(ann));
    }

    void p_int_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce, 
                   AST::Node* ann) {
      if (ce[0]->isIntVar()) {
        if (ce[1]->isIntVar()) {
          rel(s, getIntVar(s, ce[0]), irt, getIntVar(s, ce[1]), 
              ann2icl(ann));
        } else {
          rel(s, getIntVar(s, ce[0]), irt, ce[1]->getInt(), ann2icl(ann));
        }
      } else {
        rel(s, getIntVar(s, ce[1]), swap(irt), ce[0]->getInt(), 
            ann2icl(ann));
      }
    }
    void p_int_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_EQ, ce, ann);
    }
    void p_int_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_NQ, ce, ann);
    }
    void p_int_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_GQ, ce, ann);
    }
    void p_int_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_GR, ce, ann);
    }
    void p_int_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_LQ, ce, ann);
    }
    void p_int_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_LE, ce, ann);
    }
    void p_int_CMP_reif(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                        AST::Node* ann) {
      if (ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_int_CMP(s, irt, ce, ann);
        } else {
          p_int_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      if (ce[0]->isIntVar()) {
        if (ce[1]->isIntVar()) {
          rel(s, getIntVar(s, ce[0]), irt, getIntVar(s, ce[1]),
                 getBoolVar(s, ce[2]), ann2icl(ann));
        } else {
          rel(s, getIntVar(s, ce[0]), irt, ce[1]->getInt(),
                 getBoolVar(s, ce[2]), ann2icl(ann));
        }
      } else {
        rel(s, getIntVar(s, ce[1]), swap(irt), ce[0]->getInt(),
               getBoolVar(s, ce[2]), ann2icl(ann));
      }
    }

    /* Comparisons */
    void p_int_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_EQ, ce, ann);
    }
    void p_int_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_NQ, ce, ann);
    }
    void p_int_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GQ, ce, ann);
    }
    void p_int_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GR, ce, ann);
    }
    void p_int_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LQ, ce, ann);
    }
    void p_int_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LE, ce, ann);
    }

    /* linear (in-)equations */
    void p_int_lin_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                       AST::Node* ann) {
      IntArgs ia = arg2intargs(ce[0]);
      int singleIntVar;
      if (isBoolArray(s,ce[1],singleIntVar)) {
        if (singleIntVar != -1) {
          if (std::abs(ia[singleIntVar]) == 1 && ce[2]->getInt() == 0) {
            IntVar siv = getIntVar(s, ce[1]->getArray()->a[singleIntVar]);
            BoolVarArgs iv = arg2boolvarargs(s, ce[1], 0, singleIntVar);
            IntArgs ia_tmp(ia.size()-1);
            int count = 0;
            for (int i=0; i<ia.size(); i++) {
              if (i != singleIntVar)
                ia_tmp[count++] = ia[singleIntVar] == -1 ? ia[i] : -ia[i];
            }
            IntRelType t = (ia[singleIntVar] == -1 ? irt : swap(irt));
            linear(s, ia_tmp, iv, t, siv, ann2icl(ann));
          } else {
            IntVarArgs iv = arg2intvarargs(s, ce[1]);
            linear(s, ia, iv, irt, ce[2]->getInt(), ann2icl(ann));
          }
        } else {
          BoolVarArgs iv = arg2boolvarargs(s, ce[1]);
          linear(s, ia, iv, irt, ce[2]->getInt(), ann2icl(ann));
        }
      } else {
        IntVarArgs iv = arg2intvarargs(s, ce[1]);
        linear(s, ia, iv, irt, ce[2]->getInt(), ann2icl(ann));
      }
    }
    void p_int_lin_CMP_reif(FlatZincSpace& s, IntRelType irt,
                            const ConExpr& ce, AST::Node* ann) {
      if (ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_int_lin_CMP(s, irt, ce, ann);
        } else {
          p_int_lin_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      IntArgs ia = arg2intargs(ce[0]);
      int singleIntVar;
      if (isBoolArray(s,ce[1],singleIntVar)) {
        if (singleIntVar != -1) {
          if (std::abs(ia[singleIntVar]) == 1 && ce[2]->getInt() == 0) {
            IntVar siv = getIntVar(s, ce[1]->getArray()->a[singleIntVar]);
            BoolVarArgs iv = arg2boolvarargs(s, ce[1], 0, singleIntVar);
            IntArgs ia_tmp(ia.size()-1);
            int count = 0;
            for (int i=0; i<ia.size(); i++) {
              if (i != singleIntVar)
                ia_tmp[count++] = ia[singleIntVar] == -1 ? ia[i] : -ia[i];
            }
            IntRelType t = (ia[singleIntVar] == -1 ? irt : swap(irt));
            linear(s, ia_tmp, iv, t, siv, getBoolVar(s, ce[3]), 
                   ann2icl(ann));
          } else {
            IntVarArgs iv = arg2intvarargs(s, ce[1]);
            linear(s, ia, iv, irt, ce[2]->getInt(),
                   getBoolVar(s, ce[3]), ann2icl(ann));
          }
        } else {
          BoolVarArgs iv = arg2boolvarargs(s, ce[1]);
          linear(s, ia, iv, irt, ce[2]->getInt(),
                 getBoolVar(s, ce[3]), ann2icl(ann));
        }
      } else {
        IntVarArgs iv = arg2intvarargs(s, ce[1]);
        linear(s, ia, iv, irt, ce[2]->getInt(), getBoolVar(s, ce[3]), 
               ann2icl(ann));
      }
    }
    void p_int_lin_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_EQ, ce, ann);
    }
    void p_int_lin_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_EQ, ce, ann);    
    }
    void p_int_lin_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_NQ, ce, ann);
    }
    void p_int_lin_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_NQ, ce, ann);    
    }
    void p_int_lin_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_LQ, ce, ann);
    }
    void p_int_lin_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LQ, ce, ann);    
    }
    void p_int_lin_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_LE, ce, ann);
    }
    void p_int_lin_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LE, ce, ann);    
    }
    void p_int_lin_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_GQ, ce, ann);
    }
    void p_int_lin_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GQ, ce, ann);    
    }
    void p_int_lin_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_GR, ce, ann);
    }
    void p_int_lin_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GR, ce, ann);    
    }

    void p_bool_lin_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                        AST::Node* ann) {
      IntArgs ia = arg2intargs(ce[0]);
      BoolVarArgs iv = arg2boolvarargs(s, ce[1]);
      if (ce[2]->isIntVar())
        linear(s, ia, iv, irt, s.iv[ce[2]->getIntVar()], ann2icl(ann));
      else
        linear(s, ia, iv, irt, ce[2]->getInt(), ann2icl(ann));
    }
    void p_bool_lin_CMP_reif(FlatZincSpace& s, IntRelType irt,
                            const ConExpr& ce, AST::Node* ann) {
      if (ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_bool_lin_CMP(s, irt, ce, ann);
        } else {
          p_bool_lin_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      IntArgs ia = arg2intargs(ce[0]);
      BoolVarArgs iv = arg2boolvarargs(s, ce[1]);
      if (ce[2]->isIntVar())
        linear(s, ia, iv, irt, s.iv[ce[2]->getIntVar()], getBoolVar(s, ce[3]), 
               ann2icl(ann));
      else
        linear(s, ia, iv, irt, ce[2]->getInt(), getBoolVar(s, ce[3]), 
               ann2icl(ann));
    }
    void p_bool_lin_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_EQ, ce, ann);
    }
    void p_bool_lin_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_EQ, ce, ann);
    }
    void p_bool_lin_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_NQ, ce, ann);
    }
    void p_bool_lin_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_NQ, ce, ann);
    }
    void p_bool_lin_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_LQ, ce, ann);
    }
    void p_bool_lin_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_LQ, ce, ann);
    }
    void p_bool_lin_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP(s, IRT_LE, ce, ann);
    }
    void p_bool_lin_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_LE, ce, ann);
    }
    void p_bool_lin_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_GQ, ce, ann);
    }
    void p_bool_lin_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_GQ, ce, ann);
    }
    void p_bool_lin_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_GR, ce, ann);
    }
    void p_bool_lin_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) 
    {
      p_bool_lin_CMP_reif(s, IRT_GR, ce, ann);
    }

    /* arithmetic constraints */
  
    void p_int_plus(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      if (!ce[0]->isIntVar()) {
        rel(s, ce[0]->getInt() + getIntVar(s, ce[1])
                == getIntVar(s,ce[2]), ann2icl(ann));
      } else if (!ce[1]->isIntVar()) {
        rel(s, getIntVar(s,ce[0]) + ce[1]->getInt()
                == getIntVar(s,ce[2]), ann2icl(ann));
      } else if (!ce[2]->isIntVar()) {
        rel(s, getIntVar(s,ce[0]) + getIntVar(s,ce[1]) 
                == ce[2]->getInt(), ann2icl(ann));
      } else {
        rel(s, getIntVar(s,ce[0]) + getIntVar(s,ce[1]) 
                == getIntVar(s,ce[2]), ann2icl(ann));
      }
    }

    void p_int_minus(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      if (!ce[0]->isIntVar()) {
        rel(s, ce[0]->getInt() - getIntVar(s, ce[1])
                == getIntVar(s,ce[2]), ann2icl(ann));
      } else if (!ce[1]->isIntVar()) {
        rel(s, getIntVar(s,ce[0]) - ce[1]->getInt()
                == getIntVar(s,ce[2]), ann2icl(ann));
      } else if (!ce[2]->isIntVar()) {
        rel(s, getIntVar(s,ce[0]) - getIntVar(s,ce[1]) 
                == ce[2]->getInt(), ann2icl(ann));
      } else {
        rel(s, getIntVar(s,ce[0]) - getIntVar(s,ce[1]) 
                == getIntVar(s,ce[2]), ann2icl(ann));
      }
    }

    void p_int_times(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      IntVar x2 = getIntVar(s, ce[2]);
      mult(s, x0, x1, x2, ann2icl(ann));    
    }
    void p_int_div(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      IntVar x2 = getIntVar(s, ce[2]);
      div(s,x0,x1,x2, ann2icl(ann));
    }
    void p_int_mod(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      IntVar x2 = getIntVar(s, ce[2]);
      mod(s,x0,x1,x2, ann2icl(ann));
    }

    void p_int_min(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      IntVar x2 = getIntVar(s, ce[2]);
      min(s, x0, x1, x2, ann2icl(ann));
    }
    void p_int_max(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      IntVar x2 = getIntVar(s, ce[2]);
      max(s, x0, x1, x2, ann2icl(ann));
    }
    void p_int_negate(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      rel(s, x0 == -x1, ann2icl(ann));
    }

    /* Boolean constraints */
    void p_bool_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce, 
                   AST::Node* ann) {
      rel(s, getBoolVar(s, ce[0]), irt, getBoolVar(s, ce[1]), 
          ann2icl(ann));
    }
    void p_bool_CMP_reif(FlatZincSpace& s, IntRelType irt, const ConExpr& ce, 
                   AST::Node* ann) {
      rel(s, getBoolVar(s, ce[0]), irt, getBoolVar(s, ce[1]),
          getBoolVar(s, ce[2]), ann2icl(ann));
    }
    void p_bool_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_EQ, ce, ann);
    }
    void p_bool_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_EQ, ce, ann);
    }
    void p_bool_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_NQ, ce, ann);
    }
    void p_bool_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_NQ, ce, ann);
    }
    void p_bool_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_GQ, ce, ann);
    }
    void p_bool_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GQ, ce, ann);
    }
    void p_bool_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_LQ, ce, ann);
    }
    void p_bool_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LQ, ce, ann);
    }
    void p_bool_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_GR, ce, ann);
    }
    void p_bool_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GR, ce, ann);
    }
    void p_bool_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_LE, ce, ann);
    }
    void p_bool_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LE, ce, ann);
    }

#define BOOL_OP(op) \
    BoolVar b0 = getBoolVar(s, ce[0]); \
    BoolVar b1 = getBoolVar(s, ce[1]); \
    if (ce[2]->isBool()) { \
      rel(s, b0, op, b1, ce[2]->getBool(), ann2icl(ann)); \
    } else { \
      rel(s, b0, op, b1, s.bv[ce[2]->getBoolVar()], ann2icl(ann)); \
    }

#define BOOL_ARRAY_OP(op) \
    BoolVarArgs bv = arg2boolvarargs(s, ce[0]); \
    if (ce.size()==1) { \
      rel(s, op, bv, 1, ann2icl(ann)); \
    } else if (ce[1]->isBool()) { \
      rel(s, op, bv, ce[1]->getBool(), ann2icl(ann)); \
    } else { \
      rel(s, op, bv, s.bv[ce[1]->getBoolVar()], ann2icl(ann)); \
    }

    void p_bool_or(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_OR);
    }
    void p_bool_and(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_AND);
    }
    void p_array_bool_and(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_AND);
    }
    void p_array_bool_or(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_OR);
    }
    void p_array_bool_xor(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_XOR);
    }
    void p_array_bool_clause(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      BoolVarArgs bvp = arg2boolvarargs(s, ce[0]);
      BoolVarArgs bvn = arg2boolvarargs(s, ce[1]);
      clause(s, BOT_OR, bvp, bvn, 1, ann2icl(ann));
    }
    void p_array_bool_clause_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      BoolVarArgs bvp = arg2boolvarargs(s, ce[0]);
      BoolVarArgs bvn = arg2boolvarargs(s, ce[1]);
      BoolVar b0 = getBoolVar(s, ce[2]);
      clause(s, BOT_OR, bvp, bvn, b0, ann2icl(ann));
    }
    void p_bool_xor(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_XOR);
    }
    void p_bool_l_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar b0 = getBoolVar(s, ce[0]);
      BoolVar b1 = getBoolVar(s, ce[1]);
      if (ce[2]->isBool()) {
        rel(s, b1, BOT_IMP, b0, ce[2]->getBool(), ann2icl(ann));
      } else {
        rel(s, b1, BOT_IMP, b0, s.bv[ce[2]->getBoolVar()], ann2icl(ann));
      }
    }
    void p_bool_r_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_IMP);
    }
    void p_bool_not(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar x0 = getBoolVar(s, ce[0]);
      BoolVar x1 = getBoolVar(s, ce[1]);
      rel(s, x0, BOT_XOR, x1, 1, ann2icl(ann));
    }
  
    /* element constraints */
    void p_array_int_element(FlatZincSpace& s, const ConExpr& ce, 
                                 AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isInt()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = getIntVar(s, ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntArgs ia = arg2intargs(ce[1], 1);
        element(s, ia, selector, getIntVar(s, ce[2]), ann2icl(ann));
      } else {
        IntVarArgs iv = arg2intvarargs(s, ce[1], 1);
        element(s, iv, selector, getIntVar(s, ce[2]), ann2icl(ann));
      }
    }
    void p_array_bool_element(FlatZincSpace& s, const ConExpr& ce, 
                                  AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isBool()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = getIntVar(s, ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntArgs ia = arg2boolargs(ce[1], 1);
        element(s, ia, selector, getBoolVar(s, ce[2]), ann2icl(ann));
      } else {
        BoolVarArgs iv = arg2boolvarargs(s, ce[1], 1);
        element(s, iv, selector, getBoolVar(s, ce[2]), ann2icl(ann));
      }
    }
  
    /* coercion constraints */
    void p_bool2int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar x0 = getBoolVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      if (ce[0]->isBoolVar() && ce[1]->isIntVar()) {
        s.aliasBool2Int(ce[1]->getIntVar(), ce[0]->getBoolVar());
      }
      channel(s, x0, x1, ann2icl(ann));
    }

    void p_int_in(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      IntSet d = arg2intset(s,ce[1]);
      if (ce[0]->isBoolVar()) {
        IntSetRanges dr(d);
        Iter::Ranges::Singleton sr(0,1);
        Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
        IntSet d01(i);
        if (d01.size() == 0) {
          s.fail();
        } else {
          rel(s, getBoolVar(s, ce[0]), IRT_GQ, d01.min());
          rel(s, getBoolVar(s, ce[0]), IRT_LQ, d01.max());
        }
      } else {
        dom(s, getIntVar(s, ce[0]), d);
      }
    }
    void p_int_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      IntSet d = arg2intset(s,ce[1]);
      if (ce[0]->isBoolVar()) {
        IntSetRanges dr(d);
        Iter::Ranges::Singleton sr(0,1);
        Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
        IntSet d01(i);
        if (d01.size() == 0) {
          rel(s, getBoolVar(s, ce[2]) == 0);
        } else if (d01.max() == 0) {
          rel(s, getBoolVar(s, ce[2]) == !getBoolVar(s, ce[0]));
        } else if (d01.min() == 1) {
          rel(s, getBoolVar(s, ce[2]) == getBoolVar(s, ce[0]));
        } else {
          rel(s, getBoolVar(s, ce[2]) == 1);
        }
      } else {
        std::cerr << "in_in_reif("<<getIntVar(s, ce[0])<<","<<d<<","<<getBoolVar(s, ce[2])<<")\n";
        dom(s, getIntVar(s, ce[0]), d, getBoolVar(s, ce[2]));
      }
    }

    /* constraints from the standard library */
  
    void p_abs(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = getIntVar(s, ce[0]);
      IntVar x1 = getIntVar(s, ce[1]);
      abs(s, x0, x1, ann2icl(ann));
    }
  
    void p_array_int_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv0 = arg2intvarargs(s, ce[0]);
      IntVarArgs iv1 = arg2intvarargs(s, ce[1]);
      rel(s, iv0, IRT_LE, iv1, ann2icl(ann));
    }

    void p_array_int_lq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv0 = arg2intvarargs(s, ce[0]);
      IntVarArgs iv1 = arg2intvarargs(s, ce[1]);
      rel(s, iv0, IRT_LQ, iv1, ann2icl(ann));
    }
  
    void p_count(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = arg2intvarargs(s, ce[0]);
      if (!ce[1]->isIntVar()) {
        if (!ce[2]->isIntVar()) {
          count(s, iv, ce[1]->getInt(), IRT_EQ, ce[2]->getInt(), 
                ann2icl(ann));
        } else {
          count(s, iv, ce[1]->getInt(), IRT_EQ, getIntVar(s, ce[2]), 
                ann2icl(ann));
        }
      } else if (!ce[2]->isIntVar()) {
        count(s, iv, getIntVar(s, ce[1]), IRT_EQ, ce[2]->getInt(), 
              ann2icl(ann));
      } else {
        count(s, iv, getIntVar(s, ce[1]), IRT_EQ, getIntVar(s, ce[2]), 
              ann2icl(ann));
      }
    }

    void count_rel(IntRelType irt,
                   FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = arg2intvarargs(s, ce[1]);
      count(s, iv, ce[2]->getInt(), irt, ce[0]->getInt(), ann2icl(ann));
    }

    void p_at_most(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      count_rel(IRT_LQ, s, ce, ann);
    }

    void p_at_least(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      count_rel(IRT_GQ, s, ce, ann);
    }

    void p_bin_packing_load(FlatZincSpace& s, const ConExpr& ce,
                            AST::Node* ann) {
      int minIdx = ce[3]->getInt();
      IntVarArgs load = arg2intvarargs(s, ce[0]);
      IntVarArgs l;
      IntVarArgs bin = arg2intvarargs(s, ce[1]);
      for (int i=bin.size(); i--;)
        rel(s, bin[i] >= minIdx);
      if (minIdx > 0) {
        for (int i=minIdx; i--;)
          l << IntVar(s,0,0);
      } else if (minIdx < 0) {
        IntVarArgs bin2(bin.size());
        for (int i=bin.size(); i--;)
          bin2[i] = expr(s, bin[i]-minIdx, ann2icl(ann));
        bin = bin2;
      }
      l << load;
      IntArgs sizes = arg2intargs(ce[2]);
      binpacking(s, l, bin, sizes, ann2icl(ann));
    }

    void p_global_cardinality(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node* ann) {
      IntVarArgs iv0 = arg2intvarargs(s, ce[0]);
      IntArgs cover = arg2intargs(ce[1]);
      IntVarArgs iv1 = arg2intvarargs(s, ce[2]);

      Region re(s);
      IntSet cover_s(cover);
      IntSetRanges cover_r(cover_s);
      IntVarRanges* iv0_ri = re.alloc<IntVarRanges>(iv0.size());
      for (int i=iv0.size(); i--;)
        iv0_ri[i] = IntVarRanges(iv0[i]);
      Iter::Ranges::NaryUnion iv0_r(re,iv0_ri,iv0.size());
      Iter::Ranges::Diff<Iter::Ranges::NaryUnion,IntSetRanges> 
        extra_r(iv0_r,cover_r);
      Iter::Ranges::ToValues<Iter::Ranges::Diff<
        Iter::Ranges::NaryUnion,IntSetRanges> > extra(extra_r);
      for (; extra(); ++extra) {
        cover << extra.val();
        iv1 << IntVar(s,0,iv0.size());
      }
      count(s, iv0, iv1, cover, ann2icl(ann));
    }

    void p_global_cardinality_closed(FlatZincSpace& s, const ConExpr& ce,
                                     AST::Node* ann) {
      IntVarArgs iv0 = arg2intvarargs(s, ce[0]);
      IntArgs cover = arg2intargs(ce[1]);
      IntVarArgs iv1 = arg2intvarargs(s, ce[2]);
      count(s, iv0, iv1, cover, ann2icl(ann));
    }

    void p_global_cardinality_low_up(FlatZincSpace& s, const ConExpr& ce,
                                     AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs cover = arg2intargs(ce[1]);

      IntArgs lbound = arg2intargs(ce[2]);
      IntArgs ubound = arg2intargs(ce[3]);
      IntSetArgs y(cover.size());
      for (int i=cover.size(); i--;)
        y[i] = IntSet(lbound[i],ubound[i]);

      IntSet cover_s(cover);
      Region re(s);
      IntVarRanges* xrs = re.alloc<IntVarRanges>(x.size());
      for (int i=x.size(); i--;)
        xrs[i].init(x[i]);
      Iter::Ranges::NaryUnion u(re, xrs, x.size());
      Iter::Ranges::ToValues<Iter::Ranges::NaryUnion> uv(u);
      for (; uv(); ++uv) {
        if (!cover_s.in(uv.val())) {
          cover << uv.val();
          y << IntSet(0,x.size());
        }
      }
      
      count(s, x, y, cover, ann2icl(ann));
    }

    void p_global_cardinality_low_up_closed(FlatZincSpace& s,
                                            const ConExpr& ce,
                                            AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs cover = arg2intargs(ce[1]);

      IntArgs lbound = arg2intargs(ce[2]);
      IntArgs ubound = arg2intargs(ce[3]);
      IntSetArgs y(cover.size());
      for (int i=cover.size(); i--;)
        y[i] = IntSet(lbound[i],ubound[i]);

      count(s, x, y, cover, ann2icl(ann));
    }

    void p_minimum(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = arg2intvarargs(s, ce[1]);
      min(s, iv, getIntVar(s, ce[0]), ann2icl(ann));
    }

    void p_maximum(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = arg2intvarargs(s, ce[1]);
      max(s, iv, getIntVar(s, ce[0]), ann2icl(ann));
    }

    void p_regular(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = arg2intvarargs(s, ce[0]);
      int q = ce[1]->getInt();
      int symbols = ce[2]->getInt();
      IntArgs d = arg2intargs(ce[3]);
      int q0 = ce[4]->getInt();

      int noOfTrans = 0;
      for (int i=1; i<=q; i++) {
        for (int j=1; j<=symbols; j++) {
          if (d[(i-1)*symbols+(j-1)] > 0)
            noOfTrans++;
        }
      }
    
      Region re(s);
      DFA::Transition* t = re.alloc<DFA::Transition>(noOfTrans+1);
      noOfTrans = 0;
      for (int i=1; i<=q; i++) {
        for (int j=1; j<=symbols; j++) {
          if (d[(i-1)*symbols+(j-1)] > 0) {
            t[noOfTrans].i_state = i;
            t[noOfTrans].symbol  = j;
            t[noOfTrans].o_state = d[(i-1)*symbols+(j-1)];
            noOfTrans++;
          }
        }
      }
      t[noOfTrans].i_state = -1;
    
      // Final states
      AST::SetLit* sl = ce[5]->getSet();
      int* f;
      if (sl->interval) {
        f = static_cast<int*>(malloc(sizeof(int)*(sl->max-sl->min+2)));
        for (int i=sl->min; i<=sl->max; i++)
          f[i-sl->min] = i;
        f[sl->max-sl->min+1] = -1;
      } else {
        f = static_cast<int*>(malloc(sizeof(int)*(sl->s.size()+1)));
        for (int j=sl->s.size(); j--; )
          f[j] = sl->s[j];
        f[sl->s.size()] = -1;
      }
        
      DFA dfa(q0,t,f);
      free(f);
      extensional(s, iv, dfa, ann2icl(ann));
    }

    void
    p_sort(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntVarArgs y = arg2intvarargs(s, ce[1]);
      IntVarArgs xy(x.size()+y.size());
      for (int i=x.size(); i--;)
        xy[i] = x[i];
      for (int i=y.size(); i--;)
        xy[i+x.size()] = y[i];
      unshare(s, xy);
      for (int i=x.size(); i--;)
        x[i] = xy[i];
      for (int i=y.size(); i--;)
        y[i] = xy[i+x.size()];
      sorted(s, x, y, ann2icl(ann));
    }

    void
    p_inverse_offsets(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      int xoff = ce[1]->getInt();
      IntVarArgs y = arg2intvarargs(s, ce[2]);
      int yoff = ce[3]->getInt();
      channel(s, x, xoff, y, yoff, ann2icl(ann));
    }

    void
    p_increasing_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      rel(s,x,IRT_LQ,ann2icl(ann));
    }

    void
    p_increasing_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      rel(s,x,IRT_LQ,ann2icl(ann));
    }

    void
    p_decreasing_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      rel(s,x,IRT_GQ,ann2icl(ann));
    }

    void
    p_decreasing_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      rel(s,x,IRT_GQ,ann2icl(ann));
    }

    void
    p_table_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs tuples = arg2intargs(ce[1]);
      int noOfVars   = x.size();
      int noOfTuples = tuples.size()/noOfVars;
      TupleSet ts;
      for (int i=0; i<noOfTuples; i++) {
        IntArgs t(noOfVars);
        for (int j=0; j<x.size(); j++) {
          t[j] = tuples[i*noOfVars+j];
        }
        ts.add(t);
      }
      ts.finalize();
      extensional(s,x,ts,EPK_DEF,ann2icl(ann));
    }
    void
    p_table_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      IntArgs tuples = arg2boolargs(ce[1]);
      int noOfVars   = x.size();
      int noOfTuples = tuples.size()/noOfVars;
      TupleSet ts;
      for (int i=0; i<noOfTuples; i++) {
        IntArgs t(noOfVars);
        for (int j=0; j<x.size(); j++) {
          t[j] = tuples[i*noOfVars+j];
        }
        ts.add(t);
      }
      ts.finalize();
      extensional(s,x,ts,EPK_DEF,ann2icl(ann));
    }

    void p_cumulatives(FlatZincSpace& s, const ConExpr& ce,
                      AST::Node* ann) {
      IntVarArgs start = arg2intvarargs(s, ce[0]);
      IntVarArgs duration = arg2intvarargs(s, ce[1]);
      IntVarArgs height = arg2intvarargs(s, ce[2]);
      int n = start.size();
      IntVar bound = getIntVar(s, ce[3]);

      int minHeight = INT_MAX; int minHeight2 = INT_MAX;
      for (int i=n; i--;)
        if (height[i].min() < minHeight)
          minHeight = height[i].min();
        else if (height[i].min() < minHeight2)
          minHeight2 = height[i].min();
      bool disjunctive =
       (minHeight > bound.max()/2) ||
       (minHeight2 > bound.max()/2 && minHeight+minHeight2>bound.max());
      if (disjunctive) {
        rel(s, bound >= max(height));
        // Unary
        if (duration.assigned()) {
          IntArgs durationI(n);
          for (int i=n; i--;)
            durationI[i] = duration[i].val();
          unary(s,start,durationI);
        } else {
          IntVarArgs end(n);
          for (int i=n; i--;)
            end[i] = expr(s,start[i]+duration[i]);
          unary(s,start,duration,end);
        }
      } else if (height.assigned()) {
        IntArgs heightI(n);
        for (int i=n; i--;)
          heightI[i] = height[i].val();
        if (duration.assigned()) {
          IntArgs durationI(n);
          for (int i=n; i--;)
            durationI[i] = duration[i].val();
          cumulative(s, bound, start, durationI, heightI);
        } else {
          IntVarArgs end(n);
          for (int i = n; i--; )
            end[i] = expr(s,start[i]+duration[i]);
          cumulative(s, bound, start, duration, end, heightI);
        }
      } else if (bound.assigned()) {
        IntArgs machine = IntArgs::create(n,0,0);
        IntArgs limit(1, bound.val());
        IntVarArgs end(n);
        for (int i=n; i--;)
          end[i] = expr(s,start[i]+duration[i]);
        cumulatives(s, machine, start, duration, end, height, limit, true,
                    ann2icl(ann));
      } else {
        int min = Gecode::Int::Limits::max;
        int max = Gecode::Int::Limits::min;
        IntVarArgs end(start.size());
        for (int i = start.size(); i--; ) {
          min = std::min(min, start[i].min());
          max = std::max(max, start[i].max() + duration[i].max());
          end[i] = expr(s, start[i] + duration[i]);
        }
        for (int time = min; time < max; ++time) {
          IntVarArgs x(start.size());
          for (int i = start.size(); i--; ) {
            IntVar overlaps = channel(s, expr(s, (start[i] <= time) && 
                                                 (time < end[i])));
            x[i] = expr(s, overlaps * height[i]);
          }
          linear(s, x, IRT_LQ, bound);
        }
      }
    }

    void p_among_seq_int(FlatZincSpace& s, const ConExpr& ce,
                         AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntSet S = arg2intset(s, ce[1]);
      int q = ce[2]->getInt();
      int l = ce[3]->getInt();
      int u = ce[4]->getInt();
      sequence(s, x, S, q, l, u, ann2icl(ann));
    }

    void p_among_seq_bool(FlatZincSpace& s, const ConExpr& ce,
                          AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      bool val = ce[1]->getBool();
      int q = ce[2]->getInt();
      int l = ce[3]->getInt();
      int u = ce[4]->getInt();
      IntSet S(val, val);
      sequence(s, x, S, q, l, u, ann2icl(ann));
    }

    void p_schedule_unary(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs p = arg2intargs(ce[1]);
      unary(s, x, p);
    }

    void p_schedule_unary_optional(FlatZincSpace& s, const ConExpr& ce,
                                   AST::Node*) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs p = arg2intargs(ce[1]);
      BoolVarArgs m = arg2boolvarargs(s, ce[2]);
      unary(s, x, p, m);
    }

    void p_circuit(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      IntVarArgs xv = arg2intvarargs(s, ce[0]);
      circuit(s,xv,ann2icl(ann));
    }
    void p_circuit_cost_array(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node *ann) {
      IntArgs c = arg2intargs(ce[0]);
      IntVarArgs xv = arg2intvarargs(s, ce[1]);
      IntVarArgs yv = arg2intvarargs(s, ce[2]);
      IntVar z = getIntVar(s, ce[3]);
      circuit(s,c,xv,yv,z,ann2icl(ann));
    }
    void p_circuit_cost(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      IntArgs c = arg2intargs(ce[0]);
      IntVarArgs xv = arg2intvarargs(s, ce[1]);
      IntVar z = getIntVar(s, ce[2]);
      circuit(s,c,xv,z,ann2icl(ann));
    }

    void p_nooverlap(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      IntVarArgs x0 = arg2intvarargs(s, ce[0]);
      IntVarArgs w = arg2intvarargs(s, ce[1]);
      IntVarArgs y0 = arg2intvarargs(s, ce[2]);
      IntVarArgs h = arg2intvarargs(s, ce[3]);
      IntVarArgs x1(x0.size()), y1(y0.size());
      for (int i=x0.size(); i--; )
        x1[i] = expr(s, x0[i] + w[i]);
      for (int i=y0.size(); i--; )
        y1[i] = expr(s, y0[i] + h[i]);
      nooverlap(s,x0,w,x1,y0,h,y1,ann2icl(ann));
    }

    void p_precede(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntArgs c = arg2intargs(ce[1]);
      precede(s,x,c,ann2icl(ann));
    }

    void p_nvalue(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[1]);
      if (ce[0]->isIntVar()) {
        IntVar y = getIntVar(s,ce[0]);
        nvalues(s,x,IRT_EQ,y,ann2icl(ann));
      } else {
        nvalues(s,x,IRT_EQ,ce[0]->getInt(),ann2icl(ann));
      }
    }

    void p_among(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[1]);
      IntSet v = arg2intset(s, ce[2]);
      if (ce[0]->isIntVar()) {
        IntVar n = getIntVar(s,ce[0]);
        std::cerr << "count " << n << std::endl;
        count(s,x,v,IRT_EQ,n,ann2icl(ann));
      } else {
        std::cerr << "count i " << x << " " << v << " " << ce[0]->getInt() << std::endl;
        count(s,x,v,IRT_EQ,ce[0]->getInt(),ann2icl(ann));
      }
    }

    void p_member_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntVar y = getIntVar(s, ce[1]);
      member(s,x,y,ann2icl(ann));
    }
    void p_member_int_reif(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node* ann) {
      IntVarArgs x = arg2intvarargs(s, ce[0]);
      IntVar y = getIntVar(s, ce[1]);
      BoolVar b = getBoolVar(s, ce[2]);
      member(s,x,y,b,ann2icl(ann));
    }
    void p_member_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      BoolVar y = getBoolVar(s, ce[1]);
      member(s,x,y,ann2icl(ann));
    }
    void p_member_bool_reif(FlatZincSpace& s, const ConExpr& ce,
                            AST::Node* ann) {
      BoolVarArgs x = arg2boolvarargs(s, ce[0]);
      BoolVar y = getBoolVar(s, ce[1]);
      member(s,x,y,getBoolVar(s, ce[2]),ann2icl(ann));
    }

    class IntPoster {
    public:
      IntPoster(void) {
        registry().add("all_different_int", &p_distinct);
        registry().add("all_different_offset", &p_distinctOffset);
        registry().add("all_equal_int", &p_all_equal);
        registry().add("int_eq", &p_int_eq);
        registry().add("int_ne", &p_int_ne);
        registry().add("int_ge", &p_int_ge);
        registry().add("int_gt", &p_int_gt);
        registry().add("int_le", &p_int_le);
        registry().add("int_lt", &p_int_lt);
        registry().add("int_eq_reif", &p_int_eq_reif);
        registry().add("int_ne_reif", &p_int_ne_reif);
        registry().add("int_ge_reif", &p_int_ge_reif);
        registry().add("int_gt_reif", &p_int_gt_reif);
        registry().add("int_le_reif", &p_int_le_reif);
        registry().add("int_lt_reif", &p_int_lt_reif);
        registry().add("int_lin_eq", &p_int_lin_eq);
        registry().add("int_lin_eq_reif", &p_int_lin_eq_reif);
        registry().add("int_lin_ne", &p_int_lin_ne);
        registry().add("int_lin_ne_reif", &p_int_lin_ne_reif);
        registry().add("int_lin_le", &p_int_lin_le);
        registry().add("int_lin_le_reif", &p_int_lin_le_reif);
        registry().add("int_lin_lt", &p_int_lin_lt);
        registry().add("int_lin_lt_reif", &p_int_lin_lt_reif);
        registry().add("int_lin_ge", &p_int_lin_ge);
        registry().add("int_lin_ge_reif", &p_int_lin_ge_reif);
        registry().add("int_lin_gt", &p_int_lin_gt);
        registry().add("int_lin_gt_reif", &p_int_lin_gt_reif);
        registry().add("int_plus", &p_int_plus);
        registry().add("int_minus", &p_int_minus);
        registry().add("int_times", &p_int_times);
        registry().add("int_div", &p_int_div);
        registry().add("int_mod", &p_int_mod);
        registry().add("int_min", &p_int_min);
        registry().add("int_max", &p_int_max);
        registry().add("int_abs", &p_abs);
        registry().add("int_negate", &p_int_negate);
        registry().add("bool_eq", &p_bool_eq);
        registry().add("bool_eq_reif", &p_bool_eq_reif);
        registry().add("bool_ne", &p_bool_ne);
        registry().add("bool_ne_reif", &p_bool_ne_reif);
        registry().add("bool_ge", &p_bool_ge);
        registry().add("bool_ge_reif", &p_bool_ge_reif);
        registry().add("bool_le", &p_bool_le);
        registry().add("bool_le_reif", &p_bool_le_reif);
        registry().add("bool_gt", &p_bool_gt);
        registry().add("bool_gt_reif", &p_bool_gt_reif);
        registry().add("bool_lt", &p_bool_lt);
        registry().add("bool_lt_reif", &p_bool_lt_reif);
        registry().add("bool_or", &p_bool_or);
        registry().add("bool_and", &p_bool_and);
        registry().add("bool_xor", &p_bool_xor);
        registry().add("array_bool_and", &p_array_bool_and);
        registry().add("array_bool_or", &p_array_bool_or);
        registry().add("array_bool_xor", &p_array_bool_xor);
        registry().add("bool_clause", &p_array_bool_clause);
        registry().add("bool_clause_reif", &p_array_bool_clause_reif);
        registry().add("bool_left_imp", &p_bool_l_imp);
        registry().add("bool_right_imp", &p_bool_r_imp);
        registry().add("bool_not", &p_bool_not);
        registry().add("array_int_element", &p_array_int_element);
        registry().add("array_var_int_element", &p_array_int_element);
        registry().add("array_bool_element", &p_array_bool_element);
        registry().add("array_var_bool_element", &p_array_bool_element);
        registry().add("bool2int", &p_bool2int);
        registry().add("int_in", &p_int_in);
        registry().add("int_in_reif", &p_int_in_reif);
#ifndef GECODE_HAS_SET_VARS
        registry().add("set_in", &p_int_in);
        registry().add("set_in_reif", &p_int_in_reif);
#endif
      
        registry().add("array_int_lt", &p_array_int_lt);
        registry().add("array_int_lq", &p_array_int_lq);
        registry().add("count", &p_count);
        registry().add("at_least_int", &p_at_least);
        registry().add("at_most_int", &p_at_most);
        registry().add("gecode_bin_packing_load", &p_bin_packing_load);
        registry().add("global_cardinality", &p_global_cardinality);
        registry().add("global_cardinality_closed",
          &p_global_cardinality_closed);
        registry().add("global_cardinality_low_up", 
          &p_global_cardinality_low_up);
        registry().add("global_cardinality_low_up_closed", 
          &p_global_cardinality_low_up_closed);
        registry().add("minimum_int", &p_minimum);
        registry().add("maximum_int", &p_maximum);
        registry().add("regular", &p_regular);
        registry().add("sort", &p_sort);
        registry().add("inverse_offsets", &p_inverse_offsets);
        registry().add("increasing_int", &p_increasing_int);
        registry().add("increasing_bool", &p_increasing_bool);
        registry().add("decreasing_int", &p_decreasing_int);
        registry().add("decreasing_bool", &p_decreasing_bool);
        registry().add("table_int", &p_table_int);
        registry().add("table_bool", &p_table_bool);
        registry().add("cumulatives", &p_cumulatives);
        registry().add("gecode_among_seq_int", &p_among_seq_int);
        registry().add("gecode_among_seq_bool", &p_among_seq_bool);

        registry().add("bool_lin_eq", &p_bool_lin_eq);
        registry().add("bool_lin_ne", &p_bool_lin_ne);
        registry().add("bool_lin_le", &p_bool_lin_le);
        registry().add("bool_lin_lt", &p_bool_lin_lt);
        registry().add("bool_lin_ge", &p_bool_lin_ge);
        registry().add("bool_lin_gt", &p_bool_lin_gt);

        registry().add("bool_lin_eq_reif", &p_bool_lin_eq_reif);
        registry().add("bool_lin_ne_reif", &p_bool_lin_ne_reif);
        registry().add("bool_lin_le_reif", &p_bool_lin_le_reif);
        registry().add("bool_lin_lt_reif", &p_bool_lin_lt_reif);
        registry().add("bool_lin_ge_reif", &p_bool_lin_ge_reif);
        registry().add("bool_lin_gt_reif", &p_bool_lin_gt_reif);
        
        registry().add("gecode_schedule_unary", &p_schedule_unary);
        registry().add("gecode_schedule_unary_optional", &p_schedule_unary_optional);

        registry().add("gecode_circuit", &p_circuit);
        registry().add("gecode_circuit_cost_array", &p_circuit_cost_array);
        registry().add("gecode_circuit_cost", &p_circuit_cost);
        registry().add("gecode_nooverlap", &p_nooverlap);
        registry().add("gecode_precede", &p_precede);
        registry().add("nvalue",&p_nvalue);
        registry().add("among",&p_among);
        registry().add("member_int",&p_member_int);
        registry().add("gecode_member_int_reif",&p_member_int_reif);
        registry().add("member_bool",&p_member_bool);
        registry().add("gecode_member_bool_reif",&p_member_bool_reif);
      }
    };
    IntPoster __int_poster;

#ifdef GECODE_HAS_SET_VARS
    void p_set_OP(FlatZincSpace& s, SetOpType op,
                  const ConExpr& ce, AST::Node *) {
      rel(s, getSetVar(s, ce[0]), op, getSetVar(s, ce[1]), 
          SRT_EQ, getSetVar(s, ce[2]));
    }
    void p_set_union(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_UNION, ce, ann);
    }
    void p_set_intersect(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_INTER, ce, ann);
    }
    void p_set_diff(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_MINUS, ce, ann);
    }

    void p_set_symdiff(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      SetVar x = getSetVar(s, ce[0]);
      SetVar y = getSetVar(s, ce[1]);

      SetVarLubRanges xub(x);
      IntSet xubs(xub);
      SetVar x_y(s,IntSet::empty,xubs);
      rel(s, x, SOT_MINUS, y, SRT_EQ, x_y);

      SetVarLubRanges yub(y);
      IntSet yubs(yub);
      SetVar y_x(s,IntSet::empty,yubs);
      rel(s, y, SOT_MINUS, x, SRT_EQ, y_x);
    
      rel(s, x_y, SOT_UNION, y_x, SRT_EQ, getSetVar(s, ce[2]));
    }

    void p_array_set_OP(FlatZincSpace& s, SetOpType op,
                        const ConExpr& ce, AST::Node *) {
      SetVarArgs xs = arg2setvarargs(s, ce[0]);
      rel(s, op, xs, getSetVar(s, ce[1]));
    }
    void p_array_set_union(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_array_set_OP(s, SOT_UNION, ce, ann);
    }
    void p_array_set_partition(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_array_set_OP(s, SOT_DUNION, ce, ann);
    }


    void p_set_rel(FlatZincSpace& s, SetRelType srt, const ConExpr& ce) {
      rel(s, getSetVar(s, ce[0]), srt, getSetVar(s, ce[1]));
    }

    void p_set_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_EQ, ce);
    }
    void p_set_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_NQ, ce);
    }
    void p_set_subset(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_SUB, ce);
    }
    void p_set_superset(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_SUP, ce);
    }
    void p_set_le(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_LQ, ce);
    }
    void p_set_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_LE, ce);
    }
    void p_set_card(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      if (!ce[1]->isIntVar()) {
        cardinality(s, getSetVar(s, ce[0]), ce[1]->getInt(), 
                    ce[1]->getInt());
      } else {
        cardinality(s, getSetVar(s, ce[0]), getIntVar(s, ce[1]));
      }
    }
    void p_set_in(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      if (!ce[1]->isSetVar()) {
        IntSet d = arg2intset(s,ce[1]);
        if (ce[0]->isBoolVar()) {
          IntSetRanges dr(d);
          Iter::Ranges::Singleton sr(0,1);
          Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
          IntSet d01(i);
          if (d01.size() == 0) {
            s.fail();
          } else {
            rel(s, getBoolVar(s, ce[0]), IRT_GQ, d01.min());
            rel(s, getBoolVar(s, ce[0]), IRT_LQ, d01.max());
          }
        } else {
          dom(s, getIntVar(s, ce[0]), d);
        }
      } else {
        if (!ce[0]->isIntVar()) {
          dom(s, getSetVar(s, ce[1]), SRT_SUP, ce[0]->getInt());
        } else {
          rel(s, getSetVar(s, ce[1]), SRT_SUP, getIntVar(s, ce[0]));
        }
      }
    }
    void p_set_rel_reif(FlatZincSpace& s, SetRelType srt, const ConExpr& ce) {
      rel(s, getSetVar(s, ce[0]), srt, getSetVar(s, ce[1]),
          getBoolVar(s, ce[2]));
    }

    void p_set_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_EQ,ce);
    }
    void p_set_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_LQ,ce);
    }
    void p_set_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_LE,ce);
    }
    void p_set_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_NQ,ce);
    }
    void p_set_subset_reif(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node *) {
      p_set_rel_reif(s,SRT_SUB,ce);
    }
    void p_set_superset_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node *) {
      p_set_rel_reif(s,SRT_SUP,ce);
    }
    void p_set_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      if (!ce[1]->isSetVar()) {
        IntSet d = arg2intset(s,ce[1]);
        if (ce[0]->isBoolVar()) {
          IntSetRanges dr(d);
          Iter::Ranges::Singleton sr(0,1);
          Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
          IntSet d01(i);
          if (d01.size() == 0) {
            rel(s, getBoolVar(s, ce[2]) == 0);
          } else if (d01.max() == 0) {
            rel(s, getBoolVar(s, ce[2]) == !getBoolVar(s, ce[0]));
          } else if (d01.min() == 1) {
            rel(s, getBoolVar(s, ce[2]) == getBoolVar(s, ce[0]));
          } else {
            rel(s, getBoolVar(s, ce[2]) == 1);
          }
        } else {
          dom(s, getIntVar(s, ce[0]), d, getBoolVar(s, ce[2]));
        }
      } else {
        if (!ce[0]->isIntVar()) {
          dom(s, getSetVar(s, ce[1]), SRT_SUP, ce[0]->getInt(),
              getBoolVar(s, ce[2]));
        } else {
          rel(s, getSetVar(s, ce[1]), SRT_SUP, getIntVar(s, ce[0]),
              getBoolVar(s, ce[2]));
        }
      }
    }
    void p_set_disjoint(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      rel(s, getSetVar(s, ce[0]), SRT_DISJ, getSetVar(s, ce[1]));
    }
    
    void p_link_set_to_booleans(FlatZincSpace& s, const ConExpr& ce,
                                AST::Node *) {
      SetVar x = getSetVar(s, ce[0]);
      int idx = ce[2]->getInt();
      assert(idx >= 0);
      rel(s, x || IntSet(Set::Limits::min,idx-1));
      BoolVarArgs y = arg2boolvarargs(s,ce[1],idx);
      channel(s, y, x);
    }

    void p_array_set_element(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node*) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isSetVar()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = getIntVar(s, ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntSetArgs sv = arg2intsetargs(s,ce[1],1);
        element(s, sv, selector, getSetVar(s, ce[2]));
      } else {
        SetVarArgs sv = arg2setvarargs(s, ce[1], 1);
        element(s, sv, selector, getSetVar(s, ce[2]));
      }
    }

    void p_array_set_element_op(FlatZincSpace& s, const ConExpr& ce,
                                AST::Node*, SetOpType op,
                                const IntSet& universe = 
                                IntSet(Set::Limits::min,Set::Limits::max)) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isSetVar()) {
          isConstant = false;
          break;
        }
      }
      SetVar selector = getSetVar(s, ce[0]);
      dom(s, selector, SRT_DISJ, 0);
      if (isConstant) {
        IntSetArgs sv = arg2intsetargs(s,ce[1], 1);
        element(s, op, sv, selector, getSetVar(s, ce[2]), universe);
      } else {
        SetVarArgs sv = arg2setvarargs(s, ce[1], 1);
        element(s, op, sv, selector, getSetVar(s, ce[2]), universe);
      }
    }

    void p_array_set_element_union(FlatZincSpace& s, const ConExpr& ce,
                                       AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_UNION);
    }

    void p_array_set_element_intersect(FlatZincSpace& s, const ConExpr& ce,
                                       AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_INTER);
    }

    void p_array_set_element_intersect_in(FlatZincSpace& s,
                                              const ConExpr& ce,
                                              AST::Node* ann) {
      IntSet d = arg2intset(s, ce[3]);
      p_array_set_element_op(s, ce, ann, SOT_INTER, d);
    }

    void p_array_set_element_partition(FlatZincSpace& s, const ConExpr& ce,
                                           AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_DUNION);
    }

    void p_set_convex(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      convex(s, getSetVar(s, ce[0]));
    }

    void p_array_set_seq(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      SetVarArgs sv = arg2setvarargs(s, ce[0]);
      sequence(s, sv);
    }

    void p_array_set_seq_union(FlatZincSpace& s, const ConExpr& ce,
                               AST::Node *) {
      SetVarArgs sv = arg2setvarargs(s, ce[0]);
      sequence(s, sv, getSetVar(s, ce[1]));
    }

    void p_int_set_channel(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node *) {
      int xoff=ce[1]->getInt();
      assert(xoff >= 0);
      int yoff=ce[3]->getInt();
      assert(yoff >= 0);
      IntVarArgs xv = arg2intvarargs(s, ce[0], xoff);
      SetVarArgs yv = arg2setvarargs(s, ce[2], yoff, 1, IntSet(0, xoff-1));
      IntSet xd(yoff,yv.size()-1);
      for (int i=xoff; i<xv.size(); i++) {
        dom(s, xv[i], xd);
      }
      IntSet yd(xoff,xv.size()-1);
      for (int i=yoff; i<yv.size(); i++) {
        dom(s, yv[i], SRT_SUB, yd);
      }
      channel(s,xv,yv);
    }
    
    void p_range(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      int xoff=ce[1]->getInt();
      assert(xoff >= 0);
      IntVarArgs xv = arg2intvarargs(s,ce[0],xoff);
      element(s, SOT_UNION, xv, getSetVar(s,ce[2]), getSetVar(s,ce[3]));
    }
    
    void p_weights(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      IntArgs e = arg2intargs(ce[0]);
      IntArgs w = arg2intargs(ce[1]);
      SetVar x = getSetVar(s,ce[2]);
      IntVar y = getIntVar(s,ce[3]);
      weights(s,e,w,x,y);
    }
    
    class SetPoster {
    public:
      SetPoster(void) {
        registry().add("set_eq", &p_set_eq);
        registry().add("set_le", &p_set_le);
        registry().add("set_lt", &p_set_lt);
        registry().add("equal", &p_set_eq);
        registry().add("set_ne", &p_set_ne);
        registry().add("set_union", &p_set_union);
        registry().add("array_set_element", &p_array_set_element);
        registry().add("array_var_set_element", &p_array_set_element);
        registry().add("set_intersect", &p_set_intersect);
        registry().add("set_diff", &p_set_diff);
        registry().add("set_symdiff", &p_set_symdiff);
        registry().add("set_subset", &p_set_subset);
        registry().add("set_superset", &p_set_superset);
        registry().add("set_card", &p_set_card);
        registry().add("set_in", &p_set_in);
        registry().add("set_eq_reif", &p_set_eq_reif);
        registry().add("set_le_reif", &p_set_le_reif);
        registry().add("set_lt_reif", &p_set_lt_reif);
        registry().add("equal_reif", &p_set_eq_reif);
        registry().add("set_ne_reif", &p_set_ne_reif);
        registry().add("set_subset_reif", &p_set_subset_reif);
        registry().add("set_superset_reif", &p_set_superset_reif);
        registry().add("set_in_reif", &p_set_in_reif);
        registry().add("disjoint", &p_set_disjoint);
        registry().add("gecode_link_set_to_booleans", 
                       &p_link_set_to_booleans);

        registry().add("array_set_union", &p_array_set_union);
        registry().add("array_set_partition", &p_array_set_partition);
        registry().add("set_convex", &p_set_convex);
        registry().add("array_set_seq", &p_array_set_seq);
        registry().add("array_set_seq_union", &p_array_set_seq_union);
        registry().add("gecode_array_set_element_union", 
                       &p_array_set_element_union);
        registry().add("gecode_array_set_element_intersect", 
                       &p_array_set_element_intersect);
        registry().add("gecode_array_set_element_intersect_in", 
                       &p_array_set_element_intersect_in);
        registry().add("gecode_array_set_element_partition", 
                       &p_array_set_element_partition);
        registry().add("gecode_int_set_channel", 
                       &p_int_set_channel);
        registry().add("gecode_range",
                       &p_range);
        registry().add("gecode_set_weights",
                       &p_weights);
      }
    };
    SetPoster __set_poster;
#endif

  }
}}

// STATISTICS: flatzinc-any
