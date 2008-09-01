/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#ifndef __GECODE_SERIALIZATION_HH__
#define __GECODE_SERIALIZATION_HH__

#include "gecode/kernel.hh"

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_SERIALIZATION
#define GECODE_SERIALIZATION_EXPORT __declspec( dllexport )
#else
#define GECODE_SERIALIZATION_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define GECODE_SERIALIZATION_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_SERIALIZATION_EXPORT

#endif
#endif

// Configure auto-linking
#ifndef GECODE_BUILD_SERIALIZATION
#define GECODE_LIBRARY_NAME "Serialization"
#include "gecode/support/auto-link.hpp"
#endif

#include <iostream>
#include <vector>

namespace Gecode {

  /**
   * \brief Produce FlatZinc-like representation of \a home
   * \ingroup TaskSerialization
   *
   * A FlatZinc-like representation of the space \a home is written
   * to \a os, respecting the variable names established by \a m.
   *
   * The generated code is not completely FlatZinc-compatible, as
   * propagator names are represented as C++-style template names.
   *
   */
  GECODE_SERIALIZATION_EXPORT
  void emitFlatzinc(Space& home, std::ostream& os);

  /**
   * \brief Produce JavaScript representation of \a home
   * \ingroup TaskSerialization
   *
   * A JavaScript representation of the space \a home is written
   * to \a os, respecting the variable names established by \a m.
   *
   */
  GECODE_SERIALIZATION_EXPORT
  void emitJavaScript(Space& home, std::ostream& os);

#ifdef GECODE_HAS_QT
  /**
    * \brief Parse JavaScript \a model and post constraints in \a home
    * \ingroup TaskSerialization
    */
  GECODE_SERIALIZATION_EXPORT
  void fromJavaScript(Space& home, const std::string& model);
#endif

namespace Serialization {
  /**
   * \namespace Gecode::Serialization
   * \brief Serialization and deserialization support
   */
  
  /// Initialize the registry of post functions
  GECODE_SERIALIZATION_EXPORT 
  void initRegistry(void);
}
}

#ifdef GECODE_HAS_BOOST

namespace Gecode {
  /**
   * \brief Serialize \a actors to \a os
   * \ingroup TaskSerialization
   */ 
  GECODE_SERIALIZATION_EXPORT
  void boostTextSerialization(std::ostream& os,
    const std::vector<Reflection::VarSpec>& vars,
    const std::vector<Reflection::ActorSpec>& actors);
  /**
   * \brief Deserialize \a is into \a actors
   * \ingroup TaskSerialization
   */
  GECODE_SERIALIZATION_EXPORT
  void boostTextSerialization(std::istream& is,
    std::vector<Reflection::VarSpec>& vars,
    std::vector<Reflection::ActorSpec>& actors);
}

#include "gecode/serialization/boost.hpp"

#endif


#endif

// STATISTICS: serialization-any
