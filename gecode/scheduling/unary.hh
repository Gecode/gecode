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

#include <gecode/scheduling/task.hh>

/**
 * \namespace Gecode::Scheduling::Unary
 *
 * The algorithms and data structures follow (mostly):
 *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
 *   Charles University, Prague, Czech Republic, 2007.
 *
 * \brief %Scheduling for unary resources
 */

namespace Gecode { namespace Scheduling { namespace Unary {

  /// %Unary (mandatory) task with fixed processing time
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
    /// Whether task is mandatory
    bool mandatory(void) const;
    /// Whether task is excluded
    bool excluded(void) const;
    /// Whether task can still be optional
    bool optional(void) const;
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
    /// Update such that task cannot run from \a e to \a l
    ModEvent norun(Space& home, int e, int l);
    /// Mark task as mandatory
    ModEvent mandatory(Space& home);
    /// Mark task as excluded
    ModEvent excluded(Space& home);
    //@}

    /// \name Cloning
    //@{
    /// Update this task to be a clone of task \a t
    void update(Space& home, bool share, ManFixTask& t);
    //@}

    /// \name Dependencies
    //@{
    /// Subscribe propagator \a p to task
    void subscribe(Space& home, Propagator& p, PropCond pc=Int::PC_INT_BND);
    /// Cancel subscription of propagator \a p for task
    void cancel(Space& home, Propagator& p, PropCond pc=Int::PC_INT_BND);
    //@}

  };

  /**
   * \brief Print task in format est:p:lct
   * \relates ManFixTask
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t);


  /// %Unary optional task with fixed processing time
  class OptFixTask : public ManToOptTask<ManFixTask> {
  protected:
    using ManToOptTask<ManFixTask>::_m;
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

#include <gecode/scheduling/unary/task-view.hpp>

namespace Gecode { namespace Scheduling {

  /// Task view traits for forward task views
  template<>
  class TaskViewTraits<Unary::ManFixTaskFwd> {
  public:
    /// The task type
    typedef Unary::ManFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<Unary::ManFixTaskBwd> {
  public:
    /// The task type
    typedef Unary::ManFixTask Task;
  };

  /// Task view traits for forward optional task views
  template<>
  class TaskViewTraits<Unary::OptFixTaskFwd> {
  public:
    /// The task type
    typedef Unary::OptFixTask Task;
  };

  /// Task view traits for backward task views
  template<>
  class TaskViewTraits<Unary::OptFixTaskBwd> {
  public:
    /// The task type
    typedef Unary::OptFixTask Task;
  };


  /// Task traits for mandatory fixed tasks
  template<>
  class TaskTraits<Unary::ManFixTask> {
  public:
    /// The forward task view type
    typedef Unary::ManFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef Unary::ManFixTaskBwd TaskViewBwd;
  };

  /// Task traits for optional fixed tasks
  template<>
  class TaskTraits<Unary::OptFixTask> {
  public:
    /// The forward task view type
    typedef Unary::OptFixTaskFwd TaskViewFwd;
    /// The backward task view type
    typedef Unary::OptFixTaskBwd TaskViewBwd;
    /// The corresponding mandatory task
    typedef Unary::ManFixTask ManTask;
  };

}}

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Node for an omega tree
  class OmegaNode {
  public:
    /// Processing time for subtree
    int p;
    /// Earliest completion time for subtree
    int ect;
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

  /// Node for an omega lambda tree
  class OmegaLambdaNode : public OmegaNode {
  public:
    /// Undefined task
    static const int undef = -1;
    /// Processing times for subtree
    int lp;
    /// Earliest completion times for subtree
    int lect;
    /// Node which is responsible for lect
    int resEct;
    /// Node which is responsible for lp
    int resLp;
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

#include <gecode/scheduling/unary/tree.hpp>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Check mandatory tasks \a t for overload
  template<class ManTask>
  ExecStatus overload(Space& home, TaskArray<ManTask>& t);
  /// Check optional tasks \a t for overload
  template<class OptTask>
  ExecStatus overload(Space& home, Propagator& p, TaskArray<OptTask>& t);

  /// Check tasks \a t for subsumption
  template<class Task>
  ExecStatus subsumed(Space& home, Propagator& p, TaskArray<Task>& t);

  /// Propagate detectable precedences
  template<class ManTask>
  ExecStatus detectable(Space& home, TaskArray<ManTask>& t);
  /// Propagate detectable precedences
  template<class OptTask>
  ExecStatus detectable(Space& home, Propagator& p, TaskArray<OptTask>& t);

  /// Propagate not-first and not-last
  template<class ManTask>
  ExecStatus notfirstnotlast(Space& home, TaskArray<ManTask>& t);
  /// Propagate not-first and not-last
  template<class OptTask>
  ExecStatus notfirstnotlast(Space& home, Propagator& p, TaskArray<OptTask>& t);

  /// Propagate by edge finding
  template<class Task>
  ExecStatus edgefinding(Space& home, TaskArray<Task>& t);


  /**
   * \brief %Scheduling propagator for unary resource with mandatory tasks
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class ManTask>
  class ManProp : public TaskProp<ManTask> {
  protected:
    using TaskProp<ManTask>::t;
    /// Constructor for creation
    ManProp(Home home, TaskArray<ManTask>& t);
    /// Constructor for cloning \a p
    ManProp(Space& home, bool shared, ManProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on unary resource
    static ExecStatus post(Home home, TaskArray<ManTask>& t);
  };

  /**
   * \brief %Scheduling propagator for unary resource with optional tasks
   *
   * Requires \code #include <gecode/scheduling/unary.hh> \endcode
   * \ingroup FuncSchedulingProp
   */
  template<class OptTask>
  class OptProp : public TaskProp<OptTask> {
  protected:
    using TaskProp<OptTask>::t;
    /// Constructor for creation
    OptProp(Home home, TaskArray<OptTask>& t);
    /// Constructor for cloning \a p
    OptProp(Space& home, bool shared, OptProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on unary resource
    static ExecStatus post(Home home, TaskArray<OptTask>& t);
  };

}}}

#include <gecode/scheduling/unary/overload.hpp>
#include <gecode/scheduling/unary/subsumption.hpp>
#include <gecode/scheduling/unary/detectable.hpp>
#include <gecode/scheduling/unary/not-first-not-last.hpp>
#include <gecode/scheduling/unary/edge-finding.hpp>

#include <gecode/scheduling/unary/man-prop.hpp>
#include <gecode/scheduling/unary/opt-prop.hpp>

#endif

// STATISTICS: scheduling-prop
