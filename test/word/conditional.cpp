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

#include "test/word.hh"

#include <gecode/search.hh>

namespace Test { namespace Word {

  namespace Conditional {

    static Gecode::WordValue
    mask_ite(Gecode::WordValue control, Gecode::WordValue then_value,
             Gecode::WordValue else_value, Gecode::WordValue mask) {
      return ((control & then_value) | (~control & else_value)) & mask;
    }

    /**
     * The mask form is a direct per-bit truth-table actor and enforces bit
     * consistency across the control, branches, and result.
     */
    class Mask : public Test {
    public:
      Mask(void) : Test("Conditional::Mask",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == mask_ite(a[0],a[1],a[2],dom.mask());
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::ite(home,x[0],x[1],x[2],x[3]);
      }
    };

    /**
     * The Boolean form uses one direct mixed actor. With an unknown control,
     * it narrows the result to the cube hull of both branches and rejects a
     * branch only when it is disjoint from the result.
     */
    class Boolean : public Test {
    public:
      Boolean(void) : Test("Conditional::Boolean",4,Domain(2,0,3)) {}
      virtual bool solution(const Assignment& a) const {
        return a[3] == ((a[0] & 1U) ? a[1] : a[2]);
      }
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) {
        Gecode::BoolVar control(home,0,1);
        Gecode::channel(home,x[0],0,control);
        Gecode::ite(home,control,x[1],x[2],x[3]);
      }
    };

    class Lifecycle : public Base {
    private:
      class ConditionalSpace : public Gecode::Space {
      public:
        Gecode::BoolVar control;
        Gecode::WordVarArray x;
        ConditionalSpace(unsigned int width=2)
          : control(*this,0,1),
            x(*this,3,width,0,Gecode::Word::width_mask(width)) {}
        ConditionalSpace(ConditionalSpace& s) : Gecode::Space(s) {
          control.update(*this,s.control);
          x.update(*this,s.x);
        }
        virtual Gecode::Space* copy(void) {
          return new ConditionalSpace(*this);
        }
      };

      static bool partial_mask(void) {
        for (PartialAssignment p(4,1); p.has_more(); p.next()) {
          TestSpace s(4,Domain(1,0,1));
          std::vector<Domain> domains;
          for (int i=0; i<4; i++)
            domains.push_back(p[i]);
          s.narrow(domains);
          Gecode::ite(s,s.x[0],s.x[1],s.x[2],s.x[3]);
          const bool failed = s.failed();
          bool supported = false;
          bool bit_support[4][2] = {
            {false,false},{false,false},{false,false},{false,false}
          };
          for (Values c(p[0]); c(); ++c)
            for (Values t(p[1]); t(); ++t)
              for (Values e(p[2]); e(); ++e)
                for (Values r(p[3]); r(); ++r)
                  if (r.val() == mask_ite(c.val(),t.val(),e.val(),1U)) {
                    supported = true;
                    const Gecode::WordValue values[] = {
                      c.val(),t.val(),e.val(),r.val()
                    };
                    for (int i=0; i<4; i++)
                      bit_support[i][values[i]] = true;
                  }
          if (failed == supported)
            return false;
          if (!failed)
            for (int i=0; i<4; i++)
              for (int value=0; value<2; value++)
                if (s.x[i].in(static_cast<Gecode::WordValue>(value)) !=
                    bit_support[i][value])
                  return false;
        }
        return true;
      }

      static bool native_propagation(void) {
        ConditionalSpace hull(4);
        Gecode::dom(hull,hull.x[0],1U,3U);
        Gecode::dom(hull,hull.x[1],5U,7U);
        Gecode::ite(hull,hull.control,hull.x[0],hull.x[1],hull.x[2]);
        if ((hull.status() == Gecode::SS_FAILED) ||
            (hull.x[2].lo() != 1U) || (hull.x[2].hi() != 7U) ||
            (Gecode::PropagatorGroup::all.size(hull) != 1U))
          return false;

        ConditionalSpace infer(4);
        Gecode::dom(infer,infer.x[0],0U,3U);
        Gecode::dom(infer,infer.x[1],8U,11U);
        Gecode::dom(infer,infer.x[2],9U);
        Gecode::ite(infer,infer.control,infer.x[0],infer.x[1],infer.x[2]);
        if ((infer.status() == Gecode::SS_FAILED) ||
            !infer.control.assigned() || (infer.control.val() != 0) ||
            !infer.x[1].assigned() || (infer.x[1].val() != 9U) ||
            (Gecode::PropagatorGroup::all.size(infer) != 0U))
          return false;

        ConditionalSpace width_one(1);
        Gecode::dom(width_one,width_one.x[0],0U);
        Gecode::dom(width_one,width_one.x[1],1U);
        Gecode::dom(width_one,width_one.x[2],0U);
        Gecode::ite(width_one,width_one.control,width_one.x[0],
                    width_one.x[1],width_one.x[2]);
        if ((width_one.status() == Gecode::SS_FAILED) ||
            !width_one.control.assigned() || (width_one.control.val() != 1))
          return false;

        ConditionalSpace width_sixty_four(64);
        Gecode::ite(width_sixty_four,width_sixty_four.control,
                    width_sixty_four.x[0],width_sixty_four.x[1],
                    width_sixty_four.x[2]);
        return (width_sixty_four.status() != Gecode::SS_FAILED) &&
          (Gecode::PropagatorGroup::all.size(width_sixty_four) == 1U);
      }

      static bool search_recomputation(void) {
        using namespace Gecode;
        class SearchSpace : public Space {
        public:
          BoolVar control;
          WordVar then_word;
          WordVar else_word;
          WordVar result;
          SearchSpace(void)
            : control(*this,0,1), then_word(*this,1), else_word(*this,1),
              result(*this,1) {
            ite(*this,control,then_word,else_word,result);
            WordVarArgs decision = {then_word,else_word};
            branch(*this,decision,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
            branch(*this,control,BOOL_VAL_MIN());
          }
          SearchSpace(SearchSpace& s) : Space(s) {
            control.update(*this,s.control);
            then_word.update(*this,s.then_word);
            else_word.update(*this,s.else_word);
            result.update(*this,s.result);
          }
          virtual Space* copy(void) { return new SearchSpace(*this); }
        };

        SearchSpace* root = new SearchSpace;
        Search::Options options;
        options.c_d = 8;
        options.a_d = 64;
        DFS<SearchSpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (SearchSpace* solution=dfs.next()) {
          const WordValue expected=solution->control.val() ?
            solution->then_word.val() : solution->else_word.val();
          const bool ok=solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8U;
      }

    public:
      Lifecycle(void) : Base("Word::Conditional::Lifecycle") {}
      virtual bool run(void) {
        if (!partial_mask())
          return false;

        ConditionalSpace fixed;
        Gecode::dom(fixed,fixed.x[0],1U);
        Gecode::dom(fixed,fixed.x[1],2U);
        Gecode::dom(fixed,fixed.x[2],1U);
        Gecode::rel(fixed,fixed.control,Gecode::IRT_EQ,1);
        Gecode::ite(fixed,fixed.control,fixed.x[0],fixed.x[1],fixed.x[2]);
        if ((fixed.status() == Gecode::SS_FAILED) ||
            !fixed.x[2].assigned() || (fixed.x[2].val() != 1U))
          return false;

        ConditionalSpace equal;
        Gecode::dom(equal,equal.x[0],1U,3U);
        Gecode::ite(equal,equal.control,equal.x[0],equal.x[0],equal.x[1]);
        if ((equal.status() == Gecode::SS_FAILED) ||
            (equal.x[1].lo() != 1U) || (equal.x[1].hi() != 3U) ||
            (Gecode::PropagatorGroup::all.size(equal) != 1U))
          return false;

        ConditionalSpace constants;
        Gecode::dom(constants,constants.x[0],2U);
        Gecode::dom(constants,constants.x[1],1U);
        Gecode::ite(constants,constants.control,2,3U,
                    constants.x[0],constants.x[2]);
        Gecode::rel(constants,constants.control,Gecode::IRT_EQ,1);
        if ((constants.status() == Gecode::SS_FAILED) ||
            !constants.x[2].assigned() || (constants.x[2].val() != 3U))
          return false;

        ConditionalSpace alias;
        Gecode::dom(alias,alias.x[1],3U);
        Gecode::dom(alias,alias.x[2],0U);
        Gecode::ite(alias,alias.x[0],alias.x[1],alias.x[2],alias.x[0]);
        if (alias.status() == Gecode::SS_FAILED)
          return false;

        ConditionalSpace failed;
        Gecode::dom(failed,failed.x[0],2U);
        Gecode::dom(failed,failed.x[1],1U);
        Gecode::dom(failed,failed.x[2],1U);
        Gecode::rel(failed,failed.control,Gecode::IRT_EQ,1);
        Gecode::ite(failed,failed.control,
                    failed.x[0],failed.x[1],failed.x[2]);
        if (failed.status() != Gecode::SS_FAILED)
          return false;

        ConditionalSpace source;
        Gecode::ite(source,source.control,
                    source.x[0],source.x[1],source.x[2]);
        if ((source.status() == Gecode::SS_FAILED) ||
            (Gecode::PropagatorGroup::all.size(source) != 1U))
          return false;
        ConditionalSpace* clone =
          static_cast<ConditionalSpace*>(source.clone());
        Gecode::rel(*clone,clone->control,Gecode::IRT_EQ,0);
        Gecode::dom(*clone,clone->x[1],2U);
        const bool clone_ok = (clone->status() != Gecode::SS_FAILED) &&
          clone->x[2].assigned() && (clone->x[2].val() == 2U) &&
          !source.x[2].assigned();
        delete clone;
        if (!clone_ok)
          return false;

        try {
          ConditionalSpace mismatch;
          Gecode::WordVar other(mismatch,3);
          Gecode::ite(mismatch,mismatch.control,
                      mismatch.x[0],other,mismatch.x[2]);
          return false;
        } catch (const Gecode::Word::WidthMismatch&) {}
        return native_propagation() && search_recomputation();
      }
    };

    class Bounded : public Base {
    private:
      using WordValue = Gecode::WordValue;

      static WordValue encode(Gecode::WordDomainType kind,
                              unsigned int width, WordValue rank) {
        return rank ^ ((kind == Gecode::WDT_SIGNED) ?
                       (WordValue(1) << (width-1)) : 0U);
      }

      class BoundedSpace : public Gecode::Space {
      public:
        Gecode::BoolVar control;
        Gecode::WordVar then_word, else_word, result;
        BoundedSpace(unsigned int width, Gecode::WordDomainType kind,
                     WordValue tmin, WordValue tmax,
                     WordValue emin, WordValue emax,
                     WordValue rmin, WordValue rmax)
          : control(*this,0,1),
            then_word(*this,width,kind,encode(kind,width,tmin),
                      encode(kind,width,tmax)),
            else_word(*this,width,kind,encode(kind,width,emin),
                      encode(kind,width,emax)),
            result(*this,width,kind,encode(kind,width,rmin),
                   encode(kind,width,rmax)) {
          Gecode::ite(*this,control,then_word,else_word,result);
        }
        BoundedSpace(BoundedSpace& s) : Gecode::Space(s) {
          control.update(*this,s.control);
          then_word.update(*this,s.then_word);
          else_word.update(*this,s.else_word);
          result.update(*this,s.result);
        }
        Gecode::Space* copy(void) { return new BoundedSpace(*this); }
      };

      static WordValue minimum_rank(const Gecode::WordVar& x,
                                    Gecode::WordDomainType kind,
                                    unsigned int width) {
        return Gecode::Word::rank(kind,width,x.minimum());
      }

      static WordValue maximum_rank(const Gecode::WordVar& x,
                                    Gecode::WordDomainType kind,
                                    unsigned int width) {
        return Gecode::Word::rank(kind,width,x.maximum());
      }

      static bool exhaustive_intervals(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED})
          for (unsigned int width=1; width<=3; width++) {
            const WordValue n=WordValue(1) << width;
            for (WordValue tmin=0; tmin<n; tmin++)
              for (WordValue tmax=tmin; tmax<n; tmax++)
                for (WordValue emin=0; emin<n; emin++)
                  for (WordValue emax=emin; emax<n; emax++)
                    for (WordValue rmin=0; rmin<n; rmin++)
                      for (WordValue rmax=rmin; rmax<n; rmax++) {
                        const bool then_support =
                          (tmin <= rmax) && (rmin <= tmax);
                        const bool else_support =
                          (emin <= rmax) && (rmin <= emax);
                        BoundedSpace s(width,kind,tmin,tmax,emin,emax,
                                       rmin,rmax);
                        const bool failed=s.status() == SS_FAILED;
                        if (failed == (then_support || else_support)) {
                          ::Test::olog << "bounded ITE oracle failure width="
                            << width << " kind=" << kind << " intervals="
                            << tmin << ',' << tmax << ';'
                            << emin << ',' << emax << ';'
                            << rmin << ',' << rmax << std::endl;
                          return false;
                        }
                        if (failed)
                          continue;
                        if (s.control.assigned() &&
                            (s.control.val() ? !then_support : !else_support))
                          return false;
                        if (then_support != else_support &&
                            (!s.control.assigned() ||
                             (s.control.val() != (then_support ? 1 : 0))))
                          return false;
                        if (then_support && else_support &&
                            s.control.assigned())
                          return false;

                        WordValue expected_min, expected_max;
                        if (!then_support) {
                          expected_min=std::max(emin,rmin);
                          expected_max=std::min(emax,rmax);
                        } else if (!else_support) {
                          expected_min=std::max(tmin,rmin);
                          expected_max=std::min(tmax,rmax);
                        } else {
                          expected_min=std::max(rmin,std::min(tmin,emin));
                          expected_max=std::min(rmax,std::max(tmax,emax));
                        }
                        if ((minimum_rank(s.result,kind,width) != expected_min) ||
                            (maximum_rank(s.result,kind,width) != expected_max))
                          return false;

                        for (WordValue value=0; value<n; value++) {
                          const WordValue rank=
                            Gecode::Word::rank(kind,width,value);
                          const bool t_supported = then_support ?
                            ((rank >= std::max(tmin,rmin)) &&
                             (rank <= std::min(tmax,rmax))) : false;
                          const bool e_supported = else_support ?
                            ((rank >= std::max(emin,rmin)) &&
                             (rank <= std::min(emax,rmax))) : false;
                          if ((t_supported && !s.then_word.in(value)) ||
                              (e_supported && !s.else_word.in(value)) ||
                              ((t_supported || e_supported) &&
                               !s.result.in(value)))
                            return false;
                        }
                      }
          }
        return true;
      }

      static bool focused_lifecycle(void) {
        using namespace Gecode;
        for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED}) {
          BoundedSpace hull(3,kind,1,2,5,6,0,7);
          if ((hull.status() == SS_FAILED) || hull.control.assigned() ||
              (minimum_rank(hull.result,kind,3) != 1U) ||
              (maximum_rank(hull.result,kind,3) != 6U))
            return false;

          // [3,4] and [1,2] have overlapping cubes, so this decision needs
          // ranked interval disjointness in addition to the cube test.
          BoundedSpace disjoint(3,kind,3,4,1,2,1,2);
          if ((disjoint.status() == SS_FAILED) || !disjoint.control.zero() ||
              (minimum_rank(disjoint.result,kind,3) != 1U) ||
              (maximum_rank(disjoint.result,kind,3) != 2U))
            return false;

          BoundedSpace selected_then(3,kind,1,4,5,6,3,7);
          rel(selected_then,selected_then.control,IRT_EQ,1);
          if ((selected_then.status() == SS_FAILED) ||
              (minimum_rank(selected_then.then_word,kind,3) != 3U) ||
              (maximum_rank(selected_then.result,kind,3) != 4U))
            return false;
          BoundedSpace selected_else(3,kind,1,2,4,7,0,5);
          rel(selected_else,selected_else.control,IRT_EQ,0);
          if ((selected_else.status() == SS_FAILED) ||
              (minimum_rank(selected_else.result,kind,3) != 4U) ||
              (maximum_rank(selected_else.else_word,kind,3) != 5U))
            return false;
        }

        class Duplicate : public Space {
        public:
          BoolVar control;
          WordVar branch, result;
          Duplicate(void)
            : control(*this,0,1), branch(*this,4,WDT_UNSIGNED,2U,9U),
              result(*this,4,WDT_UNSIGNED,5U,12U) {
            ite(*this,control,branch,branch,result);
          }
          Duplicate(Duplicate& s) : Space(s) {
            control.update(*this,s.control); branch.update(*this,s.branch);
            result.update(*this,s.result);
          }
          Space* copy(void) { return new Duplicate(*this); }
        } duplicate;
        if ((duplicate.status() == SS_FAILED) || duplicate.control.assigned() ||
            (duplicate.branch.minimum() != 5U) ||
            (duplicate.result.maximum() != 9U))
          return false;

        class ResultAlias : public Space {
        public:
          BoolVar control;
          WordVar branch, other;
          ResultAlias(void)
            : control(*this,0,1), branch(*this,3,WDT_UNSIGNED,1U,4U),
              other(*this,3,WDT_UNSIGNED,5U,6U) {
            ite(*this,control,branch,other,branch);
          }
          ResultAlias(ResultAlias& s) : Space(s) {
            control.update(*this,s.control); branch.update(*this,s.branch);
            other.update(*this,s.other);
          }
          Space* copy(void) { return new ResultAlias(*this); }
        } alias;
        if (alias.status() == SS_FAILED)
          return false;

        BoundedSpace source(3,WDT_UNSIGNED,1,3,5,7,0,7);
        if ((source.status() == SS_FAILED) || source.control.assigned())
          return false;
        BoundedSpace* clone=static_cast<BoundedSpace*>(source.clone());
        dom(*clone,clone->result,6U);
        const bool clone_ok=(clone->status() != SS_FAILED) &&
          clone->control.zero() && clone->else_word.assigned() &&
          (clone->else_word.val() == 6U) && !source.control.assigned();
        delete clone;
        return clone_ok;
      }

      static bool replay(void) {
        using namespace Gecode;
        class ReplaySpace : public Space {
        public:
          BoolVar control;
          WordVar then_word, else_word, result;
          ReplaySpace(void)
            : control(*this,0,1),
              then_word(*this,2,WDT_UNSIGNED,0U,1U),
              else_word(*this,2,WDT_UNSIGNED,2U,3U),
              result(*this,2,WDT_UNSIGNED) {
            ite(*this,control,then_word,else_word,result);
            WordVarArgs words={then_word,else_word};
            branch(*this,words,WORD_VAR_SIZE_MIN(),WORD_VAL_SPLIT_MIN());
            branch(*this,control,BOOL_VAL_MIN());
          }
          ReplaySpace(ReplaySpace& s) : Space(s) {
            control.update(*this,s.control);
            then_word.update(*this,s.then_word);
            else_word.update(*this,s.else_word);
            result.update(*this,s.result);
          }
          Space* copy(void) { return new ReplaySpace(*this); }
        };
        ReplaySpace* root=new ReplaySpace;
        Search::Options options;
        options.c_d=64;
        options.a_d=64;
        DFS<ReplaySpace> dfs(root,options);
        delete root;
        unsigned int solutions=0;
        while (ReplaySpace* solution=dfs.next()) {
          const WordValue expected=solution->control.val() ?
            solution->then_word.val() : solution->else_word.val();
          const bool ok=solution->result.assigned() &&
            (solution->result.val() == expected) &&
            (PropagatorGroup::all.size(*solution) == 0U);
          delete solution;
          if (!ok)
            return false;
          solutions++;
        }
        return solutions == 8U;
      }

    public:
      Bounded(void) : Base("Word::Conditional::Bounded") {}
      bool run(void) {
        return exhaustive_intervals() && focused_lifecycle() && replay();
      }
    };

    Mask mask;
    Boolean boolean;
    Lifecycle lifecycle;
    Bounded bounded;

  }

}}

// STATISTICS: test-word
