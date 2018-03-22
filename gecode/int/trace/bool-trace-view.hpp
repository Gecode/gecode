/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

namespace Gecode { namespace Int {

  /// Duplicate of a Boolean view
  class BoolTraceView {
  public:
    /// Default constructor (initializes with no view)
    BoolTraceView(void);
    /// Duplicate view \a y
    BoolTraceView(Space& home, BoolView y);
    /// Update duplicated view from view \a y and modification delta \a d
    void prune(Space& home, BoolView y, const Delta& d);
    /// Update duplicate view during copying
    void update(Space&home, BoolTraceView x);
    /// Return slack of \a x
    static unsigned int slack(BoolView x);
  };


  forceinline
  BoolTraceView::BoolTraceView(void) {}
  forceinline
  BoolTraceView::BoolTraceView(Space&, BoolView) {}
  forceinline void
  BoolTraceView::prune(Space&, BoolView, const Delta&) {}
  forceinline void
  BoolTraceView::update(Space&, BoolTraceView) {}
  forceinline unsigned int
  BoolTraceView::slack(BoolView x) {
    return x.width()-1;
  }



}}

// STATISTICS: int-trace
