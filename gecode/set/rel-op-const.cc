/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004, 2005
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
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"

namespace Gecode {
  using namespace Gecode::Set;
  using namespace Gecode::Set::Rel;
  using namespace Gecode::Set::RelOp;

  void
  rel(Space* home, const IntSet& x, SetOpType op, SetVar y, SetRelType r,
      SetVar z) {
    Set::Limits::check(x, "Set::rel");
    ConstantView xv(home, x);
    rel_op_post<ConstantView,SetView,SetView>(home, xv, op, y, r, z);
  }  

  void
  rel(Space* home, SetVar x, SetOpType op, const IntSet& y, SetRelType r,
      SetVar z) {
    Set::Limits::check(y, "Set::rel");
    ConstantView yv(home, y);

    if (op==SOT_MINUS) {
      switch(r) {
      case SRT_EQ:
        {
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,SetView>
                          ::post(home,cy,x,z)));
        }
        break;
      case SRT_NQ:
        {
          SetVar tmp(home);
          GECODE_ES_FAIL(home,
                         (Distinct<SetView,SetView>
                          ::post(home,z,tmp)));
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,SetView>
                          ::post(home,cy,x,tmp)));
        }
        break;
      case SRT_SUB:
        {
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (SuperOfInter<ConstantView,SetView,SetView>
                          ::post(home,cy,x,z)));

        }
        break;
      case SRT_SUP:
        {
          SetVar tmp(home);
          GECODE_ES_FAIL(home,
                         (SubSet<SetView,SetView>::post(home,z,tmp)));
          
          SetView xv(x);
          ComplementView<SetView> cx(xv);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          ComplementView<SetView>,SetView>
                          ::post(home,yv,cx,tmp)));
        }
        break;
      case SRT_DISJ:
        {
          SetVar tmp(home);
          EmptyView emptyset;
          GECODE_ES_FAIL(home,(SuperOfInter<SetView,SetView,EmptyView>
                               ::post(home, z, tmp, emptyset)));

          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,SetView>
                          ::post(home,cy,x,tmp)));
        }
        break;
      case SRT_CMPL:
        {
          SetView xv(x);
          ComplementView<SetView> cx(xv);
          GECODE_ES_FAIL(home,
                         (Union<ConstantView,
                          ComplementView<SetView>,
                          SetView>::post(home, yv, cx, z)));
        }
        break;
      }
    } else {
      rel_op_post<ConstantView,SetView,SetView>(home, yv, op, x, r, z);
    }
  }  

  void
  rel(Space* home, SetVar x, SetOpType op, SetVar y, SetRelType r,
      const IntSet& z) {
    Set::Limits::check(z, "Set::rel");
    if (r == SRT_CMPL) {
      IntSetRanges zr(z);
      RangesCompl<IntSetRanges> zrc(zr);
      IntSet zc(zrc);
      ConstantView cz(home, zc);
      rel_eq<SetView,SetView,ConstantView>(home, x, op, y, cz);
    } else {
      ConstantView zv(home, z);
      rel_op_post_nocompl<SetView,SetView,ConstantView>(home, x, op, y, r, zv);
    }
  }  

  void
  rel(Space* home, const IntSet& x, SetOpType op, SetVar y, SetRelType r,
      const IntSet& z) {
    Set::Limits::check(x, "Set::rel");
    Set::Limits::check(z, "Set::rel");
    ConstantView xv(home, x);
    if (r == SRT_CMPL) {
      IntSetRanges zr(z);
      RangesCompl<IntSetRanges> zrc(zr);
      IntSet zc(zrc);
      ConstantView cz(home, zc);
      rel_eq<ConstantView,SetView,ConstantView>(home, xv, op, y, cz);
    } else {
      ConstantView zv(home, z);
      rel_op_post_nocompl<ConstantView,SetView,ConstantView>(home, xv, op, y, r, zv);
    }
  }  

  void
  rel(Space* home, SetVar x, SetOpType op, const IntSet& y, SetRelType r,
      const IntSet& z) {
    Set::Limits::check(y, "Set::rel");
    Set::Limits::check(z, "Set::rel");
    ConstantView yv(home, y);
    ConstantView zv(home, z);

    if (op==SOT_MINUS) {
      switch(r) {
      case SRT_EQ:
        {
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,ConstantView>
                          ::post(home,cy,x,zv)));
        }
        break;
      case SRT_NQ:
        {
          SetVar tmp(home);
          GECODE_ES_FAIL(home,
                         (Distinct<SetView,ConstantView>
                          ::post(home,tmp,zv)));
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,SetView>
                          ::post(home,cy,x,tmp)));
        }
        break;
      case SRT_SUB:
        {
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (SuperOfInter<ConstantView,SetView,ConstantView>
                          ::post(home,cy,x,zv)));

        }
        break;
      case SRT_SUP:
        {
          // z <= tmp
          SetVar tmp(home,z,Limits::int_min, Limits::int_max);
          SetView xv(x);
          ComplementView<SetView> cx(xv);
          
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          ComplementView<SetView>,SetView>
                          ::post(home,yv,cx,tmp)));
        }
        break;
      case SRT_DISJ:
        {
          SetVar tmp(home);
          SetView tmpv(tmp);
          IntSetRanges zi(z);
          GECODE_ME_FAIL(home, tmpv.excludeI(home, zi));
          
          GlbRanges<ConstantView> yr(yv);
          RangesCompl<GlbRanges<ConstantView> > yrc(yr);
          IntSet yc(yrc);
          ConstantView cy(home, yc);
          GECODE_ES_FAIL(home,
                         (Intersection<ConstantView,
                          SetView,SetView>
                          ::post(home,cy,x,tmp)));
        }
        break;
      case SRT_CMPL:
        {
          SetView xv(x);
          ComplementView<SetView> cx(xv);
          GECODE_ES_FAIL(home,
                         (Union<ConstantView,
                          ComplementView<SetView>,
                          ConstantView>::post(home, yv, cx, zv)));
        }
        break;
      }
    } else {
      if (r == SRT_CMPL) {
        IntSetRanges zr(z);
        RangesCompl<IntSetRanges> zrc(zr);
        IntSet zc(zrc);
        ConstantView cz(home, zc);
        rel_eq<ConstantView,SetView,ConstantView>(home, yv, op, x, cz);
      } else {
        rel_op_post_nocompl<ConstantView,SetView,ConstantView>(home, yv, op, x, r, zv);
      }
    }
  }  

  namespace {

    GECODE_REGISTER3(RelOp::Union<ConstantView, ComplementView<SetView>, ConstantView>);
    GECODE_REGISTER3(RelOp::Union<ConstantView, ComplementView<SetView>, SetView>);
    GECODE_REGISTER3(RelOp::Union<ConstantView, SetView, ComplementView<SetView> >);
    GECODE_REGISTER3(RelOp::Union<ConstantView, SetView, ConstantView>);
    GECODE_REGISTER3(RelOp::Union<ConstantView, SetView, SetView>);
    GECODE_REGISTER3(RelOp::Union<SetView, SetView, ConstantView>);

    GECODE_REGISTER3(RelOp::Intersection<ConstantView, ComplementView<SetView>, ConstantView>);
    GECODE_REGISTER3(RelOp::Intersection<ConstantView, ComplementView<SetView>, SetView>);
    GECODE_REGISTER3(RelOp::Intersection<ConstantView, ComplementView<SetView>, ComplementView<SetView> >);
    GECODE_REGISTER3(RelOp::Intersection<ConstantView, SetView, ComplementView<SetView> >);
    GECODE_REGISTER3(RelOp::Intersection<ConstantView, SetView, ConstantView>);
    GECODE_REGISTER3(RelOp::Intersection<ConstantView, SetView, SetView>);
    GECODE_REGISTER3(RelOp::Intersection<SetView, ComplementView<SetView>, ConstantView>);
    GECODE_REGISTER3(RelOp::Intersection<SetView, SetView, ConstantView>);

    GECODE_REGISTER3(RelOp::SubOfUnion<ConstantView, SetView, ConstantView>);
    GECODE_REGISTER3(RelOp::SubOfUnion<ConstantView, SetView, SetView>);
    GECODE_REGISTER3(RelOp::SubOfUnion<SetView, SetView, ConstantView>);

    GECODE_REGISTER3(RelOp::SuperOfInter<ConstantView, ComplementView<SetView>, ConstantView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<ConstantView, ComplementView<SetView>, SetView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<ConstantView, SetView, EmptyView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<ConstantView, SetView, ConstantView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<ConstantView, SetView, SetView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<SetView, ComplementView<SetView>, ConstantView>);
    GECODE_REGISTER3(RelOp::SuperOfInter<SetView, SetView, ConstantView>);
  }

}

// STATISTICS: set-post
