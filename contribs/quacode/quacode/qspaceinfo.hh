/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
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
#ifndef __GECODE_QSPACEINFO_HH__
#define __GECODE_QSPACEINFO_HH__

#include <iomanip>
#include <quacode/qcsp.hh>
#include <vector>

namespace Gecode {
  // Forward declaration
  class QSpaceInfo;

  struct StrategyMethodValues {
    /// Method used to build strategy during search
    static const unsigned int BUILD       = 1; ///< We build strategy
    static const unsigned int DYNAMIC     = 2; ///< Select strategy to dynamic instead of static, if not possible the strategy will be not built
    static const unsigned int FAILTHROUGH = 4; ///< If static (not dynamic) strategy is selected but not possible, we go to dynamic strategy, but it may crashes because of lack of memory
    static const unsigned int EXPAND      = 8; ///< Expand all choices of solver (takes more computing time)
  };
  /// The value of the methode used to build the winning strategy
  typedef unsigned int StrategyMethod;

  namespace Int {
    /**
     * \brief Watch propagator for a FORALL variable
     * \ingroup FuncIntProp
     */
    template<class View>
      class Watch : public BinaryPropagator<View,PC_INT_DOM> {
        protected:
          using BinaryPropagator<View,PC_INT_DOM>::x0;
          using BinaryPropagator<View,PC_INT_DOM>::x1;

          /// Initial size of x0
          unsigned int x0Size;
          /// Constructor for cloning \a p
          Watch(Space& home, bool share, Watch& p);
          /// Constructor for posting
          Watch(Home home, View x0, View x1, unsigned int _x0Size);
        public:

          /// Copy propagator during cloning
          virtual Actor* copy(Space& home, bool share);
          /// Cost function, very low
          virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
          /// Perform propagation
          virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
          /// Post watch constraint for x0
          static ExecStatus post(Home home, View x0, View x1, unsigned int _x0Size);
      };
  }

  class DynamicStrategy;
  class DynamicExpandStrategy;
  class StrategyExplore;

  // A strategy corresponds to a way to store values of variable with order.
  // For example, a binder VxEyFz with x in {0,1,2} y in {-1,1} and z in {3,4} will
  // result in a data structure of size : 1 + #{0,1,2} * (1 + ((1+1) +  (1 + #{3,4}))) = 19 boxes
  // Only one value per existential variable and all values for universal variables plus
  // the id of the variable.
  // The general formula is: Qx_i...Qx_n, Q in {V,E}
  // we get: size(Qx_n) = if (Q == E) 2 else 1+#x_n
  //         size(Qx_i) = if (Q == E) 2 + size(Qx_{i+1}) else 1 + #x_i * (1 + size(Qx_{i+1}))
  // The storage of strategy is not obvious, for previous example, it will be:
  // |id(x)|v1(x)|id(y)|v(y)|id(z)|v1(z)|v2(z)|v2(x)|id(y)|v(y)|id(z)|v1(z)|v2(z)|v3(x)|id(y)|v(y)|id(z)|v1(z)|v2(z)|
  // Forward declaration
  class Strategy {
  friend class DynamicStrategy;
  friend class DynamicExpandStrategy;
  friend class StrategyExplore;
  public:
    /// Values for current strategy state
    static const unsigned int NONE = 0;
    static const unsigned int CHOICE = 1;
    static const unsigned int SUCCESS = 2;
    static const unsigned int FAILURE = 3;

  protected:
    union Box { // A box of an array is either a variable id, either a variable idx
      struct {
        int id; // Id of variable
        unsigned int nbAlt; // Number of explored boxes of the variable
        bool needNewBlock; // Flag to know if the variable will be stored in a new linked block
                           // Useful only for dynamic strategy
      } var;
      struct {
        int inf; // The inf value of a variable
        int sup; // The sup value of a variable
        bool leaf; // Flag to know if box is a leaf or not
      } val;
      struct {
        Box* ptr; // Ptr to next linked block of strategy
                  // Useful only for dynamic strategy
        unsigned int size; // Size of next block
      } nextBlock;
    };

    // Element of the current branch tree
    struct BPtr {
      int id; // Id of variable
      int vInf; // The inf value for this variable
      int vSup; // The sup value for this variable
      Box* ptrId; // Pointer to the box which stores data about the current opened variable
      Box* ptrCur; // Pointer to the current box for variable id (is moved according to the search)
      unsigned int curRemainingStrategySize; // Current remaining size below this choice

      // The following element is only meaningful for dynamic strategy
      std::vector<Box*> blocks; // List of block to free if this node will be deleted
      unsigned int curRemainingBlockSize; // Current remaining size of the current linked block below this choice

      BPtr() : id(-1), vInf(0), vSup(0), ptrId(NULL), ptrCur(NULL), curRemainingStrategySize(0), curRemainingBlockSize(0) {}
      BPtr(int i, Box* pId, unsigned int crss) : id(i), vInf(0), vSup(0), ptrId(pId), ptrCur(NULL), curRemainingStrategySize(crss), curRemainingBlockSize(0) {}
      BPtr(int i, Box* pId, unsigned int crss, Box* pNewBlock, unsigned int crbs) : id(i), vInf(0), vSup(0), ptrId(pId), ptrCur(NULL), curRemainingStrategySize(crss), curRemainingBlockSize(crbs) { if (pNewBlock != NULL) blocks.push_back(pNewBlock); }
    };

    Box* bx; /// Array of boxes of the strategy
    std::vector<unsigned int> domSize; // Size of domain of each variable
    std::vector<int> idxInCurBranch; // Index of variable in current branch tree
    unsigned int strategyTotalSize; // Size of all the strategy
    Box* cur; /// Current pointed box of the strategy

    std::vector<BPtr> curBranch; // Current branch of the search tree (contains id of variables)
    int curDepth; // Current depth of the search tree (last chosen variable corresponds do curDepth - 1)
    unsigned int lastEvent; // Last event recorded during search

    // Print current strategy
    void print(std::ostream& os, Box* p, unsigned int curRemainingStrategySize, int depth) const;

    // Backtrack the strategy from a failure to the node of \a vId
    // We assume that \a vId corresponds to an existential variable
    void backtrackFromFailure(int vId);
    // Backtrack the strategy from a failure to the node of \a vId
    // We assume that \a vId corresponds to a universal variable
    void backtrackFromSuccess(int vId);

    // Add a new variable (new node) to the strategy
    void addVariable(int vId);
    // Add a new value to the current strategy
    void addValue(int vId, int vInf, int vSup);

    // Make the last value added to be a leaf
    void makeLeaf(void);

    // Copy constructor
    Strategy(const Strategy& s);
  public:
    // Constructors
    Strategy();
    // Destructor
    virtual ~Strategy();

    // Copy current dynamic strategy
    virtual Strategy* copy(void) const { return new Strategy(*this); }

    // Add new variable to the strategy
    void add(unsigned int id, unsigned int size);
    // Build vector of boxes, assumes that modeling is ended
    QUACODE_EXPORT virtual bool strategyInit();
    /// Clear all data of the current strategy (used when search algorithm resets as well)
    QUACODE_EXPORT virtual void strategyReset();

    // Return current depth of strategy
    int depth(void) const { return curDepth; }
    // Return true if strategy has ever been allocated
    bool allocated(void) const { return (bx != NULL); }
    // Return depth of variable in the search tree, only usefull for debugging purpose
    int varDepth(int vId) const { return idxInCurBranch[vId]; }

    /// Called when a failed scenario was found
    virtual void scenarioFailed();
    /// Called when a successful scenario was found
    virtual void scenarioSuccess(const QSpaceInfo&);
    /// Record a new choice in the scenario, for variable \a vId
    /// with value [ \a vInf , \a vSup ]
    virtual void scenarioChoice(int vId, int vInf, int vSup);

    // Print current strategy
    QUACODE_EXPORT virtual void print(std::ostream& os) const;
  };

  // This is an expanded strategy. It is based on a static strategy where all choices
  // are expanded.
  // For static expanded strategy, some data structures of Strategy are unused. For example
  // curBranch is not needed as we don't need to know about the backtrack
  class StaticExpandStrategy : public Strategy {
    // Copy constructor
    StaticExpandStrategy(const StaticExpandStrategy& s);

    // Add a new value to the current strategy
    // Return true if the value is new and has been add
    void addValue(int vId, int vInf, int vSup, unsigned int sizeBelow, bool& bForce);

  public:
    // Constructors
    StaticExpandStrategy();
    // Destructor
    virtual ~StaticExpandStrategy();

    // Copy current dynamic strategy
    virtual StaticExpandStrategy* copy(void) const { return new StaticExpandStrategy(*this); }

    // Build vector of boxes, assumes that modeling is ended
    QUACODE_EXPORT virtual bool strategyInit();

    /// Called when a failed scenario was found
    virtual void scenarioFailed();
    /// Called when a successful scenario was found
    virtual void scenarioSuccess(const QSpaceInfo& qsi);
    /// Record a new choice in the scenario, for variable \a vId
    /// with value [ \a vInf , \a vSup ]
    virtual void scenarioChoice(int vId, int vInf, int vSup);
  };


  // A dynamic strategy is a dynamic data structure to record winning strategy.
  // Contrary to instance of class Strategy, operations are not in constant time.
  // So it may increases computing time. But it does not require to allocate the
  // full strategy in memory, so it may be uses to store strategies which cannot be
  // stored with full tree, but are smaller because sub-trees are cut during the search.
  class DynamicStrategy : public Strategy {
    // Print current strategy
    void print(std::ostream& os, Box* p, unsigned int curRemainingBlockSize, int depth) const;

  protected:
    // Size of the first block (the one pointed by the bx member)
    unsigned int bxBlockSize;

    // Convert constructor
    DynamicStrategy(const Strategy& s);
    // Copy constructor
    DynamicStrategy(const DynamicStrategy& s);
    // Backtrack the strategy from a failure to the node of \a vId
    // We assume that \a vId corresponds to an existential variable
    void backtrackFromFailure(int vId);
    // Backtrack the strategy from a failure to the node of \a vId
    // We assume that \a vId corresponds to a universal variable
    void backtrackFromSuccess(int vId);

    // Add a new variable (new node) to the strategy
    void addVariable(int vId);

  public:
    // Hint to bound the maximum allocated memory space for one block
    static const unsigned int sMaxBlockMemory = 2 * 1024;
    // Constructors
    DynamicStrategy();
    // Destructor
    virtual ~DynamicStrategy();

    // Copy current dynamic strategy
    virtual DynamicStrategy* copy(void) const { return new DynamicStrategy(*this); }

    // Build vector of boxes, assumes that modeling is ended
    QUACODE_EXPORT virtual bool strategyInit();
    /// Clear all data of the current strategy (used when search algorithm resets as well)
    QUACODE_EXPORT virtual void strategyReset();

    /// Called when a failed scenario was found
    virtual void scenarioFailed();
    /// Called when a successful scenario was found
    virtual void scenarioSuccess(const QSpaceInfo&);
    /// Record a new choice in the scenario, for variable \a vId
    /// with value [ \a vInf , \a vSup ]
    virtual void scenarioChoice(int vId, int vInf, int vSup);

    // Print current strategy
    QUACODE_EXPORT virtual void print(std::ostream& os) const;

    // Static function to convert strategy object
    static DynamicStrategy* fromStaticStrategy(const Strategy& ds);
  };

  // This is an expanded strategy. It is based on a dynamic strategy where all choices
  // are expanded.
  // For dynamic expanded strategy, we use the curBranch data structure to store the blocks
  // allowed. These blocks will be freed when backtrack because of failure
  class DynamicExpandStrategy : public DynamicStrategy {
    // Copy constructor
    DynamicExpandStrategy(const DynamicExpandStrategy& s);
    // Convert constructor
    DynamicExpandStrategy(const Strategy& s);

    // Add a new value to the current strategy
    // Return true if the value is new and has been add
    void addValue(int vId, int vInf, int vSup, unsigned int& sizeBelow, unsigned int& allocatedBlockSizeBelow, bool& bForce);

  public:
    // Constructors
    DynamicExpandStrategy();
    // Destructor
    virtual ~DynamicExpandStrategy();

    // Copy current dynamic strategy
    virtual DynamicExpandStrategy* copy(void) const { return new DynamicExpandStrategy(*this); }

    // Build vector of boxes, assumes that modeling is ended
    QUACODE_EXPORT virtual bool strategyInit();

    /// Called when a failed scenario was found
    virtual void scenarioFailed();
    /// Called when a successful scenario was found
    virtual void scenarioSuccess(const QSpaceInfo& qsi);
    /// Record a new choice in the scenario, for variable \a vId
    /// with value [ \a vInf , \a vSup ]
    virtual void scenarioChoice(int vId, int vInf, int vSup);

    // Static function to convert strategy object
    static DynamicExpandStrategy* fromStaticStrategy(const Strategy& ds);
  };

    /**
     * \brief Class to iterate over a strategy
     */
    class StrategyExplore {
    private:
      /// current strategy
      const Strategy& s;
      /// Stack of nodes to keep trace of parent
      std::vector<Strategy::Box*> backStack;
    public:
      /// Initialize
      StrategyExplore(const Strategy& s);
      /// Test whether there nodes left (not on a leaf)
      bool operator ()(void) const;
      /// Move iterator to next node below (take the value of the edge)
      void operator ++(int vEdge);
      /// Move iterator to node upside (the parent)
      void operator --(void);
      /// Return node number (corresponding to the id of the variable of this node)
      unsigned int variableId(void) const;
      /// Return the number of values below the current node (i.e. the number of edges)
      unsigned int nbValues(void) const;
      /// Return the value of i^th edge of current node
      int value(unsigned int ith) const;
    };


  class QSpaceInfo {
  friend class StaticExpandStrategy;
  friend class DynamicExpandStrategy;
  template<class View> friend class ::Gecode::Int::Watch;
  /// Data structure to store link between id of variable in the binder
  /// and the Gecode object (index of IntVar or BoolVar in the array of variables)
  struct LkBinderVarObj {
    static const unsigned int BOOL = 0;
    static const unsigned int INT  = 1;
    int id;
    int type;
    LkBinderVarObj(int _i, int _t) : id(_i), type(_t) {}
  };

  protected:
    class QSpaceSharedInfoO : public SharedHandle::Object {
    private:
      // Data structure to gather information about brancher shared among all spaces
      struct QBI {
        TQuantifier quantifier; // Brancher quantifier
        unsigned int offset; // Brancher offset (number of variables before it)
        unsigned int size; // Brancher size (number of variables in brancher)
        QBI(void) : quantifier(EXISTS), offset(0), size(0) {}
        QBI(TQuantifier _q, unsigned int _o, unsigned int _s)
           : quantifier(_q), offset(_o), size(_s) {}
      };

      std::vector<LkBinderVarObj> _linkIdVars; // Link between id variable in strategy and Gecode object
      std::vector<QBI> v; // Vector of data information about branchers
      Strategy* s; // Current strategy

      // Copy constructor
      QSpaceSharedInfoO(const QSpaceSharedInfoO&);
    public:
      QSpaceSharedInfoO(StrategyMethod sm);
      virtual ~QSpaceSharedInfoO(void);
      virtual SharedHandle::Object* copy(void) const { return new QSpaceSharedInfoO(*this); }

      /// Add new quantified brancher information
      void add(const QSpaceInfo& qsi, const BrancherHandle& bh, TQuantifier _q, const BoolVarArgs& x);
      /// Add new quantified brancher information
      void add(const QSpaceInfo& qsi, const BrancherHandle& bh, TQuantifier _q, const IntVarArgs& x);
      /// Return the quantifier used for the brancher \a bh
      forceinline TQuantifier brancherQuantifier(unsigned int id) const { return v[id-1].quantifier; }
      /// Return the offset computed when the brancher \a bh was added
      forceinline unsigned int brancherOffset(const unsigned int id) const { return v[id-1].offset; }
      // Return the last id of brancher stored in Shared Info.
      // Return 0 if no brancher recorded.
      forceinline int getLastBrancherId(void) const { return v.size(); }

      /// Initialize data structures of strategy, may return another method if strategy can't be allocated with given method
      StrategyMethod strategyInit(StrategyMethod sm);
      /// Clear all data of the current strategy (used when search algorithm resets as well)
      void strategyReset(void);
      /// Print the current strategy
      void strategyPrint(std::ostream& os) const;
      /// Called when a failed scenario was found
      void scenarioFailed(void);
      /// Called when a successful scenario was found
      void scenarioSuccess(const QSpaceInfo& qsi);
      /// Record a new choice event in the scenario, for brancher id \a id,
      /// variable position \a pos and value [ \a vInf , \a vSup ]
      void scenarioChoice(unsigned int id, int pos, int vInf, int vSup);

      // Return the vector of link between the id of variable in the binder and the
      // id of the same variable in its specific data structure
      const std::vector<LkBinderVarObj>& linkIdVars(void) const;
    };

    class QSpaceSharedInfo : public SharedHandle {
      public:
        /// Constructor
        QSpaceSharedInfo(void);
        /// Copy constructor
        QSpaceSharedInfo(const QSpaceSharedInfo& bi);
        /// Initialise for use
        void init(StrategyMethod sm);
        /// Add new brancher information
        void add(const QSpaceInfo& qsi, const BrancherHandle& bh, TQuantifier _q, const IntVarArgs& x);
        void add(const QSpaceInfo& qsi, const BrancherHandle& bh, TQuantifier _q, const BoolVarArgs& x);

        /// SharedInfo access
        /// Return the quantifier used for the brancher \a id
        TQuantifier brancherQuantifier(unsigned int id) const;
        /// Return the offset computed when the brancher \a id was added
        unsigned int brancherOffset(unsigned int id) const;
        /// Initialize data structures of strategy, may return another method if strategy can't be allocated with given method
        StrategyMethod strategyInit(StrategyMethod sm);
        /// Clear all data of the current strategy (used when search algorithm resets as well)
        void strategyReset(void);
        /// Print the current strategy
        void strategyPrint(std::ostream& os) const;
        /// Called when a failed scenario was found
        void scenarioFailed(void);
        /// Called when no strategy has been found (failed problem)
        void scenarioSuccess(const QSpaceInfo& qsi);
        /// Called when a successful scenario was found
        // Record a new choice event in the strategy, for brancher id \a id,
        // variable position \a pos and value [ \a vInf , \a vSup ]
        void scenarioChoice(unsigned int id, int pos, int vInf, int vSup);

        // Return the last id of brancher stored in Shared Info.
        // Return -1 if not brancher recorded.
        int getLastBrancherId(void) const;
        // Return the vector of link between the id of variable in the binder and the
        // id of the same variable in its specific data structure
        const std::vector<LkBinderVarObj>& linkIdVars(void) const;
    };

    /// Wrapper function to record choice in scenario
    static void scenarioChoice(const Space &home, const BrancherHandle& bh, unsigned int a, BoolVar x, int i, const int& n, std::ostream& );
    static void scenarioChoice(const Space &home, const BrancherHandle& bh, unsigned int a, IntVar x, int i, const int& n, std::ostream& );

    /// The following arrays are not maintained during cloning !!!
    /// Only meaningful during modeling
    IntVarArgs _watchedIntVariables;
    IntVarArgs _unWatchedIntVariables;
    BoolVarArgs _watchedBoolVariables;
    BoolVarArgs _unWatchedBoolVariables;
    /// Return the corresponding unWatched variable if founded, NULL otherwise.
    /// If Null is returned, it means that the variable isn't universally quantified.
    BoolVar* unWatched(BoolVar x);
    /// Return the corresponding unWatched variable if founded, NULL otherwise.
    /// If Null is returned, it means that the variable isn't universally quantified.
    IntVar* unWatched(IntVar x);

    /// Shared information among all spaces
    QSpaceSharedInfo sharedInfo;

    /// Boolean flag to know if we have to record the winning strategy
    bool bRecordStrategy;
    /// The current strategy method to use during search
    StrategyMethod curStrategyMethod;

    // Number of WatchConstraint in space
    unsigned int nbWatchConstraint;
    /// Record a new watch constraint in space
    void addWatchConstraint(void);
    /// Remove a watch constraint of space
    void delWatchConstraint(void);

    /// Array of all boolean variables to branch with
    BoolVarArray _boolVars;
    /// Array of all integer variables to branch with
    IntVarArray _intVars;

    /// Update QSpace with data from brancher handle \a bh which contains variables \a x quantified with
    /// quantifier \a _q. It will also update data shared by all clones.
    QUACODE_EXPORT void updateQSpaceInfo(const BrancherHandle& bh, TQuantifier _q, const IntVarArgs& x);
    QUACODE_EXPORT void updateQSpaceInfo(const BrancherHandle& bh, TQuantifier _q, const BoolVarArgs& x);
  private:
    /// Copy Constructor (disabled)
    QSpaceInfo(const QSpaceInfo& qs);
  public:
    /// Default constructor
    QUACODE_EXPORT QSpaceInfo(void);
    /// Default destructor
    QUACODE_EXPORT ~QSpaceInfo(void);
    /// Copy Constructor
    QUACODE_EXPORT QSpaceInfo(Space& home, bool share, QSpaceInfo& qs);

    /// Return the quantifier used for the brancher \a id
    TQuantifier brancherQuantifier(unsigned int id) const;
    /// Return the offset computed when the brancher \a id was added
    unsigned int brancherOffset(unsigned int id) const;
    /// Return the number of recorded watch constraints in space
    unsigned int watchConstraints(void) const;

    /// Return the quantifier of the given integer variable
    TQuantifier quantifier(IntVar x) { return (unWatched(x)?FORALL:EXISTS); };
    /// Return the quantifier of the given boolean variable
    TQuantifier quantifier(BoolVar x) { return (unWatched(x)?FORALL:EXISTS); };

    /// Return the current method used to build strategy
    StrategyMethod strategyMethod(void) const;
    /// Set the current method used to build strategy
    void strategyMethod(StrategyMethod sm);
    /// Initialize data structures of strategy. If the current strategy method can't
    /// be used, it will update the strategy method value and returns it.
    StrategyMethod strategyInit();
    /// Clear all data of the current strategy (used when search algorithm resets as well)
    void strategyReset();
    /// Print the current strategy
    void strategyPrint(std::ostream& os) const;
    /// Called when no strategy has been found (failed problem)
    void strategyFailed();
    /// Called when a strategy has been found (satisfiable problem)
    void strategySuccess();
    /// Called when a failed scenario was found
    void scenarioFailed();
    /// Called when a successful scenario was found
    void scenarioSuccess();

    /// Return the unwatched variable corresponding to the watched one
    IntVar getUnWatched(IntVar x);
    /// Return the unwatched variable corresponding to the watched one
    BoolVar getUnWatched(BoolVar x);

    /// Set the given boolean variable \a x to be universal (It posts the according watching constraint)
    void setForAll(Home home, BoolVar x);
    /// Set the given array of boolean variables \a x to be universal (It posts the according watching constraint)
    void setForAll(Home home, const BoolVarArgs& x);
    /// Set the given integer variable \a x to be universal (It posts the according watching constraint)
    void setForAll(Home home, IntVar x);
    /// Set the given array of integer variables \a x to be universal (It posts the according watching constraint)
    void setForAll(Home home, const IntVarArgs& x);

    /// Function call when a new instance is found in QDFS algorithm
    virtual void eventNewInstance(void) const;

    /// Wrapper function to print data during branching
    static BoolVarValPrint customBoolVVP;
    static IntVarValPrint customIntVVP;
    template <class VarType> static void runCustomChoice(const Space &home, const BrancherHandle& bh, unsigned int alt, VarType x, int pos, const int& val, std::ostream& os);
    template <class VarType> static void doubleChoice(const Space &home, const BrancherHandle& bh, unsigned int alt, VarType x, int pos, const int& val, std::ostream& os);
    template <class VarType> static void tripleChoice(const Space &home, const BrancherHandle& bh, unsigned int alt, VarType x, int pos, const int& val, std::ostream& os);

    /// Branch over boolean variable \a x, quantified variable selection \a vars and value selection \a vals
    template <class BranchType> QUACODE_EXPORT std::vector<BrancherHandle> branch(Home home, const BoolVar& x, BranchType vars, IntValBranch vals, BoolBranchFilter bf=NULL, BoolVarValPrint vvp=NULL);
    /// Branch over boolean variables \a x, quantified variable selection \a vars and value selection \a vals
    template <class BranchType> QUACODE_EXPORT std::vector<BrancherHandle> branch(Home home, const BoolVarArgs& x, BranchType vars, IntValBranch vals, BoolBranchFilter bf=NULL, BoolVarValPrint vvp=NULL);
    /// Branch over integer variable \a x, quantified variable selection \a vars and value selection \a vals
    template <class BranchType> QUACODE_EXPORT std::vector<BrancherHandle> branch(Home home, const IntVar& x, BranchType vars, IntValBranch vals, IntBranchFilter bf=NULL, IntVarValPrint vvp=NULL);
    /// Branch over integer variables \a x, quantified variable selection \a vars and value selection \a vals
    template <class BranchType> QUACODE_EXPORT std::vector<BrancherHandle> branch(Home home, const IntVarArgs& x, BranchType vars, IntValBranch vals, IntBranchFilter bf=NULL, IntVarValPrint vvp=NULL);

  };
}

#include <quacode/qspaceinfo.hpp>
#include <quacode/qint/watch.hpp>

#endif
