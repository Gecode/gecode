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
 * The algorithms and data structures follow (mostly):
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
    int e(void) const;
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
    int e;
    /// Eneregy envelope for subtree
    int env;
    /// Initialize node from left child \a l and right child \a r
    void init(const OmegaNode& l, const OmegaNode& r, int c);
    /// Update node from left child \a l and right child \a r
    void update(const OmegaNode& l, const OmegaNode& r, int c);
  };

  /// Omega trees for computing ect of task sets
  template<class TaskView>
  class OmegaTree : public TaskTree<TaskView,OmegaNode,int> {
  protected:
    using TaskTree<TaskView,OmegaNode,int>::tasks;
    using TaskTree<TaskView,OmegaNode,int>::leaf;
    using TaskTree<TaskView,OmegaNode,int>::root;
    using TaskTree<TaskView,OmegaNode,int>::init;
    using TaskTree<TaskView,OmegaNode,int>::update;
    /// Capacity
    int c;
  public:
    /// Initialize tree for tasks \a t and capcity \a c
    OmegaTree(Region& r, int c, const TaskViewArray<TaskView>& t);
    /// Insert task with index \a i
    void insert(int i);
    /// Remove task with index \a i
    void remove(int i);
    /// Return energy envelope of all tasks
    int env(void) const;
  };

}}}

#include <gecode/scheduling/cumulative/omega-tree.hpp>

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Check mandatory tasks \a t for overload
  template<class ManTask>
  ExecStatus overload(Space& home, int c, TaskArray<ManTask>& t);

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
    ManProp(Space& home, int c, TaskArray<ManTask>& t);
    /// Constructor for cloning \a p
    ManProp(Space& home, bool shared, ManProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on cumulative resource
    static ExecStatus post(Space& home, int c, TaskArray<ManTask>& t);
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
    OptProp(Space& home, int c, TaskArray<OptTask>& t);
    /// Constructor for cloning \a p
    OptProp(Space& home, bool shared, OptProp& p);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator that schedules tasks on cumulative resource
    static ExecStatus post(Space& home, int c, TaskArray<OptTask>& t);
  };

}}}

#include <gecode/scheduling/cumulative/overload.hpp>
#include <gecode/scheduling/cumulative/man-prop.hpp>
#include <gecode/scheduling/cumulative/opt-prop.hpp>

#endif

// STATISTICS: scheduling-prop
