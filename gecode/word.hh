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
}

#include <gecode/word/exception.hpp>
#include <gecode/word/var-imp.hpp>

namespace Gecode {
  namespace Word { class WordView; class WordTraceView; }

  /**
   * \defgroup TaskModelWord Word-vector constraints
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
  public:
    WordVar(void);
    WordVar(const WordVar& y);
    WordVar(const Word::WordView& y);
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width);
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordValue lo, WordValue hi);
    unsigned int width(void) const;
    WordValue mask(void) const;
    WordValue lo(void) const;
    WordValue hi(void) const;
    WordValue unknown(void) const;
    unsigned int unknown_size(void) const;
    bool assigned(void) const;
    bool in(WordValue value) const;
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
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValue lo, WordValue hi);
    WordVarArgs& operator=(const WordVarArgs&) = default;
  };
  /// Word variable array
  class WordVarArray : public VarArray<WordVar> {
  public:
    WordVarArray(void);
    WordVarArray(Space& home, int n);
    WordVarArray(const WordVarArray& a);
    WordVarArray(Space& home, const WordVarArgs& a);
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValue lo, WordValue hi);
    WordVarArray& operator=(const WordVarArray&) = default;
  };

  /// Restrict \a x to the word cube described by \a lo and \a hi
  GECODE_WORD_EXPORT void dom(Home home, WordVar x,
                              WordValue lo, WordValue hi);
  /// Assign \a x to \a value
  GECODE_WORD_EXPORT void dom(Home home, WordVar x, WordValue value);

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

  /**
   * \defgroup TaskModelWordRel Word relations
   * \ingroup TaskModelWord
   */
  //@{
  /// Post the relation \a wrt between \a x and \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y);
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

  /// Branch filter function type for word variables
  typedef std::function<bool(const Space& home, WordVar x, int i)>
    WordBranchFilter;
  /// Branch merit function type for word variables
  typedef std::function<double(const Space& home, WordVar x, int i)>
    WordBranchMerit;
  /// Branch bit selection function type for word variables
  typedef std::function<unsigned int(const Space& home, WordVar x, int i)>
    WordBranchVal;
  /// Branch commit function type for word variables
  typedef std::function<void(Space& home, unsigned int a,
                             WordVar x, int i, unsigned int bit)>
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
                             const unsigned int& bit, std::ostream& o)>
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

  /// Which unknown bit to select for branching
  class WordValBranch : public ValBranch<WordVar> {
  public:
    /// Bit selection strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND  ///< Random unknown bit
    };
  protected:
    Select s;
  public:
    WordValBranch(Select s=SEL_LSB);
    WordValBranch(Select s, Rnd r);
    Select select(void) const;
  };

  /// Select the least-significant unknown bit
  WordValBranch WORD_VAL_LSB(void);
  /// Select the most-significant unknown bit
  WordValBranch WORD_VAL_MSB(void);
  /// Select a random unknown bit
  WordValBranch WORD_VAL_RND(Rnd r);

  /// Which unknown bit to select for assignment to zero
  class WordAssign : public ValBranch<WordVar> {
  public:
    /// Bit selection strategy
    enum Select {
      SEL_LSB, ///< Least-significant unknown bit
      SEL_MSB, ///< Most-significant unknown bit
      SEL_RND  ///< Random unknown bit
    };
  protected:
    Select s;
  public:
    WordAssign(Select s=SEL_LSB);
    WordAssign(Select s, Rnd r);
    Select select(void) const;
  };

  /// Assign unknown bits to zero, least-significant bit first
  WordAssign WORD_ASSIGN_LSB(void);
  /// Assign unknown bits to zero, most-significant bit first
  WordAssign WORD_ASSIGN_MSB(void);
  /// Assign unknown bits to zero in random order
  WordAssign WORD_ASSIGN_RND(Rnd r);

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
  public:
    WordTraceDelta(Word::WordTraceView o, Word::WordView n,
                   const Delta& d);
    WordValue zero(void) const;
    WordValue one(void) const;
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
