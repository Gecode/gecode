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
 *     $Date: 2007-04-05 15:25:13 +0200 (Thu, 05 Apr 2007) $ by $Author: tack $
 *     $Revision: 4322 $
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

#include "gecode/kernel.hh"
#include "gecode/search.hh"

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

#include <vector>

namespace Gecode { namespace Search {

    /// Engine for decomposition during search
    class DDSEngine : public EngineCtrl {
    private:
      /// Recomputation stack of nodes
      ReCoStack          rcs;
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
      DDSEngine(unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Initialize engine to start at space \a s
      void init(Space* s);
      /// Reset engine to restart at space \a s
      void reset(Space* s);
      /// Search for next solution or decomposition
      ExploreStatus explore(Space*& s, int& closedDecomps, 
                            std::vector<int>& significantVars);
      /// Close current decomposition component
      void closeComponent(void);
      /// Close a singleton branch
      void closeSingletonBranch(void);
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
      DDS(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Return next solution (NULL, if none exists or search has been stopped)
      DDSEngine::ExploreStatus next(Space*& s, int& closedDecomps,
                                    std::vector<int>& significantVars);
      /// Close current decomposition component
      void closeComponent(void);
      /// Close a singleton branch
      void closeSingletonBranch(void);
      /// Return statistics
      Statistics statistics(void) const;
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
  class DDS : public Search::DDS {
  private:
    int closedDecomps;
    Search::DDSEngine::ExploreStatus status;
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
    Search::DDSEngine::ExploreStatus next(T*& s,
                                          std::vector<int>& significantVars);
    /// Close current decomposition component
    void closeComponent(void);
    /// Close current decomposition component
    void closeSingletonBranch(void);
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

  /**
   * \brief Branching for decomposition during search
   *
   * This branching analyzes the constraint graph and decomposes the search
   * if the graph contains more than one connected component.
   *
   * Otherwise, the branching behaves like a normal ViewValBranching.
   *
   */    
  template <class View, class Val, class DomSize, class ViewSel, class ValSel>
  class DecomposingViewValBranching : public Branching {
  protected:
    ViewArray<View> x;
    mutable int start;
    mutable bool selectFirst;

    DecomposingViewValBranching(Space* home, bool share,
                                DecomposingViewValBranching& b);

  public:
    DecomposingViewValBranching(Space* home, ViewArray<View>& x);
    virtual bool status(const Space* home) const;
    virtual const BranchingDesc* description(const Space* home) const;
    virtual ExecStatus commit(Space* home, const BranchingDesc* d,
                              unsigned int a);
    virtual Actor* copy(Space* home, bool share);
    
    /// Return specification for this branching given a variable map \a m
    virtual Reflection::ActorSpec spec(const Space* home,
                                       Reflection::VarMap& m) const;
    // /// Return specification for a branch
    // virtual Reflection::BranchingSpec
    // branchingSpec(const Space* home, 
    //               Reflection::VarMap& m,
    //               const BranchingDesc* d) const;
    /// Actor type identifier of this branching
    static Support::Symbol ati(void);
    /// Post branching according to specification
    static void post(Space* home, Reflection::VarMap& m,
                     const Reflection::ActorSpec& spec);
    
  };
 
}

#include "gecode/dds/branching.icc"
#include "gecode/dds/dds.icc"

// STATISTICS: dds-any
