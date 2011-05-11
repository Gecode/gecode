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
   * \defgroup TaskBranchValSel Generic value selection for brancher based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /// Emty value selection choice
  class EmptyValSelChoice {
  public:
    /// Report size occupied
    size_t size(void) const;
    /// Archive into \a e
    void archive(Archive& e) const;
  };

  /**
   * \brief Base class for value selection
   *
   * Defines the brancher to be binary.
   */
  template<class _View, class _Val>
  class ValSelBase {
  public:
    /// View type
    typedef _View View;
    /// Value type
    typedef _Val Val;
    /// Choice type
    typedef EmptyValSelChoice Choice;
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// Default constructor
    ValSelBase(void);
    /// Constructor for initialization
    ValSelBase(Space& home, const ValBranchOptions& vbo);
    /// Return choice
    EmptyValSelChoice choice(Space& home);
    /// Return choice
    EmptyValSelChoice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const EmptyValSelChoice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ValSelBase& vs);
    /// Delete value selection
    void dispose(Space& home);
  };
  //@}


  // Empty value selection choice
  forceinline size_t
  EmptyValSelChoice::size(void) const {
    return sizeof(EmptyValSelChoice);
  }
  forceinline void
  EmptyValSelChoice::archive(Archive& e) const { (void)e; }

  // Value selection base class
  template<class View, class Val>
  forceinline
  ValSelBase<View,Val>::ValSelBase(void) {}
  template<class View, class Val>
  forceinline
  ValSelBase<View,Val>::ValSelBase(Space&, const ValBranchOptions&) {}
  template<class View, class Val>
  forceinline EmptyValSelChoice
  ValSelBase<View,Val>::choice(Space&) {
    EmptyValSelChoice c; return c;
  }
  template<class View, class Val>
  forceinline EmptyValSelChoice
  ValSelBase<View,Val>::choice(const Space&, Archive&) {
    EmptyValSelChoice c; return c;
  }
  template<class View, class Val>
  forceinline void
  ValSelBase<View,Val>::commit(Space&, const EmptyValSelChoice&, unsigned int) {}
  template<class View, class Val>
  forceinline void
  ValSelBase<View,Val>::update(Space&, bool, ValSelBase<View,Val>&) {}
  template<class View, class Val>
  forceinline void
  ValSelBase<View,Val>::dispose(Space&) {}

}

// STATISTICS: kernel-branch
