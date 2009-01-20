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

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#pragma GCC visibility push(default)
#endif

#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/base_object.hpp>

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#pragma GCC visibility pop
#endif

#include <gecode/support.hh>
#include <sstream>

namespace boost { namespace serialization {

  /// The different types of arguments
  enum argtype {
    BOOST_INT_ARG,           ///< Integer argument
    BOOST_VAR_ARG,           ///< Variable argument
    BOOST_ARRAY_ARG,         ///< Array argument
    BOOST_INT_ARRAY_ARG,     ///< Integer array argument
    BOOST_STRING_ARG,        ///< String argument
    BOOST_PAIR_ARG,          ///< Pair argument
    BOOST_SHARED_OBJECT_ARG, ///< Shared object argument
    BOOST_SHARED_REF_ARG     ///< Reference to shared object argument
  };

  template<class Archive>
  inline void save_construct_data(
      Archive & ar, const Gecode::Reflection::Arg* t,
      const unsigned int /*file_version*/
  ){
      // save data required to construct instance
      if (t->isInt()) {
        int out = static_cast<int>(BOOST_INT_ARG);
        ar << out;
      } else if (t->isVar()) {
        int out = static_cast<int>(BOOST_VAR_ARG);
        ar << out;
      } else if (t->isArray()) {
        int out = static_cast<int>(BOOST_ARRAY_ARG);
        ar << out;
      } else if (t->isIntArray()) {
        int out = static_cast<int>(BOOST_INT_ARRAY_ARG);
        ar << out;
      } else if (t->isString()) {
        int out = static_cast<int>(BOOST_STRING_ARG);
        ar << out;
      } else if (t->isPair()) {
        int out = static_cast<int>(BOOST_PAIR_ARG);
        ar << out;
      } else if (t->isSharedObject()) {
        int out = static_cast<int>(BOOST_SHARED_OBJECT_ARG);
        ar << out;
      } else if (t->isSharedReference()) {
        int out = static_cast<int>(BOOST_SHARED_REF_ARG);
        ar << out;
      } else {
        GECODE_NEVER;
      }
  }

  template<class Archive>
  inline void load_construct_data(
      Archive & ar, Gecode::Reflection::Arg* t,
      const unsigned int /*file_version*/
  ){
      // retrieve data from archive required to construct new instance
      int m;
      ar >> m;
      // invoke inplace constructor to initialize instance of my_class
      switch (m) {
      case BOOST_INT_ARG:
        t->initInt(0);
        break;
      case BOOST_VAR_ARG:
        t->initVar(0);
        break;
      case BOOST_ARRAY_ARG:
        t->initArray(0);
        break;
      case BOOST_INT_ARRAY_ARG:
        t->initIntArray(0);
        break;
      case BOOST_STRING_ARG:
        t->initString("");
        break;
      case BOOST_PAIR_ARG:
        t->initPair(NULL, NULL);
        break;
      case BOOST_SHARED_OBJECT_ARG:
        t->initSharedObject(NULL);
        break;
      case BOOST_SHARED_REF_ARG:
        t->initSharedReference(0);
        break;
      default:
        GECODE_NEVER;
      }
  }


  template<class Archive>
  inline void load(
    Archive& ar, Gecode::Reflection::Arg& arg,
    unsigned int /*file_version*/
  ){
    using namespace Gecode::Reflection;
    if (arg.isInt()) {
      int m; ar >> m;
      arg.initInt(m);
    } else if (arg.isVar()) {
      int m; ar >> m;
      arg.initVar(m);
    } else if (arg.isArray()) {
      int m; ar >> m;
      arg.initArray(m);
      ArrayArg* array = arg.toArray();
      for (int i=0; i<m; i++) {
        Arg* k;
        ar >> k;
        (*array)[i] = k;
      }
    } else if (arg.isIntArray()) {
      int m; ar >> m;
      arg.initArray(m);
      IntArrayArg* array = arg.toIntArray();
      for (int i=0; i<m; i++) {
        int k;
        ar >> k;
        (*array)[i] = k;
      }
    } else if (arg.isString()) {
      std::string s;
      ar >> s;
      arg.initString(s.c_str());
    } else if (arg.isPair()) {
      Arg* a; ar >> a;
      Arg* b; ar >> b;
      arg.initPair(a,b);
    } else if (arg.isSharedObject()) {
      Arg* a; ar >> a;
      arg.initSharedObject(a);
    } else if (arg.isSharedReference()) {
      int m; ar >> m;
      arg.initSharedReference(m);
    } else {
      GECODE_NEVER;
    }
  }


  template<class Archive>
  void save(
    Archive& ar,
    const Gecode::Reflection::Arg& arg,
    const unsigned int /*file_version*/
  ) {
    using namespace Gecode::Reflection;
    if (arg.isInt()) {
      int m = arg.toInt();
      ar << m;
    } else if (arg.isVar()) {
      int m = arg.toVar();
      ar << m;
    } else if (arg.isArray()) {
      const ArrayArg* array = arg.toArray();
      int s = array->size();
      ar << s;
      for (int i=0; i<s; i++) {
        const Arg* a = (*array)[i];
        ar << *a;
      }
    } else if (arg.isIntArray()) {
      const IntArrayArg* array = arg.toIntArray();
      int s = array->size();
      ar << s;
      for (int i=0; i<s; i++) {
        int a = (*array)[i];
        ar << a;
      }
    } else if (arg.isString()) {
      std::stringstream oss;
      oss << arg.toString();
      std::string s = oss.str();
      ar << s;
    } else if (arg.isPair()) {
      const Arg* a = arg.first();
      ar << *a;
      const Arg* b = arg.second();
      ar << *b;
    } else if (arg.isSharedObject()) {
      const Arg* a = arg.toSharedObject();
      ar << *a;
    } else if (arg.isSharedReference()) {
      int m = arg.toSharedReference();
      ar << m;
    } else {
      GECODE_NEVER;
    }
  }

  template<class Archive>
  void serialize(
    Archive& ar,
    Gecode::Reflection::Arg& arg,
    const unsigned int file_version
  ) {
    split_free(ar, arg, file_version);
  }

  template<class Archive>
  inline void save_construct_data(
      Archive & ar, const Gecode::Reflection::VarSpec* t,
      const unsigned int /*file_version*/
  ){
    // save data required to construct instance
    const Gecode::Reflection::Arg* a = t->dom();
    ar << *a;
    std::stringstream oss;
    oss << t->vti();
    std::string s = oss.str();
    ar << s;
  }

  template<class Archive>
  inline void load_construct_data(
      Archive & ar, Gecode::Reflection::VarSpec* t,
      const unsigned int /*file_version*/
  ){
    // retrieve data from archive required to construct new instance
    Gecode::Reflection::Arg* dom;
    ar >> dom;
    std::string vti;
    ar >> vti;
    ::new(t)
      Gecode::Reflection::VarSpec(Gecode::Support::Symbol(vti.c_str()),
        dom);
  }

  template<class Archive>
  inline void load(
    Archive& ar, Gecode::Reflection::VarSpec& var,
    unsigned int /*file_version*/
  ){
    std::string name;
    ar >> name;
    var.name(Gecode::Support::Symbol(name.c_str()));
  }

  template<class Archive>
  void save(
    Archive& ar,
    const Gecode::Reflection::VarSpec& var,
    const unsigned int /*file_version*/
  ) {
    std::stringstream oss;
    oss << var.name();
    std::string s = oss.str();
    ar << s;
  }

  template<class Archive>
  void serialize(
    Archive& ar,
    Gecode::Reflection::VarSpec& var,
    const unsigned int file_version
  ) {
    split_free(ar, var, file_version);
  }


  /////////////////////////////////////////////////////////////////////////
  // Gecode::Reflection::ActorSpec

  template<class Archive>
  inline void save_construct_data(
      Archive & ar,
      const Gecode::Reflection::ActorSpec* t,
      const unsigned int /*file_version*/
  ){
    // save data required to construct instance
    std::stringstream oss;
    oss << t->ati();
    std::string n = oss.str();
    ar << n;
  }
  template<class Archive>
  inline void load_construct_data(
      Archive & ar, Gecode::Reflection::ActorSpec* t,
      const unsigned int /*file_version*/
  ){
    // retrieve data from archive required to construct new instance
    std::string n;
    ar >> n;
    // invoke inplace constructor to initialize instance of my_class
    ::new(t)Gecode::Reflection::ActorSpec(
      Gecode::Support::Symbol(n.c_str(),true));
  }
  template<class Archive>
  void save(
    Archive& ar, const Gecode::Reflection::ActorSpec& spec,
    unsigned int /*file_version*/
  ) {
    int noa = spec.noOfArgs();
    ar << noa;
    for (int i=0; i<spec.noOfArgs(); i++) {
      Gecode::Reflection::Arg* const arg = spec[i];
      ar << arg;
    }
  }
  template<class Archive>
  void load(
    Archive& ar, Gecode::Reflection::ActorSpec& spec,
    unsigned int /*file_version*/
  ) {
    int noOfArgs;
    ar >> noOfArgs;
    for (int i=noOfArgs; i--;) {
      Gecode::Reflection::Arg* arg;
      ar >> arg;
      spec << arg;
    }
  }
  template<class Archive>
  inline void serialize(
      Archive & ar,
      Gecode::Reflection::ActorSpec& spec,
      const unsigned int file_version
  ){
      split_free(ar, spec, file_version);
  }

}}

BOOST_CLASS_EXPORT(Gecode::Reflection::Arg)
BOOST_CLASS_EXPORT(Gecode::Reflection::VarSpec)
BOOST_CLASS_EXPORT(Gecode::Reflection::ActorSpec)

// STATISTICS: serialization-any
