/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2004
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

#ifndef __GECODE_INT_EXTENSIONAL_HH__
#define __GECODE_INT_EXTENSIONAL_HH__

#include <gecode/int.hh>

#include <gecode/int/rel.hh>

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */

namespace Gecode { namespace Int { namespace Extensional {

  /**
   * \brief Domain consistent layered graph (regular) propagator
   *
   * The algorithm for the regular propagator is based on:
   *   Gilles Pesant, A Regular Language Membership Constraint
   *   for Finite Sequences of Variables, CP 2004.
   *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
   *
   * The propagator is not capable of dealing with multiple occurences
   * of the same view.
   *
   * Requires \code #include <gecode/int/extensional.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View, class Val, class Degree, class StateIdx>
  class LayeredGraph : public Propagator {
  protected:
    /// States are described by number of incoming and outgoing edges
    class State {
    public:
      Degree i_deg; ///< The in-degree (number of incoming edges)
      Degree o_deg; ///< The out-degree (number of outgoing edges)
      /// Initialize with zeroes
      void init(void);
    };
    /// %Edge defined by in-state and out-state
    class Edge {
    public:
      StateIdx i_state; ///< Number of in-state
      StateIdx o_state; ///< Number of out-state
    };
    /// %Support information for a value
    class Support {
    public:
      Val val; ///< Supported value
      Degree n_edges; ///< Number of supporting edges
      Edge* edges; ///< Supporting edges in layered graph
    };
    /// Type for support size
    typedef typename Gecode::Support::IntTypeTraits<Val>::utype ValSize;
    /// %Layer for a view in the layered graph
    class Layer {
    public:
      View x; ///< Integer view
      StateIdx n_states; ///< Number of states used by outgoing edges
      ValSize size; ///< Number of supported values
      State* states; ///< States used by outgoing edges
      Support* support; ///< Supported values
    };
    /// Iterator for telling variable domains by scanning support
    class LayerValues {
    private:
      const Support* s1; ///< Current support
      const Support* s2; ///< End of support
    public:
      /// Default constructor
      LayerValues(void);
      /// Initialize for support of layer \a l
      LayerValues(const Layer& l);
      /// Initialize for support of layer \a l
      void init(const Layer& l);
      /// Test whether more values supported
      bool operator ()(void) const;
      /// Move to next supported value
      void operator ++(void);
      /// Return supported value
      int val(void) const;
    };
    /// %Advisors for views (by position in array)
    class Index : public Advisor {
    public:
      /// The position of the view in the view array
      int i;
      /// Create index advisor
      Index(Space& home, Propagator& p, Council<Index>& c, int i);
      /// Clone index advisor \a a
      Index(Space& home, Index& a);
    };
    /// Range approximation of which positions have changed
    class IndexRange {
    private:
      int _fst; ///< First index
      int _lst; ///< Last index
    public:
      /// Initialize range as empty
      IndexRange(void);
      /// Reset range to be empty
      void reset(void);
      /// Add index \a i to range
      void add(int i);
      /// Add index range \a ir to range
      void add(const IndexRange& ir);
      /// Shift index range by \a n elements to the left
      void lshift(int n);
      /// Test whether range is empty
      bool empty(void) const;
      /// Return first position
      int fst(void) const;
      /// Return last position
      int lst(void) const;
    };
    /// The advisor council
    Council<Index> c;
    /// Number of layers (and views)
    int n;
    /// The layers of the graph
    Layer* layers;
    /// Maximal number of states per layer
    StateIdx max_states;
    /// Total number of states
    unsigned int n_states;
    /// Total number of edges
    unsigned int n_edges;
    /// Index range with in-degree modifications
    IndexRange i_ch;
    /// Index range with out-degree modifications
    IndexRange o_ch;
    /// Index range for any change (for compression)
    IndexRange a_ch;
    /// Return in state for layer \a i and state index \a is
    State& i_state(int i, StateIdx is);
    /// Return in state for layer \a i and in state of edge \a e
    State& i_state(int i, const Edge& e);
    /// Decrement out degree for in state of edge \a e for layer \a i
    bool i_dec(int i, const Edge& e);
    /// Return out state for layer \a i and state index \a os
    State& o_state(int i, StateIdx os);
    /// Return state for layer \a i and out state of edge \a e
    State& o_state(int i, const Edge& e);
    /// Decrement in degree for out state of edge \a e for layer \a i
    bool o_dec(int i, const Edge& e);
    /// Perform consistency check on data structures
    void audit(void);
    /// Initialize layered graph
    template<class Var>
    ExecStatus initialize(Space& home,
                          const VarArgArray<Var>& x, const DFA& dfa);
    /// Constructor for cloning \a p
    LayeredGraph(Space& home, LayeredGraph<View,Val,Degree,StateIdx>& p);
  public:
    /// Constructor for posting
    template<class Var>
    LayeredGraph(Home home,
                 const VarArgArray<Var>& x, const DFA& dfa);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function (defined as high linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
    /// Post propagator on views \a x and DFA \a dfa
    template<class Var>
    static ExecStatus post(Home home,
                           const VarArgArray<Var>& x, const DFA& dfa);
  };

  /// Select small types for the layered graph propagator
  template<class Var>
  ExecStatus post_lgp(Home home,
                      const VarArgArray<Var>& x, const DFA& dfa);

}}}

#include <gecode/int/extensional/layered-graph.hpp>


namespace Gecode { namespace Int { namespace Extensional {

  /// Import type
  typedef Gecode::Support::BitSetData BitSetData;

  /*
   * Forward declarations
   */
  template<unsigned int size> class TinyBitSet;

  /// Bit-set
  template<class IndexType>
  class BitSet {
    template<class> friend class BitSet;
    template<unsigned int> friend class TinyBitSet;
  protected:
    /// Limit
    IndexType _limit;
    /// Indices
    IndexType* index;
    /// Words
    BitSetData* bits;
    /// Replace the \a i th word with \a w, decrease \a limit if \a w is zero
    void replace_and_decrease(IndexType i, BitSetData w);
  public:
    /// Initialize bit set for a number of words \a n
    BitSet(Space& home, unsigned int n);
    /// Initialize during cloning
    template<class OldIndexType>
    BitSet(Space& home, const BitSet<OldIndexType>& bs);
    /// Initialize during cloning (unused)
    BitSet(Space& home, const TinyBitSet<1U>& tbs);
    /// Initialize during cloning (unused)
    BitSet(Space& home, const TinyBitSet<2U>& tbs);
    /// Initialize during cloning (unused)
    BitSet(Space& home, const TinyBitSet<3U>& tbs);
    /// Initialize during cloning (unused)
    BitSet(Space& home, const TinyBitSet<4U>& tbs);
    /// Get the limit
    unsigned int limit(void) const;
    /// Check whether the set is empty
    bool empty(void) const;
    /// Return the highest active index
    unsigned int width(void) const;
    /// Clear the first \a limit words in \a mask
    void clear_mask(BitSetData* mask) const;
    /// Add \b to \a mask
    void add_to_mask(const BitSetData* b, BitSetData* mask) const;
    /// Intersect with \a mask, sparse mask if \a sparse is true
    template<bool sparse>
    void intersect_with_mask(const BitSetData* mask);
    /// Intersect with the "or" of \a and \a b
    void intersect_with_masks(const BitSetData* a, const BitSetData* b);
    /// Check if \a has a non-empty intersection with the set
    bool intersects(const BitSetData* b) const;
    /// Perform "nand" with \a b
    void nand_with_mask(const BitSetData* b);
    /// Return the number of required bit set words
    unsigned int words(void) const;
    /// Return the number of required bit set words
    unsigned int size(void) const;
  };

}}}

#include <gecode/int/extensional/bit-set.hpp>

namespace Gecode { namespace Int { namespace Extensional {

  /// Tiny bit-set
  template<unsigned int _size>
  class TinyBitSet {
    template<unsigned int> friend class TinyBitSet;
  protected:
    /// Words
    BitSetData bits[_size];
  public:
    /// Initialize sparse bit set for a number of words \a n
    TinyBitSet(Space& home, unsigned int n);
    /// Initialize during cloning
    template<unsigned int largersize>
    TinyBitSet(Space& home, const TinyBitSet<largersize>& tbs);
    /// Initialize during cloning
    template<class IndexType>
    TinyBitSet(Space& home, const BitSet<IndexType>& bs);
    /// Get the limit
    int limit(void) const;
    /// Check whether the set is empty
    bool empty(void) const;
    /// Return the highest active index
    unsigned int width(void) const;
    /// Clear the first \a limit words in \a mask
    void clear_mask(BitSetData* mask);
    /// Add \b to \a mask
    void add_to_mask(const BitSetData* b, BitSetData* mask) const;
    /// Intersect with \a mask, sparse mask if \a sparse is true
    template<bool sparse>
    void intersect_with_mask(const BitSetData* mask);
    /// Intersect with the "or" of \a and \a b
    void intersect_with_masks(const BitSetData* a, const BitSetData* b);
    /// Check if \a has a non-empty intersection with the set
    bool intersects(const BitSetData* b);
    /// Perform "nand" with \a b
    void nand_with_mask(const BitSetData* b);
    /// Perform "nand" with and the "or" of \a a and \a b
    void nand_with_masks(const BitSetData* a, const BitSetData* b);
    /// Return the number of required bit set words
    unsigned int words(void) const;
    /// Return the total number of words
    unsigned int size(void) const;
  };
      
}}}

#include <gecode/int/extensional/tiny-bit-set.hpp>

namespace Gecode { namespace Int { namespace Extensional {

  /// Tuple type
  typedef TupleSet::Tuple Tuple;

  /// Base class for compact table propagator
  template<class View>
  class Compact : public Propagator {
  protected:
    /// Range type for supports
    typedef TupleSet::Range Range;
    /// Advisor for updating current table
    class CTAdvisor : public ViewAdvisor<View> {
    public:
      using ViewAdvisor<View>::view;
    protected:
      /// First range of support data structure
      const Range* _fst;
      /// Last range of support data structure
      const Range* _lst;
    public:
      /// \name Constructors
      //@{
      /// Initialise from parameters
      CTAdvisor(Space& home, Propagator& p, Council<CTAdvisor>& c,
                const TupleSet& ts, View x0, int i);
      /// Clone advisor \a a
      CTAdvisor(Space& home, CTAdvisor& a);
      //@}
      /// Adjust supports
      void adjust(void);
      /// Return first range of support data structure
      const Range* fst(void) const;
      /// Return lasst range of support data structure
      const Range* lst(void) const;
      /// Dispose advisor
      void dispose(Space& home, Council<CTAdvisor>& c);
    };
    /// \name Status management
    //@{ 
    /// Type of status
    enum StatusType {
      SINGLE      = 0, ///< A single view has been touched
      MULTIPLE    = 1, ///< Multiple view have been touched
      NONE        = 2, ///< No view has been touched
      PROPAGATING = 3  ///< The propagator is currently running
    };
    /// Status management
    class Status {
    protected:
      /// A tagged pointer for storing the status
      ptrdiff_t s;
    public:
      /// Initialize with type \a t (either NONE or SEVERAL)
      Status(StatusType t);
      /// Copy constructor
      Status(const Status& s);
      /// Return status type
      StatusType type(void) const;
      /// Check whether status is single and equal to \a a
      bool single(CTAdvisor& a) const;
      /// Set status to SINGLE or MULTIPLE depending on \a a
      void touched(CTAdvisor& a);
      /// Set status to NONE
      void none(void);
      /// Set status to PROPAGATING
      void propagating(void);
    };
    //@}
    /// \name Support iterators
    //@{
    /// Iterator over valid supports
    class ValidSupports {
    protected:
      /// Number of words
      const unsigned int n_words;
      /// Maximal value
      int max;
      /// Range iterator
      ViewRanges<View> xr;
      /// Support iterator
      const Range* sr;
      /// The value
      int n;
      /// The value's support
      const BitSetData* s;
    public:
      /// Initialize from initialized propagator
      ValidSupports(const Compact<View>& p, CTAdvisor& a);
      /// Initialize during initialization
      ValidSupports(const TupleSet& ts, int i, View x);
      /// Move to next supports
      void operator ++(void);
      /// Whether there are still supports left
      bool operator ()(void) const;
      /// Return supports
      const BitSetData* supports(void) const;
      /// Return supported value
      int val(void) const;
    };
    /// Iterator over lost supports
    class LostSupports {
    protected:
      /// Number of words
      const unsigned int n_words;
      /// Range information
      const Range* r;
      /// Low value
      int l;
      /// High value
      int h;
      /// The lost value's support
      const BitSetData* s;
    public:
      /// Initialize iterator for values between \a l and \a h
      LostSupports(const Compact<View>& p, CTAdvisor& a,
                   int l, int h);
      /// Move iterator to next value
      void operator ++(void);
      /// Whether iterator is done
      bool operator ()(void) const;
      /// Provide access to corresponding supports
      const BitSetData* supports(void) const;
    };
    //@}
  protected:
    /// Number of unassigned views
    int unassigned;
    /// Number of words in supports
    const unsigned int n_words;
    /// Propagator status
    Status status;
    /// The tuple set
    TupleSet ts;
    /// The advisor council
    Council<CTAdvisor> c;
    /// Constructor for cloning \a p
    Compact(Space& home, Compact& p);
    /// Constructor for posting
    Compact(Home home, ViewArray<View>& x, const TupleSet& ts);
    /// Find range for \a n
    const Range* range(CTAdvisor& a, int n);
    /// Return supports for value \a n
    const BitSetData* supports(CTAdvisor& a, int n);
  public:
    /// Delete propagator and return its size
    size_t dispose(Space& home);
  };

  /**
   * \brief Domain consistent extensional propagator
   *
   * This propagator implements the compact-table propagation
   * algorithm based on:
   *   J. Demeulenaere et. al., Compact-Table: Efficiently
   *   filtering table constraints with reversible sparse
   *   bit-sets, CP 2016.
   *   Pages 207-223, LNCS, Springer, 2016.
   *
   * Requires \code #include <gecode/int/extensional.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View, class Table>
  class CompactTable : public Compact<View> {    
  public:
    typedef typename Compact<View>::ValidSupports ValidSupports;
    typedef typename Compact<View>::Range Range;
    typedef typename Compact<View>::CTAdvisor CTAdvisor;
    typedef typename Compact<View>::StatusType StatusType;
    typedef typename Compact<View>::Status Status;
    typedef typename Compact<View>::LostSupports LostSupports;

    using Compact<View>::supports;
    using Compact<View>::unassigned;
    using Compact<View>::status;
    using Compact<View>::c;
    using Compact<View>::ts;

    /// Current table
    Table table;
    /// Constructor for cloning \a p
    template<class TableProp>
    CompactTable(Space& home, TableProp& p);
    /// Constructor for posting
    CompactTable(Home home, ViewArray<View>& x, const TupleSet& ts);
  public:
    /// Cost function
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Post propagator for views \a x and table \a t
    static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& ts);
    /// Delete propagator and return its size
    size_t dispose(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
  };

  /// Post function for compact table propagator
  template<class View>
  ExecStatus postcompact(Home home, ViewArray<View>& x, const TupleSet& ts);

}}}

#include <gecode/int/extensional/compact.hpp>

#endif

// STATISTICS: int-prop
