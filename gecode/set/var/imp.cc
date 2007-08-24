/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include "gecode/set.hh"

#include "gecode/set/var/imp-body.icc"

namespace Gecode { namespace Set {

  /*
   * "Standard" tell operations
   *
   */
  ModEvent
  SetVarImp::cardMin_full(Space* home) {
    ModEvent me = ME_SET_CARD;
    if (_cardMin == lub.size()) {
      glb.become(home, lub);
      me = ME_SET_VAL;
    }
    SetDelta d;
    notify(home, me, &d);
    return me;
  }

  ModEvent
  SetVarImp::cardMax_full(Space* home) {
    ModEvent me = ME_SET_CARD;
    if (_cardMax == glb.size()) {
      lub.become(home, glb);
      me = ME_SET_VAL;
    }
    SetDelta d;
    notify(home, me, &d);
    return me;
  }

  ModEvent
  SetVarImp::processLubChange(Space* home, SetDelta& d) {
    ModEvent me = ME_SET_LUB;
    if (_cardMax > lub.size()) {
      _cardMax = lub.size();
      if (_cardMin > _cardMax) {
        glb.become(home, lub);
        _cardMin = glb.size();
        _cardMax = glb.size();
        return ME_SET_FAILED;
      }
      me = ME_SET_CLUB;
    }
    if (_cardMax == lub.size() && _cardMin == _cardMax) {
      glb.become(home, lub);
      me = ME_SET_VAL;
      d._glbMin = glb.min();
      d._glbMax = glb.max();
    }
    notify(home, me, &d);
    return me;
  }

  ModEvent
  SetVarImp::processGlbChange(Space* home, SetDelta& d) {
    ModEvent me = ME_SET_GLB;
    if (_cardMin < glb.size()) {
      _cardMin = glb.size();
      if (_cardMin > _cardMax) {
        glb.become(home, lub);
        _cardMin = glb.size();
        _cardMax = glb.size();
        return ME_SET_FAILED;
      }
      me = ME_SET_CGLB;
    }
    if (_cardMin == glb.size() && _cardMin == _cardMax) {
      lub.become(home, glb);
      me = ME_SET_VAL;
      d._lubMin = lub.min();
      d._lubMax = lub.max();
    }
    notify(home, me, &d);
    return me;
  }

  /*
   * Copying variables
   *
   */

  forceinline
  SetVarImp::SetVarImp(Space* home, bool share, SetVarImp& x)
    : SetVarImpBase(home,share,x),
      _cardMin(x._cardMin), _cardMax(x._cardMax) {
    lub.update(home, x.lub);
    if (x.assigned()) {
      glb.become(home,lub);
    } else {
      glb.update(home,x.glb);
    }
  }


  SetVarImp*
  SetVarImp::perform_copy(Space* home, bool share) {
    return new (home) SetVarImp(home,share,*this);
  }

  Reflection::Arg*
  SetVarImp::spec(Space* home, Reflection::VarMap& m) {
    int specIndex = m.index(this);
    if (specIndex != -1)
      return new Reflection::VarArg(specIndex);

    int glbsize = 0;
    {
      BndSetRanges lrs(glb);
      while (lrs()) { glbsize++; ++lrs; }
    }
    
    Reflection::IntArrayArg* glbdom = new Reflection::IntArrayArg(glbsize*2);
    BndSetRanges lr(glb);
    for (int count=0; lr(); ++lr) {
      (*glbdom)[count++] = lr.min();
      (*glbdom)[count++] = lr.max();
    }

    int lubsize = 0;
    {
      BndSetRanges urs(lub);
      while (urs()) { lubsize++; ++urs; }
    }
    
    BndSetRanges ur(lub);
    Reflection::IntArrayArg* lubdom = new Reflection::IntArrayArg(lubsize*2);
    for (int count=0; ur(); ++ur) {
      (*lubdom)[count++] = ur.min();
      (*lubdom)[count++] = ur.max();
    }

    Reflection::PairArg* pair =
      new Reflection::PairArg(new Reflection::PairArg(glbdom, new Reflection::IntArg(_cardMin)),
                        new Reflection::PairArg(lubdom, new Reflection::IntArg(_cardMax)));

    Reflection::VarSpec* spec =
      new Reflection::VarSpec(Support::String("VTI_SET"), pair);
    return (new Reflection::VarArg(m.put(this, spec)));
  }

}}

// STATISTICS: set-var

