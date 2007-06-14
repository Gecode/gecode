/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date: 2006-08-21 23:11:21 +0000 (Mon, 21 Aug 2006) $ by $Author: pekczynski $
 *     $Revision: 3551 $
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
#include "gecode/bdd/bddprop.hh"

namespace Gecode { namespace Bdd {
  ExecStatus
  NaryBddProp::divide_conquer(Space* home, BddMgr* m, GecodeBdd& p, int l, int r) {
    // std::cout << "dc: " << l <<" " << r << "\n";
    if (l == r) {
//       std::cout << "telling on " << "x["<<x[l].offset()<<"]\n";
//       m->bdd2dot(p);
      // std::cout << "assigned ? " << x[l].assigned() << "\n";
      ModEvent me;
      me = x[l].tell_formula(home, p);
      // std::cout << "me = " << me << "\n";
      if (me_failed(me)) {
	return ES_FAILED;
      }
      return ES_OK;
    }
    int h = (r + l) / 2;

    // computing psi without recursion
    GecodeBdd left = p;
    for (int i = r; i >= h + 1; i--) {
      GecodeBdd dom = x[i].bdd_domain();
      int s = x[i].offset();
      int w = s + x[i].table_width() - 1;
      m->existquant(dom, left, s, w);
      left = dom;
    }
   
    // std::cout << "done LR \n";
    ExecStatus es = ES_OK;
    // std::cout << "rec call 1 "<< "\n";
    es = divide_conquer(home, m, left, l, h);
    if (es == ES_FAILED) {
      // std::cout << "ES FAILED CALL 1\n";
      return es;
    }

    GecodeBdd right = p;
    for (int i = h; i >= l; i-- ) {
      GecodeBdd dom = x[i].bdd_domain();
      int s = x[i].offset();
      int w = s + x[i].table_width() - 1;
      m->existquant(dom, right, s, w);
      right = dom;
    }

    // std::cout << "rec call 2 "<< "\n";
    es = divide_conquer(home, m, right, h + 1, r);
    if (es == ES_FAILED) {
      // std::cout << "ES FAILED CALL 2\n";
    }
    // std::cout << "FINISHED DC ***********\n";
    return es;
  }

  ExecStatus
  NaryBddProp::divide_conquer_conv(Space* home, BddMgr* m, GecodeBdd& p, int l, int r) {
    // std::cout << "divide_conquer_conv LR[" << l<< "," << r << "]\n";
    // m->bdd2dot(p);
    if (l == r) {
      GecodeBdd dom_p = x[l].bdd_domain() & p;
      GecodeBdd conv_p = conv_hull(m, dom_p);
//       std::cout << "conjunct\n";
//       m->bdd2dot(dom_p);
//       std::cout << "conv hull\n";
//       if (x[l].offset() == 0) {
// 	// display cardinality error in convex hull
// 	std::cout << "conjunct\n";
// 	m->bdd2dot(dom_p);
// 	std::cout << "conv hull\n";
// 	m->bdd2dot(conv_p);
// 	std::cout << "telling on ["<<x[l].offset() << "," << x[l].table_width() <<"] ";
      
//       }

      ModEvent me = x[l].tell_formula(home, conv_p);
      if (me_failed(me)) {
	return ES_FAILED;
      }
      return ES_OK;
    }
    int h = (r + l) / 2;

    GecodeBdd left = p;
    // std::cout << "quant left: ";
    for (int i = r; i >= h + 1; i--) {
      // std::cout << i << ",";
      GecodeBdd dom = x[i].bdd_domain();
      int s = x[i].offset();
      int w = s + x[i].table_width() - 1;
      m->existquant(dom, left, s, w);
      left = dom;
    }
    // std::cout << "\n";

    ExecStatus es = ES_OK;
    es = divide_conquer_conv(home, m, left, l, h);
    if (es == ES_FAILED) {
      return es;
    }

    // std::cout << "quant right: ";
    GecodeBdd right = p;
    for (int i = h; i >= l; i-- ) {
      // std::cout << i << ",";
      GecodeBdd dom = x[i].bdd_domain();
      int s = x[i].offset();
      int w = s + x[i].table_width() - 1;
      m->existquant(dom, right, s, w);
      right = dom;
    }
    // std::cout <<  "\n";
    es = divide_conquer_conv(home, m, right, h + 1, r);
    return es;
  }

//   ExecStatus
//   NaryBddProp::divide_conquer_split(Space* home, BddMgr* m, GecodeBdd& p, int l, int r) {
//     // std::cout << "dc: " << l <<" " << r << "\n";
//     if (l == r) {
//       // std::cout << "telling on " << "x["<<l<<"]"<< x[l].variable() << "\n";
//       // std::cout << "assigned ? " << x[l].assigned() << "\n";
//       GecodeBdd delta = p & x[l].bdd_remain();
//       GecodeBdd beta  = conv_hull(m, delta) & x[l].bdd_bounds();
//       GecodeBdd boundvars  = conv_project(m, beta);
//       delta = m->eliminate(delta, boundvars);
//       ModEvent me;
//       me = x[l].tell_formula(home, beta, delta);
//       // std::cout << "me = " << me << "\n";
//       if (me_failed(me)) {
// 	return ES_FAILED;
//       }
//       return ES_OK;
//     }
//     int h = (r + l) / 2;

//     // computing psi without recursion
//     GecodeBdd left = p;
//     for (int i = r; i >= h + 1; i--) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, left, s, w);
//       left = dom;
//     }
   
//     // std::cout << "done LR \n";
//     ExecStatus es = ES_OK;
//     // std::cout << "rec call 1 "<< "\n";
//     es = divide_conquer_split(home, m, left, l, h);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 1\n";
//       return es;
//     }

//     GecodeBdd right = p;
//     for (int i = h; i >= l; i-- ) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, right, s, w);
//       right = dom;
//     }

//     // std::cout << "rec call 2 "<< "\n";
//     es = divide_conquer_split(home, m, right, h + 1, r);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 2\n";
//     }
//     // std::cout << "FINISHED DC ***********\n";
//     return es;
//   }


//   ExecStatus
//   NaryBddProp::divide_conquer_split_card(Space* home, BddMgr* m, GecodeBdd& p, int l, int r) {
//   // std::cout << "dc_split_card: " << l <<" " << r << "\n";
//     if (l == r) {
//       // std::cout << "telling on " << "x["<<x[l].offset()<<"]\n";
//       // std::cout << "constraint p \n" << p << "\n";
//       // std::cout << "remainder \n" << x[l].bdd_remain() << "\n";

// //       assert(!m->cfalse(x[l].bdd_remain()));
// //       assert(!m->cfalse(x[l].bdd_bounds()));
// //       GecodeBdd delta = p & x[l].bdd_remain();
// //       GecodeBdd beta  = conv_hull(m, delta) & x[l].bdd_bounds();
// //       // std::cout << "delta \n" << delta << "\n";
// //       // std::cout << "beta \n" << beta << "\n";
// // //       ModEvent me = x[l].tell_bounds(home, beta);
// // //       if (me_failed(me)) {
// // // 	return ES_FAILED;
// // //       }
// // //       beta = x[l].bdd_bounds();

// //       GecodeBdd boundvars  = conv_project(m, beta);
// //       // std::cout << "boundvars \n" << boundvars << "\n";
// //       delta = m->eliminate(delta, boundvars);
// //       // std::cout << "delta elim \n" << delta << "\n";
// //       unsigned int off   = x[l].offset();
// //       unsigned int range = x[l].table_width();
// //       // m->bdd2dot(delta);
// //       GecodeBdd card = card_bounds(m, delta, boundvars, off, range);
// //       // m->bdd2dot(card);
// //       // std::cout << "card \n" << card << "\n";
// //       // std::cout << "beta \n" << beta << "\n";
// //       // std::cout << "conj \n" << (beta & card) << "\n";
// //       ModEvent me = x[l].tell_formula(home, beta, card);


//       ModEvent me = x[l].tell_formula_crd(home, p);
//       if (me_failed(me)) {
// 	return ES_FAILED;
//       }
//       return ES_OK;
//     }
//     int h = (r + l) / 2;

//     // computing psi without recursion
//     GecodeBdd left = p;
//     for (int i = r; i >= h + 1; i--) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, left, s, w);
//       left = dom;
//     }
   
//     // std::cout << "done LR \n";
//     ExecStatus es = ES_OK;
//     // std::cout << "rec call 1 "<< "\n";
//     es = divide_conquer_split_card(home, m, left, l, h);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 1\n";
//       return es;
//     }

//     GecodeBdd right = p;
//     for (int i = h; i >= l; i-- ) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, right, s, w);
//       right = dom;
//     }

//     // std::cout << "rec call 2 "<< "\n";
//     es = divide_conquer_split_card(home, m, right, h + 1, r);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 2\n";
//     }
//     // std::cout << "FINISHED DC ***********\n";
//     return es;
//   }

//   ExecStatus
//   NaryBddProp::divide_conquer_split_lex(Space* home, BddMgr* m, GecodeBdd& p, int l, int r) {
//     if (l == r) {
//       ModEvent me = x[l].tell_formula_lex(home, p);
//       if (me_failed(me)) {
// 	return ES_FAILED;
//       }
//       return ES_OK;
//     }
//     int h = (r + l) / 2;

//     // computing psi without recursion
//     GecodeBdd left = p;
//     for (int i = r; i >= h + 1; i--) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, left, s, w);
//       left = dom;
//     }
   
//     // std::cout << "done LR \n";
//     ExecStatus es = ES_OK;
//     // std::cout << "rec call 1 "<< "\n";
//     es = divide_conquer_split_lex(home, m, left, l, h);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 1\n";
//       return es;
//     }

//     GecodeBdd right = p;
//     for (int i = h; i >= l; i-- ) {
//       GecodeBdd dom = x[i].bdd_domain();
//       int s = x[i].offset();
//       int w = s + x[i].table_width() - 1;
//       m->existquant(dom, right, s, w);
//       right = dom;
//     }

//     // std::cout << "rec call 2 "<< "\n";
//     es = divide_conquer_split_lex(home, m, right, h + 1, r);
//     if (es == ES_FAILED) {
//       // std::cout << "ES FAILED CALL 2\n";
//     }
//     // std::cout << "FINISHED DC ***********\n";
//     return es;
//   }

}}

// STATISTICS: bdd-prop
