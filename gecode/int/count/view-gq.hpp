/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Int { namespace Count {

  template<class VX, class VY, class VZ, bool shr>
  forceinline
  GqView<VX,VY,VZ,shr>::GqView(Home home, ViewArray<VX>& x, VY y, VZ z, int c)
    : ViewBase<VX,VY,VZ>(home,x,y,z,c) {}

  template<class VX, class VY, class VZ, bool shr>
  ExecStatus
  GqView<VX,VY,VZ,shr>::post(Home home,
                             ViewArray<VX>& x, VY y, VZ z, int c) {
    GECODE_ME_CHECK(z.lq(home,x.size()-c));
    if ((vtd(y) != VTD_VARVIEW) && z.assigned())
      return GqInt<VX,VY>::post(home,x,y,z.val()+c);
    if (sharing(x,y,z))
      (void) new (home) GqView<VX,VY,VZ,true>(home,x,y,z,c);
    else
      (void) new (home) GqView<VX,VY,VZ,false>(home,x,y,z,c);
    return ES_OK;
  }

  template<class VX, class VY, class VZ, bool shr>
  forceinline
  GqView<VX,VY,VZ,shr>::GqView(Space& home, bool share,
                               GqView<VX,VY,VZ,shr>& p)
    : ViewBase<VX,VY,VZ>(home,share,p) {}

  template<class VX, class VY, class VZ, bool shr>
  Actor*
  GqView<VX,VY,VZ,shr>::copy(Space& home, bool share) {
    return new (home) GqView<VX,VY,VZ,shr>(home,share,*this);
  }

  template<class VX, class VY, class VZ, bool shr>
  ExecStatus
  GqView<VX,VY,VZ,shr>::propagate(Space& home, const ModEventDelta&) {
    count(home);

    GECODE_ME_CHECK(z.lq(home,atmost()));

    if (z.min() == atmost()) {
      GECODE_ES_CHECK(post_true(home,x,y)); 
      return home.ES_SUBSUMED(*this);
    }
    if (x.size() == 0)
      return home.ES_SUBSUMED(*this);

    if (z.assigned() && (vtd(y) != VTD_VARVIEW)) {
      VY yc(y);
      GECODE_REWRITE(*this,(GqInt<VX,VY>::post(home(*this),x,yc,z.val()+c)));
    }

    if ((vtd(y) == VTD_VARVIEW) && (z.min() > 0)) {
      /* 
       * Only if the propagator is at fixpoint here, continue
       * when things are shared: the reason is that prune
       * requires that the views in x overlap with y!
       */
      if (shr && (VX::me(Propagator::modeventdelta()) != ME_INT_NONE))
        return ES_NOFIX;

      GECODE_ES_CHECK(prune(home,x,y));

      return ES_NOFIX;
    }
    
    return shr ? ES_NOFIX : ES_FIX;
  }

}}}

// STATISTICS: int-prop
