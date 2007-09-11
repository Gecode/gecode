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
#include "log.hh"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>

namespace {
  using namespace std;
  vector<string> hinitial;
  vector<string> hops;
  typedef pair<string, bool> psb;


  bool do_logging = false;
}


namespace Log {
  using namespace std;
  void logging(bool val) {
    do_logging = val;
  }

  bool logging() {
    return do_logging;
  }

  void reset() {
    if (!do_logging) return;
    hinitial.clear();
    hops.clear();
  }

  void print(void) {
    if (!do_logging) return;
    cout << "Initial configuration" << endl;
    for (unsigned int i = 0; i < hinitial.size(); ++i) {
      cout << hinitial[i] << endl;
    }
    cout << endl;
    
    cout << "Operations" << endl;
    for (unsigned int i = 0; i < hops.size(); ++i) {
      cout << hops[i] << endl;
    }
    cout << endl;
  }

  std::string mk_name(const char* arr, int idx) {
    if (!do_logging) return string("");
    ostringstream res;
    res << arr << "[" << idx << "]";
    return res.str();
  }


  void initial(const IntVarArray& a, const char* name) {
    if (!do_logging) return;
    // Display
    ostringstream hiva;
    hiva << "IntVarArray " << name << " = ";
    hinitial.push_back(hiva.str());

    for (int i = 0; i < a.size(); ++i) {
      ostringstream ai;
      ai << "\t" << name << "[" << i << "]="
         << a[i];
      hinitial.push_back(ai.str());
    }
  }


  void initial(const BoolVar& a, const char* name) {
    if (!do_logging) return;
    // Display
    ostringstream hiva;
    hiva << "BoolVar " << name << " = " << a;
    hinitial.push_back(hiva.str());

  }

#ifdef GECODE_HAVE_SET_VARS
  void initial(const SetVarArray& a, const char* name) {
    if (!do_logging) return;
    // Display
    ostringstream hiva;
    hiva << "SetVarArray " << name << " = ";
    hinitial.push_back(hiva.str());

    for (int i = 0; i < a.size(); ++i) {
      ostringstream ai;
      ai << "\t" << name << "[" << i << "]="
         << a[i];
      hinitial.push_back(ai.str());
    }

  }
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS  
  void initial(const CpltSetVarArray& a, const char* name) {
    if (!do_logging) return;
    // Display
    ostringstream hiva;
    hiva << "CpltSetVarArray " << name << " = ";
    hinitial.push_back(hiva.str());

    for (int i = 0; i < a.size(); ++i) {
      ostringstream ai;
      ai << "\t" << name << "[" << i << "]="
	 << a[i];
      hinitial.push_back(ai.str());
    }

  }
#endif

  void log(const std::string hlog) {
    if (!do_logging) return;
    hops.push_back(hlog);
  }

  void assign(std::string name, int val) {
    if (!do_logging) return;
    ostringstream h;
    h << "Assign val=" << val << " to " << name;
    hops.push_back(h.str());
  }

  void assign(std::string name, const IntSet& val) {
    if (!do_logging) return;
    ostringstream h;
    h << "Assign val=" << val << " to " << name;
    hops.push_back(h.str());
  }

  void prune(const IntVar& v, std::string name, IntRelType irt, int val) {
    if (!do_logging) return;
    ostringstream h;
    switch (irt) {
    case IRT_NQ:
      h << "Prune val=" << val << " from " << name << " =" << v;
      break;
    case IRT_LE:
      h << "Prune vals above " << val << " from " << name << " =" << v;
      break;
    case IRT_GR:
      h << "Prune vals below " << val << " from " << name << " =" << v;
      break;
    default: break;
    }
    hops.push_back(h.str());
  }

#ifdef GECODE_HAVE_SET_VARS
  void prune(const SetVar& v, std::string name, SetRelType srt, int val) {
    if(!do_logging) return;
    ostringstream h;
    switch(srt) {
    case SRT_SUP:
      h << "Include val=" << val << " in " << name << " =" << v;
      break;
    case SRT_DISJ:
      h << "Exclude val=" << val << " from " << name << " =" << v;
      break;
    default: break;
    }
    hops.push_back(h.str());
  }

  void prune(const SetVar& v, std::string name,
             unsigned int cardMin, unsigned int cardMax) {
    if(!do_logging) return;
    ostringstream h;
    h << "Prune cardinality of " << name << " to "
      << cardMin << "/" << cardMax;
    hops.push_back(h.str());
  }
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS
  void prune(const CpltSetVar& v, std::string name, SetRelType brt, int val) {
    if(!do_logging) return;
    ostringstream h;
    switch(brt) {
    case SRT_SUP:
      h << "Include val=" << val << " in " << name << " =" << v;
      break;
    case SRT_DISJ:
      h << "Exclude val=" << val << " from " << name << " =" << v;
      break;
    default: break;
    }
    hops.push_back(h.str());
  }


  void prune(const CpltSetVar& v, std::string name,
	     unsigned int cardMin, unsigned int cardMax) {
    if(!do_logging) return;
    ostringstream h;
    h << "Prune cardinality of " << name << " to "
      << cardMin << "/" << cardMax;
    hops.push_back(h.str());
  }
#endif

  void prune_result(const IntVar& v) {
    if (!do_logging) return;
    ostringstream h;
    h << "\tPruning resulted in domain " << v;
    hops.push_back(h.str());
  }

#ifdef GECODE_HAVE_SET_VARS
  void prune_result(const SetVar& v) {
    if (!do_logging) return;
    ostringstream h;
    h << "\tPruning resulted in domain " << v;
    hops.push_back(h.str());
  }
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS
  void prune_result(const CpltSetVar& v) {
    if (!do_logging) return;
    ostringstream h;
    h << "\tPruning resulted in domain " << v;
    hops.push_back(h.str());
  }
#endif

  void fixpoint() {
    if (!do_logging) return;
    hops.push_back("Compute fixpoint");
  }

  void print(const IntVarArray& a, const char* name) {
    if (!do_logging) return;
    ostringstream h;
    if (name) h << name << "[] = ";
    h << "{";
    for (int i = 0; i < a.size(); ++i) {
      h << a[i];
      if (i != a.size()-1) h << ", ";
    }
    h << "}";
    hops.push_back(h.str());
  }

  void print(const BoolVar& a, const char* name) {
    if (!do_logging) return;
    ostringstream h;
    if (name) h << name << " = ";
    h << a;
    hops.push_back(h.str());
  }

#ifdef GECODE_HAVE_SET_VARS
  void print(const SetVarArray& a, const char* name) {
    if (!do_logging) return;
    ostringstream h;
    if (name) h << name << "[] = ";
    h << "{";
    for (int i = 0; i < a.size(); ++i) {
      h << a[i];
      if (i != a.size()-1) h << ", ";
    }
    hops.push_back(h.str());
  }
#endif

#ifdef GECODE_HAVE_CPLTSET_VARS
  void print(const CpltSetVarArray& a, const char* name) {
    if (!do_logging) return;
    ostringstream h;
    if (name) h << name << "[] = ";
    h << "{";
    for (int i = 0; i < a.size(); ++i) {
      h << a[i];
      if (i != a.size()-1) h << ", ";
    }
    hops.push_back(h.str());
  }
#endif


  std::string iclh(IntConLevel icl) {
    std::string res;
    switch(icl) {
    case ICL_DOM:
      res = "domain";
      break;
    case ICL_BND:
      res = "bound";
      break;
    case ICL_VAL:
      res = "value";
      break;
    case ICL_DEF:
      res = "default";
      break;
    }
    return res;
  }
  std::string irth(IntRelType  irt) {
    std::string hr;
    switch(irt) {
    case IRT_LQ:
      hr = "<=";
      break;
    case IRT_LE:
      hr = "<";
      break;
    case IRT_EQ:
      hr = "==";
      break;
    case IRT_NQ:
      hr = "!=";
      break;
    case IRT_GQ:
      hr = ">=";
      break;
    case IRT_GR:
      hr = ">";
      break;
    }
    return hr;
  }

}

// STATISTICS: test-core
