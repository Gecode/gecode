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
#include <type_traits>
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
   *
   * The float types make the word an encoding of an IEEE-style
   * floating-point format of fixed width. Their interval follows the
   * IEEE 754 total order of the format: negative NaNs (largest payload
   * first), negative infinity, negative numbers by decreasing magnitude,
   * -0, +0, positive numbers, positive infinity, and positive NaNs
   * (signaling below quiet). NaNs and signed zeros thus take their
   * total-order places; a format without infinities or NaNs skips them.
   * The endpoints remain encoded bit patterns. E4M3FN and E2M1 words carry
   * an inline value set by default (see WordValueSetOption). Integer word
   * constraints act on the bits of a float word and never read its
   * interval as an integer interval.
   * \ingroup TaskModelWordVars
   */
  enum class WordDomainType {
    WDT_CUBE,     ///< Cube of independently known bits
    WDT_UNSIGNED, ///< Cube intersected with an unsigned numeric interval
    WDT_SIGNED,   ///< Cube intersected with a signed numeric interval
    WDT_FP32,     ///< IEEE 754 binary32 (width 32) in total order
    WDT_BF16,     ///< bfloat16 (width 16) in total order
    WDT_BINARY16, ///< IEEE 754 binary16 (width 16) in total order
    WDT_E4M3FN,   ///< OCP FP8 E4M3FN (width 8, no infinity) in total order
    WDT_E2M1      ///< OCP FP4 E2M1 (width 4, no infinity or NaN) in total order
  };
  /// Cube of independently known bits
  static constexpr WordDomainType WDT_CUBE = WordDomainType::WDT_CUBE;
  /// Cube intersected with an unsigned numeric interval
  static constexpr WordDomainType WDT_UNSIGNED = WordDomainType::WDT_UNSIGNED;
  /// Cube intersected with a signed numeric interval
  static constexpr WordDomainType WDT_SIGNED = WordDomainType::WDT_SIGNED;
  /// IEEE 754 binary32 (width 32) in total order
  static constexpr WordDomainType WDT_FP32 = WordDomainType::WDT_FP32;
  /// bfloat16 (width 16) in total order
  static constexpr WordDomainType WDT_BF16 = WordDomainType::WDT_BF16;
  /// IEEE 754 binary16 (width 16) in total order
  static constexpr WordDomainType WDT_BINARY16 = WordDomainType::WDT_BINARY16;
  /// OCP FP8 E4M3FN (width 8, no infinity) in total order
  static constexpr WordDomainType WDT_E4M3FN = WordDomainType::WDT_E4M3FN;
  /// OCP FP4 E2M1 (width 4, no infinity or NaN) in total order
  static constexpr WordDomainType WDT_E2M1 = WordDomainType::WDT_E2M1;

  /** \brief Construction-time choice of an inline value set
   *
   * A word of width at most eight can carry its exact set of values inline,
   * next to its cube and, for a bounded word, its interval. The set is the
   * exact domain: it can hold holes that the cube and interval cannot
   * express, such as those left by removing one inner value. The cube is
   * then the bit hull of the set, and the interval of a bounded word spans
   * the set's least and greatest value in the word's order. The choice is
   * independent of WordDomainType and immutable. WVS_DEFAULT, the default
   * of every constructor, gives a value set to E4M3FN and E2M1 words only;
   * WVS_NONE opts out.
   * \ingroup TaskModelWordVars
   */
  enum class WordValueSetOption {
    WVS_DEFAULT, ///< Value set exactly for WDT_E4M3FN and WDT_E2M1 words
    WVS_NONE,    ///< Domain is the cube and optional interval only
    WVS_EXACT    ///< Domain is an inline value set (width at most eight)
  };
  /// Value set exactly for WDT_E4M3FN and WDT_E2M1 words
  static constexpr WordValueSetOption WVS_DEFAULT =
    WordValueSetOption::WVS_DEFAULT;
  /// Domain is the cube and optional interval only
  static constexpr WordValueSetOption WVS_NONE = WordValueSetOption::WVS_NONE;
  /// Domain is an inline value set (width at most eight)
  static constexpr WordValueSetOption WVS_EXACT = WordValueSetOption::WVS_EXACT;

  /** \brief Numeric reading of a word's bit pattern
   *
   * An interpretation reads the bits either as an unsigned number or as a
   * two's-complement signed number. A cube word (WDT_CUBE) can be read
   * either way. A bounded word stores its interval in one order, so a
   * WDT_UNSIGNED word has interpretation WI_UNSIGNED and a WDT_SIGNED word
   * has interpretation WI_SIGNED. A float word (such as WDT_FP32) has no
   * integer interpretation.
   * \ingroup TaskModelWordVars
   */
  enum class WordInterpretation {
    WI_UNSIGNED, ///< Unsigned number
    WI_SIGNED    ///< Two's-complement signed number
  };
  /// Unsigned number
  static constexpr WordInterpretation WI_UNSIGNED =
    WordInterpretation::WI_UNSIGNED;
  /// Two's-complement signed number
  static constexpr WordInterpretation WI_SIGNED =
    WordInterpretation::WI_SIGNED;
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
   * interval in the word's order: unsigned, signed, or the IEEE 754 total
   * order of a float format. WordDomainType selects this representation at
   * construction. A word of width at most 8 can also carry an inline set of
   * its values (WordValueSetOption), which makes its domain exact. Signed
   * operations interpret bit patterns in two's complement, independently of
   * the domain representation, and integer operations act on the bits of a
   * float word.
   *
   * A word is created from its width, which allows every value, from known
   * bits (known_bits()), from a list of values, or from endpoints in the
   * order of its domain type: <tt>WordVar(home, 8, WDT_SIGNED, -3, 5)</tt>
   * holds -3 through 5, and <tt>WordVar(home, WDT_BF16, -1.0, 1.0)</tt> the
   * bfloat16 values from -1 to 1. A constant is a one-value list, as in
   * <tt>WordVar(home, 32, {0xffffffff})</tt>.
   *
   * A constant operand of a same-width constraint has the width of its word
   * variable operands. Width-changing constraints such as extract() and
   * concat() take the width of a constant explicitly, as it is independent of
   * every variable. Arithmetic follows the SMT-LIB fixed-width bit-vector
   * semantics: it wraps modulo \f$2^{width}\f$, and division, remainder,
   * and modulus are total. A zero divisor and the signed minimum divided by
   * -1 have defined results, documented at each posting.
   * Signed arithmetic wraps on overflow, so signed minimum divided by -1 is
   * signed minimum. The overflow predicates of WordOverflowType detect these
   * cases with the SMT-LIB definitions.
   *
   * Bounded domains can improve numeric propagation, but do not imply domain
   * consistency. Unsupported numeric cases retain bit propagation with the
   * same operation semantics. Each constraint documents any stronger
   * propagation guarantee. Repeated variables are supported unless a
   * declaration states otherwise.
   *
   * Propagators apply every bound move they deduce, at any width. As for
   * IntVar, propagation can therefore converge slowly on wide domains: on
   * 64-bit words, x < y together with y < x removes a few values per pass
   * before it fails. Models should avoid such cycles.
   *
   * WordVar provides mask and numeric-bound queries. WordRelType and
   * WordOpType select direct relations and logical operations. Models branch
   * with WordVarBranch and WordValBranch, and observe changes with WordTracer.
   * For expression syntax and domain policies, see \ref TaskModelMiniModelWord.
   * Propagator implementation notes are in \ref FuncWordProp.
   * A float domain type such as WDT_FP32 makes a word a floating-point
   * encoding. For floating-point operations on such float-typed words,
   * include gecode/float-word.hh and see \ref TaskModelWordFloat; to read
   * the bits of a word in another domain type, see reinterpret().
   *
   * \ingroup TaskModel
   */
  /**
   * \defgroup TaskModelWordVars Word-vector variables and arrays
   * \ingroup TaskModelWord
   */
  class WordValArgs;
  class WordBits;

  /**
   * \brief Return the bits set in \a mask, fixed to their values in \a value
   *
   * For example, <tt>known_bits(0b1001, 0b0001)</tt> fixes bit 0 to one and
   * bit 3 to zero. Throws Word::OutOfLimits if \a value has a bit outside
   * \a mask, which also rejects most calls with swapped arguments.
   * \ingroup TaskModelWordVars
   */
  WordBits known_bits(WordValue mask, WordValue value);

  /**
   * \brief Bits of a word fixed to known values
   *
   * Each bit set in mask() is fixed to the corresponding bit of value().
   * Only known_bits() makes a WordBits, so a braced list such as
   * <tt>{0, 1}</tt> passed to a constructor is always a list of values.
   * \ingroup TaskModelWordVars
   */
  class WordBits {
  public:
    /// Return the mask of the fixed bits
    WordValue mask(void) const;
    /// Return the values of the fixed bits, which lie within mask()
    WordValue value(void) const;
  private:
    /// Construct without fixed bits
    WordBits(void);
    /// Mask of the fixed bits
    WordValue _mask;
    /// Values of the fixed bits
    WordValue _value;
    friend WordBits known_bits(WordValue mask, WordValue value);
  };

  /**
   * \brief Fixed-width word variable
   *
   * Widths from 1 through 64 are supported. The domain is a cube, optionally
   * intersected with one interval in the order of its WordDomainType:
   * unsigned, signed, or the total order of a float format, whose width
   * follows from the format. The lo() and hi() queries return known-one and
   * may-be-one masks, not numeric endpoints. Use minimum() and maximum() for
   * the endpoints of a bounded domain.
   * A word of width at most 8 with a value set (WVS_EXACT, or WVS_DEFAULT
   * for E4M3FN and E2M1) carries it as its exact domain; in() is then exact,
   * and lo(), hi(), minimum(), and maximum() describe the hull and extremes
   * of that set.
   *
   * The constructors take the width with a value-set option and, optionally:
   * known bits; a list of values; a domain type; or a domain type with
   * minimum and maximum endpoints, which may be combined with known bits.
   * Endpoints are values in the order of the domain type, so negative values
   * are written as such for WDT_SIGNED. A float format alone gives the width,
   * and its endpoints are then floating-point values. WordVarArgs and
   * WordVarArray have the same forms after the number of words.
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
    /** \brief Construct an unconstrained cube word of the given \a width
     *
     * Throws Word::OutOfLimits if \a width is not between 1 and 64, or if
     * \a value_set is WVS_EXACT and \a width is greater than 8.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct a cube word with \a known bits
     *
     * For example, <tt>WordVar(home, 4, known_bits(0b1001, 0b0001))</tt>
     * has bit 0 set and bit 3 clear, that is, the values 1, 3, 5, and 7.
     * Throws Word::OutOfLimits if \a width is not between 1 and 64,
     * \a known fixes bits outside \a width, or \a value_set is WVS_EXACT
     * and \a width is greater than 8.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordBits known,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct a word with exactly the values in \a values
     *
     * The word has the full domain of \a domain_type, restricted as by
     * dom(Home, WordVar, const WordValArgs&): a word with a value set holds
     * exactly the listed values, and another word holds their hulls plus a
     * propagator that rejects every unlisted value in them. Entries may
     * repeat and follow the endpoint rule of the bounded constructor, so
     * <tt>{WordValue(-3), 5}</tt> lists -3 and 5. A single entry gives a
     * constant, as in <tt>WordVar(home, 32, {0xffffffff})</tt>.
     *
     * Throws Word::OutOfLimits if \a width or \a domain_type is invalid,
     * including a width that differs from that of a float \a domain_type,
     * an entry does not fit \a width, or \a value_set is WVS_EXACT and
     * \a width is greater than 8. Throws Word::VariableEmptyDomain if
     * \a values is empty.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              const WordValArgs& values,
                              WordDomainType domain_type=WDT_CUBE,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct a word using the full range of \a domain_type
     *
     * Throws Word::OutOfLimits if \a width is not between 1 and 64, differs
     * from the width of a float \a domain_type, or \a value_set is
     * WVS_EXACT and \a width is greater than 8.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordDomainType domain_type,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct a word using the full range of float \a format
     *
     * The width is the width of the format. Throws Word::OutOfLimits if
     * \a format is not a float type, or \a value_set is WVS_EXACT and the
     * format is wider than 8.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, WordDomainType format,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /**
     * \brief Construct a word of float \a format with the values
     * \a minimum through \a maximum
     *
     * The endpoints are encoded exactly in \a format and ordered by its
     * total order, in which -0.0 lies below +0.0. Integer arguments are
     * values too: <tt>WordVar(home, WDT_FP32, 1, 2)</tt> holds 1.0 through
     * 2.0. The width is the width of the format. A NaN endpoint needs its
     * bit pattern, given to the bounded constructor with a width.
     *
     * Throws Word::OutOfLimits if \a format is not a float type, an
     * endpoint is a NaN or not exactly representable in \a format, or
     * \a value_set is WVS_EXACT and the format is wider than 8. Throws
     * Word::VariableEmptyDomain if \a minimum lies above \a maximum.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, WordDomainType format,
                              double minimum, double maximum,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /**
     * \brief Construct a bounded word with the endpoints \a minimum and
     * \a maximum
     *
     * An endpoint is accepted when it fits \a width as an unsigned number
     * or, read as a 64-bit two's-complement number, as a signed number. It
     * is taken modulo \f$2^{width}\f$. For an unsigned or signed word the
     * endpoints are thus numbers in the word's interpretation:
     * <tt>WordVar(home, 8, WDT_SIGNED, -3, 5)</tt> holds -3 through 5, and
     * 0xfd is the same endpoint as -3. For a float \a domain_type they are
     * bit patterns, which can also name a NaN; the constructor for a float
     * format without a width takes values instead.
     *
     * Throws Word::OutOfLimits if \a width or \a domain_type is invalid,
     * including a width that differs from that of a float \a domain_type,
     * an endpoint does not fit \a width, or \a value_set is WVS_EXACT and
     * \a width is greater than 8. Throws Word::VariableEmptyDomain if
     * \a minimum lies above \a maximum in the order of \a domain_type.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordDomainType domain_type,
                              WordValue minimum, WordValue maximum,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /**
     * \brief Construct a bounded word with the endpoints \a minimum and
     * \a maximum and with \a known bits
     *
     * The endpoints follow the rule of the bounded constructor without
     * known bits. For example,
     * <tt>WordVar(home, 4, WDT_UNSIGNED, 6, 10, known_bits(0b0001, 0))</tt>
     * holds the even values 6, 8, and 10.
     *
     * Throws Word::OutOfLimits as the bounded constructor does, and if
     * \a known fixes bits outside \a width. Throws
     * Word::VariableEmptyDomain if no value between the endpoints has the
     * known bits.
     */
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordDomainType domain_type,
                              WordValue minimum, WordValue maximum,
                              WordBits known,
                              WordValueSetOption value_set=WVS_DEFAULT);
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVar(Space& home, unsigned int width, WordDomainType domain_type,
            T minimum, U maximum,
            WordValueSetOption value_set=WVS_DEFAULT) = delete;
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVar(Space& home, unsigned int width, WordDomainType domain_type,
            T minimum, U maximum, WordBits known,
            WordValueSetOption value_set=WVS_DEFAULT) = delete;
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
    /// Test whether the word has an interval (any type but WDT_CUBE)
    bool bounded(void) const;
    /// Test whether the domain is an inline value set (see WordValueSetOption)
    bool has_value_set(void) const;
    /**
     * \brief Return the canonical minimum endpoint bit pattern
     *
     * For signed words, the result is the two's-complement encoding of the
     * minimum value, and for float words the encoding that is least in the
     * total order, not its internal rank.
     * Throws Word::BoundsOfCubeVar if this is a cube-only word.
     */
    WordValue minimum(void) const;
    /**
     * \brief Return the canonical maximum endpoint bit pattern
     *
     * For signed words, the result is the two's-complement encoding of the
     * maximum value, and for float words the encoding that is greatest in
     * the total order, not its internal rank.
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
    void _init(Space& home, unsigned int width, WordValue lo, WordValue hi,
               WordValueSetOption value_set);
    void _init(Space& home, const Word::PreparedWordDomain& domain,
               WordValueSetOption value_set);
    void _init(Space& home, const Word::PreparedWordDomain& domain,
               WordValueSetOption value_set, const WordValArgs& values);
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
    /** \brief Construct \a n unconstrained cube words
     *
     * Throws Word::OutOfLimits for a negative count, an invalid width, or
     * WVS_EXACT with a width greater than 8.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n cube words with \a known bits
     *
     * Throws Word::OutOfLimits for a negative count, an invalid width,
     * known bits outside \a width, or WVS_EXACT with a width greater
     * than 8.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordBits known,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n words with exactly the values in \a values
     *
     * Each word is restricted as by the corresponding WordVar constructor.
     * Throws Word::OutOfLimits for a negative count, an invalid width or
     * domain kind, a width that differs from that of a float
     * \a domain_type, an entry that does not fit \a width, or WVS_EXACT
     * with a width greater than 8, and Word::VariableEmptyDomain if
     * \a values is empty.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  const WordValArgs& values,
                                  WordDomainType domain_type=WDT_CUBE,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or
     * domain kind, a width that differs from that of a float
     * \a domain_type, or WVS_EXACT with a width greater than 8.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n full-domain words of float \a format
     *
     * The width is the width of the format. Throws Word::OutOfLimits for a
     * negative count, a \a format that is not a float type, or WVS_EXACT
     * with a format wider than 8.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, WordDomainType format,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n words of float \a format with the values
     * \a minimum through \a maximum
     *
     * The endpoints are values, as for the corresponding WordVar
     * constructor. Throws Word::OutOfLimits for a negative count, a
     * \a format that is not a float type, an endpoint that is a NaN or not
     * exact in \a format, or WVS_EXACT with a format wider than 8, and
     * Word::VariableEmptyDomain if \a minimum lies above \a maximum.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, WordDomainType format,
                                  double minimum, double maximum,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n bounded words with the endpoints \a minimum
     * and \a maximum
     *
     * The endpoints follow the rule of the corresponding WordVar
     * constructor: numbers in the word's interpretation, taken modulo
     * \f$2^{width}\f$, or bit patterns for a float \a domain_type. Throws
     * Word::OutOfLimits for a negative count, an invalid width or domain
     * kind, a width that differs from that of a float \a domain_type, an
     * endpoint that does not fit \a width, or WVS_EXACT with a width
     * greater than 8, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n bounded words with the endpoints \a minimum
     * and \a maximum and with \a known bits
     *
     * Throws Word::OutOfLimits as the bounded form does, and for known bits
     * outside \a width. Throws Word::VariableEmptyDomain if no value
     * between the endpoints has the known bits.
     */
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum,
                                  WordBits known,
                                  WordValueSetOption value_set=WVS_DEFAULT);
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVarArgs(Space& home, int n, unsigned int width,
               WordDomainType domain_type, T minimum, U maximum,
               WordValueSetOption value_set=WVS_DEFAULT) = delete;
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVarArgs(Space& home, int n, unsigned int width,
               WordDomainType domain_type, T minimum, U maximum,
               WordBits known,
               WordValueSetOption value_set=WVS_DEFAULT) = delete;
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
    /** \brief Construct \a n unconstrained cube words
     *
     * Throws Word::OutOfLimits for a negative count, an invalid width, or
     * WVS_EXACT with a width greater than 8.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n cube words with \a known bits
     *
     * Throws Word::OutOfLimits for a negative count, an invalid width,
     * known bits outside \a width, or WVS_EXACT with a width greater
     * than 8.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordBits known,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n words with exactly the values in \a values
     *
     * Each word is restricted as by the corresponding WordVar constructor.
     * Throws Word::OutOfLimits for a negative count, an invalid width or
     * domain kind, a width that differs from that of a float
     * \a domain_type, an entry that does not fit \a width, or WVS_EXACT
     * with a width greater than 8, and Word::VariableEmptyDomain if
     * \a values is empty.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   const WordValArgs& values,
                                   WordDomainType domain_type=WDT_CUBE,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n full-domain words
     *
     * Throws Word::OutOfLimits for a negative count, invalid width or
     * domain kind, a width that differs from that of a float
     * \a domain_type, or WVS_EXACT with a width greater than 8.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n full-domain words of float \a format
     *
     * The width is the width of the format. Throws Word::OutOfLimits for a
     * negative count, a \a format that is not a float type, or WVS_EXACT
     * with a format wider than 8.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, WordDomainType format,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n words of float \a format with the values
     * \a minimum through \a maximum
     *
     * The endpoints are values, as for the corresponding WordVar
     * constructor. Throws Word::OutOfLimits for a negative count, a
     * \a format that is not a float type, an endpoint that is a NaN or not
     * exact in \a format, or WVS_EXACT with a format wider than 8, and
     * Word::VariableEmptyDomain if \a minimum lies above \a maximum.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, WordDomainType format,
                                   double minimum, double maximum,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n bounded words with the endpoints \a minimum
     * and \a maximum
     *
     * The endpoints follow the rule of the corresponding WordVar
     * constructor: numbers in the word's interpretation, taken modulo
     * \f$2^{width}\f$, or bit patterns for a float \a domain_type. Throws
     * Word::OutOfLimits for a negative count, an invalid width or domain
     * kind, a width that differs from that of a float \a domain_type, an
     * endpoint that does not fit \a width, or WVS_EXACT with a width
     * greater than 8, and Word::VariableEmptyDomain for an empty interval.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type,
                                   WordValue minimum, WordValue maximum,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /** \brief Construct \a n bounded words with the endpoints \a minimum
     * and \a maximum and with \a known bits
     *
     * Throws Word::OutOfLimits as the bounded form does, and for known bits
     * outside \a width. Throws Word::VariableEmptyDomain if no value
     * between the endpoints has the known bits.
     */
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordDomainType domain_type,
                                   WordValue minimum, WordValue maximum,
                                   WordBits known,
                                   WordValueSetOption value_set=WVS_DEFAULT);
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVarArray(Space& home, int n, unsigned int width,
                WordDomainType domain_type, T minimum, U maximum,
                WordValueSetOption value_set=WVS_DEFAULT) = delete;
    /// Floating-point endpoints need the constructor for a float format
    template<class T, class U,
             std::enable_if_t<std::is_floating_point_v<T> ||
                              std::is_floating_point_v<U>, int> = 0>
    WordVarArray(Space& home, int n, unsigned int width,
                WordDomainType domain_type, T minimum, U maximum,
                WordBits known,
                WordValueSetOption value_set=WVS_DEFAULT) = delete;
    WordVarArray& operator=(const WordVarArray&) = default;
  };

  /**
   * \brief Restrict \a x to the word cube described by \a lo and \a hi
   *
   * Throws Word::OutOfLimits if \a lo or \a hi has bits outside the width of
   * \a x. An empty cube within the width fails \a home.
   */
  GECODE_WORD_EXPORT void dom(Home home, WordVar x,
                              WordValue lo, WordValue hi);
  /**
   * \brief Assign \a x to \a value
   *
   * Throws Word::OutOfLimits if \a value has bits outside the width of \a x.
   */
  GECODE_WORD_EXPORT void dom(Home home, WordVar x, WordValue value);
  /**
   * \brief Restrict \a x to the values in \a values
   *
   * The list may hold duplicates in any order. A word with a value set
   * keeps exactly its values that are listed, and the constraint needs no
   * propagator. Another word keeps the cube hull and, when bounded, the
   * ranked interval hull of its listed values, as extensional() does:
   * unlisted values inside those hulls may remain during propagation, but
   * assigning one fails. An empty list or an empty intersection fails
   * \a home.
   *
   * Throws Word::OutOfLimits if a value has bits outside the width of \a x.
   */
  GECODE_WORD_EXPORT void dom(Home home, WordVar x,
                              const WordValArgs& values);
  /**
   * \brief Fix the \a known bits of \a x
   *
   * Throws Word::OutOfLimits if \a known fixes bits outside the width of
   * \a x. A known bit that \a x cannot take fails \a home.
   */
  GECODE_WORD_EXPORT void dom(Home home, WordVar x, WordBits known);
  /**
   * \brief Fix the \a known bits of every word in \a x
   *
   * Throws Word::OutOfLimits if \a known fixes bits outside the width of a
   * word in \a x. A known bit that a word cannot take fails \a home.
   */
  GECODE_WORD_EXPORT void dom(Home home, const WordVarArgs& x, WordBits known);

  /** @} */

  /**
   * \defgroup TaskModelWordExtensional Extensional constraints
   * \ingroup TaskModelWord
   */
  /** @{ */

  /**
   * \brief Tuple set of encoded word values
   *
   * Each column has its own width (1..64). Construct the set with its
   * column widths, add() rows, and finalize() it before posting. Finalizing
   * sorts and deduplicates the rows and builds the support indexes; after
   * that the set is immutable, and copies and posted constraints share it.
   * An empty row set denotes false, including for zero columns, where
   * adding one empty row denotes true.
   */
  class GECODE_WORD_EXPORT WordTupleSet : public SharedHandle {
  public:
    class Data;
    /** \brief Initialize an empty tuple set with column \a widths
     *
     * Throws Word::OutOfLimits if a width is not between 1 and 64 or there
     * are too many columns.
     */
    explicit WordTupleSet(const IntArgs& widths);
    /** \brief Add row \a row
     *
     * Throws Word::OutOfLimits if the length of \a row differs from the
     * arity, a value does not fit its column width, or there are too many
     * rows, and Word::AlreadyFinalized if the set is finalized.
     */
    WordTupleSet& add(const WordValArgs& row);
    /// Finalize the set; further calls have no effect
    void finalize(void);
    /// Test whether the set is finalized
    bool finalized(void) const;
    /// Return number of columns
    int arity(void) const;
    /// Return number of rows (distinct once finalized)
    int tuples(void) const;
    /// Return width of \a column
    unsigned int width(int column) const;
    /// Return value of \a column in \a row (canonical order once finalized)
    WordValue value(int row, int column) const;
    /// Internal row storage and support indexes
    const Data& data(void) const;
  };

  /**
   * \brief Constrain words to a row of \a tuples
   *
   * Maintains the exact cube hull and, for bounded domains, the exact
   * signed or unsigned interval hull of compatible tuples. Supports mixed
   * widths, mixed domain types, and repeated variables. Interior values
   * that cannot be removed by the domain representation may remain.
   * Throws Word::NotYetFinalized if \a tuples is not finalized,
   * Word::OutOfLimits for arity mismatch, and Word::WidthMismatch for
   * column width mismatch.
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
   * \a y is the number that \a interpretation reads from the bits of \a x.
   * A cube word can be channelled with either interpretation. A bounded word
   * must use the interpretation of its construction-time domain type:
   * WI_UNSIGNED for WDT_UNSIGNED and WI_SIGNED for WDT_SIGNED; another
   * interpretation, and every float word, throws Word::OutOfLimits. The
   * channel is exact for assigned values and otherwise bounds consistent.
   */
  GECODE_WORD_EXPORT void channel(Home home, WordVar x, IntVar y,
                                  WordInterpretation interpretation);
  /** \brief Channel the unsigned encoding of word \a x to integer \a y
   *
   * Widths from 1 through 8 are supported with any Word domain type.
   * The channel retains exactly the encodings common to both domains in
   * \a y. A word with a value set also retains exactly those encodings, so
   * holes pass in both directions, including changes that remove values
   * without changing cube or bounds. Another word receives their bit hull
   * and, when bounded, their endpoint hull, and interior holes remain in
   * \a y only. Signed Word storage still maps to unsigned encoding IDs, not
   * signed integer values.
   *
   * Without a value set, adjacent constraints must share the same IntVar to
   * communicate encoding holes through this channel: separate IntVars linked
   * only through such a WordVar can lose that information.
   * Throws Word::OutOfLimits if the word is wider than eight bits.
   */
  GECODE_WORD_EXPORT void channel_encoding(Home home, WordVar x, IntVar y);
  /** \brief Return a new word with the bits of \a x, read in \a domain_type
   *
   * The result has the width of \a x, and \a domain_type decides the order
   * of its interval and branching, as for any word. The two words stay
   * linked in both directions: they are equal as bit patterns. The result
   * has a value set exactly when \a x has one. With value sets, both words
   * keep exactly their common values, so holes pass in both directions,
   * including changes that remove values without changing cube or bounds.
   * Without value sets, the link passes known bits in both directions, and
   * narrows each word's interval to the hull, in its own order, of the
   * encodings in the other word's interval. An assigned \a x assigns
   * the result and posts no propagator, and a failed \a home returns an
   * unlinked word. \a domain_type may equal the domain type of \a x, which
   * links two equal words.
   *
   * Throws Word::OutOfLimits, as the WordVar constructors do, if
   * \a domain_type is not a WordDomainType or is a float format whose
   * width differs from that of \a x. Nothing is posted then.
   */
  GECODE_WORD_EXPORT WordVar reinterpret(Home home, WordVar x,
                                         WordDomainType domain_type);
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
  enum class WordRelType {
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
  /// Equality
  static constexpr WordRelType WRT_EQ = WordRelType::WRT_EQ;
  /// Disequality
  static constexpr WordRelType WRT_NQ = WordRelType::WRT_NQ;
  /// Unsigned less than or equal
  static constexpr WordRelType WRT_ULQ = WordRelType::WRT_ULQ;
  /// Unsigned less than
  static constexpr WordRelType WRT_ULE = WordRelType::WRT_ULE;
  /// Unsigned greater than or equal
  static constexpr WordRelType WRT_UGQ = WordRelType::WRT_UGQ;
  /// Unsigned greater than
  static constexpr WordRelType WRT_UGR = WordRelType::WRT_UGR;
  /// Signed less than or equal
  static constexpr WordRelType WRT_SLQ = WordRelType::WRT_SLQ;
  /// Signed less than
  static constexpr WordRelType WRT_SLE = WordRelType::WRT_SLE;
  /// Signed greater than or equal
  static constexpr WordRelType WRT_SGQ = WordRelType::WRT_SGQ;
  /// Signed greater than
  static constexpr WordRelType WRT_SGR = WordRelType::WRT_SGR;

  /** \brief Word logical operation type
   *  \ingroup TaskModelWordLogic
   */
  enum class WordOpType {
    WOT_AND,  ///< Bitwise conjunction
    WOT_OR,   ///< Bitwise disjunction
    WOT_XOR,  ///< Bitwise exclusive disjunction
    WOT_NAND, ///< Complement of bitwise conjunction
    WOT_NOR,  ///< Complement of bitwise disjunction
    WOT_XNOR  ///< Complement of bitwise exclusive disjunction
  };
  /// Bitwise conjunction
  static constexpr WordOpType WOT_AND = WordOpType::WOT_AND;
  /// Bitwise disjunction
  static constexpr WordOpType WOT_OR = WordOpType::WOT_OR;
  /// Bitwise exclusive disjunction
  static constexpr WordOpType WOT_XOR = WordOpType::WOT_XOR;
  /// Complement of bitwise conjunction
  static constexpr WordOpType WOT_NAND = WordOpType::WOT_NAND;
  /// Complement of bitwise disjunction
  static constexpr WordOpType WOT_NOR = WordOpType::WOT_NOR;
  /// Complement of bitwise exclusive disjunction
  static constexpr WordOpType WOT_XNOR = WordOpType::WOT_XNOR;

  /// Word arithmetic overflow operation type
  enum class WordOverflowType {
    WOF_NEG_SIGNED,  ///< Signed negation overflow (SMT-LIB bvnego)
    WOF_ADD_UNSIGNED,///< Unsigned addition overflow (SMT-LIB bvuaddo)
    WOF_ADD_SIGNED,  ///< Signed addition overflow (SMT-LIB bvsaddo)
    WOF_MULT_UNSIGNED,///< Unsigned multiplication overflow (SMT-LIB bvumulo)
    WOF_MULT_SIGNED, ///< Signed multiplication overflow (SMT-LIB bvsmulo)
    WOF_DIV_SIGNED   ///< Signed division overflow (QF_BV bvsdivo extension)
  };
  /// Signed negation overflow (SMT-LIB bvnego)
  static constexpr WordOverflowType WOF_NEG_SIGNED =
    WordOverflowType::WOF_NEG_SIGNED;
  /// Unsigned addition overflow (SMT-LIB bvuaddo)
  static constexpr WordOverflowType WOF_ADD_UNSIGNED =
    WordOverflowType::WOF_ADD_UNSIGNED;
  /// Signed addition overflow (SMT-LIB bvsaddo)
  static constexpr WordOverflowType WOF_ADD_SIGNED =
    WordOverflowType::WOF_ADD_SIGNED;
  /// Unsigned multiplication overflow (SMT-LIB bvumulo)
  static constexpr WordOverflowType WOF_MULT_UNSIGNED =
    WordOverflowType::WOF_MULT_UNSIGNED;
  /// Signed multiplication overflow (SMT-LIB bvsmulo)
  static constexpr WordOverflowType WOF_MULT_SIGNED =
    WordOverflowType::WOF_MULT_SIGNED;
  /// Signed division overflow (QF_BV bvsdivo extension)
  static constexpr WordOverflowType WOF_DIV_SIGNED =
    WordOverflowType::WOF_DIV_SIGNED;

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
   * IPL_DEF and IPL_VAL provide value consistency. IPL_BND provides bounds
   * consistency over numeric intervals when all variables have the same
   * bounded interpretation; other cases fall back to value consistency.
   * IPL_DOM also provides value consistency.
   * Throws Word::ArgumentSame for duplicate variables and
   * Word::WidthMismatch if the widths differ.
   */
  GECODE_WORD_EXPORT void distinct(Home home, const WordVarArgs& x,
                                   IntPropLevel ipl=IPL_DEF);
  /// Post the relation \a wrt between \a x and \a y, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y, Reify r);
  /**
   * \brief Post the relation \a wrt between \a x and a constant of the width
   * of \a x
   *
   * Disequality removes \a value exactly from a word with a value set.
   * Without a value set, it removes \a value only once at most one unknown
   * bit remains.
   */
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordValue value);
  /// Post the relation \a wrt to a constant of the width of \a x, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordValue value, Reify r);
  /** @} */

  /**
   * \defgroup TaskModelWordLogic Word logical constraints
   * \ingroup TaskModelWord
   */
  /** @{ */
  /// Post bitwise complement \a y = ~\a x
  GECODE_WORD_EXPORT void complement(Home home, WordVar x, WordVar y);
  /// Post complement \a y = ~\a value of a constant of the width of \a y
  GECODE_WORD_EXPORT void complement(Home home, WordValue value, WordVar y);
  /// Post complement ~\a x equal to a constant of the width of \a x
  GECODE_WORD_EXPORT void complement(Home home, WordVar x, WordValue value);
  /// Post the binary logical operation \a z = \a x \a wot \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, WordVar z);
  /// Post \a z = \a x \a wot \a value for a constant of the width of \a x
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordValue value, WordVar z);
  /// Post \a x \a wot \a y = \a value for a constant of the width of \a x
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, WordValue value);
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
  /// Post whole-word conditional with a then constant of the width of \a result
  GECODE_WORD_EXPORT void ite(Home home, BoolVar control,
                              WordValue then_value, WordVar else_word,
                              WordVar result);
  /// Post whole-word conditional with an else constant of the width of \a result
  GECODE_WORD_EXPORT void ite(Home home, BoolVar control,
                              WordVar then_word, WordValue else_value,
                              WordVar result);
  /**
   * \brief Post bitwise conditional controlled by the mask \a control
   *
   * Each result bit is selected independently from the corresponding bit of
   * \a then_word or \a else_word.
   */
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              WordVar then_word, WordVar else_word,
                              WordVar result);
  /// Post bitwise conditional with a then constant of the width of \a result
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              WordValue then_value, WordVar else_word,
                              WordVar result);
  /// Post bitwise conditional with an else constant of the width of \a result
  GECODE_WORD_EXPORT void ite(Home home, WordVar control,
                              WordVar then_word, WordValue else_value,
                              WordVar result);
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
  /// Logically shift a constant of the width of \a result left
  GECODE_WORD_EXPORT void shift_left(Home home, WordValue value,
                                     unsigned int amount, WordVar result);
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
  /// Logically shift a constant of the width of \a result right
  GECODE_WORD_EXPORT void logical_shift_right(Home home, WordValue value,
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
  /// Arithmetically shift a constant of the width of \a result right
  GECODE_WORD_EXPORT void arithmetic_shift_right(Home home, WordValue value,
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
  /// Rotate a constant of the width of \a result left
  GECODE_WORD_EXPORT void rotate_left(Home home, WordValue value,
                                      unsigned int amount, WordVar result);
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
  /// Rotate a constant of the width of \a result right
  GECODE_WORD_EXPORT void rotate_right(Home home, WordValue value,
                                       unsigned int amount, WordVar result);
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
  /// Post modular addition with a constant operand of the width of \a x
  GECODE_WORD_EXPORT void add(Home home, WordVar x, WordValue value,
                              WordVar result);
  /// Post two's-complement modular negation \a result = -\a x
  GECODE_WORD_EXPORT void neg(Home home, WordVar x, WordVar result);
  /// Post modular negation of a constant of the width of \a result
  GECODE_WORD_EXPORT void neg(Home home, WordValue value, WordVar result);
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
  /// Post modular subtraction with a right constant of the width of \a x
  GECODE_WORD_EXPORT void sub(Home home, WordVar x, WordValue value,
                              WordVar result);
  /// Post modular subtraction with a left constant of the width of \a y
  GECODE_WORD_EXPORT void sub(Home home, WordValue value, WordVar y,
                              WordVar result);
  /// Post modular multiplication \a result = \a x * \a y
  GECODE_WORD_EXPORT void mult(Home home, WordVar x, WordVar y,
                               WordVar result);
  /// Post modular multiplication with a constant operand of the width of \a x
  GECODE_WORD_EXPORT void mult(Home home, WordVar x, WordValue value,
                               WordVar result);
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
                                   WordOverflowType wot, BoolVar b);
  /// Post binary arithmetic overflow predicate \a wot for \a x and \a y
  GECODE_WORD_EXPORT void overflow(Home home, WordVar x,
                                   WordOverflowType wot, WordVar y, BoolVar b);
  /** \brief Post unsigned division \a result = \a x div \a y
   *
   * Division by zero returns the all-one word.
   */
  GECODE_WORD_EXPORT void div(Home home, WordVar x, WordVar y,
                              WordVar result);
  /// Post unsigned division with a right constant of the width of \a x
  GECODE_WORD_EXPORT void div(Home home, WordVar x, WordValue value,
                              WordVar result);
  /// Post unsigned division with a left constant of the width of \a y
  GECODE_WORD_EXPORT void div(Home home, WordValue value, WordVar y,
                              WordVar result);
  /** \brief Post unsigned remainder \a result = \a x mod \a y
   *
   * Remainder by zero returns the dividend.
   */
  GECODE_WORD_EXPORT void mod(Home home, WordVar x, WordVar y,
                              WordVar result);
  /// Post unsigned remainder with a right constant of the width of \a x
  GECODE_WORD_EXPORT void mod(Home home, WordVar x, WordValue value,
                              WordVar result);
  /// Post unsigned remainder with a left constant of the width of \a y
  GECODE_WORD_EXPORT void mod(Home home, WordValue value, WordVar y,
                              WordVar result);
  /** \brief Post unsigned division and remainder together
   *
   * Uses the semantics of div() and mod(), including a zero divisor.
   * All operands have the same width and may alias each other.
   */
  GECODE_WORD_EXPORT void divmod(Home home, WordVar dividend,
                                 WordVar divisor, WordVar quotient,
                                 WordVar remainder);
  /** \brief Post signed division \a result = \a x signed_div \a y
   *
   * Inputs and result use two's-complement values, with division rounded
   * toward zero. Division by zero returns one for a negative dividend and the
   * all-one word otherwise. Signed minimum divided by -1 returns signed
   * minimum.
   */
  GECODE_WORD_EXPORT void signed_div(Home home, WordVar x, WordVar y,
                                     WordVar result);
  /// Post signed division with a right constant of the width of \a x
  GECODE_WORD_EXPORT void signed_div(Home home, WordVar x, WordValue value,
                                     WordVar result);
  /// Post signed division with a left constant of the width of \a y
  GECODE_WORD_EXPORT void signed_div(Home home, WordValue value, WordVar y,
                                     WordVar result);
  /** \brief Post signed remainder \a result = \a x signed_rem \a y
   *
   * The nonzero remainder has the dividend's sign. Remainder by zero returns
   * the dividend; signed minimum remainder -1 is zero.
   */
  GECODE_WORD_EXPORT void signed_rem(Home home, WordVar x, WordVar y,
                                     WordVar result);
  /// Post signed remainder with a right constant of the width of \a x
  GECODE_WORD_EXPORT void signed_rem(Home home, WordVar x, WordValue value,
                                     WordVar result);
  /// Post signed remainder with a left constant of the width of \a y
  GECODE_WORD_EXPORT void signed_rem(Home home, WordValue value, WordVar y,
                                     WordVar result);
  /** \brief Post signed modulus \a result = \a x signed_mod \a y
   *
   * The nonzero modulus result has the divisor's sign. Modulus by
   * zero returns the dividend; signed minimum modulus -1 is zero.
   */
  GECODE_WORD_EXPORT void signed_mod(Home home, WordVar x, WordVar y,
                                     WordVar result);
  /// Post signed modulus with a right constant of the width of \a x
  GECODE_WORD_EXPORT void signed_mod(Home home, WordVar x, WordValue value,
                                     WordVar result);
  /// Post signed modulus with a left constant of the width of \a y
  GECODE_WORD_EXPORT void signed_mod(Home home, WordValue value, WordVar y,
                                     WordVar result);

  /// One row in a joint exact-integer linear Word relation
  class WordLinearRow {
  public:
    /// Kind of linear relation represented by the row
    enum class Type {
      EQUAL,     ///< Linear equality
      RANGE,     ///< Linear relation between a lower and an upper bound
      CONGRUENCE ///< Linear congruence modulo a fixed modulus
    };
    /// Linear equality
    static constexpr Type EQUAL = Type::EQUAL;
    /// Linear relation between a lower and an upper bound
    static constexpr Type RANGE = Type::RANGE;
    /// Linear congruence modulo a fixed modulus
    static constexpr Type CONGRUENCE = Type::CONGRUENCE;
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
   * use unsigned values. Assigned tuples are checked exactly. Propagation
   * does not guarantee bounds or domain consistency, and all values of
   * \a ipl select the same propagation.
   */
  GECODE_WORD_EXPORT void
  product_balance(Home home, const WordVarArgs& x, WordValue c,
                  const WordVarArgs& y, WordValue d,
                  IntPropLevel ipl=IPL_DEF);
  /** \brief Enforce product(x)=d*q+r, d>0 and 0<=r<d, without intermediate
   * wrapping
   *
   * Propagation does not guarantee bounds or domain consistency, and all
   * values of \a ipl select the same propagation.
   */
  GECODE_WORD_EXPORT void
  product_divmod(Home home, const WordVarArgs& x, WordVar d, WordVar q,
                 WordVar r, IntPropLevel ipl=IPL_DEF);
  /** \brief Enforce c+sum(a[i]*x[i])=d*q+r, d>0 and 0<=r<d
   *
   * Each propagation filters the bounds of all words by interval reasoning
   * on the identity, and the weighted sum as a row of linear_system().
   * IPL_ADVANCED adds the extra subset-sum filtering of linear_system() for
   * that row.
   */
  GECODE_WORD_EXPORT void
  linear_divmod(Home home, const IntArgs& a, const WordVarArgs& x, int c,
                WordVar d, WordVar q, WordVar r,
                IntPropLevel ipl=IPL_DEF);
  /** \brief Constrain y to the least value not below x congruent to phase
   * modulo spacing
   *
   * Propagation follows linear_system() for the rows x<=y<=x+spacing-1 and
   * y=phase (mod spacing), including the extra filtering of IPL_ADVANCED.
   */
  GECODE_WORD_EXPORT void
  quantize_up(Home home, WordVar x, WordValue spacing, WordValue phase,
              WordVar y, IntPropLevel ipl=IPL_DEF);
  /** \brief Enforce a joint system of exact integer equalities, ranges, and
   * congruences
   *
   * Each propagation filters every row at every domain size: an equality or
   * range row by interval reasoning on the bounds of its words, and an
   * equality or congruence row by divisibility and the known low bits of its
   * words. Assigned tuples are checked exactly. Propagation does not
   * guarantee bounds or domain consistency. As extra work on top, posting
   * combines a bounded number of pairs of equality rows that share a word
   * and fails on a contradictory pair. IPL_ADVANCED adds a second stage of
   * extra work: subset-sum filtering of each row whose unknown bits have a
   * small total normalized weight, and filtering with a bounded number of
   * rows derived from the combined pairs.
   */
  GECODE_WORD_EXPORT void
  linear_system(Home home, const WordVarArgs& x,
                const WordLinearRowArgs& rows,
                IntPropLevel ipl=IPL_DEF);
  /** \brief Relate fixed-radix digits, last digit fastest, to their exact
   * integer rank
   *
   * Propagation follows linear_system() for the single rank row, including
   * the extra filtering of IPL_ADVANCED. It also bounds every digit below its
   * radix and links the bits of the fastest digits to the rank while their
   * radices are powers of two.
   */
  GECODE_WORD_EXPORT void
  mixed_radix(Home home, const WordVarArgs& digits,
              const WordValArgs& radices, WordVar rank,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Enforce outputs=A*coordinates+offset with explicit coordinates
   *
   * Matrix \a a is row-major with one row per output and one column per
   * coordinate. Propagation follows linear_system() for one equality row
   * per output, including the extra work of posting and IPL_ADVANCED.
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
    /// Variable selection strategy
    enum class Select {
      SEL_NONE = 0,        ///< First unassigned
      SEL_RND,             ///< Random (uniform, for tie breaking)
      SEL_MERIT_MIN,       ///< With least merit
      SEL_MERIT_MAX,       ///< With highest merit
      SEL_DEGREE_MIN,      ///< With smallest degree
      SEL_DEGREE_MAX,      ///< With largest degree
      SEL_AFC_MIN,         ///< With smallest accumulated failure count
      SEL_AFC_MAX,         ///< With largest accumulated failure count
      SEL_ACTION_MIN,      ///< With lowest action
      SEL_ACTION_MAX,      ///< With highest action
      SEL_CHB_MIN,         ///< With lowest CHB Q-score
      SEL_CHB_MAX,         ///< With highest CHB Q-score
      SEL_SIZE_MIN,        ///< With fewest unknown bits
      SEL_SIZE_MAX,        ///< With most unknown bits
      SEL_DEGREE_SIZE_MIN, ///< With smallest degree per unknown bit
      SEL_DEGREE_SIZE_MAX, ///< With largest degree per unknown bit
      SEL_AFC_SIZE_MIN,    ///< With smallest accumulated failure count per unknown bit
      SEL_AFC_SIZE_MAX,    ///< With largest accumulated failure count per unknown bit
      SEL_ACTION_SIZE_MIN, ///< With smallest action per unknown bit
      SEL_ACTION_SIZE_MAX, ///< With largest action per unknown bit
      SEL_CHB_SIZE_MIN,    ///< With smallest CHB Q-score per unknown bit
      SEL_CHB_SIZE_MAX     ///< With largest CHB Q-score per unknown bit
    };
    /// First unassigned
    static constexpr Select SEL_NONE = Select::SEL_NONE;
    /// Random (uniform, for tie breaking)
    static constexpr Select SEL_RND = Select::SEL_RND;
    /// With least merit
    static constexpr Select SEL_MERIT_MIN = Select::SEL_MERIT_MIN;
    /// With highest merit
    static constexpr Select SEL_MERIT_MAX = Select::SEL_MERIT_MAX;
    /// With smallest degree
    static constexpr Select SEL_DEGREE_MIN = Select::SEL_DEGREE_MIN;
    /// With largest degree
    static constexpr Select SEL_DEGREE_MAX = Select::SEL_DEGREE_MAX;
    /// With smallest accumulated failure count
    static constexpr Select SEL_AFC_MIN = Select::SEL_AFC_MIN;
    /// With largest accumulated failure count
    static constexpr Select SEL_AFC_MAX = Select::SEL_AFC_MAX;
    /// With lowest action
    static constexpr Select SEL_ACTION_MIN = Select::SEL_ACTION_MIN;
    /// With highest action
    static constexpr Select SEL_ACTION_MAX = Select::SEL_ACTION_MAX;
    /// With lowest CHB Q-score
    static constexpr Select SEL_CHB_MIN = Select::SEL_CHB_MIN;
    /// With highest CHB Q-score
    static constexpr Select SEL_CHB_MAX = Select::SEL_CHB_MAX;
    /// With fewest unknown bits
    static constexpr Select SEL_SIZE_MIN = Select::SEL_SIZE_MIN;
    /// With most unknown bits
    static constexpr Select SEL_SIZE_MAX = Select::SEL_SIZE_MAX;
    /// With smallest degree per unknown bit
    static constexpr Select SEL_DEGREE_SIZE_MIN = Select::SEL_DEGREE_SIZE_MIN;
    /// With largest degree per unknown bit
    static constexpr Select SEL_DEGREE_SIZE_MAX = Select::SEL_DEGREE_SIZE_MAX;
    /// With smallest accumulated failure count per unknown bit
    static constexpr Select SEL_AFC_SIZE_MIN = Select::SEL_AFC_SIZE_MIN;
    /// With largest accumulated failure count per unknown bit
    static constexpr Select SEL_AFC_SIZE_MAX = Select::SEL_AFC_SIZE_MAX;
    /// With smallest action per unknown bit
    static constexpr Select SEL_ACTION_SIZE_MIN = Select::SEL_ACTION_SIZE_MIN;
    /// With largest action per unknown bit
    static constexpr Select SEL_ACTION_SIZE_MAX = Select::SEL_ACTION_SIZE_MAX;
    /// With smallest CHB Q-score per unknown bit
    static constexpr Select SEL_CHB_SIZE_MIN = Select::SEL_CHB_SIZE_MIN;
    /// With largest CHB Q-score per unknown bit
    static constexpr Select SEL_CHB_SIZE_MAX = Select::SEL_CHB_SIZE_MAX;
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
    enum class Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_SPLIT_MIN, ///< Ranked interval split, lower half first
      SEL_SPLIT_MAX, ///< Ranked interval split, upper half first
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
    /// Least-significant unknown bit
    static constexpr Select SEL_LSB = Select::SEL_LSB;
    /// Most-significant unknown bit
    static constexpr Select SEL_MSB = Select::SEL_MSB;
    /// Random unknown bit
    static constexpr Select SEL_RND = Select::SEL_RND;
    /// Ranked interval split, lower half first
    static constexpr Select SEL_SPLIT_MIN = Select::SEL_SPLIT_MIN;
    /// Ranked interval split, upper half first
    static constexpr Select SEL_SPLIT_MAX = Select::SEL_SPLIT_MAX;
    /// User-defined value and commit functions
    static constexpr Select SEL_VAL_COMMIT = Select::SEL_VAL_COMMIT;
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
  /**
   * \brief Split a bounded variable's ranked interval, lower half first
   *
   * Ranks follow the order of the word's domain type, which for a float
   * word is the IEEE total order, so the lower half holds the lesser values.
   * A word with a value set splits after the lower median of its set (see
   * WORD_ASSIGN_MED()), so the halves hold equally many values or the lower
   * half one more. Other words split near the midpoint of the rank bounds.
   */
  WordValBranch WORD_VAL_SPLIT_MIN(void);
  /// Split as WORD_VAL_SPLIT_MIN(), upper half first
  WordValBranch WORD_VAL_SPLIT_MAX(void);
  /**
   * \brief Select a value with \a v and commit it with \a c
   *
   * With no commit callback, the value is interpreted as a bit position and
   * the alternatives fix that bit to zero and one. Creating a choice then
   * throws Word::OutOfLimits for a bit at or above the width and
   * Word::BitAlreadyKnown for a bit that is already known. A user-defined
   * commit callback does not provide generic no-good literals.
   */
  WordValBranch WORD_VAL(WordBranchVal v, WordBranchCommit c=nullptr);

  /// Select bit-zero, admitted ranked-value, or callback assignment
  class WordAssign : public ValBranch<WordVar> {
  public:
    /// Bit selection, ranked value, or callback strategy
    enum class Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND, ///< Random unknown bit
      SEL_MIN, ///< Minimum admitted ranked value
      SEL_MED, ///< Median admitted ranked value
      SEL_MAX, ///< Maximum admitted ranked value
      SEL_VAL_COMMIT ///< User-defined value and commit functions
    };
    /// Least-significant unknown bit
    static constexpr Select SEL_LSB = Select::SEL_LSB;
    /// Most-significant unknown bit
    static constexpr Select SEL_MSB = Select::SEL_MSB;
    /// Random unknown bit
    static constexpr Select SEL_RND = Select::SEL_RND;
    /// Minimum admitted ranked value
    static constexpr Select SEL_MIN = Select::SEL_MIN;
    /// Median admitted ranked value
    static constexpr Select SEL_MED = Select::SEL_MED;
    /// Maximum admitted ranked value
    static constexpr Select SEL_MAX = Select::SEL_MAX;
    /// User-defined value and commit functions
    static constexpr Select SEL_VAL_COMMIT = Select::SEL_VAL_COMMIT;
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
  /**
   * \brief Assign each bounded variable to its minimum admitted ranked value
   *
   * Ranks follow the order of the word's domain type, which for a float
   * word is the IEEE total order: the minimum of a float word that admits a
   * negative NaN is a negative NaN.
   */
  WordAssign WORD_ASSIGN_MIN(void);
  /**
   * \brief Assign each bounded variable to an admitted median ranked value
   *
   * Ranks follow the order of the word's domain type (the IEEE total order
   * for a float word). For a word with a value set, the value is the lower
   * median of the set in rank order: of n values, the one with (n-1)/2
   * values below it. For other words, it is the least admitted rank at or
   * above the midpoint of the rank bounds, rounded down. Both rules agree on
   * a domain without holes. A full FP32 word thus takes -0 (0x80000000).
   */
  WordAssign WORD_ASSIGN_MED(void);
  /**
   * \brief Assign each bounded variable to its maximum admitted ranked value
   *
   * Ranks follow the order of the word's domain type (the IEEE total order
   * for a float word).
   */
  WordAssign WORD_ASSIGN_MAX(void);
  /**
   * \brief Assign a value selected by \a v and committed by \a c
   *
   * With no commit callback, the value is interpreted as a bit position and
   * that bit is fixed to zero. Creating a choice then throws
   * Word::OutOfLimits for a bit at or above the width and
   * Word::BitAlreadyKnown for a bit that is already known. A user-defined
   * commit callback does not provide generic no-good literals.
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
    /**
     * \brief Return the old minimum as an internal order rank
     *
     * Ranks are not encoded endpoints. For unsigned words the rank equals the
     * encoding; for signed words it is the two's-complement encoding with
     * the sign bit flipped. For float words it is the position in the total
     * order: the bitwise complement of an encoding with the sign bit set,
     * and any other encoding with the sign bit set. The result is 0 for
     * cube-only words.
     */
    WordValue old_rank_minimum(void) const;
    /// Return the old maximum as an internal order rank (see old_rank_minimum())
    WordValue old_rank_maximum(void) const;
    /// Return the new minimum as an internal order rank (see old_rank_minimum())
    WordValue new_rank_minimum(void) const;
    /// Return the new maximum as an internal order rank (see old_rank_minimum())
    WordValue new_rank_maximum(void) const;
    /**
     * \brief Return the values removed inside the new cube and bounds
     *
     * These are the values that the event removed from a value set while
     * the new cube and bounds still admit them, so neither zero(), one(),
     * nor the ranks show their removal. A values-only change removes only
     * such values. The result is empty for a word without a value set.
     */
    const Word::WordValueSet& new_holes(void) const;
    /// Test whether the trace event fixed any cube bits
    bool bits_changed(void) const;
    /// Test whether the trace event changed ranked bounds
    bool bounds_changed(void) const;
  private:
    WordValue _zero;
    WordValue _one;
    WordDomainType _domain_type;
    WordValue _old_rank_minimum;
    WordValue _old_rank_maximum;
    WordValue _new_rank_minimum;
    WordValue _new_rank_maximum;
    Word::WordValueSet _new_holes;
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

  /**
   * \brief Print \a x as its cube, its bounds, and the holes of a value set
   *
   * A cube prints as width'b followed by one character per bit from the
   * most significant one: 1, 0, or ? for an unknown bit. Bounds follow as
   * [tag:min..max] with hexadecimal endpoint encodings, where the tag names
   * the order: u, s, or the float format (fp32, bf16, binary16, e4m3fn,
   * e2m1). Float endpoints are the least and greatest encodings in the
   * total order, so a full E2M1 word prints as 4'b????[e2m1:0xf..0x7]. A word
   * with a value set appends \\{...}, the values that cube and bounds admit
   * but the set excludes (see Word::print_value_ranges), when there are any.
   * \relates Gecode::WordVar
   */
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
