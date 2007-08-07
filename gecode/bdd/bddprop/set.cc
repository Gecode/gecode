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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/bdd.hh"

// for set bounds propagators with bdds
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"
#include "gecode/set/int.hh"
#include "gecode/set/convex.hh"
#include "gecode/set/sequence.hh"
#include "gecode/set/distinct.hh"
#include "gecode/set/select.hh"


#include "gecode/bdd/bddprop.hh"



using namespace Gecode::Bdd;

namespace Gecode {

  // set constraints using bdd variables
  // \todo put trunk template functions into icc files in order to include them here

//   void rel(Space* home, BddVar x, SetRelType r, BddVar y, BoolVar b) {
//     BddView bx(x);
//     BddView by(y);
//     CrdBddView cx(bx);
//     CrdBddView cy(by);
//     SetBddView sx(cx);
//     SetBddView sy(cy);
//     rel_re<SetBddView, SetBddView>(home, sx, r, sy, b);
//   }
  
//   void rel(Space* home, BddVar s, SetRelType r, IntVar x, SetConLevel scl) {
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);
//     Gecode::Int::IntView ix(x);
//     Gecode::Gecode::Set::SingletonView xsingle(ix);
//     Gecode::rel_post<SetBddView, Gecode::Gecode::Set::SingletonView>(home, ss, r, xsingle); 
//   }

//   void rel(Space* home, BddVar s, SetRelType r, IntVar x, BoolVar b, SetConLevel scl) {
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);
//     Gecode::Int::IntView ix(x);
//     Gecode::Gecode::Set::SingletonView xsingle(ix);
//     rel_re<SetBddView, Gecode::Gecode::Set::SingletonView>(home, ss, r, xsingle, b);
//   }

//   void rel(Space* home, IntVar x, SetRelType r, BddVar s, BoolVar b, SetConLevel scl) {
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

//   void rel(Space* home, BddVar s, IntRelType r, IntVar x, SetConLevel scl) {
//     if (home->failed()) return;
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     switch(r) {
//     case IRT_EQ:
//       {
// 	Gecode::Int::IntView xv(x);
// 	Gecode::Set::SingletonView xsingle(xv);
// 	GECODE_ES_FAIL(home,
// 		       (Set::Rel::Eq<SetBddView,Gecode::Set::SingletonView>
// 			::post(home,ss,xsingle)));
//       }
//       break;
//     case IRT_NQ:
//       {
// 	Gecode::Int::IntView xv(x);
// 	Gecode::Set::SingletonView xsingle(xv);
// 	GECODE_ES_FAIL(home,
// 		       (Set::Rel::Distinct<SetBddView,Gecode::Set::SingletonView>
// 			::post(home,ss,xsingle)));

//       }
//       break;
//     case IRT_LQ:
//       {
// 	IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
// 	rel(home, tmp, IRT_LQ, x);
// 	GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetBddView, PC_BDD_DOM>
// 			      ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_LE:
//       {
// 	IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
// 	rel(home, tmp, IRT_LE, x);
// 	GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetBddView, PC_BDD_DOM>
// 			      ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_GQ:
//       {
// 	IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
// 	rel(home, tmp, IRT_GQ, x);
// 	GECODE_ES_FAIL(home, (Set::Int::MinElement<SetBddView, PC_BDD_DOM>
// 			      ::post(home,ss,tmp)));
//       }
//       break;
//     case IRT_GR:
//       {
// 	IntVar tmp(home, Limits::Int::int_min, Limits::Int::int_max);
// 	rel(home, tmp, IRT_GR, x);
// 	GECODE_ES_FAIL(home, (Set::Int::MinElement<SetBddView, PC_BDD_DOM>
// 			      ::post(home,ss,tmp)));
//       }
//       break;
//     }
//   }

//   void rel(Space* home, IntVar x, IntRelType r, BddVar s, SetConLevel scl) {
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
//   min(Space* home, BddVar s, IntVar x){
//     if (home->failed()) return;
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     GECODE_ES_FAIL(home, (Set::Int::MinElement<SetBddView, PC_BDD_DOM>
// 		   ::post(home,ss,x)));
//   }
//   void
//   max(Space* home, BddVar s, IntVar x){
//     if (home->failed()) return;
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     GECODE_ES_FAIL(home, (Set::Int::MaxElement<SetBddView, PC_BDD_DOM>
// 		   ::post(home,ss,x)));
//   }

//   void
//   match(Space* home, BddVar s, const IntVarArgs& x) {
//     if (home->failed()) return;
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     ViewArray<Gecode::Int::IntView> xa(home,x);
//     GECODE_ES_FAIL(home, (Set::Int::Match<SetBddView>::post(home,ss,xa)));
//   }

  void
  channel(Space* home, const IntVarArgs& x, const BddVarArgs& y) {
    if (home->failed()) return;
    ViewArray<Gecode::Int::IntView> xa(home,x);
    ViewArray<BddView> bv(home,y);
    GECODE_ES_FAIL(home, (Set::Int::Channel<BddView, PC_BDD_DOM>::post(home,xa,bv)));
  }

//   void weights(Space* home, const IntArgs& elements, const IntArgs& weights,
// 	       BddVar x, IntVar y) {
//     if (home->failed()) return;
//     BddView bs(x);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     Gecode::Int::IntView yv(y);
//     GECODE_ES_FAIL(home, (Set::Int::Weights<SetBddView>
// 			  ::post(home,elements,weights,ss,yv)));
//   }

//   void
//   cardinality(Space* home, BddVar s, IntVar x) {
//     if (home->failed()) return;
//     BddView bs(s);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);
//     GECODE_ES_FAIL(home, (Set::Int::Card<SetBddView, PC_BDD_DOM>::post(home, ss, x)));
//   }

//   void rel(Space* home, BddVar x, SetOpType op, BddVar y, 
// 	   SetRelType r, BddVar z) {
//     BddView xv(x); 
//     BddView yv(y); 
//     BddView zv(z); 

//     CrdBddView cx(xv); 
//     CrdBddView cy(yv); 
//     CrdBddView cz(zv); 

//     SetBddView sx(cx); 
//     SetBddView sy(cy); 
//     SetBddView sz(cz); 

//     Gecode::Set::RelOp::rel_op_post<SetBddView, SetBddView, SetBddView>(home, sx, op, sy, r, sz);
//   }

//   void  rel(Space* home, SetOpType op, const BddVarArgs& x, BddVar y) {
//     if (home->failed()) return;
//     ViewArray<BddView> bv(home,x);
//     ViewArray<CrdBddView> cv(home, bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetBddView> sv(home, cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }
   
//     BddView bs(y);
//     CrdBddView cs(bs);
//     SetBddView ss(cs);

//     switch(op) {
//     case SOT_UNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::UnionN<SetBddView,SetBddView>::post(home, sv, ss)));
//       break;
//     case SOT_DUNION: 
//       GECODE_ES_FAIL(home,
// 		     (Set::RelOp::PartitionN<SetBddView,SetBddView>::post(home, sv, ss)));
// //       GECODE_ES_FAIL(home,
// // 		     (Set::RelOp::PartitionN<BddView,BddView>::post(home, bv, bs)));
      
//       break;
//     case SOT_INTER:
//       {
// 	GECODE_ES_FAIL(home,
// 		       (Set::RelOp::IntersectionN<SetBddView,SetBddView>
// 			::post(home, sv, ss)));
//       }
//       break;
//     case SOT_MINUS:
//       throw InvalidRelation("rel minus");
//       break;
//     }
//   }

//   void  rel(Space* home, SetOpType op, const IntVarArgs& x, BddVar y) {
//     if (home->failed()) return;
//     ViewArray<Gecode::Set::SingletonView> xa(home,x.size());
//     for (int i=x.size(); i--;) {
//       Gecode::Int::IntView iv(x[i]);
//       Gecode::Set::SingletonView sv(iv);
//       xa[i] = sv;
//     }

//     BddView bs(y);
//     CrdBddView cv(bs);
//     SetBddView sv(cv);

//     std::cerr << "allocating was successfull!\n";

//     switch(op) {
//     case SOT_UNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::UnionN<Gecode::Set::SingletonView,SetBddView>
// 			   ::post(home, xa, sv)));
//       break;
//     case SOT_DUNION:
//       GECODE_ES_FAIL(home,(Set::RelOp::PartitionN<Gecode::Set::SingletonView,SetBddView>
// 			   ::post(home, xa, sv)));
//       break;
//     case SOT_INTER:
//       GECODE_ES_FAIL(home,
// 		     (Set::RelOp::IntersectionN<Gecode::Set::SingletonView,SetBddView>
// 		      ::post(home, xa, sv)));
//       break;
//     case SOT_MINUS:
//       throw InvalidRelation("rel minus");
//       break;
//     }
//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, BddVar y,
// 	    SetRelType r, BddVar z) {
//     ConstantView xv(home, x);

//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     BddView bz(z);
//     CrdBddView cz(bz);
//     SetBddView sz(cz);

//     Gecode::Set::RelOp::rel_op_post<ConstantView,SetBddView,SetBddView>(home, xv, op, sy, r, sz);

//   }

//   void  rel(Space* home, BddVar x, SetOpType op, const IntSet& y,
// 	    SetRelType r, BddVar z) {
//     ConstantView yv(home, y);
//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);

//     BddView bz(z);
//     CrdBddView cz(bz);
//     SetBddView sz(cz);

//     if (op==SOT_MINUS) {
//       switch(r) {
//       case SRT_EQ:
// 	{
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,SetBddView>
// 			  ::post(home,cy, sx, sz)));
// 	}
// 	break;
//       case SRT_NQ:
// 	{
// 	  SetVar tmp(home);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::Rel::Distinct<SetBddView,SetView>
// 			  ::post(home,sz,tmp)));
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,SetView>
// 			  ::post(home,cy,sx,tmp)));
// 	}
// 	break;
//       case SRT_SUB:
// 	{
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::SuperOfInter<SetBddView,
// 			  ComplementView<ConstantView>,SetBddView>
// 			  ::post(home,sx,cy,sz)));
// 	}
// 	break;
//       case SRT_SUP:
// 	{
// 	  SetVar tmp(home);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::Rel::SubSet<SetBddView,SetView>::post(home,sz,tmp)));
	  
// 	  ComplementView<SetBddView> cx(sx);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ConstantView,
// 			  ComplementView<SetBddView>,SetView>
// 			  ::post(home,yv,cx,tmp)));
// 	}
// 	break;
//       case SRT_DISJ:
// 	{
// 	  SetVar tmp(home);
// 	  EmptyView emptyset;
// 	  GECODE_ES_FAIL(home,(Set::RelOp::SuperOfInter<SetBddView,SetView,EmptyView>
// 			       ::post(home, sz, tmp, emptyset)));

// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,SetView>
// 			  ::post(home,cy,sx,tmp)));
// 	}
// 	break;
//       case SRT_CMPL:
// 	{

// 	  ComplementView<SetBddView> cx(sx);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Union<ConstantView,
// 			  ComplementView<SetBddView>,
// 			  SetBddView>::post(home, yv, cx, sz)));
// 	}
// 	break;
//       }
//     } else {
//       Gecode::Set::RelOp::rel_op_post<ConstantView,SetBddView,SetBddView>(home, yv, op, sx, r, sz);
//     }

//   }

//   void  rel(Space* home, BddVar x, SetOpType op, BddVar y,
// 	    SetRelType r, const IntSet& z) {
//     ConstantView zv(home, z);
//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);

//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);
//     Gecode::Set::RelOp::rel_op_post<SetBddView,SetBddView,ConstantView>(home, sx, op, sy, r, zv);
//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, const IntSet& y,
// 	    SetRelType r, BddVar z) {
//     ConstantView xv(home, x);
//     ConstantView yv(home, y);

//     BddView bz(z);
//     CrdBddView cz(bz);
//     SetBddView sz(cz);
//     Gecode::Set::RelOp::rel_op_post<ConstantView,ConstantView,SetBddView>(home, xv, op, yv, r, sz);

//   }

//   void  rel(Space* home, const IntSet& x, SetOpType op, BddVar y, SetRelType r,
// 	    const IntSet& z) {
//     ConstantView xv(home, x);
//     ConstantView zv(home, z);
//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);
//     Gecode::Set::RelOp::rel_op_post<ConstantView,SetBddView,ConstantView>(home, xv, op, sy, r, zv);

//   }

//   void  rel(Space* home, BddVar x, SetOpType op, const IntSet& y, SetRelType r,
// 	    const IntSet& z) {
//     ConstantView yv(home, y);
//     ConstantView zv(home, z);

//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);
//     if (op==SOT_MINUS) {
//       switch(r) {
//       case SRT_EQ:
// 	{
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,ConstantView>
// 			  ::post(home,cy,sx,zv)));
// 	}
// 	break;
//       case SRT_NQ:
// 	{
// 	  SetVar tmp(home);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::Rel::Distinct<ConstantView,SetView>
// 			  ::post(home,zv,tmp)));
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,SetView>
// 			  ::post(home,cy,sx,tmp)));
// 	}
// 	break;
//       case SRT_SUB:
// 	{
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::SuperOfInter<SetBddView,
// 			  ComplementView<ConstantView>,ConstantView>
// 			  ::post(home,sx,cy,zv)));
// 	}
// 	break;
//       case SRT_SUP:
// 	{
// 	  // z <= tmp
// 	  SetVar tmp(home,z,Limits::Set::int_min, Limits::Set::int_max);
// 	  ComplementView<SetBddView> cx(sx);
	  
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ConstantView,
// 			  ComplementView<SetBddView>,SetView>
// 			  ::post(home,yv,cx,tmp)));
// 	}
// 	break;
//       case SRT_DISJ:
// 	{
// 	  SetVar tmp(home);
// 	  SetView tmpv(tmp);
// 	  IntSetRanges zi(z);
// 	  GECODE_ME_FAIL(home, tmpv.excludeI(home, zi));
	  
// 	  ComplementView<ConstantView> cy(yv);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Intersection<ComplementView<ConstantView>,
// 			  SetBddView,SetView>
// 			  ::post(home,cy,sx,tmp)));
// 	}
// 	break;
//       case SRT_CMPL:
// 	{
// 	  ComplementView<SetBddView> cx(sx);
// 	  GECODE_ES_FAIL(home,
// 			 (Set::RelOp::Union<ConstantView,
// 			  ComplementView<SetBddView>,
// 			  ConstantView>::post(home, yv, cx, zv)));
// 	}
// 	break;
//       }
//     } else {
//       Gecode::Set::RelOp::rel_op_post<ConstantView,SetBddView,ConstantView>(home, yv, op, sx, r, zv);
//     }
//   }


//   void convex(Space* home, BddVar x) {
//     if (home->failed()) return;
//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);

//     GECODE_ES_FAIL(home, (Gecode::Set::Convex::Convex<SetBddView, PC_BDD_DOM>::post(home, sx)));
//   }

//   void convexHull(Space* home, BddVar x, BddVar y) {
//     if (home->failed()) return;
//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);

//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     GECODE_ES_FAIL(home, (Gecode::Set::Convex::ConvexHull<SetBddView, PC_BDD_DOM>::post(home, sx, sy)));
//   }

//   void sequence(Space* home, const BddVarArgs& x) {
//     if (home->failed()) return;
//     if (x.size()==0)
//       throw ArgumentEmpty("Set::seq");

//     ViewArray<BddView> bv(home,x);
//     ViewArray<CrdBddView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetBddView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home, (Gecode::Set::Sequence::Seq<SetBddView, PC_BDD_DOM>::post(home, sv)));
//   }

//   void sequentialUnion(Space* home, const BddVarArgs& y, BddVar x) {
//     if (home->failed()) return;
//     ViewArray<BddView> bv(home, y);
//     ViewArray<CrdBddView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetBddView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     BddView bx(x);
//     CrdBddView cx(bx);
//     SetBddView sx(cx);

//     GECODE_ES_FAIL(home, (Gecode::Set::Sequence::SeqU<SetBddView, PC_BDD_DOM>::post(home, sv, sx)));

//   }

//   void atmostOne(Space* home, const BddVarArgs& x, unsigned int c) {
//     if (home->failed()) return;
//     if (x.size() < 2)
//       return;
//     ViewArray<BddView> bv(home,x);
//     ViewArray<CrdBddView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetBddView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home, (Gecode::Set::Distinct::AtmostOne<SetBddView, PC_BDD_DOM>::post(home, sv, c)));
//   }

//   void distinct(Space* home, const BddVarArgs& x, unsigned int c) {
//     if (home->failed()) return;
//     if (x.size() < 2)
//       return;
//     ViewArray<BddView> bv(home,x);
//     ViewArray<CrdBddView> cv(home,bv.size());
//     for (int i = bv.size(); i--; ) { cv[i].init(bv[i]); }
//     ViewArray<SetBddView> sv(home,cv.size());
//     for (int i = cv.size(); i--; ) { sv[i].init(cv[i]); }

//     GECODE_ES_FAIL(home,(Gecode::Set::Distinct::Distinct<SetBddView, PC_BDD_DOM>::post(home, sv, c)));
//   }

//   void selectUnion(Space* home, const BddVarArgs& x, BddVar y, BddVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<ComplementView<SetBddView> > iv(home, x);
//     IntSet universe(Limits::Set::int_min, Limits::Set::int_max);

//     BddView bv(z);
//     CrdBddView cv(bv);
//     SetBddView sv(cv);
//     ComplementView<SetBddView> cz(sv);

//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     GECODE_ES_FAIL(home,(Gecode::Set::Select::SelectIntersection<ComplementView<SetBddView>,
// 			 SetBddView>::post(home,cz,iv,sy,universe)));

//   }

//   void selectInter(Space* home, const BddVarArgs& x, BddVar y, BddVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetBddView> iv(home, x);
//     IntSet universe(Limits::Set::int_min,
// 		    Limits::Set::int_max);
//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     BddView bz(z);
//     CrdBddView cz(bz);
//     SetBddView sz(cz);

//     GECODE_ES_FAIL(home,
//                    (Gecode::Set::Select::SelectIntersection<SetBddView,SetBddView>::
// 		    post(home,sz,iv,sy,universe)));
//   }


//   void selectInterIn(Space* home, const BddVarArgs& x, BddVar y, BddVar z,
// 		     const IntSet& universe) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetBddView> iv(home, x);
//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     BddView bz(z);
//     CrdBddView cz(bz);
//     SetBddView sz(cz);

//     GECODE_ES_FAIL(home,
//                    (Gecode::Set::Select::SelectIntersection<SetBddView,SetBddView>::
// 		    post(home,sz,iv,sy,universe)));
//   }


//   void selectDisjoint(Space* home, const BddVarArgs& x, BddVar y) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<SetBddView> iv(home, x);

//     BddView by(y);
//     CrdBddView cy(by);
//     SetBddView sy(cy);

//     GECODE_ES_FAIL(home, (Gecode::Set::Select::SelectDisjoint<SetBddView>::post(home,iv,sy)));

//   }

//   void selectSet(Space* home, const BddVarArgs& x, IntVar y, BddVar z) {
//     if (home->failed()) return;
//     Set::Select::IdxViewArray<ComplementView<SetBddView> > iv(home, x);
//     Gecode::Int::IntView yv(y);
//     SingletonView single(yv);

//     BddView bv(z);
//     CrdBddView cv(bv);
//     SetBddView sv(cv);
//     ComplementView<SetBddView> cz(sv);

//     IntSet universe(Limits::Set::int_min, Limits::Set::int_max);
//     GECODE_ES_FAIL(home,(Gecode::Set::Select::SelectIntersection<ComplementView<SetBddView>,
// 			 SingletonView>::post(home, cz, iv,
// 					      single, universe)));
//   }



}

// STATISTICS: bdd-post
