/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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
#ifndef __GECODE_LOG_HH__
#define __GECODE_LOG_HH__

#include "gecode/kernel.hh"
#include "gecode/int.hh"

#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

namespace Log {

  using namespace Gecode;

  void logging(bool val);
  bool logging();
  void reset();

  void print(/*std::ostream& o, */bool for_display = true);

  std::string mk_name(const char* arr, int idx);

  void initial(const IntVarArray& a, const char* name);
  void initial(const BoolVar& a, const char* name);
  void log(std::string hlog, const std::string clog);
  void assign(std::string name, int val);
  void assign(std::string name, const IntSet& val);
  void prune(const IntVar& v, std::string name, IntRelType irt, int val);
  void prune_result(const IntVar& v);
  void flush();
  void fixpoint();
  void print(const IntVarArray& a, const char* name=NULL);
  void print(const BoolVar& a, const char* name=NULL);

#ifdef GECODE_HAVE_SET_VARS
  void initial(const SetVarArray& a, const char*name);
  void prune(const SetVar& v, std::string name, SetRelType irt, int val);
  void prune(const SetVar& v, std::string name,
             unsigned int cardMin, unsigned int cardMax);
  void prune_result(const SetVar& v);
  void print(const SetVarArray& a, const char* name=NULL);
#endif
}

#endif /* __GECODE_LOG_HH__ */

// STATISTICS: test-core

