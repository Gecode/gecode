/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#ifndef __GECODE_SCHEDULING_HH__
#define __GECODE_SCHEDULING_HH__

#include <gecode/int.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_SCHEDULING
#define GECODE_SCHEDULING_EXPORT __declspec( dllexport )
#else
#define GECODE_SCHEDULING_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_SCHEDULING_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_SCHEDULING_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_SCHEDULING
#define GECODE_LIBRARY_NAME "Scheduling"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode::Scheduling
 * \brief Scheduling support
 *
 * The Gecode::Scheduling namespace contains abstractions, propagators and
 * branchings for modelling and solving scheduling problems.
 *
 */

#include <gecode/scheduling/exception.hpp>

#include <gecode/scheduling/imp/task.hpp>
#include <gecode/scheduling/imp/resource.hpp>
#include <gecode/scheduling/imp/schedule.hpp>

namespace Gecode {

  /**
   * \brief Schedule
   *
   * A schedule collects all resources used by tasks comprising the schedule.
   * \ingroup TaskModelSchedule
   */
  class Schedule {
  private:
    /// Implementation of a schedule
    Scheduling::ScheduleImp* si;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (schedule remains unininitialized)
    Schedule(void);
    /// Copy constructor
    Schedule(const Schedule& s);
    /// Initialize schedule without any resource
    Schedule(Space& home);
    /// Initialize resource without any resource
    void init(Space& home);
    //@}

    /// \name Value Access
    //@{
    /// Return number of resources for this schedule
    int resources(void) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this schedule to be a clone of schedule \a s
    void update(Space& home, bool share, Schedule& s);
    //@}
  };

  /**
   * \brief Resource
   *
   * A resource collects all tasks to be executed on that resource.
   * \ingroup TaskModelSchedule
   */
  class Resource {
  private:
    /// Implementation of a resource
    Scheduling::ResourceImp* ri;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (resource remains unininitialized)
    Resource(void);
    /// Copy constructor
    Resource(const Resource& r);
    /// Initialize resource for schedule \a s with capacity \a cap
    Resource(Space& home, Schedule& s, int cap=1);
    /// Initialize resource for schedule \a s with capacity \a cap
    void init(Space& home, Schedule& s, int cap=1);
    //@}

    /// \name Value Access
    //@{
    /// Return number of tasks on this resource
    int tasks(void) const;
    /// Return capacity of resource
    int cap(void) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this resource to be a clone of resource \a r
    void update(Space& home, bool share, Resource& t);
    //@}
  };

  /**
   * \brief Task
   *
   * A task is described by its start time, duration and resource usage.
   * \ingroup TaskModelSchedule
   */
  class Task {
  private:
    /// Implementation of a task
    Scheduling::TaskImp* ti;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (task remains unininitialized)
    Task(void);
    /// Copy constructor
    Task(const Task& t);
    /**
     * \brief Initialize task on resource
     *
     * Initialize task on resource \a with start time between \a est
     * and \a lst, duration \a dur and resource usage \a use.
     */
    Task(Space& home, Resource& r, int est, int lst, int dur, int use=1);
    /**
     * \brief Initialize task on resource
     *
     * Initialize task on resource \a with start time \a st,
     * duration \a dur and resource usage \a use.
     */
    Task(Space& home, Resource& r, IntVar st, int dur, int use=1);
    /**
     * \brief Initialize task on resource
     *
     * Initialize task on resource \a with start time between \a est
     * and \a lst, duration \a dur and resource usage \a use.
     */
    void init(Space& home, Resource& r, int est, int lst, int dur, int use=1);
    /**
     * \brief Initialize task on resource
     *
     * Initialize task on resource \a with start time \a st,
     * duration \a dur and resource usage \a use.
     */
    void init(Space& home, Resource& r, IntVar st, int dur, int use=1);
    //@}


    /// \name Value Access
    //@{
    /// Return integer variable for start time
    IntVar start(void) const;
    /// Return duration
    int dur(void) const;
    /// Return resource usage
    int use(void) const;

    /// Return earliest start timr
    int est(void) const;
    /// Return latest start time
    int lst(void) const;
    /// Return earliest completion time
    int ect(void) const;
    /// Return latest completion time
    int lct(void) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this task to be a clone of task \a t
    void update(Space& home, bool share, Task& t);
    //@}
  };

  /**
   * \brief Iterate over all resources of a schedule
   *
   * \ingroup TaskModelSchedule
   */
  class Resources {
  private:
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Resources(void);
    /// Initialize with resources from schedule \a s
    Resources(const Schedule& s);
    /// Initialize with resources from schedule \a s
    void init(const Schedule& s);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a resource
    bool operator ()(void) const;
    /// Move iterator to next resource (if possible)
    void operator ++(void);
    //@}

    /// \name Resource access
    //@{
    /// Return resource
    Resource resource(void) const;
    //@}
  };

  /**
   * \brief Iterate over all tasks of a resource
   *
   * \ingroup TaskModelSchedule
   */
  class Tasks {
  private:
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Tasks(void);
    /// Initialize with tasks from resource \a r
    Tasks(const Resource& r);
    /// Initialize with tasks from resource \a r
    void init(const Resource& r);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a task
    bool operator ()(void) const;
    /// Move iterator to next task (if possible)
    void operator ++(void);
    //@}

    /// \name Task access
    //@{
    /// Return task
    Task task(void) const;
    //@}
  };

  /**
   * \brief Print task \a t
   * \relates Gecode::Task
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Task& t);
  /**
   * \brief Print resource \a r
   * \relates Gecode::Resource
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Resource& r);
  /**
   * \brief Print schedule \a s
   * \relates Gecode::Resource
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Schedule& s);


  /**
   * \defgroup TaskModelScheduleConstraints Scheduling constraints
   * \ingroup TaskModelSchedule
   *
   */
  //@{
  /// Constrains tasks on the individual resources of schedule \a s to be scheduled
  GECODE_SCHEDULING_EXPORT void
  schedule(Space& home, const Schedule& s);
  //@}

  /**
   * \defgroup TaskModelScheduleBranching Scheduling branchings
   * \ingroup TaskModelSchedule
   *
   */
  //@{
  /// Order tasks on the individual resources of schedule \a s
  GECODE_SCHEDULING_EXPORT void
  branch(Space& home, const Schedule& s);
  /// Assign start times of already ordered tasks on the individual resources of schedule \a s
  GECODE_SCHEDULING_EXPORT void
  assign(Space& home, const Schedule& s);
  //@}

  /**
   * \defgroup TaskModelScheduleDirect Low-level constraint access
   * \ingroup TaskModelSchedule
   *
   */
  //@{
  /**
   * \brief Serialize all unary tasks described by start times \a st and duration \a dur
   * \li Throws an exception of type Scheduling::ArgumentSizeMismatch, if
   *     \a st and \a dur are of different size.
   * \li Throws an exception of type Scheduling::ArgumentSame, if \a st
   *     contains the same unassigned variable multiply.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Space& home, const IntVarArgs& st, const IntArgs& dur);
  //@}

}

#endif

// IFDEF: GECODE_HAS_SCHEDULING
// STATISTICS: scheduling-post

