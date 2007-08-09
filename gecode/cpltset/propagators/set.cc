/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"

// for set bounds propagators with bdds
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"
#include "gecode/set/int.hh"
#include "gecode/set/convex.hh"
#include "gecode/set/sequence.hh"
#include "gecode/set/distinct.hh"
#include "gecode/set/select.hh"


#include "gecode/cpltset/propagators.hh"



using namespace Gecode::CpltSet;

namespace Gecode {

  // set constraints using bdd variables
  // \todo put trunk template functions into icc files in order to include them here

//   void rel(Space* home, CpltSetVar x, SetRelType r, CpltSetVar y, BoolVar b) {
//     CpltSetView bx(x);
//     CpltSetView by(y);
//     CrdCpltSetView cx(bx);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sx(cx);
//     SetCpltSetView sy(cy);
//     rel_re<SetCpltSetView, SetCpltSetView>(home, sx, r, sy, b);
//   }
  
//   void rel(Space* home, CpltSetVar s, SetRelType r, IntVar x, SetConLevel scl) {
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);
//     Gecode::Int::IntView ix(x);
//     Gecode::Gecode::Set::SingletonView xsingle(ix);
//     Gecode::rel_post<SetCpltSetView, Gecode::Gecode::Set::SingletonView>(home, ss, r, xsingle); 
//   }

//   void rel(Space* home, CpltSetVar s, SetRelType r, IntVar x, BoolVar b, SetConLevel scl) {
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);
//     Gecode::Int::IntView ix(x);
//     Gecode::Gecode::Set::SingletonView xsingle(ix);
//     rel_re<SetCpltSetView, Gecode::Gecode::Set::SingletonView>(home, ss, r, xsingle, b);
//   }

//   void rel(Space* home, IntVar x, SetRelType r, CpltSetVar s, BoolVar b, SetConLevel scl) {
//     switch(r) {
//     case SRT_SUB:
//       rel(home, s, SRT_SUP, x, b, scl);
//       break;
//     case SRT_SUP:
//       rel(home, s, SRT_SUB, x, b, scl);
//       break;
//     default:
//       rel(home, s, r, x, b, scl);
//     }
//   }

//   void rel(Space* home, CpltSetVar s, IntRelType r, IntVar x, SetConLevel scl) {
//     if (home->failed()) return;
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     switch(r) {
//     case IRT_EQ:
//       {
//         Gecode::Int::IntView xv(x);
//         Gecode::Set::SingletonView xsingle(xv);
//         GECODE_ES_FAIL(home,
//                        (Set::Rel::Eq<SetCpltSetView,Gecode::Set::SingletonView>
//                         ::post(home,ss,xsingle)));
//       }
//       break;
//     case IRT_NQ:
//       {
//         Gecode::Int::IntView xv(x);
//         Gecode::Set::SingletonView xsingle(xv);
//         GECODE_ES_FAIL(home,
//                        (Set::Rel::Distinct<SetCpltSetView,Gecode::Set::SingletonView>
//                         ::post(home,ss,xsingle)));

//       }
//       break;
//     case IRT_LQ:
//       {
//         IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
//         rel(home, tmp, IRT_LQ, x);
//         GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetCpltSetView, PC_CPLTSET_DOM>
//                               ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_LE:
//       {
//         IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
//         rel(home, tmp, IRT_LE, x);
//         GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetCpltSetView, PC_CPLTSET_DOM>
//                               ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_GQ:
//       {
//         IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
//         rel(home, tmp, IRT_GQ, x);
//         GECODE_ES_FAIL(home, (Set::Int::MinElement<SetCpltSetView, PC_CPLTSET_DOM>
//                               ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_GR:
//       {
//         IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
//         rel(home, tmp, IRT_GR, x);
//         GECODE_ES_FAIL(home, (Set::Int::MinElement<SetCpltSetView, PC_CPLTSET_DOM>
//                               ::post(home,ss,tmp)));
//       }
//       break;
//     }
//   }

//   void rel(Space* home, IntVar x, IntRelType r, CpltSetVar s, SetConLevel scl) {
//     IntRelType rr;
//     switch (r) {
//     case IRT_LE: rr=IRT_GR; break;
//     case IRT_LQ: rr=IRT_GQ; break;
//     case IRT_GR: rr=IRT_LE; break;
//     case IRT_GQ: rr=IRT_LQ; break;
//     default: rr=r;
//     }
//     rel(home, s, rr, x, scl);
//   }

//   void
//   min(Space* home, CpltSetVar s, IntVar x){
//     if (home->failed()) return;
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     GECODE_ES_FAIL(home, (Set::Int::MinElement<SetCpltSetView, PC_CPLTSET_DOM>
//                    ::post(home,ss,x)));
//   }
//   void
//   max(Space* home, CpltSetVar s, IntVar x){
//     if (home->failed()) return;
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetCpltSetView, PC_CPLTSET_DOM>
//                    ::post(home,ss,x)));
//   }

//   void
//   match(Space* home, CpltSetVar s, const IntVarArgs& x) {
//     if (home->failed()) return;
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     ViewArray<Gecode::Int::IntView> xa(home,x);
//     GECODE_ES_FAIL(home, (Set::Int::Match<SetCpltSetView>::post(home,ss,xa)));
//   }

  void
  channel(Space* home, const IntVarArgs& x, const CpltSetVarArgs& y) {
    if (home->failed()) return;
    ViewArray<Gecode::Int::IntView> xa(home,x);
    ViewArray<CpltSetView> bv(home,y);
    GECODE_ES_FAIL(home, (Set::Int::Channel<CpltSetView, PC_CPLTSET_DOM>::post(home,xa,bv)));
  }

//   void weights(Space* home, const IntArgs& elements, const IntArgs& weights,
//                CpltSetVar x, IntVar y) {
//     if (home->failed()) return;
//     CpltSetView bs(x);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     Gecode::Int::IntView yv(y);
//     GECODE_ES_FAIL(home, (Set::Int::Weights<SetCpltSetView>
//                           ::post(home,elements,weights,ss,yv)));
//   }

//   void
//   cardinality(Space* home, CpltSetVar s, IntVar x) {
//     if (home->failed()) return;
//     CpltSetView bs(s);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);
//     GECODE_ES_FAIL(home, (Set::Int::Card<SetCpltSetView, PC_CPLTSET_DOM>::post(home, ss, x)));
//   }

//   void rel(Space* home, CpltSetVar x, SetOpType op, CpltSetVar y, 
//            SetRelType r, CpltSetVar z) {
//     CpltSetView xv(x); 
//     CpltSetView yv(y); 
//     CpltSetView zv(z); 

//     CrdCpltSetView cx(xv); 
//     CrdCpltSetView cy(yv); 
//     CrdCpltSetView cz(zv); 

//     SetCpltSetView sx(cx); 
//     SetCpltSetView sy(cy); 
//     SetCpltSetView sz(cz); 

//     Gecode::Set::RelOp::rel_op_post<SetCpltSetView, SetCpltSetView, SetCpltSetView>(home, sx, op, sy, r, sz);
//   }

//   void  rel(Space* home, SetOpType op, const CpltSetVarArgs& x, CpltSetVar y) {
//     if (home->failed()) return;
//     ViewArray<CpltSetView> bv(home,x);
//     ViewArray<CrdCpltSetView> cv(home, bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetCpltSetView> sv(home, cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }
   
//     CpltSetView bs(y);
//     CrdCpltSetView cs(bs);
//     SetCpltSetView ss(cs);

//     switch(op) {
//     case SOT_UNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::UnionN<SetCpltSetView,SetCpltSetView>::post(home, sv, ss)));
//       break;
//     case SOT_DUNION: 
//       GECODE_ES_FAIL(home,
//                      (Set::RelOp::PartitionN<SetCpltSetView,SetCpltSetView>::post(home, sv, ss)));
// //       GECODE_ES_FAIL(home,
// //                      (Set::RelOp::PartitionN<CpltSetView,CpltSetView>::post(home, bv, bs)));
      
//       break;
//     case SOT_INTER:
//       {
//         GECODE_ES_FAIL(home,
//                        (Set::RelOp::IntersectionN<SetCpltSetView,SetCpltSetView>
//                         ::post(home, sv, ss)));
//       }
//       break;
//     case SOT_MINUS:
//       throw InvalidRelation("rel minus");
//       break;
//     }
//   }

//   void  rel(Space* home, SetOpType op, const IntVarArgs& x, CpltSetVar y) {
//     if (home->failed()) return;
//     ViewArray<Gecode::Set::SingletonView> xa(home,x.size());
//     for (int i=x.size(); i--;) {
//       Gecode::Int::IntView iv(x[i]);
//       Gecode::Set::SingletonView sv(iv);
//       xa[i] = sv;
//     }

//     CpltSetView bs(y);
//     CrdCpltSetView cv(bs);
//     SetCpltSetView sv(cv);

//     std::cerr << "allocating was successfull!\n";

//     switch(op) {
//     case SOT_UNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::UnionN<Gecode::Set::SingletonView,SetCpltSetView>
//                            ::post(home, xa, sv)));
//       break;
//     case SOT_DUNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::PartitionN<Gecode::Set::SingletonView,SetCpltSetView>
//                            ::post(home, xa, sv)));
//       break;
//     case SOT_INTER:
//       GECODE_ES_FAIL(home,
//                      (Set::RelOp::IntersectionN<Gecode::Set::SingletonView,SetCpltSetView>
//                       ::post(home, xa, sv)));
//       break;
//     case SOT_MINUS:
//       throw InvalidRelation("rel minus");
//       break;
//     }
//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, CpltSetVar y,
//             SetRelType r, CpltSetVar z) {
//     ConstantView xv(home, x);

//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     CpltSetView bz(z);
//     CrdCpltSetView cz(bz);
//     SetCpltSetView sz(cz);

//     Gecode::Set::RelOp::rel_op_post<ConstantView,SetCpltSetView,SetCpltSetView>(home, xv, op, sy, r, sz);

//   }

//   void  rel(Space* home, CpltSetVar x, SetOpType op, const IntSet& y,
//             SetRelType r, CpltSetVar z) {
//     ConstantView yv(home, y);
//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);

//     CpltSetView bz(z);
//     CrdCpltSetView cz(bz);
//     SetCpltSetView sz(cz);

//     if (op==SOT_MINUS) {
//       switch(r) {
//       case SRT_EQ:
//         {
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,SetCpltSetView>
//                           ::post(home,cy, sx, sz)));
//         }
//         break;
//       case SRT_NQ:
//         {
//           SetVar tmp(home);
//           GECODE_ES_FAIL(home,
//                          (Set::Rel::Distinct<SetCpltSetView,SetView>
//                           ::post(home,sz,tmp)));
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,SetView>
//                           ::post(home,cy,sx,tmp)));
//         }
//         break;
//       case SRT_SUB:
//         {
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::SuperOfInter<SetCpltSetView,
//                           ComplementView<ConstantView>,SetCpltSetView>
//                           ::post(home,sx,cy,sz)));
//         }
//         break;
//       case SRT_SUP:
//         {
//           SetVar tmp(home);
//           GECODE_ES_FAIL(home,
//                          (Set::Rel::SubSet<SetCpltSetView,SetView>::post(home,sz,tmp)));
          
//           ComplementView<SetCpltSetView> cx(sx);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ConstantView,
//                           ComplementView<SetCpltSetView>,SetView>
//                           ::post(home,yv,cx,tmp)));
//         }
//         break;
//       case SRT_DISJ:
//         {
//           SetVar tmp(home);
//           EmptyView emptyset;
//           GECODE_ES_FAIL(home,(Set::RelOp::SuperOfInter<SetCpltSetView,SetView,EmptyView>
//                                ::post(home, sz, tmp, emptyset)));

//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,SetView>
//                           ::post(home,cy,sx,tmp)));
//         }
//         break;
//       case SRT_CMPL:
//         {

//           ComplementView<SetCpltSetView> cx(sx);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Union<ConstantView,
//                           ComplementView<SetCpltSetView>,
//                           SetCpltSetView>::post(home, yv, cx, sz)));
//         }
//         break;
//       }
//     } else {
//       Gecode::Set::RelOp::rel_op_post<ConstantView,SetCpltSetView,SetCpltSetView>(home, yv, op, sx, r, sz);
//     }

//   }

//   void  rel(Space* home, CpltSetVar x, SetOpType op, CpltSetVar y,
//             SetRelType r, const IntSet& z) {
//     ConstantView zv(home, z);
//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);

//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);
//     Gecode::Set::RelOp::rel_op_post<SetCpltSetView,SetCpltSetView,ConstantView>(home, sx, op, sy, r, zv);
//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, const IntSet& y,
//             SetRelType r, CpltSetVar z) {
//     ConstantView xv(home, x);
//     ConstantView yv(home, y);

//     CpltSetView bz(z);
//     CrdCpltSetView cz(bz);
//     SetCpltSetView sz(cz);
//     Gecode::Set::RelOp::rel_op_post<ConstantView,ConstantView,SetCpltSetView>(home, xv, op, yv, r, sz);

//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, CpltSetVar y, SetRelType r,
//             const IntSet& z) {
//     ConstantView xv(home, x);
//     ConstantView zv(home, z);
//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);
//     Gecode::Set::RelOp::rel_op_post<ConstantView,SetCpltSetView,ConstantView>(home, xv, op, sy, r, zv);

//   }

//   void  rel(Space* home, CpltSetVar x, SetOpType op, const IntSet& y, SetRelType r,
//             const IntSet& z) {
//     ConstantView yv(home, y);
//     ConstantView zv(home, z);

//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);
//     if (op==SOT_MINUS) {
//       switch(r) {
//       case SRT_EQ:
//         {
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,ConstantView>
//                           ::post(home,cy,sx,zv)));
//         }
//         break;
//       case SRT_NQ:
//         {
//           SetVar tmp(home);
//           GECODE_ES_FAIL(home,
//                          (Set::Rel::Distinct<ConstantView,SetView>
//                           ::post(home,zv,tmp)));
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,SetView>
//                           ::post(home,cy,sx,tmp)));
//         }
//         break;
//       case SRT_SUB:
//         {
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::SuperOfInter<SetCpltSetView,
//                           ComplementView<ConstantView>,ConstantView>
//                           ::post(home,sx,cy,zv)));
//         }
//         break;
//       case SRT_SUP:
//         {
//           // z <= tmp
//           SetVar tmp(home,z,Limits::Set::int_min, Limits::Set::int_max);
//           ComplementView<SetCpltSetView> cx(sx);
          
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ConstantView,
//                           ComplementView<SetCpltSetView>,SetView>
//                           ::post(home,yv,cx,tmp)));
//         }
//         break;
//       case SRT_DISJ:
//         {
//           SetVar tmp(home);
//           SetView tmpv(tmp);
//           IntSetRanges zi(z);
//           GECODE_ME_FAIL(home, tmpv.excludeI(home, zi));
          
//           ComplementView<ConstantView> cy(yv);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Intersection<ComplementView<ConstantView>,
//                           SetCpltSetView,SetView>
//                           ::post(home,cy,sx,tmp)));
//         }
//         break;
//       case SRT_CMPL:
//         {
//           ComplementView<SetCpltSetView> cx(sx);
//           GECODE_ES_FAIL(home,
//                          (Set::RelOp::Union<ConstantView,
//                           ComplementView<SetCpltSetView>,
//                           ConstantView>::post(home, yv, cx, zv)));
//         }
//         break;
//       }
//     } else {
//       Gecode::Set::RelOp::rel_op_post<ConstantView,SetCpltSetView,ConstantView>(home, yv, op, sx, r, zv);
//     }
//   }


//   void convex(Space* home, CpltSetVar x) {
//     if (home->failed()) return;
//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);

//     GECODE_ES_FAIL(home, (Gecode::Set::Convex::Convex<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sx)));
//   }

//   void convexHull(Space* home, CpltSetVar x, CpltSetVar y) {
//     if (home->failed()) return;
//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);

//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     GECODE_ES_FAIL(home, (Gecode::Set::Convex::ConvexHull<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sx, sy)));
//   }

//   void sequence(Space* home, const CpltSetVarArgs& x) {
//     if (home->failed()) return;
//     if (x.size()==0)
//       throw ArgumentEmpty("Set::seq");

//     ViewArray<CpltSetView> bv(home,x);
//     ViewArray<CrdCpltSetView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetCpltSetView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home, (Gecode::Set::Sequence::Seq<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sv)));
//   }

//   void sequentialUnion(Space* home, const CpltSetVarArgs& y, CpltSetVar x) {
//     if (home->failed()) return;
//     ViewArray<CpltSetView> bv(home, y);
//     ViewArray<CrdCpltSetView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetCpltSetView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     CpltSetView bx(x);
//     CrdCpltSetView cx(bx);
//     SetCpltSetView sx(cx);

//     GECODE_ES_FAIL(home, (Gecode::Set::Sequence::SeqU<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sv, sx)));

//   }

//   void atmostOne(Space* home, const CpltSetVarArgs& x, unsigned int c) {
//     if (home->failed()) return;
//     if (x.size() < 2)
//       return;
//     ViewArray<CpltSetView> bv(home,x);
//     ViewArray<CrdCpltSetView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetCpltSetView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home, (Gecode::Set::Distinct::AtmostOne<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sv, c)));
//   }

//   void distinct(Space* home, const CpltSetVarArgs& x, unsigned int c) {
//     if (home->failed()) return;
//     if (x.size() < 2)
//       return;
//     ViewArray<CpltSetView> bv(home,x);
//     ViewArray<CrdCpltSetView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetCpltSetView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home,(Gecode::Set::Distinct::Distinct<SetCpltSetView, PC_CPLTSET_DOM>::post(home, sv, c)));
//   }

//   void selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<ComplementView<SetCpltSetView> > iv(home, x);
//     IntSet universe(Limits::Set::int_min, Limits::Set::int_max);

//     CpltSetView bv(z);
//     CrdCpltSetView cv(bv);
//     SetCpltSetView sv(cv);
//     ComplementView<SetCpltSetView> cz(sv);

//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     GECODE_ES_FAIL(home,(Gecode::Set::Select::SelectIntersection<ComplementView<SetCpltSetView>,
//                          SetCpltSetView>::post(home,cz,iv,sy,universe)));

//   }

//   void selectInter(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetCpltSetView> iv(home, x);
//     IntSet universe(Limits::Set::int_min,
//                     Limits::Set::int_max);
//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     CpltSetView bz(z);
//     CrdCpltSetView cz(bz);
//     SetCpltSetView sz(cz);

//     GECODE_ES_FAIL(home,
//                    (Gecode::Set::Select::SelectIntersection<SetCpltSetView,SetCpltSetView>::
//                     post(home,sz,iv,sy,universe)));
//   }


//   void selectInterIn(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z,
//                      const IntSet& universe) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetCpltSetView> iv(home, x);
//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     CpltSetView bz(z);
//     CrdCpltSetView cz(bz);
//     SetCpltSetView sz(cz);

//     GECODE_ES_FAIL(home,
//                    (Gecode::Set::Select::SelectIntersection<SetCpltSetView,SetCpltSetView>::
//                     post(home,sz,iv,sy,universe)));
//   }


//   void selectDisjoint(Space* home, const CpltSetVarArgs& x, CpltSetVar y) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetCpltSetView> iv(home, x);

//     CpltSetView by(y);
//     CrdCpltSetView cy(by);
//     SetCpltSetView sy(cy);

//     GECODE_ES_FAIL(home, (Gecode::Set::Select::SelectDisjoint<SetCpltSetView>::post(home,iv,sy)));

//   }

//   void selectSet(Space* home, const CpltSetVarArgs& x, IntVar y, CpltSetVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<ComplementView<SetCpltSetView> > iv(home, x);
//     Gecode::Int::IntView yv(y);
//     SingletonView single(yv);

//     CpltSetView bv(z);
//     CrdCpltSetView cv(bv);
//     SetCpltSetView sv(cv);
//     ComplementView<SetCpltSetView> cz(sv);

//     IntSet universe(Limits::Set::int_min, Limits::Set::int_max);
//     GECODE_ES_FAIL(home,(Gecode::Set::Select::SelectIntersection<ComplementView<SetCpltSetView>,
//                          SingletonView>::post(home, cz, iv,
//                                               single, universe)));
//   }



}

// STATISTICS: bdd-post
