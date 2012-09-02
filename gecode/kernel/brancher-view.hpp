/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

namespace Gecode {

  /**
   * \defgroup TaskBranchViewSel Generic view selection for brancher based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /// Emty view selection choice
  class EmptyViewSelChoice {
  public:
    /// Report size occupied
    size_t size(void) const;
    /// Archive into \a e
    void archive(Archive& e) const;
  };

  /**
   * \brief Base class for view selection
   */
  template<class _View>
  class ViewSelBase {
  public:
    /// View type
    typedef _View View;
    /// View selection choice
    typedef EmptyViewSelChoice Choice;
    /// Default constructor
    ViewSelBase(void);
    /// Constructor for initialization
    ViewSelBase(Space& home, const VarBranchOptions& vbo);
    /// Return choice
    EmptyViewSelChoice choice(Space& home);
    /// Return choice
    EmptyViewSelChoice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const EmptyViewSelChoice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelBase& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view selection
    void dispose(Space& home);
  };

  /**
   * \brief Select first not assigned view
   */
  template<class View>
  class ViewSelNone : public ViewSelBase<View> {
  public:
    /// Default constructor
    ViewSelNone(void);
    /// Constructor for initialization
    ViewSelNone(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
  };

  /// Random generator with archiving (to be used in branchers)
  class ArchivedRandomGenerator : public Support::RandomGenerator {
  public:
    /// Default constructor
    ArchivedRandomGenerator(void);
    /// Constructor
    ArchivedRandomGenerator(unsigned int seed);
    /// Copy constructor
    ArchivedRandomGenerator(const Support::RandomGenerator& r);
    /// Archive
    void archive(Archive& e) const;
  };

  /**
   * \brief View selection class for random selection
   */
  template<class _View>
  class ViewSelRnd {
  protected:
    /// Random number generator
    ArchivedRandomGenerator r;
    /// Number of views considered so far
    unsigned int n;
  public:
    /// View type
    typedef _View View;
    /// View selection choice
    typedef ArchivedRandomGenerator Choice;
    /// Default constructor
    ViewSelRnd(void);
    /// Constructor for initialization
    ViewSelRnd(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, _View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, _View x, int i);
    /// Return choice
    Choice choice(Space& home);
    /// Return choice
    Choice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const Choice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelRnd& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view selection
    void dispose(Space& home);
  };

  /**
   * \brief View selection class for view with least merit
   */
  template<class Merit, class View>
  class ViewSelMin : public ViewSelBase<View> {
  protected:
    /// The merit used
    Merit m;
    /// So-far least merit value
    double sfb;
  public:
    /// Default constructor
    ViewSelMin(void);
    /// Constructor for initialization
    ViewSelMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelMin<Merit,View>& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit
    void dispose(Space& home);
  };

  /**
   * \brief View selection class for view with largest merit
   */
  template<class Merit, class View>
  class ViewSelMax : public ViewSelBase<View> {
  protected:
    /// The merit used
    Merit m;
    /// So-far largest merit value
    double sfb;
  public:
    /// Default constructor
    ViewSelMax(void);
    /// Constructor for initialization
    ViewSelMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelMax<Merit,View>& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit
    void dispose(Space& home);
  };
  //@}

  // Empty view selection choice
  forceinline size_t
  EmptyViewSelChoice::size(void) const {
    return sizeof(EmptyViewSelChoice);
  }

  forceinline void
  EmptyViewSelChoice::archive(Archive& e) const { (void)e; }

  // Selection base class
  template<class View>
  forceinline
  ViewSelBase<View>::ViewSelBase(void) {}
  template<class View>
  forceinline
  ViewSelBase<View>::ViewSelBase(Space&, const VarBranchOptions&) {}
  template<class View>
  forceinline EmptyViewSelChoice
  ViewSelBase<View>::choice(Space&) {
    EmptyViewSelChoice c; return c;
  }
  template<class View>
  forceinline EmptyViewSelChoice
  ViewSelBase<View>::choice(const Space&, Archive&) {
    EmptyViewSelChoice c; return c;
  }
  template<class View>
  forceinline void
  ViewSelBase<View>::commit(Space&, const EmptyViewSelChoice&, unsigned int) {}
  template<class View>
  forceinline void
  ViewSelBase<View>::update(Space&, bool, ViewSelBase<View>&) {}
  template<class View>
  forceinline bool
  ViewSelBase<View>::notice(void) const {
    return false;
  }
  template<class View>
  forceinline void
  ViewSelBase<View>::dispose(Space&) {}

  // Select first view
  template<class View>
  forceinline
  ViewSelNone<View>::ViewSelNone(void) {}
  template<class View>
  forceinline
  ViewSelNone<View>::ViewSelNone(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelNone<View>::init(Space&, View, int) {
    return VSS_BEST;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelNone<View>::select(Space&, View, int) {
    return VSS_BEST;
  }

  // Archived random generator
  forceinline
  ArchivedRandomGenerator::ArchivedRandomGenerator(void) {}
  forceinline
  ArchivedRandomGenerator::ArchivedRandomGenerator(unsigned int seed)
    : Support::RandomGenerator(seed) {}
  forceinline
  ArchivedRandomGenerator::ArchivedRandomGenerator
    (const Support::RandomGenerator& r) : Support::RandomGenerator(r) {}
  forceinline void
  ArchivedRandomGenerator::archive(Archive& e) const { e << seed(); }

  // Select variable by random
  template<class View>
  forceinline
  ViewSelRnd<View>::ViewSelRnd(void) : n(0) {}
  template<class View>
  forceinline
  ViewSelRnd<View>::ViewSelRnd(Space&, const VarBranchOptions& vbo)
    : r(vbo.seed), n(0) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelRnd<View>::init(Space&, View, int) {
    n=1;
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelRnd<View>::select(Space&, View, int) {
    n++;
    return (r(n) == (n-1)) ? VSS_BETTER : VSS_WORSE;
  }
  template<class View>
  forceinline typename ViewSelRnd<View>::Choice
  ViewSelRnd<View>::choice(Space&) {
    return r;
  }
  template<class View>
  forceinline typename ViewSelRnd<View>::Choice
  ViewSelRnd<View>::choice(const Space&, Archive& e) {
    return Choice(e.get());
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::commit(Space&, const Choice& c, unsigned int) {
    r = c;
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::update(Space&, bool, ViewSelRnd<View>& vs) {
    r = vs.r;
  }
  template<class View>
  forceinline bool
  ViewSelRnd<View>::notice(void) const {
    return false;
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::dispose(Space&) {
  }

  // Select variable with least merit
  template<class Merit, class View>
  forceinline
  ViewSelMin<Merit,View>::ViewSelMin(void) 
    : sfb(0.0) {}
  template<class Merit, class View>
  forceinline
  ViewSelMin<Merit,View>::ViewSelMin(Space& home,
                                     const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), m(home,vbo), sfb(0.0) {}
  template<class Merit, class View>
  forceinline ViewSelStatus
  ViewSelMin<Merit,View>::init(Space& home, View x, int i) {
    sfb = m(home,x,i);
    return VSS_BETTER;
  }
  template<class Merit, class View>
  forceinline ViewSelStatus
  ViewSelMin<Merit,View>::select(Space& home, View x, int i) {
    double mxi = m(home,x,i);
    if (mxi < sfb) {
      sfb = mxi;
      return VSS_BETTER;
    } else if (mxi > sfb) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  template<class Merit, class View>
  forceinline void
  ViewSelMin<Merit,View>::update(Space& home, bool share, 
                                 ViewSelMin<Merit,View>& vsm) {
    m.update(home, share, vsm.m);
  }
  template<class Merit, class View>
  forceinline bool
  ViewSelMin<Merit,View>::notice(void) const {
    return m.notice();
  }
  template<class Merit, class View>
  forceinline void
  ViewSelMin<Merit,View>::dispose(Space& home) {
    m.dispose(home);
  }


  // Select variable with largest merit
  template<class Merit, class View>
  forceinline
  ViewSelMax<Merit,View>::ViewSelMax(void) 
    : sfb(0.0) {}
  template<class Merit, class View>
  forceinline
  ViewSelMax<Merit,View>::ViewSelMax(Space& home,
                                     const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), m(home,vbo), sfb(0.0) {}
  template<class Merit, class View>
  forceinline ViewSelStatus
  ViewSelMax<Merit,View>::init(Space& home, View x, int i) {
    sfb = m(home,x,i);
    return VSS_BETTER;
  }
  template<class Merit, class View>
  forceinline ViewSelStatus
  ViewSelMax<Merit,View>::select(Space& home, View x, int i) {
    double mxi = m(home,x,i);
    if (mxi > sfb) {
      sfb = mxi;
      return VSS_BETTER;
    } else if (mxi < sfb) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  template<class Merit, class View>
  forceinline void
  ViewSelMax<Merit,View>::update(Space& home, bool share, 
                                 ViewSelMax<Merit,View>& vsm) {
    m.update(home, share, vsm.m);
  }
  template<class Merit, class View>
  forceinline bool
  ViewSelMax<Merit,View>::notice(void) const {
    return m.notice();
  }
  template<class Merit, class View>
  forceinline void
  ViewSelMax<Merit,View>::dispose(Space& home) {
    m.dispose(home);
  }

}

// STATISTICS: kernel-branch
