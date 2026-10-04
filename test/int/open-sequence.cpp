/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

#include "test/int.hh"

#include <gecode/search.hh>

#include <memory>

namespace Test { namespace Int {

  namespace OpenSequence {

    class BranchSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      BranchSpace(void)
        : sequence(*this,
            [](int i) {
              return Gecode::IntSet(i,i+1);
            },3) {
        Gecode::branch(*this,sequence);
      }

      BranchSpace(BranchSpace& s)
        : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BranchSpace(*this);
      }
    };

    class Branch : public ::Test::Base {
    public:
      Branch(void)
        : ::Test::Base("Int::OpenSequence::Branch") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        Search::Options options;
        options.threads = 1;
        options.c_d = 1000;
        options.a_d = 1000;
        BranchSpace* root = new BranchSpace;
        DFS<BranchSpace> engine(root,options);
        delete root;

        int solutions = 0;
        while (BranchSpace* solution = engine.next()) {
          const int size = solution->sequence.size();
          bool valid =
            solution->sequence.length().assigned() &&
            (solution->sequence.length().val() == size);
          for (int i=0; i<size; i++)
            valid &= solution->sequence[i].assigned();
          delete solution;
          if (!valid)
            return false;
          solutions++;
        }
        return solutions == 15;
      }
    };

    class BranchOrderSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      BranchOrderSpace(int order)
        : sequence(*this,Gecode::IntSet(0,1),3) {
        using namespace Gecode;
        DFA accepts_one(0,{
            {0,0,0}, {0,1,1},
            {1,0,1}, {1,1,1}
          },{1});
        extensional(*this,sequence,accepts_one);
        if (order < 0)
          branch(*this,sequence);
        else
          branch(*this,sequence,static_cast<OpenIntBranch>(order));
      }

      BranchOrderSpace(BranchOrderSpace& s)
        : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BranchOrderSpace(*this);
      }
    };

    class BranchOrder : public ::Test::Base {
    protected:
      static bool
      check(int order, int first_length) {
        using namespace Gecode;
        Search::Options options;
        options.threads = 1;
        options.c_d = 1000;
        options.a_d = 1000;
        BranchOrderSpace* root = new BranchOrderSpace(order);
        DFS<BranchOrderSpace> engine(root,options);
        delete root;

        int solutions = 0;
        int first = -1;
        while (BranchOrderSpace* solution = engine.next()) {
          if (solutions == 0)
            first = solution->sequence.size();
          solutions++;
          delete solution;
        }
        return (first == first_length) && (solutions == 11);
      }

    public:
      BranchOrder(void)
        : ::Test::Base("Int::OpenSequence::BranchOrder") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        return check(-1,1) &&
          check(OIB_HORIZON_FIRST,1) &&
          check(OIB_VALUE_FIRST,3);
      }
    };

    class TransitionSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      TransitionSpace(void)
        : sequence(*this,Gecode::IntSet(0,4),
            [](Gecode::Space& home, Gecode::OpenIntVarSequence x, int i) {
              using namespace Gecode;
              if (i == 0) {
                rel(home,x[i],IRT_EQ,0);
              } else {
                linear(home,IntArgs({-1,1}),
                       IntVarArgs({x[i-1],x[i]}),IRT_EQ,1);
              }
            },4) {
        Gecode::branch(*this,sequence);
      }

      TransitionSpace(TransitionSpace& s)
        : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new TransitionSpace(*this);
      }
    };

    class Transition : public ::Test::Base {
    public:
      Transition(void)
        : ::Test::Base("Int::OpenSequence::Transition") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        Search::Options options;
        options.threads = 1;
        options.c_d = 1000;
        options.a_d = 1000;
        TransitionSpace* root = new TransitionSpace;
        DFS<TransitionSpace> engine(root,options);
        delete root;

        int solutions = 0;
        while (TransitionSpace* solution = engine.next()) {
          bool valid =
            solution->sequence.length().assigned() &&
            (solution->sequence.length().val() ==
             solution->sequence.size());
          for (int i=0; i<solution->sequence.size(); i++)
            valid &= solution->sequence[i].assigned() &&
              (solution->sequence[i].val() == i);
          delete solution;
          if (!valid)
            return false;
          solutions++;
        }
        return solutions == 5;
      }
    };


    class ConstraintSpace : public Gecode::Space {
    public:
      enum Kind {
        DISTINCT,
        REL,
        SLIDING,
        SLIDING_SUM,
        MIN_MAX,
        PRECEDE
      };

      Gecode::OpenIntVarSequence sequence;
      Gecode::IntVar minimum;
      Gecode::IntVar maximum;

      ConstraintSpace(Kind kind, int size)
        : sequence(*this,Gecode::IntSet(0,2),size),
          minimum(*this,0,2), maximum(*this,0,2) {
        using namespace Gecode;
        switch (kind) {
        case DISTINCT:
          distinct(*this,sequence);
          break;
        case REL:
          rel(*this,sequence,IRT_LE);
          break;
        case SLIDING:
          Gecode::sequence(*this,sequence,IntSet(1,1),2,1,1);
          break;
        case SLIDING_SUM:
          slidingsum(*this,sequence,2,2,2);
          break;
        case MIN_MAX:
          min(*this,sequence,minimum);
          max(*this,sequence,maximum);
          rel(*this,minimum,IRT_EQ,0);
          rel(*this,maximum,IRT_EQ,2);
          break;
        case PRECEDE:
          precede(*this,sequence,0,1);
          break;
        }
        rel(*this,sequence.length(),IRT_EQ,size);
        branch(*this,sequence);
      }

      ConstraintSpace(ConstraintSpace& s)
        : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
        minimum.update(*this,s.minimum);
        maximum.update(*this,s.maximum);
      }

      virtual Gecode::Space*
      copy(void) {
        return new ConstraintSpace(*this);
      }
    };

    int
    solutions(ConstraintSpace::Kind kind, int size,
              bool (*valid)(const ConstraintSpace&)) {
      using namespace Gecode;
      ConstraintSpace* root = new ConstraintSpace(kind,size);
      DFS<ConstraintSpace> engine(root);
      delete root;
      int n = 0;
      while (ConstraintSpace* solution = engine.next()) {
        const bool ok = valid(*solution);
        delete solution;
        if (!ok)
          return -1;
        n++;
      }
      return n;
    }

    bool
    distinct_solution(const ConstraintSpace& s) {
      for (int i=0; i<s.sequence.size(); i++)
        for (int j=0; j<i; j++)
          if (s.sequence[i].val() == s.sequence[j].val())
            return false;
      return true;
    }

    bool
    rel_solution(const ConstraintSpace& s) {
      for (int i=1; i<s.sequence.size(); i++)
        if (s.sequence[i-1].val() >= s.sequence[i].val())
          return false;
      return true;
    }

    bool
    sliding_solution(const ConstraintSpace& s) {
      for (int i=1; i<s.sequence.size(); i++) {
        const int count =
          (s.sequence[i-1].val() == 1) + (s.sequence[i].val() == 1);
        if (count != 1)
          return false;
      }
      return true;
    }

    bool
    min_max_solution(const ConstraintSpace& s) {
      int lower = s.sequence[0].val();
      int upper = lower;
      for (int i=1; i<s.sequence.size(); i++) {
        lower = std::min(lower,s.sequence[i].val());
        upper = std::max(upper,s.sequence[i].val());
      }
      return (lower == s.minimum.val()) && (upper == s.maximum.val());
    }

    bool
    sliding_sum_solution(const ConstraintSpace& s) {
      for (int i=1; i<s.sequence.size(); i++)
        if (s.sequence[i-1].val() + s.sequence[i].val() != 2)
          return false;
      return true;
    }

    bool
    precede_solution(const ConstraintSpace& s) {
      bool seen = false;
      for (int i=0; i<s.sequence.size(); i++) {
        if ((s.sequence[i].val() == 1) && !seen)
          return false;
        seen |= s.sequence[i].val() == 0;
      }
      return true;
    }

    class Constraint : public ::Test::Base {
    protected:
      ConstraintSpace::Kind kind;
      int size;
      int expected;
      bool (*valid)(const ConstraintSpace&);
    public:
      Constraint(const std::string& name, ConstraintSpace::Kind kind0,
                 int size0, int expected0,
                 bool (*valid0)(const ConstraintSpace&))
        : ::Test::Base("Int::OpenSequence::"+name),
          kind(kind0), size(size0), expected(expected0), valid(valid0) {}

      virtual bool
      run(void) {
        return solutions(kind,size,valid) == expected;
      }
    };

    class Precede : public ::Test::Base {
    public:
      Precede(void)
        : ::Test::Base("Int::OpenSequence::Precede") {}

      virtual bool
      run(void) {
        int expected = 0;
        for (int x0=0; x0<=2; x0++)
          for (int x1=0; x1<=2; x1++)
            for (int x2=0; x2<=2; x2++) {
              bool seen = false;
              bool valid = true;
              const int values[3] = {x0,x1,x2};
              for (int i=0; i<3; i++) {
                if ((values[i] == 1) && !seen)
                  valid = false;
                seen |= values[i] == 0;
              }
              expected += valid;
            }
        return solutions(ConstraintSpace::PRECEDE,3,
                         precede_solution) == expected;
      }
    };

    class EmptyMinimum : public ::Test::Base {
    public:
      EmptyMinimum(void)
        : ::Test::Base("Int::OpenSequence::EmptyMinimum") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        class MinimumSpace : public Space {
        public:
          OpenIntVarSequence sequence;
          IntVar result;

          MinimumSpace(void)
            : sequence(*this,0), result(*this,0,1) {
            min(*this,sequence,result);
            sequence.close(*this);
          }

          MinimumSpace(MinimumSpace& s)
            : Space(s) {
            sequence.update(*this,s.sequence);
            result.update(*this,s.result);
          }

          virtual Space*
          copy(void) {
            return new MinimumSpace(*this);
          }
        } space;
        return space.status() == SS_FAILED;
      }
    };

    class IncrementalConstraints : public ::Test::Base {
    public:
      IncrementalConstraints(void)
        : ::Test::Base("Int::OpenSequence::IncrementalConstraints") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        class IncrementalSpace : public Space {
        public:
          OpenIntVarSequence sequence;
          IntVar minimum;
          IntVar maximum;

          IncrementalSpace(void)
            : sequence(*this,IntSet(0,2),2),
              minimum(*this,0,2), maximum(*this,0,2) {
            distinct(*this,sequence);
            rel(*this,sequence,IRT_LE);
            Gecode::sequence(*this,sequence,IntSet(2,2),2,1,1);
            slidingsum(*this,sequence,2,2,2);
            min(*this,sequence,minimum);
            max(*this,sequence,maximum);
            precede(*this,sequence,0,2);
          }

          IncrementalSpace(IncrementalSpace& s)
            : Space(s) {
            sequence.update(*this,s.sequence);
            minimum.update(*this,s.minimum);
            maximum.update(*this,s.maximum);
          }

          virtual Space*
          copy(void) {
            return new IncrementalSpace(*this);
          }
        } source;

        source.sequence.materialize(source,1);
        rel(source,source.sequence[0],IRT_EQ,0);
        if ((source.status() == SS_FAILED) ||
            !source.sequence[0].assigned())
          return false;

        IncrementalSpace* clone =
          static_cast<IncrementalSpace*>(source.clone());
        clone->sequence.materialize(*clone,2);
        clone->sequence.close(*clone);
        const bool valid =
          (clone->status() != SS_FAILED) &&
          clone->sequence[1].assigned() &&
          (clone->sequence[1].val() == 2) &&
          clone->minimum.assigned() && (clone->minimum.val() == 0) &&
          clone->maximum.assigned() && (clone->maximum.val() == 2);
        delete clone;
        return valid && (source.sequence.size() == 1);
      }
    };

    class CallbackSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      CallbackSpace(Gecode::OpenIntVarSequence::Domain domain,
                    Gecode::OpenIntVarSequence::Transition transition)
        : sequence(*this,domain,transition,8) {}

      CallbackSpace(CallbackSpace& s) : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new CallbackSpace(*this);
      }
    };

    class Dispose : public ::Test::Base {
    public:
      Dispose(void)
        : ::Test::Base("Int::OpenSequence::Dispose") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        std::shared_ptr<int> domain(new int(0));
        std::shared_ptr<int> transition(new int(0));
        CallbackSpace* source = new CallbackSpace(
          [domain](int) { return IntSet(0,1); },
          [transition](Space&, OpenIntVarSequence, int) {});
        source->sequence.materialize(*source,2);
        DFA dfa(0,{{0,0,0},{0,1,0}},{0});
        extensional(*source,source->sequence,dfa);
        if (source->status() == SS_FAILED) {
          delete source;
          return false;
        }
        CallbackSpace* clone = static_cast<CallbackSpace*>(source->clone());
        delete source;
        const bool retained = (domain.use_count() > 1) &&
                              (transition.use_count() > 1);
        delete clone;
        return retained && (domain.use_count() == 1) &&
                           (transition.use_count() == 1);
      }
    };

    class CallbackLength : public ::Test::Base {
    public:
      CallbackLength(void)
        : ::Test::Base("Int::OpenSequence::CallbackLength") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int assign=0; assign<2; assign++)
          for (int close=0; close<2; close++) {
            CallbackSpace source([](int) { return IntSet(0,1); },
              [assign](Space& home, OpenIntVarSequence sequence, int i) {
                if (i == 0)
                  rel(home,sequence.length(),assign ? IRT_EQ : IRT_GQ,3);
              });
            rel(source,source.sequence.length(),IRT_GQ,1);
            if (close)
              source.sequence.close(source);
            if ((source.status() == SS_FAILED) ||
                (source.sequence.size() != 3))
              return false;
          }
        return true;
      }
    };

    class FailedGet : public ::Test::Base {
    public:
      FailedGet(void)
        : ::Test::Base("Int::OpenSequence::FailedGet") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(0,1); },
          [](Space& home, OpenIntVarSequence, int) { home.fail(); });
        IntVar x = source.sequence.get(source,4);
        return source.failed() && (x.varimp() == nullptr);
      }
    };

    class ValueCommit : public ::Test::Base {
    public:
      ValueCommit(void)
        : ::Test::Base("Int::OpenSequence::ValueCommit") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(0,1); },
          [](Space& home, OpenIntVarSequence, int i) {
            if (i == 1)
              home.fail();
          });
        source.sequence.materialize(source,1);
        branch(source,source.sequence,OIB_VALUE_FIRST);
        if (source.status() != SS_BRANCH)
          return false;
        const Choice* choice = source.choice();
        bool valid = true;
        for (unsigned int alternative=0; alternative<2; alternative++) {
          CallbackSpace* clone = static_cast<CallbackSpace*>(source.clone());
          rel(*clone,clone->sequence.length(),IRT_GQ,2);
          clone->commit(*choice,alternative);
          valid &= !clone->failed() && (clone->sequence.size() == 1) &&
            clone->sequence[0].assigned() &&
            (clone->sequence[0].val() == static_cast<int>(alternative));
          valid &= clone->status() == SS_FAILED;
          delete clone;
        }
        delete choice;
        return valid;
      }
    };

    class ImpossibleWindow : public ::Test::Base {
    public:
      ImpossibleWindow(void)
        : ::Test::Base("Int::OpenSequence::ImpossibleWindow") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int sum=0; sum<2; sum++)
          for (int size=0; size<3; size++) {
            CallbackSpace source([](int) { return IntSet(0,1); },
              [](Space&, OpenIntVarSequence, int) {});
            if (sum)
              slidingsum(source,source.sequence,2,2,1);
            else
              Gecode::sequence(source,source.sequence,IntSet(0,1),2,3,3);
            rel(source,source.sequence.length(),IRT_EQ,size);
            const bool failed = source.status() == SS_FAILED;
            if (failed != (size == 2))
              return false;
            if (!failed && (source.sequence.size() != size))
              return false;
          }
        return true;
      }
    };

    ValueCommit value_commit;
    ImpossibleWindow impossible_window;

    Dispose disposal;
    CallbackLength callback_length;
    FailedGet failed_get;

    Branch branch;
    BranchOrder branch_order;
    Transition transition;
    Constraint distinct("Distinct",ConstraintSpace::DISTINCT,3,6,
                        distinct_solution);
    Constraint relation("Rel",ConstraintSpace::REL,3,1,rel_solution);
    Constraint sliding("Sliding",ConstraintSpace::SLIDING,4,8,
                       sliding_solution);
    Constraint sliding_sum("SlidingSum",ConstraintSpace::SLIDING_SUM,4,3,
                           sliding_sum_solution);
    Constraint min_max("MinMax",ConstraintSpace::MIN_MAX,3,12,
                       min_max_solution);
    Precede precedence;
    EmptyMinimum empty_minimum;
    IncrementalConstraints incremental_constraints;

  }

}}

// STATISTICS: test-int
