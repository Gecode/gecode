/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
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

#include <deque>
#include <set>

namespace Gecode {

  /**
   * \defgroup TaskBranchViewVal Generic brancher based on view and value selection
   *
   * Implements view-based brancher for an array of views and value.
   * \ingroup TaskActor
   */

  //@{
  /// %Choice storing position and value, and symmetric literals to be
  //  excluded on the right branch.
  template<class Val>
  class GECODE_VTABLE_EXPORT LDSBChoice : public PosValChoice<Val> {
  private:
    /// Set of literals to be excluded.
    const Literal * const _literals;
    const int _nliterals;
  public:
    /// Initialize choice for brancher \a b, position \a p, value \a
    /// n, and set of literals \a literals (of size \a nliterals)
    LDSBChoice(const Brancher& b, unsigned int a, const Pos& p, const Val& n, const Literal* literals, int nliterals);
    /// Destructor
    ~LDSBChoice(void);
    /// Return literals
    const Literal* literals(void) const;
    /// Return number of literals
    int nliterals(void) const;
    /// Report size occupied
    virtual size_t size(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

  template<class Val>
  forceinline
  LDSBChoice<Val>::LDSBChoice(const Brancher& b, unsigned int a, const Pos& p, const Val& n, const Literal* literals, int nliterals)
    : PosValChoice<Val>(b,a,p,n), _literals(literals), _nliterals(nliterals) {}

  template<class Val>
  forceinline
  LDSBChoice<Val>::~LDSBChoice() {
    delete [] _literals;
  }
  
  template<class Val>
  forceinline const Literal*
  LDSBChoice<Val>::literals() const { return _literals; }

  template<class Val>
  forceinline int
  LDSBChoice<Val>::nliterals() const { return _nliterals; }

  template<class Val>
  forceinline size_t
  LDSBChoice<Val>::size(void) const {
    return sizeof(LDSBChoice<Val>);
  }

  template<class Val>
  forceinline void
  LDSBChoice<Val>::archive(Archive& e) const {
    PosValChoice<Val>::archive(e);
    e << _nliterals;
    for (int i = 0 ; i < _nliterals ; i++) {
      e << _literals[i].variable;
      e << _literals[i].value;
    }
  }
  

  /**
   * \brief Symmetry-breaking brancher with generic view and value
   * selection
   *
   * Implements view-based branching for an array of views (of type
   * \a View) and value (of type \a Val).
   *
   */
  template<class View, int n, class Val, unsigned int a>
  class LDSBBrancher : public ViewValBrancher<View,n,Val,a> {
    typedef typename ViewBrancher<View,n>::BranchFilter BranchFilter;
  public:
    /// Symmetry information
    SymmetryImp<View>** _syms;
    int _nsyms;
    // Position of variable that last choice was created for
    int _prevPos;
  protected:
    /// Constructor for cloning \a b
    LDSBBrancher(Space& home, bool share, LDSBBrancher& b);
    /// Constructor for creation
    LDSBBrancher(Home home, 
                 ViewArray<View>& x,
                 ViewSel<View>* vs[n], 
                 ValSelCommitBase<View,Val>* vsc,
                 SymmetryImp<View>** syms, int nsyms,
                 BranchFilter bf);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a b
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int b);
    /// Perform cloning
    virtual Actor* copy(Space& home, bool share);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
    /// Brancher post function
    static void post(Home home, 
                     ViewArray<View>& x,
                     ViewSel<View>* vs[n],
                     ValSelCommitBase<View,Val>* vsc,
                     SymmetryImp<View>** syms,
                     int nsyms,
                     BranchFilter bf);
  };
  //@}

  /*
   * Symmetry-breaking brancher based on variable/value selection
   *
   */

  /*
   * Note that the "commit" method is not defined here here; it must
   * be defined in variable-specific code.
   */
  template<class View, int n, class Val, unsigned int a>
  LDSBBrancher<View,n,Val,a>
  ::LDSBBrancher(Home home, ViewArray<View>& x,
                 ViewSel<View>* vs[n],
                 ValSelCommitBase<View,Val>* vsc,
                 SymmetryImp<View>** syms, int nsyms,
                 BranchFilter bf)
    : ViewValBrancher<View,n,Val,a>(home, x, vs, vsc, bf),
      _syms(syms),
      _nsyms(nsyms),
      _prevPos(-1)
  {
    home.notice(*this, AP_DISPOSE);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline void
  LDSBBrancher<View,n,Val,a>::
  post(Home home, ViewArray<View>& x,
       ViewSel<View>* vs[n], ValSelCommitBase<View,Val>* vsc,
       SymmetryImp<View>** syms, int nsyms,
       BranchFilter bf) {
    (void) new (home) LDSBBrancher<View,n,Val,a>(home,x,vs,vsc,syms,nsyms,bf);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline
  LDSBBrancher<View,n,Val,a>::
  LDSBBrancher(Space& home, bool shared, LDSBBrancher<View,n,Val,a>& b)
    : ViewValBrancher<View,n,Val,a>(home,shared,b), 
      _nsyms(b._nsyms),
      _prevPos(b._prevPos) {
    _syms = home.alloc<SymmetryImp<View>*>(_nsyms);
    for (int i = 0 ; i < _nsyms ; i++)
      _syms[i] = b._syms[i]->copy(home, shared);
  }
  
  template<class View, int n, class Val, unsigned int a>
  Actor*
  LDSBBrancher<View,n,Val,a>::copy(Space& home, bool shared) {
    return new (home) LDSBBrancher<View,n,Val,a>(home,shared,*this);
  }


  // Compute choice
  template<class View, int n, class Val, unsigned int a>
  const Choice*
  LDSBBrancher<View,n,Val,a>::choice(Space& home) {
    // Making the PVC here is not so nice, I think.
    const Choice* c = ViewValBrancher<View,n,Val,a>::choice(home);
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
    int nliterals = seen.size();
    Literal* literals = new Literal[nliterals];
    std::set<Literal>::iterator it = seen.begin();
    for (int i = 0 ; i < nliterals ; i++) {
      literals[i] = *it;
      ++it;
    }
    
    return new LDSBChoice<Val>(*this,a,choicePos,choiceVal, literals, nliterals);
  }
  

  template<class View, int n, class Val, unsigned int a>
  const Choice*
  LDSBBrancher<View,n,Val,a>::choice(const Space& home, Archive& e) {
    (void) home;
    int p; e >> p;
    Val v; e >> v;
    int nliterals; e >> nliterals;
    Literal* literals = new Literal[nliterals];
    for (int i = 0 ; i < nliterals ; i++) {
      e >> literals[i].variable;
      e >> literals[i].value;
    }
    return new LDSBChoice<Val>(*this,a,p,v, literals, nliterals);
  }

  template<class View>
  ModEvent prune(Space& home, View x, int v);

  template<class View, int n, class Val, unsigned int a>
  ExecStatus
  LDSBBrancher<View,n,Val,a>
  ::commit(Space& home, const Choice& c, unsigned int b) {
    const LDSBChoice<Val>& pvc
      = static_cast<const LDSBChoice<Val>&>(c);
    int choicePos = pvc.pos().pos;
    int choiceVal = pvc.val();

    if (b == 0) {
      // Post the branching constraint.
      ExecStatus fromBase = ViewValBrancher<View,n,Val,a>::commit(home, c, b);
      GECODE_ES_CHECK(fromBase);
      for (int i = 0 ; i < this->_nsyms ; i++)
        this->_syms[i]->update(Literal(choicePos, choiceVal));
    } else if (b == 1) {
      // Post the branching constraint.
      ExecStatus fromBase = ViewValBrancher<View,n,Val,a>::commit(home, c, b);
      GECODE_ES_CHECK(fromBase);
      
      // Post prunings.
      int nliterals = pvc.nliterals();
      const Literal* literals = pvc.literals();
      for (int i = 0 ; i < nliterals ; i++) {
        const Literal& l = literals[i];
        ModEvent me = prune<View>(home, this->x[l.variable], l.value);
        GECODE_ME_CHECK(me);
      }
    }

    return ES_OK;
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline size_t
  LDSBBrancher<View,n,Val,a>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    (void) ViewValBrancher<View,n,Val,a>::dispose(home);
    return sizeof(LDSBBrancher<View,n,Val,a>);
  }

}

// STATISTICS: kernel-branch
