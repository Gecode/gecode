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

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_SERIALIZATION_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_SERIALIZATION_EXPORT

#endif
#endif

#include "gecode/kernel.hh"
#include <iostream>
#include <vector>

namespace Gecode {

  GECODE_SERIALIZATION_EXPORT
  void emitFlatzinc(Space* home, Reflection::VarMap& m, std::ostream& os);

  /// Serialization support
  namespace Serialization {

    /// Deserialization from VarSpec and ActorSpec
    class Deserializer {
    private:
      /// The space in which to create variables and post constraints
      Space* home;
      /// The VarMap that indicates which variables to reuse
      Reflection::VarMap& m;
      
    public:
      /// Constructor
      Deserializer(Space* home0, Reflection::VarMap& m0);
      
      /// Return the VarMap
      Reflection::VarMap& varMap(void) const;
      
      /// Create a new variable from \a spec
      GECODE_SERIALIZATION_EXPORT
      void var(Reflection::VarSpec& spec);

      /// Post the constraint defined by \a spec
      GECODE_SERIALIZATION_EXPORT
      void post(Reflection::ActorSpec& spec);
    };
  
  
  }
  
}

#ifdef GECODE_HAVE_BOOST_SERIALIZATION

namespace Gecode {
  /// Serialize \a actors to \a os
  void boostTextSerialization(std::ostream& os,
    const std::vector<Reflection::ActorSpec*>& actors);
  /// Deserialize \a is into \a actors
  void boostTextSerialization(std::istream& is,
    std::vector<Reflection::ActorSpec*>& actors);
}

#include "gecode/serialization/boost.icc"

#endif

#include "gecode/serialization/deserializer.icc"

#endif

// STATISTICS: serialization-any
