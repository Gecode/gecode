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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_KERNEL_HH__
#define __GECODE_KERNEL_HH__

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "gecode/config.icc"

/*
 * Support for DLLs under Windows
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

#ifdef GCC_HASCLASSVISIBILITY

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

#include "gecode/kernel/memory.icc"
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
 * Variable views and testing for shared variables
 *
 */

#include "gecode/kernel/view.icc"



/*
 * Variable and argument arrays
 *
 */

#include "gecode/kernel/array.icc"




/*
 * Common propagator and branching patterns
 *
 */

#include "gecode/kernel/propagator.icc"
#include "gecode/kernel/branching.icc"

#endif


// STATISTICS: kernel-other
