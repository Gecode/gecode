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

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

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
          branch(*this,sequence,static_cast<OpenVarBranch>(order));
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
          check(OVB_HORIZON_FIRST,1) &&
          check(OVB_VALUE_FIRST,3);
      }
    };

    class SelectorSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      SelectorSpace(Gecode::IntValBranch vals, Gecode::OpenVarBranch order)
        : sequence(*this,[](int i) { return Gecode::IntSet(i,i+1); },3) {
        using namespace Gecode;
        branch(*this,sequence,tiebreak(INT_VAR_SIZE_MIN(),INT_VAR_MAX_MAX()),
               vals,order);
      }

      SelectorSpace(SelectorSpace& s) : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new SelectorSpace(*this);
      }
    };

    class Selectors : public ::Test::Base {
    public:
      Selectors(void) : ::Test::Base("Int::OpenSequence::Selectors") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        const IntValBranch values[] = {
          INT_VAL_MIN(), INT_VAL_MAX(), INT_VAL_MED(), INT_VAL_RND(Rnd(1)),
          INT_VAL_SPLIT_MIN(), INT_VAL_SPLIT_MAX(),
          INT_VAL_RANGE_MIN(), INT_VAL_RANGE_MAX(),
          INT_VALUES_MIN(), INT_VALUES_MAX(),
          INT_VAL([](const Space&, IntVar x, int) { return x.max(); })
        };
        for (const IntValBranch& vals : values)
          for (int order=0; order<2; order++) {
            SelectorSpace root(vals,static_cast<OpenVarBranch>(order));
            Search::Options options;
            options.threads = 1;
            options.c_d = 1000;
            options.a_d = 1000;
            DFS<SelectorSpace> engine(&root,options);
            int solutions = 0;
            while (SelectorSpace* solution = engine.next()) {
              bool valid = solution->sequence.length().assigned() &&
                (solution->sequence.length().val() == solution->sequence.size());
              for (int i=0; i<solution->sequence.size(); i++)
                valid &= solution->sequence[i].assigned() &&
                  (solution->sequence[i].val() >= i) &&
                  (solution->sequence[i].val() <= i+1);
              delete solution;
              if (!valid)
                return false;
              solutions++;
            }
            if (solutions != 15)
              return false;
          }
        return true;
      }
    };

    class NoGoods : public ::Test::Base {
    public:
      NoGoods(void) : ::Test::Base("Int::OpenSequence::NoGoods") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        SelectorSpace root(INT_VAL_MIN(),OVB_VALUE_FIRST);
        root.sequence.materialize(root,1);
        Search::NodeStop stop(6);
        Search::Options options;
        options.stop = &stop;
        options.threads = 1;
        options.nogoods_limit = 100;
        std::unique_ptr<Search::Engine> engine
          (Search::dfsengine(&root,options));
        bool found[16] = {};
        const auto record = [&found](SelectorSpace* solution) {
          int code = 1;
          for (int i=0; i<solution->sequence.size(); i++)
            code = 2*code + solution->sequence[i].val()-i;
          found[code] = true;
          delete solution;
        };
        while (Space* solution = engine->next())
          record(static_cast<SelectorSpace*>(solution));
        if (!engine->stopped())
          return false;
        // Deeper choices refer to positions absent from the restart root.
        engine->nogoods().post(root);
        DFS<SelectorSpace> restarted(&root);
        while (SelectorSpace* solution = restarted.next())
          record(solution);
        // Extraction may omit literals, but must retain every unvisited word.
        for (int code=2; code<16; code++)
          if (!found[code])
            return false;
        return true;
      }
    };

    class SelectorChoiceSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarSequence sequence;

      SelectorChoiceSpace(Gecode::IntValBranch vals)
        : sequence(*this,Gecode::IntSet({0,2,5}),3) {
        using namespace Gecode;
        sequence.materialize(*this,3);
        branch(*this,sequence,
               tiebreak(INT_VAR_SIZE_MIN(),
                        INT_VAR_MERIT_MAX([](const Space&, IntVar, int i) {
                          return i;
                        })),vals,OVB_VALUE_FIRST,
               [](const Space&, IntVar, int i) { return i != 1; },
               [](const Space&, const Brancher&, unsigned int, IntVar,
                  int i, const int& value, std::ostream& out) {
                 out << i << ":" << value;
               });
      }

      SelectorChoiceSpace(SelectorChoiceSpace& s) : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new SelectorChoiceSpace(*this);
      }
    };

    class SelectorChoice : public ::Test::Base {
    public:
      SelectorChoice(void) : ::Test::Base("Int::OpenSequence::SelectorChoice") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int multi=0; multi<2; multi++) {
          SelectorChoiceSpace source(multi ? INT_VALUES_MAX() : INT_VAL_MAX());
          if (source.status() != SS_BRANCH)
            return false;
          const Choice* original = source.choice();
          Archive archive;
          original->archive(archive);
          const Choice* restored = source.choice(archive);
          bool valid = restored->alternatives() == (multi ? 3U : 2U);
          for (unsigned int a=0; a<restored->alternatives(); a++) {
            SelectorChoiceSpace* clone =
              static_cast<SelectorChoiceSpace*>(source.clone());
            clone->commit(*restored,a);
            const int expected[] = {5,2,0};
            valid &= !clone->failed() && !clone->sequence[0].assigned() &&
              !clone->sequence[1].assigned();
            if (multi || (a == 0))
              valid &= clone->sequence[2].assigned() &&
                (clone->sequence[2].val() == expected[a]);
            else
              valid &= !clone->sequence[2].in(5);
            delete clone;
          }
          SelectorChoiceSpace* excluded =
            static_cast<SelectorChoiceSpace*>(source.clone());
          NGL* literal = excluded->ngl(*restored,0);
          valid &= (literal != nullptr) && (literal->status(*excluded) == NGL::NONE);
          if (literal)
            valid &= (literal->prune(*excluded) != ES_FAILED) &&
              !excluded->sequence[2].in(5);
          delete excluded;
          std::ostringstream printed;
          source.print(*restored,0,printed);
          valid &= printed.str() == "2:5";
          delete restored;
          delete original;
          if (!valid)
            return false;
        }
        return true;
      }
    };

    class BoolSpace : public Gecode::Space {
    public:
      Gecode::OpenBoolVarSequence sequence;

      BoolSpace(Gecode::OpenVarBranch order)
        : sequence(*this,3) {
        using namespace Gecode;
        DFA accepts_one(0,{{0,0,0},{0,1,1},{1,0,1},{1,1,1}},{1});
        extensional(*this,sequence,accepts_one);
        branch(*this,sequence,
               BOOL_VAR_MERIT_MAX([](const Space&, BoolVar, int i) { return i; }),
               BOOL_VAL_MAX(),order);
      }

      BoolSpace(BoolSpace& s) : Gecode::Space(s) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BoolSpace(*this);
      }
    };

    class BoolSequence : public ::Test::Base {
    public:
      BoolSequence(void) : ::Test::Base("Int::OpenSequence::Bool") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int order=0; order<2; order++)
          for (int recompute=0; recompute<2; recompute++) {
            BoolSpace root(static_cast<OpenVarBranch>(order));
            Search::Options options;
            options.threads = 1;
            options.c_d = recompute ? 1000 : 1;
            options.a_d = options.c_d;
            DFS<BoolSpace> engine(&root,options);
            int solutions = 0;
            while (BoolSpace* solution = engine.next()) {
              bool valid = solution->sequence.length().assigned() &&
                (solution->sequence.length().val() == solution->sequence.size());
              int ones = 0;
              for (int i=0; i<solution->sequence.size(); i++) {
                valid &= solution->sequence[i].assigned();
                ones += solution->sequence[i].val();
              }
              delete solution;
              if (!valid || (ones == 0))
                return false;
              solutions++;
            }
            if (solutions != 11)
              return false;
          }
        return true;
      }
    };

    class FactorySpace : public Gecode::Space {
    public:
#ifdef GECODE_HAS_SET_VARS
      Gecode::OpenVarSequence<Gecode::SetVar> sets;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
      Gecode::OpenVarSequence<Gecode::FloatVar> floats;
#endif

      FactorySpace(void) {
        using namespace Gecode;
#ifdef GECODE_HAS_SET_VARS
        sets = OpenVarSequence<SetVar>(*this,
          [](Space& home, int i) {
            return SetVar(home,IntSet(i,i),IntSet(i,i));
          },3);
        sets.materialize(*this,1);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        floats = OpenVarSequence<FloatVar>(*this,
          [](Space& home, int i) { return FloatVar(home,i,i); },3);
        floats.materialize(*this,1);
#endif
      }

      FactorySpace(FactorySpace& s) : Gecode::Space(s) {
#ifdef GECODE_HAS_SET_VARS
        sets.update(*this,s.sets);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        floats.update(*this,s.floats);
#endif
      }

      virtual Gecode::Space*
      copy(void) {
        return new FactorySpace(*this);
      }
    };

    class Factory : public ::Test::Base {
    public:
      Factory(void) : ::Test::Base("Int::OpenSequence::Factory") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        FactorySpace source;
        if (source.status() == SS_FAILED)
          return false;
        FactorySpace* clone = static_cast<FactorySpace*>(source.clone());
#ifdef GECODE_HAS_SET_VARS
        clone->sets.materialize(*clone,3);
        clone->sets.close(*clone);
        source.sets.close(source);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        clone->floats.materialize(*clone,3);
        clone->floats.close(*clone);
        source.floats.close(source);
#endif
        bool valid = (clone->status() != SS_FAILED) &&
                     (source.status() != SS_FAILED);
#ifdef GECODE_HAS_SET_VARS
        valid &= (source.sets.size() == 1) && (clone->sets.size() == 3) &&
          (clone->sets.length().val() == 3) &&
          clone->sets[2].assigned() && (clone->sets[2].glbMin() == 2);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        valid &= (source.floats.size() == 1) && (clone->floats.size() == 3) &&
          (clone->floats.length().val() == 3) &&
          clone->floats[2].assigned() && (clone->floats[2].min() == 2);
#endif
        delete clone;
        return valid;
      }
    };

    class FixedStatistics : public ::Test::Base {
    public:
      FixedStatistics(void) : ::Test::Base("Int::OpenSequence::FixedStatistics") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int chb=0; chb<2; chb++) {
          SelectorSpace open(INT_VAL_MIN(),OVB_VALUE_FIRST);
          bool rejected = false;
          try {
            branch(open,open.sequence,
                   chb ? INT_VAR_CHB_MIN() : INT_VAR_ACTION_MIN(1.0),
                   INT_VAL_MIN());
          } catch (const Gecode::Int::UnknownBranching&) {
            rejected = true;
          }
          if (!rejected)
            return false;
          SelectorChoiceSpace fixed(INT_VAL_MIN());
          branch(fixed,fixed.sequence,
                 chb ? INT_VAR_CHB_MIN() : INT_VAR_ACTION_MIN(1.0),
                 INT_VAL_MIN());
          DFS<SelectorChoiceSpace> engine(&fixed);
          int solutions = 0;
          while (SelectorChoiceSpace* solution = engine.next()) {
            solutions++;
            delete solution;
          }
          if (solutions != 27)
            return false;
        }
        return true;
      }
    };

    Factory factory;
    FixedStatistics fixed_statistics;
    Selectors selectors;
    NoGoods no_goods;
    SelectorChoice selector_choice;
    BoolSequence bool_sequence;

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

    class GroupSpace : public Gecode::Space {
    public:
      enum Kind { REL, SUM, PRECEDE };
      Gecode::OpenIntVarSequence sequence;
      Gecode::PropagatorGroup group;

      GroupSpace(Kind kind, bool close)
        : sequence(*this,Gecode::IntSet(0,2),3) {
        using namespace Gecode;
        switch (kind) {
        case REL:
          rel((*this)(group),sequence,IRT_NQ);
          break;
        case SUM:
          slidingsum((*this)(group),sequence,2,2,2);
          break;
        case PRECEDE:
          precede((*this)(group),sequence,0,1);
          break;
        }
        sequence.materialize(*this,2);
        if (close)
          sequence.close(*this);
      }

      GroupSpace(GroupSpace& s) : Gecode::Space(s), group(s.group) {
        sequence.update(*this,s.sequence);
      }

      virtual Gecode::Space*
      copy(void) {
        return new GroupSpace(*this);
      }
    };

    /// Test group control after an open constraint has posted its children
    class Group : public ::Test::Base {
    protected:
      GroupSpace::Kind kind;
      bool kill;
    public:
      Group(const std::string& name, GroupSpace::Kind kind0, bool kill0)
        : ::Test::Base("Int::OpenSequence::Group::"+name+
                       (kill0 ? "::Kill" : "::Disable")),
          kind(kind0), kill(kill0) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int close=0; close<2; close++) {
          GroupSpace source(kind,close != 0);
          if (source.status() == SS_FAILED)
            return false;
          if (kill)
            source.group.kill(source);
          else
            source.group.disable(source);
          // These values remain in the domains but violate each constraint.
          rel(source,source.sequence[0],IRT_EQ,
              kind == GroupSpace::PRECEDE ? 2 : 0);
          rel(source,source.sequence[1],IRT_EQ,
              kind == GroupSpace::PRECEDE ? 1 : 0);
          if (source.status() == SS_FAILED)
            return false;
          if (!kill) {
            source.group.enable(source);
            if (source.status() != SS_FAILED)
              return false;
          }
        }
        return true;
      }
    };

    Group group_rel_kill("Rel",GroupSpace::REL,true);
    Group group_rel_disable("Rel",GroupSpace::REL,false);
    Group group_sum_kill("Sum",GroupSpace::SUM,true);
    Group group_sum_disable("Sum",GroupSpace::SUM,false);
    Group group_precede_kill("Precede",GroupSpace::PRECEDE,true);
    Group group_precede_disable("Precede",GroupSpace::PRECEDE,false);

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
        branch(source,source.sequence,OVB_VALUE_FIRST);
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
