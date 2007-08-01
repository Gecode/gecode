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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set.hh"
#include "gecode/iter.hh"
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"

namespace Gecode {
  using namespace Gecode::Set;
  using namespace Gecode::Set::Rel;
  using namespace Gecode::Set::RelOp;

  void
  rel(Space* home, const IntSet& x, SetOpType op, SetVar y, SetRelType r,
      SetVar z) {
    ConstantView xv(home, x);
    rel_op_post<ConstantView,SetView,SetView>(home, xv, op, y, r, z);
  }  

  void
  rel(Space* home, SetVar x, SetOpType op, const IntSet& y, SetRelType r,
      SetVar z) {
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
          SetVar tmp(home,z,Limits::Set::int_min, Limits::Set::int_max);
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

}

// STATISTICS: set-post
