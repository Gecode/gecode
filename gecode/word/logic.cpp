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
 */

#include <gecode/word/logic.hh>
#include <gecode/word/rel.hh>

namespace Gecode {

  namespace {
    /// Evaluates one logical bit; throws UnknownOperation for an invalid wot.
    unsigned int evaluate_bit(WordOpType wot, unsigned int x,
                             unsigned int y) {
      switch (wot) {
      case WOT_AND:  return x & y;
      case WOT_OR:   return x | y;
      case WOT_XOR:  return x ^ y;
      case WOT_NAND: return 1U ^ (x & y);
      case WOT_NOR:  return 1U ^ (x | y);
      case WOT_XNOR: return 1U ^ (x ^ y);
      default: throw Word::UnknownOperation("Word::rel");
      }
    }

    /// Selects AND, OR or XOR; throws UnknownOperation for an invalid wot.
    WordOpType get_primitive(WordOpType wot) {
      switch (wot) {
      case WOT_AND: case WOT_NAND: return WOT_AND;
      case WOT_OR:  case WOT_NOR:  return WOT_OR;
      case WOT_XOR: case WOT_XNOR: return WOT_XOR;
      default: throw Word::UnknownOperation("Word::rel");
      }
    }

    /// Classifies complementing operations; throws UnknownOperation if invalid.
    bool is_negated(WordOpType wot) {
      switch (wot) {
      case WOT_AND: case WOT_OR: case WOT_XOR: return false;
      case WOT_NAND: case WOT_NOR: case WOT_XNOR: return true;
      default: throw Word::UnknownOperation("Word::rel");
      }
    }

    /// Encodes the binary relation; throws UnknownOperation for an invalid wot.
    unsigned int make_binary_table(WordOpType wot) {
      unsigned int table = 0;
      for (unsigned int x=0; x<2; x++) {
        for (unsigned int y=0; y<2; y++) {
          const unsigned int z = evaluate_bit(wot,x,y);
          table |= 1U << (x | (y << 1) | (z << 2));
        }
      }
      return table;
    }

    /// Posts a uniform table for two or three views.
    void post_uniform_table(Home home, const Word::WordView* views, int n,
                            unsigned int table) {
      WordValue allowed[8] = {0,0,0,0,0,0,0,0};
      const WordValue mask = views[0].mask();
      for (unsigned int t=0; t<(1U << n); t++) {
        if ((table & (1U << t)) != 0)
          allowed[t] = mask;
      }
      Word::Logic::post_table(home,views,n,allowed);
    }

    /// Assigns the empty identity; throws UnknownOperation for an invalid wot.
    void assign_identity(Home home, WordVar y, WordOpType wot) {
      WordValue value;
      switch (wot) {
      case WOT_AND: case WOT_NOR: case WOT_XNOR:
        value = Word::width_mask(y.width()); break;
      case WOT_OR: case WOT_XOR: case WOT_NAND:
        value = 0; break;
      default:
        throw Word::UnknownOperation("Word::rel");
      }
      GECODE_ME_FAIL(Word::WordView(y).eq(home,value));
    }

    /// Combines primitive AND/OR/XOR values; other operations are unreachable.
    WordValue combine_primitive(WordOpType wot, WordValue x, WordValue y) {
      switch (wot) {
      case WOT_AND: return x&y;
      case WOT_OR:  return x|y;
      case WOT_XOR: return x^y;
      default: GECODE_NEVER;
      }
      return 0;
    }

    /// Original input roles and the starting identity for primitive folding.
    struct NaryInput {
      const WordVarArgs& variables;
      WordOpType wot;
      WordValue constant;
    };

    /// Home-owned normalized view storage and the folded assigned operands.
    struct NormalizedOperandsResult {
      ViewArray<Word::WordView> views;
      WordValue constant;
    };

    /// Folds assigned inputs and normalizes duplicates before result-alias handling.
    NormalizedOperandsResult normalize_operands(Home home, const NaryInput& input) {
      ViewArray<Word::WordView> x(home,input.variables.size());
      WordValue constant=input.constant;
      const WordOpType wot=input.wot;
      int n=0;
      for (int i=0; i<input.variables.size(); i++) {
        Word::WordView next(input.variables[i]);
        if (next.assigned()) {
          constant=combine_primitive(wot,constant,next.val());
          continue;
        }
        x[n++]=next;
      }
      // Group duplicates once instead of scanning all previous operands.
      if (n > 1)
        Support::quicksort<Word::WordView>(&x[0],n);
      int unique=0;
      for (int i=0; i<n;) {
        int end=i+1;
        while ((end < n) && (x[end] == x[i])) {
          end++;
        }
        const bool has_remaining_operand=
          (wot != WOT_XOR) || ((end-i) % 2 != 0);
        if (has_remaining_operand)
          x[unique++]=x[i];
        i=end;
      }
      x.size(unique);
      return {x,constant};
    }

    /// Keeps distinct view order and the projected bit-position admission masks.
    struct TableProjectionResult {
      Word::WordView views[4];
      int size;
      WordValue allowed[16];
    };

    /// Projects one to four original roles without allocating actor view storage.
    TableProjectionResult project_table(const Word::WordView* original, int n,
                                 const WordValue* allowed) {
      TableProjectionResult projection={
        {original[0],original[0],original[0],original[0]},0,{}
      };
      int map[4];
      for (int i=0; i<n; i++) {
        map[i]=0;
        while ((map[i] < projection.size) &&
               !(original[i] == projection.views[map[i]])) {
          map[i]++;
        }
        if (map[i] == projection.size)
          projection.views[projection.size++]=original[i];
      }
      for (unsigned int t=0; t<(1U << projection.size); t++) {
        unsigned int source=0;
        for (int i=0; i<n; i++) {
          source |= ((t >> map[i]) & 1U) << i;
        }
        projection.allowed[t] |= allowed[source];
      }
      return projection;
    }

    /// Normalizes primitive inputs, then selects the surviving relation actor.
    void post_nary_primitive(Home home, WordOpType wot,
                             const WordVarArgs& input, WordVar result,
                             WordValue constant) {
      const NaryInput original={input,wot,constant};
      const NormalizedOperandsResult normalized=normalize_operands(home,original);
      ViewArray<Word::WordView> x=normalized.views;
      constant=normalized.constant;

      if (wot == WOT_XOR) {
        int alias=0;
        Word::WordView y(result);
        while ((alias < x.size()) && !(x[alias] == y)) {
          alias++;
        }
        if (alias < x.size()) {
          x.move_lst(alias);
          Word::ConstWordView zero(result.width(),0);
          GECODE_ES_FAIL((Word::Logic::Nary<
            Word::Logic::NO_XOR,Word::ConstWordView>::post(
              home,x,zero,constant)));
          return;
        }
      }

      switch (wot) {
      case WOT_AND:
        GECODE_ES_FAIL((Word::Logic::Nary<
          Word::Logic::NO_AND,Word::WordView>::post(
            home,x,Word::WordView(result),constant)));
        break;
      case WOT_OR:
        GECODE_ES_FAIL((Word::Logic::Nary<
          Word::Logic::NO_OR,Word::WordView>::post(
            home,x,Word::WordView(result),constant)));
        break;
      case WOT_XOR:
        GECODE_ES_FAIL((Word::Logic::Nary<
          Word::Logic::NO_XOR,Word::WordView>::post(
            home,x,Word::WordView(result),constant)));
        break;
      default:
        GECODE_NEVER;
      }
    }
  }

  namespace Word { namespace Logic {

    void
    post_table(Home home, const WordView* original, int n,
               const WordValue* allowed) {
      assert((n >= 1) && (n <= 4));
      const TableProjectionResult projected=project_table(original,n,allowed);
      ViewArray<WordView> views(home,projected.size);
      for (int i=0; i<projected.size; i++) {
        views[i]=projected.views[i];
      }
      GECODE_ES_FAIL(Table::post(home,views,projected.allowed));
    }

  }}

  void
  complement(Home home, WordVar x, WordVar y) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::complement");
    GECODE_POST;
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y)};
    constexpr unsigned int kComplementTable=0x6U;
    post_uniform_table(home,views,2,kComplementTable);
  }

  void
  complement(Home home, unsigned int width, WordValue value, WordVar y) {
    Word::ConstWordView c(width,value);
    if (width != y.width())
      throw Word::WidthMismatch("Word::complement");
    GECODE_POST;
    GECODE_ME_FAIL(Word::WordView(y).eq(home,(~c.val()) & c.mask()));
  }

  void
  complement(Home home, WordVar x, unsigned int width, WordValue value) {
    Word::ConstWordView c(width,value);
    if (x.width() != width)
      throw Word::WidthMismatch("Word::complement");
    GECODE_POST;
    GECODE_ME_FAIL(Word::WordView(x).eq(home,(~c.val()) & c.mask()));
  }

  void
  rel(Home home, WordVar x, WordOpType wot, WordVar y, WordVar z) {
    const bool has_width_mismatch=
      (x.width() != y.width()) || (x.width() != z.width());
    if (has_width_mismatch)
      throw Word::WidthMismatch("Word::rel");
    const unsigned int table = make_binary_table(wot);
    GECODE_POST;
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y),
                                    Word::WordView(z)};
    const bool has_distinct_views=
      (views[0] != views[1]) && (views[0] != views[2]) &&
      (views[1] != views[2]);
    if (has_distinct_views) {
      switch (wot) {
      case WOT_OR:
        GECODE_ES_FAIL((Word::Logic::Binary<Word::Logic::BO_OR>::post(
          home,views[0],views[1],views[2])));
        return;
      case WOT_XOR:
        GECODE_ES_FAIL((Word::Logic::Binary<Word::Logic::BO_XOR>::post(
          home,views[0],views[1],views[2])));
        return;
      default: break;
      }
    }
    post_uniform_table(home,views,3,table);
  }

  void
  rel(Home home, WordVar x, WordOpType wot, unsigned int width,
      WordValue value, WordVar z) {
    Word::ConstWordView c(width,value);
    const bool has_width_mismatch=
      (x.width() != width) || (z.width() != width);
    if (has_width_mismatch)
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    WordValue allowed[4] = {0,0,0,0};
    const WordValue zero = ~c.val() & c.mask();
    for (unsigned int t=0; t<4; t++) {
      const unsigned int xv = t & 1U;
      const unsigned int zv = (t >> 1) & 1U;
      if (evaluate_bit(wot,xv,0) == zv)
        allowed[t] |= zero;
      if (evaluate_bit(wot,xv,1) == zv)
        allowed[t] |= c.val();
    }
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(z)};
    Word::Logic::post_table(home,views,2,allowed);
  }

  void
  rel(Home home, WordVar x, WordOpType wot, WordVar y,
      unsigned int width, WordValue value) {
    Word::ConstWordView c(width,value);
    const bool has_width_mismatch=
      (x.width() != width) || (y.width() != width);
    if (has_width_mismatch)
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    WordValue allowed[4] = {0,0,0,0};
    const WordValue zero = ~c.val() & c.mask();
    for (unsigned int t=0; t<4; t++) {
      const unsigned int xv = t & 1U;
      const unsigned int yv = (t >> 1) & 1U;
      allowed[t] = (evaluate_bit(wot,xv,yv) == 0) ? zero : c.val();
    }
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y)};
    Word::Logic::post_table(home,views,2,allowed);
  }

  void
  rel(Home home, WordOpType wot, const WordVarArgs& x, WordVar y) {
    const WordOpType base = get_primitive(wot);
    const bool is_negated_operation = is_negated(wot);
    for (int i=0; i<x.size(); i++) {
      if (x[i].width() != y.width())
        throw Word::WidthMismatch("Word::rel");
    }
    GECODE_POST;

    if (x.size() == 0) {
      assign_identity(home,y,wot);
      return;
    }
    if (x.size() == 1) {
      if (is_negated_operation)
        complement(home,x[0],y);
      else
        GECODE_ES_FAIL((Word::Rel::Eq<Word::WordView,Word::WordView>
                        ::post(home,Word::WordView(x[0]),Word::WordView(y))));
      return;
    }

    // XNOR folds the complement into XOR before cancelling aliased operands.
    const bool has_full_identity=(base == WOT_AND) || (wot == WOT_XNOR);
    const WordValue constant=has_full_identity ? y.mask() : 0;
    const bool is_direct=!is_negated_operation || (base == WOT_XOR);
    if (is_direct) {
      post_nary_primitive(home,base,x,y,constant);
      return;
    }
    for (int i=0; i<x.size(); i++) {
      if (Word::WordView(x[i]) == Word::WordView(y)) {
        // y = NAND(y,...) requires y=mask; y = NOR(y,...) requires y=0.
        GECODE_ME_FAIL(Word::WordView(y).eq(
          home,(base == WOT_AND) ? y.mask() : 0));
        break;
      }
    }
    WordVar aggregate(home,y.width(),y.domain_type());
    post_nary_primitive(home,base,x,aggregate,constant);
    complement(home,aggregate,y);
  }

}

// STATISTICS: word-post
