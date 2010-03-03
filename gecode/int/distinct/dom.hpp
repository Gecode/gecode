/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include <climits>

namespace Gecode { namespace Int { namespace Distinct {

  /**
   * \brief Class for combining two pointers with a flag
   *
   * When one pointer is given, this other can be retrieved.
   *
   */

  template<class T>
  class CombPtrFlag {
  private:
    /// Store pointer and flag
    ptrdiff_t cpf;
  public:
    /// Initialize with pointer \a p1 and \a p2
    CombPtrFlag(T* p1, T* p2);
    /// Initialize with pointer \a p1 and \a p2
    void init(T* p1, T* p2);
    /// Return the other pointer when \a p is given
    T* ptr(T* p) const;
    /// Check whether flag is set
    int is_set(void) const;
    /// Set flag
    void set(void);
    /// Clear flag
    void unset(void);
  };

  /**
   * \brief Bidirectional links for both edges and anchors in nodes of view-value graph
   *
   */
  class BiLink {
  private:
    BiLink* _prev; BiLink* _next;
  public:
    BiLink(void);

    BiLink* prev(void) const; void prev(BiLink*);
    BiLink* next(void) const; void next(BiLink*);

    void add(BiLink*);
    void unlink(void);

    void mark(void);
    bool marked(void) const;

    bool empty(void) const;
  };

  template<class View> class Edge;

  /**
   * \brief Base-class for nodes (both view and value nodes)
   *
   * Note: the obvious ill-design to have also nodes and edges
   * parametric wrt View is because the right design (having template
   * function members) gets miscompiled (and actually not even compiled
   * with some C++ compilers). Duh!
   *
   */
  template<class View>
  class Node : public BiLink {
  public:
    unsigned int low, min, comp;
    Edge<View>* iter;

    Node(void);

    Edge<View>* edge_fst(void) const;
    Edge<View>* edge_lst(void) const;

    static void  operator delete(void*, size_t);
    static void  operator delete(void*,Space&);
    static void* operator new(size_t, Space&);
  };

  /**
   * \brief Value nodes in view-value graph
   *
   */
  template<class View>
  class ValNode : public Node<View> {
  protected:
    /// The value of the node
    const int      _val;
    /// The matching edge
    Edge<View>*    _matching;
    /// The next value node
    ValNode<View>* _next_val;
  public:
    /// Initialize with value \a v
    ValNode(int v);
    /// Initialize with value \a v and successor \a n
    ValNode(int v, ValNode<View>* n);
    /// Return value of node
    int val(void) const;
    /// Set matching edge to \a m
    void matching(Edge<View>* m);
    /// Return matching edge (NULL if unmatched)
    Edge<View>* matching(void) const;
    /// Return pointer to next value node fields
    ValNode<View>** next_val_ref(void);
    /// Return next value node
    ValNode<View>* next_val(void) const;
    /// Set next value node to \a v
    void next_val(ValNode<View>* v);
  };

  /**
   * \brief View nodes in view-value graph
   *
   */
  template<class View>
  class ViewNode : public Node<View> {
  protected:
    /// The node's view
    View        _view;
    /// The first value edge
    Edge<View>* _val_edges;
  public:
    /// Initialize new node for view \a x
    ViewNode(View x);
    /// Return first edge of all value edges
    Edge<View>*  val_edges(void) const;
    /// Return pointer to first edge fields of all value edges
    Edge<View>** val_edges_ref(void);
    /// Return view
    View view(void) const;
  };

  /**
   * \brief Edges in view-value graph
   *
   */
  template<class View>
  class Edge : public BiLink {
  protected:
    /// Next edge in chain of value edges
    Edge<View>*              _next_edge;
    /// Combine source and destination node anf flag
    CombPtrFlag<Node<View> > sd;
  public:
    /// Construct new edge between \a x and \a v
    Edge(ValNode<View>* v, ViewNode<View>* x);
    /// Return destination of edge when source \a s is given
    Node<View>* dst(Node<View>* s) const;

    /// Return view node when value node \a v is given
    ViewNode<View>* view(ValNode<View>* v) const;
    /// Return value node when view node \a x is given
    ValNode<View>* val(ViewNode<View>* x) const;

    bool used(Node<View>*) const;
    void use(void);
    void free(void);

    void revert(Node<View>*);

    Edge<View>*  next_edge(void) const;
    Edge<View>** next_edge_ref(void);

    Edge<View>* next(void) const;

    static void  operator delete(void*, size_t);
    static void  operator delete(void*,Space&);
    static void* operator new(size_t, Space&);
  };

}}}

#include <gecode/int/distinct/combptr.hpp>
#include <gecode/int/distinct/bilink.hpp>
#include <gecode/int/distinct/edge.hpp>
#include <gecode/int/distinct/node.hpp>

namespace Gecode { namespace Int { namespace Distinct {


  template<class View>
  forceinline
  DomCtrl<View>::ViewValGraph::ViewValGraph(void)
    : view(NULL), val(NULL), count(1) {}

  template<class View>
  forceinline bool
  DomCtrl<View>::ViewValGraph::initialized(void) const {
    return view != NULL;
  }

  template<class View>
  forceinline bool
  DomCtrl<View>::ViewValGraph::match(ViewNodeStack& m, ViewNode<View>* x) {
    count++;
  start:
    // Try to find matching edge cheaply: is there a free edge around?
    {
      Edge<View>* e = x->val_edges();
      // This holds true as domains are never empty
      assert(e != NULL);
      do {
        if (!e->val(x)->matching()) {
          e->revert(x); e->val(x)->matching(e);
          // Found a matching, revert all edges on stack
          while (!m.empty()) {
            x = m.pop(); e = x->iter;
            e->val(x)->matching()->revert(e->val(x));
            e->revert(x); e->val(x)->matching(e);
          }
          return true;
        }
        e = e->next_edge();
      } while (e != NULL);
    }
    // No, find matching edge by augmenting path method
    Edge<View>* e = x->val_edges();
    do {
      if (e->val(x)->matching()->view(e->val(x))->min < count) {
        e->val(x)->matching()->view(e->val(x))->min = count;
        m.push(x); x->iter = e;
        x = e->val(x)->matching()->view(e->val(x));
        goto start;
      }
    next:
      e = e->next_edge();
    } while (e != NULL);
    if (!m.empty()) {
      x = m.pop(); e = x->iter; goto next;
    }
    // All nodes and edges unsuccessfully tried
    return false;
  }

  template<class View>
  forceinline void
  DomCtrl<View>::ViewValGraph::init(Space& home, ViewNode<View>* x) {
    Edge<View>** edge_p = x->val_edges_ref();
    ViewValues<View> xi(x->view());
    ValNode<View>** v = &val;
    while (xi() && (*v != NULL)) {
      if ((*v)->val() == xi.val()) {
        // Value node does already exist, create new edge
        *edge_p = new (home) Edge<View>(*v,x);
        edge_p = (*edge_p)->next_edge_ref();
        v = (*v)->next_val_ref();
        ++xi;
      } else if ((*v)->val() < xi.val()) {
        // Skip to next value node
        v = (*v)->next_val_ref();
      } else {
        // Value node does not yet exist, create and link it
        ValNode<View>* nv = new (home) ValNode<View>(xi.val(),*v);
        *v = nv; v = nv->next_val_ref();
        *edge_p = new (home) Edge<View>(nv,x);
        edge_p = (*edge_p)->next_edge_ref();
        ++xi; n_val++;
      }
    }
    // Create missing value nodes
    while (xi()) {
      ValNode<View>* nv = new (home) ValNode<View>(xi.val(),*v);
      *v = nv; v = nv->next_val_ref();
      *edge_p = new (home) Edge<View>(nv,x);
      edge_p = (*edge_p)->next_edge_ref();
      ++xi; n_val++;
    }
    *edge_p = NULL;
  }

  template<class View>
  ExecStatus
  DomCtrl<View>::ViewValGraph::init(Space& home, int n, View* x) {
    n_view = n;
    view = home.alloc<ViewNode<View>*>(n_view);

    // Find value information for construction of view value graph
    int min = x[n_view-1].min();
    int max = x[n_view-1].max();
    for (int i=n_view-1; i--; ) {
      min = std::min(min,x[i].min());
      max = std::max(max,x[i].max());
    }

    unsigned int width = static_cast<unsigned int>(max-min+1);

    // Definitly not enough values
    if (width < static_cast<unsigned int>(n_view))
      return ES_FAILED;

    // Initialize view nodes
    for (int i=n; i--; )
      view[i] = new (home) ViewNode<View>(x[i]);

    n_val = 0;
    val = NULL;

    Region r(home);

    if (static_cast<unsigned int>(n_view)*4 >= width) {
      // Values are dense: use a mapping
      ValNode<View>** val2node = r.alloc<ValNode<View>* >(width);

      for (unsigned int i=width; i--; )
        val2node[i]=NULL;

      for (int i=n; i--; ) {
        Edge<View>** edge_p = view[i]->val_edges_ref();
        for (ViewValues<View> xi(x[i]); xi(); ++xi) {
          if (val2node[xi.val()-min] == NULL)
            val2node[xi.val()-min] = new (home) ValNode<View>(xi.val());
          *edge_p = new (home) Edge<View>(val2node[xi.val()-min],view[i]);
          edge_p = (*edge_p)->next_edge_ref();
        }
        *edge_p = NULL;
      }

      for (unsigned int i=width; i--; )
        if (val2node[i] != NULL) {
          val2node[i]->next_val(val);
          val = val2node[i];
          n_val++;
        }

    } else {
      // Values are sparse
      for (int i=n; i--; )
        init(home,view[i]);
    }

    ViewNodeStack m(r,n_view);
    for (int i = n_view; i--; )
      if (!match(m,view[i]))
        return ES_FAILED;
    return ES_OK;
  }

  template<class View>
  forceinline void
  DomCtrl<View>::ViewValGraph::mark(Space& home) {
    Region r(home);
    {
      // Marks all edges as used that are on simple paths in the graph
      // that start from a free (unmatched node) by depth-first-search
      Support::StaticStack<ValNode<View>*,Region> visit(r,n_val);

      // Insert all free nodes: they can be only value nodes as we
      // have a maximum matching covering all view nodes
      count++;
      {
        ValNode<View>** v = &val;
        while (*v != NULL)
          if (!(*v)->matching()) {
            if ((*v)->empty()) {
              *v = (*v)->next_val();
              n_val--;
            } else {
              (*v)->min = count;
              visit.push(*v);
              v = (*v)->next_val_ref();
            }
          } else {
            v = (*v)->next_val_ref();
          }
      }

      // Invariant: only value nodes are on the stack!
      while (!visit.empty()) {
        ValNode<View>* n = visit.pop();
        for (Edge<View>* e = n->edge_fst(); e != n->edge_lst(); e=e->next()) {
          // Get the value node
          e->use();
          ViewNode<View>* x = e->view(n);
          if (x->min < count) {
            x->min = count;
            assert(x->edge_fst()->next() == x->edge_lst());
            ValNode<View>* m = x->edge_fst()->val(x);
            x->edge_fst()->use();
            if (m->min < count) {
              m->min = count;
              visit.push(m);
            }
          }
        }
      }
    }

    {
      Support::StaticStack<Node<View>*,Region> scc(r,n_val+n_view);
      Support::StaticStack<Node<View>*,Region> visit(r,n_val+n_view);

      count++;
      unsigned int cnt0 = count;
      unsigned int cnt1 = count;

      for (int i = n_view; i--; )
        if (view[i]->min < count) {
          Node<View>* w = view[i];
        start:
          w->low = w->min = cnt0++;
          scc.push(w);
          Edge<View>* e = w->edge_fst();
          while (e != w->edge_lst()) {
            if (e->dst(w)->min < count) {
              visit.push(w); w->iter = e;
              w=e->dst(w);
              goto start;
            }
          next:
            if (e->dst(w)->low < w->min)
              w->min = e->dst(w)->low;
            e = e->next();
          }
          if (w->min < w->low) {
            w->low = w->min;
          } else {
            Node<View>* v;
            do {
              v = scc.pop();
              v->comp = cnt1;
              v->low  = UINT_MAX;
            } while (v != w);
            cnt1++;
          }
          if (!visit.empty()) {
            w=visit.pop(); e=w->iter; goto next;
          }
        }
      count = cnt0+1;
    }
  }

  /// Prunes the values read from a view node
  template<class View>
  class PruneVal {
  protected:
    /// View node
    ViewNode<View>* x;
    /// Current value edge
    Edge<View>* e;
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize with edges for view node \a y
    PruneVal(ViewNode<View>* y);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a value or done
    bool operator ()(void) const;
    /// Move iterator to next value (if possible)
    void operator ++(void);
    //@}

    /// \name Value access
    //@{
    /// Return current value
    int val(void) const;
    //@}
  };

  template<class View>
  forceinline
  PruneVal<View>::PruneVal(ViewNode<View>* y)
    : x(y), e(y->val_edges()) {
    while ((e != NULL) && e->used(x))
      e = e->next_edge();
  }
  template<class View>
  forceinline bool
  PruneVal<View>::operator ()(void) const {
    return e != NULL;
  }
  template<class View>
  forceinline void
  PruneVal<View>::operator ++(void) {
    do {
      e = e->next_edge();
    } while ((e != NULL) && e->used(x));
  }
  template<class View>
  forceinline int
  PruneVal<View>::val(void) const {
    assert(!e->used(x));
    return e->val(x)->val();
  }

  template<class View>
  forceinline ExecStatus
  DomCtrl<View>::ViewValGraph::tell(Space& home, bool& assigned) {
    assigned = false;
    // Tell constraints and also eliminate nodes and edges
    for (int i = n_view; i--; ) {
      ViewNode<View>* x = view[i];
      if (!x->edge_fst()->used(x)) {
        GECODE_ME_CHECK(x->view().eq(home,x->edge_fst()->val(x)->val()));
        x->edge_fst()->val(x)->matching(NULL);
        for (Edge<View>* e = x->val_edges(); e != NULL; e = e->next_edge())
          e->unlink();
        view[i] = view[--n_view];
        assigned = true;
      } else {
        PruneVal<View> pv(view[i]);
        GECODE_ME_CHECK(view[i]->view().minus_v(home,pv,false));
      }
    }
    return ES_OK;
  }

  template<class View>
  forceinline void
  DomCtrl<View>::ViewValGraph::purge(void) {
    if (count > (UINT_MAX >> 1)) {
      count = 1;
      for (int i=n_view; i--; )
        view[i]->min = 0;
      for (ValNode<View>* v = val; v != NULL; v = v->next_val())
        v->min = 0;
    }
  }

  template<class View>
  bool
  DomCtrl<View>::ViewValGraph::sync(Space& home) {
    Region r(home);
    // Stack for view nodes to be rematched
    ViewNodeStack re(r,n_view);
    // Synchronize nodes
    for (int i = n_view; i--; ) {
      ViewNode<View>* x = view[i];
      if (x->view().assigned()) {
        x->edge_fst()->val(x)->matching(NULL);
        for (Edge<View>* e = x->val_edges(); e != NULL; e = e->next_edge())
          e->unlink();
        view[i] = view[--n_view];
      } else {
        ViewRanges<View> r(x->view());
        Edge<View>*  m = x->edge_fst();      // Matching edge
        Edge<View>** p = x->val_edges_ref();
        Edge<View>*  e = *p;
        do {
          while (e->val(x)->val() < r.min()) {
            // Skip edge
            e->unlink(); e->mark();
            e = e->next_edge();
          }
          *p = e;
          assert(r.min() == e->val(x)->val());
          // This edges must be kept
          for (unsigned int j=r.width(); j--; ) {
            e->free();
            p = e->next_edge_ref();
            e = e->next_edge();
          }
          ++r;
        } while (r());
        *p = NULL;
        while (e != NULL) {
          e->unlink(); e->mark();
          e = e->next_edge();
        }
        if (m->marked()) {
          // Matching has been deleted!
          m->val(x)->matching(NULL);
          re.push(x);
        }
      }
    }
    ViewNodeStack m(r,n_view);
    while (!re.empty())
      if (!match(m,re.pop()))
        return false;
    return true;
  }



  /*
   * The propagation controller
   *
   */

  template<class View>
  forceinline
  DomCtrl<View>::DomCtrl(void) {}

  template<class View>
  forceinline bool
  DomCtrl<View>::available(void) {
    return vvg.initialized();
  }

  template<class View>
  forceinline ExecStatus
  DomCtrl<View>::init(Space& home, int n, View* x) {
    return vvg.init(home,n,x);
  }

  template<class View>
  forceinline ExecStatus
  DomCtrl<View>::sync(Space& home) {
    vvg.purge();
    return vvg.sync(home) ? ES_OK : ES_FAILED;
  }

  template<class View>
  forceinline ExecStatus
  DomCtrl<View>::propagate(Space& home, bool& assigned) {
    vvg.mark(home);
    return vvg.tell(home,assigned);
  }


  /*
   * The propagator proper
   *
   */

  template<class View>
  forceinline
  Dom<View>::Dom(Home home, ViewArray<View>& x)
    : NaryPropagator<View,PC_INT_DOM>(home,x) {}

  template<class View>
  ExecStatus
  Dom<View>::post(Home home, ViewArray<View>& x) {
    if (x.size() == 2)
      return Rel::Nq<View>::post(home,x[0],x[1]);
    if (x.size() == 3)
      return TerDom<View>::post(home,x[0],x[1],x[2]);
    if (x.size() > 3) {
      // Do bounds propagation to make view-value graph smaller
      GECODE_ES_CHECK(prop_bnd<View>(home,x));
      (void) new (home) Dom<View>(home,x);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  Dom<View>::Dom(Space& home, bool share, Dom<View>& p)
    : NaryPropagator<View,PC_INT_DOM>(home,share,p) {}

  template<class View>
  PropCost
  Dom<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_INT_VAL)
      return PropCost::linear(PropCost::LO, x.size());
    else
      return PropCost::quadratic(PropCost::HI, x.size());
  }

  template<class View>
  Actor*
  Dom<View>::copy(Space& home, bool share) {
    return new (home) Dom<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  Dom<View>::propagate(Space& home, const ModEventDelta& med) {
    if (View::me(med) == ME_INT_VAL) {
      ExecStatus es = prop_val<View,false>(home,x);
      GECODE_ES_CHECK(es);
      if (x.size() < 2)
        return ES_SUBSUMED(home,*this);
      if (es == ES_FIX)
        return ES_FIX_PARTIAL(*this,View::med(ME_INT_DOM));
      es = prop_bnd<View>(home,x);
      GECODE_ES_CHECK(es);
      if (x.size() < 2)
        return ES_SUBSUMED(home,*this);
      es = prop_val<View,true>(home,x);
      GECODE_ES_CHECK(es);
      if (x.size() < 2)
        return ES_SUBSUMED(home,*this);
      return ES_FIX_PARTIAL(*this,View::med(ME_INT_DOM));
    }

    if (x.size() == 2)
      GECODE_REWRITE(*this,Rel::Nq<View>::post(home(*this),x[0],x[1]));
    if (x.size() == 3)
      GECODE_REWRITE(*this,TerDom<View>::post(home(*this),x[0],x[1],x[2]));

    if (dc.available()) {
      GECODE_ES_CHECK(dc.sync(home));
    } else {
      GECODE_ES_CHECK(dc.init(home,x.size(),&x[0]));
    }

    bool assigned;
    GECODE_ES_CHECK(dc.propagate(home,assigned));

    return ES_FIX;
  }

}}}

// STATISTICS: int-prop

