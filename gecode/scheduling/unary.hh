/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_SCHEDULING_UNARY_HH__
#define __GECODE_SCHEDULING_UNARY_HH__

#include <gecode/scheduling.hh>

/**
 * \namespace Gecode::Scheduling::Unary
 *
 * The algorithms and data structures follow (mostly):
 *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
 *   Charles University, Prague, Czech Republic, 2007.
 *
 * \brief Scheduling for unary resources
 */

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Unary (mandatory) task with fixed processing time
  class ManFixTask {
  protected:
    /// Start time
    Int::IntView _s;
    /// Processing time
    int _p;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ManFixTask(void);
    /// Initialize with start time \a s and processing time \a p
    ManFixTask(IntVar s, int p);
    /// Initialize with start time \a s and processing time \a p
    void init(IntVar s, int p);
    //@}

    /// \name Value access
    //@{
    /// Return earliest start time
    int est(void) const;
    /// Return earliest completion time
    int ect(void) const;
    /// Return latest start time
    int lst(void) const;
    /// Return latest completion time
    int lct(void) const;
    /// Return start time
    IntVar st(void) const;
    /// Return processing time
    int p(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether task is assigned
    bool assigned(void) const;
    //@}

    /// \name Value update
    //@{
    /// Update earliest start time to \a n
    ModEvent est(Space& home, int n);
    /// Update earliest completion time to \a n
    ModEvent ect(Space& home, int n);
    /// Update latest start time to \a n
    ModEvent lst(Space& home, int n);
    /// Update latest completion time to \a n
    ModEvent lct(Space& home, int n);
    //@}

    /// \name Cloning
    //@{
    /// Update this task to be a clone of task \a t
    void update(Space& home, bool share, ManFixTask& t);
    //@}

    /// \name Dependencies
    //@{
    /// Subscribe propagator \a p to task
    void subscribe(Space& home, Propagator& p);
    /// Cancel subscription of propagator \a p for task
    void cancel(Space& home, Propagator& p);
    //@}

  };

  /**
   * \brief Print task in format est:p:lct
   * \relates Task
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t);


  /// Unary optional task with fixed processing time
  class OptFixTask : public ManFixTask {
  protected:
    /// Boolean view whether task is mandatory (= 1) or not
    Int::BoolView _m;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    OptFixTask(void);
    /// Initialize with start time \a s, processing time \a p, and mandatory flag \a m
    OptFixTask(IntVar s, int p, BoolVar m);
    /// Initialize with start time \a s, processing time \a p, and mandatory flag \a m
    void init(IntVar s, int p, BoolVar m);
    //@}

    /// \name Value access
    //@{
    /// Whether task is mandatory
    bool mandatory(void) const;
    /// Whether task is excluded
    bool excluded(void) const;
    /// Whether task can still be optional
    bool optional(void) const;
    //@}

    //@{
    /// Test whether task is assigned
    bool assigned(void) const;
    //@}

    /// \name Value update
    //@{
    /// Mark task as mandatory
    ModEvent mandatory(Space& home);
    /// Mark task as excluded
    ModEvent excluded(Space& home);
    //@}

    /// \name Cloning
    //@{
    /// Update this task to be a clone of task \a t
    void update(Space& home, bool share, OptFixTask& t);
    //@}

    /// \name Dependencies
    //@{
    /// Subscribe propagator \a p to task
    void subscribe(Space& home, Propagator& p);
    /// Cancel subscription of propagator \a p for task
    void cancel(Space& home, Propagator& p);
    //@}

  };

  /**
   * \brief Print optional task in format est:p:lct:m
   * \relates OptFixTask
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFixTask& t);

}}}

#include <gecode/scheduling/unary/task.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Task mapper: turns a task view into its dual
  template<class TaskView>
  class FwdToBwd : public TaskView {
  public:
    /// \name Value access
    //@{
    /// Return earliest start time
    int est(void) const;
    /// Return earliest completion time
    int ect(void) const;
    /// Return latest start time
    int lst(void) const;
    /// Return latest completion time
    int lct(void) const;
    //@}

    /// \name Value update
    //@{
    /// Update earliest start time to \a n
    ModEvent est(Space& home, int n);
    /// Update earliest completion time to \a n
    ModEvent ect(Space& home, int n);
    /// Update latest start time to \a n
    ModEvent lst(Space& home, int n);
    /// Update latest completion time to \a n
    ModEvent lct(Space& home, int n);
    //@}
  };

}}}

#include <gecode/scheduling/unary/task-mapper.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Forward mandatory task view
  typedef ManFixTask ManFixTaskFwd;

  /// Backward (dual) mandatory task view
  typedef FwdToBwd<ManFixTaskFwd> ManFixTaskBwd;

  /// Forward optional task view
  typedef OptFixTask OptFixTaskFwd;

  /// Backward (dual) optional task view
  typedef FwdToBwd<OptFixTaskFwd> OptFixTaskBwd;


  /**
   * \brief Print backward task view in format est:p:lct
   * \relates ManFixTaskBwd
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTaskBwd& t);

  /**
   * \brief Print optional backward task view in format est:p:lct:m
   * \relates OptFixTaskBwd
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFixTaskBwd& t);



  /**
   * \brief Traits class for mapping task views to tasks
   *
   * Each task view must specialize this traits class and add a \code
   * typedef \endcode for the task corresponding to this task view.
   */
  template<class TaskView>
  class TaskViewTraits {};

  /// Task view traits for forward task views
  template<>
  class TaskViewTraits<ManFixTaskFwd> {
  public:
    /// The task type
    typedef ManFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<ManFixTaskBwd> {
  public:
    /// The task type
    typedef ManFixTask Task;
  };

  /// Task view traits for forward optional task views
  template<>
  class TaskViewTraits<OptFixTaskFwd> {
  public:
    /// The task type
    typedef OptFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<OptFixTaskBwd> {
  public:
    /// The task type
    typedef OptFixTask Task;
  };

  /**
   * \brief Traits class for mapping tasks to task views
   *
   * Each task must specialize this traits class and add \code
   * typedef \endcode for the task views corresponding to this task.
   */
  template<class Task>
  class TaskTraits {};

  /// Task traits for mandatory tasks
  template<>
  class TaskTraits<ManFixTask> {
  public:
    /// The forward task view type
    typedef ManFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef ManFixTaskBwd TaskViewBwd;
  };

  /// Task traits for optional tasks
  template<>
  class TaskTraits<OptFixTask> {
  public:
    /// The forward task view type
    typedef OptFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef OptFixTaskBwd TaskViewBwd;
    /// The corresponding mandatory task
    typedef ManFixTask ManTask;
  };

}}}

#include <gecode/scheduling/unary/task-view.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Task array
  template<class Task>
  class TaskArray {
  private:
    /// Number of tasks (size)
    int n;
    /// Tasks
    Task* t;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (array of size 0)
    TaskArray(void);
    /// Allocate memory for \a n tasks (no initialization)
    TaskArray(Space& home, int n);
    /// Initialize from task array \a a (share elements)
    TaskArray(const TaskArray<Task>& a);
    /// Initialize from task array \a a (share elements)
    const TaskArray<Task>& operator =(const TaskArray<Task>& a);
    //@}

    /// \name Array size
    //@{
    /// Return size of array (number of elements)
    int size(void) const;
    /// Set size of array (number of elements) to \a n, must not be larger
    void size(int n);
    //@}

    /// \name Array elements
    //@{
    /// Return task at position \a i
    Task& operator [](int i);
    /// Return task at position \a i
    const Task& operator [](int i) const;
    //@}

    /// \name Dependencies
    //@{
    /// Subscribe propagator \a p to all tasks
    void subscribe(Space& home, Propagator& p);
    /// Cancel subscription of propagator \a p for all tasks
    void cancel(Space& home, Propagator& p);
    //@}

    /// \name Cloning
    //@{
    /// Update array to be a clone of array \a a
    void update(Space&, bool share, TaskArray& a);
    //@}

  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates TaskArray
   */
  template<class Char, class Traits, class Task>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, 
              const TaskArray<Task>& t);


  /// Task view array
  template<class TaskView>
  class TaskViewArray {
  protected:
    /// The underlying task type
    typedef typename TaskViewTraits<TaskView>::Task Task;
    /// Access to task array
    TaskArray<Task>& t;
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize from task array \a a
    TaskViewArray(TaskArray<Task>& t);
    //@}

    /// \name Array information
    //@{
    /// Return size of array (number of elements)
    int size(void) const;
    //@}

    /// \name Array elements
    //@{
    /// Return task view at position \a i
    TaskView& operator [](int i);
    /// Return task view at position \a i
    const TaskView& operator [](int i) const;
    //@}
  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates TaskViewArray
   */
  template<class Char, class Traits, class TaskView>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, 
              const TaskViewArray<TaskView>& t);

}}}

#include <gecode/scheduling/unary/task-array.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// How to sort tasks
  enum SortTaskOrder {
    STO_EST, ///< Sort by earliest start times
    STO_ECT, ///< Sort by earliest completion times
    STO_LST, ///< Sort by latest start times
    STO_LCT  ///< Sort by latest completion times
  };

  /// Sort task view array \a t according to \a sto and \a inc (increasing or decreasing)
  template<class TaskView, SortTaskOrder sto, bool inc>
  void sort(TaskViewArray<TaskView>& t);

  /// Sort \a map for task view array \a t according to \a sto and \a inc (increasing or decreasing)
  template<class TaskView, SortTaskOrder sto, bool inc>
  void sort(int* map, const TaskViewArray<TaskView>& t);

}}}

#include <gecode/scheduling/unary/sort.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Allows to iterate over task views according to a specified order
  template<class TaskView, SortTaskOrder sto, bool inc>
  class TaskViewIterator {
  protected:
    /// Map for iteration order
    int* map;
    /// Current position
    int i;
  public:
    /// Initialize iterator
    TaskViewIterator(Region& r, const TaskViewArray<TaskView>& t);
    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a task
    bool operator ()(void) const;
    /// How many tasks are left to be iterated
    int left(void) const;
    /// Move iterator to next task
    void operator ++(void);
    //@}

    /// \name %Task access
    //@{
    /// Return current task position
    int task(void) const;
    //@}
  };

}}}

#include <gecode/scheduling/unary/task-iterator.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Task trees
  template<class TaskView>
  class TaskTree {
  protected:
    /// The tasks from which the tree is computed
    const TaskViewArray<TaskView>& tasks;
    /// Map task number to leave node number in right order
    int* leave;
    /// Return number of inner nodes
    int inner(void) const;
    /// Return number of nodes for balanced binary tree
    int nodes(void) const;
    /// Whether \a i is index of root
    static bool root(int i);
    /// Return index of left child of \a i
    static int left(int i);
    /// Return index of right child of \a i
    static int right(int i);
    /// Return index of parent of \a i
    static int parent(int i);
  public:
    /// Initialize tree for tasks \a t
    TaskTree(Region& r, const TaskViewArray<TaskView>& t);
  };

}}}

#include <gecode/scheduling/unary/task-tree.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Omega trees for computing ect of task sets
  template<class TaskView>
  class OmegaTree : public TaskTree<TaskView> {
  protected:
    using TaskTree<TaskView>::tasks;
    using TaskTree<TaskView>::leave;
    using TaskTree<TaskView>::root;
    using TaskTree<TaskView>::parent;
    using TaskTree<TaskView>::left;
    using TaskTree<TaskView>::right;
    using TaskTree<TaskView>::nodes;
    using TaskTree<TaskView>::inner;
    /// Node for an omega tree
    class Node {
    public:
      /// Processing time for subtree
      int p;
      /// Earliest completion time for subtree
      int ect;
      /// Default constructor for uninitialized node
      Node(void);
    };
    /// Task nodes
    Node* node;
    /// Update tree from \a i up to root
    void update(int i);
  public:
    /// Initialize tree for tasks \a t
    OmegaTree(Region& r, const TaskViewArray<TaskView>& t);
    /// Insert task with index \a i
    void insert(int i);
    /// Remove task with index \a i
    void remove(int i);
    /// Return earliest completion time of all tasks
    int ect(void) const;
    /// Return earliest completion time of all tasks but \a i
    int ect(int i) const;
  };

}}}

#include <gecode/scheduling/unary/omega-tree.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Omega-lambda trees for computing ect of task sets
  template<class TaskView>
  class OmegaLambdaTree : public TaskTree<TaskView> {
  protected:
    using TaskTree<TaskView>::tasks;
    using TaskTree<TaskView>::leave;
    using TaskTree<TaskView>::root;
    using TaskTree<TaskView>::parent;
    using TaskTree<TaskView>::left;
    using TaskTree<TaskView>::right;
    using TaskTree<TaskView>::nodes;
    using TaskTree<TaskView>::inner;
    /// Undefined node
    static const int undef = -1;
    /// Node for an omega lambda tree
    class Node {
    public:
      /// Processing times for subtree
      int p, lp;
      /// Earliest completion times for subtree
      int ect, lect;
      /// Node which is responsible for lect
      int r;
      /// Default constructor for uninitialized node
      Node(void);
    };
    /// Task nodes
    Node* node;
    /// Update tree from \a i up to root
    void update(int i);
  public:
    /// Initialize tree for tasks \a t with all tasks included, if \a inc is true
    OmegaLambdaTree(Region& r, const TaskViewArray<TaskView>& t, 
                    bool inc=true);
    /// Shift task with index \a i from omega to lambda
    void shift(int i);
    /// Insert task with index \a i to omega
    void oinsert(int i);
    /// Insert task with index \a i to lambda
    void linsert(int i);
    /// Remove task with index \a i from lambda
    void lremove(int i);
    /// Whether has responsible task
    bool lempty(void) const;
    /// Return responsible task
    int responsible(void) const;
    /// Return earliest completion time of all tasks
    int ect(void) const;
    /// Return earliest completion time of all tasks excluding lambda tasks
    int lect(void) const;
  };

}}}

#include <gecode/scheduling/unary/omega-lambda-tree.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /**
   * \brief Propagator for unary resource
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class Task>
  class TaskPropagator : public Propagator {
  protected:
    /// Tasks
    TaskArray<Task> t;
    /// Constructor for creation
    TaskPropagator(Space& home, TaskArray<Task>& t);
    /// Constructor for cloning \a p
    TaskPropagator(Space& home, bool shared, TaskPropagator<Task>& p);
  public:
    /// Cost function (defined as high linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Propagator for unary resource
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class Task>
  class TaskOnePropagator : public TaskPropagator<Task> {
  protected:
    using TaskPropagator<Task>::t;
    /// Single additional task
    Task u;
    /// Constructor for creation
    TaskOnePropagator(Space& home, TaskArray<Task>& t, Task& u);
    /// Constructor for cloning \a p
    TaskOnePropagator(Space& home, bool shared, TaskOnePropagator<Task>& p);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/scheduling/unary/task-propagator.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Purge optional tasks that are excluded and possibly rewrite propagator
  ExecStatus purge(Space& home, Propagator& p, TaskArray<OptFixTask>& t);

  /// Check tasks \a t for overload
  ExecStatus overloaded(Space& home, TaskArray<ManFixTask>& t);
  /// Check tasks \a t for overload
  ExecStatus overloaded(Space& home, Propagator& p, TaskArray<OptFixTask>& t);

  /// Check tasks \a t for subsumption
  template<class Task>
  ExecStatus subsumed(Space& home, Propagator& p, TaskArray<Task>& t);

  /// Propagate detectable precedences
  template<class Task>
  ExecStatus detectable(Space& home, TaskArray<Task>& t);

  /// Propagate not-first and not-last
  template<class Task>
  ExecStatus notfirstnotlast(Space& home, TaskArray<Task>& t);

  /// Propagate by edge finding
  template<class Task>
  ExecStatus edgefinding(Space& home, TaskArray<Task>& t);


  /**
   * \brief Scheduling propagator for unary resource with mandatory tasks
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class ManTask>
  class Mandatory : public TaskPropagator<ManTask> {
  protected:
    using TaskPropagator<ManTask>::t;
    /// Constructor for creation
    Mandatory(Space& home, TaskArray<ManTask>& t);
    /// Constructor for cloning \a p
    Mandatory(Space& home, bool shared, Mandatory& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on unary resource
    static ExecStatus post(Space& home, TaskArray<ManTask>& t);
  };

  /**
   * \brief Scheduling propagator for unary resource with optional tasks
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class OptTask>
  class Optional : public TaskPropagator<OptTask> {
  protected:
    using TaskPropagator<OptTask>::t;
    /// Constructor for creation
    Optional(Space& home, TaskArray<OptTask>& t);
    /// Constructor for cloning \a p
    Optional(Space& home, bool shared, Optional& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on unary resource
    static ExecStatus post(Space& home, TaskArray<OptTask>& t);
  };

}}}

#include <gecode/scheduling/unary/overload.hpp>
#include <gecode/scheduling/unary/subsumption.hpp>
#include <gecode/scheduling/unary/detectable.hpp>
#include <gecode/scheduling/unary/not-first-not-last.hpp>
#include <gecode/scheduling/unary/edge-finding.hpp>

#include <gecode/scheduling/unary/mandatory.hpp>
#include <gecode/scheduling/unary/purge.hpp>
#include <gecode/scheduling/unary/optional.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Post that mandatory task \a t1 must be before mandatory task \a t2
  template<class ManTask>
  ExecStatus manbefore(Space& home, ManTask& t1, ManTask& t2);

}}}

#include <gecode/scheduling/unary/man-before.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

}}}

#endif

// STATISTICS: scheduling-prop
