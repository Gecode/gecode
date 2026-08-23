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

    class Bounded : public Base {
    private:
      using WordValue = Gecode::WordValue;

      struct Spec {
        WordValue lo, hi, minimum, maximum;
      };

      static WordValue encode(Gecode::WordDomainType kind,
                              unsigned int width, WordValue rank) {
        return Gecode::Word::rank(kind,width,rank);
      }

      static Spec spec(Gecode::WordDomainType kind, unsigned int width,
                       unsigned int values) {
        const WordValue n=WordValue(1) << width;
        Spec s={n-1,0,n-1,0};
        for (WordValue value=0; value<n; value++)
          if ((values & (1U << value)) != 0) {
            const WordValue rank=Gecode::Word::rank(kind,width,value);
            s.lo &= value; s.hi |= value;
            s.minimum=std::min(s.minimum,rank);
            s.maximum=std::max(s.maximum,rank);
          }
        return s;
      }

      static bool contains(const Spec& s, Gecode::WordDomainType kind,
                           unsigned int width, WordValue value) {
        const WordValue rank=Gecode::Word::rank(kind,width,value);
        return ((value & s.lo) == s.lo) && ((value & ~s.hi) == 0) &&
          (rank >= s.minimum) && (rank <= s.maximum);
      }

      class OracleSpace : public Gecode::Space {
      public:
        Gecode::WordVar a, b, result;
        Gecode::IntVar index;
        OracleSpace(unsigned int width, Gecode::WordDomainType kind,
                    const Spec& as, const Spec& bs, const Spec& rs)
          : a(*this,width,kind,encode(kind,width,as.minimum),
              encode(kind,width,as.maximum)),
            b(*this,width,kind,encode(kind,width,bs.minimum),
              encode(kind,width,bs.maximum)),
            result(*this,width,kind,encode(kind,width,rs.minimum),
                   encode(kind,width,rs.maximum)),
            index(*this,0,1) {
          Gecode::dom(*this,a,as.lo,as.hi);
          Gecode::dom(*this,b,bs.lo,bs.hi);
          Gecode::dom(*this,result,rs.lo,rs.hi);
          Gecode::WordVarArgs words={a,b};
          Gecode::element(*this,words,index,result);
        }
        OracleSpace(OracleSpace& s) : Gecode::Space(s) {
          a.update(*this,s.a); b.update(*this,s.b);
          result.update(*this,s.result); index.update(*this,s.index);
        }
        Gecode::Space* copy(void) { return new OracleSpace(*this); }
      };

      static bool exhaustive_oracle(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED})
          for (unsigned int width=1; width<=2; width++) {
            const unsigned int n=1U << width;
            const unsigned int domains=1U << n;
            for (unsigned int am=1; am<domains; am++)
              for (unsigned int bm=1; bm<domains; bm++)
                for (unsigned int rm=1; rm<domains; rm++) {
                  const Spec as=spec(kind,width,am);
                  const Spec bs=spec(kind,width,bm);
                  const Spec rs=spec(kind,width,rm);
                  bool support[2]={false,false};
                  bool value_support[2][4]={{false,false,false,false},
                                            {false,false,false,false}};
                  for (WordValue value=0; value<n; value++) {
                    if (!contains(rs,kind,width,value))
                      continue;
                    value_support[0][value]=contains(as,kind,width,value);
                    value_support[1][value]=contains(bs,kind,width,value);
                    support[0] = support[0] || value_support[0][value];
                    support[1] = support[1] || value_support[1][value];
                  }

                  OracleSpace s(width,kind,as,bs,rs);
                  const bool failed=s.status() == SS_FAILED;
                  if (failed) {
                    if (support[0] || support[1])
                      return false;
                    continue;
                  }
                  const WordVar candidates[2]={s.a,s.b};
                  for (int candidate=0; candidate<2; candidate++) {
                    if (support[candidate] && !s.index.in(candidate))
                      return false;
                    for (WordValue value=0; value<n; value++)
                      if (value_support[candidate][value] &&
                          (!candidates[candidate].in(value) ||
                           !s.result.in(value)))
                        return false;
                  }
                }
          }
        return true;
      }

      class RangeSpace : public Gecode::Space {
      public:
        Gecode::WordVar a, b, c, result;
        Gecode::IntVar index;
        RangeSpace(Gecode::WordDomainType kind,
                   WordValue amin, WordValue amax,
                   WordValue bmin, WordValue bmax,
                   WordValue cmin, WordValue cmax,
                   WordValue rmin, WordValue rmax)
          : a(*this,3,kind,encode(kind,3,amin),encode(kind,3,amax)),
            b(*this,3,kind,encode(kind,3,bmin),encode(kind,3,bmax)),
            c(*this,3,kind,encode(kind,3,cmin),encode(kind,3,cmax)),
            result(*this,3,kind,encode(kind,3,rmin),encode(kind,3,rmax)),
            index(*this,0,2) {
          Gecode::WordVarArgs words={a,b,c};
          Gecode::element(*this,words,index,result);
        }
        RangeSpace(RangeSpace& s) : Gecode::Space(s) {
          a.update(*this,s.a); b.update(*this,s.b); c.update(*this,s.c);
          result.update(*this,s.result); index.update(*this,s.index);
        }
        Gecode::Space* copy(void) { return new RangeSpace(*this); }
      };

      static WordValue minimum_rank(const Gecode::WordVar& x,
                                    Gecode::WordDomainType kind) {
        return Gecode::Word::rank(kind,3,x.minimum());
      }

      static WordValue maximum_rank(const Gecode::WordVar& x,
                                    Gecode::WordDomainType kind) {
        return Gecode::Word::rank(kind,3,x.maximum());
      }

      static bool focused(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED}) {
          RangeSpace prune(kind,0,1,3,4,6,7,2,5);
          if ((prune.status() == SS_FAILED) || !prune.index.assigned() ||
              (prune.index.val() != 1) ||
              (minimum_rank(prune.result,kind) != 3U) ||
              (maximum_rank(prune.result,kind) != 4U))
            return false;

          RangeSpace hull(kind,1,2,5,6,7,7,0,7);
          rel(hull,hull.index,IRT_LQ,1);
          if ((hull.status() == SS_FAILED) ||
              (minimum_rank(hull.result,kind) != 1U) ||
              (maximum_rank(hull.result,kind) != 6U))
            return false;

          RangeSpace source(kind,0,2,3,4,5,7,0,7);
          if ((source.status() == SS_FAILED) || source.index.assigned())
            return false;
          RangeSpace* clone=static_cast<RangeSpace*>(source.clone());
          dom(*clone,clone->result,encode(kind,3,6));
          const bool clone_ok=(clone->status() != SS_FAILED) &&
            clone->index.assigned() && (clone->index.val() == 2) &&
            !source.index.assigned();
          delete clone;
          if (!clone_ok)
            return false;
        }

        class Duplicate : public Space {
        public:
          WordVar a, b, result;
          IntVar index;
          Duplicate(void)
            : a(*this,3,WDT_UNSIGNED,1U,3U),
              b(*this,3,WDT_UNSIGNED,6U,7U),
              result(*this,3,WDT_UNSIGNED,0U,4U), index(*this,0,2) {
            WordVarArgs words={a,a,b};
            element(*this,words,index,result);
          }
          Duplicate(Duplicate& s) : Space(s) {
            a.update(*this,s.a); b.update(*this,s.b);
            result.update(*this,s.result); index.update(*this,s.index);
          }
          Space* copy(void) { return new Duplicate(*this); }
        } duplicate;
        if ((duplicate.status() == SS_FAILED) ||
            !duplicate.index.in(0) || !duplicate.index.in(1) ||
            duplicate.index.in(2))
          return false;

        class Sparse : public Space {
        public:
          WordVarArray x;
          WordVar result;
          IntVar index;
          Sparse(void)
            : x(*this,4,3,WDT_UNSIGNED,0U,7U),
              result(*this,3,WDT_UNSIGNED,0U,7U),
              index(*this,IntSet(IntArgs({0,2}))) {
            element(*this,x,index,result);
          }
          Sparse(Sparse& s) : Space(s) {
            x.update(*this,s.x); result.update(*this,s.result);
            index.update(*this,s.index);
          }
          Space* copy(void) { return new Sparse(*this); }
        } sparse;
        dom(sparse,sparse.x[0],1U);
        dom(sparse,sparse.x[2],5U);
        if ((sparse.status() == SS_FAILED) || sparse.index.in(1) ||
            sparse.index.in(3) || (sparse.result.lo() != 1U) ||
            (sparse.result.hi() != 5U))
          return false;

        class Alias : public Space {
        public:
          WordVar result, other;
          IntVar index;
          Alias(void)
            : result(*this,3,WDT_UNSIGNED,1U,3U),
              other(*this,3,WDT_UNSIGNED,6U,7U), index(*this,0,1) {
            WordVarArgs words={result,other};
            element(*this,words,index,result);
          }
          Alias(Alias& s) : Space(s) {
            result.update(*this,s.result); other.update(*this,s.other);
            index.update(*this,s.index);
          }
          Space* copy(void) { return new Alias(*this); }
        } alias;
        return (alias.status() != SS_FAILED) && alias.index.assigned() &&
          (alias.index.val() == 0);
      }

      static bool replay(void) {
        using namespace Gecode;
        class SearchSpace : public RangeSpace {
        public:
          SearchSpace(void)
            : RangeSpace(WDT_UNSIGNED,0,2,3,4,5,7,0,7) {
            branch(*this,index,INT_VAL_MIN());
            WordVarArgs words={a,b,c,result};
            branch(*this,words,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
          }
          SearchSpace(SearchSpace& s) : RangeSpace(s) {}
          Space* copy(void) { return new SearchSpace(*this); }
        };
        SearchSpace* root=new SearchSpace;
        Search::Options options;
        options.c_d=1; options.a_d=1;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (SearchSpace* solution=dfs.next()) {
          const bool ok=solution->index.assigned() &&
            solution->result.assigned() &&
            (solution->result.val() ==
             (solution->index.val() == 0 ? solution->a.val() :
              (solution->index.val() == 1 ? solution->b.val() :
                                            solution->c.val()))) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 54U;
      }

    public:
      Bounded(void) : Base("Word::Element::Bounded") {}
      bool run(void) {
        return exhaustive_oracle() && focused() && replay();
      }
    };

    Bounded bounded;

  }

}}

// STATISTICS: test-word
