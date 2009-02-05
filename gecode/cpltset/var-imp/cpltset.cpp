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

#include <gecode/cpltset.hh>

namespace Gecode { namespace CpltSet {

  CpltSetVarImp::CpltSetVarImp(Space& home,
                               int glbMin, int glbMax, int lubMin, int lubMax,
                               unsigned int cardMin, unsigned int cardMax)
    : CpltSetVarImpBase(home), domain(bdd_true()),
      min(lubMin), max(lubMax), _assigned(false) {

    IntSet glb(glbMin, glbMax);
    IntSet lub(lubMin, lubMax);
    testConsistency(glb, lub, cardMin, cardMax, "CpltSetVarImp");

    _offset = manager.allocate(tableWidth());

    for (int i = glbMax; i >= glbMin; i--) {
      domain &= element(i - min);
    }

    unsigned int range = tableWidth();
    domain &= cardcheck(home, range, _offset,
                        static_cast<int> (cardMin),
                        static_cast<int> (cardMax));

    assert(!manager.cfalse(domain));
  }

  CpltSetVarImp::CpltSetVarImp(Space& home,
                               int glbMin, int glbMax, const IntSet& lubD,
                               unsigned int cardMin, unsigned int cardMax)
    : CpltSetVarImpBase(home), domain(bdd_true()),
      min(lubD.min()), max(lubD.max()),
      _assigned(false) {

    IntSet glb(glbMin, glbMax);
    testConsistency(glb, lubD, cardMin, cardMax, "CpltSetVarImp");

    IntSetRanges lub(lubD);
    Iter::Ranges::ToValues<IntSetRanges> lval(lub);
    Iter::Ranges::ValCache<Iter::Ranges::ToValues<IntSetRanges> > vc(lval);

    _offset = manager.allocate(tableWidth());
    vc.last();

    int c = glbMax;
    for (int i = max; i >= min; i--) {
      if (i != vc.val()) {
        if (c >= glbMin && c == i) {
          throw CpltSet::GlbLubSpecNoSubset("CpltSetVarImp");
        }
        domain &= elementNeg(i - min);
      } else {
        if (c >= glbMin && c == i) {
          domain &= element(i - min);
          c--;
        }
        --vc;
      }
    }

    unsigned int range = tableWidth();
    domain &= cardcheck(home, range, _offset,
                        static_cast<int> (cardMin),
                        static_cast<int> (cardMax));

    assert(!manager.cfalse(domain));
  }

  CpltSetVarImp::CpltSetVarImp(Space& home,
                               const IntSet& glbD, int lubMin, int lubMax,
                               unsigned int cardMin, unsigned int cardMax)
    : CpltSetVarImpBase(home), domain(bdd_true()),
      min(lubMin), max(lubMax), _assigned(false) {

    IntSet lub(lubMin, lubMax);
    testConsistency(glbD, lub, cardMin, cardMax, "CpltSetVarImp");

    IntSetRanges glb(glbD);
    Iter::Ranges::ToValues<IntSetRanges> gval(glb);
    Iter::Ranges::ValCache<Iter::Ranges::ToValues<IntSetRanges> > vc(gval);

    vc.last();
    _offset = manager.allocate(tableWidth());

    for (vc.last(); vc(); --vc) {
      domain &= element(vc.val() - min);
    }

    unsigned int range = tableWidth();
    domain &= cardcheck(home, range, _offset,
                        static_cast<int> (cardMin),
                        static_cast<int> (cardMax));

    assert(!manager.cfalse(domain));
  }

  CpltSetVarImp::CpltSetVarImp(Space& home,
                               const IntSet& glbD,const IntSet& lubD,
                               unsigned int cardMin, unsigned int cardMax)
    : CpltSetVarImpBase(home), domain(bdd_true()),
      min(lubD.min()), max(lubD.max()),
      _assigned(false) {

    testConsistency(glbD, lubD, cardMin, cardMax, "CpltSetVarImp");

    IntSetRanges glb(glbD);
    Iter::Ranges::ToValues<IntSetRanges> gval(glb);
    Iter::Ranges::ValCache<Iter::Ranges::ToValues<IntSetRanges> >
      vcglb(gval);
    IntSetRanges lub(lubD);
    Iter::Ranges::ToValues<IntSetRanges> lval(lub);
    Iter::Ranges::ValCache<Iter::Ranges::ToValues<IntSetRanges> >
      vclub(lval);

    _offset = manager.allocate(tableWidth());

    vcglb.last();
    vclub.last();

    for (int i = max; i >= min; i--) {
      if (i != vclub.val()) {
        if (vcglb() && vcglb.val() == i) {
          throw CpltSet::GlbLubSpecNoSubset("CpltSetVarImp");
        }
        domain &= elementNeg(i - min);
      } else {
        if (vcglb() && vcglb.val() == i) {
          domain &= element(i - min);
        }
        --vclub;
      }
    }

    unsigned int range = tableWidth();
    domain &= cardcheck(home,range, _offset,
                        static_cast<int> (cardMin),
                        static_cast<int> (cardMax));

    assert(!manager.cfalse(domain));
  }

  // copy bddvar
  CpltSetVarImp::CpltSetVarImp(Space& home, bool share, CpltSetVarImp& x)
    : CpltSetVarImpBase(home,share,x),
      domain(x.domain), min(x.min), max(x.max),
      _offset(x._offset), _assigned(false) {
  }

  inline void
  CpltSetVarImp::dispose(Space&) {
    manager.dispose(domain);
    // only variables with nodes in the table need to be disposed
    if (!(_offset == 0 &&
          min == Set::Limits::min &&
          max == Set::Limits::max)
        ) {
      manager.dispose(_offset, (int) tableWidth());
    }
  }

  CpltSetVarImp*
  CpltSetVarImp::perform_copy(Space& home, bool share) {
    CpltSetVarImp* ptr = new (home) CpltSetVarImp(home,share,*this);
    return ptr;
  }

  // a variable is only assigned if all bdd variables representing
  // the elements of the set have either a constant false or a constant true
  bool
  CpltSetVarImp::assigned(void) {
    if (!_assigned) {
      // (C1) there is only one solution (i.e. only one path)
      bool cond1 = (unsigned int) manager.bddpath(domain) == 1;
      // (C2) the solution talks about all elements
      //      (i.e. the number of nodes used for the bdd uses the bdd nodes
      //      of all elements for the CpltSetVar)
      bool cond2 = (unsigned int) manager.bddsize(domain) == tableWidth();
      _assigned = cond1 && cond2;
    }
    return _assigned;
  }

  ModEvent
  CpltSetVarImp::intersect(Space& home, bdd& d) {
    bool assigned_before = assigned();
    bdd olddom = domain;
    domain &= d;

    bool assigned_new = assigned();
    if (manager.cfalse(domain))
      return ME_CPLTSET_FAILED;

    ModEvent me = ME_CPLTSET_NONE;
    if (assigned_new) {
      if (assigned_before) {
        me = ME_CPLTSET_NONE;
        return me;
      } else {
        me =  ME_CPLTSET_VAL;
      }
      Delta d;
      return notify(home, me, d);
    } else {
      if (olddom != domain) {
        me = ME_CPLTSET_DOM;
        Delta d;
        return notify(home, me, d);
      }
    }
    return me;
  }

  ModEvent
  CpltSetVarImp::exclude(Space& home, int a, int b) {
    // values are already excluded
    if (a > max  || b < min)
      return ME_CPLTSET_NONE;

    int mi = std::max(min, a);
    int ma = std::min(b, max);

    bdd notinlub  = bdd_true();
    // get the negated bdds for value i in [a..b]
    for (int i = ma; i >= mi; i--)
      notinlub &= elementNeg(i - min);

    return intersect(home, notinlub);
  }

  ModEvent
  CpltSetVarImp::include(Space& home, int a, int b) {
    if (a < min || b > max)
      return ME_CPLTSET_FAILED;

    bdd in_glb  = bdd_true();
    for (int i = b; i >= a; i--)
      in_glb &= element(i - min);

    return intersect(home, in_glb);
  }

  ModEvent
  CpltSetVarImp::nq(Space& home, int a, int b) {
    if (b < min || a > max)
      return ME_CPLTSET_NONE;

    Iter::Ranges::Singleton m(a, b);
    bdd ass = !(iterToBdd(m));
    return intersect(home, ass);
  }

  ModEvent
  CpltSetVarImp::eq(Space& home, int a, int b) {
    if (b < min || a > max)
      return ME_CPLTSET_FAILED;

    Iter::Ranges::Singleton m(a, b);
    bdd ass = iterToBdd(m);
    return intersect(home, ass);
  }

  ModEvent
  CpltSetVarImp::intersect(Space& home, int a, int b) {
    ModEvent me_left = exclude(home, Set::Limits::min, a - 1);

    if (me_failed(me_left) || me_left == ME_CPLTSET_VAL)
      return me_left;

    ModEvent me_right = exclude(home, b + 1, Set::Limits::max);

    if (me_failed(me_right) || me_right == ME_CPLTSET_VAL)
      return me_right;

    if (me_left > 0 || me_right > 0)
      return ME_CPLTSET_DOM;

    return ME_CPLTSET_NONE;
  }

  ModEvent
  CpltSetVarImp::cardinality(Space& home, int l, int u) {
    unsigned int maxcard = tableWidth();
    // compute the cardinality formula
    bdd c = cardcheck(home, maxcard, _offset, l, u);
    return intersect(home, c);
  }

  bool
  CpltSetVarImp::knownIn(int v) const {
    if (manager.ctrue(domain))
      return false;
    if (v<min || v>max)
      return false;
    bdd bv = manager.negbddpos(_offset+v-min);
    return (manager.cfalse(domain & bv));
  }

  bool
  CpltSetVarImp::knownOut(int v) const {
    if (manager.ctrue(domain))
      return false;
    if (v<min || v>max)
      return false;
    bdd bv = manager.bddpos(_offset+v-min);
    return (manager.cfalse(domain & bv));
  }

  unsigned int
  CpltSetVarImp::lubSize(void) const {
    if (manager.ctrue(domain))
      return tableWidth();

    BddIterator iter(domain);
    int out = 0;
    while (iter()) {
      if (iter.status() == FIX_NOT_LUB)
        out++;
      ++iter;
    }
    return tableWidth() - out;
  }

  int
  CpltSetVarImp::glbMin(void) const {
    if (manager.ctrue(domain))
      return initialLubMin();

    BddIterator iter(domain);
    while (iter()) {
      if (iter.status() == FIX_GLB) {
        int idx = iter.label() - offset();
        return initialLubMin() + idx;
      }
      ++iter;
    }
    return MIN_OF_EMPTY;
  }

  int
  CpltSetVarImp::glbMax(void) const {
    if (manager.ctrue(domain))
      return initialLubMax();

    BddIterator iter(domain);
    int lastglb = -1;
    while (iter()) {
      if (iter.status() == FIX_GLB) {
        int idx = iter.label() - offset();
        lastglb = initialLubMin() + idx;
      }
      ++iter;
    }
    return (lastglb == -1) ? MAX_OF_EMPTY : lastglb;
  }

  unsigned int
  CpltSetVarImp::glbSize(void) const {
    if (manager.ctrue(domain)) { return 0; }
    BddIterator iter(domain);
    int size = 0;
    while (iter()) {
      if (iter.status() == FIX_GLB) { size++; }
      ++iter;
    }
    return size;
  }

  int
  CpltSetVarImp::unknownMin(void) const {
    if (manager.ctrue(domain)) { return initialLubMin(); }
    BddIterator iter(domain);
    while (iter()) {
      NodeStatus status = iter.status();
      if (status == FIX_UNKNOWN || status == UNDET) {
        int idx = iter.label() - offset();
        return initialLubMin() + idx;
      }
      ++iter;
    }
    return MIN_OF_EMPTY;
  }

  int
  CpltSetVarImp::unknownMax(void) const {
    if (manager.ctrue(domain))
      return initialLubMax();

    BddIterator iter(domain);
    int lastunknown = -1;
    while (iter()) {
      NodeStatus status = iter.status();
      if (status == FIX_UNKNOWN || status == UNDET) {
        int idx = iter.label() - offset();
        lastunknown = initialLubMin() + idx;
      }
      ++iter;
    }
    return (lastunknown == -1) ?  MAX_OF_EMPTY : lastunknown;
  }

  unsigned int
  CpltSetVarImp::unknownSize(void) const {
    int size = tableWidth();
    if (manager.ctrue(domain))
      return size;

    BddIterator iter(domain);
    while (iter()) {
      NodeStatus status = iter.status();
      if (status == FIX_GLB || status == FIX_NOT_LUB) { size--; }
      ++iter;
    }
    return size;
  }

  int
  CpltSetVarImp::lubMin(void) const {
    if (manager.ctrue(domain)) { return initialLubMin(); }
    Gecode::Set::LubRanges<CpltSetVarImp*> lub(this);
    return !lub() ? MIN_OF_EMPTY : lub.min();
  }

  int
  CpltSetVarImp::lubMax(void) const {
    if (manager.ctrue(domain)) { return initialLubMax(); }
    Gecode::Set::LubRanges<CpltSetVarImp*> lub(this);
    if (!lub()) { return MAX_OF_EMPTY; }
    int maxlub = initialLubMax();
    while (lub()) {
      maxlub = lub.max();
      ++lub;
    }
    return maxlub;
  }

  int
  CpltSetVarImp::lubMinN(int n) const {
    if (manager.ctrue(domain))
      return initialLubMin() + n;

    Gecode::Set::LubRanges<CpltSetVarImp*> lub(this);

    if (!lub())
      return MIN_OF_EMPTY;

    while (lub()) {
      if (n < (int) lub.width()) {
        return lub.min() + n;
      } else {
        n -= lub.width();
      }
      ++lub;
    }
    // what to return if n is greater than the number of possible values ?
    // we should throw an exception here
    return MIN_OF_EMPTY;
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

      assert(!manager.leaf(c));
      // insert bdd root into dqueue
      nodes[l] = c;
      manager.mark(nodes[l]);
      markref++;
      l++;
    }
    if (operator ()()) {
      operator ++();
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
  BddIterator::operator ++(void) {
    if (empty()) { // no more nodes on the stack to be iterated
      singleton = false;
      cache_unmark();
      assert(markref == 0);
      return;
    }

    // a true path ending on a previous level was detected
    // hence the current level cannot be fixed
    // description of onlyleaves and bypassed in var.hpp
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
        nodes[norm] = bdd();
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
        nodes[shift] = bdd();
        l--;
      }

      l--;
      manager.unmark(nodes[l]);
      markref--;
      cur = nodes[l];
      assert(!manager.marked(cur));
      nodes[l] = bdd();

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
          nodes[norm] = bdd();
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
        nodes[shift] = bdd();
        r++;
      }
      // check whether right-hand side nodes has fixed vars
      r++;
      manager.unmark(nodes[r]);
      markref--;
      cur = nodes[r];
      assert(!manager.marked(cur));

      nodes[r] = bdd();
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

  // Create disposer
  GECODE_CPLTSET_EXPORT VarDisposer<CpltSetVarImp> vtd;

}}

// STATISTICS: cpltset-var
