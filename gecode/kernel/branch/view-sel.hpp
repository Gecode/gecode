/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

  /**
   * \defgroup TaskBranchViewSel Generic view selection for brancher based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /// Abstract class for view selection
  template<class _View>
  class ViewSel {
  public:
    /// Define the view type
    typedef _View View;
    /// The corresponding variable type
    typedef typename View::VarType Var;
    /// \name Initialization
    //@{
    /// Constructor for creation
    ViewSel(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSel(Space& home, ViewSel<View>& vs);
    //@}
    /// \name View selection and tie breaking
    //@{
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s) = 0;
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s,
                       BrancherFilter<View>& f) = 0;
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s,
                       BrancherNoFilter<View>& f);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n) = 0;
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherFilter<View>& f) = 0;
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherNoFilter<View>& f);
    /// Break ties in \a x and update to new ties
    virtual void brk(Space& home, ViewArray<View>& x,
                     int* ties, int& n) = 0;
    /// Select a view from \a x considering views with positions in \a ties
    virtual int select(Space& home, ViewArray<View>& x,
                       int* ties, int n) = 0;
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home) = 0;
    /// Whether dispose must always be called (that is, notice is needed)
    virtual bool notice(void) const;
    /// Dispose view selection
    virtual void dispose(Space& home);
    /// Unused destructor
    virtual ~ViewSel(void);
    //@}
    /// \name Memory management
    //@{
    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// Return memory to space
    static void operator delete(void* p, Space& home);
    /// Needed for exceptions
    static void operator delete(void* p);
    //@}
  };

  /// Select the first unassigned view
  template<class View>
  class ViewSelNone : public ViewSel<View> {
  protected:
    typedef typename ViewSel<View>::Var Var;
  public:
    /// \name Initialization
    //@{
    /// Constructor for creation
    ViewSelNone(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelNone(Space& home, ViewSelNone<View>& vs);
    //@}
    /// \name View selection and tie breaking
    //@{
    /// Select a view from \a x starting at \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s);
    /// Select a view from \a x starting at \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s,
                       BrancherFilter<View>& f);
    /// Select ties from \a x starting at \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n);
    /// Select ties from \a x starting at \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherFilter<View>& f);
    /// Break ties in \a x and update to new ties
    virtual void brk(Space& home, ViewArray<View>& x,
                     int* ties, int& n);
    /// Select a view from \a x considering view with positions in \a ties
    virtual int select(Space& home, ViewArray<View>& x, int* ties, int n);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };

  /// Select a view randomly
  template<class View>
  class ViewSelRnd : public ViewSel<View> {
  protected:
    typedef typename ViewSel<View>::Var Var;
    /// The random number generator used
    Rnd r;
  public:
    /// \name Initialization
    //@{
    /// Constructor for creation
    ViewSelRnd(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelRnd(Space& home, ViewSelRnd<View>& vs);
    //@}
    /// \name View selection and tie breaking
    //@{
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s);
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s,
                       BrancherFilter<View>& f);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherFilter<View>& f);
    /// Break ties in \a x and update to new ties
    virtual void brk(Space& home, ViewArray<View>& x, int* ties, int& n);
    /// Select a view from \a x considering view with positions in \a ties
    virtual int select(Space& home, ViewArray<View>& x, int* ties, int n);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };

  /// Choose views with smaller merit values
  class ChooseMin {
  public:
    /// Return true if \a a is better than \a b
    template<class Val>
    bool operator ()(Val a, Val b) const;
  };

  /// Choose views with larger merit values
  class ChooseMax {
  public:
    /// Return true if \a a is better than \a b
    template<class Val>
    bool operator ()(Val a, Val b) const;
  };

  /// Choose view according to merit
  template<class Choose, class Merit>
  class ViewSelChoose : public ViewSel<typename Merit::View> {
  protected:
    typedef typename ViewSel<typename Merit::View>::Var Var;
    typedef typename ViewSel<typename Merit::View>::View View;
    /// Type of merit
    typedef typename Merit::Val Val;
    /// How to choose
    Choose c;
    /// The merit object used
    Merit m;
  public:
    /// \name Initialization
    //@{
    /// Constructor for creation
    ViewSelChoose(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelChoose(Space& home, ViewSelChoose<Choose,Merit>& vs);
    //@}
    /// \name View selection and tie breaking
    //@{
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s);
    /// Select a view from \a x starting from \a s and return its position
    virtual int select(Space& home, ViewArray<View>& x, int s,
                       BrancherFilter<View>& f);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherFilter<View>& f);
    /// Break ties in \a x and update to new ties
    virtual void brk(Space& home, ViewArray<View>& x, int* ties, int& n);
    /// Select a view from \a x considering views with positions in \a ties
    virtual int select(Space& home, ViewArray<View>& x, int* ties, int n);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Whether dispose must always be called (that is, notice is needed)
    virtual bool notice(void) const;
    /// Delete view selection
    virtual void dispose(Space& home);
    //@}
  };


  /// Choose view according to merit taking tie-break limit into account
  template<class Choose, class Merit>
  class ViewSelChooseTbl : public ViewSelChoose<Choose,Merit> {
  protected:
    typedef typename ViewSelChoose<Choose,Merit>::Val Val;
    typedef typename ViewSelChoose<Choose,Merit>::View View;
    typedef typename ViewSelChoose<Choose,Merit>::Var Var;
    using ViewSelChoose<Choose,Merit>::c;
    using ViewSelChoose<Choose,Merit>::m;
    /// Tie-break limit function
    SharedData<BranchTbl> tbl;
  public:
    /// \name Initialization
    //@{
    /// Constructor for initialization
    ViewSelChooseTbl(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelChooseTbl(Space& home, ViewSelChooseTbl<Choose,Merit>& vs);
    //@}
    /// \name View selection and tie breaking
    //@{
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n);
    /// Select ties from \a x starting from \a s
    virtual void ties(Space& home, ViewArray<View>& x, int s,
                      int* ties, int& n,
                      BrancherFilter<View>& f);
    /// Break ties in \a x and update to new ties
    virtual void brk(Space& home, ViewArray<View>& x, int* ties, int& n);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Whether dispose must always be called (that is, notice is needed)
    virtual bool notice(void) const;
    /// Delete view selection
    virtual void dispose(Space& home);
    //@}
  };

  /// Select view with least merit
  template<class Merit>
  class ViewSelMin : public ViewSelChoose<ChooseMin,Merit> {
    typedef typename ViewSelChoose<ChooseMin,Merit>::View View;
    typedef typename ViewSelChoose<ChooseMin,Merit>::Var Var;
  public:
    /// \name Initialization
    //@{
    /// Constructor for initialization
    ViewSelMin(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelMin(Space& home, ViewSelMin<Merit>& vs);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };

  /// Select view with least merit taking tie-break limit into account
  template<class Merit>
  class ViewSelMinTbl : public ViewSelChooseTbl<ChooseMin,Merit> {
    typedef typename ViewSelChooseTbl<ChooseMin,Merit>::View View;
    typedef typename ViewSelChooseTbl<ChooseMin,Merit>::Var Var;
  public:
    /// \name Initialization
    //@{
    /// Constructor for initialization
    ViewSelMinTbl(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelMinTbl(Space& home, ViewSelMinTbl<Merit>& vs);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };

  /// Select view with largest merit
  template<class Merit>
  class ViewSelMax : public ViewSelChoose<ChooseMax,Merit> {
    typedef typename ViewSelChoose<ChooseMax,Merit>::View View;
    typedef typename ViewSelChoose<ChooseMax,Merit>::Var Var;
  public:
    /// \name Initialization
    //@{
    /// Constructor for initialization
    ViewSelMax(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelMax(Space& home, ViewSelMax<Merit>& vs);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };

  /// Select view with largest merit taking tie-break limit into account
  template<class Merit>
  class ViewSelMaxTbl : public ViewSelChooseTbl<ChooseMax,Merit> {
    typedef typename ViewSelChooseTbl<ChooseMax,Merit>::View View;
    typedef typename ViewSelChooseTbl<ChooseMax,Merit>::Var Var;
  public:
    /// \name Initialization
    //@{
    /// Constructor for initialization
    ViewSelMaxTbl(Space& home, const VarBranch<Var>& vb);
    /// Constructor for copying during cloning
    ViewSelMaxTbl(Space& home, ViewSelMaxTbl<Merit>& vs);
    //@}
    /// \name Resource management and cloning
    //@{
    /// Create copy during cloning
    virtual ViewSel<View>* copy(Space& home);
    //@}
  };
  //@}


  template<class View>
  forceinline
  ViewSel<View>::ViewSel(Space&, const VarBranch<Var>&) {}
  template<class View>
  forceinline
  ViewSel<View>::ViewSel(Space&, ViewSel<View>&) {}
  template<class View>
  int
  ViewSel<View>::select(Space&, ViewArray<View>&, int,
                        BrancherNoFilter<View>&) {
    GECODE_NEVER;
    return 0;
  }
  template<class View>
  void
  ViewSel<View>::ties(Space&, ViewArray<View>&, int,
                      int*, int&,
                      BrancherNoFilter<View>&) {
    GECODE_NEVER;
  }
  template<class View>
  bool
  ViewSel<View>::notice(void) const {
    return false;
  }
  template<class View>
  void
  ViewSel<View>::dispose(Space&) {}
  template<class View>
  ViewSel<View>::~ViewSel(void) {}
  template<class View>
  forceinline void
  ViewSel<View>::operator delete(void*) {}
  template<class View>
  forceinline void
  ViewSel<View>::operator delete(void*, Space&) {}
  template<class View>
  forceinline void*
  ViewSel<View>::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }



  template<class View>
  forceinline
  ViewSelNone<View>::ViewSelNone(Space& home, const VarBranch<Var>& vb)
      : ViewSel<View>(home,vb) {}
  template<class View>
  forceinline
  ViewSelNone<View>::ViewSelNone(Space& home, ViewSelNone<View>& vs)
    : ViewSel<View>(home,vs) {}
  template<class View>
  int
  ViewSelNone<View>::select(Space&, ViewArray<View>&, int s) {
    return s;
  }
  template<class View>
  int
  ViewSelNone<View>::select(Space&, ViewArray<View>&, int s,
                            BrancherFilter<View>&) {
    return s;
  }
  template<class View>
  void
  ViewSelNone<View>::ties(Space&, ViewArray<View>& x, int s,
                          int* ties, int& n) {
    int j=0; ties[j++]=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned())
        ties[j++]=i;
    n=j;
    assert(n > 0);
  }
  template<class View>
  void
  ViewSelNone<View>::ties(Space& home, ViewArray<View>& x, int s,
                          int* ties, int& n,
                          BrancherFilter<View>& f) {
    int j=0; ties[j++]=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned() && f(home,x[i],i))
        ties[j++]=i;
    n=j;
    assert(n > 0);
  }
  template<class View>
  void
  ViewSelNone<View>::brk(Space&, ViewArray<View>&, int*, int&) {
    // Nothing needs to be done
  }
  template<class View>
  int
  ViewSelNone<View>::select(Space&, ViewArray<View>&, int* ties, int) {
    return ties[0];
  }
  template<class View>
  ViewSel<View>*
  ViewSelNone<View>::copy(Space& home) {
    return new (home) ViewSelNone<View>(home,*this);
  }


  template<class View>
  forceinline
  ViewSelRnd<View>::ViewSelRnd(Space& home, const VarBranch<Var>& vb)
      : ViewSel<View>(home,vb), r(vb.rnd()) {}
  template<class View>
  forceinline
  ViewSelRnd<View>::ViewSelRnd(Space& home, ViewSelRnd<View>& vs)
      : ViewSel<View>(home,vs), r(vs.r) {}
  template<class View>
  int
  ViewSelRnd<View>::select(Space&, ViewArray<View>& x, int s) {
    unsigned int n=1;
    int j=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned()) {
        n++;
        if (r(n) == 0U)
          j=i;
      }
    return j;
  }
  template<class View>
  int
  ViewSelRnd<View>::select(Space& home, ViewArray<View>& x, int s,
                           BrancherFilter<View>& f) {
    unsigned int n=1;
    int j=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned() && f(home,x[i],i)) {
        n++;
        if (r(n) == 0U)
          j=i;
      }
    return j;
  }
  template<class View>
  void
  ViewSelRnd<View>::ties(Space& home, ViewArray<View>& x, int s,
                         int* ties, int& n) {
    n=1; ties[0] = select(home,x,s);
  }
  template<class View>
  void
  ViewSelRnd<View>::ties(Space& home, ViewArray<View>& x, int s,
                         int* ties, int& n,
                         BrancherFilter<View>&) {
    n=1; ties[0] = select(home,x,s);
  }
  template<class View>
  void
  ViewSelRnd<View>::brk(Space&, ViewArray<View>&, int* ties, int& n) {
    ties[0] = ties[static_cast<int>(r(static_cast<unsigned int>(n)))];
    n=1;
  }
  template<class View>
  int
  ViewSelRnd<View>::select(Space&, ViewArray<View>&, int* ties, int n) {
    return ties[static_cast<int>(r(static_cast<unsigned int>(n)))];
  }
  template<class View>
  ViewSel<View>*
  ViewSelRnd<View>::copy(Space& home) {
    return new (home) ViewSelRnd<View>(home,*this);
  }


  template<class Val>
  forceinline bool
  ChooseMin::operator ()(Val a, Val b) const {
    return a < b;
  }
  template<class Val>
  forceinline bool
  ChooseMax::operator ()(Val a, Val b) const {
    return a > b;
  }


  template<class Choose, class Merit>
  forceinline
  ViewSelChoose<Choose,Merit>::ViewSelChoose(Space& home, const VarBranch<Var>& vb)
    : ViewSel<View>(home,vb), m(home,vb) {}

  template<class Choose, class Merit>
  forceinline
  ViewSelChoose<Choose,Merit>::ViewSelChoose(Space& home,
                                             ViewSelChoose<Choose,Merit>& vs)
    : ViewSel<View>(home,vs), m(home,vs.m) {}

  template<class Choose, class Merit>
  int
  ViewSelChoose<Choose,Merit>::select(Space& home, ViewArray<View>& x, int s) {
    // Consider x[s] as the so-far best view
    int b_i = s;
    Val b_m = m(home,x[s],s);
    // Scan all non-assigned views from s+1 onwards
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned()) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b_m)) {
          b_i = i; b_m = mxi;
        }
      }
    return b_i;
  }

  template<class Choose, class Merit>
  int
  ViewSelChoose<Choose,Merit>::select(Space& home, ViewArray<View>& x, int s,
                                      BrancherFilter<View>& f) {
    // Consider x[s] as the so-far best view
    int b_i = s;
    Val b_m = m(home,x[s],s);
    // Scan all non-assigned views from s+1 onwards
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned() && f(home,x[i],i)) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b_m)) {
          b_i = i; b_m = mxi;
        }
      }
    return b_i;
  }

  template<class Choose, class Merit>
  void
  ViewSelChoose<Choose,Merit>::ties(Space& home, ViewArray<View>& x, int s,
                                    int* ties, int& n) {
    // Consider x[s] as the so-far best view and record as tie
    Val b = m(home,x[s],s);
    int j=0; ties[j++]=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned()) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b)) {
          // Found a better one, reset all ties and record
          j=0; ties[j++]=i; b=mxi;
        } else if (mxi == b) {
          // Found a tie, record
          ties[j++]=i;
        }
      }
    n=j;
    // There must be at least one tie, of course!
    assert(n > 0);
  }

  template<class Choose, class Merit>
  void
  ViewSelChoose<Choose,Merit>::ties(Space& home, ViewArray<View>& x, int s,
                                    int* ties, int& n,
                                    BrancherFilter<View>& f) {
    // Consider x[s] as the so-far best view and record as tie
    Val b = m(home,x[s],s);
    int j=0; ties[j++]=s;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned() && f(home,x[i],i)) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b)) {
          // Found a better one, reset all ties and record
          j=0; ties[j++]=i; b=mxi;
        } else if (mxi == b) {
          // Found a tie, record
          ties[j++]=i;
        }
      }
    n=j;
    // There must be at least one tie, of course!
    assert(n > 0);
  }

  template<class Choose, class Merit>
  void
  ViewSelChoose<Choose,Merit>::brk(Space& home, ViewArray<View>& x,
                                   int* ties, int& n) {
    // Keep first tie in place
    Val b = m(home,x[ties[0]],ties[0]);
    int j=1;
    // Scan remaining ties
    for (int i=1; i<n; i++) {
      Val mxi = m(home,x[ties[i]],ties[i]);
      if (c(mxi,b)) {
        // Found a better one, reset all ties
        b=mxi; j=0; ties[j++]=ties[i];
      } else if (mxi == b) {
        // Found a tie and record it
        ties[j++]=ties[i];
      }
    }
    n=j;
    // There must be at least one tie, of course!
    assert(n > 0);
  }

  template<class Choose, class Merit>
  int
  ViewSelChoose<Choose,Merit>::select(Space& home, ViewArray<View>& x,
                                      int* ties, int n) {
    int b_i = ties[0];
    Val b_m = m(home,x[ties[0]],ties[0]);
    for (int i=1; i<n; i++) {
      Val mxi = m(home,x[ties[i]],ties[i]);
      if (c(mxi,b_m)) {
        b_i = ties[i]; b_m = mxi;
      }
    }
    return b_i;
  }

  template<class Choose, class Merit>
  bool
  ViewSelChoose<Choose,Merit>::notice(void) const {
    return m.notice();
  }

  template<class Choose, class Merit>
  void
  ViewSelChoose<Choose,Merit>::dispose(Space& home) {
    m.dispose(home);
  }


  template<class Choose, class Merit>
  forceinline
  ViewSelChooseTbl<Choose,Merit>::ViewSelChooseTbl(Space& home,
                                                   const VarBranch<Var>& vb)
    : ViewSelChoose<Choose,Merit>(home,vb), tbl(vb.tbl()) {
    if (!tbl())
      throw InvalidFunction("ViewSelChooseTbl::ViewSelChooseTbl");
  }

  template<class Choose, class Merit>
  forceinline
  ViewSelChooseTbl<Choose,Merit>::ViewSelChooseTbl
  (Space& home, ViewSelChooseTbl<Choose,Merit>& vs)
    : ViewSelChoose<Choose,Merit>(home,vs), tbl(vs.tbl) {
  }

  template<class Choose, class Merit>
  void
  ViewSelChooseTbl<Choose,Merit>::ties(Space& home, ViewArray<View>& x, int s,
                                       int* ties, int& n) {
    // Find the worst and best merit value
    Val w = m(home,x[s],s);
    Val b = w;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned()) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b))
          b=mxi;
        else if (c(w,mxi))
          w=mxi;
      }
    // Compute tie-break limit
    GECODE_VALID_FUNCTION(tbl());
    double l = tbl()(home,static_cast<double>(w),static_cast<double>(b));
    // If the limit is not better than the worst merit, everything is a tie
    if (!c(l,static_cast<double>(w))) {
      int j=0;
      for (int i=s; i<x.size(); i++)
        if (!x[i].assigned())
          ties[j++]=i;
      n=j;
    } else {
      // The limit is not allowed to better than the best merit value
      if (c(l,static_cast<double>(b)))
        l = static_cast<double>(b);
      // Record all ties that are not worse than the limit merit value
      int j=0;
      for (int i=s; i<x.size(); i++)
        if (!x[i].assigned() && !c(l,static_cast<double>(m(home,x[i],i))))
          ties[j++]=i;
      n=j;
    }
    // There will be at least one tie (the best will qualify, of course)
    assert(n > 0);
  }

  template<class Choose, class Merit>
  void
  ViewSelChooseTbl<Choose,Merit>::ties(Space& home, ViewArray<View>& x, int s,
                                       int* ties, int& n,
                                       BrancherFilter<View>& f) {
    // Find the worst and best merit value
    assert(f(home,x[s],s));
    Val w = m(home,x[s],s);
    Val b = w;
    for (int i=s+1; i<x.size(); i++)
      if (!x[i].assigned() && f(home,x[i],i)) {
        Val mxi = m(home,x[i],i);
        if (c(mxi,b))
          b=mxi;
        else if (c(w,mxi))
          w=mxi;
      }
    // Compute tie-break limit
    GECODE_VALID_FUNCTION(tbl());
    double l = tbl()(home,static_cast<double>(w),static_cast<double>(b));
    // If the limit is not better than the worst merit, everything is a tie
    if (!c(l,static_cast<double>(w))) {
      int j=0;
      for (int i=s; i<x.size(); i++)
        if (!x[i].assigned() && f(home,x[i],i))
          ties[j++]=i;
      n=j;
    } else {
      // The limit is not allowed to better than the best merit value
      if (c(l,static_cast<double>(b)))
        l = static_cast<double>(b);
      // Record all ties that are not worse than the limit merit value
      int j=0;
      for (int i=s; i<x.size(); i++)
        if (!x[i].assigned() && f(home,x[i],i) &&
            !c(l,static_cast<double>(m(home,x[i],i))))
          ties[j++]=i;
      n=j;
    }
    // There will be at least one tie (the best will qualify, of course)
    assert(n > 0);
  }

  template<class Choose, class Merit>
  void
  ViewSelChooseTbl<Choose,Merit>::brk(Space& home, ViewArray<View>& x,
                                      int* ties, int& n) {
    // Find the worst and best merit value
    Val w = m(home,x[ties[0]],ties[0]);
    Val b = w;
    for (int i=1; i<n; i++) {
      Val mxi = m(home,x[ties[i]],ties[i]);
      if (c(mxi,b))
        b=mxi;
      else if (c(w,mxi))
        w=mxi;
    }
    // Compute tie-break limit
    GECODE_VALID_FUNCTION(tbl());
    double l = tbl()(home,static_cast<double>(w),static_cast<double>(b));
    // If the limit is not better than the worst merit, everything is a tie
    // and no breaking is required
    if (c(l,static_cast<double>(w))) {
      // The limit is not allowed to better than the best merit value
      if (c(l,static_cast<double>(b)))
        l = static_cast<double>(b);
      // Keep all ties that are not worse than the limit merit value
      int j=0;
      for (int i=0; i<n; i++)
        if (!c(l,static_cast<double>(m(home,x[ties[i]],ties[i]))))
          ties[j++]=ties[i];
      n=j;
    }
    // There will be at least one tie (the best will qualify)
    assert(n > 0);
  }
  template<class Choose, class Merit>
  bool
  ViewSelChooseTbl<Choose,Merit>::notice(void) const {
    return true;
  }
  template<class Choose, class Merit>
  void
  ViewSelChooseTbl<Choose,Merit>::dispose(Space&) {
    tbl.~SharedData<BranchTbl>();
  }



  template<class Merit>
  forceinline
  ViewSelMin<Merit>::ViewSelMin(Space& home, const VarBranch<Var>& vb)
    : ViewSelChoose<ChooseMin,Merit>(home,vb) {}

  template<class Merit>
  forceinline
  ViewSelMin<Merit>::ViewSelMin(Space& home, ViewSelMin<Merit>& vs)
    : ViewSelChoose<ChooseMin,Merit>(home,vs) {}

  template<class Merit>
  ViewSel<typename ViewSelMin<Merit>::View>*
  ViewSelMin<Merit>::copy(Space& home) {
    return new (home) ViewSelMin<Merit>(home,*this);
  }


  template<class Merit>
  forceinline
  ViewSelMinTbl<Merit>::ViewSelMinTbl(Space& home, const VarBranch<Var>& vb)
    : ViewSelChooseTbl<ChooseMin,Merit>(home,vb) {}

  template<class Merit>
  forceinline
  ViewSelMinTbl<Merit>::ViewSelMinTbl(Space& home, ViewSelMinTbl<Merit>& vs)
    : ViewSelChooseTbl<ChooseMin,Merit>(home,vs) {}

  template<class Merit>
  ViewSel<typename ViewSelMinTbl<Merit>::View>*
  ViewSelMinTbl<Merit>::copy(Space& home) {
    return new (home) ViewSelMinTbl<Merit>(home,*this);
  }



  template<class Merit>
  forceinline
  ViewSelMax<Merit>::ViewSelMax(Space& home, const VarBranch<Var>& vb)
    : ViewSelChoose<ChooseMax,Merit>(home,vb) {}

  template<class Merit>
  forceinline
  ViewSelMax<Merit>::ViewSelMax(Space& home, ViewSelMax<Merit>& vs)
    : ViewSelChoose<ChooseMax,Merit>(home,vs) {}

  template<class Merit>
  ViewSel<typename ViewSelMax<Merit>::View>*
  ViewSelMax<Merit>::copy(Space& home) {
    return new (home) ViewSelMax<Merit>(home,*this);
  }



  template<class Merit>
  forceinline
  ViewSelMaxTbl<Merit>::ViewSelMaxTbl(Space& home, const VarBranch<Var>& vb)
    : ViewSelChooseTbl<ChooseMax,Merit>(home,vb) {}

  template<class Merit>
  forceinline
  ViewSelMaxTbl<Merit>::ViewSelMaxTbl(Space& home, ViewSelMaxTbl<Merit>& vs)
    : ViewSelChooseTbl<ChooseMax,Merit>(home,vs) {}

  template<class Merit>
  ViewSel<typename ViewSelMaxTbl<Merit>::View>*
  ViewSelMaxTbl<Merit>::copy(Space& home) {
    return new (home) ViewSelMaxTbl<Merit>(home,*this);
  }



}

// STATISTICS: kernel-branch
