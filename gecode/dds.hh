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

#include <boost/graph/adjacency_list.hpp>
#include <vector>

namespace Gecode {

  namespace Decomposition {
    
    /**
     * \brief Constraint graph supporting connected component analysis
     *
     */
    class ConstraintGraph {
    protected:
      /// The actual graph implementation
      boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> g;
    public:
      /// Construct graph of initial size \a n
      ConstraintGraph(int n);

      /// Add an edge between nodes \a i and \a j
      void addEdge(int i, int j);
      
      /// Output dot representation of the graph on \a out
      GECODE_DDS_EXPORT void printDot(std::ostream & out = std::cout);

      /// A type that describes a component
      typedef std::vector<int> ComponentLabel;

      /** 
       * \brief Perform connected component analysis
       *
       * Fills each \a label[i] with the component index for the node i
       * and return the overall number of components
       */
      GECODE_DDS_EXPORT int getComponentLabel(ComponentLabel &label);
    };

    /// Branching description for decomposition
    class DecompDesc : public BranchingDesc {
    protected:
      /// The significant variables in the components
      const ConstraintGraph::ComponentLabel label;
      /// Start index
      const int start;
      /// Selected variables
      const std::vector<int> select;

    public:
      /// Constructor
      DecompDesc(const Branching* b, int start,
                 const ConstraintGraph::ComponentLabel& l,
                 const std::vector<int>& select);
      /// Return the stored label
      const ConstraintGraph::ComponentLabel& getComponentLabel(void) const;
      /// Return the selected view index for component \a alt
      int getSelection(unsigned int alt) const;
      /// Return the start index
      int getStart(void) const;
      /// Return the significant variables for alternative \a alt in \a sv
      void significantVars(int alt, std::vector<int>& sv) const;
      /// Return size
      virtual size_t size(void) const;
    };

    /// A branching description for singleton components
    class SingletonDescBase {
    protected:
      /// Size of the domain of the singleton component
      unsigned int _size;
    public:
      /// Return size of the domain of the singleton component
      unsigned int domainSize(void) const;
    };

    /// Wrapper class for PosValDesc that stores an additional component size
    template <class Val, int alt>
    class SingletonDesc
    : public PosValDesc<Val,alt>, public SingletonDescBase {
    public:
      /// Constructor
      SingletonDesc(const Branching* b, Val v, int start, unsigned int size);
    };
  }

  namespace Search {

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
     * spaces. In order to use depth-first search on subclasses of DDSSpace, 
     * additional functionality providing the necessary typecasts is available
     * in Gecode::DDS.
     */
    class GECODE_DDS_EXPORT DDS {
    protected:
      /// Engine used for exploration
      DDSEngine e;
    public:
      /**
       * \brief Initialize search engine
       * \param s root node (subclass of Space)
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
   * This class supports dynamic decomposition depth-first search for
   * subclasses \a T of DDSSpace.
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
     * \param s root node (subclass of DDSSpace)
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
   * \param s root node (subclass \a T of DDSSpace)
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
    
  template <class View, class Val, class DomSize, class ViewSel, class ValSel>
  class DecomposingViewValBranching : public Branching {
  protected:
    ViewArray<View> x;
    mutable int start;
    mutable bool selectFirst;

    DecomposingViewValBranching(Space* home, bool share,
                                DecomposingViewValBranching& b);

    // merges all singlet cluster into the last non-singlet cluster
    // and returns the new label number
    int mergeAssigned(Decomposition::ConstraintGraph::ComponentLabel & label,
                      int labelNumber) const;
  public:
    DecomposingViewValBranching(Space* home, ViewArray<View>& x);
    virtual bool status(const Space* home) const;
    virtual const BranchingDesc* description(const Space* home) const;
    virtual ExecStatus commit(Space* home, const BranchingDesc* d,
                              unsigned int a);
    virtual Actor* copy(Space* home, bool share);
  };
 
}

#include "gecode/dds/constraintgraph.icc"
#include "gecode/dds/branching.icc"
#include "gecode/dds/dds.icc"

// STATISTICS: dds-any
