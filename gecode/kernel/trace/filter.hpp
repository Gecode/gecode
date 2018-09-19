/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

namespace Gecode {

  class TraceFilter;

  /**
   * \brief Trace filter expressions
   * \ingroup TaskTrace
   */
  class TFE {
    friend GECODE_KERNEL_EXPORT TFE operator -(const TFE& r);
    friend GECODE_KERNEL_EXPORT TFE propagator(PropagatorGroup g);
    friend GECODE_KERNEL_EXPORT TFE post(PropagatorGroup g);
    friend class TraceFilter;
  protected:
    /// Type of trace filter expression
    enum NodeType {
      NT_GROUP,    ///< Propagator or brancher group
      NT_NEGATE,   ///< Negation of expression
      NT_ADD       ///< More than one expression
    };
    /// Node for trace filter expression
    class Node : public HeapAllocated {
    public:
      /// Nodes are reference counted
      unsigned int use;
      /// Type of expression
      NodeType t;
      /// Number of leaf groups
      int n;
      /// Group
      Group g;
      /// Which operations to consider for propagator groups
      char w;
      /// Subexpressions
      Node *l, *r;
      /// Default constructor
      Node(void);
      /// Decrement reference count and possibly free memory
      GECODE_KERNEL_EXPORT
      bool decrement(void);
    };
    /// Pointer to trace filter expression node
    Node* n;
    /// Initialize with no node
    TFE(void);
    /// Initialize with node \a n
    TFE(Node* n);
    /// Initialize with propagator group \a g and flags \a what
    void init(Group g, char what);
    /// Return negated the expresssion
    TFE negate(void) const;
  public:
    /// An expression for a propagator group \a g
    GECODE_KERNEL_EXPORT
    TFE(PropagatorGroup g);
    /// An expression for a brancher group \a g
    GECODE_KERNEL_EXPORT
    TFE(BrancherGroup g);
    /// Expression for other than propagator, brancher, or post
    GECODE_KERNEL_EXPORT
    static TFE other(void);
    /// Copy constructor
    GECODE_KERNEL_EXPORT
    TFE(const TFE& e);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    TFE& operator =(const TFE& e);
    /// Add expression \a e
    GECODE_KERNEL_EXPORT
    TFE& operator +=(const TFE& e);
    /// Add expression \a e as negative expression
    GECODE_KERNEL_EXPORT
    TFE& operator -=(const TFE& e);
    /// Destructor
    GECODE_KERNEL_EXPORT
    ~TFE(void);
  };

  /// Disjunctive combination of trace filter expressions \a l and \a r
  TFE operator +(TFE l, const TFE& r);
  /// Positive expression
  TFE operator +(const TFE& e);
  /// Combine positive expression \a l and negative expression \a r
  TFE operator -(TFE l, const TFE& r);
  /// Return negative expression of \a e
  GECODE_KERNEL_EXPORT
  TFE operator -(const TFE& e);
  /// Only propagators (but not post functions) from \a g are considered
  GECODE_KERNEL_EXPORT
  TFE propagator(PropagatorGroup g);
  /// Only post functions (but not propagators) from \a g are considered
  GECODE_KERNEL_EXPORT
  TFE post(PropagatorGroup g);


  /**
   * \brief Trace filters
   * \ingroup TaskTrace
   */
  class TraceFilter : public SharedHandle {
  protected:
    /// The actual object storing the shared filters
    class TFO : public SharedHandle::Object {
    public:
      /// Filter information
      struct Filter {
        /// The filter group
        Group g;
        /// Whether the filter is negative
        bool neg;
        /// One bit set for each operation type
        char what;
      };
      class StackFrame {
      public:
        /// The node
        TFE::Node* n;
        /// Whether it is negated
        bool neg;
        /// Default constructor
        StackFrame(void);
        /// Initialize
        StackFrame(TFE::Node* n, bool neg);
      };
      /// The number of filters
      int n;
      /// The filters
      Filter* f;
      /// Fill the filters
      GECODE_KERNEL_EXPORT
      void fill(TFE::Node* n);
      /// Initialize without any filter and with fixpoint and done tracing
      TFO(void);
      /// Initialize with trace filter expression \a e
      TFO(const TFE& e);
      /// Initialize with propagator group \a g
      TFO(PropagatorGroup g);
      /// Initialize with brancher group \a g
      TFO(BrancherGroup g);
      /// Check whether filter is true for view trace information \a vti
      bool operator ()(const ViewTraceInfo& vti) const;
      /// Check whether filter is true for propagator group \a pg
      bool operator ()(PropagatorGroup pg) const;
      /// Check whether filter is true for brancher group \a bg
      bool operator ()(BrancherGroup bg) const;
      /// Destructor
      GECODE_KERNEL_EXPORT
      virtual ~TFO(void);
    };
  public:
    /// Initialize without any filter
    GECODE_KERNEL_EXPORT
    TraceFilter(void);
    /// Initialize with trace filter expression \a e
    GECODE_KERNEL_EXPORT
    TraceFilter(const TFE& e);
    /// Initialize with propagator group \a g
    GECODE_KERNEL_EXPORT
    TraceFilter(PropagatorGroup g);
    /// Initialize with brancher group \a g
    GECODE_KERNEL_EXPORT
    TraceFilter(BrancherGroup g);
    /// Copy constructor
    GECODE_KERNEL_EXPORT
    TraceFilter(const TraceFilter& tf);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    TraceFilter& operator =(const TraceFilter& tf);
    /// Check whether filter is true for view trace information \a vti
    bool operator ()(const ViewTraceInfo& vti) const;
    /// Check whether filter is true for propagator group \a pg
    bool operator ()(PropagatorGroup pg) const;
    /// Check whether filter is true for brancher group \a bg
    bool operator ()(BrancherGroup bg) const;
    /// Default filter: without any filter
    GECODE_KERNEL_EXPORT
    static TraceFilter all;
  };



  /*
   * Trace expression filters
   *
   */

  forceinline
  TFE::Node::Node(void)
    : use(1), l(NULL), r(NULL) {}
  forceinline
  TFE::TFE(void) : n(NULL) {}
  forceinline
  TFE::TFE(TFE::Node* n0) : n(n0) {}

  forceinline TFE
  operator +(TFE l, const TFE& r) {
    l += r; return l;
  }
  forceinline TFE
  operator +(const TFE& e) {
    return e;
  }
  forceinline TFE
  operator -(TFE l, const TFE& r) {
    l -= r; return l;
  }


  /*
   * Trace filters
   *
   */
  forceinline
  TraceFilter::TFO::TFO(void) : n(0), f(NULL) {}
  forceinline
  TraceFilter::TFO::TFO(const TFE& e)
    : n(e.n->n),
      f((n == 0) ? NULL : heap.alloc<Filter>(n)) {
    if (n > 0)
      fill(e.n);
  }
  forceinline
  TraceFilter::TFO::TFO(PropagatorGroup g) : n(1) {
    f = heap.alloc<Filter>(1);
    f[0].g = g; f[0].neg = false;
    f[0].what = 1 << ViewTraceInfo::PROPAGATOR;

  }
  forceinline
  TraceFilter::TFO::TFO(BrancherGroup g) : n(1) {
    f = heap.alloc<Filter>(1);
    f[0].g = g; f[0].neg = false;
    f[0].what = 1 << ViewTraceInfo::BRANCHER;

  }
  forceinline bool
  TraceFilter::TFO::operator ()(const ViewTraceInfo& vti) const {
    if (n == 0)
      return true;
    for (int i=0; i<n; i++)
      if (f[i].what & (1 << vti.what())) {
        // Group is of the right type
        switch (vti.what()) {
        case ViewTraceInfo::PROPAGATOR:
          if (f[i].g.in(vti.propagator().group()) != f[i].neg)
            return true;
          break;
        case ViewTraceInfo::BRANCHER:
          if (f[i].g.in(vti.brancher().group()) != f[i].neg)
            return true;
          break;
        case ViewTraceInfo::POST:
          if (f[i].g.in(vti.post()) != f[i].neg)
            return true;
          break;
        case ViewTraceInfo::OTHER:
          return true;
        default:
          GECODE_NEVER;
        }
      }
    return false;
  }

  forceinline bool
  TraceFilter::operator ()(const ViewTraceInfo& vti) const {
    return static_cast<TFO*>(object())->operator ()(vti);
  }

  forceinline bool
  TraceFilter::TFO::operator ()(PropagatorGroup pg) const {
    if (n == 0)
      return true;
    for (int i=0; i<n; i++)
      if ((f[i].what & (1 << ViewTraceInfo::PROPAGATOR)) &&
          (f[i].g.in(pg) != f[i].neg))
        return true;
    return false;
  }

  forceinline bool
  TraceFilter::operator ()(PropagatorGroup pg) const {
    return static_cast<TFO*>(object())->operator ()(pg);
  }

  forceinline bool
  TraceFilter::TFO::operator ()(BrancherGroup bg) const {
    if (n == 0)
      return true;
    for (int i=0; i<n; i++)
      if ((f[i].what & (1 << ViewTraceInfo::BRANCHER)) &&
          (f[i].g.in(bg) != f[i].neg))
        return true;
    return false;
  }

  forceinline bool
  TraceFilter::operator ()(BrancherGroup bg) const {
    return static_cast<TFO*>(object())->operator ()(bg);
  }

}

// STATISTICS: kernel-trace
