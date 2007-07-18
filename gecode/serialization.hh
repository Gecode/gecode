/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2007-01-17 11:28:00 +0100 (Wed, 17 Jan 2007) $ by $Author: tack $
 *     $Revision: 4060 $
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
#include <map>

namespace Gecode {

  GECODE_SERIALIZATION_EXPORT
  void emitFlatzinc(Space* home, Reflection::VarMap& m, std::ostream& os);

  namespace Serialization {

    /// Registry of constraint posting and variable creation functions
    class Registry {
    public:
      /// The type of constraint posting functions
      typedef void (*poster) (Space*,
                              const std::vector<VarBase*>&,
                              Reflection::ActorSpec&);
      /// The type of variable creation functions
      typedef VarBase* (*varCreator) (Space*,
                                      Reflection::VarSpec&);

      /// Create a new variable in \a home from \a spec and return it
      GECODE_SERIALIZATION_EXPORT
      VarBase* createVar(Space* home, Reflection::VarSpec& spec);

      /// Post constraint in \a home for \a spec using variables \a vars
      GECODE_SERIALIZATION_EXPORT
      void post(Space* home, const std::vector<VarBase*>& vars,
                Reflection::ActorSpec& spec);

      /// Register variable creation function for variable type identifier \a vti
      GECODE_SERIALIZATION_EXPORT
      void add(int vti, varCreator vc);
      /// Register constraint posting function for constraint name \a id
      GECODE_SERIALIZATION_EXPORT
      void add(const char* id, poster p);
    private:
      /// The registry of constraint posting functions
      std::map<std::string, poster> posters;
      /// The registry of variable creation functions
      std::map<int, varCreator> varCreators;
    };
    
    /// The registry object
    extern Registry registry;

    /// Deserialization from VarSpec and ActorSpec
    class Deserializer {
    private:
      /// The space in which to create variables and post constraints
      Space* home;
      /// The VarMap that indicates which variables to reuse
      Reflection::VarMap& m;
      /// A vector of newly created variables
      std::vector<VarBase*> v;
      
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
