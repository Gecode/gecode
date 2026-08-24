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

#include <gecode/word/distinct.hh>
#include <gecode/word/rel.hh>

namespace Gecode {

  namespace {
    ExecStatus
    post_word_distinct_val(Home home, const WordVarArgs& x) {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          GECODE_ES_CHECK((Word::Rel::Nq<
            Word::WordView,Word::WordView>::post(
              home,Word::WordView(x[i]),Word::WordView(x[j]))));
      return ES_OK;
    }
  }

  void
  distinct(Home home, const WordVarArgs& x, IntPropLevel ipl) {
    if (same(x))
      throw Word::ArgumentSame("Word::distinct");
    if (x.size() > 1) {
      const unsigned int width=x[0].width();
      for (int i=1; i<x.size(); i++)
        if (x[i].width() != width)
          throw Word::WidthMismatch("Word::distinct");
    }
    GECODE_POST;

    if ((vbd(ipl) == IPL_BND) && (x.size() > 1)) {
      const WordDomainType kind=x[0].domain_type();
      bool homogeneous=(kind != WDT_CUBE);
      for (int i=1; homogeneous && (i<x.size()); i++)
        homogeneous=x[i].domain_type() == kind;
      if (homogeneous && (kind == WDT_UNSIGNED)) {
        ViewArray<Word::UnsignedWordView> views(home,x.size());
        for (int i=0; i<x.size(); i++)
          views[i]=Word::UnsignedWordView(x[i]);
        GECODE_ES_FAIL((Word::Distinct::Bnd<
          Word::UnsignedWordView>::post(home,views)));
        return;
      }
      if (homogeneous && (kind == WDT_SIGNED)) {
        ViewArray<Word::SignedWordView> views(home,x.size());
        for (int i=0; i<x.size(); i++)
          views[i]=Word::SignedWordView(x[i]);
        GECODE_ES_FAIL((Word::Distinct::Bnd<
          Word::SignedWordView>::post(home,views)));
        return;
      }
    }

    GECODE_ES_FAIL(post_word_distinct_val(home,x));
  }

}

// STATISTICS: word-post
