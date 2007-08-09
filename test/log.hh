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
#ifndef __GECODE_LOG_HH__
#define __GECODE_LOG_HH__

#include "gecode/kernel.hh"
#include "gecode/int.hh"

#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS
#include "gecode/cpltset.hh"
#endif

namespace Log {

  using namespace Gecode;

  void logging(bool val);
  bool logging();
  void reset();

  void print(bool for_display = true);

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

#ifdef GECODE_HAVE_CPLTSET_VARS
  void initial(const CpltSetVarArray& a, const char*name);
  void prune(const CpltSetVar& v, std::string name, SetRelType irt, int val);
  void prune(const CpltSetVar& v, std::string name,
	     unsigned int cardMin, unsigned int cardMax);
  void prune_result(const CpltSetVar& v);
  void print(const CpltSetVarArray& a, const char* name=NULL);
#endif

  std::string iclh(IntConLevel icl);
  std::string iclc(IntConLevel icl);
  std::string irth(IntRelType  irt);
  std::string irtc(IntRelType  irt);
}

#endif /* __GECODE_LOG_HH__ */

// STATISTICS: test-core

