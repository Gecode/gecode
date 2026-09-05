/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#ifndef GECODE_WORD_HH
#define GECODE_WORD_HH

#include <cstdint>
#include <iostream>
#include <vector>
#include <gecode/int.hh>

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))
# ifdef GECODE_BUILD_WORD
#  define GECODE_WORD_EXPORT __declspec(dllexport)
# else
#  define GECODE_WORD_EXPORT __declspec(dllimport)
# endif
#else
# ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#  define GECODE_WORD_EXPORT __attribute__((visibility("default")))
# else
#  define GECODE_WORD_EXPORT
# endif
#endif

#ifndef GECODE_BUILD_WORD
# define GECODE_LIBRARY_NAME "Word"
# include <gecode/support/auto-link.hpp>
#endif

namespace Gecode {
  /** \brief Unsigned storage type for word values
   *  \ingroup TaskModelWordVars
   */
  typedef std::uint64_t WordValue;

  /// Construction-time representation for a word domain
  enum WordDomainType {
    WDT_CUBE,     ///< Cube of independently known bits
    WDT_UNSIGNED, ///< Cube intersected with an unsigned numeric interval
    WDT_SIGNED    ///< Cube intersected with a signed numeric interval
  };
}

#include <gecode/word/exception.hpp>
#include <gecode/word/var-imp.hpp>

namespace Gecode {
  namespace Word { class WordView; class WordTraceView; }

  /**
   * \defgroup TaskModelWord Word-vector constraints
   *
   * A word variable has an immutable width from 1 through 64. Every domain
   * contains a cube represented by a known-one lower mask and a may-be-one
   * upper mask. WDT_UNSIGNED and WDT_SIGNED optionally intersect that cube
   * with one immutable-interpretation numeric interval; WDT_CUBE retains the
   * original cube alone. Constants therefore always carry an explicit width.
   * Arithmetic is fixed-width and wraps modulo \f$2^{width}\f$. Operations
   * with policy-dependent edge cases default to WordSemantics::WS_SMTLIB.
   *
   * Bounded dispatch is selective. A posting uses a numeric actor only for a
   * proved compatible interpretation and regime. Cube, mixed-kind, wrapping,
   * or otherwise unsupported cases (including aliases not handled by the
   * selected bounded actor) deliberately use the ordinary cube actor for the
   * complete relation; its tells still synchronize any bounded variables. A
   * bounded domain does not change operation semantics and does not imply
   * domain-consistent propagation. Bounded actors that combine inexpensive
   * numeric rules with a width-dependent cube algorithm handle bound-only
   * events as a low-cost stage. They batch local cube/interval synchronization
   * and reschedule the cube stage at its linear cost only when synchronization
   * actually fixes bits. Numeric-only and already-cheap actors remain
   * single-stage.
   *
   * WordVar provides the variable and mask interface. WordRelType and
   * WordOpType select direct relation and logical postings. Models branch with
   * WordVarBranch and WordValBranch, and can observe changes with WordTracer.
   * MiniModel users can compose the same operations with WordExpr; expression
   * nodes lower through the direct posting API and retain their width and
   * WordSemantics.
   *
   * \par Implementation and tested propagation inventory
   *
   * | Public operation | Implementation | Tested property |
   * | --- | --- | --- |
   * | `dom`, WordVar queries, and bounded constructors | Native atomic cube update, optionally synchronized with one unsigned or signed interval | Exact represented-domain membership, canonical endpoints, assignment, and failure |
   * | `channel` | Direct word-bit/BoolVar cube actor, plus a mixed WordVar/IntVar numeric bounds channel with explicit unsigned or signed interpretation | Bit consistency, exact assigned numeric equality, bounds consistency, bounded-domain synchronization, cloning, and recomputation |
   * | `reduce_and`, `reduce_or`, `reduce_xor` | Direct Word/Bool cube reduction actors for every domain kind | Assigned semantics, decisive bits, parity completion, and lifecycle |
   * | `element` | Homogeneous bounded table/result views without result aliases use an unsigned or signed interval actor; cube, mixed-kind, and aliased-result cases use the direct mixed Int/Word cube actor | Index support pruning, supported-result cube hull, and sound selected interval bounds |
   * | `popcount`, `count_leading_zeros`, `count_trailing_zeros` | Direct mixed Word/Int cube-count actors for every domain kind | Population bounds/extrema and zero-prefix propagation |
   * | `rel` with `WRT_EQ`, `WRT_NQ` | Compatible bounded variable and constant views use transactional cube-and-interval equality; other cases use direct cube equality/disequality; reified disequality rewrites through equality | Exact equality intersection, sound disequality exclusion, and all reification modes |
   * | `rel` with unsigned and signed order types | Matching numeric domains use direct interval actors; cube, mixed, and opposite-kind cases use the MSB-first actors; greater and reified strict relations use the established operand/control rewrites | Assigned semantics and sound numeric-bound or word-level bit pruning |
   * | `complement`; binary and n-ary logical `rel` with all WordOpType values | Cube actors for every domain kind: distinct-view binary OR/XOR use direct native actors, aliases and other binary forms use generic Table, and n-ary forms use primitive globals plus optional complement | Per-bit consistency, bounded-domain synchronization, and assigned semantics |
   * | Boolean and word-mask `ite` | Pairwise-distinct homogeneous bounded Boolean branches/result use an unsigned or signed interval actor; aliases, mixed kinds, and the per-bit word-mask form use cube actors | Sound partial propagation and selected interval hulls; mask form is bit consistent |
   * | `extract`, `concat`, `repeat`, `zero_extend`, `sign_extend` | Direct fixed masked-copy cube actors for every domain kind | Bit consistency for copied bits and groups |
   * | Fixed `shift_left`, `logical_shift_right`, `arithmetic_shift_right`, `rotate_left`, `rotate_right` | Distinct unsigned non-wrapping or overshift left shift can use a numeric actor; every other case uses the direct fixed cube actor | Sound interval bounds where selected; bit consistency and boundary amounts everywhere |
   * | Variable `shift_left`, `logical_shift_right`, `arithmetic_shift_right` | Pairwise-distinct non-wrapping unsigned left shift adds a numeric actor to the exact cube-hull actor; all other forms use the cube actor alone. The amount is interpreted as unsigned | Assigned semantics, sound partial-amount cube hulls, and sound selected interval bounds |
   * | Binary `add`, `sub`, `mult` | Same-kind signed or unsigned non-wrapping regimes use transactional numeric-plus-cube actors; mixed or wrapping regimes use the native cube actors | Assigned modular semantics and sound partial/inverse propagation |
   * | `neg`; carry and borrow `add`/`sub` | Signed non-singular negation and classified unsigned carry/borrow can use bounded actors; all other regimes use cube actors | Assigned modular semantics, flag rows, aliases, and lifecycle |
   * | N-ary `add` | Same-kind unsigned or signed inputs/result use a bounded actor when their complete interval sum cannot wrap; other regimes use the native cube carry-support actor. Assigned constants are folded at posting | Assigned modular semantics and sound partial/inverse cube and interval support |
   * | `product_mod` | Homogeneous unsigned inputs/result use a bounded actor when the input intervals prove that the mathematical product fits the Word width; other regimes use the direct mixed Word/Int cube actor. Reification uses the cube control actor and dispatches the asserted relation through the same path | Unconditionally positive modulus, mathematical product before modulus reduction, sound result cube/interval pruning, all reification modes, and lifecycle |
   * | `gcd`, `signed_gcd` | Compatible unsigned inputs/result, or signed inputs with an unsigned result, use bounded mathematical-GCD actors; other direct cases use cube actors. Reified forms use cube control actors and dispatch an asserted relation through the corresponding direct path | `gcd(0,0)=0`, signed magnitude, sound cube/interval pruning, all reification modes, and lifecycle |
   * | `divides`, `signed_divides` | Homogeneous unsigned or signed operands use bounded mathematical-divisibility actors; mixed-kind and cube operands use cube actors. The public relation is reified through a cube control actor | Zero-divisor semantics, sound cube/interval pruning, all reification modes, and lifecycle |
   * | Arithmetic `overflow` predicates | Compositions over arithmetic, bit channels, and reified relations; compatible intermediates preserve their kind | Assigned semantics, SMT-LIB signed overflow cases, and lifecycle |
   * | Unsigned `div`, `mod`, combined `divmod` | Homogeneous unsigned operands with a settled divisor-zero regime use transactional bounded actors; other cases use the native cube actors; combined quotient/remainder uses one shared actor | Exact SMT-LIB zero-divisor rows and sound partial/inverse propagation |
   * | `signed_div`, `signed_rem`, `signed_mod` | Homogeneous signed operands with a settled divisor sign/zero regime use transactional bounded actors; other cases use native signed cube actors | Exact SMT-LIB zero-divisor and minimum/-1 rows, plus sound sign and partial propagation |
   * | `branch`, `assign` and their selectors | Native unknown-bit choices plus admitted ranked split/minimum/median/maximum choices for bounded domains | Archive, no-good, clone, and recomputation lifecycle |
   * | `trace`, WordTraceDelta and word printing | Standard integration reports cube bits, immutable kind, and ranked endpoints | Bit/bound/combined deltas and trace lifecycle |
   * | `distinct` | `IPL_VAL` (the default) and `IPL_DOM` post native pairwise Word disequalities; explicit `IPL_BND` uses one Hall-interval actor for homogeneous unsigned or signed bounded arrays and otherwise falls back to value consistency | Value exclusion, optional bounds consistency over one ranked interval, width-64 endpoints, aliases, cloning, and recomputation |
   * | WordExpr logical, relation, conditional, structural, and arithmetic expressions | Ref-counted MiniModel DAG lowered through the corresponding direct API. Ordinary syntax is cube-default; `WordExpr::post`, `word_rel`, `bit`, reductions, and overflow accept an explicit WordDomainType policy, and incompatible intermediate operations fall back to cube temporaries | Direct-posting parity, explicit-policy preservation, width/policy rejection, copies, clones, and recomputation |
   */
  /**
   * \defgroup TaskModelWordVars Word-vector variables and arrays
   * \ingroup TaskModelWord
   */
  /**
   * \brief Fixed-width word variable
   *
   * The domain is represented by a known-one lower mask and a may-be-one
   * upper mask. Widths from 1 through 64 are supported.
   * \ingroup TaskModelWordVars
   */
  class WordVar : public VarImpVar<Word::WordVarImp> {
    friend class WordVarArray;
    friend class WordVarArgs;
  private:
    using VarImpVar<Word::WordVarImp>::x;
    void _init(Space& home, unsigned int width, WordValue lo, WordValue hi);
    void _init(Space& home, unsigned int width, WordValue lo, WordValue hi,
               WordDomainType domain_type, WordValue minimum,
               WordValue maximum);
  public:
    /// Construct an uninitialized variable handle
    WordVar(void);
    /// Copy variable handle \a y
    WordVar(const WordVar& y);
    /// Construct a variable handle from view \a y
    WordVar(const Word::WordView& y);
    /** \brief Construct an unconstrained word of the given \a width
     *
     * Throws Word::OutOfLimits if \a width is not between 1 and 64.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width);
    /** \brief Construct a word with cube bounds \a lo and \a hi
     *
     * Throws Word::OutOfLimits if \a width is not between 1 and 64 or a
     * mask has bits outside \a width. Throws Word::VariableEmptyDomain if
     * the masks have no common value.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordValue lo, WordValue hi);
    /** \brief Construct a word using the full range of \a domain_type
     *
     * Throws Word::OutOfLimits if \a width is not between 1 and 64 or
     * \a domain_type is not WDT_CUBE, WDT_UNSIGNED, or WDT_SIGNED.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordDomainType domain_type);
    /**
     * \brief Construct a bounded word with encoded endpoint bit patterns
     *
     * For signed words, \a minimum and \a maximum use the word's
     * two's-complement bit patterns, not internal signed-order ranks.
     * Throws Word::OutOfLimits if \a width, \a domain_type, or an endpoint
     * encoding is invalid. Throws Word::VariableEmptyDomain if the encoded
     * interval is empty.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordDomainType domain_type,
                              WordValue minimum, WordValue maximum);
    /**
     * \brief Construct a bounded word with cube masks and encoded endpoints
     *
     * For signed words, \a minimum and \a maximum use the word's
     * two's-complement bit patterns, not internal signed-order ranks.
     * Throws Word::OutOfLimits if \a width, \a domain_type, a mask, or an
     * endpoint encoding is invalid. Throws Word::VariableEmptyDomain if the
     * cube and encoded interval have an empty intersection.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordValue lo, WordValue hi,
                              WordDomainType domain_type,
                              WordValue minimum, WordValue maximum);
    /// Return the immutable width
    unsigned int width(void) const;
    /// Return the mask containing all significant bits
    WordValue mask(void) const;
    /// Return the known-one lower mask
    WordValue lo(void) const;
    /// Return the may-be-one upper mask
    WordValue hi(void) const;
    /// Return the immutable domain representation
    WordDomainType domain_type(void) const;
    /// Test whether the word has a numeric interval
    bool bounded(void) const;
    /**
     * \brief Return the canonical minimum endpoint bit pattern
     *
     * For signed words, the result is the two's-complement encoding of the
     * minimum value, not its internal signed-order rank.
     * Throws Word::BoundsOfCubeVar if this is a cube-only word.
     */
    WordValue minimum(void) const;
    /**
     * \brief Return the canonical maximum endpoint bit pattern
     *
     * For signed words, the result is the two's-complement encoding of the
     * maximum value, not its internal signed-order rank.
     * Throws Word::BoundsOfCubeVar if this is a cube-only word.
     */
    WordValue maximum(void) const;
    /// Return the mask of unknown bits
    WordValue unknown(void) const;
    /// Return the number of unknown bits
    unsigned int unknown_size(void) const;
    /// Test whether the word is assigned
    bool assigned(void) const;
    /// Test whether concrete \a value belongs to the represented domain
    bool in(WordValue value) const;
    /// Return the assigned value; throws Word::ValOfUnassignedVar otherwise
    WordValue val(void) const;
    WordVar& operator=(const WordVar&) = default;
  };
}

#include <gecode/word/var/word.hpp>
#include <gecode/word/view.hpp>
#include <gecode/word/trace/trace-view.hpp>

namespace Gecode {
  forceinline WordVar::WordVar(const Word::WordView& y)
    : VarImpVar<Word::WordVarImp>(y.varimp()) {}
}

#include <gecode/word/array-traits.hpp>

namespace Gecode {
  /// Passing word variables
  class WordVarArgs : public VarArgArray<WordVar> {
  public:
    WordVarArgs(void);
    explicit WordVarArgs(int n);
    WordVarArgs(const WordVarArgs& a);
    WordVarArgs(const VarArray<WordVar>& a);
    WordVarArgs(const std::vector<WordVar>& a);
    WordVarArgs(std::initializer_list<WordVar> a);
    template<class InputIterator> WordVarArgs(InputIterator first, InputIterator last);
    /** \brief Construct \a n cube words
     *
     * Throws Word::OutOfLimits for an invalid width or mask, and
     * Word::VariableEmptyDomain if the masks have no common value.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValue lo, WordValue hi);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for an invalid width or domain kind.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type);
    /** \brief Construct bounded words with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for an invalid width, domain kind, or endpoint
     * encoding, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum);
    /** \brief Construct bounded cubes with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for an invalid width, mask, domain kind, or endpoint
     * encoding, and Word::VariableEmptyDomain if the cube and interval have
     * an empty intersection.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValue lo, WordValue hi,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum);
    WordVarArgs& operator=(const WordVarArgs&) = default;
  };
  /// Word variable array
  class WordVarArray : public VarArray<WordVar> {
  public:
    WordVarArray(void);
    WordVarArray(Space& home, int n);
    WordVarArray(const WordVarArray& a);
    WordVarArray(Space& home, const WordVarArgs& a);
    /** \brief Construct \a n cube words
     *
     * Throws Word::OutOfLimits for an invalid width or mask, and
     * Word::VariableEmptyDomain if the masks have no common value.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValue lo, WordValue hi);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for an invalid width or domain kind.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type);
    /** \brief Construct bounded words with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for an invalid width, domain kind, or endpoint
     * encoding, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type,
                                   WordValue minimum, WordValue maximum);
    /** \brief Construct bounded cubes with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for an invalid width, mask, domain kind, or endpoint
     * encoding, and Word::VariableEmptyDomain if the cube and interval have
     * an empty intersection.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValue lo, WordValue hi,
                                   WordDomainType domain_type,
                                   WordValue minimum, WordValue maximum);
    WordVarArray& operator=(const WordVarArray&) = default;
  };

  /// Restrict \a x to the word cube described by \a lo and \a hi
  GECODE_WORD_EXPORT void dom(Home home, WordVar x,
                              WordValue lo, WordValue hi);
  /// Assign \a x to \a value
  GECODE_WORD_EXPORT void dom(Home home, WordVar x, WordValue value);

  /**
   * \defgroup TaskModelWordElement Element constraints
   * \ingroup TaskModelWord
   */
  //@{
  /**
   * \brief Post the element constraint \f$x_i=y\f$
   *
   * The index \a i is zero based. All words in \a x and \a y must have
   * the same width.
   */
  GECODE_WORD_EXPORT void element(Home home, const WordVarArgs& x,
                                  IntVar i, WordVar y);
  //@}

  /**
   * \defgroup TaskModelWordCount Bit-count constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Relate \a count to the number of one bits in \a x
  GECODE_WORD_EXPORT void popcount(Home home, WordVar x, IntVar count);
  /** \brief Relate \a count to the number of leading zero bits in \a x
   *
   * The count of the all-zero word is the width of \a x.
   */
  GECODE_WORD_EXPORT void count_leading_zeros(Home home, WordVar x,
                                              IntVar count);
  /** \brief Relate \a count to the number of trailing zero bits in \a x
   *
   * The count of the all-zero word is the width of \a x.
   */
  GECODE_WORD_EXPORT void count_trailing_zeros(Home home, WordVar x,
                                               IntVar count);
  //@}

  /**
   * \defgroup TaskModelWordChannel Channel constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Channel bit \a bit of \a x to Boolean variable \a b
  GECODE_WORD_EXPORT void channel(Home home, WordVar x,
                                  unsigned int bit, BoolVar b);
  /// Channel bit \a bit of \a x to Boolean constant \a value
  GECODE_WORD_EXPORT void channel(Home home, WordVar x,
                                  unsigned int bit, int value);
  /** \brief Bounds-channel word \a x to integer variable \a y
   *
   * The interpretation must be WDT_UNSIGNED or WDT_SIGNED. A cube word can
   * be channelled with either interpretation. A bounded word must use its
   * construction-time interpretation. The channel is exact for assigned
   * values and bounds consistent otherwise.
   */
  GECODE_WORD_EXPORT void channel(Home home, WordVar x, IntVar y,
                                  WordDomainType interpretation);
  /// Reduce all significant bits of \a x by conjunction into \a b
  GECODE_WORD_EXPORT void reduce_and(Home home, WordVar x, BoolVar b);
  /// Reduce all significant bits of \a x by disjunction into \a b
  GECODE_WORD_EXPORT void reduce_or(Home home, WordVar x, BoolVar b);
  /** \brief Reduce all significant bits of \a x by exclusive-or into \a b
   *
   * Parity can narrow a word cube only when at most one bit remains unknown.
   */
  GECODE_WORD_EXPORT void reduce_xor(Home home, WordVar x, BoolVar b);
  //@}

  /// Word relation type
  enum WordRelType {
    WRT_EQ, ///< Equality
    WRT_NQ, ///< Disequality
    WRT_ULQ, ///< Unsigned less than or equal
    WRT_ULE, ///< Unsigned less than
    WRT_UGQ, ///< Unsigned greater than or equal
    WRT_UGR, ///< Unsigned greater than
    WRT_SLQ, ///< Signed less than or equal
    WRT_SLE, ///< Signed less than
    WRT_SGQ, ///< Signed greater than or equal
    WRT_SGR  ///< Signed greater than
  };

  /// Word logical operation type
  enum WordOpType {
    WOT_AND,  ///< Bitwise conjunction
    WOT_OR,   ///< Bitwise disjunction
    WOT_XOR,  ///< Bitwise exclusive disjunction
    WOT_NAND, ///< Complement of bitwise conjunction
    WOT_NOR,  ///< Complement of bitwise disjunction
    WOT_XNOR  ///< Complement of bitwise exclusive disjunction
  };

  /** \brief Policy for semantics-dependent arithmetic edge cases
   *
   * The initial policy follows SMT-LIB: unsigned division by zero returns the
   * all-one word, unsigned remainder by zero returns the dividend, and signed
   * division, remainder, and modulus are total, including minimum/-1 overflow.
   * The policy is explicit so later alternatives need not change WordVar.
   */
  enum WordSemantics {
    WS_SMTLIB ///< SMT-LIB fixed-width word semantics
  };

  /// Word arithmetic overflow operation type
  enum WordOverflowType {
    WOF_NEG_SIGNED,  ///< Signed negation overflow (SMT-LIB bvnego)
    WOF_ADD_UNSIGNED,///< Unsigned addition overflow (SMT-LIB bvuaddo)
    WOF_ADD_SIGNED,  ///< Signed addition overflow (SMT-LIB bvsaddo)
    WOF_MULT_UNSIGNED,///< Unsigned multiplication overflow (SMT-LIB bvumulo)
    WOF_MULT_SIGNED, ///< Signed multiplication overflow (SMT-LIB bvsmulo)
    WOF_DIV_SIGNED   ///< Signed division overflow (QF_BV bvsdivo extension)
  };

  /**
   * \defgroup TaskModelWordRel Word relations
   * \ingroup TaskModelWord
   */
  //@{
  /// Post the relation \a wrt between \a x and \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y);
  /** \brief Post distinctness of all words in \a x
   *
   * Value consistency is the default and is implemented by native pairwise
   * Word disequalities. IPL_BND uses Hall-interval bounds propagation when
   * all variables have the same bounded interpretation; other cases fall
   * back to value consistency. IPL_DOM currently uses value consistency.
   * Throws Word::ArgumentSame for duplicate variables and
   * Word::WidthMismatch if the widths differ.
   */
  GECODE_WORD_EXPORT void distinct(Home home, const WordVarArgs& x,
                                   IntPropLevel ipl=IPL_VAL);
  /// Post the relation \a wrt between \a x and \a y, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y, Reify r);
  /// Post the relation \a wrt between \a x and an explicitly-sized constant
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              unsigned int width, WordValue value);
  /// Post the relation \a wrt to a constant, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              unsigned int width, WordValue value, Reify r);
  //@}

  /**
   * \defgroup TaskModelWordLogic Word logical constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Post bitwise complement \a y = ~\a x
  GECODE_WORD_EXPORT void complement(Home home, WordVar x, WordVar y);
  /// Post complement from an explicitly-sized constant input
  GECODE_WORD_EXPORT void complement(Home home, unsigned int width,
                                     WordValue value, WordVar y);
  /// Post complement of \a x equal to an explicitly-sized constant
  GECODE_WORD_EXPORT void complement(Home home, WordVar x,
                                     unsigned int width, WordValue value);
  /// Post the binary logical operation \a z = \a x \a wot \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, WordVar z);
  /// Post a binary operation with an explicitly-sized constant operand
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              unsigned int width, WordValue value, WordVar z);
  /// Post a binary operation equal to an explicitly-sized constant result
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, unsigned int width, WordValue value);
  /**
   * \brief Post the n-ary logical operation \a y = \a wot(\a x)
   *
   * Nand, nor, and xnor are the complement of the complete and, or, and
   * xor aggregate respectively. Empty aggregates use the corresponding
   * fixed-width identity.
   */
  GECODE_WORD_EXPORT void rel(Home home, WordOpType wot,
                              const WordVarArgs& x, WordVar y);
  //@}

  /**
   * \defgroup TaskModelWordConditional Word conditional constraints
   * \ingroup TaskModelWord
   */
  //@{
  /** \brief Post whole-word conditional
   *
   * Posts \a result = \a control ? \a then_word : \a else_word.
   */
  GECODE_WORD_EXPORT void ite(Home home, BoolVar control,
                              WordVar then_word, WordVar else_word,
                              WordVar result);
  /// Post whole-word conditional with an explicitly-sized then constant
  GECODE_WORD_EXPORT void ite(Home home, BoolVar control,
                              unsigned int width, WordValue then_value,
                              WordVar else_word, WordVar result);
  /// Post whole-word conditional with an explicitly-sized else constant
  GECODE_WORD_EXPORT void ite(Home home, BoolVar control,
                              WordVar then_word, unsigned int width,
                              WordValue else_value, WordVar result);
  /**
   * \brief Post bitwise conditional controlled by the mask \a control
   *
   * Each result bit is selected independently from the corresponding bit of
   * \a then_word or \a else_word.
   */
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              WordVar then_word, WordVar else_word,
                              WordVar result);
  /// Post bitwise conditional with an explicitly-sized then constant
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              unsigned int width, WordValue then_value,
                              WordVar else_word, WordVar result);
  /// Post bitwise conditional with an explicitly-sized else constant
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              WordVar then_word, unsigned int width,
                              WordValue else_value, WordVar result);
  //@}

  /**
   * \defgroup TaskModelWordStructure Fixed structural constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Extract \a width bits of \a x starting at least-significant index \a first
  GECODE_WORD_EXPORT void extract(Home home, WordVar x,
                                  unsigned int first, unsigned int width,
                                  WordVar y);
  /// Extract from an explicitly-sized constant
  GECODE_WORD_EXPORT void extract(Home home, unsigned int input_width,
                                  WordValue value, unsigned int first,
                                  unsigned int width, WordVar y);
  /// Concatenate \a high above \a low into \a result
  GECODE_WORD_EXPORT void concat(Home home, WordVar high, WordVar low,
                                 WordVar result);
  /// Concatenate an explicitly-sized high constant above \a low
  GECODE_WORD_EXPORT void concat(Home home, unsigned int high_width,
                                 WordValue high, WordVar low, WordVar result);
  /// Concatenate \a high above an explicitly-sized low constant
  GECODE_WORD_EXPORT void concat(Home home, WordVar high,
                                 unsigned int low_width, WordValue low,
                                 WordVar result);
  /// Repeat \a x in \a count blocks from least to most significant
  GECODE_WORD_EXPORT void repeat(Home home, WordVar x,
                                 unsigned int count, WordVar result);
  /// Repeat an explicitly-sized constant
  GECODE_WORD_EXPORT void repeat(Home home, unsigned int input_width,
                                 WordValue value, unsigned int count,
                                 WordVar result);
  /// Zero-extend \a x to the explicitly declared \a result_width
  GECODE_WORD_EXPORT void zero_extend(Home home, WordVar x,
                                      unsigned int result_width,
                                      WordVar result);
  /// Zero-extend an explicitly-sized constant
  GECODE_WORD_EXPORT void zero_extend(Home home, unsigned int input_width,
                                      WordValue value,
                                      unsigned int result_width,
                                      WordVar result);
  /// Sign-extend \a x to the explicitly declared \a result_width
  GECODE_WORD_EXPORT void sign_extend(Home home, WordVar x,
                                      unsigned int result_width,
                                      WordVar result);
  /// Sign-extend an explicitly-sized constant
  GECODE_WORD_EXPORT void sign_extend(Home home, unsigned int input_width,
                                      WordValue value,
                                      unsigned int result_width,
                                      WordVar result);
  /// Logically shift \a x left by the constant \a amount
  GECODE_WORD_EXPORT void shift_left(Home home, WordVar x,
                                     unsigned int amount, WordVar result);
  /** \brief Logically shift \a x left by unsigned word \a amount
   *
   * All operands have the same width. Amounts greater than or equal to the
   * width produce zero, as in SMT-LIB.
   */
  GECODE_WORD_EXPORT void shift_left(Home home, WordVar x,
                                     WordVar amount, WordVar result);
  /// Logically shift an explicitly-sized constant left
  GECODE_WORD_EXPORT void shift_left(Home home, unsigned int width,
                                     WordValue value, unsigned int amount,
                                     WordVar result);
  /// Logically shift \a x right by the constant \a amount
  GECODE_WORD_EXPORT void logical_shift_right(Home home, WordVar x,
                                              unsigned int amount,
                                              WordVar result);
  /** \brief Logically shift \a x right by unsigned word \a amount
   *
   * All operands have the same width. Amounts greater than or equal to the
   * width produce zero, as in SMT-LIB.
   */
  GECODE_WORD_EXPORT void logical_shift_right(Home home, WordVar x,
                                              WordVar amount,
                                              WordVar result);
  /// Logically shift an explicitly-sized constant right
  GECODE_WORD_EXPORT void logical_shift_right(Home home, unsigned int width,
                                              WordValue value,
                                              unsigned int amount,
                                              WordVar result);
  /// Arithmetically shift \a x right by the constant \a amount
  GECODE_WORD_EXPORT void arithmetic_shift_right(Home home, WordVar x,
                                                 unsigned int amount,
                                                 WordVar result);
  /** \brief Arithmetically shift \a x right by unsigned word \a amount
   *
   * All operands have the same width. Amounts greater than or equal to the
   * width fill the result with the sign bit, as in SMT-LIB.
   */
  GECODE_WORD_EXPORT void arithmetic_shift_right(Home home, WordVar x,
                                                 WordVar amount,
                                                 WordVar result);
  /// Arithmetically shift an explicitly-sized constant right
  GECODE_WORD_EXPORT void arithmetic_shift_right(Home home,
                                                 unsigned int width,
                                                 WordValue value,
                                                 unsigned int amount,
                                                 WordVar result);
  /// Rotate \a x left by the constant \a amount modulo its width
  GECODE_WORD_EXPORT void rotate_left(Home home, WordVar x,
                                      unsigned int amount, WordVar result);
  /// Rotate an explicitly-sized constant left
  GECODE_WORD_EXPORT void rotate_left(Home home, unsigned int width,
                                      WordValue value, unsigned int amount,
                                      WordVar result);
  /// Rotate \a x right by the constant \a amount modulo its width
  GECODE_WORD_EXPORT void rotate_right(Home home, WordVar x,
                                       unsigned int amount, WordVar result);
  /// Rotate an explicitly-sized constant right
  GECODE_WORD_EXPORT void rotate_right(Home home, unsigned int width,
                                       WordValue value, unsigned int amount,
                                       WordVar result);
  //@}

  /**
   * \defgroup TaskModelWordArithmetic Word arithmetic constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Post modular addition \a result = \a x + \a y
  GECODE_WORD_EXPORT void add(Home home, WordVar x, WordVar y,
                              WordVar result);
  /** \brief Post the modular sum of the variables in \a x
   *
   * The empty sum is zero and a singleton sum is equality. All variables
   * must have the same width as \a result.
   */
  GECODE_WORD_EXPORT void add(Home home, const WordVarArgs& x,
                              WordVar result);
  /** \brief Post modular addition and expose its unsigned carry
   *
   * Posts \a result = \a x + \a y modulo the common width. \a carry is one
   * exactly when the unsigned sum is at least 2 raised to that width.
   */
  GECODE_WORD_EXPORT void add(Home home, WordVar x, WordVar y,
                              WordVar result, BoolVar carry);
  /// Post modular addition with an explicitly-sized constant operand
  GECODE_WORD_EXPORT void add(Home home, WordVar x, unsigned int width,
                              WordValue value, WordVar result);
  /// Post two's-complement modular negation \a result = -\a x
  GECODE_WORD_EXPORT void neg(Home home, WordVar x, WordVar result);
  /// Post modular negation of an explicitly-sized constant
  GECODE_WORD_EXPORT void neg(Home home, unsigned int width,
                              WordValue value, WordVar result);
  /// Post modular subtraction \a result = \a x - \a y
  GECODE_WORD_EXPORT void sub(Home home, WordVar x, WordVar y,
                              WordVar result);
  /** \brief Post modular subtraction and expose its unsigned borrow
   *
   * Posts \a result = \a x - \a y modulo the common width. \a borrow is one
   * exactly when \a x is unsigned-less-than \a y.
   */
  GECODE_WORD_EXPORT void sub(Home home, WordVar x, WordVar y,
                              WordVar result, BoolVar borrow);
  /// Post modular subtraction with an explicitly-sized right operand
  GECODE_WORD_EXPORT void sub(Home home, WordVar x, unsigned int width,
                              WordValue value, WordVar result);
  /// Post modular subtraction with an explicitly-sized left operand
  GECODE_WORD_EXPORT void sub(Home home, unsigned int width,
                              WordValue value, WordVar y, WordVar result);
  /// Post modular multiplication \a result = \a x * \a y
  GECODE_WORD_EXPORT void mult(Home home, WordVar x, WordVar y,
                               WordVar result);
  /// Post modular multiplication with an explicitly-sized constant operand
  GECODE_WORD_EXPORT void mult(Home home, WordVar x, unsigned int width,
                               WordValue value, WordVar result);
  /** \brief Post mathematical product modulo a positive integer modulus
   *
   * All word operands have the same width. The mathematical product of the
   * two unsigned Word values is reduced modulo \a modulus before conversion
   * to the result Word; it is not first reduced modulo the Word width.
   * \a modulus is constrained to positive values.
   */
  GECODE_WORD_EXPORT void product_mod(Home home, WordVar x, WordVar y,
                                      IntVar modulus, WordVar result);
  /** \brief Post reified mathematical product modulo a positive modulus
   *
   * The positive-modulus contract is unconditional. The relation
   * \f$result=(x\cdot y)\bmod modulus\f$ is reified by \a r.
   */
  GECODE_WORD_EXPORT void product_mod(Home home, WordVar x, WordVar y,
                                      IntVar modulus, WordVar result,
                                      Reify r);
  /** \brief Constrain \a result to the unsigned mathematical GCD
   *
   * All words must have the same width. The relation uses unsigned Word
   * values and defines gcd(0,0)=0.
   */
  GECODE_WORD_EXPORT void gcd(Home home, WordVar x, WordVar y,
                              WordVar result,
                              IntPropLevel ipl=IPL_DEF);
  /// Reify the unsigned mathematical GCD relation
  GECODE_WORD_EXPORT void gcd(Home home, WordVar x, WordVar y,
                              WordVar result, Reify r,
                              IntPropLevel ipl=IPL_DEF);
  /** \brief Constrain \a result to the signed mathematical GCD magnitude
   *
   * Inputs use two's-complement signed values. The same-width result stores
   * the nonnegative magnitude as an unsigned bit pattern; an unsigned-bounded
   * result enables numeric bounds propagation.
   */
  GECODE_WORD_EXPORT void signed_gcd(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     IntPropLevel ipl=IPL_DEF);
  /// Reify the signed mathematical GCD relation
  GECODE_WORD_EXPORT void signed_gcd(Home home, WordVar x, WordVar y,
                                     WordVar result, Reify r,
                                     IntPropLevel ipl=IPL_DEF);
  /** \brief Reify unsigned mathematical divisibility
   *
   * Zero divides zero and no nonzero value. This is mathematical integer
   * divisibility, not multiplication modulo the Word width.
   */
  GECODE_WORD_EXPORT void divides(Home home, WordVar divisor,
                                  WordVar dividend, Reify r,
                                  IntPropLevel ipl=IPL_DEF);
  /// Reify signed mathematical divisibility
  GECODE_WORD_EXPORT void signed_divides(Home home, WordVar divisor,
                                         WordVar dividend, Reify r,
                                         IntPropLevel ipl=IPL_DEF);
  /// Post unary arithmetic overflow predicate \a wot for \a x
  GECODE_WORD_EXPORT void overflow(Home home, WordVar x,
                                   WordOverflowType wot, BoolVar b,
                                   WordSemantics semantics=WS_SMTLIB);
  /// Post binary arithmetic overflow predicate \a wot for \a x and \a y
  GECODE_WORD_EXPORT void overflow(Home home, WordVar x,
                                   WordOverflowType wot, WordVar y, BoolVar b,
                                   WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned division \a result = \a x div \a y
  GECODE_WORD_EXPORT void div(Home home, WordVar x, WordVar y,
                              WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned division with an explicitly-sized right operand
  GECODE_WORD_EXPORT void div(Home home, WordVar x, unsigned int width,
                              WordValue value, WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned division with an explicitly-sized left operand
  GECODE_WORD_EXPORT void div(Home home, unsigned int width,
                              WordValue value, WordVar y, WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned remainder \a result = \a x mod \a y
  GECODE_WORD_EXPORT void mod(Home home, WordVar x, WordVar y,
                              WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned remainder with an explicitly-sized right operand
  GECODE_WORD_EXPORT void mod(Home home, WordVar x, unsigned int width,
                              WordValue value, WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned remainder with an explicitly-sized left operand
  GECODE_WORD_EXPORT void mod(Home home, unsigned int width,
                              WordValue value, WordVar y, WordVar result,
                              WordSemantics semantics=WS_SMTLIB);
  /// Post unsigned division and remainder together
  GECODE_WORD_EXPORT void divmod(Home home, WordVar dividend,
                                 WordVar divisor, WordVar quotient,
                                 WordVar remainder,
                                 WordSemantics semantics=WS_SMTLIB);
  /// Post signed division \a result = \a x signed_div \a y
  GECODE_WORD_EXPORT void signed_div(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed division with an explicitly-sized right operand
  GECODE_WORD_EXPORT void signed_div(Home home, WordVar x,
                                     unsigned int width, WordValue value,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed division with an explicitly-sized left operand
  GECODE_WORD_EXPORT void signed_div(Home home, unsigned int width,
                                     WordValue value, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed remainder \a result = \a x signed_rem \a y
  GECODE_WORD_EXPORT void signed_rem(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed remainder with an explicitly-sized right operand
  GECODE_WORD_EXPORT void signed_rem(Home home, WordVar x,
                                     unsigned int width, WordValue value,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed remainder with an explicitly-sized left operand
  GECODE_WORD_EXPORT void signed_rem(Home home, unsigned int width,
                                     WordValue value, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed modulus \a result = \a x signed_mod \a y
  GECODE_WORD_EXPORT void signed_mod(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed modulus with an explicitly-sized right operand
  GECODE_WORD_EXPORT void signed_mod(Home home, WordVar x,
                                     unsigned int width, WordValue value,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  /// Post signed modulus with an explicitly-sized left operand
  GECODE_WORD_EXPORT void signed_mod(Home home, unsigned int width,
                                     WordValue value, WordVar y,
                                     WordVar result,
                                     WordSemantics semantics=WS_SMTLIB);
  //@}

  /// Branch filter function type for word variables
  typedef std::function<bool(const Space& home, WordVar x, int i)>
    WordBranchFilter;
  /// Branch merit function type for word variables
  typedef std::function<double(const Space& home, WordVar x, int i)>
    WordBranchMerit;
  /// Branch bit selection function type for word variables
  typedef std::function<WordValue(const Space& home, WordVar x, int i)>
    WordBranchVal;
  /// Branch commit function type for word variables
  typedef std::function<void(Space& home, unsigned int a,
                             WordVar x, int i, WordValue value)>
    WordBranchCommit;

}

#include <gecode/word/branch/traits.hpp>

namespace Gecode {

  /// Recording AFC information for word variables
  class WordAFC : public AFC {
  public:
    WordAFC(void);
    WordAFC(const WordAFC& a);
    WordAFC& operator =(const WordAFC& a);
    WordAFC(Home home, const WordVarArgs& x, double d=1.0,
            bool share=true);
    void init(Home home, const WordVarArgs& x, double d=1.0,
              bool share=true);
  };

  /// Recording action information for word variables
  class WordAction : public Action {
  public:
    WordAction(void);
    WordAction(const WordAction& a);
    WordAction& operator =(const WordAction& a);
    GECODE_WORD_EXPORT
    WordAction(Home home, const WordVarArgs& x, double d=1.0,
               bool p=true, bool f=true, WordBranchMerit bm=nullptr);
    GECODE_WORD_EXPORT void
    init(Home home, const WordVarArgs& x, double d=1.0,
         bool p=true, bool f=true, WordBranchMerit bm=nullptr);
  };

  /// Recording CHB information for word variables
  class WordCHB : public CHB {
  public:
    WordCHB(void);
    WordCHB(const WordCHB& c);
    WordCHB& operator =(const WordCHB& c);
    GECODE_WORD_EXPORT
    WordCHB(Home home, const WordVarArgs& x, WordBranchMerit bm=nullptr);
    GECODE_WORD_EXPORT void
    init(Home home, const WordVarArgs& x, WordBranchMerit bm=nullptr);
  };

  /// Function type for printing branching alternatives for word variables
  typedef std::function<void(const Space& home, const Brancher& b,
                             unsigned int a, WordVar x, int i,
                             const WordValue& value, std::ostream& o)>
    WordVarValPrint;

  /**
   * \defgroup TaskModelWordBranch Branching
   * \ingroup TaskModelWord
  */

  /// Which word variable to select for branching
  class WordVarBranch : public VarBranch<WordVar> {
  public:
    enum Select {
      SEL_NONE = 0,
      SEL_RND,
      SEL_MERIT_MIN,
      SEL_MERIT_MAX,
      SEL_DEGREE_MIN,
      SEL_DEGREE_MAX,
      SEL_AFC_MIN,
      SEL_AFC_MAX,
      SEL_ACTION_MIN,
      SEL_ACTION_MAX,
      SEL_CHB_MIN,
      SEL_CHB_MAX,
      SEL_SIZE_MIN,
      SEL_SIZE_MAX,
      SEL_DEGREE_SIZE_MIN,
      SEL_DEGREE_SIZE_MAX,
      SEL_AFC_SIZE_MIN,
      SEL_AFC_SIZE_MAX,
      SEL_ACTION_SIZE_MIN,
      SEL_ACTION_SIZE_MAX,
      SEL_CHB_SIZE_MIN,
      SEL_CHB_SIZE_MAX
    };
  protected:
    Select s;
  public:
    WordVarBranch(void);
    WordVarBranch(Rnd r);
    WordVarBranch(Select s, BranchTbl t);
    WordVarBranch(Select s, double d, BranchTbl t);
    WordVarBranch(Select s, WordAFC a, BranchTbl t);
    WordVarBranch(Select s, WordAction a, BranchTbl t);
    WordVarBranch(Select s, WordCHB c, BranchTbl t);
    WordVarBranch(Select s, WordBranchMerit mf, BranchTbl t);
    Select select(void) const;
    void expand(Home home, const WordVarArgs& x);
  };

  WordVarBranch WORD_VAR_NONE(void);
  WordVarBranch WORD_VAR_RND(Rnd r);
  WordVarBranch WORD_VAR_MERIT_MIN(WordBranchMerit bm,
                                   BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_MERIT_MAX(WordBranchMerit bm,
                                   BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_DEGREE_MIN(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_DEGREE_MAX(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_MIN(double d=1.0, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_MIN(WordAFC a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_MAX(double d=1.0, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_MAX(WordAFC a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_MIN(double d=1.0, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_MIN(WordAction a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_MAX(double d=1.0, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_MAX(WordAction a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_MIN(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_MIN(WordCHB c, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_MAX(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_MAX(WordCHB c, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_SIZE_MIN(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_SIZE_MAX(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_DEGREE_SIZE_MIN(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_DEGREE_SIZE_MAX(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_SIZE_MIN(double d=1.0,
                                      BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_SIZE_MIN(WordAFC a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_SIZE_MAX(double d=1.0,
                                      BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_AFC_SIZE_MAX(WordAFC a, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_SIZE_MIN(double d=1.0,
                                         BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_SIZE_MIN(WordAction a,
                                         BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_SIZE_MAX(double d=1.0,
                                         BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_ACTION_SIZE_MAX(WordAction a,
                                         BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_SIZE_MIN(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_SIZE_MIN(WordCHB c, BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_SIZE_MAX(BranchTbl tbl=nullptr);
  WordVarBranch WORD_VAR_CHB_SIZE_MAX(WordCHB c, BranchTbl tbl=nullptr);

  /// Select an unknown bit or split a bounded ranked interval
  class WordValBranch : public ValBranch<WordVar> {
  public:
    /// Bit selection strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_SPLIT_MIN, ///< Ranked interval split, lower half first
      SEL_SPLIT_MAX, ///< Ranked interval split, upper half first
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
  protected:
    Select s;
  public:
    WordValBranch(Select s=SEL_LSB);
    WordValBranch(Select s, Rnd r);
    /// Initialize with value function \a v and commit function \a c
    WordValBranch(WordBranchVal v, WordBranchCommit c);
    Select select(void) const;
  };

  /// Select the least-significant unknown bit
  WordValBranch WORD_VAL_LSB(void);
  /// Select the most-significant unknown bit
  WordValBranch WORD_VAL_MSB(void);
  /// Select a random unknown bit
  WordValBranch WORD_VAL_RND(Rnd r);
  /// Split a bounded variable's ranked interval, lower half first
  WordValBranch WORD_VAL_SPLIT_MIN(void);
  /// Split a bounded variable's ranked interval, upper half first
  WordValBranch WORD_VAL_SPLIT_MAX(void);
  /**
   * \brief Select a value with \a v and commit it with \a c
   *
   * With no commit callback, the value is interpreted as a bit position and
   * the alternatives fix that bit to zero and one. A user-defined commit
   * callback does not provide generic no-good literals.
   */
  WordValBranch WORD_VAL(WordBranchVal v, WordBranchCommit c=nullptr);

  /// Select bit-zero or admitted ranked-value assignment
  class WordAssign : public ValBranch<WordVar> {
  public:
    /// Bit selection strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_MIN, ///< Minimum admitted ranked value
      SEL_MED, ///< Median admitted ranked value
      SEL_MAX, ///< Maximum admitted ranked value
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
  protected:
    Select s;
  public:
    WordAssign(Select s=SEL_LSB);
    WordAssign(Select s, Rnd r);
    /// Initialize with value function \a v and commit function \a c
    WordAssign(WordBranchVal v, WordBranchCommit c);
    Select select(void) const;
  };

  /// Assign unknown bits to zero, least-significant bit first
  WordAssign WORD_ASSIGN_LSB(void);
  /// Assign unknown bits to zero, most-significant bit first
  WordAssign WORD_ASSIGN_MSB(void);
  /// Assign unknown bits to zero in random order
  WordAssign WORD_ASSIGN_RND(Rnd r);
  /// Assign each bounded variable to its minimum admitted ranked value
  WordAssign WORD_ASSIGN_MIN(void);
  /// Assign each bounded variable to an admitted median ranked value
  WordAssign WORD_ASSIGN_MED(void);
  /// Assign each bounded variable to its maximum admitted ranked value
  WordAssign WORD_ASSIGN_MAX(void);
  /**
   * \brief Assign a value selected by \a v and committed by \a c
   *
   * With no commit callback, the value is interpreted as a bit position and
   * that bit is fixed to zero. A user-defined commit callback does not provide
   * generic no-good literals.
   */
  WordAssign WORD_ASSIGN(WordBranchVal v, WordBranchCommit c=nullptr);

  /// Branch over all word variables, trying zero before one
  GECODE_WORD_EXPORT void branch(Home home, const WordVarArgs& x,
                                 WordVarBranch vars, WordValBranch vals,
                                 WordBranchFilter bf=nullptr,
                                 WordVarValPrint vvp=nullptr);
  /// Branch over all word variables, trying zero before one
  GECODE_WORD_EXPORT void branch(Home home, const WordVarArgs& x,
                                 WordValBranch vals=WORD_VAL_LSB());
  /// Branch over one word variable, trying zero before one
  GECODE_WORD_EXPORT void branch(Home home, WordVar x,
                                 WordValBranch vals=WORD_VAL_LSB());
  /// Assign all word variables by fixing selected unknown bits to zero
  GECODE_WORD_EXPORT void assign(Home home, const WordVarArgs& x,
                                 WordVarBranch vars, WordAssign vals,
                                 WordBranchFilter bf=nullptr,
                                 WordVarValPrint vvp=nullptr);
  /// Assign all word variables by fixing selected unknown bits to zero
  GECODE_WORD_EXPORT void assign(Home home, const WordVarArgs& x,
                                 WordAssign vals=WORD_ASSIGN_LSB());
  /// Assign one word variable by fixing selected unknown bits to zero
  GECODE_WORD_EXPORT void assign(Home home, WordVar x,
                                 WordAssign vals=WORD_ASSIGN_LSB());

  /// Delta reported by word-variable tracing
  class WordTraceDelta {
  private:
    WordValue _zero;
    WordValue _one;
    WordDomainType _domain_type;
    WordValue _old_minimum;
    WordValue _old_maximum;
    WordValue _new_minimum;
    WordValue _new_maximum;
  public:
    WordTraceDelta(Word::WordTraceView o, Word::WordView n,
                   const Delta& d);
    /// Return bits newly fixed to zero
    WordValue zero(void) const;
    /// Return bits newly fixed to one
    WordValue one(void) const;
    /// Return the immutable domain interpretation
    WordDomainType domain_type(void) const;
    /// Test whether the traced variable has ranked bounds
    bool bounded(void) const;
    /// Return the old internal order-rank minimum
    WordValue old_minimum(void) const;
    /// Return the old internal order-rank maximum
    WordValue old_maximum(void) const;
    /// Return the new internal order-rank minimum
    WordValue new_minimum(void) const;
    /// Return the new internal order-rank maximum
    WordValue new_maximum(void) const;
    /// Test whether the trace event fixed any cube bits
    bool bits_changed(void) const;
    /// Test whether the trace event changed ranked bounds
    bool bounds_changed(void) const;
  };

}

#include <gecode/word/trace/delta.hpp>
#include <gecode/word/trace/traits.hpp>

namespace Gecode {

  /// Tracer for word variables
  typedef ViewTracer<Word::WordView> WordTracer;
  /// Trace recorder for word variables
  typedef ViewTraceRecorder<Word::WordView> WordTraceRecorder;

  /// Standard word-variable tracer
  class GECODE_WORD_EXPORT StdWordTracer : public WordTracer {
  protected:
    std::ostream& os;
  public:
    StdWordTracer(std::ostream& os0=std::cerr);
    virtual void init(const Space& home, const WordTraceRecorder& t);
    virtual void prune(const Space& home, const WordTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, WordTraceDelta& d);
    virtual void fix(const Space& home, const WordTraceRecorder& t);
    virtual void fail(const Space& home, const WordTraceRecorder& t);
    virtual void done(const Space& home, const WordTraceRecorder& t);
    static StdWordTracer def;
  };

  GECODE_WORD_EXPORT void
  trace(Home home, const WordVarArgs& x, TraceFilter tf,
        int te=(TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        WordTracer& t=StdWordTracer::def);
  void
  trace(Home home, const WordVarArgs& x,
        int te=(TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        WordTracer& t=StdWordTracer::def);

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const WordVar& x);
}

#include <gecode/word/array.hpp>
#include <gecode/word/branch.hpp>
#include <gecode/word/print.hpp>
#include <gecode/word/trace.hpp>

#endif

// IFDEF: GECODE_HAS_WORD_VARS
// STATISTICS: word-post
