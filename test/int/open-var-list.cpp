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

  namespace OpenList {

    class BranchSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      BranchSpace(void)
        : list(*this,
            [](int i) {
              return Gecode::IntSet(i,i+1);
            },3) {
        Gecode::branch(*this,list);
      }

      BranchSpace(BranchSpace& s)
        : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BranchSpace(*this);
      }
    };

    class Branch : public ::Test::Base {
    public:
      Branch(void)
        : ::Test::Base("Int::OpenList::Branch",TestTags::check()) {}

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
          const int size = solution->list.prefix_size();
          bool valid =
            solution->list.length().assigned() &&
            (solution->list.length().val() == size);
          for (int i=0; i<size; i++)
            valid &= solution->list[i].assigned();
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
      Gecode::OpenIntVarList list;

      enum Order { INCREMENTAL, LENGTH_MIN, LENGTH_MAX };

      BranchOrderSpace(Order order)
        : list(*this,Gecode::IntSet(0,1),3) {
        using namespace Gecode;
        DFA accepts_one(0,{
            {0,0,0}, {0,1,1},
            {1,0,1}, {1,1,1}
          },{1});
        extensional(*this,list,accepts_one);
        if (order == LENGTH_MIN)
          branch(*this,list.length(),INT_VAL_MIN());
        else if (order == LENGTH_MAX)
          branch(*this,list.length(),INT_VAL_MAX());
        branch(*this,list);
      }

      BranchOrderSpace(BranchOrderSpace& s)
        : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BranchOrderSpace(*this);
      }
    };

    class BranchOrder : public ::Test::Base {
    protected:
      static bool
      check(BranchOrderSpace::Order order, int first_length) {
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
            first = solution->list.prefix_size();
          solutions++;
          delete solution;
        }
        return (first == first_length) && (solutions == 11);
      }

    public:
      BranchOrder(void)
        : ::Test::Base("Int::OpenList::BranchOrder") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        return check(BranchOrderSpace::INCREMENTAL,3) &&
          check(BranchOrderSpace::LENGTH_MIN,1) &&
          check(BranchOrderSpace::LENGTH_MAX,3);
      }
    };

    class SelectorSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      SelectorSpace(Gecode::IntValBranch vals, bool length_first=false)
        : list(*this,[](int i) { return Gecode::IntSet(i,i+1); },3) {
        using namespace Gecode;
        if (length_first)
          branch(*this,list.length(),INT_VAL_MIN());
        branch(*this,list,tiebreak(INT_VAR_SIZE_MIN(),INT_VAR_MAX_MAX()),vals);
      }

      SelectorSpace(SelectorSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new SelectorSpace(*this);
      }
    };

    class Selectors : public ::Test::Base {
    public:
      Selectors(void) : ::Test::Base("Int::OpenList::Selectors") {}

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
          for (bool length_first : {false,true}) {
            SelectorSpace root(vals,length_first);
            Search::Options options;
            options.threads = 1;
            options.c_d = 1000;
            options.a_d = 1000;
            DFS<SelectorSpace> engine(&root,options);
            int solutions = 0;
            while (SelectorSpace* solution = engine.next()) {
              bool valid = solution->list.length().assigned() &&
                (solution->list.length().val() == solution->list.prefix_size());
              for (int i=0; i<solution->list.prefix_size(); i++)
                valid &= solution->list[i].assigned() &&
                  (solution->list[i].val() >= i) &&
                  (solution->list[i].val() <= i+1);
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
      NoGoods(void) : ::Test::Base("Int::OpenList::NoGoods") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        SelectorSpace root(INT_VAL_MIN());
        root.list.materialize(root,1);
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
          for (int i=0; i<solution->list.prefix_size(); i++)
            code = 2*code + solution->list[i].val()-i;
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
      Gecode::OpenIntVarList list;

      SelectorChoiceSpace(Gecode::IntValBranch vals)
        : list(*this,Gecode::IntSet({0,2,5}),3) {
        using namespace Gecode;
        list.materialize(*this,3);
        branch(*this,list,
               tiebreak(INT_VAR_SIZE_MIN(),
                        INT_VAR_MERIT_MAX([](const Space&, IntVar, int i) {
                          return i;
                        })),vals,
               [](const Space&, IntVar, int i) { return i != 1; },
               [](const Space&, const Brancher&, unsigned int, IntVar,
                  int i, const int& value, std::ostream& out) {
                 out << i << ":" << value;
               });
      }

      SelectorChoiceSpace(SelectorChoiceSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new SelectorChoiceSpace(*this);
      }
    };

    class SelectorChoice : public ::Test::Base {
    public:
      SelectorChoice(void) : ::Test::Base("Int::OpenList::SelectorChoice") {}

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
            valid &= !clone->failed() && !clone->list[0].assigned() &&
              !clone->list[1].assigned();
            if (multi || (a == 0))
              valid &= clone->list[2].assigned() &&
                (clone->list[2].val() == expected[a]);
            else
              valid &= !clone->list[2].in(5);
            delete clone;
          }
          SelectorChoiceSpace* excluded =
            static_cast<SelectorChoiceSpace*>(source.clone());
          NGL* literal = excluded->ngl(*restored,0);
          valid &= (literal != nullptr) && (literal->status(*excluded) == NGL::NONE);
          if (literal)
            valid &= (literal->prune(*excluded) != ES_FAILED) &&
              !excluded->list[2].in(5);
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
      Gecode::OpenBoolVarList list;

      BoolSpace(bool length_first)
        : list(*this,3) {
        using namespace Gecode;
        DFA accepts_one(0,{{0,0,0},{0,1,1},{1,0,1},{1,1,1}},{1});
        extensional(*this,list,accepts_one);
        if (length_first)
          branch(*this,list.length(),INT_VAL_MIN());
        branch(*this,list,
               BOOL_VAR_MERIT_MAX([](const Space&, BoolVar, int i) { return i; }),
               BOOL_VAL_MAX());
      }

      BoolSpace(BoolSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new BoolSpace(*this);
      }
    };

    class BoolList : public ::Test::Base {
    public:
      BoolList(void) : ::Test::Base("Int::OpenList::Bool",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (bool length_first : {false,true})
          for (int recompute=0; recompute<2; recompute++) {
            BoolSpace root(length_first);
            Search::Options options;
            options.threads = 1;
            options.c_d = recompute ? 1000 : 1;
            options.a_d = options.c_d;
            DFS<BoolSpace> engine(&root,options);
            int solutions = 0;
            while (BoolSpace* solution = engine.next()) {
              bool valid = solution->list.length().assigned() &&
                (solution->list.length().val() == solution->list.prefix_size());
              int ones = 0;
              for (int i=0; i<solution->list.prefix_size(); i++) {
                valid &= solution->list[i].assigned();
                ones += solution->list[i].val();
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
      Gecode::OpenVarList<Gecode::SetVar> sets;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
      Gecode::OpenVarList<Gecode::FloatVar> floats;
#endif

      FactorySpace(void) {
        using namespace Gecode;
#ifdef GECODE_HAS_SET_VARS
        sets = OpenVarList<SetVar>(*this,
          [](Space& home, int i) {
            return SetVar(home,IntSet(i,i),IntSet(i,i));
          },3);
        sets.materialize(*this,1);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        floats = OpenVarList<FloatVar>(*this,
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
      Factory(void) : ::Test::Base("Int::OpenList::Factory",TestTags::check()) {}

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
        valid &= (source.sets.prefix_size() == 1) &&
          (clone->sets.prefix_size() == 3) &&
          (clone->sets.length().val() == 3) &&
          clone->sets[2].assigned() && (clone->sets[2].glbMin() == 2);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        valid &= (source.floats.prefix_size() == 1) &&
          (clone->floats.prefix_size() == 3) &&
          (clone->floats.length().val() == 3) &&
          clone->floats[2].assigned() && (clone->floats[2].min() == 2);
#endif
        delete clone;
        return valid;
      }
    };

    class FixedStatistics : public ::Test::Base {
    public:
      FixedStatistics(void) : ::Test::Base("Int::OpenList::FixedStatistics") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int chb=0; chb<2; chb++) {
          SelectorSpace open(INT_VAL_MIN());
          bool rejected = false;
          try {
            branch(open,open.list,
                   chb ? INT_VAR_CHB_MIN() : INT_VAR_ACTION_MIN(1.0),
                   INT_VAL_MIN());
          } catch (const Gecode::Int::UnknownBranching&) {
            rejected = true;
          }
          if (!rejected)
            return false;
          SelectorChoiceSpace fixed(INT_VAL_MIN());
          branch(fixed,fixed.list,
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
    BoolList bool_list;

    class TransitionSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      TransitionSpace(void)
        : list(*this,Gecode::IntSet(0,4),
            [](Gecode::Space& home, Gecode::OpenIntVarList x, int i) {
              using namespace Gecode;
              if (i == 0) {
                rel(home,x[i],IRT_EQ,0);
              } else {
                linear(home,IntArgs({-1,1}),
                       IntVarArgs({x[i-1],x[i]}),IRT_EQ,1);
              }
            },4) {
        Gecode::branch(*this,list);
      }

      TransitionSpace(TransitionSpace& s)
        : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new TransitionSpace(*this);
      }
    };

    class Transition : public ::Test::Base {
    public:
      Transition(void)
        : ::Test::Base("Int::OpenList::Transition") {}

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
            solution->list.length().assigned() &&
            (solution->list.length().val() ==
             solution->list.prefix_size());
          for (int i=0; i<solution->list.prefix_size(); i++)
            valid &= solution->list[i].assigned() &&
              (solution->list[i].val() == i);
          delete solution;
          if (!valid)
            return false;
          solutions++;
        }
        return solutions == 5;
      }
    };


    /// Test the relations on a closed open list with the integer harness
    class Constraint : public Test {
    public:
      enum Kind { DISTINCT, REL, SLIDING, SLIDING_SUM, MIN_MAX, PRECEDE };
    protected:
      Kind kind;
    public:
      Constraint(const std::string& name, Kind k, int size)
        : Test("OpenList::"+name,size,0,2), kind(k) {}

      virtual bool
      solution(const Assignment& x) const {
        switch (kind) {
        case DISTINCT:
          for (int i=0; i<x.size(); i++)
            for (int j=0; j<i; j++)
              if (x[i] == x[j])
                return false;
          return true;
        case REL:
          for (int i=1; i<x.size(); i++)
            if (x[i-1] >= x[i])
              return false;
          return true;
        case SLIDING:
          for (int i=1; i<x.size(); i++)
            if ((x[i-1] == 1) + (x[i] == 1) != 1)
              return false;
          return true;
        case SLIDING_SUM:
          for (int i=1; i<x.size(); i++)
            if (x[i-1] + x[i] != 2)
              return false;
          return true;
        case MIN_MAX: {
          int lower = x[0];
          int upper = x[0];
          for (int i=1; i<x.size(); i++) {
            lower = std::min(lower,x[i]);
            upper = std::max(upper,x[i]);
          }
          return (lower == 0) && (upper == 2);
        }
        case PRECEDE: {
          bool seen = false;
          for (int i=0; i<x.size(); i++) {
            if ((x[i] == 1) && !seen)
              return false;
            seen |= x[i] == 0;
          }
          return true;
        }
        }
        GECODE_NEVER;
        return false;
      }

      virtual void
      post(Gecode::Space& home, Gecode::IntVarArray& x) {
        using namespace Gecode;
        OpenIntVarList list(home,x.size());
        switch (kind) {
        case DISTINCT:
          distinct(home,list);
          break;
        case REL:
          rel(home,list,IRT_LE);
          break;
        case SLIDING:
          Gecode::sequence(home,list,IntSet(1,1),2,1,1);
          break;
        case SLIDING_SUM:
          slidingsum(home,list,2,2,2);
          break;
        case MIN_MAX: {
          IntVar minimum(home,0,0);
          IntVar maximum(home,2,2);
          min(home,list,minimum);
          max(home,list,maximum);
          break;
        }
        case PRECEDE:
          precede(home,list,0,1);
          break;
        }
        for (int i=0; i<x.size(); i++)
          list.append(home,x[i]);
        list.close(home);
      }
    };

    /// An empty list has no minimum
    class EmptyMinimum : public Test {
    public:
      EmptyMinimum(void) : Test("OpenList::EmptyMinimum",1,0,1) {}
      virtual bool solution(const Assignment&) const { return false; }
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        Gecode::OpenIntVarList list(home,0);
        Gecode::min(home,list,x[0]);
        list.close(home);
      }
    };

    class IncrementalConstraints : public ::Test::Base {
    public:
      IncrementalConstraints(void)
        : ::Test::Base("Int::OpenList::IncrementalConstraints",
                       TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        class IncrementalSpace : public Space {
        public:
          OpenIntVarList list;
          IntVar minimum;
          IntVar maximum;

          IncrementalSpace(void)
            : list(*this,IntSet(0,2),2),
              minimum(*this,0,2), maximum(*this,0,2) {
            distinct(*this,list);
            rel(*this,list,IRT_LE);
            Gecode::sequence(*this,list,IntSet(2,2),2,1,1);
            slidingsum(*this,list,2,2,2);
            min(*this,list,minimum);
            max(*this,list,maximum);
            precede(*this,list,0,2);
          }

          IncrementalSpace(IncrementalSpace& s)
            : Space(s) {
            list.update(*this,s.list);
            minimum.update(*this,s.minimum);
            maximum.update(*this,s.maximum);
          }

          virtual Space*
          copy(void) {
            return new IncrementalSpace(*this);
          }
        } source;

        source.list.materialize(source,1);
        rel(source,source.list[0],IRT_EQ,0);
        if ((source.status() == SS_FAILED) ||
            !source.list[0].assigned())
          return false;

        IncrementalSpace* clone =
          static_cast<IncrementalSpace*>(source.clone());
        clone->list.materialize(*clone,2);
        clone->list.close(*clone);
        const bool valid =
          (clone->status() != SS_FAILED) &&
          clone->list[1].assigned() &&
          (clone->list[1].val() == 2) &&
          clone->minimum.assigned() && (clone->minimum.val() == 0) &&
          clone->maximum.assigned() && (clone->maximum.val() == 2);
        delete clone;
        return valid && (source.list.prefix_size() == 1);
      }
    };

    class CallbackSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      CallbackSpace(Gecode::OpenIntVarList::Domain domain,
                    Gecode::OpenIntVarList::Transition transition)
        : list(*this,domain,transition,8) {}

      CallbackSpace(CallbackSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new CallbackSpace(*this);
      }
    };

    class GroupSpace : public Gecode::Space {
    public:
      enum Kind { REL, SUM, PRECEDE };
      Gecode::OpenIntVarList list;
      Gecode::PropagatorGroup group;

      GroupSpace(Kind kind, bool close)
        : list(*this,Gecode::IntSet(0,2),3) {
        using namespace Gecode;
        switch (kind) {
        case REL:
          rel((*this)(group),list,IRT_NQ);
          break;
        case SUM:
          slidingsum((*this)(group),list,2,2,2);
          break;
        case PRECEDE:
          precede((*this)(group),list,0,1);
          break;
        }
        list.materialize(*this,2);
        if (close)
          list.close(*this);
      }

      GroupSpace(GroupSpace& s) : Gecode::Space(s), group(s.group) {
        list.update(*this,s.list);
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
        : ::Test::Base("Int::OpenList::Group::"+name+
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
          rel(source,source.list[0],IRT_EQ,
              kind == GroupSpace::PRECEDE ? 2 : 0);
          rel(source,source.list[1],IRT_EQ,
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
        : ::Test::Base("Int::OpenList::Dispose",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        std::shared_ptr<int> domain(new int(0));
        std::shared_ptr<int> transition(new int(0));
        CallbackSpace* source = new CallbackSpace(
          [domain](int) { return IntSet(0,1); },
          [transition](Space&, OpenIntVarList, int) {});
        source->list.materialize(*source,2);
        DFA dfa(0,{{0,0,0},{0,1,0}},{0});
        extensional(*source,source->list,dfa);
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
        : ::Test::Base("Int::OpenList::CallbackLength",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int assign=0; assign<2; assign++)
          for (int close=0; close<2; close++) {
            CallbackSpace source([](int) { return IntSet(0,1); },
              [assign](Space& home, OpenIntVarList list, int i) {
                if (i == 0)
                  rel(home,list.length(),assign ? IRT_EQ : IRT_GQ,3);
              });
            rel(source,source.list.length(),IRT_GQ,1);
            if (close)
              source.list.close(source);
            if ((source.status() == SS_FAILED) ||
                (source.list.prefix_size() != 3))
              return false;
          }
        return true;
      }
    };

    class FailedGet : public ::Test::Base {
    public:
      FailedGet(void)
        : ::Test::Base("Int::OpenList::FailedGet") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(0,1); },
          [](Space& home, OpenIntVarList, int) { home.fail(); });
        IntVar x = source.list.get(source,4);
        return source.failed() && (x.varimp() == nullptr);
      }
    };

    class ValueCommit : public ::Test::Base {
    public:
      ValueCommit(void)
        : ::Test::Base("Int::OpenList::ValueCommit") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(0,1); },
          [](Space& home, OpenIntVarList, int i) {
            if (i == 1)
              home.fail();
          });
        source.list.materialize(source,1);
        branch(source,source.list);
        if (source.status() != SS_BRANCH)
          return false;
        const Choice* choice = source.choice();
        bool valid = true;
        for (unsigned int alternative=0; alternative<2; alternative++) {
          CallbackSpace* clone = static_cast<CallbackSpace*>(source.clone());
          rel(*clone,clone->list.length(),IRT_GQ,2);
          clone->commit(*choice,alternative);
          valid &= !clone->failed() && (clone->list.prefix_size() == 1) &&
            clone->list[0].assigned() &&
            (clone->list[0].val() == static_cast<int>(alternative));
          valid &= clone->status() == SS_FAILED;
          delete clone;
        }
        delete choice;
        return valid;
      }
    };

    /// Replay value choices after propagation has extended the original list
    class ValueReplay : public ::Test::Base {
    public:
      ValueReplay(void)
        : ::Test::Base("Int::OpenList::ValueReplay",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(0,1); },
          [](Space& home, OpenIntVarList list, int i) {
            if (i == 0) {
              BoolVar zero(home,0,1);
              rel(home,list[0],IRT_EQ,0,Reify(zero));
              rel(home,list.length(),IRT_GQ,2,Reify(zero,RM_IMP));
            }
          });
        source.list.materialize(source,1);
        branch(source,source.list);
        if (source.status() != SS_BRANCH)
          return false;
        std::unique_ptr<const Choice> first(source.choice());
        std::unique_ptr<CallbackSpace> replay
          (static_cast<CallbackSpace*>(source.clone()));
        source.commit(*first,0);
        if ((source.status() != SS_BRANCH) ||
            (source.list.prefix_size() != 2))
          return false;
        std::unique_ptr<const Choice> second(source.choice());
        // Recomputation commits the path without propagating between choices.
        replay->commit(*first,0);
        replay->commit(*second,0);
        return (replay->status() != SS_FAILED) &&
          (replay->list.prefix_size() == 2) &&
          replay->list[0].assigned() && (replay->list[0].val() == 0) &&
          replay->list[1].assigned() && (replay->list[1].val() == 0);
      }
    };

    class ImpossibleWindow : public ::Test::Base {
    public:
      ImpossibleWindow(void)
        : ::Test::Base("Int::OpenList::ImpossibleWindow") {}

      virtual bool
      run(void) {
        using namespace Gecode;
        for (int sum=0; sum<2; sum++)
          for (int size=0; size<3; size++) {
            CallbackSpace source([](int) { return IntSet(0,1); },
              [](Space&, OpenIntVarList, int) {});
            if (sum)
              slidingsum(source,source.list,2,2,1);
            else
              Gecode::sequence(source,source.list,IntSet(0,1),2,3,3);
            rel(source,source.list.length(),IRT_EQ,size);
            const bool failed = source.status() == SS_FAILED;
            if (failed != (size == 2))
              return false;
            if (!failed && (source.list.prefix_size() != size))
              return false;
          }
        return true;
      }
    };

    ValueCommit value_commit;
    ValueReplay value_replay;
    ImpossibleWindow impossible_window;

    Dispose disposal;
    CallbackLength callback_length;
    FailedGet failed_get;

    Branch branch;
    BranchOrder branch_order;
    Transition transition;
    Constraint distinct("Distinct",Constraint::DISTINCT,3);
    Constraint relation("Rel",Constraint::REL,3);
    Constraint sliding("Sliding",Constraint::SLIDING,4);
    Constraint sliding_sum("SlidingSum",Constraint::SLIDING_SUM,4);
    Constraint min_max("MinMax",Constraint::MIN_MAX,3);
    Constraint precedence("Precede",Constraint::PRECEDE,3);
    EmptyMinimum empty_minimum;
    IncrementalConstraints incremental_constraints;

  }

}}

// STATISTICS: test-int
