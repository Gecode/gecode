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

    class Registry {
    public:
      typedef void (*poster) (Space*,
                              const std::vector<VarBase*>&,
                              Reflection::ActorSpec&);
      typedef VarBase* (*varCreator) (Space*,
                                      Reflection::VarSpec&);

      GECODE_SERIALIZATION_EXPORT
      VarBase* createVar(Space*, Reflection::VarSpec&);

      GECODE_SERIALIZATION_EXPORT
      void post(Space*, const std::vector<VarBase*>&, Reflection::ActorSpec&);

      GECODE_SERIALIZATION_EXPORT
      void add(int vti, varCreator vc);
      GECODE_SERIALIZATION_EXPORT
      void add(const char* id, poster p);
    private:
      std::map<std::string, poster> posters;
      std::map<int, varCreator> varCreators;
    };
    
    extern Registry registry;

    class Deserializer {
    private:
      Space* home;
      Reflection::VarMap& m;
      std::vector<VarBase*> v;
      
    public:
      Deserializer(Space* home0, Reflection::VarMap& m0);
      
      Reflection::VarMap& varMap(void) const;
      
      GECODE_SERIALIZATION_EXPORT
      void var(Reflection::VarSpec& spec);

      GECODE_SERIALIZATION_EXPORT
      void post(Reflection::ActorSpec& spec);
    };
  
  
  }
  
}

#include "gecode/serialization/deserializer.icc"

#endif

// STATISTICS: serialization-any
