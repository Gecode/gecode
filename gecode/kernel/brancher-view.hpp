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

  /**
   * \brief View selection class for view with smallest degree
   */
  template<class View>
  class ViewSelDegreeMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest degree
    unsigned int degree;
  public:
    /// Default constructor
    ViewSelDegreeMin(void);
    /// Constructor for initialization
    ViewSelDegreeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
  };

  /**
   * \brief View selection class for view with largest degree
   */
  template<class View>
  class ViewSelDegreeMax : public ViewSelBase<View> {
  protected:
    /// So-far largest degree
    unsigned int degree;
  public:
    /// Default constructor
    ViewSelDegreeMax(void);
    /// Constructor for initialization
    ViewSelDegreeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
  };

  /**
   * \brief View selection class for view with smallest accumulated failure count
   */
  template<class View>
  class ViewSelAfcMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest afc
    double afc;
  public:
    /// Default constructor
    ViewSelAfcMin(void);
    /// Constructor for initialization
    ViewSelAfcMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
  };

  /**
   * \brief View selection class for view with largest accumulated failure count
   */
  template<class View>
  class ViewSelAfcMax : public ViewSelBase<View> {
  protected:
    /// So-far largest afc
    double afc;
  public:
    /// Default constructor
    ViewSelAfcMax(void);
    /// Constructor for initialization
    ViewSelAfcMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
  };

  /**
   * \brief View selection class for view with lowest activity
   */
  template<class View>
  class ViewSelActivityMin : public ViewSelBase<View> {
  protected:
    /// Activity information
    Activity activity;
    /// So-far lowest activity
    double a;
  public:
    /// Default constructor
    ViewSelActivityMin(void);
    /// Constructor for initialization
    ViewSelActivityMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelActivityMin& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief View selection class for view with highest activity
   */
  template<class View>
  class ViewSelActivityMax : public ViewSelBase<View> {
  protected:
    /// Activity information
    Activity activity;
    /// So-far highest activity
    double a;
  public:
    /// Default constructor
    ViewSelActivityMax(void);
    /// Constructor for initialization
    ViewSelActivityMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelActivityMax& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
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


  // Select variable with smallest degree
  template<class View>
  forceinline
  ViewSelDegreeMin<View>::ViewSelDegreeMin(void) : degree(0U) {}
  template<class View>
  forceinline
  ViewSelDegreeMin<View>::ViewSelDegreeMin(Space& home,
                                           const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), degree(0U) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMin<View>::init(Space&, View x, int) {
    degree = x.degree();
    return (degree == 0) ? VSS_BEST : VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMin<View>::select(Space&, View x, int) {
    if (x.degree() < degree) {
      degree = x.degree();
      return (degree == 0) ? VSS_BEST : VSS_BETTER;
    } else if (x.degree() > degree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with largest degree
  template<class View>
  forceinline
  ViewSelDegreeMax<View>::ViewSelDegreeMax(void) : degree(0) {}
  template<class View>
  forceinline
  ViewSelDegreeMax<View>::ViewSelDegreeMax(Space& home,
                                           const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), degree(0U) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMax<View>::init(Space&, View x, int) {
    degree = x.degree();
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMax<View>::select(Space&, View x, int) {
    if (x.degree() > degree) {
      degree = x.degree();
      return VSS_BETTER;
    } else if (x.degree() < degree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with smallest afc
  template<class View>
  forceinline
  ViewSelAfcMin<View>::ViewSelAfcMin(void) : afc(0.0) {}
  template<class View>
  forceinline
  ViewSelAfcMin<View>::ViewSelAfcMin(Space& home,
                                     const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), afc(0.0) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelAfcMin<View>::init(Space&, View x, int) {
    afc = x.afc();
    return (afc == 0.0) ? VSS_BEST : VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelAfcMin<View>::select(Space&, View x, int) {
    if (x.afc() < afc) {
      afc = x.afc();
      return (afc == 0.0) ? VSS_BEST : VSS_BETTER;
    } else if (x.afc() > afc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with largest afc
  template<class View>
  forceinline
  ViewSelAfcMax<View>::ViewSelAfcMax(void) : afc(0.0) {}
  template<class View>
  forceinline
  ViewSelAfcMax<View>::ViewSelAfcMax(Space& home,
                                     const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), afc(0.0) {}
  template<class View>
  forceinline ViewSelStatus
  ViewSelAfcMax<View>::init(Space&, View x, int) {
    afc = x.afc();
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelAfcMax<View>::select(Space&, View x, int) {
    double xafc = x.afc();
    if (xafc > afc) {
      afc = xafc;
      return VSS_BETTER;
    } else if (xafc < afc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with lowest activity
  template<class View>
  forceinline
  ViewSelActivityMin<View>::ViewSelActivityMin(void) : a(0.0) {}
  template<class View>
  forceinline
  ViewSelActivityMin<View>::ViewSelActivityMin(Space& home,
                                               const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), activity(vbo.activity), a(0.0) {
    if (!activity.initialized())
      throw MissingActivity("ViewSelActivityMin (VAR_ACTIVITY_MIN)");
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelActivityMin<View>::init(Space&, View, int i) {
    a = activity[i];
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelActivityMin<View>::select(Space&, View, int i) {
    if (activity[i] < a) {
      a = activity[i];
      return VSS_BETTER;
    } else if (activity[i] > a) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  template<class View>
  forceinline void
  ViewSelActivityMin<View>::update(Space& home, bool share, 
                                   ViewSelActivityMin<View>& vs) {
    activity.update(home, share, vs.activity);
  }
  template<class View>
  forceinline bool
  ViewSelActivityMin<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  ViewSelActivityMin<View>::dispose(Space&) {
    activity.~Activity();
  }

  // Select variable with highest activity
  template<class View>
  forceinline
  ViewSelActivityMax<View>::ViewSelActivityMax(void) : a(0.0) {}
  template<class View>
  forceinline
  ViewSelActivityMax<View>::ViewSelActivityMax(Space& home,
                                               const VarBranchOptions& vbo)
    : ViewSelBase<View>(home,vbo), activity(vbo.activity), a(0.0) {
    if (!activity.initialized())
      throw MissingActivity("ViewSelActivityMax (VAR_ACTIVITY_MAX)");
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelActivityMax<View>::init(Space&, View, int i) {
    a = activity[i];
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelActivityMax<View>::select(Space&, View, int i) {
    if (activity[i] > a) {
      a = activity[i];
      return VSS_BETTER;
    } else if (activity[i] < a) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  template<class View>
  forceinline void
  ViewSelActivityMax<View>::update(Space& home, bool share, 
                                   ViewSelActivityMax<View>& vs) {
    activity.update(home, share, vs.activity);
  }
  template<class View>
  forceinline bool
  ViewSelActivityMax<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  ViewSelActivityMax<View>::dispose(Space&) {
    activity.~Activity();
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

}

// STATISTICS: kernel-branch
