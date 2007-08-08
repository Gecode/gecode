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

#include "gecode/cpltset.hh"

using namespace Gecode::Bdd;


namespace Gecode {

  // \todo FIXME similar replace intset with dynamicarray

  /** \brief Set variable order as presented in HLS for CpltSetVars in \a x 
   *  It is possible to already order \a add additional variables not yet declared
   *  as needed in the case of SingletonBddViews, i.e. while using hybrid 
   *  Int Bdd constraints. 
   */
  void 
  hls_order(Space* home, const CpltSetVarArray& x) {
    // std::cout << "start hls_order()\n";
    int n = x.size();
    ViewArray<BddView> view(home, n);
    for (int i = n; i--; ) {
      view[i] = x[i];
    }
    // std::cout << "call variableorder()\n";
    variableorder(view);
  }

  /** \brief Set variable order as presented in HLS for CpltSetVars in \a x  and in \a y
   *  It is possible to already order \a add additional variables not yet declared
   *  as needed in the case of SingletonBddViews, i.e. while using hybrid 
   *  Int Bdd constraints. 
   */
  void 
  hls_order(Space* home, const CpltSetVarArray& x, const CpltSetVarArray& y) {
    // std::cout << "start hls_order(x, y)\n";
    int n = x.size();
    ViewArray<BddView> viewx(home, n);
    for (int i = n; i--; ) {
      viewx[i] = x[i];
    }

    n = y.size();
    ViewArray<BddView> viewy(home, n);
    for (int i = n; i--; ) {
      viewy[i] = y[i];
    }
    // std::cout << "call variableorder(x, y)\n";
    variableorder(viewx, viewy);
  }

//   /** \brief Set variable order as presented in HLS for CpltSetVars in \a x  and in \a y
//    *  It is possible to already order \a add additional variables not yet declared
//    *  as needed in the case of SingletonBddViews, i.e. while using hybrid 
//    *  Int Bdd constraints. 
//    */
//   void 
//   hls_order(Space* home, CpltSetVarArray** x, int xs) {
    
//     GECODE_AUTOARRAY(ViewArray<BddView>*, viewptr, xs);
//     int c = 0;
//     for (int i = 0; i < xs; i++) {
//       int n = x[i]->size();
//       ViewArray<BddView> viewx(home, n);
//       for (int j = n; j--; ) {
// 	viewx[j] = (*(x[i]))[j];
//       }
//       viewptr[c] = &viewx;
//       c++;
//     }
//     for (int i = 0; i < xs; i++) {
//       for (int j = 0; j < viewptr[i]->size(); j++) {
// 	std::cerr << (*(viewptr[i]))[j] << " ";
//       }
//       std::cerr << "\n";
//     }
//     variableorder(&viewptr[0], xs);
//   }

  void
  hls_order(Space* home, CpltSetVarArray** x, int xs) {
    // std::cerr << "start order\n";
    
    BMI* mgr = (*(x[0]))[0].manager();
    // std::cerr << "allocated \n";
    unsigned int var_in_tab = mgr->allocated();
    
    // std::cerr << "var_in_tab = " << var_in_tab << "\n";

    unsigned int min_offset = (*(x[0]))[0].offset();
    unsigned int max_width  = (*(x[0]))[0].table_width();
    int minv = (*(x[0]))[0].mgr_min();
    int maxv = (*(x[0]))[0].mgr_max();

    for (int i = 0; i < xs; i++) {

      int n = x[i]->size();
      for (int j = 0; j < n; j++) {
	unsigned int off   = (*(x[i]))[j].offset();
	unsigned int width = (*(x[i]))[j].table_width();
	int cmin  = (*(x[i]))[j].mgr_min();
	int cmax  = (*(x[i]))[j].mgr_max();
	// std::cerr << "off= " << off << " width=" << width << " cmin=" << cmin << " cmax= " << cmax << "\n";
	if (off < min_offset) {
	  min_offset = off;
	}
	if (width > max_width) {
	  max_width = width;
	}
	if (cmax > maxv) {
	  maxv = cmax;
	}
	if (cmin < minv) {
	  minv = cmin;
	}
      }
    }
    
    // std::cerr << "max_width = " << max_width  << " min_offset = " << min_offset << "\n";

    GECODE_AUTOARRAY(int, hls_order, var_in_tab);

    // we do not care about variables 
    // that are not in the scope of the constraint
    int c = 0;
    for (unsigned int i = 0; i < min_offset; i++, c++) {
      hls_order[i] = i;
    }

    // ordering those that lie in the scope
    for (unsigned int f = 0; f < max_width; f++) {
      for (int z = 0; z < xs; z++) {
	for (int i = 0; i < x[z]->size(); i++) {
	  if ( ((*(x[z]))[i].mgr_min() <= minv + (int) f) && 
	       (minv + (int) f <= (*(x[z]))[i].mgr_max())) {
	    // std::cerr << "do (*(x[z])) \t";
	    int xshift = (*(x[z]))[i].mgr_min() - minv;
	    int xo = (*(x[z]))[i].offset();
	    // std::cerr << "order " << xo << "\n";
	    int xw = (*(x[z]))[i].table_width();
	    int cur= xo + f - xshift;
	    if (cur <= xo + xw) {
	      hls_order[c] = cur;
	      c++;
	    }
	  }
	}
      }
    }

    // IMPORTANT DO NOT FORGET REMAINING LARGER VARIABLES
    for (unsigned int i = c; i < var_in_tab; i++, c++) {
      hls_order[i] = i;
    }   

//     for (unsigned int i = 0; i < var_in_tab; i++) {
//       std::cerr << hls_order[i] << " ";
//     }   
//     std::cerr << "\n";


    // std::cerr << "set order\n";
    mgr->setorder(&hls_order[0]);
    // std::cerr << "ordering done\n";
  }

  void
  conv_hull_project(BMI* mgr, GecodeBdd& robdd, 
		    GecodeBdd& hull, GecodeBdd& poshull) {
    if (mgr->leaf(robdd)) { hull = robdd; return; }
    if (mgr->leaf(robdd)) { poshull = robdd; return; }
    assert(robdd != BDDTOP);
    assert(robdd != BDDBOT);

    BddIterator bi(mgr, robdd);
    // int label_shift = mgr->bddidx(robdd);
    // // std::cout << "computed shift\n";
    while(bi()) {
      int stat = bi.status();
      int lev  = bi.label();
      GecodeBdd cur = mgr->bddpos(lev);
      // in order to be able to quantify ALL bounds out
      // we have to store the variables not in the lub
      // in their positive form
      if (stat == FIX_GLB) {
	assert(!mgr->marked(cur));
	hull    &= cur;
	poshull &= cur;
      }
      if (stat == FIX_NOT_LUB) {
	poshull &= cur;
	hull    &= mgr->negbddpos(lev);
      }
      ++bi;
    }
    return;
  }

  void
  conv_hull(BMI* mgr, GecodeBdd& robdd, GecodeBdd& hull) {
    if (mgr->leaf(robdd)) { hull = robdd; return; }
    BddIterator bi(mgr, robdd);
    while(bi()) {
      int stat = bi.status();
      int lev  = bi.label();
      // in order to be able to quantify ALL bounds out
      // we have to store the variables not in the lub
      // in their positive form
      if (stat == FIX_GLB)     { hull &= mgr->bddpos(lev); }
      if (stat == FIX_NOT_LUB) { hull &= mgr->negbddpos(lev); }
      ++bi;
    }
    return;
  }
  
  void
  conv_project(BMI* mgr, GecodeBdd& robdd, GecodeBdd& poshull) {
    if (mgr->leaf(robdd)) { poshull = robdd; return; }
    BddIterator bi(mgr, robdd);
    while(bi()) {
      int stat = bi.status();
      int lev  = bi.label();
      // in order to be able to quantify ALL bounds out
      // we have to store the variables not in the lub
      // in their positive form
      if (stat == FIX_GLB)     { poshull &= mgr->bddpos(lev); }
      if (stat == FIX_NOT_LUB) { poshull &= mgr->bddpos(lev); }
      ++bi;
    }
    return;
  }

  GecodeBdd 
  bdd_vars(BMI* mgr, GecodeBdd& robdd) {

    GecodeBdd allvars    = BDDTOP;
    BddIterator bi(mgr, robdd);
    while(bi()) {
      int lev  = bi.label();
      // in order to be able to quantify ALL bounds out
      // we have to store the variables not in the lub
      // in their positive form
      GecodeBdd cur = mgr->bddpos(lev);
      assert(!mgr->marked(cur));
      allvars &= cur;
      ++bi;
    }
    
    return allvars;
  }

  GecodeBdd 
  cardeq(BMI* mgr, int offset, int c, int n, int r) {
    // std::cout << "cardeq:" <<  offset << ","  << c << " " << n << " " << r << "\n";

    GECODE_AUTOARRAY(GecodeBdd, layer, n);
    // the use of autoarray now requires explicit initialization
    // otherwise the bdd nodes are not known in the global table
    for (int i = n; i--;) 
      layer[i].init();

    // build the nodes of the lowest layer
    layer[0] = BDDTOP;
    for (int i = 1; i <= c; i++) {
      layer[i].init();
      layer[i] = mgr->bddpos(offset + r - i + 1);
    }

    // connect the lowest layer
    for (int i = 1; i < n; i++) {
      layer[i] = mgr->ite(layer[i], layer[i - 1], BDDBOT);
    }

    // build layers on top
    for (int k = r + 1; --k; ) {
      int col = k;
      for (int i = 0; i < n; i++) {
	GecodeBdd t = BDDTOP;
	if (i == 0) {
	  t = BDDBOT;
	} else {
	  t = layer[i-1]; 
	}
	layer[i] = mgr->ite(mgr->bddpos(offset + col), t,layer[i]);
	col--;
	if (col < 0) { k = 1; break;}
      }
    }
    //std::cout << "cardeq finished\n";
    return layer[n - 1];
  }

  GecodeBdd 
  cardlqgq(BMI* mgr, int offset, int cl, int cr, int n, int r) {
    //std::cout << "cardlqgq:" <<  offset << "," << cl << "," << cr << " " << n << " " << r << "\n";

    GECODE_AUTOARRAY(GecodeBdd, layer, n);
    // the use of autoarray now requires explicit initialization
    // otherwise the bdd nodes are not known in the global table
    for (int i = n; i--;) 
      layer[i].init();

    // creates TOP v(c) v(c-1) ... v(c - cl + 1)
    layer[n - cl - 1] = BDDTOP;
    int k = r;
    for (int i = n - cl ; i < n; i++) {
        layer[i] = mgr->ite(mgr->bddpos(offset + k), layer[i - 1], BDDBOT);
      k--;
    }

    //for (k = r - 1; k > 0; k--) {
    // for (k = r; --k; ) {
    for (k = r; k--; ) {
      int col = k;
      // cl < cr <= tab  ==> n - cl > 0 
      for (int i = n - cl; i < n; i++) { 
	//std::cerr << "i= " << i << " col test= " << col << " < " << (r + 1 - cr) << " and " << layer[i] << "\n";
	GecodeBdd t = layer[i-1]; 
	layer[i] = mgr->ite(mgr->bddpos(offset + col), t, layer[i]);
	col--;
	if (col < r + 1 - cr) { k = 0; break;}
      }
    }

    if (cr == r + 1) { // cardinality equals table_width, all elements allowed
      return layer[n - 1];
    }

    if (cr == r) {
      int col = r;
      // one single large layer
      GecodeBdd t = BDDTOP;
      GecodeBdd f = BDDTOP;
      GecodeBdd zerot = BDDBOT;
      GecodeBdd zerof = t;
      for (int i = 0; i < n; i++) {
	if (i == 0) {
	  t = zerot;
	  f = zerof;
	} else {
	  t = layer[i-1];
	  if (i > n - cl - 1) { // connect lower layer
	    f = layer[i];
	  }
	}
	layer[i] = mgr->ite(mgr->bddpos(offset + col), t ,f);
	col--;
	if (col < 0) { break;}
      }

      return layer[n- 1];
    }
  
    // connect lower and upper parts of the bdd
    // one single layer in between
    int col = r;
    {
      GecodeBdd t = BDDTOP;
      GecodeBdd f = BDDTOP;
      for (int i = 0; i < n; i++) {
	if (i == 0) {
	  t = BDDBOT;
	} else {
	  t = layer[i-1];
	  // NOTE: ONLY CONNECT if cl > 0
	  if (i > n - cl - 1 && cl > 0) { // connect lower layer
	    f = layer[i];
	  }
	}
	layer[i] = mgr->ite(mgr->bddpos(offset + col), t ,f);
	col--;
	if (col < 0) { break;}
      }
    }

    // the remaining layers 
    // for (k = r - 1; k > 0; k--) {
    for (k = r; --k; ) {
      int col = k;
      for (int i = 0; i < n; i++) {
	GecodeBdd t = BDDTOP;
	if (i == 0) {
	  t = BDDBOT;
	} else {
	  t = layer[i-1]; 
	}
	layer[i] = mgr->ite(mgr->bddpos(offset + col), t, layer[i]);
	col--;
	if (col < 0) { k = 1; break;}
      }
    }
    return layer[n - 1];
  }

  GecodeBdd 
  cardcheck(BMI* mgr, int xtab, int offset, int cl, int cr) {
    // std::cout << "cardcheck:" << xtab << "," << offset << "," << cl << "," << cr << "\n";
    if (cr > xtab) { 
      cr = xtab;
    }
    int r = xtab - 1; // rightmost bit in bitvector
    int n = cr + 1; // layer size
    if (cl > xtab || cl > cr)  // inconsistent cardinality
      return BDDBOT;

    if (cr == 0) {    // cl <= cr
      // build the emptyset
      GecodeBdd empty = BDDTOP;
      for (int i = xtab; i--; ) {
	empty &= mgr->negbddpos(offset + i);
      }
      return empty;
    }
  
    if (cl == cr) {
      if (cr == xtab) {
	// build the full set
	GecodeBdd full = BDDTOP;
	for (int i = xtab; i--; ) {
	  full &= mgr->bddpos(offset + i);
	}
	return full;
      } else {
	return cardeq(mgr, offset, cr, n, r);
      }
    }

    // cl < cr
    if (cr == xtab) {
      if (cl == 0) {   // no cardinality restriction
	return BDDTOP;
      }
    }
    return cardlqgq(mgr, offset, cl, cr, n, r);
  }

  GecodeBdd 
  cardrec_bin(BMI*& mgr, int j, unsigned int& off1, unsigned int& range1, 
	      unsigned int& off2, unsigned int& range2, int l, int u) {
    if ((l <= 0) && ((int) range1 <= u)) {
      return BDDTOP;
    }
    if ( ((int) range1 < l) || (u < 0)) {
      return BDDBOT;
    }
    GecodeBdd cur = BDDTOP;
    if (j < (int) range1) {      
      cur = ( mgr->bddpos(off1 + j) & mgr->bddpos(off2 + j) );
      j++;
    } else {
      if (l > 0 ) {
	return BDDBOT;
      } else {
	return BDDTOP;
      }
    }

    GecodeBdd neg = BDDTOP;
    neg &= (!cur & cardrec_bin(mgr, j, off1, range1, off2, range2, l, u));
    GecodeBdd pos = BDDTOP;
    pos &= (cur  & cardrec_bin(mgr, j, off1, range1, off2, range2, l - 1, u - 1));
    GecodeBdd cv = BDDTOP;
    cv &= (neg | pos);
    return cv;
  }


  GecodeBdd 
  cardrec(BMI*& mgr, GecodeBdd& boundvars, 
	  int j, unsigned int& offset, unsigned int& range, int l, int u) {
  // std::cout << "cardrec withthout boundvars\n";
    GecodeBdd b = boundvars;
    if (!mgr->leaf(boundvars)) {
      int idx = mgr->bddidx(boundvars) - offset;
    // std::cout << "out j=" << j << " idx = " << idx << "\n";
      while ( j == idx ) {
	// std::cout  << "in j=" << j << " idx = " << idx << "\n";
	j++;
	boundvars = mgr->iftrue(boundvars);
	if (mgr->leaf(boundvars)) {
	  break;
	}
	idx = mgr->bddidx(boundvars) - offset;
      }
    }

    if ((l <= 0) && ((int) range <= u)) {
      return BDDTOP;
    }
    if ( ((int) range < l) || (u < 0)) {
      return BDDBOT;
    }
    GecodeBdd cur = BDDTOP;
    if (j < (int) range) {      
      cur = mgr->bddpos(offset + j);
      j++;
    } else {
      if (l > 0 ) {
	return BDDBOT;
      } else {
	return BDDTOP;
      }
    }
    b = boundvars;
    GecodeBdd neg = (!cur & cardrec(mgr, b, j, offset, range, l, u));
    b = boundvars;
    GecodeBdd pos = (cur  & cardrec(mgr, b, j, offset, range, l - 1, u - 1));
    GecodeBdd cv = BDDTOP;
    cv &= (neg | pos);
    return cv;
  }

  GecodeBdd 
  lexlt(BMI*& mgr, unsigned int& xoff, unsigned int& yoff, unsigned int& range, int n) {
//     if (n > (int) range - 1) { return BDDBOT; }
    if (n < 0) { return BDDBOT; }

    GecodeBdd cur = BDDTOP;
    cur &= ((mgr->negbddpos(xoff + n)) & (mgr->bddpos(yoff + n)));
    cur |= ( ((mgr->bddpos(xoff + n)) % (mgr->bddpos(yoff + n))) & lexlt(mgr, xoff, yoff, range, n - 1));
    return cur;
  }

  GecodeBdd 
  lexlq(BMI*& mgr, unsigned int& xoff, unsigned int& yoff, unsigned int& range, int n) {
    // only difference to lexlt
//     if (n > static_cast<int> (range) - 1) { return BDDTOP; }
    if (n < 0) { return BDDTOP; }

    GecodeBdd cur = BDDTOP;
    cur &= ((mgr->negbddpos(xoff + n)) & (mgr->bddpos(yoff + n)));
    cur |= ( ((mgr->bddpos(xoff + n)) % (mgr->bddpos(yoff + n))) & lexlq(mgr, xoff, yoff, range, n - 1));
    return cur;
  }


  GecodeBdd 
  lexltrev(BMI*& mgr, unsigned int& xoff, unsigned int& yoff, unsigned int& range, int n) {
    if (n > (int) range - 1) { return BDDBOT; }
    GecodeBdd cur = BDDTOP;
    cur &= ((mgr->negbddpos(xoff + n)) & (mgr->bddpos(yoff + n)));
    cur |= ( ((mgr->bddpos(xoff + n)) % (mgr->bddpos(yoff + n))) & lexltrev(mgr, xoff, yoff, range, n + 1));
    return cur;
  }

  GecodeBdd 
  lexlqrev(BMI*& mgr, unsigned int& xoff, unsigned int& yoff, unsigned int& range, int n) {
    // only difference to lexlt
    if (n > static_cast<int> (range) - 1) { return BDDTOP; }

    GecodeBdd cur = BDDTOP;
    cur &= ((mgr->negbddpos(xoff + n)) & (mgr->bddpos(yoff + n)));
    cur |= ( ((mgr->bddpos(xoff + n)) % (mgr->bddpos(yoff + n))) & lexlqrev(mgr, xoff, yoff, range, n + 1));
    return cur;
  }

  void card_count(BMI* mgr, GecodeBdd& remain, GecodeBdd& boundvars,
		  unsigned int n, unsigned int& offset, 
		  unsigned int& range, int& l, int& u) {

   // std::cout << "cc: " << "n= " << n << " offset = " << offset << " range = " << range << " l = " << l << " u= " << u << "\n";
    // skip variables in the boolean vector that 
    // belong to the fixed part of the bdd
    if (!mgr->leaf(boundvars)) {
      unsigned int idx = mgr->bddidx(boundvars) - offset;
      while ( n == idx ) {
	n++;
	boundvars = mgr->iftrue(boundvars);
	if (mgr->leaf(boundvars)) {
	  break;
	}
	idx = mgr->bddidx(boundvars) - offset;
      }
    }

   // std::cout << "skipped: " << "n= " << n << " offset = " << offset << " range = " << range << " l = " << l << " u= " << u << "\n";
    if (mgr->cfalse(remain)) {
      // mark false branch by 
      // (Limits::Set::card_max, -Limits::Set::card_max)

       // std::cout << "remain is false leaf\n";
      l = Limits::Set::card_max;
      u = -Limits::Set::card_max;
       // std::cout << "computed values l=" << l << " and u= " << u <<"\n";
      return;
    } else {
      if (mgr->ctrue(remain)) {
	if (n == range) {
	  // if we have seen all variables in the tree
	  // mark the true branch with (0,0)
	   // std::cout << "remain is true leaf vector finished\t";
	  l = 0;
	  u = 0;
	   // std::cout << "(" << l << "," << u <<")\n";
	  return;
	} else {
	  // if we have not yet seen all variables in the tree
	  // mark the true branch with (0, range - n)
	   // std::cout << "remain is true bdd return vector size\t";
	  l = 0;
	  u = range - n;
	   // std::cout << "(" << l << "," << u <<")\n";
	  return;
	}
      } else {
	 // std::cout << "start recursion full tree\n";
// 	 // std::cout << "boundvars = " << boundvars << "\n ";
// 	 // std::cout << "remain    = " << remain << "\n";

	 // std::cout << "idx = "<< mgr->bddidx(remain) - offset<< "\n";
	    GecodeBdd b = boundvars;
	if (mgr->bddidx(remain) - offset == n) {
	  int lt = 0;
	  int ut = 0;
	  GecodeBdd t = mgr->iftrue(remain);
	  
	  card_count(mgr, t,  b, n + 1, offset, range, lt, ut);
	   // std::cout << "TB(" << lt << "," << ut <<")\n";

	  int lf = 0;
	  int uf = 0;
	  b = boundvars;
	  GecodeBdd f = mgr->iffalse(remain);
	  card_count(mgr, f,  b, n + 1, offset, range, lf, uf);
	   // std::cout << "FB(" << lf << "," << uf <<")\n";
	  
	  l = std::min(lt + 1, lf);
	  u = std::max(ut + 1, uf);
	   // std::cout << "OVERALL(" << l << "," << u <<")\n";
	  return;
	} else {
	   // std::cout << "tree smaller than vector\n";
	  b = boundvars;
	  card_count(mgr, remain,  b, n + 1, offset, range, l, u);
	  u++;
	   // std::cout << "RES(" << l << "," << u <<")\n";
	  return;
	}
      }
    }
  }

  // NEW CARDINALITY COUNTING FOR EXTRACTING BOUNDS
  // TEST NEW STRUCTURE FOR EXTRACTING CARD

  // mark all nodes in the dqueue
  void
  extcache_mark(BMI* mgr, Support::DynamicArray<GecodeBdd>& nodes, 
		int n, int& l, int& r, int& markref) {
    // std::cout << "CACHE_MARK\n";
    // the left side
    if (l > 0) {
      for (int i = 0; i < l; i++) {
	if (!mgr->marked(nodes[i])) {
	  mgr->mark(nodes[i]);
	  markref++;
	}
      }
    }
    // the right side
    if (r < n - 1) {
      for (int i = r + 1; i < n; i++) {
	if (!mgr->marked(nodes[i])) {
	  mgr->mark(nodes[i]);
	  markref++;
	}
      }
    }
  }

  // unmark all nodes in the dqueue
  void
  extcache_unmark(BMI* mgr, Support::DynamicArray<GecodeBdd>& nodes, 
		  int n, int& l, int& r, int& markref) {
    // std::cout << "CACHE_UNMARK\n";
    if (l > 0) {
      for (int i = 0; i < l; i++) {
	if (mgr->marked(nodes[i])) {
	  mgr->unmark(nodes[i]);
	  markref--;
	} 
      }
    }
    if (r < n - 1) {
      for (int i = r + 1; i < n; i++) {
	if (mgr->marked(nodes[i])) {
	  mgr->unmark(nodes[i]);
	  markref--;
	} 
      }
    }
  }


  // iterate to the next level of nodes
  void 
  extcardbounds(BMI* mgr, int& markref, GecodeBdd& c, int& n, int& l, int& r,
		bool& singleton, int& _level, 
		Support::DynamicArray<GecodeBdd>& nodes, 
		int& curmin, int& curmax) {

    // std::cout << "INCREMENT\n";
    GecodeBdd cur = BDDTOP;

    if (((l == 0) && (r == n - 1))){ // no more nodes on the stack to be iterated
      singleton = false;
      extcache_unmark(mgr, nodes, n, l, r, markref);
      assert(markref == 0);
      return;
    }

    // mark nodes under exploration
    extcache_mark(mgr, nodes, n, l, r,markref);
    if (l > 0) {
      _level++;
    }

    // NEW
    bool stackleft = false;
    while (l > 0) {

      // std::cout << "left: " << l << "\n";
      // stackprint(mgr, nodes, cards, cardvar, n, l, r, maxcard);
      stackleft = true;
      // std::cout << "stackprint done\n";
      /* 
       * if left side contains at least two nodes 
       * L: n_1 n_2 ______
       * if level(n_1) < level(n_2) move n_2 to the right end of the dqueue
       * maintain the invariant: 
       * for all nodes n_i, n_j in L: level(n_i) = level(n_j)
       * difference detected set Gecode::Bdd::UNDET
       */
      // std::cout << "check shift1-LEFT\n";
      while ((l > 1) && 
	     mgr->bddidx(nodes[l - 1]) < mgr->bddidx(nodes[l - 2])) {
	// std::cout << "shift1 left: " << l << "\n";
	int shift = l - 2;
	int norm  = l - 1;
	mgr->unmark(nodes[shift]); markref--;
	//flag = Gecode::Bdd::UNDET;
	if (r == n - 1) {
	  nodes[r] = nodes[shift];
	  mgr->mark(nodes[r]); markref++;
	} else {    
	  for (int i = r; i < n - 1; i++) {
	    nodes[i] = nodes[i + 1];
	  }
	  nodes[n - 1] = nodes[shift];
	  mgr->mark(nodes[n - 1]); markref++;
	}
	r--;
	nodes[shift] = nodes[norm];
	nodes[norm].init();
	l--;
      }
      // symmetric case
      // std::cout << "check shift2-LEFT\n";
      while ((l > 1) && 
	     mgr->bddidx(nodes[l - 1]) > mgr->bddidx(nodes[l - 2])) {
	// std::cout << "shift2 left: " << l << "\n";
	int shift = l - 1;
	mgr->unmark(nodes[shift]); markref--;
	//flag = Gecode::Bdd::UNDET;
	if (r == n - 1) {
	  nodes[r] = nodes[shift];
	  mgr->mark(nodes[r]); markref++;
	} else {    
	  for (int i = r; i < n - 1; i++) {
	    nodes[i] = nodes[i + 1];
	  }
	  nodes[n - 1] = nodes[shift];
	  mgr->mark(nodes[n - 1]); markref++;
	}
	r--;
	nodes[shift].init();
	l--;
      }
      // std::cout << "shift checked\n";
      //
      l--;
      mgr->unmark(nodes[l]);
      markref--; 
      cur = nodes[l];
      // std::cout << "cur lev: " << _level << " and card: " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";
      assert(!mgr->marked(cur));
      nodes[l].init();

      // cur is an internal node 
      if (!mgr->leaf(cur)) {
	GecodeBdd t   = mgr->iftrue(cur);
	GecodeBdd f   = mgr->iffalse(cur);
	// unsigned int cur_idx = mgr->bddidx(cur);

	bool leaf_t = mgr->leaf(t);
	bool leaf_f = mgr->leaf(f);

	if (!leaf_t) {
	  if (!mgr->marked(t)) {
	    // if we encounter different indices in the child nodes
	    // we delay inserting the larger one in the dqueue
	    nodes[r] = t;
	    mgr->lbCard(t, mgr->lbCard(cur) + 1);
	    mgr->ubCard(t, mgr->ubCard(cur) + 1);
	    // std::cout << "set card: " << mgr->lbCard(cur) + 1 << ".." << mgr->ubCard(cur) + 1 << "\n";
	    // std::cout << "test1: " << mgr->lbCard(t) << ".." << mgr->ubCard(t) << "\n";
	    // std::cout << "test2: " << mgr->lbCard(nodes[r]) << ".." << mgr->ubCard(nodes[r]) << "\n";

	    mgr->mark(nodes[r]);
	    markref++;
	    r--;
	  } else {
	    int lc = std::min(mgr->lbCard(cur) + 1, mgr->lbCard(t));
	    int uc = std::max(mgr->ubCard(cur) + 1, mgr->ubCard(t));
	    // std::cout << "min/max update to " << lc << ".." << uc << "\n";
	    mgr->lbCard(t, lc);
	    mgr->ubCard(t, uc);
	  }
	} else {
	  // leaf_t true
	  if (mgr->ctrue(t)) {
	    // std::cout << "t = true leaf\t ";
	    // std::cout << " " << mgr->lbCard(cur) + 1<< ".." << mgr->ubCard(cur) + 1 << "\n";

	    int minval = mgr->lbCard(cur) + 1;
	    int maxval = mgr->ubCard(cur) + 1;

// 	    Gecode::IntSetRanges ir(out);
// 	    Gecode::Iter::Ranges::Singleton s(minval, maxval);
// 	    Gecode::Iter::Ranges::Union<IntSetRanges, Iter::Ranges::Singleton> a(ir, s);
// 	    Gecode::IntSet ur(a);
// 	    out.update(false, ur);

	    curmin = std::min(curmin, minval);
	    curmax = std::max(curmax, maxval);
	    
	  }
	}

	if (!leaf_f) {
	  if (!mgr->marked(f)) {
	    nodes[r] = f;
	    mgr->lbCard(f, mgr->lbCard(cur));
	    mgr->ubCard(f, mgr->ubCard(cur));
	    // std::cout << "set card: " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";
	    mgr->mark(nodes[r]);
	    markref++; 
	    r--;
	  } else {
	    int lc = std::min(mgr->lbCard(cur), mgr->lbCard(f));
	    int uc = std::max(mgr->ubCard(cur), mgr->ubCard(f));
	    // std::cout << "min/max update to " << lc << ".." << uc << "\n";
	    mgr->lbCard(f, lc);
	    mgr->ubCard(f, uc);
	  }
	} else {
	  // leaf_f true
	  if (mgr->ctrue(f)) {
	    // std::cout << "f = true leaf (F)\t";
	    // std::cout << " " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";

	    int minval = mgr->lbCard(cur);
	    int maxval = mgr->ubCard(cur);

// 	    // std::cout << "current out: " << out << "\n";
// 	    Gecode::IntSetRanges ir(out);
// 	    Gecode::Iter::Ranges::Singleton s(minval, maxval);
// 	    Gecode::Iter::Ranges::Union<IntSetRanges, Iter::Ranges::Singleton> a(ir, s);
// 	    Gecode::IntSet ur(a);
// 	    // std::cout << "ur = " << ur << "\n";
// 	    out.update(false, ur);
// 	    // std::cout << "new out = " << out << "\n";

	    curmin = std::min(curmin, minval);
	    curmax = std::max(curmax, maxval);

	  }
	}
      } else {
	// cur is a leaf node
	// std::cout << "at leaf\n";
      }
      if (((l == 0) && (r == n - 1))) {
	// a singleton node can only occur at the left queue end
	// as the root node is always inserted at the left end
	singleton = true;
      }
    }

    // ensure that iterations processes alternately 
    // left and right altnerately
    if (stackleft) {
      extcache_unmark(mgr, nodes, n, l, r, markref);
      assert(markref == 0);
      // std::cout << "return 1 _level=" << _level << "\n";
      return;
    }
   
    if (r < n - 1) {
      _level++;
    }

    // process right stack half
    while (r < n - 1) {

      // std::cout << "right: " << r << "\n";
      // std::cout << "check shift1-RIGHT\n";
      while ((r < n - 2) && mgr->bddidx(nodes[r + 1]) < mgr->bddidx(nodes[r + 2])) {
	// std::cout << "shift1 right: " << l << "\n";
        int shift = r + 2;
        int norm  = r + 1;
        mgr->unmark(nodes[shift]); markref--;
        //flag = Gecode::Bdd::UNDET;
        if (l == 0) {
          nodes[l] = nodes[shift];
          mgr->mark(nodes[l]); markref++;
        } else {    
          for (int i = l; i > 0; i--) {
            nodes[i] = nodes[i - 1];
          }
          nodes[0] = nodes[shift];
          mgr->mark(nodes[0]); markref++;
        }
        l++;
        nodes[shift] = nodes[norm];
        nodes[norm].init();
        r++;
      }

      // std::cout << "check shift2-RIGHT\n";
      while ((r < n - 2) && mgr->bddidx(nodes[r + 1]) > mgr->bddidx(nodes[r + 2])) {
	// std::cout << "shift2 right: " << l << "\n";
	int shift = r + 1;
	mgr->unmark(nodes[shift]); markref--;
	//flag = Gecode::Bdd::UNDET;
	if (l == 0) {
	  nodes[l] = nodes[shift];
	  mgr->mark(nodes[l]); markref++;
	} else {
	  for (int i = l; i > 0; i--) {
	    nodes[i] = nodes[i - 1];
	  }
	  nodes[0] = nodes[shift];
	  mgr->mark(nodes[0]); markref++;
	}
	l++;
	nodes[shift].init();
	r++;
      }


      // check whether right-hand side nodes has fixed vars
      r++;
      mgr->unmark(nodes[r]);
      markref--; 
      cur = nodes[r];
      // std::cout << "cur lev: " << _level << " and card: " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";
      assert(!mgr->marked(cur));

      nodes[r].init();
      // cur is internal node, that is cur is neither BDDBOT nor BDDTOP
      if (!mgr->leaf(cur)) {
	GecodeBdd t   = mgr->iftrue(cur);
	GecodeBdd f   = mgr->iffalse(cur);

	bool leaf_t = mgr->leaf(t);
	bool leaf_f = mgr->leaf(f);

	if (!leaf_t) {
	  if (!mgr->marked(t)) {
	    nodes[l] = t;
	    mgr->lbCard(t, mgr->lbCard(cur) + 1);
	    mgr->ubCard(t, mgr->ubCard(cur) + 1);
	    // std::cout << "set card: " << mgr->lbCard(cur) + 1 << ".." << mgr->ubCard(cur) + 1 << "\n";
	    mgr->mark(nodes[l]);
	    markref++;
	    l++;
	  } else {
	    int lc = std::min(mgr->lbCard(cur) + 1, mgr->lbCard(t));
	    int uc = std::max(mgr->ubCard(cur) + 1, mgr->ubCard(t));
	    // std::cout << "min/max update to " << lc << ".." << uc << "\n";
	    mgr->lbCard(t, lc);
	    mgr->ubCard(t, uc);
	  }

	} else {
	  // leaf_t true
	  if (mgr->ctrue(t)) {
	    // std::cout << "t = true leaf\t ";
	    // std::cout << " " << mgr->lbCard(cur) + 1 << ".." << mgr->ubCard(cur) + 1 << "\n";
	    
	    int minval = mgr->lbCard(cur) + 1;
	    int maxval = mgr->ubCard(cur) + 1;

// 	    Gecode::IntSetRanges ir(out);
// 	    Gecode::Iter::Ranges::Singleton s(minval, maxval);
// 	    Gecode::Iter::Ranges::Union<IntSetRanges, Iter::Ranges::Singleton> a(ir, s);
// 	    Gecode::IntSet ur(a);
// 	    out.update(false, ur);

	    curmin = std::min(curmin, minval);
	    curmax = std::max(curmax, maxval);

	  } 
	}

	if (!leaf_f) {
	  if (!mgr->marked(f)) {
	    nodes[l] = f;
	    mgr->lbCard(f, mgr->lbCard(cur));
	    mgr->ubCard(f, mgr->ubCard(cur));
	    // std::cout << "set card: " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";
	    mgr->mark(nodes[l]);
	    markref++;
	    l++;
	  } else {
	    int lc = std::min(mgr->lbCard(cur), mgr->lbCard(f));
	    int uc = std::max(mgr->ubCard(cur), mgr->ubCard(f));
	    // std::cout << "min/max update to " << lc << ".." << uc << "\n";
	    mgr->lbCard(f, lc);
	    mgr->ubCard(f, uc);
	  }
	}  else {
	  // leaf_f true
	  if (mgr->ctrue(f)) {
	    // std::cout << "f = true leaf (S) \t ";
	    // std::cout << " " << mgr->lbCard(cur) << ".." << mgr->ubCard(cur) << "\n";

	    int minval = mgr->lbCard(cur);
	    int maxval = mgr->ubCard(cur);

// 	    Gecode::IntSetRanges ir(out);
// 	    Gecode::Iter::Ranges::Singleton s(minval, maxval);
// 	    Gecode::Iter::Ranges::Union<IntSetRanges, Iter::Ranges::Singleton> a(ir, s);
// 	    Gecode::IntSet ur(a);
// 	    out.update(false, ur);

	    curmin = std::min(curmin, minval);
	    curmax = std::max(curmax, maxval);

	  } 
	}
      }
      if (((l == 0) && (r == n - 1))) {
	// a singleton node can only occur at the left queue end
	// as the root node is always inserted at the left end
	singleton = true;
      }

    } // end processing right stack

    // stackprint(mgr, nodes, cards, cardvar, n, l, r, maxcard);
    extcache_unmark(mgr, nodes, n, l, r, markref);
    assert(markref == 0);
    // std::cout << "return2 _level=" << _level << "\n";

    // std::cout << " end test " << "\n";
    // std::cout << out << "\n";

  } // end increment op



  // START EXT CARD INFO
  void getcardbounds(BMI* mgr, GecodeBdd& c, int& curmin, int& curmax) {
    // try to extract the card info
    int markref = 0;
    int csize   = mgr->bddsize(c);
    int l       = 0;
    int r       = csize - 1;
    bool singleton = (csize == 1);
    int _level = -1;
    Support::DynamicArray<GecodeBdd> nodes(csize);
    
    // the given ROBDD c has internal nodes
    if (!mgr->leaf(c)) {

      for (int i = csize; i--; ) {       
        nodes[i].init();
      }

      nodes[l] = c;
      mgr->lbCard(nodes[l], 0);
      mgr->ubCard(nodes[l], 0);
      mgr->mark(nodes[l]);
      markref++;
      l++;      
    } else {
      // \todo FIXME implement cardoutput for leaf!
      std::cerr << "NOT IMPLEMENTED CARDOUTPUT FOR LEAF !\n";
    }

    // contains the set of all allowed cardinalities
    // deprecated IntSet out = IntSet::empty;

    if (! ((l == 0) && (r == csize - 1)) // ! empty
        || singleton) {
      while (! ((l == 0) && (r == csize - 1)) || singleton) {
        Gecode::extcardbounds(mgr, markref, c, csize, l, r, 
                              singleton, _level, nodes, 
                              curmin, curmax// , out
			      );
      }
    }

    // curmin and curmax are set locally (arguments passed by reference)
//     curmin = out.min();
//     curmax = out.max();
    // std::cout << "possible values for cardinality: \t" << out << "\n";
  }
  /// END NEW STRUCTURE

  // END CARDINALITY COUNTING FOR EXTRACTING BOUNDS

  
  GecodeBdd lex_lb(BMI*& mgr, GecodeBdd& remain, GecodeBdd& boundvars, 
	      unsigned int n, unsigned int& offset, unsigned int& range) {
    // std::cout << "lexlb: " << "n= " << n << " offset = " << offset << " range = " << range << "\n";
    // std::cout << "bounds = \n" << boundvars << "\n";
    // skip variables in the boolean vector that 
    // belong to the fixed part of the bdd
    if (!mgr->leaf(boundvars)) {
      unsigned int idx = mgr->bddidx(boundvars) - offset;
      while ( n == idx ) {
	n++;
	boundvars = mgr->iftrue(boundvars);
	if (mgr->leaf(boundvars)) {
	  break;
	}
	idx = mgr->bddidx(boundvars) - offset;
      }
    }
    // std::cout << "skipped: " << "n= " << n << " offset = " << offset << " range = " << range << "\n";
    // std::cout << "bounds = \n" << boundvars << "\n";
    if (n == range || mgr->ctrue(remain)) {
      // std::cout << "result &= TOP\n";
      return BDDTOP;
    }
    
    assert(!mgr->leaf(remain));
    GecodeBdd recresult = BDDTOP;
    unsigned int cidx = mgr->bddidx(remain);
    GecodeBdd cur     = mgr->bddpos(cidx);
    if (n == cidx - offset) {
      // std::cout << "n= " << n << "idx = " << mgr->bddidx(remain) - offset << "\n";

      GecodeBdd t   = mgr->iftrue(remain);
      GecodeBdd f   = mgr->iffalse(remain);

      if (mgr->cfalse(f)) {
	// std::cout << "else is false\n";
	return (cur & lex_lb(mgr, t, boundvars, n + 1, offset, range));
      } else {
	// std::cout << "else is not false\n";
	return (cur | (!cur & lex_lb(mgr, f, boundvars, n + 1, offset, range)));
      }
    } else {
      // std::cout << "different vector and bdd\n";     
      return (cur | (!cur & lex_lb(mgr, remain, boundvars, n + 1, offset, range)));
    }
  }


  GecodeBdd lex_ub(BMI*& mgr, GecodeBdd& remain, GecodeBdd& boundvars, 
	      unsigned int n, unsigned int& offset, unsigned int& range) {
    // std::cout << "lexub: " << "n= " << n << " offset = " << offset << " range = " << range << "\n";
    // skip variables in the boolean vector that 
    // belong to the fixed part of the bdd
    if (!mgr->leaf(boundvars)) {
      unsigned int idx = mgr->bddidx(boundvars) - offset;
      while ( n == idx ) {
	n++;
	boundvars = mgr->iftrue(boundvars);
	if (mgr->leaf(boundvars)) {
	  break;
	}
	idx = mgr->bddidx(boundvars) - offset;
      }
    }

    if (n == range || mgr->ctrue(remain)) {
      return BDDTOP;
    }
    
    assert(!mgr->leaf(remain));
    unsigned int cidx = mgr->bddidx(remain);
    GecodeBdd cur     = mgr->bddpos(cidx);
    GecodeBdd recresult = BDDTOP;

    if (n == cidx - offset) {
      GecodeBdd t   = mgr->iftrue(remain);
      GecodeBdd f   = mgr->iffalse(remain);

      if (mgr->cfalse(t)) {
	return !cur & lex_ub(mgr, f, boundvars,  n + 1, offset, range);
      } else {
	return (!cur | (cur & lex_ub(mgr, t, boundvars,  n + 1, offset, range)));
      }
    } else {     
      return (!cur | (cur & lex_ub(mgr, remain, boundvars,  n + 1, offset, range)));
    }
  }

}

// STATISTICS: bdd-support
