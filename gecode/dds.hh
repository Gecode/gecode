/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#ifndef __GECODE_DDS_HH__
#define __GECODE_DDS_HH__

#include "gecode/search.hh"
#include "gecode/int.hh"

#include <vector>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_DDS
#define GECODE_DDS_EXPORT __declspec( dllexport )
#else
#define GECODE_DDS_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_DDS_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_DDS_EXPORT
#endif

#endif

namespace Gecode {
  namespace Decomposition {

    /*
     * Decomposition during search
     *
     */
  
    /// Engine for decomposition during search
    class DDSEngine : public Search::EngineCtrl {
    private:
      /// Recomputation stack of nodes
      Search::ReCoStack          rcs;
      /// Current space being explored
      Space*             cur;
      /// Copying recomputation distance
      const unsigned int c_d;
      /// Distance until next clone
      unsigned int       d;
    public:
      /// The possible status of the engine after calling next
      enum ExploreStatus {
        EXS_SOLUTION, EXS_START_DECOMP, EXS_NEXT_DECOMP, EXS_END_DECOMP,
        EXS_SINGLETON_BRANCH, EXS_DONE
      };
      /**
       * \brief Initialize engine
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      DDSEngine(unsigned int c_d, unsigned int a_d,
                Search::Stop* st, size_t sz);
      /// Initialize engine to start at space \a s
      void init(Space* s);
      /// Reset engine to restart at space \a s
      void reset(Space* s);
      /// Search for next solution or decomposition
      ExploreStatus explore(Space*& s, int& closedDecomps, 
                            std::vector<int>& significantVars);
      /// Close current decomposition component
      void closeComponent(void);
      /// Return stack size used by engine
      size_t stacksize(void) const;
      /// Destructor
      ~DDSEngine(void);
    };

    /**
     * \brief Dynamic Decomposition Depth-first search engine
     *
     * This class implements dynamic decomposition depth-first exploration for
     * spaces.
     */
    class GECODE_DDS_EXPORT DDS {
    protected:
      /// Engine used for exploration
      DDSEngine e;
    public:
      /**
       * \brief Initialize search engine
       * \param s root node
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      DDS(Space* s, unsigned int c_d, unsigned int a_d,
          Search::Stop* st, size_t sz);
      /// Return next solution (NULL, if none exists or search has been stopped)
      DDSEngine::ExploreStatus next(Space*& s, int& closedDecomps,
                                    std::vector<int>& significantVars);
      /// Close current decomposition component
      void closeComponent(void);
      /// Return statistics
      Search::Statistics statistics(void) const;
      /// Check whether engine has been stopped
      bool stopped(void) const;
    };  
  }

  /**
   * \brief Dynamic Decomposition Depth-first search engine
   *
   * This class supports dynamic decomposition depth-first search.
   * \ingroup TaskIntSearch
   */
  template <class T>
  class DDS : public Decomposition::DDS {
  private:
    int closedDecomps;
    Decomposition::DDSEngine::ExploreStatus status;
    std::vector<int> significantVars;
    T* sol;
  public:
    /**
     * \brief Initialize search engine
     * \param s root node
     * \param c_d minimal recomputation distance
     * \param a_d adaptive recomputation distance
     * \param st %Stop-object
     */
    DDS(T* s,
        unsigned int c_d=Search::Config::c_d,
        unsigned int a_d=Search::Config::a_d,
        Search::Stop* st=NULL);
    /// Return next solution or decomposition
    Decomposition::DDSEngine::ExploreStatus next(T*& s,
                                          std::vector<int>& significantVars);
    /// Close current decomposition component
    void closeComponent(void);
  };

  /**
   * \brief Perform counting dynamic decomposition depth-first search
   * \param s root node
   * \param c_d minimal recomputation distance
   * \param a_d adaptive recomputation distance
   * \ingroup TaskIntSearch
   */
  template <class Int, class T>
  Int countDDS(T* s,
               unsigned int c_d=Search::Config::c_d,
               unsigned int a_d=Search::Config::a_d,
               Search::Statistics* stat=NULL,
               Search::Stop* st=NULL);

  /// Branch over \a x with variable selection \a vars and value selection \a vals, applying decomposition during search if possible
  GECODE_DDS_EXPORT void
  decomposingBranch(Space& home, const IntVarArgs& x, 
                    IntVarBranch vars, IntValBranch vals);
  /// Branch over \a x with variable selection \a vars and value selection \a vals, applying decomposition during search if possible
  GECODE_DDS_EXPORT void
  decomposingBranch(Space& home, const BoolVarArgs& x, 
                    IntVarBranch vars, IntValBranch vals);

}

#include "gecode/dds/decomposing-branching.hpp"
#include "gecode/dds/dds.hpp"

#endif

// STATISTICS: dds-any
