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
#include "gecode/bdd/var/imp-body.icc"

namespace Gecode { namespace Bdd {

    /// copy bddvar
    forceinline
    BddVarImp::BddVarImp(Space* home, bool share, BddVarImp& x)
      : BddVarImpBase(home,share,x), 
	domain(x.domain), min(x.min), max(x.max),
	_offset(x._offset), assignment(false) {
      mgr = x.mgr;
    }
  
    BddVarImp*
    BddVarImp::perform_copy(Space* home, bool share) {
      BddVarImp* ptr = new (home) BddVarImp(home,share,*this);
      return ptr;
    }

    Reflection::Arg*
    BddVarImp::spec(Space* home, Reflection::VarMap& m) {
      // \todo FIXME implemente reflection
      return NULL;
    }

  
#define OLDPRINTF(b) (mgr->cfalse(b) ? 0 : mgr->bddidx(b))
#define OLDPRINTT(b) std::cout << (mgr->ctrue(b) ? 1 : OLDPRINTF(b)) <<" ";
#define OLDPRINTBDD(b) std::cout << mgr->bddidx(b) << " " ; OLDPRINTT(mgr->iftrue(b)) OLDPRINTT(mgr->iffalse(b)) std::cout << "";
  

#define PRINTF(b) (mgr->cfalse(b) ? 0 : b.getroot())
#define PRINTT(b) std::cout << (mgr->ctrue(b) ? 1 : PRINTF(b)) <<" ";
#define PRINTBDD(b) std::cout << b.getroot() << " " ; PRINTT(mgr->iftrue(b)) PRINTT(mgr->iffalse(b)) std::cout << "\n";

#define STACKPRINT//					\
// std::cout << "stack is= ";				\
//   for (int i = 0; i < n; i++){			\
//    if (mgr->cfalse(nodes[i])) {			\
// std::cout << "x";					\
//     } else {						\
//        OLDPRINTBDD(nodes[i])				\
//     }						\
//   }							\
// std::cout << "LR("<<l << "," << r << ")\n"; //	\
// // //   for (int i = 0; i < m; i++){			\
// // //     assert(flags[i]>= -1);			\
// // //     assert(flags[i] <= 5);			\
// // // std::cout << flags[i] << " ";			\
// // //   }						\
// // // std::cout << "\n";

    // initialize the iterator structure
    void
    BddIterator::init(BMI* manager, const GecodeBdd& b) {
      markref = 0;
      mgr = manager;
      c = b;
      n = mgr->bddsize(c);
      l = 0;
      r = n - 1;
      bypassed = false;
      onlyleaves = false;
      singleton = (n == 1);
      _level = -1;

      if (!mgr->leaf(c)) {
	Support::DynamicArray<GecodeBdd> dummy(n);
	nodes = dummy;
      
	for (int i = n; i--; ){
	  nodes[i].init();
	}
	STACKPRINT;

	assert(!mgr->leaf(c));
	// insert bdd root into dqueue
	nodes[l] = c;
	mgr->mark(nodes[l]);
	markref++;
	l++;      
      }
      if (operator()()) {
	operator++();
      }
    }

    // mark all nodes in the dqueue
    void
    BddIterator::cache_mark(void) {
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
    BddIterator::cache_unmark(void) {
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
    BddIterator::operator++(void) {
      if (empty()) { // no more nodes on the stack to be iterated
	singleton = false;
	cache_unmark();
	assert(markref == 0);
	return;
      }

      // a true path ending on a previous level was detected
      // hence the current level cannot be fixed
      // description of onlyleaves and bypassed in var.icc
      if (onlyleaves || bypassed) {
	flag = UNDET;
      } else {
	// reset flag
	flag = INIT;
      }

      // mark nodes under exploration
      cache_mark();
      if (l > 0) {
	_level++;
      }

      bool stackleft = false;
      while (l > 0) {
	STACKPRINT;
	stackleft = true;

	/* 
	 * if left side contains at least two nodes 
	 * L: n_1 n_2 ______
	 * if level(n_1) < level(n_2) move n_2 to the right end of the dqueue
	 * maintain the invariant: 
	 * for all nodes n_i, n_j in L: level(n_i) = level(n_j)
	 * difference detected set UNDET
	 */
	while ((l > 1) && 
	       mgr->bddidx(nodes[l - 1]) < mgr->bddidx(nodes[l - 2])) {
	  int shift = l - 2;
	  int norm  = l - 1;
	  mgr->unmark(nodes[shift]); markref--;
	  flag = UNDET;
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
	while ((l > 1) && 
	       mgr->bddidx(nodes[l - 1]) > mgr->bddidx(nodes[l - 2])) {
	  int shift = l - 1;
	  mgr->unmark(nodes[shift]); markref--;
	  flag = UNDET;
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

	l--;
	mgr->unmark(nodes[l]);
	markref--; 
	cur = nodes[l];
	assert(!mgr->marked(cur));
	nodes[l].init();

	// cur is an internal node, 
	// that is nor true nor else branch are leaves
	if (!mgr->leaf(cur)) {
	  GecodeBdd t   = mgr->iftrue(cur);
	  GecodeBdd f   = mgr->iffalse(cur);
	  // unsigned int cur_idx = mgr->bddidx(cur);
	  bool fixed_glb       = mgr->cfalse(f);
	  bool fixed_not_lub   = mgr->cfalse(t);

	  bool leaf_t = mgr->leaf(t);
	  bool leaf_f = mgr->leaf(f);
	
	  if (flag != UNDET) {
	    // REMOVED FROM LUB
	    if (fixed_not_lub) {
	      if (flag > INIT && flag != FIX_NOT_LUB) {
		// not the same status on all paths
		flag = UNDET;
	      } else {
		flag = FIX_NOT_LUB;
	      }
	    } else {
	      // INSIDE GLB
	      if (fixed_glb) {
		if (flag > INIT && flag != FIX_GLB) {
		  // not the same status on all paths
		  flag = UNDET;
		} else {
		  flag = FIX_GLB;
		}
	      } else {
		if (flag > INIT && flag != FIX_UNKNOWN) {
		  // not the same status on all paths
		  flag = UNDET;
		} else {
		  // not fixed on all paths leading to true
		  flag = FIX_UNKNOWN;
		}
	      }
	    }
	  }
	
	  if (!leaf_t) {
	    if (!mgr->marked(t)) {
	      // if we encounter different indices in the child nodes
	      // we delay inserting the larger one in the dqueue
	      nodes[r] = t;
	      mgr->mark(nodes[r]);
	      markref++;
	      r--;
	    }
	    // else child leads directly to true 
	    if (mgr->ctrue(f)) {
	      bypassed = true;
	    } 
	  } else {
	    // larger levels cannot be fixed as they do not lie
	    // on all paths leading towards a true leaf
	    // leaf_t && leaf_f  => (mgr->ctrue(t) || mgr->ctrue(f))
	    onlyleaves |= leaf_f;
	  }

	  if (!leaf_f) {
	    if (!mgr->marked(f)) {
	      nodes[r] = f;
	      mgr->mark(nodes[r]);
	      markref++; 
	      r--;
	    }
	    if (mgr->ctrue(t)) {
	      // std::cout << "bypassed f left\n";
	      bypassed = true;
	      // std::cout << "bypass " << "["<<_level<<"]= UNDET\n";
	      // flag = UNDET;
	      // break;
	    } 
	  }
	}
	if (empty()) {
	  // a singleton node can only occur at the left queue end
	  // as the root node is always inserted at the left end
	  singleton = true;
	}
      }

      // ensure that iterations processes alternately 
      // left and right altnerately
      if (stackleft) {
	cache_unmark();
	assert(markref == 0);
	return;
      }
    
      if (r < n - 1) {
	_level++;
      }

      // process right stack half
      while (r < n - 1) {
	STACKPRINT
	  while ((r < n - 2) && mgr->bddidx(nodes[r + 1]) < mgr->bddidx(nodes[r + 2])) {
	    int shift = r + 2;
	    int norm  = r + 1;
	    mgr->unmark(nodes[shift]); markref--;
	    flag = UNDET;
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
	while ((r < n - 2) && mgr->bddidx(nodes[r + 1]) > mgr->bddidx(nodes[r + 2])) {
	  int shift = r + 1;
	  mgr->unmark(nodes[shift]); markref--;
	  flag = UNDET;
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
	assert(!mgr->marked(cur));

	nodes[r].init();
	// cur is internal node, that is cur is neither BDDBOT nor BDDTOP
	if (!mgr->leaf(cur)) {
	  GecodeBdd t   = mgr->iftrue(cur);
	  GecodeBdd f   = mgr->iffalse(cur);

	  bool fixed_glb = mgr->cfalse(f);
	  bool fixed_not_lub = mgr->cfalse(t);

	  bool leaf_t = mgr->leaf(t);
	  bool leaf_f = mgr->leaf(f);

	  if (flag != UNDET) {
	    if (fixed_not_lub) {
	      if (flag > INIT && flag != FIX_NOT_LUB) {
		flag = UNDET;
	      } else {
		flag = FIX_NOT_LUB;
	      }
	    } else {
	      if (fixed_glb) {
		if (flag > INIT && flag != FIX_GLB) {
		  flag = UNDET;
		} else {
		  flag = FIX_GLB;
		}
	      } else {
		if (flag > INIT && flag != FIX_UNKNOWN) {
		  flag = UNDET;
		} else {
		  flag = FIX_UNKNOWN;
		}
	      }
	    }
	  }

	  if (!leaf_t) {
	    if (!mgr->marked(t)) {
	      nodes[l] = t;
	      mgr->mark(nodes[l]);
	      markref++;
	      l++;
	    }

	    // bypassed t right
	    if (mgr->ctrue(f)) {
	      bypassed = true;
	    }
	  } else {
	    // if on the same level one node has internal childs and
	    // the other one has only leaf childs
	    // then the next level cannot be fixed
	    onlyleaves |= leaf_f;
	  }

	  if (!leaf_f) {
	    if (!mgr->marked(f)) {
	      nodes[l] = f;
	      mgr->mark(nodes[l]);
	      markref++;
	      l++;
	    } 
	    // bypassed f right
	    if (mgr->ctrue(t)) {
	      bypassed = true;
	    }
	  }
	}
	if (empty()) {
	  // a singleton node can only occur at the left queue end
	  // as the root node is always inserted at the left end
	  singleton = true;
	}
      } // end processing right stack

      STACKPRINT;
      cache_unmark();
      assert(markref == 0);
    } // end increment op
  
  }}

// STATISTICS: bdd-var
