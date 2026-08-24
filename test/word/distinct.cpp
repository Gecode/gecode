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

#include "test/word.hh"

namespace Test { namespace Word {

  namespace Distinct {

    class Posting : public Base {
    private:
      class PostingSpace : public Gecode::Space {
      public:
        PostingSpace(void) {}
        PostingSpace(PostingSpace& s) : Gecode::Space(s) {}
        virtual Gecode::Space* copy(void) {
          return new PostingSpace(*this);
        }
      };

      static bool unsigned_hall(void) {
        using namespace Gecode;
        PostingSpace value;
        WordVarArgs xv(3);
        xv[0]=WordVar(value,3,WDT_UNSIGNED,0,1);
        xv[1]=WordVar(value,3,WDT_UNSIGNED,0,1);
        xv[2]=WordVar(value,3,WDT_UNSIGNED,0,2);
        distinct(value,xv);
        if ((value.status() == SS_FAILED) || (xv[2].minimum() != 0U))
          return false;

        PostingSpace bounds;
        WordVarArgs xb(3);
        xb[0]=WordVar(bounds,3,WDT_UNSIGNED,0,1);
        xb[1]=WordVar(bounds,3,WDT_UNSIGNED,0,1);
        xb[2]=WordVar(bounds,3,WDT_UNSIGNED,0,2);
        distinct(bounds,xb,IPL_BND);
        if ((bounds.status() == SS_FAILED) || (xb[2].minimum() != 2U))
          return false;

        PostingSpace impossible;
        WordVarArgs xi(impossible,3,3,WDT_UNSIGNED,0,1);
        distinct(impossible,xi,IPL_BND);
        return impossible.status() == SS_FAILED;
      }

      static bool signed_hall(void) {
        using namespace Gecode;
        PostingSpace home;
        WordVarArgs x(3);
        x[0]=WordVar(home,3,WDT_SIGNED,4,5);
        x[1]=WordVar(home,3,WDT_SIGNED,4,5);
        x[2]=WordVar(home,3,WDT_SIGNED,4,6);
        distinct(home,x,IPL_BND);
        return (home.status() != SS_FAILED) && (x[2].minimum() == 6U);
      }

      static bool width64(void) {
        using namespace Gecode;
        const WordValue maximum=~WordValue(0);
        PostingSpace home;
        WordVarArgs x(3);
        x[0]=WordVar(home,64,WDT_UNSIGNED,maximum-1U,maximum);
        x[1]=WordVar(home,64,WDT_UNSIGNED,maximum-1U,maximum);
        x[2]=WordVar(home,64,WDT_UNSIGNED,maximum-2U,maximum);
        distinct(home,x,IPL_BND);
        return (home.status() != SS_FAILED) &&
          (x[2].maximum() == maximum-2U);
      }

      static bool errors(void) {
        using namespace Gecode;
        try {
          PostingSpace home;
          WordVar x(home,4);
          distinct(home,WordVarArgs({x,x}));
          return false;
        } catch (const Gecode::Word::ArgumentSame&) {}
        try {
          PostingSpace home;
          WordVarArgs x({WordVar(home,3),WordVar(home,4)});
          distinct(home,x);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

    public:
      Posting(void) : Base("Word::Distinct::Posting") {}
      virtual bool run(void) {
        return unsigned_hall() && signed_hall() && width64() && errors();
      }
    };

    Posting posting;

    class Search : public Base {
    private:
      class SearchSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        SearchSpace(void)
          : x(*this,3,2,Gecode::WDT_UNSIGNED,0,3) {
          Gecode::distinct(*this,x,Gecode::IPL_BND);
          Gecode::branch(*this,x,Gecode::WORD_VAR_SIZE_MIN(),
                         Gecode::WORD_VAL_LSB());
        }
        SearchSpace(SearchSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new SearchSpace(*this);
        }
      };
    public:
      Search(void) : Base("Word::Distinct::Search") {}
      virtual bool run(void) {
        using namespace Gecode;
        SearchSpace* root=new SearchSpace;
        Gecode::Search::Options options;
        options.c_d=2;
        options.a_d=8;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (SearchSpace* solution=dfs.next()) {
          const bool distinct_values=
            solution->x[0].val() != solution->x[1].val() &&
            solution->x[0].val() != solution->x[2].val() &&
            solution->x[1].val() != solution->x[2].val();
          delete solution;
          if (!distinct_values)
            return false;
          solutions++;
        }
        return solutions == 24U;
      }
    };

    Search search;

  }

}}
