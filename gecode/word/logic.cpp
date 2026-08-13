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
    unsigned int logic_value(WordOpType wot, unsigned int x,
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

    WordOpType primitive(WordOpType wot) {
      switch (wot) {
      case WOT_AND: case WOT_NAND: return WOT_AND;
      case WOT_OR:  case WOT_NOR:  return WOT_OR;
      case WOT_XOR: case WOT_XNOR: return WOT_XOR;
      default: throw Word::UnknownOperation("Word::rel");
      }
    }

    bool negated(WordOpType wot) {
      switch (wot) {
      case WOT_AND: case WOT_OR: case WOT_XOR: return false;
      case WOT_NAND: case WOT_NOR: case WOT_XNOR: return true;
      default: throw Word::UnknownOperation("Word::rel");
      }
    }

    unsigned int binary_table(WordOpType wot) {
      unsigned int table = 0;
      for (unsigned int x=0; x<2; x++)
        for (unsigned int y=0; y<2; y++) {
          const unsigned int z = logic_value(wot,x,y);
          table |= 1U << (x | (y << 1) | (z << 2));
        }
      return table;
    }

    void post_uniform_table(Home home, const Word::WordView* views, int n,
                            unsigned int table) {
      WordValue allowed[8] = {0,0,0,0,0,0,0,0};
      const WordValue mask = views[0].mask();
      for (unsigned int t=0; t<(1U << n); t++)
        if ((table & (1U << t)) != 0)
          allowed[t] = mask;
      Word::Logic::post_table(home,views,n,allowed);
    }

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

    WordValue combine(WordOpType wot, WordValue x, WordValue y) {
      switch (wot) {
      case WOT_AND: return x&y;
      case WOT_OR:  return x|y;
      case WOT_XOR: return x^y;
      default: GECODE_NEVER;
      }
      return 0;
    }

    void post_nary_primitive(Home home, WordOpType wot,
                             const WordVarArgs& input, WordVar result) {
      const WordValue mask=result.mask();
      WordValue constant=(wot == WOT_AND) ? mask : 0;
      ViewArray<Word::WordView> x(home,input.size());
      int n=0;
      for (int i=0; i<input.size(); i++) {
        Word::WordView next(input[i]);
        if (next.assigned()) {
          constant=combine(wot,constant,next.val());
          continue;
        }
        int duplicate=0;
        while ((duplicate < n) && !(x[duplicate] == next))
          duplicate++;
        if (duplicate == n) {
          x[n++]=next;
        } else if (wot == WOT_XOR) {
          n--;
          x[duplicate]=x[n];
        }
      }
      x.size(n);

      if (wot == WOT_XOR) {
        int alias=0;
        Word::WordView y(result);
        while ((alias < x.size()) && !(x[alias] == y))
          alias++;
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
      WordView unique[4];
      int map[4];
      int m = 0;
      for (int i=0; i<n; i++) {
        map[i] = 0;
        while ((map[i] < m) && !(original[i] == unique[map[i]]))
          map[i]++;
        if (map[i] == m)
          unique[m++] = original[i];
      }

      WordValue projected[16] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
      };
      for (unsigned int t=0; t<(1U << m); t++) {
        unsigned int source = 0;
        for (int i=0; i<n; i++)
          source |= ((t >> map[i]) & 1U) << i;
        projected[t] |= allowed[source];
      }

      ViewArray<WordView> views(home,m);
      for (int i=0; i<m; i++)
        views[i] = unique[i];
      GECODE_ES_FAIL(Table::post(home,views,projected));
    }

  }}

  void
  complement(Home home, WordVar x, WordVar y) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::complement");
    GECODE_POST;
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y)};
    post_uniform_table(home,views,2,0x6U);
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
    if ((x.width() != y.width()) || (x.width() != z.width()))
      throw Word::WidthMismatch("Word::rel");
    const unsigned int table = binary_table(wot);
    GECODE_POST;
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y),
                                    Word::WordView(z)};
    if ((views[0] != views[1]) && (views[0] != views[2]) &&
        (views[1] != views[2])) {
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
    if ((x.width() != width) || (z.width() != width))
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    WordValue allowed[4] = {0,0,0,0};
    const WordValue zero = ~c.val() & c.mask();
    for (unsigned int t=0; t<4; t++) {
      const unsigned int xv = t & 1U;
      const unsigned int zv = (t >> 1) & 1U;
      if (logic_value(wot,xv,0) == zv)
        allowed[t] |= zero;
      if (logic_value(wot,xv,1) == zv)
        allowed[t] |= c.val();
    }
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(z)};
    Word::Logic::post_table(home,views,2,allowed);
  }

  void
  rel(Home home, WordVar x, WordOpType wot, WordVar y,
      unsigned int width, WordValue value) {
    Word::ConstWordView c(width,value);
    if ((x.width() != width) || (y.width() != width))
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    WordValue allowed[4] = {0,0,0,0};
    const WordValue zero = ~c.val() & c.mask();
    for (unsigned int t=0; t<4; t++) {
      const unsigned int xv = t & 1U;
      const unsigned int yv = (t >> 1) & 1U;
      allowed[t] = (logic_value(wot,xv,yv) == 0) ? zero : c.val();
    }
    const Word::WordView views[] = {Word::WordView(x),Word::WordView(y)};
    Word::Logic::post_table(home,views,2,allowed);
  }

  void
  rel(Home home, WordOpType wot, const WordVarArgs& x, WordVar y) {
    const WordOpType base = primitive(wot);
    const bool negate = negated(wot);
    for (int i=0; i<x.size(); i++)
      if (x[i].width() != y.width())
        throw Word::WidthMismatch("Word::rel");
    GECODE_POST;

    if (x.size() == 0) {
      assign_identity(home,y,wot);
      return;
    }
    if (x.size() == 1) {
      if (negate)
        complement(home,x[0],y);
      else
        GECODE_ES_FAIL((Word::Rel::Eq<Word::WordView,Word::WordView>
                        ::post(home,Word::WordView(x[0]),Word::WordView(y))));
      return;
    }

    if (!negate) {
      post_nary_primitive(home,base,x,y);
      return;
    }
    WordVar aggregate(home,y.width());
    post_nary_primitive(home,base,x,aggregate);
    complement(home,aggregate,y);
  }

}

// STATISTICS: word-post
