/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
 *     Christian Schulte, 2009
 *     Guido Tack, 2009
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

namespace Gecode { namespace Int { namespace GCC {

  /**
   * \brief Bounds constraint (BC) type
   *
   * If BC = UBC, then we argue about the Upper Bounds Constraint
   * else we use the functions for the Lower Bounds Constraint
   */
  enum BC {UBC = 1, LBC = 0};

  class Edge;
  /// Base class for nodes in the variable-value-graph
  class Node {
  private:
    /// stores all incident edges on the node
    Edge* e;
    /// returns the first edge
    Edge* fst;
    /// returns the last edge
    Edge* lst;
    /// single incoming edge used for storing a path in the algorithms
    Edge* ie;
    /// Mark as matching edge in LBC
    bool  lm;
    /// Mark as matching edge in UBC
    bool  um;
    /// Explicit node type
    bool  type;
  public:
    /// stores the number of incident edges on the node
    int noe;

    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Node(void);
    /// Constructor that set type to \a t
    Node(bool t);
    //@}
    /// Destructor
    virtual ~Node(void) {};

    /// \name Access
    //@{
    /// return reference to the incident edges
    Edge** adj(void);
    /// return pointer to the first incident edge
    Edge* first(void) const;
    /// return pointer to the last incident edge
    Edge* last(void) const;
    /// return pointer to the node's inedge
    Edge* inedge(void) const;
    /// return the type of the node
    bool get_type(void) const;
    /// access the matching flag of the node
    template<BC>
    bool get_match_flag(void) const;
    /// get the information on the node (either var-index or value)
    virtual int get_info(void) const = 0;
    /// test whether the node is matched
    virtual bool is_matched(BC) const = 0;
    /// check whether a node has been removed from the graph
    bool removed(void) const;
    //@}

    /// \name Update
    //@{
    /// set the first edge pointer to \a p
    void first(Edge* p);
    /// set the last edge pointer to \a p
    void last(Edge* p);
    /// set the inedge pointer to \a p
    void inedge(Edge* p);
    /// set the matching flag to \a f
    template<BC>
    void set_match_flag(bool f);
    /// set the node information to \a i
    virtual void set_info(int i) = 0;
    //@}
  };

  /// %Variable Node
  class VarNode : public Node {
  private:
    /// stores the matching edge on this node in the UBC
    Edge* ubm;
    /// stores the matching edge on this node in the LBC
    Edge* lbm;
  public:
    /// stores the variable index of the node
    int var;

    /// stores the variable index of the node
    int xindex;

    /// \name Constructors and initialization
    //@{
    /// Default constructor
    VarNode(void);
    /// Creates a variable node with index i
    VarNode(int i, int oidx);
    //@}

    /// \name Access
    //@{
    /// return the matching edge on the node
    template<BC>
    Edge* get_match(void) const;
    /// return the variable index of the node
    int get_info(void) const;
    /// returns whether the node is still matchable
    bool is_matched(BC) const;
    /// tests whether the node is matched or not
    template<BC>
    bool matched(void) const;
    //@}

    /// \name Update
    //@{
    /// set the node info to \a i
    void set_info(int i);
    /// set the pointer of the matching edge to m
    template<BC>
    void set_match(Edge* m);
    /// match the node
    template<BC>
    void match(void);
    /// unmatch the node
    template<BC>
    void unmatch(void);
    //@}
  };

  /// Value node
  class ValNode : public Node {
  private:
    /// minimal required occurence of the value as stored in k
    int _klb;
    /// maximal required occurence of the value as stored in k
    int _kub;
    /// index to acces the value via cardinality array k
    int _kidx;
    /// stores the current number of occurences of the value
    int _kcount;
    /// store numbre of conflicting matching edges
    int noc;

    /**
     * \brief Minimal capacity of the value node
     *
     * Stores who often the value node can be matched in a LBC-matching
     */
    int lb;
    int ublow;
    /**
     * \brief Maximal capacity of the value node
     *
     * Stores who often the value node can be matched in a UBC-matching
     */
    int ub;

  public:
    /// stores the value of the node
    int val;
    /// stores the index of the node
    int idx;

    /// \name Constructors and destructors
    //@{
    /// Default constructor
    ValNode(void);
    /**
     * \brief Constructor for value node
     *
     * with minimal capacity \a min,
     * maximal capacity \a max,
     * the value \a value and the index \a k_access in \a k
     */
    ValNode(int min, int max, int value, int kidx, int kshift, int count);
    //@}

    /// \name Access
    //@{
    /// set the max cap for LBC
    void set_maxlow(int i);
    /// get max cap for LBC
    int get_maxlow(void) const;
    /// Mark the value node as conflicting in case of variable cardinalities
    void card_conflict(int c);
    /// Check whether the value node is conflicting
    int card_conflict(void) const;
    /// Reduce the conflict counter
    void red_conflict(void);
    /// increases the value counter
    void inc(void);
    /// returns the current number of occurences of the value
    int kcount(void) const;
    /// returns the number of incident matching edges on a value node
    template<BC>
    int incid_match(void) const;
    /// returns the index in cardinality array k
    int kindex(void) const;
    /// return the node information
    int get_info(void) const;
    /// returns \a true if the node is matched in BC, \a false otherwise
    template<BC>
    bool matched(void) const;
    /// tests whether the node is a sink
    bool sink(void) const;
    /// tests whether the node is a source
    bool source(void) const;
    /// return the minimal node capacity as stored in \a k
    int kmin(void) const;
    /// return the maximal node capacity as stored in \a k
    int kmax(void) const;
    /// return minimal or maximal capacity
    template<BC>
    int kbound(void) const;
    /// returns whether the node is still matchable
    bool is_matched(BC) const;
    //@}

    /// \name Update
    //@{
    void kcount(int);
    /// changes the index in the cardinality array k
    void kindex(int);
    /// decrease the node-capacity
    template<BC>
    void dec(void);
    /// increase the node-capacity
    template<BC>
    void inc(void);
    /// return the the node-capacity
    template<BC>
    int cap(void) const;
    /// set the node-capacity to \a c
    template<BC>
    void set_cap(int c);
    /// match the node
    template<BC>
    void match(void);
    /// unmatch the node
    template<BC>
    void unmatch(void);
    /// node reset to original capacity values
    void reset(void);
    /// set the node infomration to \a i
    void set_info(int i);
    /// set the minimal k-capacity to min
    void set_kmin(int min);
    /// set the maximal k-capacity to max
    void set_kmax(int max);
    //@}
  };

  /// Class for edges \f$ e(x,v) \f$ in the variable-value-graph
  class Edge {
  private:
    /// pointer to the variable node
    VarNode* x;
    /// pointer to the value node
    ValNode* v;
    /// pointer to the next edge incident on the same variable node
    Edge* next_edge;
    /// pointer to the previous edge incident on the same variable node
    Edge* prev_edge;
    /// pointer to the next edge on the same value node
    Edge* next_vedge;
    /// pointer to the previous edge on the same value node
    Edge* prev_vedge;
    /// Flags for edges
    enum EdgeFlag {
      /// No flags set
      EF_NONE  = 0,
      /// Whether edge is used in LBC
      EF_MRKLB = 1 << 0,
      /// Whether edge is used in UBC
      EF_MRKUB = 1 << 1,
      /// Whether edge is matched in LBC
      EF_LM    = 1 << 2,
      /// Whether edge is matched in UBC
      EF_UM    = 1 << 3,
      /// Whether edge has been deleted
      EF_DEL   = 1 << 4
    };
    /// Flags for edges
    unsigned char ef;
  public:
    /// \name Constructors
    //@{
    /// Default constructor
    Edge(void) {}
    /**
     * \brief Construct edge \f$e(x,v)\f$ from variable node \a x
     *  and value node \a y
     */
    Edge(VarNode* x, ValNode* v);
    //@}

    /// \name Access
    //@{
    /**
     * \brief return whether the edge is used
     *
     * An edge can be used in a matching on the graph,
     * a path on the graph or a cycle in the graph.
     *
     */
    template<BC>
    bool used(void) const;
    /// return whether the edge is matched
    template<BC>
    bool matched(void) const;
    /// return whether the edge has been deleted from the graph
    bool deleted(void) const;
    /**
     * \brief return a pointer to the next edge
     *  If \a t is false the function returns the next edge incident on \a x
     *  otherwise it returns the next edge incident on \a v
     */
    Edge* next(bool t) const;
    /// return the pointer to the next edge incident on \a x
    Edge* next(void) const;
    /// return the pointer to the previous edge incident on \a x
    Edge* prev(void) const;
    /// return the pointer to the next edge incident on \a v
    Edge* vnext(void) const;
    /// return the pointer to the previous edge incident on \a v
    Edge* vprev(void) const;
    /// return the pointer to the variable node \a x of this edge
    VarNode* getVar(void) const;
    /// return the pointer to the value node \a v of this edge
    ValNode* getVal(void) const;
    /**
     * \brief return pointer to \a x if \a t = true otherwise return \a v
     *
     */
    Node* getMate(bool t) const;
    //@}

    /// Update
    //@{
    /// Mark the edge as used
    template<BC>
    void use(void);
    /// Mark the edge as unused
    template<BC>
    void free(void);
    /**
     * \brief Reset the edge ( free the edge, and unmatch
     * the edge including its end-nodes
     *
     */
    template<BC>
    void reset(void);
    /// Match the edge
    template<BC>
    void match(void);
    /// Unmatch the edge and the incident nodes
    template<BC>
    void unmatch(void);
    /// Unmatch the edge and  ( \a x if t=false,  \a v otherwise )
    template<BC>
    void unmatch(bool t);
    /// Unlink the edge from the linked list of edges
    void unlink(void);
    /// Mark the edge as deleted during synchronization
    void del_edge(void);
    /// Insert the edge again
    void insert_edge(void);
    /// return the reference to the next edge incident on \a x
    Edge** next_ref(void);
    /// return the reference to the previous edge incident on \a x
    Edge** prev_ref(void);
    /// return the reference to the next edge incident on \a v
    Edge** vnext_ref(void);
    /// return the reference to the previous edge incident on \a v
    Edge** vprev_ref(void);
    //@}
  };


  /**
   * \brief Variable-value-graph used during propagation
   *
   */
  template<class Card>
  class VarValGraph {
  private:
    /// Temporary stack for nodes
    typedef Support::StaticStack<Node*,Region> NodeStack;
    /// Bitset
    typedef Support::BitSet<Region> BitSet;
    /// Problem variables
    ViewArray<IntView>& x;
    /// Copy keeping track of removed variables
    ViewArray<IntView>& y;
    /// Occurences
    ViewArray<Card>& k;
    /// Variable partition representing the problem variables
    VarNode** vars;
    /**
     * \brief Value partition
     *  For each value
     *  \f$ v_i\in V=\left(\bigcup_\{0, \dots, |x|-1\}\right) D_i \f$
     *  in the domains of the
     *  problem variables there is a node in the graph.
     */
    ValNode** vals;
    /// Edges connecting the variable and the value partition
    Edge* edges;
    /// cardinality of the variable partition
    int n_var;
    /**
     * \brief  cardinality of the value partition
     *
     * Computed as \f$ |V| = \left(\bigcup_\{0, \dots, |x|-1\}\right) D_i \f$
     */
    int n_val;
    /// the number of edges in the graph
    int n_edge;
    /// total number of nodes in the graph
    int node_size;
    /**
     * \brief The sum over the minimal capacities of all value nodes
     *
     *  \f$sum_min = \sum_{v_i \in V} l_i= k[i].min() \f$
     */
    int sum_min;
    /**
     * \brief The sum over the maximal capacities of all value nodes
     *
     * \f$sum_max = \sum_{v_i \in V} l_i= k[i].max() \f$
     */
    int sum_max;
  public:
    /// \name Constructors and Destructors
    //@{
    VarValGraph(Space& home,
                ViewArray<IntView>&, ViewArray<IntView>&, ViewArray<Card>&,
                int , int , int );
    //@}
    /// \name Graph-interface
    //@{
    /// Check whether minimum requirements shrink variable domains
    ExecStatus min_require(Space& home);

    /**
     * \brief Synchronization of the graph
     *
     * If the graph has already been constructed and some edges have
     * been removed during propagation \fn sync removes those edges
     * that do not longer belong to the graph associated with the current
     * variable domains.
     */
    ExecStatus sync(Space& home);

    /// Remove edges that do not belong to any maximal matching
    template<BC>
    ExecStatus narrow(Space& home);

    /** \brief Compute a maximum matching M on the graph
     *
     *  - If BC=UBC then \f$|M|= |X|\f$
     *  - If BC=LBC then \f$|M|= \sum_{i\in \{ 0, \dots, |X|-1\}}
     *    k[i].min()\f$
     */
    template<BC>
    ExecStatus maximum_matching(Space& home);

    /// Compute possible free alternating paths in the graph
    template<BC>
    void free_alternating_paths(Space& home);
    /// Compute possible strongly connected components of the graph
    template<BC>
    void strongly_connected_components(Space& home);
    /**
     * \brief Test whether the current maximal matching on the graph
     * can be augmented by an alternating path starting and ending with
     * a free node.
     */
    template<BC>
    bool augmenting_path(Space& home, Node*);

  protected:
    /**
     * \brief Perform depth-first search on the graph
     *
     * Depth first search used to compute the
     * strongly connected components of the graph.
     */
    template<BC>
    void dfs(Node*, BitSet&, BitSet&, int[],
             NodeStack&, NodeStack&, int&);

    //@}
  public:
    /// Allocate memory for the graph
    void* operator new(size_t t, Space& home);
    /// Deallocation (void)
    void operator delete(void*, Space&) {}
  };



  /*
   * Nodes
   *
   */
  forceinline
  Node::Node(void) {}

  forceinline
  Node::Node(bool t) 
    : e(NULL), fst(NULL), lst(NULL), ie(NULL), 
      lm(false), um(false), type(t), noe(0) {}

  forceinline Edge**
  Node::adj(void) {
    return &e;
  }
  forceinline  Edge*
  Node::first(void) const {
    return fst;
  }
  forceinline Edge*
  Node::last(void) const {
    return lst;
  }
  forceinline void
  Node::first(Edge* p) {
    fst = p;
  }
  forceinline void
  Node::last(Edge* p) {
    lst = p;
  }

  forceinline bool
  Node::get_type(void) const {
    return type;
  }
  forceinline Edge*
  Node::inedge(void) const {
    return ie;
  }
  forceinline void
  Node::inedge(Edge* p) {
    ie = p;
  }

  template<BC direction>
  forceinline void
  Node::set_match_flag(bool b) {
    if (direction == UBC) {
      um = b;
    } else {
      lm = b;
    }
  }
  template<BC direction>
  forceinline bool
  Node::get_match_flag(void) const {
    if (direction == UBC) {
      return um;
    } else {
      return lm;
    }
  }
  forceinline bool
  Node::removed(void) const {
    return noe == 0;
  }



  /*
   * Variable nodes
   *
   */
  forceinline
  VarNode::VarNode(void) {}

  forceinline
  VarNode::VarNode(int x, int orig_idx) :
    Node(false), ubm(NULL), lbm(NULL), var(x), xindex(orig_idx) {}

  template<BC direction>
  forceinline bool
  VarNode::matched(void) const {
    return get_match_flag<direction>();
  }

  forceinline bool
  VarNode::is_matched(BC d) const {
    if (d == UBC) {
      return matched<UBC>();
    } else {
      return matched<LBC>();
    }
  }

  template<BC direction>
  forceinline void
  VarNode::match(void) {
    set_match_flag<direction>(true);
  }

  template<BC direction>
  forceinline void
  VarNode::set_match(Edge* p) {
    if (direction == UBC) {
      ubm = p;
    } else {
      lbm = p;
    }
  }

  template<BC direction>
  forceinline void
  VarNode::unmatch(void) {
    set_match_flag<direction>(false);
    set_match<direction>(NULL);
  }

  template<BC direction>
  forceinline Edge*
  VarNode::get_match(void) const {
    if (direction == UBC) {
      return ubm;
    } else {
      return lbm;
    }
  }

  forceinline void
  VarNode::set_info(int i) {
    var = i;
  }

  forceinline int
  VarNode::get_info(void) const {
    return var;
  }



  /*
   * Value nodes
   *
   */
  forceinline
  ValNode::ValNode(void) {}

  forceinline
  ValNode::ValNode(int min, int max, int value,
                   int kidx, int kshift, int count) :
    Node(true), _klb(min), _kub(max), _kidx(kidx), _kcount(count),
    noc(0),
    lb(min), ublow(max), ub(max),
    val(value), idx(kshift) {}

  forceinline void
  ValNode::set_maxlow(int i) {
    assert(i >= lb);
    ublow = i;
  }

  forceinline int
  ValNode::get_maxlow(void) const {
    if (_klb == _kub) {
      assert(ublow == lb);
    }
    return ublow;
  }


  forceinline void
  ValNode::card_conflict(int c) {
    noc = c;
  }

  forceinline void
  ValNode::red_conflict(void) {
    noc--;
    assert(noc >= 0);
  }

  forceinline int
  ValNode::card_conflict(void) const {
    return noc;
  }

  forceinline bool
  ValNode::is_matched(BC d) const {
    if (d == UBC) {
      return matched<UBC>();
    } else {
      return ublow == 0;
    }
  }

  forceinline void
  ValNode::reset(void) {
    lb = _klb;
    ublow = _kub;
    ub = _kub;
    noe = 0;
  }

  template<BC direction>
  forceinline int
  ValNode::kbound(void) const {
    if (direction == UBC) {
      return _kub;
    } else {
      return _klb;
    }
  }

  forceinline int
  ValNode::kmax(void) const {
    return _kub;
  }

  forceinline int
  ValNode::kmin(void) const {
    return _klb;
  }

  forceinline void
  ValNode::set_kmin(int klb) {
    _klb = klb;
  }

  forceinline void
  ValNode::set_kmax(int kub) {
    _kub = kub;
  }

  template<BC direction>
  forceinline int
  ValNode::cap(void) const {
    if (direction == UBC) {
      return ub;
    } else {
      return lb;
    }
  }

  template<BC direction>
  forceinline void
  ValNode::dec(void) {
    if (direction == UBC) {
      ub--;
    } else {
      lb--;
      ublow--;
    }
  }

  template<BC direction>
  forceinline void
  ValNode::inc(void) {
    if (direction == UBC) {
      ub++;
    } else {
      lb++;
      ublow++;
    }
  }

  template<BC direction>
  forceinline void
  ValNode::match(void) {
    dec<direction>();
  }

  template<BC direction>
  forceinline void
  ValNode::unmatch(void) {
    inc<direction>();
  }

  template<BC direction>
  forceinline bool
  ValNode::matched(void) const {
    return cap<direction>() == 0;
  }

  template<BC direction>
  forceinline void
  ValNode::set_cap(int c) {
    if (direction == UBC)
      ub = c;
    else
      lb = c;
  }

  forceinline void
  ValNode::set_info(int i) {
    idx = i;
  }

  forceinline int
  ValNode::get_info(void) const {
    return idx;
  }

  forceinline void
  ValNode::inc(void) {
    _kcount++;
  }

  forceinline int
  ValNode::kcount(void) const {
    return _kcount;
  }

  forceinline void
  ValNode::kcount(int c) {
    _kcount = c;
  }

  forceinline void
  ValNode::kindex(int i) {
    _kidx = i;
  }

  forceinline int
  ValNode::kindex(void) const {
    return _kidx;
  }

  /// Returs the number of incident matching edges on the node
  template<BC direction>
  forceinline int
  ValNode::incid_match(void) const {
    if (direction == LBC)
      return _kub - ublow + _kcount;
    else
      return _kub - ub + _kcount;
  }


  forceinline bool
  ValNode::sink(void) const {
    // there are only incoming edges
    // in case of the UBC-matching
    return (_kub - ub == noe);
  }

  forceinline bool
  ValNode::source(void) const {
    // there are only incoming edges
    // in case of the UBC-matching
    return (_klb - lb == noe);
  }



  /*
   * Edges
   *
   */
  forceinline void
  Edge::unlink(void) {
    // unlink from variable side
    Edge* p = prev_edge;
    Edge* n = next_edge;

    if (p != NULL)
      *p->next_ref() = n;
    if (n != NULL)
      *n->prev_ref() = p;

    if (this == x->first()) {
      Edge** ref = x->adj();
      *ref = n;
      x->first(n);
    }

    if (this == x->last())
      x->last(p);

    // unlink from value side
    Edge* pv = prev_vedge;
    Edge* nv = next_vedge;

    if (pv != NULL)
      *pv->vnext_ref() = nv;
    if (nv != NULL)
      *nv->vprev_ref() = pv;
    if (this == v->first()) {
      Edge** ref = v->adj();
      *ref = nv;
      v->first(nv);
    }
    if (this == v->last())
      v->last(pv);
  }

  forceinline
  Edge::Edge(VarNode* var, ValNode* val) :
    x(var), v(val),
    next_edge(NULL), prev_edge(NULL),
    next_vedge(NULL), prev_vedge(NULL), ef(EF_NONE) {}

  template<BC direction>
  forceinline void
  Edge::use(void) {
    if (direction == UBC) {
      ef |= EF_MRKUB;
    } else {
      ef |= EF_MRKLB;
    }
  }

  template<BC direction>
  forceinline void
  Edge::free(void) {
    /// the failure is here, capacity is not increased for value nodes
    if (direction == UBC) {
      ef &= ~EF_MRKUB;
    } else {
      ef &= ~EF_MRKLB;
    }
  }

  template<BC direction>
  forceinline void
  Edge::reset(void) {
    this->free<direction>();
    this->unmatch<direction>();
  }

  template<BC direction>
  forceinline bool
  Edge::used(void) const {
    if (direction == UBC) {
      return (ef & EF_MRKUB) != 0;
    } else {
      return (ef & EF_MRKLB) != 0;
    }
  }

  forceinline Edge*
  Edge::next(void) const {
    return next_edge;
  }

  forceinline Edge*
  Edge::next(bool t) const {
    if (t) {
      return next_vedge;
    } else {
      return next_edge;
    }
  }

  forceinline Edge*
  Edge::vnext(void) const {
    return next_vedge;
  }

  forceinline Edge**
  Edge::vnext_ref(void) {
    return &next_vedge;
  }

  forceinline Edge*
  Edge::prev(void) const {
    return prev_edge;
  }

  forceinline Edge**
  Edge::prev_ref(void) {
    return &prev_edge;
  }

  forceinline Edge*
  Edge::vprev(void) const {
    return prev_vedge;
  }

  forceinline Edge**
  Edge::vprev_ref(void) {
    return &prev_vedge;
  }

  forceinline Edge**
  Edge::next_ref(void) {
    return &next_edge;
  }
  forceinline VarNode*
  Edge::getVar(void) const {
    assert(x != NULL);
    return x;
  }

  forceinline ValNode*
  Edge::getVal(void) const {
    assert(v != NULL);
    return v;
  }

  forceinline Node*
  Edge::getMate(bool type) const {
    if (type)
      return x;
    else
      return v;
  }

  template<BC direction>
  forceinline void
  Edge::unmatch(void) {
    if (direction == UBC) {
      ef &= ~EF_UM;
    } else {
      ef &= ~EF_LM;
    }
    x->unmatch<direction>();
    v->unmatch<direction>();
  }

  template<BC direction>
  forceinline void
  Edge::unmatch(bool node) {
    if (direction == UBC) {
      ef &= ~EF_UM;
    } else {
      ef &= ~EF_LM;
    }
    if (node) {
      v->template unmatch<direction>();
    } else {
      x->template unmatch<direction>();
    }
  }

  template<BC direction>
  forceinline void
  Edge::match(void) {
    if (direction == UBC) {
      ef |= EF_UM;
      x->template match<direction>();
      x->template set_match<direction>(this);
      v->template match<direction>();
    } else {
      ef |= EF_LM;
      x->template match<direction>();
      x->template set_match<direction>(this);
      assert(x->template matched<direction>());
      v->template match<direction>();
    }
  }

  template<BC direction>
  forceinline bool
  Edge::matched(void) const {
    if (direction == UBC) {
      return (ef & EF_UM) != 0;
    } else {
      return (ef & EF_LM) != 0;
    }
  }

  forceinline void
  Edge::del_edge(void) {
    ef |= EF_DEL;
  }

  forceinline void
  Edge::insert_edge(void) {
    ef &= ~EF_DEL;
  }


  forceinline bool
  Edge::deleted(void) const {
    return (ef & EF_DEL) != 0;
  }




  /**
   * \brief Constructor for the variable-value-graph
   *
   * The variable parition is initialized with the variables from \a x,
   * the value partition is initialized with the values from \a v,
   * where \f$ \forall v_i\in V:\f$ \a val_idx[i] is the
   * index of \f$ v_i\f$ in \f$ k \f$. \a nov denotes the cardinality of the
   * value partition in the graph, \a noe the number of edges in the
   * graph, \a min_occ the sum over the minimal occurences
   * of all values and \a max_occ the sum over the maximal occurences
   * of all values.
   **/

  template<class Card>
  VarValGraph<Card>::VarValGraph(Space& home,
                                 ViewArray<IntView>& xref,
                                 ViewArray<IntView>& yref,
                                 ViewArray<Card>& kref,
                                 int noe, int smin, int smax)
    : x(xref),
      y(yref),
      k(kref),
      n_var(x.size()),
      n_val(k.size()),
      n_edge(noe),
      node_size(n_var + n_val),
      sum_min(smin),
      sum_max(smax) {

    edges = home.alloc<Edge>(n_edge);
    VarNode* vars_ptr = home.alloc<VarNode>(n_var);
    ValNode* vals_ptr = home.alloc<ValNode>(n_val);
    vars = home.alloc<VarNode*>(n_var);
    vals = home.alloc<ValNode*>(n_val);

    for (int i = n_val; i--; ) {
      int kmi = k[i].min();
      int kma = k[i].max();
      int kc  = k[i].counter();
      if (kc != kma) {
        if (kmi >= kc) {
          kmi -=kc;
          assert(kmi >=0);
        } else {
          kmi = 0;
        }
        kma -= kc;
        assert (kma > 0);
        vals[i] = new (vals_ptr + i)
          ValNode(kmi, kma, k[i].card(), i, i + n_var, kc);
      } else {
        vals[i] = new (vals_ptr + i)
          ValNode(0, 0, k[i].card(), i, i + n_var, kc);
      }
    }

    for (int i = n_var; i--; ) {

      vars[i]          = new (vars_ptr + i) VarNode(i, i);
      VarNode* vrn     = vars[i];
      // get the space for the edges of the varnode
      Edge** xadjacent = vrn->adj();

      ViewValues<IntView> xiter(x[i]);
      int j = 0;
      for (; xiter(); ++xiter) {
        int v = xiter.val();
        // get the correct index for the value
        while(vals[j]->val < v) {
          j++;
        }
        ValNode* vln = vals[j];
        *xadjacent = new (edges) Edge(vars_ptr + i, vals_ptr + j);
        vrn->noe++;
        if (vrn->first() == NULL) {
          vrn->first(*xadjacent);
        }
        Edge* oldprev  = vrn->last();
        vrn->last(*xadjacent);
        Edge** newprev = vrn->last()->prev_ref();
        *newprev       = oldprev;

        if (vln->first() == NULL) {
          vln->first(*xadjacent);
          vln->last(*xadjacent);
          vln->noe++;
        } else {
          Edge* old      = vln->first();
          vln->first(*xadjacent);
          Edge** newnext = vln->first()->vnext_ref();
          *newnext       = old;
          Edge** setprev = old->vprev_ref();
          *setprev       = vln->first();
          vln->noe++;
        }
        edges++;
        xadjacent = (*xadjacent)->next_ref();
      }
      *xadjacent = NULL;
    }
  }


  template<class Card>
  inline ExecStatus
  VarValGraph<Card>::min_require(Space& home) {
    for (int i = n_val; i--; ) {
      ValNode* vln = vals[i];
      if (vln->noe > 0) {
        if (k[i].min() == vln->noe) {
          // all variable nodes reachable from vln should be equal to vln->val
          for (Edge* e = vln->first(); e != NULL; e = e->vnext()) {
            VarNode* vrn = e->getVar();
            int vi = vrn->get_info();
            for (Edge* f = vrn->first(); f != NULL; f = f->next()) {
              if (f != e) {
                ValNode* w = f->getVal();
                w->noe--;
                vrn->noe--;
                f->del_edge();
                f->unlink();
              }
            }
            assert(vrn->noe == 1);

            GECODE_ME_CHECK(x[vi].eq(home, vln->val));

            vars[vi] = vars[--n_var];
            vars[vi]->set_info(vi);

            int n = x.size();
            x[vi] = x[--n];
            x.size(n);

            node_size--;
            vln->noe--;
          }


          int vidx = vln->kindex();
          if (Card::propagate)
            GECODE_ME_CHECK(k[vidx].eq(home, k[vidx].min()));

          k[vidx].counter(k[vidx].min());

          vln->template set_cap<UBC>(0);
          vln->template set_cap<LBC>(0);
          vln->set_maxlow(0);

          if (sum_min && sum_min >= k[vidx].min())
            sum_min -= k[vidx].min();
          assert(sum_min >=0);

          if (sum_max && sum_max >= k[vidx].max())
            sum_max -= k[vidx].max();
          assert(sum_max >=0);
        }
      } else {
        vals[i]->template set_cap<UBC>(0);
        vals[i]->template set_cap<LBC>(0);
        vals[i]->set_maxlow(0);
        vals[i]->set_kmax(0);
        vals[i]->set_kmin(0);
      }

      if (Card::propagate && (k[i].counter() == 0))
        GECODE_ME_CHECK(k[i].lq(home, vals[i]->noe));
    }

    for (int i = n_val; i--; )
      vals[i]->set_info(n_var + i);

    return ES_OK;
  }

  template<class Card>
  inline ExecStatus
  VarValGraph<Card>::sync(Space& home) {
    Region r(home);
    NodeStack re(r,node_size);

    // synchronize cardinality variables
    if (Card::propagate) {
      for (int i = n_val; i--; ) {
        ValNode* v  = vals[i];
        int inc_ubc = v->template incid_match<UBC>();
        int inc_lbc = v->template incid_match<LBC>();
        if (v->noe == 0) {
          inc_ubc = 0;
          inc_lbc = 0;
        }
        int rm = v->kmax() - k[i].max();
        // the cardinality bounds have been modified
        if (k[i].max() < v->kmax() || k[i].min() > v->kmin()) {
          if (k[i].max() != k[i].counter() || k[i].max() == 0) {
            // update the bounds
            v->set_kmax(k[i].max());
            v->set_kmin(k[i].min());

            //everything is fine
            if (inc_ubc <= k[i].max()) {
              // adjust capacities
              v->template set_cap<UBC>(k[i].max() - (inc_ubc));
              v->set_maxlow(k[i].max() - (inc_lbc));
              if (v->kmin() == v->kmax())
                v->template set_cap<LBC>(k[i].max() - (inc_lbc));
            } else {
              // set cap to max and resolve conflicts on view side
              // set to full capacity for later rescheduling
              if (v->template cap<UBC>())
                v->template set_cap<UBC>(k[i].max());
              v->set_maxlow(k[i].max() - (inc_lbc));
              if (v->kmin() == v->kmax())
                v->template set_cap<LBC>(k[i].max() - (inc_lbc));
              v->card_conflict(rm);
            }
          }
        }
        if (inc_lbc < k[i].min() && v->noe > 0) {
          v->template set_cap<LBC>(k[i].min() - inc_lbc);
          re.push(v);
        }
      }

      for (int i = n_var; i--; ) {
        Edge* mub = vars[i]->template get_match<UBC>();
        if (mub != NULL) {
          ValNode* vu = mub->getVal();
          if ((vars[i]->noe != 1) && vu->card_conflict()) {
            vu->red_conflict();
            mub->template unmatch<UBC>(vars[i]->get_type());
            re.push(vars[i]);
          }
        }
      }
    }

    // go on with synchronization
    assert(x.size() == n_var);
    for (int i = n_var; i--; ) {

      VarNode* vrn = vars[i];
      if (static_cast<int>(x[i].size()) != vrn->noe) {
        // if the variable is already assigned
        if (x[i].assigned()) {
          int  v = x[i].val();
          ValNode* rv = NULL;
          int rv_idx  = 0;
          Edge* mub = vrn->template get_match<UBC>();
          if ((mub != NULL) && (v != mub->getVal()->val)) {
            mub->template unmatch<UBC>();
            re.push(vars[i]);
          }

          Edge* mlb = vrn->template get_match<LBC>();
          if (mlb != NULL) {
            ValNode* vln = mlb->getVal();
            if (v != vln->val) {
              mlb->template unmatch<LBC>();
              if (vln->template incid_match<LBC>() < vln->kmin())
                re.push(vln);
            }
          }

          for (Edge* e = vrn->first(); e != NULL; e = e->next()) {
            ValNode* vln = e->getVal();
            if (vln->val != v) {
              vrn->noe--;
              e->getVal()->noe--;
              e->del_edge();
              e->unlink();
            } else {
              rv = e->getVal();
              rv_idx = rv->kindex();
            }
          }
        } else {

          // delete the edge
          ViewValues<IntView> xiter(x[i]);
          Edge*  mub = vrn->template get_match<UBC>();
          Edge*  mlb = vrn->template get_match<LBC>();
          Edge** p   = vrn->adj();
          Edge*  e   = *p;
          do {
            // search the edge that has to be deleted
            while (e != NULL && (e->getVal()->val < xiter.val())) {
              // Skip edge
              e->getVal()->noe--;
              vrn->noe--;
              e->del_edge();
              e->unlink();
              e = e ->next();
              *p = e;
            }

            assert(xiter.val() == e->getVal()->val);

            // This edge must be kept
            e->template free<UBC>();
            e->template free<LBC>();
            ++xiter;
            p = e->next_ref();
            e = e->next();
          } while (xiter());
          *p = NULL;
          while (e) {
            e->getVar()->noe--;
            e->getVal()->noe--;
            e->del_edge();
            e->unlink();
            e = e->next();
          }

          if ((mub != NULL) && mub->deleted()) {
            mub->template unmatch<UBC>();
            re.push(vars[i]);
          }

          //lower bound matching can be zero
          if ((mlb != NULL) && mlb->deleted()) {
            ValNode* vln = mlb->getVal();
            mlb->template unmatch<LBC>();
            if (vln->template incid_match<LBC>() < vln->kmin())
              re.push(vln);
          }
        }
      }
      vars[i]->set_info(i);
    }

    for (int i = n_val; i--; ) {
      if ((k[i].min() > vals[i]->noe) && (k[i].counter() == 0))
        return ES_FAILED;
      vals[i]->set_info(n_var + i);
    }

    // start repair
    while (!re.empty()) {
      Node* n = re.pop();
      if (!n->removed())
        if (!n->get_type()) {
          VarNode* vrn = static_cast<VarNode*>(n);
          if (!vrn->template matched<UBC>() &&
              !augmenting_path<UBC>(home,vrn))
            return ES_FAILED;
        } else {
          ValNode* vln = static_cast<ValNode*>(n);
          while (!vln->template matched<LBC>())
            if (!augmenting_path<LBC>(home,vln))
              return ES_FAILED;
        }
    }

    return ES_OK;
  }

  template<class Card> template<BC direction>
  inline ExecStatus
  VarValGraph<Card>::narrow(Space& home) {
    for (int i = n_var; i--; ) {
      VarNode* vrn = vars[i];
      if (vrn->noe == 1) {
        Edge* e = vrn->first();
        ValNode* v = e->getVal();
        e->template free<direction>();
        GECODE_ME_CHECK(x[i].eq(home, v->val));
        v->inc();
      }
    }
    for (int i = n_val; i--; ) {
      ValNode* v = vals[i];
      if (Card::propagate && (k[i].counter() == 0))
        GECODE_ME_CHECK(k[i].lq(home, v->noe));
      if (v->noe > 0) {
        if (Card::propagate)
          GECODE_ME_CHECK(k[i].lq(home, v->noe));

        // If the maximum number of occurences of a value is reached
        // it cannot be consumed by another view

        if (v->kcount() == v->kmax()) {
          int vidx = v->kindex();

          k[i].counter(v->kcount());

          if (Card::propagate)
            GECODE_ME_CHECK(k[i].eq(home, k[i].counter()));

          bool delall = v->card_conflict() && (v->noe > v->kmax());

          for (Edge* e = v->last(); e != NULL; e = e->vprev()) {
            VarNode* vrn = e->getVar();
            if (vrn->noe == 1) {
              vrn->noe--;
              v->noe--;
              int vi= vrn->get_info();

              int n = x.size();
              x[vi] = x[--n];
              x.size(n);

              vars[vi] = vars[--n_var];
              vars[vi]->set_info(vi);
              node_size--;
              e->del_edge();
              e->unlink();

            } else if (delall) {
              GECODE_ME_CHECK(x[vrn->get_info()].nq(home, v->val));
              vrn->noe--;
              v->noe--;
              e->del_edge();
              e->unlink();
            }
          }
          v->template set_cap<UBC>(0);
          v->template set_cap<LBC>(0);
          v->set_maxlow(0);
          if (sum_min && sum_min >= k[vidx].min())
            sum_min -= k[vidx].min();
          assert(sum_min >=0);

          if (sum_max && sum_max >= k[vidx].max())
            sum_max -= k[vidx].max();
          assert(sum_max >=0);

        } else if (v->kcount() > 0) {
          v->kcount(0);
        }
      }
    }
    for (int i = n_var; i--; )
      vars[i]->set_info(i);

    for (int i = n_val; i--; ) {
      if (vals[i]->noe == 0) {
        vals[i]->template set_cap<UBC>(0);
        vals[i]->template set_cap<LBC>(0);
        vals[i]->set_maxlow(0);
      }
      vals[i]->set_info(n_var + i);
    }

    for (int i = n_var; i--; )
      if (vars[i]->noe > 1)
        for (Edge* e = vars[i]->first(); e != NULL; e = e->next())
          if (!e->template matched<direction>() &&
              !e->template used<direction>()) {
            GECODE_ME_CHECK(x[i].nq(home, e->getVal()->val));
          } else {
            e->template free<direction>();
          }

    return ES_OK;
  }

  template<class Card>  template<BC direction>
  inline ExecStatus
  VarValGraph<Card>::maximum_matching(Space& home) {
    int required_size = 0;
    int card_match    = 0;

    if (direction == UBC)
      required_size = n_var;
    else
      required_size = sum_min;

    // find an intial matching in O(n*d)
    // greedy algorithm
    for (int i = n_val; i--; ) {
      ValNode* vln = vals[i];
      for (Edge* e = vln->first(); e != NULL ; e = e->vnext()) {
        VarNode* vrn = e->getVar();
        if (!vrn->template matched<direction>() &&
            !vln->template matched<direction>()) {
          e->template match<direction>();
          card_match++;
        }
      }
    }

    Region r(home);
    if (card_match < required_size) {
      if (direction == LBC) {
        // collect free value nodes
        ValNode** free = r.alloc<ValNode*>(n_val);
        int f = 0;
        // find failed nodes
        for (int i = n_val; i--; ) {
          ValNode* vln = vals[i];
          if (!vln->template matched<direction>())
            free[f++] = vln;
        }

        for (int i = 0; i < f; i++) {
          while(!free[i]->template matched<direction>()) {
            if (augmenting_path<direction>(home,free[i])) {
              card_match++;
            } else {
              break;
            }
          }
        }
      } else {
        VarNode** free = r.alloc<VarNode*>(n_var);
        int f = 0;
        // find failed nodes
        for (int i = n_var; i--; ) {
          VarNode* vrn = vars[i];
          if (!vrn->template matched<direction>())
            free[f++] = vrn;
        }

        for (int i = 0; i < f; i++) {
          if (!free[i]->template matched<direction>()) {
            if (augmenting_path<direction>(home,free[i])) {
              card_match++;
            }
          }
        }
      }
    }
    return (card_match >= required_size) ? ES_OK : ES_FAILED;
  }

  template<class Card> template<BC direction>
  inline bool
  VarValGraph<Card>::augmenting_path(Space& home, Node* v) {
    Region r(home);
    NodeStack ns(r,node_size);
    BitSet visited(r,node_size);
    Edge** start = r.alloc<Edge*>(node_size);

    // augmenting path starting in a free var node
    assert(!v->is_matched(direction));

    // keep track of the nodes that have already been visited
    Node* sn = v;

    // mark the start partition
    bool sp = sn->get_type();

    // nodes in sp only follow free edges
    // nodes in V - sp only follow matched edges

    for (int i = node_size; i--; )
      if (i >= n_var) {
        vals[i-n_var]->inedge(NULL);
        start[i] = vals[i-n_var]->first();
      } else {
        vars[i]->inedge(NULL);
        start[i] = vars[i]->first();
      }

    v->inedge(NULL);
    ns.push(v);
    visited.set(v->get_info());
    while (!ns.empty()) {
      Node* v = ns.top();
      Edge* e = NULL;
      if (v->get_type() == sp) {
        // follow next free edge
        e = start[v->get_info()];
        while ((e != NULL) && e->template matched<direction>()) {
          e = e->next(v->get_type());
        }
      } else {
        e = start[v->get_info()];
        while ((e != NULL) && !e->template matched<direction>()) {
          e = e->next(v->get_type());
        }
        start[v->get_info()] = e;
      }
      if (e != NULL) {
        start[v->get_info()] = e->next(v->get_type());
        Node* w = e->getMate(v->get_type());
        if (!visited.get(w->get_info())) {
          // unexplored path
          if (!w->is_matched(direction) && w->get_type() != sp) {
            if (v->inedge() != NULL) {
              // augmenting path of length l > 1
              e->template match<direction>();
              break;
            } else {
              // augmenting path of length l = 1
              e->template match<direction>();
              ns.pop();
              return true;
            }
          } else {
            w->inedge(e);
            visited.set(w->get_info());
            // find matching edge m incident with w
            ns.push(w);
          }
        }
      } else {
        // tried all outgoing edges without finding an augmenting path
        ns.pop();
      }
    }

    bool pathfound = !ns.empty();

    while (!ns.empty()) {
      Node* t = ns.pop();
      if (t != sn) {
        Edge* in = t->inedge();
        if (t->get_type() != sp) {
          in->template match<direction>();
        } else if (!sp) {
          in->template unmatch<direction>(!sp);
        } else {
          in->template unmatch<direction>();
        }
      }
    }
    return pathfound;
  }

  template<class Card> template<BC direction>
  inline void
  VarValGraph<Card>::free_alternating_paths(Space& home) {
    Region r(home);
    NodeStack ns(r,node_size);
    BitSet visited(r,node_size);

    if (direction == LBC) {
      // after a maximum matching on the value nodes there still can be
      // free value nodes, hence we have to consider ALL nodes whether
      // they are the starting point of an even alternating path in G
      for (int i = n_var; i--; )
        if (!vars[i]->is_matched(LBC)) {
          ns.push(vars[i]); visited.set(vars[i]->get_info());
        }
      for (int i = n_val; i--; )
        if (!vals[i]->is_matched(LBC)) {
          ns.push(vals[i]); visited.set(vals[i]->get_info());
        }

    } else {
      // clearly, after a maximum matching on the x variables
      // corresponding to a set cover on x there are NO free var nodes
      for (int i = n_val; i--; )
        if (!vals[i]->is_matched(UBC)) {
          ns.push(vals[i]); visited.set(vals[i]->get_info());
        }
    }

    while (!ns.empty()) {
      Node* node = ns.top();
      ns.pop();
      if (node->get_type()) {
        // ValNode
        ValNode* vln = static_cast<ValNode*>(node);
        for (Edge* cur = vln->first(); cur != NULL; cur = cur->vnext()) {
          VarNode* mate = cur->getVar();
          bool follow = false;
          switch (direction) {
            // edges in M_l are directed from values to variables
          case LBC:
            follow = cur->template matched<direction>();
            break;
          case UBC:
            follow = !cur->template matched<direction>();
            break;
          default: GECODE_NEVER;
          }
          if (follow) {
            // mark the edge
            cur->template use<direction>();
            if (!visited.get(mate->get_info())) {
              ns.push(mate); visited.set(mate->get_info());
            }
          }
        }
      } else {
        // VarNode
        VarNode* vrn = static_cast<VarNode*>(node);
        switch (direction) {
        case LBC: 
          // after LBC-matching we can follow every unmatched edge
          for (Edge* cur = vrn->first(); cur != NULL; cur = cur->next()) {
            ValNode* mate = cur->getVal();
            if (!cur->template matched<LBC>()) {
              cur->template use<LBC>();
              if (!visited.get(mate->get_info())) {
                ns.push(mate); visited.set(mate->get_info());
              }
            }
          }
          break;
        case UBC: 
          // after ub-matching we can only follow a matched edge
          {
            Edge* cur = vrn->template get_match<UBC>();
            if (cur != NULL) {
              cur->template use<UBC>();
              ValNode* mate = cur->getVal();
              if (!visited.get(mate->get_info())) {
                ns.push(mate); visited.set(mate->get_info());
              }
            }
          }
          break;
        default: 
          GECODE_NEVER;
        }
      }
    }
  }

  template<class Card> template<BC direction>
  inline void
  VarValGraph<Card>::dfs(Node* v,
                         BitSet& inscc, BitSet& in_unfinished, int dfsnum[],
                         NodeStack& roots, NodeStack& unfinished,
                         int& count) {
    count++;
    int v_index            = v->get_info();
    dfsnum[v_index]        = count;
    inscc.set(v_index);
    in_unfinished.set(v_index);

    unfinished.push(v);
    roots.push(v);
    for (Edge* e = v->first(); e != NULL; e = e->next(v->get_type())) {
      bool condition = false;
      // LBC-matching
      if (direction == LBC) {
        // ValNode
        if (v->get_type()) {
          condition = e->template matched<LBC>();
        } else {
          condition = !e->template matched<LBC>();
        }
        // UBC - matching
      } else {
        if (v->get_type()) {
          // in an upper bound matching a valnode only can follow unmatched edges
          condition = !e->template matched<UBC>();
        } else {
          condition = e->template matched<UBC>();
        }
      }
      if (condition) {
        Node* w = e->getMate(v->get_type());
        int w_index = w->get_info();

        assert(w_index < node_size);
        if (!inscc.get(w_index)) {
          // w is an uncompleted scc
          w->inedge(e);
          dfs<direction>(w, inscc, in_unfinished, dfsnum,
                         roots, unfinished, count);
        } else if (in_unfinished.get(w_index)) {
          // even alternating cycle found mark the edge closing the cycle,
          // completing the scc
          e->template use<direction>();
          // if w belongs to an scc we detected earlier
          // merge components
          assert(roots.top()->get_info() < node_size);
          while (dfsnum[roots.top()->get_info()] > dfsnum[w_index]) {
            roots.pop();
          }
        }
      }
    }

    if (v == roots.top()) {
      while (v != unfinished.top()) {
        // w belongs to the scc with root v
        Node* w = unfinished.top();
        w->inedge()->template use<direction>();
        in_unfinished.clear(w->get_info());
        unfinished.pop();
      }
      assert(v == unfinished.top());
      in_unfinished.clear(v_index);
      roots.pop();
      unfinished.pop();
    }
  }

  template<class Card> template<BC direction>
  inline void
  VarValGraph<Card>::strongly_connected_components(Space& home) {
    Region r(home);
    BitSet inscc(r,node_size);
    BitSet in_unfinished(r,node_size);
    int* dfsnum = r.alloc<int>(node_size);

    for (int i = node_size; i--; )
      dfsnum[i]=0;

    int count = 0;
    NodeStack roots(r,node_size);
    NodeStack unfinished(r,node_size);

    for (int i = n_var; i--; )
      dfs<direction>(vars[i], inscc, in_unfinished, dfsnum,
                     roots, unfinished, count);
  }

  template<class Card>
  forceinline void*
  VarValGraph<Card>::operator new(size_t t, Space& home) {
    return home.ralloc(t);
  }

}}}

// STATISTICS: int-prop


