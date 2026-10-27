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
#include "examples/common-pattern.hh"

#include <gecode/int/growable-int-set.hh>
#include <gecode/search.hh>

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

#include <memory>
#include <set>

namespace Test { namespace Int {

  namespace OpenList {

    /// Space used by open list materialization tests
    class OpenListSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      OpenListSpace(void)
        : list(*this,
            [](int i) {
              return Gecode::IntSet(i,i);
            },6) {}

      OpenListSpace(OpenListSpace& s)
        : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new OpenListSpace(*this);
      }
    };

    /// Test automatic and explicit list materialization
    class OpenListMaterialize : public ::Test::Base {
    public:
      OpenListMaterialize(void)
        : ::Test::Base("Int::OpenList::Materialize",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        OpenListSpace source;
        rel(source,source.list.length(),IRT_GQ,3);
        if ((source.status() == SS_FAILED) ||
            (source.list.known_length() != 3))
          return false;
        for (int i=0; i<3; i++)
          if (!source.list[i].assigned() ||
              (source.list[i].val() != i))
            return false;

        OpenListSpace* clone =
          static_cast<OpenListSpace*>(source.clone());
        rel(*clone,clone->list.length(),IRT_GQ,5);
        const bool clone_ok =
          (clone->status() != SS_FAILED) &&
          (clone->list.known_length() == 5) &&
          clone->list[4].assigned() &&
          (clone->list[4].val() == 4);
        delete clone;
        if (!clone_ok || (source.list.known_length() != 3))
          return false;

        OpenListSpace direct;
        IntVar x = direct.list.get(direct,3);
        return (direct.list.known_length() == 4) &&
               (direct.list.length().min() == 4) &&
               x.assigned() && (x.val() == 3) &&
               (direct.status() != SS_FAILED);
      }
    };

    OpenListMaterialize open_list_materialize;

    class BranchOrderSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;

      enum class Order { INCREMENTAL, LENGTH_MIN, LENGTH_MAX };

      BranchOrderSpace(Order order)
        : list(*this,Gecode::IntSet(0,1),3) {
        using namespace Gecode;
        DFA accepts_one(0,{
            {0,0,0}, {0,1,1},
            {1,0,1}, {1,1,1}
          },{1});
        extensional(*this,list,accepts_one);
        if (order == Order::LENGTH_MIN)
          branch(*this,list.length(),INT_VAL_MIN());
        else if (order == Order::LENGTH_MAX)
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
            first = solution->list.known_length();
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
        return check(BranchOrderSpace::Order::INCREMENTAL,3) &&
          check(BranchOrderSpace::Order::LENGTH_MIN,1) &&
          check(BranchOrderSpace::Order::LENGTH_MAX,3);
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
                (solution->list.length().val() == solution->list.known_length());
              for (int i=0; i<solution->list.known_length(); i++)
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
          for (int i=0; i<solution->list.known_length(); i++)
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
        // Extracted nogoods must allow every unvisited word.
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
                (solution->list.length().val() == solution->list.known_length());
              int ones = 0;
              for (int i=0; i<solution->list.known_length(); i++) {
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

    template<class Var>
    class AliasSpace : public Gecode::Space {
    public:
      Gecode::OpenVarList<Var> list;

      AliasSpace(void)
        : list(*this,[](Gecode::Space& home, int) {
            return Var(home,0,1);
          },16) {}

      AliasSpace(AliasSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
      }

      virtual Gecode::Space*
      copy(void) {
        return new AliasSpace(*this);
      }

      bool
      rejects(int i) {
        const int n = list.known_length();
        try {
          list.append(*this,list[i]);
        } catch (const Gecode::Int::ArgumentSame&) {
          return list.known_length() == n;
        }
        return false;
      }
    };

    class Aliases : public ::Test::Base {
    protected:
      template<class Var>
      static bool
      check(void) {
        using namespace Gecode;
        AliasSpace<Var> source;
        Var assigned(source,0,0);
        source.list.append(source,assigned);
        source.list.append(source,assigned);
        Var first(source,0,1);
        source.list.append(source,first);
        source.list.append(source,Var(source,0,1));
        if (!source.rejects(2))
          return false;
        rel(source,first,IRT_EQ,0);
        source.list.append(source,first);
        if ((source.list.known_length() != 5) ||
            (source.status() == SS_FAILED))
          return false;
        std::unique_ptr<AliasSpace<Var>> clone
          (static_cast<AliasSpace<Var>*>(source.clone()));
        if (!clone->rejects(3))
          return false;
        clone->list.append(*clone,Var(*clone,0,1));
        rel(*clone,clone->list[3],IRT_EQ,0);
        clone->list.append(*clone,clone->list[3]);
        return (clone->list.known_length() == 7) &&
          (clone->status() != SS_FAILED) &&
          (source.list.known_length() == 5) &&
          !source.list[3].assigned() && source.rejects(3);
      }

    public:
      Aliases(void) : ::Test::Base("Int::OpenList::Aliases",TestTags::check()) {}

      virtual bool
      run(void) {
        return check<Gecode::IntVar>() && check<Gecode::BoolVar>();
      }
    };

    Aliases aliases;

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
        valid &= (source.sets.known_length() == 1) &&
          (clone->sets.known_length() == 3) &&
          (clone->sets.length().val() == 3) &&
          clone->sets[2].assigned() && (clone->sets[2].glbMin() == 2);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        valid &= (source.floats.known_length() == 1) &&
          (clone->floats.known_length() == 3) &&
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
             solution->list.known_length());
          for (int i=0; i<solution->list.known_length(); i++)
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


    /// View handles share growth while cloned spaces keep independent domains
    class Views : public ::Test::Base {
      class ViewSpace : public Gecode::Space {
      public:
        Gecode::OpenIntVarList list;
        Gecode::OpenViewList<Gecode::Int::IntView> direct;
        Gecode::OpenViewList<Gecode::Int::MinusView> negative;

        ViewSpace(void)
          : list(*this,Gecode::IntSet(0,10),5),
            direct(list), negative(list) {}

        ViewSpace(ViewSpace& s) : Gecode::Space(s) {
          list.update(*this,s.list);
          direct.update(*this,s.direct);
          negative.update(*this,s.negative);
        }

        Gecode::Space* copy(void) override {
          return new ViewSpace(*this);
        }
      };
    public:
      Views(void)
        : ::Test::Base("Int::OpenList::Views",TestTags::check()) {}

      bool run(void) override {
        using namespace Gecode;
        ViewSpace source;
        Gecode::Int::IntView length = source.negative.length();
        if (me_failed(length.gq(source,2)) ||
            (source.status() == SS_FAILED) ||
            (source.direct.known_length() != 2) ||
            (source.negative.known_length() != 2))
          return false;
        if (me_failed(source.negative[0].gq(source,-7)) ||
            me_failed(source.direct[0].gq(source,2)) ||
            (source.status() == SS_FAILED) ||
            (source.list[0].min() != 2) || (source.list[0].max() != 7) ||
            (source.negative[0].min() != -7) ||
            (source.negative[0].max() != -2))
          return false;

        std::unique_ptr<ViewSpace> clone
          (static_cast<ViewSpace*>(source.clone()));
        clone->negative.materialize(*clone,3);
        if (me_failed(clone->negative[0].eq(*clone,-5)) ||
            me_failed(clone->direct.length().eq(*clone,3)) ||
            (clone->status() == SS_FAILED) ||
            (clone->list.known_length() != 3) ||
            (clone->direct.known_length() != 3) ||
            (clone->direct[0].val() != 5) ||
            (clone->negative[2].min() != -10))
          return false;
        source.direct.materialize(source,4);
        return (source.status() != SS_FAILED) &&
          (source.list.known_length() == 4) &&
          (source.negative.known_length() == 4) &&
          (source.negative[3].max() == 0) &&
          (source.list[0].min() == 2) && (source.list[0].max() == 7) &&
          (clone->negative.known_length() == 3) &&
          (clone->list.length().val() == 3);
      }
    };


    /// Test constraint semantics both while growing and after closure
    class Constraint : public OpenTest {
    public:
      enum class Kind {
        DISTINCT, REL, SLIDING, SLIDING_SUM, SLIDING_RANGE, MIN_MAX,
        PRECEDE, PRECEDE_SAME
      };
    protected:
      Kind kind;
      int width;
      Gecode::IntRelType relation;
    public:
      Constraint(const std::string& name, Kind k, int size, int width0=2,
                 Gecode::IntPropLevel level=Gecode::IPL_DEF)
        : OpenTest("OpenList::"+name,size,0,Gecode::IntSet(0,2),level),
          kind(k), width(width0), relation(Gecode::IRT_LE) {}

      Constraint(Gecode::IntRelType relation0, Gecode::IntPropLevel level)
        : OpenTest("OpenList::Rel::"+str(relation0)+"::"+str(level),
                   3,0,Gecode::IntSet({0,2,5}),level),
          kind(Kind::REL), width(0), relation(relation0) {}

      virtual bool
      solution(const Assignment& x) const {
        switch (kind) {
        case Kind::DISTINCT:
          for (int i=0; i<x.size(); i++)
            for (int j=0; j<i; j++)
              if (x[i] == x[j])
                return false;
          return true;
        case Kind::REL:
          for (int i=1; i<x.size(); i++)
            if (!cmp(x[i-1],relation,x[i]))
              return false;
          return true;
        case Kind::SLIDING:
          for (int i=0; i+width<=x.size(); i++) {
            int count = 0;
            for (int j=i; j<i+width; j++)
              count += x[j] == 1;
            if (count != 1)
              return false;
          }
          return true;
        case Kind::SLIDING_SUM:
        case Kind::SLIDING_RANGE:
          for (int i=0; i+width<=x.size(); i++) {
            int sum = 0;
            for (int j=i; j<i+width; j++)
              sum += x[j];
            if ((kind == Kind::SLIDING_SUM) ? sum != 2 :
                (sum < 1) || (sum > 3))
              return false;
          }
          return true;
        case Kind::MIN_MAX: {
          int lower = x[0];
          int upper = x[0];
          for (int i=1; i<x.size(); i++) {
            lower = std::min(lower,x[i]);
            upper = std::max(upper,x[i]);
          }
          return (lower == 0) && (upper == 2);
        }
        case Kind::PRECEDE: {
          bool seen = false;
          for (int i=0; i<x.size(); i++) {
            if ((x[i] == 1) && !seen)
              return false;
            seen |= x[i] == 0;
          }
          return true;
        }
        case Kind::PRECEDE_SAME:
          for (int i=0; i<x.size(); i++)
            if (x[i] == 0)
              return false;
          return true;
        }
        GECODE_NEVER;
        return false;
      }

      virtual void
      post(Gecode::Space& home, Gecode::OpenIntVarList list,
           Gecode::IntVarArray&) {
        using namespace Gecode;
        switch (kind) {
        case Kind::DISTINCT:
          distinct(home,list);
          break;
        case Kind::REL:
          rel(home,list,relation,ipl);
          break;
        case Kind::SLIDING:
          Gecode::sequence(home,list,IntSet(1,1),width,1,1);
          break;
        case Kind::SLIDING_SUM:
          slidingsum(home,list,width,2,2,ipl);
          break;
        case Kind::SLIDING_RANGE:
          slidingsum(home,list,width,1,3,ipl);
          break;
        case Kind::MIN_MAX: {
          IntVar minimum(home,0,0);
          IntVar maximum(home,2,2);
          min(home,list,minimum);
          max(home,list,maximum);
          break;
        }
        case Kind::PRECEDE:
          precede(home,list,0,1);
          break;
        case Kind::PRECEDE_SAME:
          precede(home,list,0,0);
          break;
        }
      }
    };

    /// Extremum bounds must follow old items as well as newly appended items
    class Extremum : public OpenTest {
      bool minimum;
      bool alias;
    public:
      Extremum(bool minimum0, bool alias0, Gecode::IntPropLevel level)
        : OpenTest(std::string("OpenList::")+(minimum0 ? "Min" : "Max")+
                   (alias0 ? "Alias::" : "Result::")+str(level),
                   3,alias0 ? 0 : 1,Gecode::IntSet(0,2),level),
          minimum(minimum0), alias(alias0) {}

      bool solution(const Assignment& x) const override {
        int value = x[0];
        for (int i=1; i<3; i++)
          value = minimum ? std::min(value,x[i]) : std::max(value,x[i]);
        return value == x[alias ? 0 : 3];
      }

      void post(Gecode::Space& home, Gecode::OpenIntVarList list,
                Gecode::IntVarArray& x) override {
        if (minimum)
          Gecode::min(home,list,x[alias ? 0 : 3],ipl);
        else
          Gecode::max(home,list,x[alias ? 0 : 3],ipl);
      }
    };

    /// Bounds propagate through existing items before the list is closed
    class ExtremumBounds : public ::Test::Base {
      class BoundsSpace : public Gecode::Space {
      public:
        Gecode::OpenIntVarList list;
        Gecode::IntVar minimum;
        Gecode::IntVar maximum;

        BoundsSpace(void)
          : list(*this,Gecode::IntSet(0,10),5),
            minimum(*this,0,10), maximum(*this,0,10) {
          Gecode::min(*this,list,minimum);
          Gecode::max(*this,list,maximum);
        }
        BoundsSpace(BoundsSpace& s) : Gecode::Space(s) {
          list.update(*this,s.list);
          minimum.update(*this,s.minimum);
          maximum.update(*this,s.maximum);
        }
        Gecode::Space* copy(void) override { return new BoundsSpace(*this); }
      };
    public:
      ExtremumBounds(void)
        : ::Test::Base("Int::OpenList::ExtremumBounds",TestTags::check()) {}

      bool run(void) override {
        using namespace Gecode;
        BoundsSpace source;
        source.list.materialize(source,2);
        if (source.status() == SS_FAILED)
          return false;
        rel(source,source.list[0],IRT_GQ,4);
        rel(source,source.list[1],IRT_GQ,6);
        rel(source,source.list[0],IRT_LQ,7);
        rel(source,source.list[1],IRT_LQ,8);
        if ((source.status() == SS_FAILED) ||
            (source.maximum.min() != 6) || (source.minimum.max() != 7) ||
            (source.minimum.min() != 0) || (source.maximum.max() != 10))
          return false;
        rel(source,source.minimum,IRT_GQ,5);
        rel(source,source.maximum,IRT_LQ,7);
        if ((source.status() == SS_FAILED) ||
            (source.list[0].min() != 5) || (source.list[1].min() != 6) ||
            (source.list[0].max() != 7) || (source.list[1].max() != 7))
          return false;
        std::unique_ptr<BoundsSpace> clone
          (static_cast<BoundsSpace*>(source.clone()));
        clone->list.materialize(*clone,3);
        if ((clone->status() == SS_FAILED) ||
            (clone->list[2].min() != 5) || (clone->list[2].max() != 7) ||
            (source.list.known_length() != 2))
          return false;
        rel(*clone,clone->list[2],IRT_EQ,5);
        if ((clone->status() == SS_FAILED) || (clone->minimum.max() != 5) ||
            (source.minimum.max() != 7))
          return false;
        clone->list.close(*clone);
        rel(*clone,clone->list[0],IRT_EQ,7);
        if ((clone->status() == SS_FAILED) ||
            !clone->minimum.assigned() || (clone->minimum.val() != 5) ||
            !clone->maximum.assigned() || (clone->maximum.val() != 7))
          return false;
        rel(source,source.maximum,IRT_LQ,5);
        return source.status() == SS_FAILED;
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
        return valid && (source.list.known_length() == 1);
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

    /// Range unions and later clone growth preserve canonical sets
    class GrowableIntSet : public ::Test::Base {
      class SetSpace : public Gecode::Space {
      public:
        Gecode::Int::GrowableIntSet values;

        SetSpace(void) {}

        SetSpace(SetSpace& s) : Gecode::Space(s) {
          values.update(*this,s.values);
        }

        ~SetSpace(void) {
          values.dispose(*this);
        }

        Gecode::Space* copy(void) override {
          return new SetSpace(*this);
        }
      };

      static bool
      check(const SetSpace& home, const Gecode::IntSet& expected) {
        Gecode::Int::GrowableIntSetRanges actual(home.values);
        Gecode::IntSetRanges ranges(expected);
        for (; actual() && ranges(); ++actual, ++ranges) {
          if ((actual.min() != ranges.min()) ||
              (actual.max() != ranges.max()) ||
              (actual.width() != ranges.width()))
            return false;
          for (int value : {ranges.min()-1,ranges.min(),
                            ranges.max(),ranges.max()+1})
            if (home.values.in(value) != expected.in(value))
              return false;
        }
        if (actual() || ranges())
          return false;
        for (int value=-40; value<=40; value++)
          if (home.values.in(value) != expected.in(value))
            return false;
        return true;
      }

      static bool
      add(SetSpace& home, std::set<int>& expected,
          const Gecode::IntSet& batch) {
        Gecode::IntSetRanges ranges(batch);
        home.values.add_r(home,ranges);
        for (Gecode::IntSetValues values(batch); values(); ++values)
          expected.insert(values.val());
        const std::vector<int> values(expected.begin(),expected.end());
        return check(home,Gecode::IntSet(values.data(),
                                       static_cast<int>(values.size())));
      }

    public:
      GrowableIntSet(void)
        : ::Test::Base("Int::OpenList::GrowableIntSet",TestTags::check()) {}

      bool run(void) override {
        using namespace Gecode;
        SetSpace source;
        std::set<int> expected;
        const IntSet batches[] = {
          IntSet::empty, IntSet({10,12}), IntSet(10,12),
          IntSet(-10,-8), IntSet(20,22), IntSet(0,2), IntSet(3,9),
          IntSet(-9,-7), IntSet(18,20), IntSet(11,11),
          IntSet({{-12,-9},{-6,1},{15,16},{19,24}}),
          IntSet({Gecode::Int::Limits::min,Gecode::Int::Limits::max})
        };
        for (const IntSet& batch : batches)
          if (!add(source,expected,batch))
            return false;
        if (source.status() == SS_FAILED)
          return false;
        std::unique_ptr<SetSpace> clone
          (static_cast<SetSpace*>(source.clone()));
        std::unique_ptr<SetSpace> sibling
          (static_cast<SetSpace*>(source.clone()));
        std::set<int> clone_expected(expected);
        const IntSet growth({{-30,-29},{-20,-19},{14,14},{18,18},
                             {26,26},{28,28},{30,30}});
        if (!add(*clone,clone_expected,IntSet(26,26)) ||
            !add(*clone,clone_expected,growth) ||
            !add(source,expected,IntSet::empty) ||
            !add(*sibling,expected,IntSet::empty))
          return false;
        // A single wide range exercises unsigned width and removes all holes.
        const IntSet full(Gecode::Int::Limits::min,Gecode::Int::Limits::max);
        IntSetRanges ranges(full);
        source.values.add_r(source,ranges);
        return check(source,full) &&
          add(*clone,clone_expected,IntSet::empty) &&
          add(*sibling,expected,IntSet::empty);
      }
    };

    /// Values assigned before list growth can trigger several pruning waves
    class DistinctWaves : public ::Test::Base {
    public:
      DistinctWaves(void)
        : ::Test::Base("Int::OpenList::DistinctWaves",TestTags::check()) {}

      bool run(void) override {
        using namespace Gecode;
        CallbackSpace source([](int) { return IntSet(1,4); },
                             [](Space&, OpenIntVarList, int) {});
        distinct(source,source.list);
        source.list.append(source,IntVar(source,1,1));
        if (source.status() == SS_FAILED)
          return false;
        std::unique_ptr<CallbackSpace> duplicate
          (static_cast<CallbackSpace*>(source.clone()));
        // Reverse the chain so each assignment needs another pruning wave.
        for (int lower=3; lower>=1; lower--)
          source.list.append(source,IntVar(source,lower,lower+1));
        if (source.status() == SS_FAILED)
          return false;
        for (int i=1; i<4; i++)
          if (!source.list[i].assigned() || (source.list[i].val() != 5-i))
            return false;
        source.list.close(source);
        if ((source.status() == SS_FAILED) ||
            !source.list.length().assigned() ||
            (source.list.length().val() != 4))
          return false;
        duplicate->list.append(*duplicate,IntVar(*duplicate,1,2));
        duplicate->list.append(*duplicate,IntVar(*duplicate,1,2));
        return duplicate->status() == SS_FAILED;
      }
    };

    GrowableIntSet growable_int_set;
    DistinctWaves distinct_waves;

    class GroupSpace : public Gecode::Space {
    public:
      enum class Kind { REL, SUM, PRECEDE };
      Gecode::OpenIntVarList list;
      Gecode::PropagatorGroup group;

      GroupSpace(Kind kind, bool close)
        : list(*this,Gecode::IntSet(0,2),3) {
        using namespace Gecode;
        switch (kind) {
        case Kind::REL:
          rel((*this)(group),list,IRT_NQ);
          break;
        case Kind::SUM:
          slidingsum((*this)(group),list,2,2,2);
          break;
        case Kind::PRECEDE:
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
          // These values are in the domains but violate each constraint.
          rel(source,source.list[0],IRT_EQ,
              kind == GroupSpace::Kind::PRECEDE ? 2 : 0);
          rel(source,source.list[1],IRT_EQ,
              kind == GroupSpace::Kind::PRECEDE ? 1 : 0);
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

    Group group_rel_kill("Rel",GroupSpace::Kind::REL,true);
    Group group_rel_disable("Rel",GroupSpace::Kind::REL,false);
    Group group_sum_kill("Sum",GroupSpace::Kind::SUM,true);
    Group group_sum_disable("Sum",GroupSpace::Kind::SUM,false);
    Group group_precede_kill("Precede",GroupSpace::Kind::PRECEDE,true);
    Group group_precede_disable("Precede",GroupSpace::Kind::PRECEDE,false);

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
                (source.list.known_length() != 3))
              return false;
          }
        return true;
      }
    };

    /// Callbacks suppress AFC unless opted in; nested growth restores each scope
    class NoGPI : public ::Test::Base {
    protected:
      static void
      post(Gecode::Space& home, Gecode::PropagatorGroup group) {
        Gecode::IntVar x(home,0,10);
        Gecode::IntVar y(home,0,10);
        Gecode::rel(home(group),x,Gecode::IRT_LE,y);
      }

      static int
      count(const Gecode::Space& home, Gecode::PropagatorGroup group,
            double afc) {
        int n = 0;
        for (Gecode::Propagators ps(home,group); ps(); ++ps) {
          if (ps.propagator().afc() != afc)
            return -1;
          n++;
        }
        return n;
      }

    public:
      NoGPI(void)
        : ::Test::Base("Int::OpenList::NoGPI",TestTags::check()) {}

      virtual bool
      run(void) {
        using namespace Gecode;
        CallbackSpace home([](int) { return IntSet(0,1); },
                           [](Space&, OpenIntVarList, int) {});
        PropagatorGroup factory;
        PropagatorGroup transition;
        PropagatorGroup selected;
        PropagatorGroup ordinary;
        PropagatorGroup suppressed;
        OpenVarList<IntVar> list(home,
          [factory,selected](Space& home, int) {
            post(home,factory);
            {
              Space::UseGPI use_gpi(home);
              post(home,selected);
            }
            return IntVar(home,0,10);
          },
          [transition,selected](Space& home, OpenVarList<IntVar> list, int i) {
            post(home,transition);
            if (i == 3)
              throw 1;
            {
              Space::UseGPI use_gpi(home);
              post(home,selected);
              if (i == 0)
                list.materialize(home,2);
              post(home,selected);
            }
            post(home,transition);
          },4);

        list.materialize(home,1);
        post(home,ordinary);
        if ((list.known_length() != 2) ||
            (count(home,factory,0.0) != 2) ||
            (count(home,transition,0.0) != 4) ||
            (count(home,selected,1.0) != 6))
          return false;
        {
          Space::NoGPI no_gpi(home);
          {
            Space::UseGPI use_gpi(home);
            list.append(home,IntVar(home,0,10));
            post(home,ordinary);
          }
          post(home,suppressed);
        }
        bool threw = false;
        try {
          list.append(home,IntVar(home,0,10));
        } catch (int) {
          threw = true;
        }
        post(home,ordinary);
        return threw && (count(home,factory,0.0) == 2) &&
          (count(home,transition,0.0) == 7) &&
          (count(home,selected,1.0) == 8) &&
          (count(home,ordinary,1.0) == 3) &&
          (count(home,suppressed,0.0) == 1) &&
          (home.status() != SS_FAILED);
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

    /// A value commit must not construct positions required by pending propagation
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
          valid &= !clone->failed() && (clone->list.known_length() == 1) &&
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
            (source.list.known_length() != 2))
          return false;
        std::unique_ptr<const Choice> second(source.choice());
        // Recomputation commits the path without propagating between choices.
        replay->commit(*first,0);
        for (unsigned int a=0; a<second->alternatives(); a++) {
          std::ostringstream printed;
          replay->print(*second,a,printed);
          if (printed.str().empty() || (replay->list.known_length() != 1))
            return false;
        }
        replay->commit(*second,0);
        return (replay->status() != SS_FAILED) &&
          (replay->list.known_length() == 2) &&
          replay->list[0].assigned() && (replay->list[0].val() == 0) &&
          replay->list[1].assigned() && (replay->list[1].val() == 0);
      }
    };

    /// Impossible window bounds still permit lists too short to contain a window
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
            if (!failed && (source.list.known_length() != size))
              return false;
          }
        return true;
      }
    };

    /// Search model for the private common-pattern projection
    class PatternSpace : public Gecode::Space {
    public:
      Gecode::OpenIntVarList list;
      Gecode::IntVar frequency;

      PatternSpace(const PatternMining::Database& data, int support)
        : list(*this,data.domain(),data.max_length()) {
        Gecode::rel(*this,list.length(),Gecode::IRT_GQ,1);
        frequency = PatternMining::frequent(*this,list,data,support);
        Gecode::branch(*this,list);
      }

      PatternSpace(PatternSpace& s) : Gecode::Space(s) {
        list.update(*this,s.list);
        frequency.update(*this,s.frequency);
      }

      virtual Gecode::Space* copy(void) {
        return new PatternSpace(*this);
      }
    };

    /// Compare complete solution sets with independently enumerated subsequences
    class PatternEnumeration : public ::Test::Base {
    public:
      PatternEnumeration(void)
        : ::Test::Base("Int::OpenList::Pattern::Enumeration",
                       TestTags::check()) {}

      virtual bool run(void) {
        using namespace Gecode;
        const PatternMining::Sequences inputs[] = {
          {{17,1000000,17},{17,1000000,17},{1000000,17,42},{17,42}},
          {{1,1},{1,2},{2,1}}
        };
        for (const PatternMining::Sequences& input : inputs) {
          std::set<std::vector<int>> candidates;
          for (const std::vector<int>& row : input)
            for (unsigned int mask=1; mask<(1U<<row.size()); mask++) {
              std::vector<int> pattern;
              for (unsigned int i=0; i<row.size(); i++)
                if (mask & (1U<<i))
                  pattern.push_back(row[i]);
              candidates.insert(pattern);
            }
          const PatternMining::Database data(input);
          const int thresholds[] = {1,2,static_cast<int>(input.size())};
          for (int threshold : thresholds) {
            std::set<std::vector<int>> expected;
            std::set<std::vector<int>> found;
            for (const std::vector<int>& pattern : candidates) {
              int support = 0;
              for (const std::vector<int>& row : input) {
                auto next = row.begin();
                bool matches = true;
                for (int item : pattern) {
                  next = std::find(next,row.end(),item);
                  if (next == row.end()) {
                    matches = false;
                    break;
                  }
                  ++next;
                }
                support += matches;
              }
              if (support >= threshold)
                expected.insert(pattern);
            }
            PatternSpace root(data,threshold);
            Search::Options options;
            options.threads = 1;
            options.c_d = 1000;
            options.a_d = 1000;
            DFS<PatternSpace> search(&root,options);
            while (PatternSpace* solution = search.next()) {
              std::unique_ptr<PatternSpace> owned(solution);
              if (!solution->list.length().assigned() ||
                  (solution->list.length().val() !=
                   solution->list.known_length()))
                return false;
              std::vector<int> pattern;
              for (int i=0; i<solution->list.known_length(); i++) {
                if (!solution->list[i].assigned())
                  return false;
                pattern.push_back(solution->list[i].val());
              }
              if (!solution->frequency.assigned() ||
                  (solution->frequency.val() != data.support(pattern)) ||
                  !found.insert(pattern).second)
                return false;
            }
            if (found != expected)
              return false;
          }
        }
        return true;
      }
    };

    /// Clones keep separate projections; later assignments wait for their prefix
    class PatternGrowth : public ::Test::Base {
    public:
      PatternGrowth(void)
        : ::Test::Base("Int::OpenList::Pattern::Growth",TestTags::check()) {}

      virtual bool run(void) {
        using namespace Gecode;
        const PatternMining::Database data({{1,2,1},{1,2,2},{1,1,2}});
        PatternSpace source(data,2);
        source.list.materialize(source,1);
        rel(source,source.list[0],IRT_EQ,1);
        if (source.status() == SS_FAILED)
          return false;
        std::unique_ptr<PatternSpace> clone
          (static_cast<PatternSpace*>(source.clone()));
        clone->list.materialize(*clone,2);
        rel(*clone,clone->list[1],IRT_EQ,1);
        if ((clone->status() == SS_FAILED) ||
            !clone->list.length().assigned() ||
            (clone->list.length().val() != 2) ||
            (source.list.known_length() != 1) ||
            (source.list.length().min() != 1))
          return false;
        source.list.materialize(source,2);
        rel(source,source.list[1],IRT_EQ,2);
        if ((source.status() == SS_FAILED) ||
            !source.list.length().assigned() ||
            (source.list.length().val() != 2) ||
            (clone->list[1].val() != 1))
          return false;

        // Assigning a later position must not advance the projected prefix.
        PatternSpace later(data,2);
        later.list.materialize(later,2);
        rel(later,later.list[1],IRT_EQ,2);
        if ((later.status() == SS_FAILED) || later.list[0].assigned())
          return false;
        std::unique_ptr<PatternSpace> invalid
          (static_cast<PatternSpace*>(later.clone()));
        rel(*invalid,invalid->list[0],IRT_EQ,2);
        if (invalid->status() != SS_FAILED)
          return false;
        rel(later,later.list[0],IRT_EQ,1);
        if (later.status() == SS_FAILED)
          return false;

        // Both singleton prefixes are frequent, but no extension is frequent.
        const PatternMining::Database terminal({{1,1},{1,2},{2,1}});
        for (int item=1; item<=2; item++) {
          PatternSpace leaf(terminal,2);
          leaf.list.materialize(leaf,1);
          rel(leaf,leaf.list[0],IRT_EQ,item);
          if ((leaf.status() == SS_FAILED) ||
              !leaf.list.length().assigned() ||
              (leaf.list.length().val() != 1))
            return false;
        }
        return true;
      }
    };

    /// Input validation and exact rounding of percentage support
    class PatternInput : public ::Test::Base {
    public:
      PatternInput(void)
        : ::Test::Base("Int::OpenList::Pattern::Input",TestTags::check()) {}

      virtual bool run(void) {
        using namespace PatternMining;
        std::istringstream rows("# comment\n17 -1 1000000 -1 -2\r\n"
                                "17 -1 1000000 -1 -2\n");
        if (read_database(rows,Format::SPMF) !=
              Sequences({{17,1000000},{17,1000000}}) ||
            read_sequence("ABA",Format::LETTERS) != std::vector<int>({1,2,1}) ||
            read_sequence("17 1000000",Format::INTEGERS) !=
              std::vector<int>({17,1000000}) ||
            (minimum_support("33.333333",3) != 1) ||
            (minimum_support("33.333334",3) != 2) ||
            (minimum_support("100",2000000000) != 2000000000) ||
            (minimum_support("0.000001",2000000000) != 20))
          return false;
        for (const char* text : {"1 2 -1 -2", "1 -2", "1 -1",
                                 "1 -1 -2 2", "2147483648 -1 -2"}) {
          bool rejected = false;
          try {
            (void) read_sequence(text,Format::SPMF);
          } catch (const std::invalid_argument&) {
            rejected = true;
          }
          if (!rejected)
            return false;
        }
        for (const char* text : {"0", "101", "0.0000001",
                                 "184467440737095516160"}) {
          bool rejected = false;
          try {
            (void) minimum_support(text,3);
          } catch (const std::invalid_argument&) {
            rejected = true;
          }
          if (!rejected)
            return false;
        }
        return true;
      }
    };

    PatternEnumeration pattern_enumeration;
    PatternGrowth pattern_growth;
    PatternInput pattern_input;

    ValueCommit value_commit;
    ValueReplay value_replay;
    ImpossibleWindow impossible_window;

    Dispose disposal;
    CallbackLength callback_length;
    NoGPI no_gpi;
    FailedGet failed_get;

    BranchOrder branch_order;
    Transition transition;
    Views views;
    Constraint distinct("Distinct",Constraint::Kind::DISTINCT,3);
    Constraint rel_eq(Gecode::IRT_EQ,Gecode::IPL_BND);
    Constraint rel_eq_dom(Gecode::IRT_EQ,Gecode::IPL_DOM);
    Constraint rel_nq(Gecode::IRT_NQ,Gecode::IPL_DEF);
    Constraint rel_lq(Gecode::IRT_LQ,Gecode::IPL_DEF);
    Constraint rel_le(Gecode::IRT_LE,Gecode::IPL_DEF);
    Constraint rel_gq(Gecode::IRT_GQ,Gecode::IPL_DEF);
    Constraint rel_gr(Gecode::IRT_GR,Gecode::IPL_DEF);
    Constraint sliding("Sliding",Constraint::Kind::SLIDING,4);
    Constraint sliding_one("Sliding1",Constraint::Kind::SLIDING,3,1);
    Constraint sliding_sum("SlidingSum",Constraint::Kind::SLIDING_SUM,4);
    Constraint sliding_range("SlidingSumRange",Constraint::Kind::SLIDING_RANGE,4);
    Constraint range_one("SlidingSumRange1",Constraint::Kind::SLIDING_RANGE,3,1);
    Constraint min_max("MinMax",Constraint::Kind::MIN_MAX,3);
    Constraint precedence("Precede",Constraint::Kind::PRECEDE,3);
    Constraint precedence_same("PrecedeSame",Constraint::Kind::PRECEDE_SAME,3);
    Constraint sliding_three("Sliding3",Constraint::Kind::SLIDING,5,3);
    Constraint sum_three("SlidingSum3",Constraint::Kind::SLIDING_SUM,5,3);
    Constraint sum_three_dom("SlidingSum3Dom",Constraint::Kind::SLIDING_SUM,
                             5,3,Gecode::IPL_DOM);
    Constraint sum_two_bnd("SlidingSum2Bnd",Constraint::Kind::SLIDING_SUM,
                           4,2,Gecode::IPL_BND);
    Constraint range_three("SlidingSumRange3",Constraint::Kind::SLIDING_RANGE,5,3);
    Extremum min_result(true,false,Gecode::IPL_BND);
    Extremum max_result(false,false,Gecode::IPL_BND);
    Extremum min_domain(true,false,Gecode::IPL_DOM);
    Extremum max_domain(false,false,Gecode::IPL_DOM);
    Extremum min_alias(true,true,Gecode::IPL_DOM);
    Extremum max_alias(false,true,Gecode::IPL_DOM);
    ExtremumBounds extremum_bounds;
    EmptyMinimum empty_minimum;
    IncrementalConstraints incremental_constraints;

  }

}}

// STATISTICS: test-int
