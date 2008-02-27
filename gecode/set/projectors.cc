/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#include "gecode/set/projectors/propagator.hh"

using namespace Gecode::Set;

namespace Gecode {

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
                 ProjectorSet& ps, bool negated) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    if (negated) {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<true>::post(home, 
                                                             x, ps)));
    } else {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<false>::post(home, 
                                                              x, ps)));
    }
    
  }

  void projector(Space* home, const SetVarArgs& xa,
                 ProjectorSet& ps, bool negated) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, xa.size());
    for (int i=x.size(); i--;)
      x[i] = xa[i];
    if (negated) {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<true>::post(home, 
                                                             x, ps)));
    } else {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<false>::post(home, 
                                                              x, ps)));
    }
    
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
                 const BoolVar& bv, ProjectorSet& ps) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    Gecode::Int::BoolView b(bv);
    GECODE_ES_FAIL(home,
                   (Projection::ReNaryProjection::post(home, 
                                                       x, b, ps)));
  }

  void projector(Space* home,
                 const SetVar& xa, const SetVar& ya, const SetVar& za,
                 ProjectorSet& ps, bool negated) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    if (negated) {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<true>::post(home, 
                                                             x, ps)));
    } else {
      GECODE_ES_FAIL(home,
                     (Projection::NaryProjection<false>::post(home, 
                                                              x, ps)));
    }
    
  }

  void projector(Space* home,
                 const SetVar& xa, const SetVar& ya, const SetVar& za,
                 const BoolVar& bv, ProjectorSet& ps) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    Gecode::Int::BoolView b(bv);
    GECODE_ES_FAIL(home,
                   (Projection::ReNaryProjection::post(home, 
                                                          x, b, ps)));
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
                 const IntVar& i, Projector& p) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    Gecode::Int::IntView iv(i);
    GECODE_ES_FAIL(home,
                   (Projection::CardProjection::post(home,x,i,p)));
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
                 const SetVar& za, const IntVar& i, Projector& p) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    Gecode::Int::IntView iv(i);
    GECODE_ES_FAIL(home,
                   (Projection::CardProjection::post(home,x,i,p)));
  }
  

}

// STATISTICS: set-post
