/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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
 *
 */

#ifndef GECODE_INT_BRANCH_OPEN_HPP
#define GECODE_INT_BRANCH_OPEN_HPP

#include <gecode/int/branch.hh>

namespace Gecode { namespace Int { namespace Branch {

  /// Standard view/value selectors over the materialized prefix
  template<class View, class Filter, int n>
  class OpenListBrancher : public Brancher {
  protected:
    typedef typename View::VarType Var;
    OpenViewList<View> list;
    ViewSel<View>* vs[n];
    /// Binary value selector, or null for one alternative per domain value
    ValSelCommitBase<View,int>* vsc;
    Filter filter;
    SharedData<VarValPrint<Var,int>> printer;
    /// Whether enumerated domain values are tried in increasing order
    bool values_min;
    bool select_first;
    mutable int start;

    OpenListBrancher(Home home, OpenViewList<View> list0,
                     ViewSel<View>* vs0[n],
                     ValSelCommitBase<View,int>* vsc0,
                     bool values_min0,
                     bool select_first0,
                     BranchFilter<Var> bf, VarValPrint<Var,int> vvp)
      : Brancher(home), list(list0), vsc(vsc0),
        filter(bf), printer(vvp), values_min(values_min0),
        select_first(select_first0), start(0) {
      for (int i=0; i<n; i++)
        vs[i] = vs0[i];
      home.notice(*this,AP_DISPOSE,true);
    }

    OpenListBrancher(Space& home, OpenListBrancher& b)
      : Brancher(home,b), vsc(nullptr),
        filter(b.filter), printer(b.printer),
        values_min(b.values_min), select_first(b.select_first), start(b.start) {
      list.update(home,b.list);
      int copied = 0;
      try {
        if (b.vsc)
          vsc = b.vsc->copy(home);
        for (; copied<n; copied++)
          vs[copied] = b.vs[copied]->copy(home);
      } catch (...) {
        for (int i=0; i<copied; i++)
          vs[i]->dispose(home);
        if (vsc)
          vsc->dispose(home);
        throw;
      }
    }

    /// Select from a temporary array, keeping positions stable across growth
    int
    position(Space& home) {
      if (select_first)
        return start;
      Region r;
      ViewArray<View> x(r,list.known_length());
      for (int i=0; i<x.size(); i++)
        x[i] = list[i];
      if (n == 1)
        return filter ? vs[0]->select(home,x,start,filter)
                      : vs[0]->select(home,x,start);
      int* ties = r.alloc<int>(x.size()-start);
      int n_ties;
      if (filter)
        vs[0]->ties(home,x,start,ties,n_ties,filter);
      else
        vs[0]->ties(home,x,start,ties,n_ties);
      for (int i=1; (i<n-1) && (n_ties>1); i++)
        vs[i]->brk(home,x,ties,n_ties);
      return (n_ties > 1) ? vs[n-1]->select(home,x,ties,n_ties) : ties[0];
    }

    static const Choice*
    value_choice(const Brancher& b, int i, IntView x) {
      return new PosValuesChoice(b,Pos(i),x);
    }

    static const Choice*
    value_choice(const Brancher&, int, BoolView) {
      GECODE_NEVER;
      return nullptr;
    }

    int
    value(const Choice& c, unsigned int a) const {
      const PosValuesChoice& pvc =
        static_cast<const PosValuesChoice&>(c);
      return pvc.val(values_min ? a : pvc.alternatives()-1-a);
    }

  public:
    virtual bool
    status(const Space& home) const {
      for (int i=start; i<list.known_length(); i++)
        if (!list[i].assigned() && filter(home,list[i],i)) {
          start = i;
          return true;
        }
      return list.length().max() > list.known_length();
    }

    virtual const Choice*
    choice(Space& home) {
      for (int i=start; i<list.known_length(); i++)
        if (!list[i].assigned() && filter(home,list[i],i)) {
          start = i;
          const int p = position(home);
          View x(list[p]);
          return vsc ? new PosValChoice<int>(*this,2,Pos(p),vsc->val(home,x,p))
                     : value_choice(*this,p,x);
        }
      // Position -1 marks a length choice: close here or require another item.
      return new PosValChoice<int>(*this,2,Pos(-1),list.known_length());
    }

    virtual const Choice*
    choice(const Space&, Archive& e) {
      int p;
      e >> p;
      if ((p < 0) || vsc) {
        int v;
        e >> v;
        return new PosValChoice<int>(*this,2,Pos(p),v);
      }
      unsigned int a;
      e >> a;
      return new PosValuesChoice(*this,a,Pos(p),e);
    }

    virtual ExecStatus
    commit(Space& home, const Choice& c, unsigned int a) {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p >= 0) {
        // Recomputation can replay choices before propagation grows the prefix.
        if (p >= list.known_length()) {
          list.materialize(home,p+1);
          if (home.failed())
            return ES_FAILED;
        }
        View x(list[p]);
        const ModEvent me = vsc
          ? vsc->commit(home,a,x,p,static_cast<const PosValChoice<int>&>(c).val())
          : x.eq(home,value(c,a));
        return me_failed(me) ? ES_FAILED : ES_OK;
      }
      const int size = static_cast<const PosValChoice<int>&>(c).val();
      if (a == 0)
        return me_failed(list.length().eq(home,size))
          ? ES_FAILED : ES_OK;
      list.materialize(home,size+1);
      return home.failed() ? ES_FAILED : ES_OK;
    }

    virtual NGL*
    ngl(Space& home, const Choice& c, unsigned int a) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0)
        return (a == 0) ? new (home) EqNGL<IntView>
          (home,list.length(),
           static_cast<const PosValChoice<int>&>(c).val()) : nullptr;
      // No-good extraction can run in an ancestor with a shorter prefix.
      if (p >= list.known_length())
        return nullptr;
      View x(list[p]);
      return vsc
        ? vsc->ngl(home,a,x,static_cast<const PosValChoice<int>&>(c).val())
        : new (home) EqNGL<View>(home,x,value(c,a));
    }

    virtual void
    print(const Space& home, const Choice& c, unsigned int a,
          std::ostream& o) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0) {
        o << "list length " << ((a == 0) ? "=" : ">") << " "
          << static_cast<const PosValChoice<int>&>(c).val();
      } else {
        const int v = vsc ? static_cast<const PosValChoice<int>&>(c).val()
                          : value(c,a);
        // Tracing can print replayed choices before commit grows the prefix.
        if (p >= list.known_length()) {
          o << "list[" << p << "] alternative " << a << " (value " << v << ")";
          return;
        }
        View x(list[p]);
        if (printer())
          printer()(home,*this,a,Var(x),p,v,o);
        else if (vsc)
          vsc->print(home,a,x,p,v,o);
        else
          o << "list[" << p << "] = " << v;
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenListBrancher(home,*this);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE,true);
      for (int i=0; i<n; i++)
        vs[i]->dispose(home);
      if (vsc)
        vsc->dispose(home);
      filter.dispose(home);
      printer.~SharedData<VarValPrint<Var,int>>();
      list.~OpenViewList();
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }

    template<class VarBranch>
    static void
    post(Home home, OpenViewList<View> list, VarBranch vars[n],
         ValSelCommitBase<View,int>* vsc,
         bool values_min,
         BranchFilter<Var> bf, VarValPrint<Var,int> vvp) {
      ViewSel<View>* vs[n];
      for (int i=0; i<n; i++)
        vs[i] = viewsel(home,vars[i]);
      (void) new (home)
        OpenListBrancher(home,list,vs,vsc,values_min,
                         vars[0].select() == VarBranch::SEL_NONE,bf,vvp);
    }
  };

  /// Reject fixed-position statistics while new variables can still appear
  inline void
  check_open_selector(IntVarBranch vars, bool open) {
    switch (vars.select()) {
    case IntVarBranch::SEL_ACTION_MIN: case IntVarBranch::SEL_ACTION_MAX:
    case IntVarBranch::SEL_ACTION_SIZE_MIN: case IntVarBranch::SEL_ACTION_SIZE_MAX:
    case IntVarBranch::SEL_CHB_MIN: case IntVarBranch::SEL_CHB_MAX:
    case IntVarBranch::SEL_CHB_SIZE_MIN: case IntVarBranch::SEL_CHB_SIZE_MAX:
      if (open)
        throw UnknownBranching("Int::branch");
      break;
    default: break;
    }
  }

  inline void
  check_open_selector(BoolVarBranch vars, bool open) {
    switch (vars.select()) {
    case BoolVarBranch::SEL_ACTION_MIN: case BoolVarBranch::SEL_ACTION_MAX:
    case BoolVarBranch::SEL_CHB_MIN: case BoolVarBranch::SEL_CHB_MAX:
      if (open)
        throw UnknownBranching("Int::branch");
      break;
    default: break;
    }
  }

  /// Select binary branching or enumeration of integer domain values
  inline ValSelCommitBase<IntView,int>*
  open_valselcommit(Home home, IntValBranch vals, bool& values_min) {
    values_min = vals.select() != IntValBranch::SEL_VALUES_MAX;
    return ((vals.select() == IntValBranch::SEL_VALUES_MIN) ||
            (vals.select() == IntValBranch::SEL_VALUES_MAX))
      ? nullptr : valselcommit(home,vals);
  }

  /// Boolean value branching always uses a binary selector
  inline ValSelCommitBase<BoolView,int>*
  open_valselcommit(Home home, BoolValBranch vals, bool& values_min) {
    values_min = true;
    return valselcommit(home,vals);
  }

  /// Choose the native filtered or unfiltered brancher policy
  template<class View, int n, class VarBranch>
  void
  post_open_brancher(Home home, OpenViewList<View> list,
                     VarBranch vars[n], ValSelCommitBase<View,int>* vsc,
                     bool values_min,
                     BranchFilter<typename View::VarType> bf,
                     VarValPrint<typename View::VarType,int> vvp) {
    if (bf)
      OpenListBrancher<View,BrancherFilter<View>,n>::post
        (home,list,vars,vsc,values_min,bf,vvp);
    else
      OpenListBrancher<View,BrancherNoFilter<View>,n>::post
        (home,list,vars,vsc,values_min,bf,vvp);
  }

  template<class View, class VarBranch, class ValBranch>
  void
  post_open_brancher(Home home, OpenViewList<View> list,
                     TieBreak<VarBranch> vars, ValBranch vals,
                     BranchFilter<typename View::VarType> bf,
                     VarValPrint<typename View::VarType,int> vvp) {
    VarBranch selectors[4] = {vars.a,vars.b,vars.c,vars.d};
    int n = 1;
    while ((n < 4) && (selectors[n-1].select() != VarBranch::SEL_NONE) &&
           (selectors[n-1].select() != VarBranch::SEL_RND) &&
           (selectors[n].select() != VarBranch::SEL_NONE))
      n++;
    typedef typename View::VarType Var;
    typename ArrayTraits<VarArgArray<Var>>::ArgsType x(list.known_length());
    for (int i=0; i<x.size(); i++)
      x[i] = list[i];
    for (int i=0; i<n; i++) {
      check_open_selector(selectors[i],list.length().max() > list.known_length());
      selectors[i].expand(home,x);
    }
    bool values_min;
    ValSelCommitBase<View,int>* vsc = open_valselcommit(home,vals,values_min);
    switch (n) {
    case 1:
      post_open_brancher<View,1>
        (home,list,selectors,vsc,values_min,bf,vvp);
      break;
    case 2:
      post_open_brancher<View,2>
        (home,list,selectors,vsc,values_min,bf,vvp);
      break;
    case 3:
      post_open_brancher<View,3>
        (home,list,selectors,vsc,values_min,bf,vvp);
      break;
    case 4:
      post_open_brancher<View,4>
        (home,list,selectors,vsc,values_min,bf,vvp);
      break;
    }
  }

}}}

#endif

// STATISTICS: int-branch
