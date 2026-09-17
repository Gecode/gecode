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

  /** \brief Construction-time representation for a word domain
   *
   * The representation is immutable. Explicitly signed and unsigned word
   * operations choose their own interpretation; exact integer arithmetic
   * constraints interpret WDT_SIGNED operands as signed values.
   * Signed intervals order two's-complement values; their public endpoints
   * remain encoded WordValue bit patterns.
   * \ingroup TaskModelWordVars
   */
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
   * A word variable has an immutable width from 1 through 64. Its domain is
   * a cube of known-one and may-be-one bits, optionally intersected with an
   * unsigned or signed numeric interval. WordDomainType selects this
   * representation at construction. Signed operations interpret bit patterns
   * in two's complement, independently of the domain representation.
   * Constants carry an explicit width. Modular arithmetic wraps modulo
   * \f$2^{width}\f$; exceptional division cases follow WordSemantics.
   *
   * Bounded domains can improve numeric propagation, but do not imply domain
   * consistency. Unsupported numeric cases retain bit propagation with the
   * same operation semantics. Each constraint documents any stronger
   * propagation guarantee. Repeated variables are supported unless a
   * declaration states otherwise.
   *
   * WordVar provides mask and numeric-bound queries. WordRelType and
   * WordOpType select direct relations and logical operations. Models branch
   * with WordVarBranch and WordValBranch, and observe changes with WordTracer.
   * For expression syntax and domain policies, see \ref TaskModelMiniModelWord.
   * Propagator implementation notes are in \ref FuncWordProp.
   *
   * \ingroup TaskModel
   */
  /**
   * \defgroup TaskModelWordVars Word-vector variables and arrays
   * \ingroup TaskModelWord
   */
  /**
   * \brief Fixed-width word variable
   *
   * Widths from 1 through 64 are supported. The domain is a cube, optionally
   * intersected with one unsigned or signed interval. The lo() and hi()
   * queries return known-one and may-be-one masks, not numeric endpoints.
   * Use minimum() and maximum() for the endpoints of a bounded domain.
   * \ingroup TaskModelWordVars
   */
  class WordVar : public VarImpVar<Word::WordVarImp> {
    friend class WordVarArray;
    friend class WordVarArgs;
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
  private:
    using VarImpVar<Word::WordVarImp>::x;
    void _init(Space& home, unsigned int width, WordValue lo, WordValue hi);
    void _init(Space& home, const Word::PreparedWordDomain& domain);
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
  /** \addtogroup TaskModelWordVars
   * @{
   */

  /// Passing fixed word values
  class WordValArgs : public ArgArray<WordValue> {
  public:
    WordValArgs(void);
    explicit WordValArgs(int n);
    WordValArgs(const SharedArray<WordValue>& x);
    WordValArgs(const std::vector<WordValue>& x);
    WordValArgs(std::initializer_list<WordValue> x);
    template<class InputIterator>
    WordValArgs(InputIterator first, InputIterator last);
    WordValArgs(int n, const WordValue* e);
    WordValArgs(const ArgArray<WordValue>& a);
  };

  /// Passing word variables
  class WordVarArgs : public VarArgArray<WordVar> {
  public:
    WordVarArgs(void);
    /** \brief Construct \a n uninitialized handles
     *
     * Throws Word::OutOfLimits if \a n is negative; zero is allowed.
     */
    explicit WordVarArgs(int n);
    WordVarArgs(const WordVarArgs& a);
    WordVarArgs(const VarArray<WordVar>& a);
    WordVarArgs(const std::vector<WordVar>& a);
    WordVarArgs(std::initializer_list<WordVar> a);
    template<class InputIterator> WordVarArgs(InputIterator first, InputIterator last);
    /** \brief Construct \a n cube words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or mask,
     * and Word::VariableEmptyDomain if the masks have no common value.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValue lo, WordValue hi);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or
     * domain kind.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type);
    /** \brief Construct bounded words with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for a negative count, invalid width, domain kind, or
     * endpoint encoding, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum);
    /** \brief Construct bounded cubes with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for a negative count, invalid width, mask, domain kind,
     * or endpoint encoding, and Word::VariableEmptyDomain if the cube and
     * interval have an empty intersection.
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
    /** \brief Construct \a n uninitialized handles
     *
     * Throws Word::OutOfLimits if \a n is negative; zero is allowed.
     */
    WordVarArray(Space& home, int n);
    WordVarArray(const WordVarArray& a);
    WordVarArray(Space& home, const WordVarArgs& a);
    /** \brief Construct \a n cube words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or mask,
     * and Word::VariableEmptyDomain if the masks have no common value.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValue lo, WordValue hi);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or
     * domain kind.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type);
    /** \brief Construct bounded words with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for a negative count, invalid width, domain kind, or
     * endpoint encoding, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type,
                                   WordValue minimum, WordValue maximum);
    /** \brief Construct bounded cubes with encoded endpoints
     *
     * Signed endpoints are two's-complement patterns. Throws
     * Word::OutOfLimits for a negative count, invalid width, mask, domain kind,
     * or endpoint encoding, and Word::VariableEmptyDomain if the cube and
     * interval have an empty intersection.
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

  /** @} */

  /**
   * \defgroup TaskModelWordExtensional Extensional constraints
   * \ingroup TaskModelWord
   */
  /** @{ */

  /**
   * \brief Immutable tuple set of encoded word values
   *
   * Each column has its own width (1..64). Rows are deduplicated and
   * indexed once; copies and posted constraints share the immutable indexes.
   * An empty row set denotes false, including for zero columns.
   */
  class GECODE_WORD_EXPORT WordTupleSet : public SharedHandle {
  public:
    class Data;
    WordTupleSet(const std::vector<unsigned int>& widths,
                 const std::vector<std::vector<WordValue>>& rows);
    int arity(void) const;
    int tuples(void) const;
    unsigned int width(int column) const;
    WordValue value(int row, int column) const;
    /// Internal immutable support indexes
    const Data& data(void) const;
  };

  /**
   * \brief Constrain words to a row of \a tuples
   *
   * Maintains the exact cube hull and, for bounded domains, the exact
   * signed or unsigned interval hull of compatible tuples. Supports mixed
   * widths, mixed domain types, and repeated variables. Interior values
   * that cannot be removed by the domain representation may remain.
   * Throws Word::OutOfLimits for arity mismatch and Word::WidthMismatch
   * for column width mismatch.
   */
  GECODE_WORD_EXPORT void extensional(Home home, const WordVarArgs& x,
                                      const WordTupleSet& tuples);



  /** @} */

  /**
   * \defgroup TaskModelWordElement Element constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
  /**
   * \brief Post the element constraint \f$x_i=y\f$
   *
   * The index \a i is zero based. All words in \a x and \a y must have
   * the same width.
   */
  GECODE_WORD_EXPORT void element(Home home, const WordVarArgs& x,
                                  IntVar i, WordVar y);
  /** @} */

  /**
   * \defgroup TaskModelWordCount Bit-count constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
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
  /** @} */

  /**
   * \defgroup TaskModelWordChannel Channel constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
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
  /** @} */

  /** \brief Word relation type
   *
   * Signed relations use two's-complement values; unsigned relations use
   * unsigned values, regardless of the operands' domain representations.
   * \ingroup TaskModelWordRel
   */
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

  /** \brief Word logical operation type
   *  \ingroup TaskModelWordLogic
   */
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
   * WS_SMTLIB is the only supported policy. Operations taking this policy
   * throw Word::UnknownOperation for any other value. Division, remainder,
   * and modulus are total, including a zero divisor and signed minimum/-1.
   * Their exceptional results are documented at the corresponding postings.
   * \ingroup TaskModelWordArithmetic
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
  /** @{ */
  /// Post the relation \a wrt between \a x and \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y);
  /** \brief Post distinctness of all words in \a x
   *
   * IPL_VAL, the default, provides value consistency. IPL_BND provides bounds
   * consistency over numeric intervals when all variables have the same
   * bounded interpretation; other cases fall back to value consistency.
   * IPL_DOM also provides value consistency.
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
  /** @} */

  /**
   * \defgroup TaskModelWordLogic Word logical constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
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
  /** @} */

  /**
   * \defgroup TaskModelWordConditional Word conditional constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
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
  /** @} */

  /**
   * \defgroup TaskModelWordStructure Fixed structural constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
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
  /** \brief Rotate \a x left by word \a count modulo its width
   *
   * WordVar widths range from 1 through 64. Unequal operand widths throw
   * Word::WidthMismatch. The count is its unsigned encoded pattern,
   * including when its domain uses signed order. Counts may exceed the width.
   * Propagation narrows the bit masks of all three operands. Bounds contribute
   * through mask synchronization; interval holes and alias correlations may
   * remain until assignment. This does not enforce domain consistency.
   */
  GECODE_WORD_EXPORT void rotate_left(Home home, WordVar x,
                                      WordVar count, WordVar result);
  /// Rotate an explicitly-sized constant left
  GECODE_WORD_EXPORT void rotate_left(Home home, unsigned int width,
                                      WordValue value, unsigned int amount,
                                      WordVar result);
  /// Rotate \a x right by the constant \a amount modulo its width
  GECODE_WORD_EXPORT void rotate_right(Home home, WordVar x,
                                       unsigned int amount, WordVar result);
  /** \brief Rotate \a x right by word \a count modulo its width
   *
   * WordVar widths range from 1 through 64. Unequal operand widths throw
   * Word::WidthMismatch. The count is its unsigned encoded pattern,
   * including when its domain uses signed order. Counts may exceed the width.
   * Propagation narrows the bit masks of all three operands. Bounds contribute
   * through mask synchronization; interval holes and alias correlations may
   * remain until assignment. This does not enforce domain consistency.
   */
  GECODE_WORD_EXPORT void rotate_right(Home home, WordVar x,
                                      WordVar count, WordVar result);
  /// Rotate an explicitly-sized constant right
  GECODE_WORD_EXPORT void rotate_right(Home home, unsigned int width,
                                       WordValue value, unsigned int amount,
                                       WordVar result);
  /** @} */

  /**
   * \defgroup TaskModelWordArithmetic Word arithmetic constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
  /// Post modular addition \a result = \a x + \a y
  GECODE_WORD_EXPORT void add(Home home, WordVar x, WordVar y,
                              WordVar result);
  /** \brief Post the modular sum of the variables in \a x
   *
   * The empty sum is zero and a singleton sum is equality. All variables
   * must have the same width as \a result. Repeated variables, including
   * aliases of \a result, are supported.
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
   * \a modulus is constrained to positive values. Propagation can restrict
   * masks and numeric bounds, but does not guarantee bounds or domain
   * consistency. Unsupported masked endpoints can remain.
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
   * values and defines gcd(0,0)=0. Repeated variables are supported.
   * Propagation can restrict masks and numeric bounds, but does not guarantee
   * bounds or domain consistency. Unsupported endpoints can remain.
   * All values of \a ipl currently select the same propagation.
   */
  GECODE_WORD_EXPORT void gcd(Home home, WordVar x, WordVar y,
                              WordVar result,
                              IntPropLevel ipl=IPL_DEF);
  /** \brief Reify the unsigned mathematical GCD relation
   *
   * The width, alias, and propagation rules of gcd() also apply here.
   */
  GECODE_WORD_EXPORT void gcd(Home home, WordVar x, WordVar y,
                              WordVar result, Reify r,
                              IntPropLevel ipl=IPL_DEF);
  /** \brief Constrain \a result to the signed mathematical GCD magnitude
   *
   * Inputs use two's-complement signed values. The same-width result stores
   * the nonnegative magnitude as an unsigned bit pattern, including the
   * magnitude of the signed minimum. An unsigned-bounded result permits
   * numeric bounds propagation with signed-bounded inputs. The relation
   * defines gcd(0,0)=0 and supports repeated variables. It does not guarantee
   * bounds or domain consistency. All values of \a ipl currently select the
   * same propagation.
   */
  GECODE_WORD_EXPORT void signed_gcd(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     IntPropLevel ipl=IPL_DEF);
  /** \brief Reify the signed mathematical GCD relation
   *
   * The width, alias, and propagation rules of signed_gcd() also apply here.
   */
  GECODE_WORD_EXPORT void signed_gcd(Home home, WordVar x, WordVar y,
                                     WordVar result, Reify r,
                                     IntPropLevel ipl=IPL_DEF);
  /** \brief Reify unsigned mathematical divisibility
   *
   * Operands have the same width and use unsigned values. Zero divides zero
   * and no nonzero value. This is mathematical integer divisibility, not
   * multiplication modulo the Word width. Repeated variables are supported.
   * The relation follows the mode of \a r. Cube and interval domains cannot
   * represent arbitrary holes, so excluding a value can require further
   * narrowing. Propagation does not guarantee domain consistency. All values
   * of \a ipl currently select the same propagation.
   */
  GECODE_WORD_EXPORT void divides(Home home, WordVar divisor,
                                  WordVar dividend, Reify r,
                                  IntPropLevel ipl=IPL_DEF);
  /** \brief Reify signed mathematical divisibility
   *
   * Operands use two's-complement signed values, including the signed minimum.
   * The width, zero, alias, reification, and propagation rules of divides()
   * also apply here.
   */
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
  /** \brief Post unsigned division \a result = \a x div \a y
   *
   * With WS_SMTLIB, division by zero returns the all-one word.
   */
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
  /** \brief Post unsigned remainder \a result = \a x mod \a y
   *
   * With WS_SMTLIB, remainder by zero returns the dividend.
   */
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
  /** \brief Post unsigned division and remainder together
   *
   * Uses the semantics of div() and mod(), including a zero divisor.
   * All operands have the same width and may alias each other.
   */
  GECODE_WORD_EXPORT void divmod(Home home, WordVar dividend,
                                 WordVar divisor, WordVar quotient,
                                 WordVar remainder,
                                 WordSemantics semantics=WS_SMTLIB);
  /** \brief Post signed division \a result = \a x signed_div \a y
   *
   * Inputs and result use two's-complement values, with division rounded
   * toward zero. With WS_SMTLIB, division by zero returns one for a negative
   * dividend and the all-one word otherwise. Signed minimum divided by -1
   * returns signed minimum.
   */
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
  /** \brief Post signed remainder \a result = \a x signed_rem \a y
   *
   * The nonzero remainder has the dividend's sign. With WS_SMTLIB, remainder
   * by zero returns the dividend; signed minimum remainder -1 is zero.
   */
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
  /** \brief Post signed modulus \a result = \a x signed_mod \a y
   *
   * The nonzero modulus result has the divisor's sign. With WS_SMTLIB,
   * modulus by zero returns the dividend; signed minimum modulus -1 is zero.
   */
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

  /// One row in a joint exact-integer linear Word relation
  class WordLinearRow {
  public:
    /// Kind of linear relation represented by the row
    enum Type { EQUAL, RANGE, CONGRUENCE };
  private:
    Type _type;
    IntArgs _coefficients;
    int _lower, _upper;
    WordValue _modulus;
    WordLinearRow(Type type, const IntArgs& a, int lower, int upper,
                  WordValue modulus);
  public:
    /// Construct the tautology 0=0
    WordLinearRow(void);
    /// Construct sum(a[i]*x[i])=b
    static WordLinearRow equal(const IntArgs& a, int b);
    /// Construct l<=sum(a[i]*x[i])<=u
    static WordLinearRow range(const IntArgs& a, int l, int u);
    /// Construct sum(a[i]*x[i])=r (mod modulus)
    static WordLinearRow congruence(const IntArgs& a, int r,
                                    WordValue modulus);
    /// Return the relation kind
    Type type(void) const;
    /// Return the coefficients
    const IntArgs& coefficients(void) const;
    /// Return the lower bound, equality value, or congruence residue
    int lower(void) const;
    /// Return the upper bound (equal to lower except for a range)
    int upper(void) const;
    /// Return the modulus for a congruence and zero otherwise
    WordValue modulus(void) const;
  };

  /// Passing rows for a joint exact-integer linear Word relation
  class WordLinearRowArgs : public ArgArray<WordLinearRow> {
  public:
    using ArgArray<WordLinearRow>::ArgArray;
    WordLinearRowArgs(void) : ArgArray<WordLinearRow>(0) {}
  };

  /** \brief Exact integer product balance
   *
   * Enforces \f$c\prod x=d\prod y\f$ without intermediate wrapping.
   * WDT_SIGNED operands use two's-complement signed values; all other operands
   * use unsigned values. Assigned tuples are checked exactly.
   */
  GECODE_WORD_EXPORT void
  product_balance(Home home, const WordVarArgs& x, WordValue c,
                  const WordVarArgs& y, WordValue d,
                  IntPropLevel ipl=IPL_DEF);
  /// Enforce product(x)=d*q+r, d>0 and 0<=r<d, without intermediate wrapping
  GECODE_WORD_EXPORT void
  product_divmod(Home home, const WordVarArgs& x, WordVar d, WordVar q,
                 WordVar r, IntPropLevel ipl=IPL_DEF);
  /// Enforce c+sum(a[i]*x[i])=d*q+r, d>0 and 0<=r<d
  GECODE_WORD_EXPORT void
  linear_divmod(Home home, const IntArgs& a, const WordVarArgs& x, int c,
                WordVar d, WordVar q, WordVar r,
                IntPropLevel ipl=IPL_DEF);
  /// Constrain y to the least value not below x congruent to phase modulo spacing
  GECODE_WORD_EXPORT void
  quantize_up(Home home, WordVar x, WordValue spacing, WordValue phase,
              WordVar y, IntPropLevel ipl=IPL_DEF);
  /// Enforce a joint system of exact integer equalities, ranges, and congruences
  GECODE_WORD_EXPORT void
  linear_system(Home home, const WordVarArgs& x,
                const WordLinearRowArgs& rows,
                IntPropLevel ipl=IPL_DEF);
  /// Relate fixed-radix digits, last digit fastest, to their exact integer rank
  GECODE_WORD_EXPORT void
  mixed_radix(Home home, const WordVarArgs& digits,
              const WordValArgs& radices, WordVar rank,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Enforce outputs=A*coordinates+offset with explicit coordinates
   *
   * Matrix \a a is row-major with one row per output and one column per
   * coordinate.
   */
  GECODE_WORD_EXPORT void
  bounded_image(Home home, const WordVarArgs& coordinates,
                const IntArgs& a, const IntArgs& offset,
                const WordVarArgs& outputs,
                IntPropLevel ipl=IPL_DEF);
  /** @} */

  /** \addtogroup TaskModelWordBranch
   * @{
   */

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

  /** @} */

}

#include <gecode/word/branch/traits.hpp>

namespace Gecode {

  /** \addtogroup TaskModelWordBranch
   * @{
   */

  /// Recording AFC information for word variables
  class WordAFC : public AFC {
  public:
    WordAFC(void);
    WordAFC(const WordAFC& a);
    WordAFC& operator =(const WordAFC& a);
    /// Record AFC for \a x, sharing across spaces when \a is_shared is true
    WordAFC(Home home, const WordVarArgs& x, double d=1.0,
            bool is_shared=true);
    /// Record AFC for \a x, sharing across spaces when \a is_shared is true
    void init(Home home, const WordVarArgs& x, double d=1.0,
              bool is_shared=true);
  };

  /// Recording action information for word variables
  class WordAction : public Action {
  public:
    WordAction(void);
    WordAction(const WordAction& a);
    WordAction& operator =(const WordAction& a);
    /** \brief Record action for \a x
     *
     * Count propagation when \a is_propagation_counted is true and failure
     * when \a is_failure_counted is true.
     */
    GECODE_WORD_EXPORT
    WordAction(Home home, const WordVarArgs& x, double d=1.0,
               bool is_propagation_counted=true,
               bool is_failure_counted=true, WordBranchMerit bm=nullptr);
    /** \brief Record action for \a x
     *
     * Count propagation when \a is_propagation_counted is true and failure
     * when \a is_failure_counted is true.
     */
    GECODE_WORD_EXPORT void
    init(Home home, const WordVarArgs& x, double d=1.0,
         bool is_propagation_counted=true,
         bool is_failure_counted=true, WordBranchMerit bm=nullptr);
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

  /** @} */

  /**
   * \defgroup TaskModelWordBranch Branching
   * \ingroup TaskModelWord
   */
  /** @{ */

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
  protected:
    Select s;
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
    /// Bit selection, ranked split, or callback strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_SPLIT_MIN, ///< Ranked interval split, lower half first
      SEL_SPLIT_MAX, ///< Ranked interval split, upper half first
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
    WordValBranch(Select s=SEL_LSB);
    WordValBranch(Select s, Rnd r);
    /// Initialize with value function \a v and commit function \a c
    WordValBranch(WordBranchVal v, WordBranchCommit c);
    Select select(void) const;
  protected:
    Select s;
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

  /// Select bit-zero, admitted ranked-value, or callback assignment
  class WordAssign : public ValBranch<WordVar> {
  public:
    /// Bit selection, ranked value, or callback strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_MIN, ///< Minimum admitted ranked value
      SEL_MED, ///< Median admitted ranked value
      SEL_MAX, ///< Maximum admitted ranked value
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
    WordAssign(Select s=SEL_LSB);
    WordAssign(Select s, Rnd r);
    /// Initialize with value function \a v and commit function \a c
    WordAssign(WordBranchVal v, WordBranchCommit c);
    Select select(void) const;
  protected:
    Select s;
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

  /// Branch over \a x using the supplied variable and value strategies
  GECODE_WORD_EXPORT void branch(Home home, const WordVarArgs& x,
                                 WordVarBranch vars, WordValBranch vals,
                                 WordBranchFilter bf=nullptr,
                                 WordVarValPrint vvp=nullptr);
  /// Branch over \a x in array order using the supplied value strategy
  GECODE_WORD_EXPORT void branch(Home home, const WordVarArgs& x,
                                 WordValBranch vals=WORD_VAL_LSB());
  /// Branch over \a x using the supplied value strategy
  GECODE_WORD_EXPORT void branch(Home home, WordVar x,
                                 WordValBranch vals=WORD_VAL_LSB());
  /// Assign \a x using the supplied variable and assignment strategies
  GECODE_WORD_EXPORT void assign(Home home, const WordVarArgs& x,
                                 WordVarBranch vars, WordAssign vals,
                                 WordBranchFilter bf=nullptr,
                                 WordVarValPrint vvp=nullptr);
  /// Assign \a x in array order using the supplied assignment strategy
  GECODE_WORD_EXPORT void assign(Home home, const WordVarArgs& x,
                                 WordAssign vals=WORD_ASSIGN_LSB());
  /// Assign \a x using the supplied assignment strategy
  GECODE_WORD_EXPORT void assign(Home home, WordVar x,
                                 WordAssign vals=WORD_ASSIGN_LSB());

  /** @} */

  /**
   * \defgroup TaskWordTrace Tracing for word variables
   * \ingroup TaskTrace TaskModelWord
   */
  /** @{ */

  /// Delta reported by word-variable tracing
  class WordTraceDelta {
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
  private:
    WordValue _zero;
    WordValue _one;
    WordDomainType _domain_type;
    WordValue _old_minimum;
    WordValue _old_maximum;
    WordValue _new_minimum;
    WordValue _new_maximum;
  };

  /** @} */

}

#include <gecode/word/trace/delta.hpp>
#include <gecode/word/trace/traits.hpp>

namespace Gecode {

  /** \addtogroup TaskWordTrace
   * @{
   */

  /// Tracer for word variables
  typedef ViewTracer<Word::WordView> WordTracer;
  /// Trace recorder for word variables
  typedef ViewTraceRecorder<Word::WordView> WordTraceRecorder;

  /// Standard word-variable tracer
  class GECODE_WORD_EXPORT StdWordTracer : public WordTracer {
  public:
    StdWordTracer(std::ostream& os0=std::cerr);
    virtual void init(const Space& home, const WordTraceRecorder& t);
    virtual void prune(const Space& home, const WordTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, WordTraceDelta& d);
    virtual void fix(const Space& home, const WordTraceRecorder& t);
    virtual void fail(const Space& home, const WordTraceRecorder& t);
    virtual void done(const Space& home, const WordTraceRecorder& t);
    static StdWordTracer def;
  protected:
    std::ostream& os;
  };

  GECODE_WORD_EXPORT void
  trace(Home home, const WordVarArgs& x, TraceFilter tf,
        int te=(TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        WordTracer& t=StdWordTracer::def);
  void
  trace(Home home, const WordVarArgs& x,
        int te=(TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        WordTracer& t=StdWordTracer::def);

  /** @} */

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
