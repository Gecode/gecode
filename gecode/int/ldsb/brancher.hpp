/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
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

#include <deque>
#include <set>

namespace Gecode { namespace Int { namespace LDSB {

  forceinline
  Literal::Literal(void)
    : _variable(-1), _value(-1) {}

  forceinline
  Literal::Literal(int idx, int val)
    : _variable(idx), _value(val) {}

  forceinline
  bool
  Literal::operator <(const Literal &rhs) const {
    int d = rhs._variable - _variable;
    if      (d > 0)  return true;
    else if (d == 0) return rhs._value > _value;
    else             return false;
  }


  template<class Val>
  inline
  LDSBChoice<Val>::LDSBChoice(const Brancher& b, unsigned int a, const Pos& p,
                              const Val& n, const Literal* literals,
                              int nliterals)
    : PosValChoice<Val>(b,a,p,n), _literals(literals), _nliterals(nliterals)
    {}

  template<class Val>
  LDSBChoice<Val>::~LDSBChoice(void) {
    delete [] _literals;
  }

  template<class Val>
  forceinline const Literal*
  LDSBChoice<Val>::literals(void) const { return _literals; }

  template<class Val>
  forceinline int
  LDSBChoice<Val>::nliterals(void) const { return _nliterals; }

  template<class Val>
  void
  LDSBChoice<Val>::archive(Archive& e) const {
    PosValChoice<Val>::archive(e);
    e << _nliterals;
    for (int i = 0 ; i < _nliterals ; i++) {
      e << _literals[i]._variable;
      e << _literals[i]._value;
    }
  }



  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  LDSBBrancher<View,n,Val,a,Filter,Print>
  ::LDSBBrancher(Home home, ViewArray<View>& x,
                 ViewSel<View>* vs[n],
                 ValSelCommitBase<View,Val>* vsc,
                 SymmetryImp<View>** syms, int nsyms,
                 BranchFilter<LDSBBrancher<View,n,Val,a,Filter,Print>::Var> bf,
                 VarValPrint<LDSBBrancher<View,n,Val,a,Filter,Print>::Var,Val> vvp)
    : ViewValBrancher<View,n,Val,a,Filter,Print>(home, x, vs, vsc, bf, vvp),
      _syms(syms),
      _nsyms(nsyms),
      _prevPos(-1)
  {
    home.notice(*this, AP_DISPOSE, true);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline void
  LDSBBrancher<View,n,Val,a,Filter,Print>::
  post(Home home, ViewArray<View>& x,
       ViewSel<View>* vs[n], ValSelCommitBase<View,Val>* vsc,
       SymmetryImp<View>** syms, int nsyms,
       BranchFilter<LDSBBrancher<View,n,Val,a,Filter,Print>::Var> bf,
       VarValPrint<LDSBBrancher<View,n,Val,a,Filter,Print>::Var,Val> vvp) {
    (void) new (home) LDSBBrancher<View,n,Val,a,Filter,Print>
      (home,x,vs,vsc,syms,nsyms,bf,vvp);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline
  LDSBBrancher<View,n,Val,a,Filter,Print>::
  LDSBBrancher(Space& home,
               LDSBBrancher<View,n,Val,a,Filter,Print>& b)
    : ViewValBrancher<View,n,Val,a,Filter,Print>(home,b),
      _nsyms(b._nsyms),
      _prevPos(b._prevPos) {
    _syms = home.alloc<SymmetryImp<View>*>(_nsyms);
    for (int i = 0 ; i < _nsyms ; i++)
      _syms[i] = b._syms[i]->copy(home);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  Actor*
  LDSBBrancher<View,n,Val,a,Filter,Print>::copy(Space& home) {
    return new (home) LDSBBrancher<View,n,Val,a,Filter,Print>
      (home,*this);
  }


  // Compute choice
  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  const Choice*
  LDSBBrancher<View,n,Val,a,Filter,Print>::choice(Space& home) {
    // Making the PVC here is not so nice, I think.
    const Choice* c = ViewValBrancher<View,n,Val,a,Filter,Print>::choice(home);
    const PosValChoice<Val>* pvc = static_cast<const PosValChoice<Val>* >(c);

    // Compute symmetries.

    int choicePos = pvc->pos().pos;
    int choiceVal = pvc->val();
    delete c;

    _prevPos = choicePos;

    // TODO: It should be possible to use simpler containers than the
    // STL ones here.
    std::deque<Literal> queue;
    std::set<Literal> seen;

    seen.insert(Literal(choicePos, choiceVal));
    queue.push_back(Literal(choicePos, choiceVal));

    do {
      Literal l = queue[0];
      queue.pop_front();

      for (int i = 0 ; i < _nsyms ; i++) {
        ArgArray<Literal> toExclude = _syms[i]->symmetric(l, this->x);
        for (int j = 0 ; j < toExclude.size() ; ++j) {
          if (seen.find(toExclude[j]) == seen.end())
            queue.push_back(toExclude[j]);
          seen.insert(toExclude[j]);
        }
      }
    } while (queue.size() > 0);

    // Convert "seen" vector into array.
    int nliterals = static_cast<int>(seen.size());
    Literal* literals = new Literal[nliterals];
    std::set<Literal>::iterator it = seen.begin();
    for (int i = 0 ; i < nliterals ; i++) {
      literals[i] = *it;
      ++it;
    }

    return new LDSBChoice<Val>(*this,a,choicePos,choiceVal, literals, nliterals);
  }


  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  const Choice*
  LDSBBrancher<View,n,Val,a,Filter,Print>::choice(const Space& home,
                                                  Archive& e) {
    (void) home;
    int p; e >> p;
    Val v; e >> v;
    int nliterals; e >> nliterals;
    Literal* literals = new Literal[nliterals];
    for (int i = 0 ; i < nliterals ; i++) {
      e >> literals[i]._variable;
      e >> literals[i]._value;
    }
    return new LDSBChoice<Val>(*this,a,p,v, literals, nliterals);
  }

  template <>
  inline
  ModEvent
  prune<Int::IntView>(Space& home, Int::IntView x, int v) {
    return x.nq(home, v);
  }

  template <>
  inline
  ModEvent
  prune<Int::BoolView>(Space& home, Int::BoolView x, int v) {
    return x.nq(home, v);
  }


  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  ExecStatus
  LDSBBrancher<View,n,Val,a,Filter,Print>
  ::commit(Space& home, const Choice& c, unsigned int b) {
    const LDSBChoice<Val>& pvc
      = static_cast<const LDSBChoice<Val>&>(c);
    int choicePos = pvc.pos().pos;
    int choiceVal = pvc.val();

    if (b == 0) {
      // Post the branching constraint.
      ExecStatus fromBase = ViewValBrancher<View,n,Val,a,Filter,Print>
        ::commit(home, c, b);
      GECODE_ES_CHECK(fromBase);
      for (int i = 0 ; i < this->_nsyms ; i++)
        this->_syms[i]->update(Literal(choicePos, choiceVal));
    } else if (b == 1) {
      // Post the branching constraint.
      ExecStatus fromBase = ViewValBrancher<View,n,Val,a,Filter,Print>
        ::commit(home, c, b);
      GECODE_ES_CHECK(fromBase);

      // Post prunings.
      int nliterals = pvc.nliterals();
      const Literal* literals = pvc.literals();
      for (int i = 0 ; i < nliterals ; i++) {
        const Literal& l = literals[i];
        ModEvent me = prune<View>(home, this->x[l._variable], l._value);
        GECODE_ME_CHECK(me);
      }
    }

    return ES_OK;
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  size_t
  LDSBBrancher<View,n,Val,a,Filter,Print>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE,true);
    (void) ViewValBrancher<View,n,Val,a,Filter,Print>::dispose(home);
    return sizeof(LDSBBrancher<View,n,Val,a,Filter,Print>);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline void
  postldsbbrancher(Home home,
                   ViewArray<View>& x,
                   ViewSel<View>* vs[n],
                   ValSelCommitBase<View,Val>* vsc,
                   SymmetryImp<View>** syms, int nsyms,
                   BranchFilter<typename View::VarType> bf,
                   VarValPrint<typename View::VarType,Val> vvp) {
    if (bf) {
      if (vvp) {
        LDSBBrancher<View,n,Val,a,BrancherFilter<View>,BrancherPrint<View,Val>>
          ::post(home,x,vs,vsc,syms,nsyms,bf,vvp);
      } else {
        LDSBBrancher<View,n,Val,a,BrancherFilter<View>,BrancherNoPrint<View,Val> >
          ::post(home,x,vs,vsc,syms,nsyms,bf,vvp);
      }
    } else {
      if (vvp) {
        LDSBBrancher<View,n,Val,a,BrancherNoFilter<View>,BrancherPrint<View,Val>>
          ::post(home,x,vs,vsc,syms,nsyms,bf,vvp);
      } else {
        LDSBBrancher<View,n,Val,a,BrancherNoFilter<View>,BrancherNoPrint<View,Val> >
          ::post(home,x,vs,vsc,syms,nsyms,bf,vvp);
      }
    }
  }      

}}}

// STATISTICS: int-branch
