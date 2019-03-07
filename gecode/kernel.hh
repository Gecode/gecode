/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

#ifndef __GECODE_KERNEL_HH__
#define __GECODE_KERNEL_HH__

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <cfloat>

#include <functional>

#include <gecode/support.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_KERNEL
#define GECODE_KERNEL_EXPORT __declspec( dllexport )
#else
#define GECODE_KERNEL_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_KERNEL_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_KERNEL_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_KERNEL
#define GECODE_LIBRARY_NAME "Kernel"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode
 * \brief %Gecode toplevel namespace
 *
 * The Gecode namespace contains nested namespaces for
 * the various submodules (for example Int for the
 * definition of integer propagator classes). Functionality
 * that is used for interfacing (search engines, variables,
 * and so on) or belongs to the %Gecode %Kernel is contained
 * directly in the Gecode namespace.
 *
 */

namespace Gecode {

  /// Kernel configuration parameters
  namespace Kernel { namespace Config {
    /// Rescale factor for action and afc values
    const double rescale = 1e-50;
    /// Rescale action and afc values when larger than this
    const double rescale_limit = DBL_MAX * rescale;

    /// Initial value for alpha in CHB
    const double chb_alpha_init = 0.4;
    /// Limit for decreasing alpha in CHB
    const double chb_alpha_limit = 0.06;
    /// Alpha decrement in CHB
    const double chb_alpha_decrement = 1e-6;
    /// Initial value for Q-score in CHB
    const double chb_qscore_init = 0.05;
  }}

}

/*
 * General exceptions and kernel exceptions
 *
 */

#include <gecode/kernel/exception.hpp>



/*
 * Basic kernel services and memory management
 *
 */

#include <gecode/kernel/shared-object.hpp>
#include <gecode/kernel/memory/config.hpp>
#include <gecode/kernel/memory/manager.hpp>
#include <gecode/kernel/memory/region.hpp>

/*
 * Macros for checking failure
 *
 */

#include <gecode/kernel/macros.hpp>


/*
 * Gecode kernel
 *
 */

#include <gecode/kernel/archive.hpp>
#include <gecode/kernel/gpi.hpp>
#include <gecode/kernel/shared-space-data.hpp>
#include <gecode/kernel/core.hpp>
#include <gecode/kernel/modevent.hpp>
#include <gecode/kernel/range-list.hpp>


/*
 * Variables and testing for shared variables
 *
 */

#include <gecode/kernel/var.hpp>


/*
 * Views
 *
 */

#include <gecode/kernel/view.hpp>


/*
 * Arrays and other data
 *
 */

#include <gecode/kernel/data/array.hpp>
#include <gecode/kernel/data/shared-array.hpp>
#include <gecode/kernel/data/shared-data.hpp>
#include <gecode/kernel/data/rnd.hpp>


/*
 * Common propagator patterns
 *
 */

#include <gecode/kernel/propagator/pattern.hpp>
#include <gecode/kernel/propagator/subscribed.hpp>
#include <gecode/kernel/propagator/advisor.hpp>
#include <gecode/kernel/propagator/wait.hpp>


/*
 * Abstractions for branching
 *
 */

namespace Gecode {

  /**
   * \defgroup TaskModelBranch Generic branching support
   *
   * Support for randomization and tie-breaking that are independent
   * of a particular variable domain.
   *
   * \ingroup TaskModel
   */

  /**
   * \defgroup TaskModelBranchExec Branch with a function
   *
   * This does not really branch (it just offers a single alternative) but
   * executes a single function during branching. A typical
   * application is to post more constraints after another brancher
   * has finished.
   *
   * \ingroup TaskModelBranch
   */
  //@{
  /// Call the function \a f (with the current space as argument) for branching
  GECODE_KERNEL_EXPORT void
  branch(Home home, std::function<void(Space& home)> f);
  //@}

}

#include <gecode/kernel/branch/traits.hpp>
#include <gecode/kernel/branch/action.hpp>
#include <gecode/kernel/branch/afc.hpp>
#include <gecode/kernel/branch/chb.hpp>
#include <gecode/kernel/branch/var.hpp>
#include <gecode/kernel/branch/val.hpp>
#include <gecode/kernel/branch/tiebreak.hpp>
#include <gecode/kernel/branch/merit.hpp>
#include <gecode/kernel/branch/filter.hpp>
#include <gecode/kernel/branch/view-sel.hpp>
#include <gecode/kernel/branch/print.hpp>
#include <gecode/kernel/branch/view.hpp>
#include <gecode/kernel/branch/val-sel.hpp>
#include <gecode/kernel/branch/val-commit.hpp>
#include <gecode/kernel/branch/val-sel-commit.hpp>
#include <gecode/kernel/branch/view-val.hpp>


/*
 * Automatically generated variable implementations
 *
 */

#include <gecode/kernel/var-imp.hpp>


/*
 * Trace support
 *
 */

#include <gecode/kernel/trace/traits.hpp>
#include <gecode/kernel/trace/filter.hpp>
#include <gecode/kernel/trace/tracer.hpp>
#include <gecode/kernel/trace/recorder.hpp>
#include <gecode/kernel/trace/print.hpp>

namespace Gecode {

  /**
   * \brief Create tracer
   * \ingroup TaskTrace
   */
  GECODE_KERNEL_EXPORT void
  trace(Home home, TraceFilter tf,
        int te = (TE_PROPAGATE | TE_COMMIT | TE_POST),
        Tracer& t = StdTracer::def);
  /**
   * \brief Create tracer
   * \ingroup TaskTrace
   */
  void
  trace(Home home,
        int te = (TE_PROPAGATE | TE_COMMIT | TE_POST),
        Tracer& t = StdTracer::def);

}

#include <gecode/kernel/trace/general.hpp>

/*
 * Allocator support
 *
 */

#include <gecode/kernel/memory/allocators.hpp>


#endif

// STATISTICS: kernel-other
