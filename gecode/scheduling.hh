/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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
 * \brief %Scheduling constraints and branchers
 *
 */

namespace Gecode {

  /**
   * \defgroup TaskModelScheduling Scheduling constraints and branchers
   * \ingroup TaskModel
   */
  //@{

  /// Type of task for scheduling constraints
  enum TaskType {
    TT_FIXP, //< Task with fixed processing time
    TT_FIXS, //< Task with fixed start time
    TT_FIXE  //< Task with fixed end time
  };
  
  /// Passing task type arguments
  typedef PrimArgArray<TaskType> TaskTypeArgs;
  /// Traits of %TaskTypeArgs
  template<>
  class ArrayTraits<PrimArgArray<TaskType> > {
  public:
    typedef TaskTypeArgs StorageType;
    typedef TaskType     ValueType;
    typedef TaskTypeArgs ArgsType;
  };

  /**
   * \brief Post propagators for the cumulatives constraint
   *
   * This function creates propagators for the cumulatives constraint
   * presented in <em> "A new multi-resource cumulatives constraint
   * with negative heights"</em>, Nicolas Beldiceanu and Mats
   * Carlsson, Principles and Practice of Constraint Programming 2002.
   *
   * The constraint models a set of machines and a set of tasks that
   * should be assigned to the machines. The machines have a positive
   * resource limit and the tasks each have a resource usage that can
   * be either positive, negative, or zero. The constraint is enforced
   * over each point in time for a machine where there is at least one
   * task assigned.
   *
   * The limit for a machine is either the maximum amount available at
   * any given time (\a at_most = true), or else the least amount to
   * be used (\a at_most = false).
   *
   * \param home current space
   * \param machine \f$ machine_i \f$ is the machine assigned to task \f$ i \f$
   * \param start \f$ start_i \f$ is the start date assigned to task \f$ i \f$
   * \param duration \f$ duration_i \f$ is the duration of task \f$ i \f$
   * \param end \f$ end_i \f$ is the end date assigned to task \f$ i \f$
   * \param height \f$ height_i \f$ is the height is the amount of
   *               resources consumed by task \f$ i \f$
   * \param limit \f$ limit_r \f$ is the amount of resource available
   *              for machine \f$ r \f$
   * \param at_most \a at_most tells if the amount of resources used
   *                for a machine should be less than the limit (\a at_most
   *                = true) or greater than the limit (\a at_most = false)
   * \param icl Supports value-consistency only (\a icl = ICL_VAL, default).
   *
   * \exception Int::ArgumentSizeMismatch thrown if the sizes 
   *            of the arguments representing tasks does not match.
   * \exception Int::OutOfLimits thrown if any numerical argument is
   *            larger than Int::Limits::max or less than
   *            Int::Limits::min.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_SCHEDULING_EXPORT void
  cumulatives(Home home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with start times \a s and processing times \a p
   * on a unary resource. The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s 
   *    and \a p are of different size.
   *  - Throws an exception of type Int::ArgumentSame, if \a s contains
   *    the same unassigned variable multiply.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntArgs& p);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. The propagator uses the 
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, or \a m are of different size.
   *  - Throws an exception of type Int::ArgumentSame, if \a s contains
   *    the same unassigned variable multiply.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntArgs& p, 
        const BoolVarArgs& m);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with flexible times \a flex and fixed times \a fix
   * on a unary resource. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then 
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s 
   *    and \a p are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative for a task with type <code>TT_FIXP</code>
   *    or that could generate an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& s, const IntArgs& p);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with flexible times \a flex, fixed times \a fix,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then 
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator uses the 
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative for a task with type <code>TT_FIXP</code>
   *    or that could generate an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& s, const IntArgs& p, const BoolVarArgs& m);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with start times \a s, processing times \a p, and
   * end times \a e
   * on a unary resource. The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   * The processing times are constrained to be non-negative.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a s 
   * and \a p are of different size.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p, 
        const IntVarArgs& e);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * end times \a e,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. The propagator uses the 
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis, 
   *   Charles University, Prague, Czech Republic, 2007.
   * 
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The processing times are constrained to be non-negative.
   *
   * The propagator performs overload checking, detectable precendence
   * propagation, not-first-not-last propagation, and edge finding.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   * \a p, or \a m are of different size.
   */
  GECODE_SCHEDULING_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e, const BoolVarArgs& m);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with flexible times \a flex, fixed times \a fix, and
   * use capacity \a u on a cumulative resource with capacity \a c. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then 
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a t, \a s 
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& flex, const IntArgs& fix, const IntArgs& u);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule tasks with flexible times \a flex, fixed times \a fix,
   * use capacity \a u, and whether a task is mandatory \a m (a task is 
   * mandatory if the Boolean variable is 1) on a cumulative resource with 
   * capacity \a c. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then 
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then 
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a t, \a s 
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with start times \a s, processing times \a p, and
   * use capacity \a u on a cumulative resource with capacity \a c.
   *
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s 
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * used capacity \a u, and whether a task is mandatory \a m (a task is 
   * mandatory if the Boolean variable is 1) on a cumulative resource 
   * with capacity \a c.
   * 
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, \a u, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with start times \a s, processing times \a p,
   * end times \a e, and
   * use capacity \a u on a cumulative resource with capacity \a c.
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s 
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a u or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * end times \a e,
   * used capacity \a u, and whether a task is mandatory \a m (a task is 
   * mandatory if the Boolean variable is 1) on a cumulative resource 
   * with capacity \a c.
   * 
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs time-tabling, overload checking, and 
   * edge-finding. It uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative 
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume 
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative 
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS, 
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, \a u, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a u or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_SCHEDULING_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p, 
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m);
  //@}

}

#endif

// STATISTICS: scheduling-post

