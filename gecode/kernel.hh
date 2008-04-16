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

#ifndef __GECODE_KERNEL_HH__
#define __GECODE_KERNEL_HH__

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "gecode/support.hh"

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

/**
 * \namespace Gecode
 * \brief Gecode toplevel namespace
 *
 * The Gecode namespace contains nested namespaces for
 * the various submodules (for example Int for the
 * definition of integer propagator classes). Functionality
 * that is used for interfacing (search engines, variables,
 * and so on) or belongs to the %Gecode %Kernel is contained
 * directly in the Gecode namespace.
 *
 */

/*
 * General exceptions and kernel exceptions
 *
 */

#include "gecode/kernel/exception.icc"



/*
 * Basic kernel services
 *
 */

#include "gecode/kernel/memory-manager.icc"


/*
 * Macros for checking failure
 *
 */


#include "gecode/kernel/macros.icc"

/*
 * Gecode kernel
 *
 */

#include "gecode/kernel/core.icc"
#include "gecode/kernel/modevent.icc"

/*
 * Traits linking variables, views, and variable implementations
 *
 */

#include "gecode/kernel/var-traits.icc"

/*
 * Variables and testing for shared variables
 *
 */

#include "gecode/kernel/var.icc"

/*
 * Access to the constraint graph
 *
 */

#include "gecode/kernel/reflection.icc"

/*
 * Views
 *
 */

#include "gecode/kernel/view.icc"

/*
 * Variable and argument arrays
 *
 */

#include "gecode/kernel/array.icc"

/*
 * Access to the constraint graph
 *
 */

#include "gecode/kernel/var-map.icc"

/*
 * Shared arrays
 *
 */

#include "gecode/kernel/shared-array.icc"

/*
 * Common propagator and branching patterns
 *
 */

#include "gecode/kernel/propagator.icc"
#include "gecode/kernel/advisor.icc"
#include "gecode/kernel/branching.icc"

/*
 * Branching for decomposition during search
 *
 */

#include "gecode/kernel/decomposing-branching.icc"

/*
 * Automatically generated variable implementations
 *
 */

#include "gecode/kernel/var-imp.icc"

#endif

// STATISTICS: kernel-other
