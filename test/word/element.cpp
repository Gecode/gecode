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

  namespace Element {

    class Lifecycle : public Base {
    private:
      class ElementSpace : public Gecode::Space {
      public:
        Gecode::WordVarArray x;
        Gecode::IntVar index;
        Gecode::WordVar result;
        ElementSpace(void)
          : x(*this,3,2,0,3), index(*this,0,2), result(*this,2) {
          Gecode::element(*this,x,index,result);
        }
        ElementSpace(ElementSpace& s) : Gecode::Space(s) {
          x.update(*this,s.x);
          index.update(*this,s.index);
          result.update(*this,s.result);
        }
        virtual Gecode::Space* copy(void) {
          return new ElementSpace(*this);
        }
      };

      static bool focused(void) {
        class PostingSpace : public Gecode::Space {
        public:
          PostingSpace(void) {}
          PostingSpace(PostingSpace& s) : Gecode::Space(s) {}
          virtual Gecode::Space* copy(void) {
            return new PostingSpace(*this);
          }
        };

        ElementSpace hull;
        Gecode::dom(hull,hull.x[0],1U,3U);
        Gecode::dom(hull,hull.x[1],3U);
        Gecode::dom(hull,hull.x[2],0U);
        Gecode::rel(hull,hull.index,Gecode::IRT_LQ,1);
        if ((hull.status() == Gecode::SS_FAILED) ||
            (hull.result.lo() != 1U) || (hull.result.hi() != 3U))
          return false;
        Gecode::dom(hull,hull.result,3U);
        if (hull.status() == Gecode::SS_FAILED)
          return false;

        ElementSpace prune;
        Gecode::dom(prune,prune.x[0],1U);
        Gecode::dom(prune,prune.x[1],2U,3U);
        Gecode::dom(prune,prune.x[2],0U);
        Gecode::dom(prune,prune.result,2U);
        if ((prune.status() == Gecode::SS_FAILED) ||
            !prune.index.assigned() || (prune.index.val() != 1))
          return false;

        ElementSpace failed;
        Gecode::dom(failed,failed.x[0],1U);
        Gecode::dom(failed,failed.x[1],1U);
        Gecode::dom(failed,failed.x[2],1U);
        Gecode::dom(failed,failed.result,2U);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        ElementSpace invalid_low;
        Gecode::rel(invalid_low,invalid_low.index,Gecode::IRT_EQ,-1);
        if (invalid_low.status() != Gecode::SS_FAILED)
          return false;
        ElementSpace invalid_high;
        Gecode::rel(invalid_high,invalid_high.index,Gecode::IRT_EQ,3);
        if (invalid_high.status() != Gecode::SS_FAILED)
          return false;

        try {
          PostingSpace empty_home;
          Gecode::WordVarArgs empty;
          Gecode::IntVar index(empty_home,0,0);
          Gecode::WordVar result(empty_home,2);
          Gecode::element(empty_home,empty,index,result);
          return false;
        } catch (const Gecode::Word::TooFewArguments&) {}
        try {
          PostingSpace mismatch;
          Gecode::WordVarArgs words(2);
          words[0] = Gecode::WordVar(mismatch,2);
          words[1] = Gecode::WordVar(mismatch,3);
          Gecode::IntVar index(mismatch,0,1);
          Gecode::WordVar result(mismatch,2);
          Gecode::element(mismatch,words,index,result);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return true;
      }

      static bool aliases_and_clone(void) {
        using namespace Gecode;
        class AliasSpace : public Space {
        public:
          WordVar a;
          WordVar b;
          IntVar index;
          AliasSpace(void)
            : a(*this,2), b(*this,2), index(*this,0,2) {
            WordVarArgs words = {a,a,b};
            element(*this,words,index,a);
          }
          AliasSpace(AliasSpace& s) : Space(s) {
            a.update(*this,s.a);
            b.update(*this,s.b);
            index.update(*this,s.index);
          }
          virtual Space* copy(void) { return new AliasSpace(*this); }
        };

        AliasSpace source;
        if (source.status() == SS_FAILED)
          return false;
        AliasSpace* copy = static_cast<AliasSpace*>(source.clone());
        dom(*copy,copy->a,1U);
        dom(*copy,copy->b,2U);
        rel(*copy,copy->index,IRT_GQ,2);
        const bool ok = (copy->status() == SS_FAILED) &&
          !source.a.assigned() && !source.b.assigned();
        delete copy;
        return ok;
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public ElementSpace {
        public:
          SearchSpace(void) : ElementSpace() {
            branch(*this,index,INT_VAL_MIN());
            branch(*this,x,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : ElementSpace(s) {}
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions = 0;
        while (SearchSpace* solution = dfs.next()) {
          const bool ok = solution->index.assigned() &&
            solution->result.assigned() &&
            (solution->result.val() ==
             solution->x[solution->index.val()].val()) &&
            (PropagatorGroup::all.size(*solution) == 0);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 192;
      }

    public:
      Lifecycle(void) : Base("Word::Element::Lifecycle") {}
      virtual bool run(void) {
        return focused() && aliases_and_clone() && search_recomputation();
      }
    };

    Lifecycle lifecycle;

  }

}}

// STATISTICS: test-word
