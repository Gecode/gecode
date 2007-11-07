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
#include "gecode/cpltset/var/imp-body.icc"

namespace Gecode { namespace CpltSet {

  // copy bddvar
  forceinline
  CpltSetVarImp::CpltSetVarImp(Space* home, bool share, CpltSetVarImp& x)
    : CpltSetVarImpBase(home,share,x), 
      domain(x.domain), min(x.min), max(x.max),
      _offset(x._offset), _assigned(false) {
  }

  CpltSetVarImp*
  CpltSetVarImp::perform_copy(Space* home, bool share) {
    CpltSetVarImp* ptr = new (home) CpltSetVarImp(home,share,*this);
    return ptr;
  }

  Reflection::Arg*
  CpltSetVarImp::spec(Space* home, Reflection::VarMap& m) {
    // \todo FIXME implemente reflection
    return NULL;
  }

  // initialize the iterator structure
  void
  BddIterator::init(const bdd& b) {
    markref = 0;
    c = b;
    n = manager.bddsize(c);
    l = 0;
    r = n - 1;
    bypassed = false;
    onlyleaves = false;
    singleton = (n == 1);
    _level = -1;

    if (!manager.leaf(c)) {
      SharedArray<bdd> dummy(n);
      nodes = dummy;
    
      for (int i = n; i--; ){
        new (&nodes[i]) bdd;
        nodes[i].init();
      }

      assert(!manager.leaf(c));
      // insert bdd root into dqueue
      nodes[l] = c;
      manager.mark(nodes[l]);
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
        if (!manager.marked(nodes[i])) {
          manager.mark(nodes[i]);
          markref++;
        }
      }
    }
    // the right side
    if (r < n - 1) {
      for (int i = r + 1; i < n; i++) {
        if (!manager.marked(nodes[i])) {
          manager.mark(nodes[i]);
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
        if (manager.marked(nodes[i])) {
          manager.unmark(nodes[i]);
          markref--;
        } 
      }
    }
    if (r < n - 1) {
      for (int i = r + 1; i < n; i++) {
        if (manager.marked(nodes[i])) {
          manager.unmark(nodes[i]);
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
             manager.bddidx(nodes[l - 1]) < manager.bddidx(nodes[l - 2])) {
        int shift = l - 2;
        int norm  = l - 1;
        manager.unmark(nodes[shift]); markref--;
        flag = UNDET;
        if (r == n - 1) {
          nodes[r] = nodes[shift];
          manager.mark(nodes[r]); markref++;
        } else {    
          for (int i = r; i < n - 1; i++) {
            nodes[i] = nodes[i + 1];
          }
          nodes[n - 1] = nodes[shift];
          manager.mark(nodes[n - 1]); markref++;
        }
        r--;
        nodes[shift] = nodes[norm];
        nodes[norm].init();
        l--;
      }
      // symmetric case
      while ((l > 1) && 
             manager.bddidx(nodes[l - 1]) > manager.bddidx(nodes[l - 2])) {
        int shift = l - 1;
        manager.unmark(nodes[shift]); markref--;
        flag = UNDET;
        if (r == n - 1) {
          nodes[r] = nodes[shift];
          manager.mark(nodes[r]); markref++;
        } else {    
          for (int i = r; i < n - 1; i++) {
            nodes[i] = nodes[i + 1];
          }
          nodes[n - 1] = nodes[shift];
          manager.mark(nodes[n - 1]); markref++;
        }
        r--;
        nodes[shift].init();
        l--;
      }

      l--;
      manager.unmark(nodes[l]);
      markref--; 
      cur = nodes[l];
      assert(!manager.marked(cur));
      nodes[l].init();

      // cur is an internal node, 
      // that is nor true nor else branch are leaves
      if (!manager.leaf(cur)) {
        bdd t   = manager.iftrue(cur);
        bdd f   = manager.iffalse(cur);
        // unsigned int cur_idx = manager.bddidx(cur);
        bool fixed_glb       = manager.cfalse(f);
        bool fixed_not_lub   = manager.cfalse(t);

        bool leaf_t = manager.leaf(t);
        bool leaf_f = manager.leaf(f);
      
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
          if (!manager.marked(t)) {
            // if we encounter different indices in the child nodes
            // we delay inserting the larger one in the dqueue
            nodes[r] = t;
            manager.mark(nodes[r]);
            markref++;
            r--;
          }
          // else child leads directly to true 
          if (manager.ctrue(f)) {
            bypassed = true;
          } 
        } else {
          // larger levels cannot be fixed as they do not lie
          // on all paths leading towards a true leaf
          // leaf_t && leaf_f  => (manager.ctrue(t) || manager.ctrue(f))
          onlyleaves |= leaf_f;
        }

        if (!leaf_f) {
          if (!manager.marked(f)) {
            nodes[r] = f;
            manager.mark(nodes[r]);
            markref++; 
            r--;
          }
          if (manager.ctrue(t)) {
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
        while ((r < n - 2) && manager.bddidx(nodes[r + 1]) < 
                              manager.bddidx(nodes[r + 2])) {
          int shift = r + 2;
          int norm  = r + 1;
          manager.unmark(nodes[shift]); markref--;
          flag = UNDET;
          if (l == 0) {
            nodes[l] = nodes[shift];
            manager.mark(nodes[l]); markref++;
          } else {    
            for (int i = l; i > 0; i--) {
              nodes[i] = nodes[i - 1];
            }
            nodes[0] = nodes[shift];
            manager.mark(nodes[0]); markref++;
          }
          l++;
          nodes[shift] = nodes[norm];
          nodes[norm].init();
          r++;
        }
      while ((r < n - 2) && manager.bddidx(nodes[r + 1]) > 
                            manager.bddidx(nodes[r + 2])) {
        int shift = r + 1;
        manager.unmark(nodes[shift]); markref--;
        flag = UNDET;
        if (l == 0) {
          nodes[l] = nodes[shift];
          manager.mark(nodes[l]); markref++;
        } else {
          for (int i = l; i > 0; i--) {
            nodes[i] = nodes[i - 1];
          }
          nodes[0] = nodes[shift];
          manager.mark(nodes[0]); markref++;
        }
        l++;
        nodes[shift].init();
        r++;
      }
      // check whether right-hand side nodes has fixed vars
      r++;
      manager.unmark(nodes[r]);
      markref--; 
      cur = nodes[r];
      assert(!manager.marked(cur));

      nodes[r].init();
      // cur is internal node, that is cur is neither
      // bdd_false() nor bdd_true()
      if (!manager.leaf(cur)) {
        bdd t   = manager.iftrue(cur);
        bdd f   = manager.iffalse(cur);

        bool fixed_glb = manager.cfalse(f);
        bool fixed_not_lub = manager.cfalse(t);

        bool leaf_t = manager.leaf(t);
        bool leaf_f = manager.leaf(f);

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
          if (!manager.marked(t)) {
            nodes[l] = t;
            manager.mark(nodes[l]);
            markref++;
            l++;
          }

          // bypassed t right
          if (manager.ctrue(f)) {
            bypassed = true;
          }
        } else {
          // if on the same level one node has internal childs and
          // the other one has only leaf childs
          // then the next level cannot be fixed
          onlyleaves |= leaf_f;
        }

        if (!leaf_f) {
          if (!manager.marked(f)) {
            nodes[l] = f;
            manager.mark(nodes[l]);
            markref++;
            l++;
          } 
          // bypassed f right
          if (manager.ctrue(t)) {
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

    cache_unmark();
    assert(markref == 0);
  } // end increment op
  
}}

// STATISTICS: cpltset-var
