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

#ifndef __GECODE_SCHEDULING_CUMULATIVE_HH__
#define __GECODE_SCHEDULING_CUMULATIVE_HH__

#include <gecode/scheduling/task.hh>
#include <gecode/scheduling/unary.hh>

/**
 * \namespace Gecode::Scheduling::Cumulative
 *
 * The edge-finding and overload-checking algorithms and data structures
 * follow (mostly):
 *   Petr Vilím, Max Energy Filtering Algorithm for Discrete
 *   Cumulative Resources, CP-AI-OR, 2009.
 *   Petr Vilím, Edge Finding Filtering Algorithm for Discrete
 *   Cumulative Resources in O(kn log n), CP, 2009.
 *
 * \brief Scheduling for cumulative resources
 */

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Cumulative (mandatory) task with fixed processing time
  class ManFixTask : public Unary::ManFixTask {
  protected:
    /// Required capacity
    int _c;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ManFixTask(void);
    /// Initialize with start time \a s, processing time \a p, and required capacity \a c
    ManFixTask(IntVar s, int p, int c);
    /// Initialize with start time \a s, processing time \a p, and required capacity \a c
    void init(IntVar s, int p, int c);
    //@}

    /// \name Value access
    //@{
    /// Return required capacity
    int c(void) const;
    /// Return required energy
    double e(void) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this task to be a clone of task \a t
    void update(Space& home, bool share, ManFixTask& t);
    //@}

  };

  /**
   * \brief Print task in format est:[p,c]:lct
   * \relates ManFixTask
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t);


  /// Cumulative optional task with fixed processing time
  class OptFixTask : public ManToOptTask<ManFixTask> {
  protected:
    using ManToOptTask<ManFixTask>::_m;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    OptFixTask(void);
    /// Initialize with start time \a s, processing time \a p, required capacity \a c, and mandatory flag \a m
    OptFixTask(IntVar s, int p, int c, BoolVar m);
    /// Initialize with start time \a s, processing time \a p, required capacity \a c, and mandatory flag \a m
    void init(IntVar s, int p, int c, BoolVar m);
    //@}
  };

  /**
   * \brief Print optional task in format est:[p,c]:lct:m
   * \relates OptFixTask
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFixTask& t);

}}}

#include <gecode/scheduling/cumulative/task.hpp>

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Forward mandatory fixed task view
  typedef ManFixTask ManFixTaskFwd;

  /// Backward (dual) mandatory fixed task view
  typedef FwdToBwd<ManFixTaskFwd> ManFixTaskBwd;

  /// Forward optional fixed task view
  typedef OptFixTask OptFixTaskFwd;

  /// Backward (dual) optional fixed task view
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

}}}

#include <gecode/scheduling/cumulative/task-view.hpp>

namespace Gecode { namespace Scheduling {

  /// Task view traits for forward task views
  template<>
  class TaskViewTraits<Cumulative::ManFixTaskFwd> {
  public:
    /// The task type
    typedef Cumulative::ManFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<Cumulative::ManFixTaskBwd> {
  public:
    /// The task type
    typedef Cumulative::ManFixTask Task;
  };

  /// Task view traits for forward optional task views
  template<>
  class TaskViewTraits<Cumulative::OptFixTaskFwd> {
  public:
    /// The task type
    typedef Cumulative::OptFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<Cumulative::OptFixTaskBwd> {
  public:
    /// The task type
    typedef Cumulative::OptFixTask Task;
  };


  /// Task traits for mandatory fixed tasks
  template<>
  class TaskTraits<Cumulative::ManFixTask> {
  public:
    /// The forward task view type
    typedef Cumulative::ManFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef Cumulative::ManFixTaskBwd TaskViewBwd;
  };

  /// Task traits for optional fixed tasks
  template<>
  class TaskTraits<Cumulative::OptFixTask> {
  public:
    /// The forward task view type
    typedef Cumulative::OptFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef Cumulative::OptFixTaskBwd TaskViewBwd;
    /// The corresponding mandatory task
    typedef Cumulative::ManFixTask ManTask;
  };

}}

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Node for an omega tree
  class OmegaNode {
  public:
    /// Energy for subtree
    double e;
    /// Energy envelope for subtree
    double env;
    /// Initialize node from left child \a l and right child \a r
    void init(const OmegaNode& l, const OmegaNode& r);
    /// Update node from left child \a l and right child \a r
    void update(const OmegaNode& l, const OmegaNode& r);
  };

  /// Omega trees for computing ect of task sets
  template<class TaskView>
  class OmegaTree : public TaskTree<TaskView,OmegaNode> {
  protected:
    using TaskTree<TaskView,OmegaNode>::tasks;
    using TaskTree<TaskView,OmegaNode>::leaf;
    using TaskTree<TaskView,OmegaNode>::root;
    using TaskTree<TaskView,OmegaNode>::init;
    using TaskTree<TaskView,OmegaNode>::update;
    /// Capacity
    int c;
  public:
    /// Initialize tree for tasks \a t and capacity \a c
    OmegaTree(Region& r, int c, const TaskViewArray<TaskView>& t);
    /// Insert task with index \a i
    void insert(int i);
    /// Remove task with index \a i
    void remove(int i);
    /// Return energy envelope of all tasks
    double env(void) const;
  };

  /// Node for an extended omega tree
  class ExtOmegaNode : public OmegaNode {
  public:
    /// Energy envelope for subtree
    double cenv;
    /// Initialize node from left child \a l and right child \a r
    void init(const ExtOmegaNode& l, const ExtOmegaNode& r);
    /// Update node from left child \a l and right child \a r
    void update(const ExtOmegaNode& l, const ExtOmegaNode& r);
  };

  /// Omega trees for computing ect of task sets
  template<class TaskView>
  class ExtOmegaTree : public TaskTree<TaskView,ExtOmegaNode> {
  protected:
    using TaskTree<TaskView,ExtOmegaNode>::tasks;
    using TaskTree<TaskView,ExtOmegaNode>::leaf;
    using TaskTree<TaskView,ExtOmegaNode>::root;
    using TaskTree<TaskView,ExtOmegaNode>::init;
    using TaskTree<TaskView,ExtOmegaNode>::update;
    using TaskTree<TaskView,ExtOmegaNode>::node;
    using TaskTree<TaskView,ExtOmegaNode>::n_leaf;
    using TaskTree<TaskView,ExtOmegaNode>::n_left;
    using TaskTree<TaskView,ExtOmegaNode>::left;
    using TaskTree<TaskView,ExtOmegaNode>::n_right;
    using TaskTree<TaskView,ExtOmegaNode>::right;
    using TaskTree<TaskView,ExtOmegaNode>::n_root;
    using TaskTree<TaskView,ExtOmegaNode>::n_parent;
    /// Capacity
    int c, ci;
  public:
    /// Initialize tree for tasks \a t and capacities \a c, \a ci
    ExtOmegaTree(Region& r, int c, const TaskViewArray<TaskView>& t, int ci);
    /// Compute update for task with index \a i
    int diff(int i);
  };


  /// Node for an omega lambda tree
  class OmegaLambdaNode : public OmegaNode {
  public:
    /// Undefined task
    static const int undef = -1;
    /// Energy for subtree
    double le;
    /// Energy envelope for subtree
    double lenv;
    /// Node which is responsible for lect
    int res;
    /// Initialize node from left child \a l and right child \a r
    void init(const OmegaLambdaNode& l, const OmegaLambdaNode& r);
    /// Update node from left child \a l and right child \a r
    void update(const OmegaLambdaNode& l, const OmegaLambdaNode& r);
  };

  /// Omega-lambda trees for computing ect of task sets
  template<class TaskView>
  class OmegaLambdaTree : public TaskTree<TaskView,OmegaLambdaNode> {
  protected:
    using TaskTree<TaskView,OmegaLambdaNode>::tasks;
    using TaskTree<TaskView,OmegaLambdaNode>::leaf;
    using TaskTree<TaskView,OmegaLambdaNode>::root;
    using TaskTree<TaskView,OmegaLambdaNode>::init;
    using TaskTree<TaskView,OmegaLambdaNode>::update;
    /// Capacity
    int c;
  public:
    /// Initialize tree for tasks \a t and capcity \a c with all tasks included in omega
    OmegaLambdaTree(Region& r, int c, const TaskViewArray<TaskView>& t);
    /// Shift task with index \a i from omega to lambda
    void shift(int i);
    /// Remove task with index \a i from lambda
    void lremove(int i);
    /// Whether has responsible task
    bool lempty(void) const;
    /// Return responsible task
    int responsible(void) const;
    /// Return energy envelope of all tasks
    double env(void) const;
    /// Return energy envelope of all tasks excluding lambda tasks
    double lenv(void) const;
  };

}}}

#include <gecode/scheduling/cumulative/tree.hpp>

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Perform basic propagation
  template<class Task>
  ExecStatus basic(Space& home, Propagator& p, int c, TaskArray<Task>& t);

  /// Check mandatory tasks \a t for overload
  template<class ManTask>
  ExecStatus overload(Space& home, int c, TaskArray<ManTask>& t);

  /// Propagate by edge finding
  template<class Task>
  ExecStatus edgefinding(Space& home, int c, TaskArray<Task>& t);

  /**
   * \brief Scheduling propagator for cumulative resource with mandatory tasks
   *
   * Requires \code #include <gecode/scheduling/cumulative.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class ManTask>
  class ManProp : public TaskProp<ManTask> {
  protected:
    using TaskProp<ManTask>::t;
    /// Resource capacity
    int c;
    /// Constructor for creation
    ManProp(Home home, int c, TaskArray<ManTask>& t);
    /// Constructor for cloning \a p
    ManProp(Space& home, bool shared, ManProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on cumulative resource
    static ExecStatus post(Home home, int c, TaskArray<ManTask>& t);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Scheduling propagator for cumulative resource with optional tasks
   *
   * Requires \code #include <gecode/scheduling/cumulative.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class OptTask>
  class OptProp : public TaskProp<OptTask> {
  protected:
    using TaskProp<OptTask>::t;
    /// Resource capacity
    int c;
    /// Constructor for creation
    OptProp(Home home, int c, TaskArray<OptTask>& t);
    /// Constructor for cloning \a p
    OptProp(Space& home, bool shared, OptProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on cumulative resource
    static ExecStatus post(Home home, int c, TaskArray<OptTask>& t);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/scheduling/cumulative/basic.hpp>
#include <gecode/scheduling/cumulative/overload.hpp>
#include <gecode/scheduling/cumulative/edge-finding.hpp>
#include <gecode/scheduling/cumulative/man-prop.hpp>
#include <gecode/scheduling/cumulative/opt-prop.hpp>

#endif

// STATISTICS: scheduling-prop
