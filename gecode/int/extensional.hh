/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Zayenz Lagerkvist, 2007
 *     Christian Schulte, 2004
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_INT_EXTENSIONAL_HH
#define GECODE_INT_EXTENSIONAL_HH

#include <gecode/int.hh>

#include <gecode/int/rel.hh>

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */

namespace Gecode { namespace Int { namespace Extensional {

  /// Compressed tuple-word support list
  class CompressedSupport {
  protected:
    /// First support word
    const TupleSet::CSupportWord* b;
    /// One past last support word
    const TupleSet::CSupportWord* e;
  public:
    /// Initialize as empty support list
    CompressedSupport(void);
    /// Initialize from support word range
    CompressedSupport(const TupleSet::CSupportWord* b0,
                      const TupleSet::CSupportWord* e0);
    /// Return first support word
    const TupleSet::CSupportWord* begin(void) const;
    /// Return one past last support word
    const TupleSet::CSupportWord* end(void) const;
    /// Whether support list is empty
    bool empty(void) const;
  };

  /**
   * \brief Domain consistent layered graph (regular) propagator
   *
   * The algorithm for the regular propagator is based on:
   *   Gilles Pesant, A Regular Language Membership Constraint
   *   for Finite Sequences of Variables, CP 2004.
   *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
   *
   * The propagator is not capable of dealing with multiple occurrences
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
    /// Number of active words
    IndexType _active_words;
    /// Number of addressable word slots
    IndexType _word_capacity;
    /// Original word index for each active word position
    IndexType* _word_index;
    /// Active word data
    BitSetData* _word_bits;
    /// Reverse map from word index to active position+1 (optional)
    IndexType* _active_position;
    /// Replace active word \a active_pos, dropping it if \a word is zero
    void replace_and_decrease(IndexType active_pos, BitSetData word);
  public:
    /// Initialize bit set for a number of words \a n
    BitSet(Space& home, unsigned int n, bool indexed=false);
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
    /// Get the number of active words
    unsigned int limit(void) const;
    /// Check whether the set is empty
    bool empty(void) const;
    /// Make the set empty
    void flush(void);
    /// Return the highest active index
    unsigned int width(void) const;
    /// Clear all active words in \a mask
    void clear_mask(BitSetData* mask) const;
    /// Add \a support to \a mask
    void add_to_mask(const BitSetData* support, BitSetData* mask) const;
    /// Add compressed support list to \a mask
    void add_to_mask(const CompressedSupport& support, BitSetData* mask) const;
    /// Intersect with \a mask, sparse mask if \a sparse is true
    template<bool sparse>
    void intersect_with_mask(const BitSetData* mask);
    /// Intersect with compressed support list
    void intersect_with_mask(const CompressedSupport& support);
    /// Intersect with the "or" of \a a and \a b
    void intersect_with_masks(const BitSetData* a, const BitSetData* b);
    /// Intersect with the "or" of two compressed support lists
    void intersect_with_masks(const CompressedSupport& a,
                              const CompressedSupport& b);
    /// Check if \a has a non-empty intersection with the set
    bool intersects(const BitSetData* mask) const;
    /// Check if compressed support list intersects with the set
    bool intersects(const CompressedSupport& support) const;
    /// Perform "nand" with \a mask
    void nand_with_mask(const BitSetData* mask);
    /// Perform "nand" with compressed support list
    void nand_with_mask(const CompressedSupport& support);
    /// Return the number of ones
    unsigned long long int ones(void) const;
    /// Return the number of ones after intersection with \a mask
    unsigned long long int ones(const BitSetData* mask) const;
    /// Return the number of ones after intersection with compressed support list
    unsigned long long int ones(const CompressedSupport& support) const;
    /// Return an upper bound on the number of bits
    unsigned long long int bits(void) const;
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
    BitSetData _bits[_size];
  public:
    /// Initialize sparse bit set for a number of words \a n
    TinyBitSet(Space& home, unsigned int n, bool indexed=false);
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
    /// Make the set empty
    void flush(void);
    /// Return the highest active index
    unsigned int width(void) const;
    /// Clear the first \a limit words in \a mask
    void clear_mask(BitSetData* mask);
    /// Add \b to \a mask
    void add_to_mask(const BitSetData* b, BitSetData* mask) const;
    /// Add compressed support list to \a mask
    void add_to_mask(const CompressedSupport& s, BitSetData* mask) const;
    /// Intersect with \a mask, sparse mask if \a sparse is true
    template<bool sparse>
    void intersect_with_mask(const BitSetData* mask);
    /// Intersect with compressed support list
    void intersect_with_mask(const CompressedSupport& s);
    /// Intersect with the "or" of \a and \a b
    void intersect_with_masks(const BitSetData* a, const BitSetData* b);
    /// Intersect with the "or" of two compressed support lists
    void intersect_with_masks(const CompressedSupport& a,
                              const CompressedSupport& b);
    /// Check if \a has a non-empty intersection with the set
    bool intersects(const BitSetData* b);
    /// Check if compressed support list intersects with the set
    bool intersects(const CompressedSupport& s);
    /// Perform "nand" with \a b
    void nand_with_mask(const BitSetData* b);
    /// Perform "nand" with compressed support list
    void nand_with_mask(const CompressedSupport& s);
    /// Perform "nand" with and the "or" of \a a and \a b
    void nand_with_masks(const BitSetData* a, const BitSetData* b);
    /// Return the number of ones
    unsigned long long int ones(void) const;
    /// Return the number of ones after intersection with \a b
    unsigned long long int ones(const BitSetData* b) const;
    /// Return the number of ones after intersection with compressed support list
    unsigned long long int ones(const CompressedSupport& s) const;
    /// Return an upper bound on the number of bits
    unsigned long long int bits(void) const;
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

  /// Optional variable index stored by a compact-table advisor
  template<bool indexed>
  class CompactAdvisorIndex;

  /// Compact-table advisor without a variable index
  template<>
  class CompactAdvisorIndex<false> {
  protected:
    /// Initialize without storing \a i
    CompactAdvisorIndex(int i);
    /// Return a dummy index
    int index(void) const;
  };

  /// Compact-table advisor with a variable index
  template<>
  class CompactAdvisorIndex<true> {
  protected:
    /// Variable index
    int _index;
    /// Initialize with index \a i
    CompactAdvisorIndex(int i);
    /// Return the variable index
    int index(void) const;
  };

  /// Advisor shared by compact-table support representations
  template<class View, bool pos, bool indexed>
  class CompactAdvisor : public ViewAdvisor<View>,
                         private CompactAdvisorIndex<indexed> {
  protected:
    /// Range type for supports
    typedef TupleSet::Range Range;
    /// First range of support data structure
    const Range* _fst;
    /// Last range of support data structure
    const Range* _lst;
  public:
    using ViewAdvisor<View>::view;
    /// Initialize from parameters
    CompactAdvisor(Space& home, Propagator& p,
                   Council<CompactAdvisor>& c, const TupleSet& ts,
                   View x, int i);
    /// Clone advisor \a a
    CompactAdvisor(Space& home, CompactAdvisor& a);
    /// Adjust supports to the current view bounds
    void adjust(void);
    /// Return the variable index
    int index(void) const;
    /// Return first range of support data structure
    const Range* fst(void) const;
    /// Return last range of support data structure
    const Range* lst(void) const;
    /// Dispose advisor
    void dispose(Space& home, Council<CompactAdvisor>& c);
  };

  /// Touched-advisor status shared by positive compact-table propagators
  template<class Advisor>
  class CompactStatus {
  protected:
    /// A tagged advisor pointer or a status value
    ptrdiff_t s;
  public:
    /// Type of status
    enum StatusType {
      SINGLE      = 0, ///< A single view has been touched
      MULTIPLE    = 1, ///< Multiple views have been touched
      NONE        = 2, ///< No view has been touched
      PROPAGATING = 3  ///< The propagator is currently running
    };
    /// Initialize with status \a t
    CompactStatus(StatusType t);
    /// Copy constructor
    CompactStatus(const CompactStatus& status);
    /// Return status type
    StatusType type(void) const;
    /// Test whether only advisor \a a was touched
    bool single(Advisor& a) const;
    /// Record that advisor \a a was touched
    void touched(Advisor& a);
    /// Record that no advisor has been touched
    void none(void);
    /// Record that propagation is in progress
    void propagating(void);
  };

  /// Shared implementation of positive compact-table propagation
  template<class Actor>
  class PosCompactAlgorithm;
  /// Shared implementation of negative compact-table propagation
  template<class Actor>
  class NegCompactAlgorithm;
  /// Shared implementation of reified compact-table propagation
  template<class Actor>
  class ReCompactAlgorithm;

  /// Base class for compact table propagator
  template<class View, bool pos>
  class Compact : public Propagator {
  protected:
    /// Range type for supports
    typedef TupleSet::Range Range;
    /// Advisor for updating current table
    typedef CompactAdvisor<View,pos,false> CTAdvisor;
    //@}
    /// \name Support iterators
    //@{
    /// Iterator over valid supports
    class ValidSupports {
    protected:
      /// Number of words
      const unsigned int n_words;
      /// Maximal value
      int max_value;
      /// Range iterator
      ViewRanges<View> view_ranges;
      /// Support iterator
      const Range* support_range;
      /// The last range
      const Range* last_support_range;
      /// The value
      int value;
      /// The value's support
      const BitSetData* support_words;
      /// Find a new value (only for negative case)
      void find(void);
    public:
      /// Initialize from initialized propagator
      ValidSupports(const Compact<View,pos>& p, CTAdvisor& a);
      /// Initialize during initialization
      ValidSupports(const TupleSet& ts, int i, View x);
      /// Move to next supports
      void operator ++(void);
      /// Whether there are still supports left
      bool operator ()(void) const;
      /// Return support representation
      const BitSetData* support(void) const;
      /// Return supported value
      int val(void) const;
    };
    /// Iterator over lost supports
    class LostSupports {
    protected:
      /// Number of words
      const unsigned int n_words;
      /// Range information
      const Range* support_range;
      /// Last range
      const Range* last_support_range;
      /// Low value
      int value;
      /// High value
      int last_value;
      /// The lost value's support
      const BitSetData* support_words;
    public:
      /// Initialize iterator for values between \a first_value and \a last_value
      LostSupports(const Compact<View,pos>& p, CTAdvisor& a,
                   int first_value, int last_value);
      /// Move iterator to next value
      void operator ++(void);
      /// Whether iterator is done
      bool operator ()(void) const;
      /// Return support representation
      const BitSetData* support(void) const;
    };
    //@}
    /// \name Testing the number of unassigned variables
    //@{
    /// Whether all variables are assigned
    bool all(void) const;
    /// Whether at most one variable is unassigned
    bool atmostone(void) const;
    //@}
  protected:
    /// Number of words in supports
    const unsigned int n_words;
    /// The tuple set
    TupleSet ts;
    /// The advisor council
    Council<CTAdvisor> c;
    /// Constructor for cloning \a p
    Compact(Space& home, Compact& p);
    /// Constructor for posting
    Compact(Home home, const TupleSet& ts);
    /// Setup the actual table
    template<class Table>
    void setup(Space& home, Table& table, ViewArray<View>& x);
    /// Check whether the table covers the whole Cartedion product
    template<class Table>
    bool full(const Table& table) const;
    /// Find range for \a n
    const Range* range(CTAdvisor& a, int n);
    /// Return supports for value \a n
    const BitSetData* supports(CTAdvisor& a, int n);
  public:
    /// Cost function
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    size_t dispose(Space& home);
  };

  /**
   * \brief Domain consistent positive extensional propagator
   *
   * This propagator implements the compact-table propagation
   * algorithm based on:
   *   J. Demeulenaere et. al., Compact-Table: Efficiently
   *   filtering table constraints with reversible sparse
   *   bit-sets, CP 2016.
   *
   * Requires \code #include <gecode/int/extensional.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View, class Table>
  class PosCompact : public Compact<View,true> {
    template<class Actor> friend class PosCompactAlgorithm;
  public:
    typedef View ViewType;
    typedef typename Compact<View,true>::ValidSupports ValidSupports;
    typedef typename Compact<View,true>::Range Range;
    typedef typename Compact<View,true>::CTAdvisor CTAdvisor;
    typedef typename Compact<View,true>::LostSupports LostSupports;

    using Compact<View,true>::setup;
    using Compact<View,true>::supports;
    using Compact<View,true>::all;
    using Compact<View,true>::atmostone;
    using Compact<View,true>::c;
    using Compact<View,true>::ts;

    /// Status management
    typedef CompactStatus<CTAdvisor> Status;
    /// Status type
    typedef typename Status::StatusType StatusType;
    /// Propagator status
    Status status;
    /// Current table
    Table table;
    /// Check whether the table is empty
    bool empty(void) const;
    /// Constructor for cloning \a p
    template<class TableProp>
    PosCompact(Space& home, TableProp& p);
    /// Constructor for posting
    PosCompact(Home home, ViewArray<View>& x, const TupleSet& ts);
  public:
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

  /// Post function for positive compact table propagator
  template<class View>
  ExecStatus postposcompact(Home home, ViewArray<View>& x, const TupleSet& ts);
  /// Post function for positive compact table with compressed supports
  template<class View>
  ExecStatus postposcompact_compressed(Home home, ViewArray<View>& x,
                                       const TupleSet& ts);

  /**
   * \brief Domain consistent negative extensional propagator
   *
   * This propagator implements the compact-table propagation
   * algorithm based on:
   *   J. Demeulenaere et. al., Compact-Table: Efficiently
   *   filtering table constraints with reversible sparse
   *   bit-sets, CP 2016.
   * and (negative tables) on:
   *   H. Verhaeghe et al., Extending Compact-Table to
   *   Negative and Short Tables. AAAI 2017.
   *
   * Requires \code #include <gecode/int/extensional.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View, class Table>
  class NegCompact : public Compact<View,false> {
    template<class Actor> friend class NegCompactAlgorithm;
  public:
    typedef View ViewType;
    typedef typename Compact<View,false>::ValidSupports ValidSupports;
    typedef typename Compact<View,false>::Range Range;
    typedef typename Compact<View,false>::CTAdvisor CTAdvisor;

    using Compact<View,false>::setup;
    using Compact<View,false>::full;
    using Compact<View,false>::supports;
    using Compact<View,false>::atmostone;
    using Compact<View,false>::c;
    using Compact<View,false>::ts;

    /// Current table
    Table table;
    /// Constructor for cloning \a p
    template<class TableProp>
    NegCompact(Space& home, TableProp& p);
    /// Constructor for posting
    NegCompact(Home home, ViewArray<View>& x, const TupleSet& ts);
  public:
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
  ExecStatus postnegcompact(Home home, ViewArray<View>& x, const TupleSet& ts);
  /// Post function for negative compact table with compressed supports
  template<class View>
  ExecStatus postnegcompact_compressed(Home home, ViewArray<View>& x,
                                       const TupleSet& ts);


  /// Domain consistent reified extensional propagator
  template<class View, class Table, class CtrlView, ReifyMode rm>
  class ReCompact : public Compact<View,false> {
    template<class Actor> friend class ReCompactAlgorithm;
  public:
    typedef View ViewType;
    static constexpr ReifyMode mode = rm;
    typedef typename Compact<View,false>::ValidSupports ValidSupports;
    typedef typename Compact<View,false>::Range Range;
    typedef typename Compact<View,false>::CTAdvisor CTAdvisor;

    using Compact<View,false>::setup;
    using Compact<View,false>::full;
    using Compact<View,false>::supports;
    using Compact<View,false>::c;
    using Compact<View,false>::ts;

    /// Current table
    Table table;
    /// Boolean control view
    CtrlView b;
    /// The views (for rewriting)
    ViewArray<View> y;
    /// Constructor for cloning \a p
    template<class TableProp>
    ReCompact(Space& home, TableProp& p);
    /// Constructor for posting
    ReCompact(Home home, ViewArray<View>& x, const TupleSet& ts, CtrlView b);
    static ExecStatus post_pos(Home home, ViewArray<View>& x,
                               const TupleSet& ts);
    static ExecStatus post_neg(Home home, ViewArray<View>& x,
                               const TupleSet& ts);
  public:
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Post propagator for views \a x and table \a t
    static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& ts,
                           CtrlView b);
    /// Delete propagator and return its size
    size_t dispose(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
  };

  /// Post function for compact table propagator
  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus postrecompact(Home home, ViewArray<View>& x, const TupleSet& ts,
                           CtrlView b);
  /// Post function for reified compact table with compressed supports
  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus postrecompact_compressed(Home home, ViewArray<View>& x,
                                      const TupleSet& ts, CtrlView b);

}}}

#include <gecode/int/extensional/compact.hpp>

#endif

// STATISTICS: int-prop
