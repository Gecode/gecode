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
  };

  /**
   * \brief Base class for view selection
   */
  template <class _View>
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
    /// Commit to choice
    void commit(Space& home, const EmptyViewSelChoice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelBase& vs);
    /// Delete view selection
    void dispose(Space& home);
  };

  /**
   * \brief Select first not assigned view
   */
  template <class View>
  class ViewSelNone : public ViewSelBase<View> {
  public:
    /// Default constructor
    ViewSelNone(void);
    /// Constructor for initialization
    ViewSelNone(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, View x);
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
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, View x);
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
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, View x);
  };

  /**
   * \brief View selection class for random selection
   */
  template<class _View>
  class ViewSelRnd {
  protected:
    /// Random number generator
    Support::RandomGenerator r;
    /// Number of views considered so far
    unsigned int n;
  public:
    /// View type
    typedef _View View;
    /// View selection choice
    typedef Support::RandomGenerator Choice;
    /// Default constructor
    ViewSelRnd(void);
    /// Constructor for initialization
    ViewSelRnd(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, _View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, _View x);
    /// Return choice
    Support::RandomGenerator choice(Space& home);
    /// Commit to choice
    void commit(Space& home, const Support::RandomGenerator& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelRnd& vs);
    /// Delete view selection
    void dispose(Space& home);
  };
  //@}

  // Empty view selection choice
  forceinline size_t
  EmptyViewSelChoice::size(void) const {
    return sizeof(EmptyViewSelChoice);
  }

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
  forceinline void
  ViewSelBase<View>::commit(Space&, const EmptyViewSelChoice&, unsigned int) {}
  template<class View>
  forceinline void
  ViewSelBase<View>::update(Space&, bool, ViewSelBase<View>&) {}
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
  ViewSelNone<View>::init(Space&, View) {
    return VSS_BEST;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelNone<View>::select(Space&, View) {
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
  ViewSelDegreeMin<View>::init(Space&, View x) {
    degree = x.degree();
    return (degree == 0) ? VSS_BEST : VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMin<View>::select(Space&, View x) {
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
  ViewSelDegreeMax<View>::init(Space&, View x) {
    degree = x.degree();
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelDegreeMax<View>::select(Space&, View x) {
    if (x.degree() > degree) {
      degree = x.degree();
      return VSS_BETTER;
    } else if (x.degree() < degree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


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
  ViewSelRnd<View>::init(Space&, View) {
    n=1;
    return VSS_BETTER;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelRnd<View>::select(Space&, View) {
    n++;
    return (r(n) == (n-1)) ? VSS_BETTER : VSS_WORSE;
  }
  template<class View>
  forceinline Support::RandomGenerator
  ViewSelRnd<View>::choice(Space&) {
    return r;
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::commit(Space&, const Support::RandomGenerator& c,
                           unsigned int) {
    r = c;
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::update(Space&, bool, ViewSelRnd<View>& vs) {
    r = vs.r;
  }
  template<class View>
  forceinline void
  ViewSelRnd<View>::dispose(Space&) {
  }

}

// STATISTICS: kernel-branch
