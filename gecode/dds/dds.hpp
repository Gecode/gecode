/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Martin Mann <mmann@informatik.uni-freiburg.de>
 *     Sebastian Will <will@informatik.uni-freiburg.de>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *     Martin Mann, 2008
 *     Sebastian Will, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <stack>

namespace Gecode {
  namespace Decomposition {

    /*
     * DDS engine
     *
     */
    forceinline
    DDSEngine::DDSEngine(unsigned int c_d0, unsigned int a_d0,
                         Search::Stop* st, size_t sz)
      : Search::EngineCtrl(st,sz), rcs(a_d0), cur(NULL), c_d(c_d0), d(0) {}


    forceinline void
    DDSEngine::init(Space* s) {
      cur = s;
    }

    forceinline void
    DDSEngine::reset(Space* s) {
      delete cur;
      rcs.reset();
      cur = s;
      d   = 0;
      EngineCtrl::reset(s);
    }

    forceinline size_t
    DDSEngine::stacksize(void) const {
      return rcs.stacksize();
    }

    forceinline DDSEngine::ExploreStatus
    DDSEngine::explore(Space*& sol, int& closedDecomps,
                       std::vector<int>& significantVars) {
      start();
      while (true) {
        while (cur) {
          if (stop(stacksize()))
            return EXS_DONE;
          switch (cur->status(propagate)) {
          case SS_FAILED:
            fail++;
            delete cur;
            cur = NULL;
            EngineCtrl::current(NULL);
            break;
          case SS_SOLVED:
            {
              sol = cur;
              cur = NULL;
              EngineCtrl::current(NULL);
              return EXS_SOLUTION;
            }
          case SS_BRANCH:
            {
              Space* c;
              if ((d == 0) || (d >= c_d)) {
                clone++;
                c = cur->clone(true);
                d = 1;
              } else {
                c = NULL;
                d++;
              }
              const BranchingDesc* desc = rcs.push(*this,cur,c);
              EngineCtrl::push(c,desc);
              commit++;
              if (const Decomposition::SingletonDescBase* sd=
                  dynamic_cast<const Decomposition::SingletonDescBase*>(desc))
              {
                sol = cur;
                rcs.closeBranch<Decomposition::SingletonDescBase,
                                true>(*this);
                cur = NULL;
                EngineCtrl::current(NULL);
                significantVars.resize(2);
                significantVars[0] = sd->domainSize();
                return EXS_SINGLETON_BRANCH;
              } else if (const Decomposition::DecompDesc* dd=
                  dynamic_cast<const Decomposition::DecompDesc*>(desc)) {
                cur->commit(*desc,0);
                significantVars = dd->significantVars(0);
                // decomp++;
                return EXS_START_DECOMP;
              }
              cur->commit(*desc,0);
            }
            break;
          default: GECODE_NEVER;
          }
        }
        int alt=-1;
        if (const BranchingDesc* nextDesc =
              rcs.nextDesc<Decomposition::DecompDesc>(*this, alt,
                                                      closedDecomps)) {
          cur = static_cast<Space*>(rcs.recompute<false>(d,*this));
          EngineCtrl::current(cur);
          if (const Decomposition::DecompDesc* dd=
                dynamic_cast<const Decomposition::DecompDesc*>(nextDesc)) {
            significantVars = dd->significantVars(alt);
            return EXS_NEXT_DECOMP;
          }
          if (closedDecomps > 0) {
            closedDecomps--;
            return EXS_END_DECOMP;
          }
        } else {
          return EXS_DONE;
        }
      }
      return EXS_DONE;
    }

    forceinline void
    DDSEngine::closeComponent(void) {
      rcs.closeBranch<Decomposition::DecompDesc,false>(*this);
      cur = NULL;
      EngineCtrl::current(NULL);
    }

    forceinline
    DDSEngine::~DDSEngine(void) {
      delete cur;
      rcs.reset();
    }

  }

  /*
   * Control for DDS search engine
   *
   */

  template <class T>
  forceinline
  DDS<T>::DDS(T* s, unsigned int c_d, unsigned int a_d, Search::Stop* st)
    : Decomposition::DDS(s,c_d,a_d,st,sizeof(T)), closedDecomps(-1),
      sol(NULL) {}

  template <class T>
  forceinline Decomposition::DDSEngine::ExploreStatus
  DDS<T>::next(T*& s, std::vector<int>& sv) {
    switch (closedDecomps) {
    case -1:
      {
        closedDecomps = 0;
        Space* result;
        status =
          Decomposition::DDS::next(result, closedDecomps, significantVars);
        sol = static_cast<T*>(result);

        if (closedDecomps > 0) {
          closedDecomps--;
          s = NULL;
          return Decomposition::DDSEngine::EXS_END_DECOMP;
        }
        closedDecomps--;
        s = sol;
        sv = significantVars;
        return status;
      }
    case 0:
      {
        closedDecomps--;
        s = sol;
        sv = significantVars;
        return status;
      }
    default:
      {
        closedDecomps--;
        s = NULL;
        return Decomposition::DDSEngine::EXS_END_DECOMP;
      }
    }
  }

  template <class T>
  forceinline void
  DDS<T>::closeComponent(void) {
    Decomposition::DDS::closeComponent();
  }

  template <class Int, class T>
  Int
  countDDS(T* s, unsigned int c_d, unsigned int a_d, Search::Statistics* stat,
           Search::Stop* st) {
    DDS<T> d(s,c_d,a_d,st);
    std::stack<Int> countStack;
    countStack.push(0);
    std::vector<int> significantVars;
    T* sol;
    while (true) {
      Decomposition::DDSEngine::ExploreStatus status =
        d.next(sol, significantVars);
      switch (status) {
      case Decomposition::DDSEngine::EXS_SOLUTION:
        delete sol;
        countStack.top() = countStack.top() + 1;
        break;
      case Decomposition::DDSEngine::EXS_SINGLETON_BRANCH:
        delete sol;
        countStack.top() = countStack.top() + significantVars[0];
        break;
      case Decomposition::DDSEngine::EXS_START_DECOMP:
        countStack.push(1); // new decomposition node
        countStack.push(0); // first component
        break;
      case Decomposition::DDSEngine::EXS_NEXT_DECOMP:
        {
          Int tmp = countStack.top();
          if (tmp == 0) {
            // previous component had no solutions
            d.closeComponent();
          } else {
            countStack.pop();
            assert(!countStack.empty());
            countStack.top() = countStack.top() * tmp;
            countStack.push(0); // this, the next component
          }
        }
        break;
      case Decomposition::DDSEngine::EXS_END_DECOMP:
        {
          Int tmp = countStack.top();
          countStack.pop();
          tmp = countStack.top() * tmp;
          countStack.pop();
          countStack.top() += tmp;
        }
        break;
      case Decomposition::DDSEngine::EXS_DONE:
        if (stat!=NULL)
          *stat = d.statistics();
        return countStack.top();
        break;
      }
    }
  }
}

// STATISTICS: dds-any
