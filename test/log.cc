/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date: 2005-08-03 17:17:11 +0200 (Wed, 03 Aug 2005) $ by $Author: schulte $
 *     $Revision: 2126 $
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
#include "log.hh"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

namespace {
  std::vector<std::string> hinitial;
  std::vector<std::string> hops;
  std::vector<std::string> cdecla;
  std::vector<std::string> cinitlist;
  std::vector<std::string> cinit;
  std::vector<std::string> cupdate;
  std::vector<std::string> cops;
  std::vector<std::string> cnames;

  std::string space = ""
  "#include \"kernel.hh\" \n"
  "#include \"int.hh\" \n"
  "#include \"set.hh\" \n"
  "#include <iostream> \n"
  " \n"
  "using namespace Gecode; \n"
  " \n"
  "  unsigned int tmp; \n"
  " \n"
  "  class ErrorSpace : public Space { \n"
  "  public: \n"
  "DECL"
  "    ErrorSpace() : \n"
  "INITLIST \n"
  "    { \n"
  "INIT"
  "    } \n"
  "    ErrorSpace(bool share, ErrorSpace& s) : Space(share,s) { \n"
  "UPDATE"
  "    } \n"
  "    virtual Space* copy(bool share) { \n"
  "      return new ErrorSpace(share,*this); \n"
  "    } \n"
  "    void print() {\n"
  "PRINTOPS"
  "    }\n"
  "    void run() { \n"
  "OPERATIONS"
  "    } \n"
  "  }; \n"
  "  int main() { \n"
  "    ErrorSpace es; \n"
  "    es.run(); \n"
  "    switch (es.status(tmp)) { \n"
  "    case SS_FAILED: \n"
  "      std::cout << \"Failed.\" << std::endl; \n"
  "      break; \n"
  "    case SS_SOLVED: \n"
  "      std::cout << \"Solved.\" << std::endl; \n"
  "      break; \n"
  "    default: \n"
  "      std::cout << \"Other\" << std::endl; \n"
  "      break; \n"
  "    } \n"
  "  } \n";

  std::string printloop = ""
  "\tstd::cout << \"NAME[] = {\";\n"
  "\tfor (int i = 0; i < NAME.size(); ++i) {\n"
  "\t\tstd::cout << NAME[i];\n"
  "\t\tif (i != NAME.size()-1) std::cout << \", \";\n"
  "\t}\n"
  "\tstd::cout << \"}\" << std::endl;\n";

  bool do_logging = false;
}


namespace Log {
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
    cdecla.clear();
    cinitlist.clear();
    cinit.clear();
    cupdate.clear();
    cops.clear();
    cnames.clear();
  }

  void print(/*std::ostream& o, */bool for_display) {
    if (!do_logging) return;
    if (for_display) {
      std::cout << "Initial configuration" << std::endl;
      for (unsigned int i = 0; i < hinitial.size(); ++i) {
	std::cout << hinitial[i] << std::endl;
      }
      std::cout << std::endl;
      
      std::cout << "Operations" << std::endl;
      for (unsigned int i = 0; i < hops.size(); ++i) {
	std::cout << hops[i] << std::endl;
      }
      std::cout << std::endl;
    } else {
      std::string error_space = space;

      std::ostringstream decl;
      for (unsigned int i = 0; i < cdecla.size(); ++i) {
	decl << cdecla[i] << std::endl;
      }
      error_space.replace(error_space.find("DECL"), 4,
			  decl.str(), 0, decl.str().length());

      std::ostringstream initlist;
      for (unsigned int i = 0; i < cinitlist.size(); ++i) {
	initlist << cinitlist[i];
	if (i+1 != cinitlist.size()) initlist << ", ";
      }
      error_space.replace(error_space.find("INITLIST"), 9,
			  initlist.str(), 0, initlist.str().length());

      std::ostringstream init;
      for (unsigned int i = 0; i < cinit.size(); ++i) {
	init << cinit[i] << std::endl;
      }
      error_space.replace(error_space.find("INIT"), 4,
			  init.str(), 0, init.str().length());

      std::ostringstream update;
      for (unsigned int i = 0; i < cupdate.size(); ++i) {
	update << cupdate[i] << std::endl;
      }
      error_space.replace(error_space.find("UPDATE"), 6,
			  update.str(), 0, update.str().length());

      std::ostringstream operations;
      for (unsigned int i = 0; i < cops.size(); ++i) {
	operations << cops[i] << std::endl;
      }
      error_space.replace(error_space.find("OPERATIONS"), 10,
			  operations.str(), 0, operations.str().length());

      std::ostringstream printops;
      for (unsigned int i = 0; i < cnames.size(); ++i) {
	std::string pop = printloop,
	  name = cnames[i];
	while (pop.find("NAME") != std::string::npos)
	  pop.replace(pop.find("NAME"), 4, name, 0, name.length());
	printops << pop;
      }
      error_space.replace(error_space.find("PRINTOPS"), 8,
			  printops.str(), 0, printops.str().length());

      std::cout << error_space;
    }
    
  }

  std::string mk_name(const char* arr, int idx) {
    if (!do_logging) return std::string("");
    std::ostringstream res;
    res << arr << "[" << idx << "]";
    return res.str();
  }


  void initial(const IntVarArray& a, const char* name) {
    if (!do_logging) return;
    // Display
    std::ostringstream hiva;
    hiva << "IntVarArray " << name << " = ";
    hinitial.push_back(hiva.str());

    for (int i = 0; i < a.size(); ++i) {
      std::ostringstream ai;
      ai << "\t" << name << "[" << i << "]="
	 << a[i];
      hinitial.push_back(ai.str());
    }

    // Execution
    std::ostringstream civa;
    civa << "\tIntVarArray " << name << ";";
    cdecla.push_back(civa.str());

    std::ostringstream initlist;
    initlist << "\t" << name << "(this," << a.size() << ")";
    cinitlist.push_back(initlist.str());

    std::ostringstream update;
    update << "\t" <<  name << ".update(this, share, s." << name << ");";
    cupdate.push_back(update.str());    

    cnames.push_back(name);

    for (int i = 0; i < a.size(); ++i) {
      std::ostringstream init;
      init << "\tconst int arr" << i << "[][2] = {";
      int n = 0;
      for (IntVarRanges it(a[i]); it(); ++it, ++n) {
	if (n) init << ",";
	init << "{" << it.min() << "," << it.max() << "}";
      }
      init << "};\n\t"
	   << name << "[" << i << "] = IntVar(this, IntSet(arr"
	   << i << ", " << n << "));";
      cinit.push_back(init.str());
    }
  }

#ifdef GECODE_HAVE_SET_VARS
  void initial(const SetVarArray& a, const char* name) {
    if (!do_logging) return;
    // Display
    std::ostringstream hiva;
    hiva << "SetVarArray " << name << " = ";
    hinitial.push_back(hiva.str());

    for (int i = 0; i < a.size(); ++i) {
      std::ostringstream ai;
      ai << "\t" << name << "[" << i << "]="
	 << a[i];
      hinitial.push_back(ai.str());
    }

    // Execution
    std::ostringstream civa;
    civa << "\tSetVarArray " << name << ";";
    cdecla.push_back(civa.str());

    std::ostringstream initlist;
    initlist << "\t" << name << "(this," << a.size() << ")";
    cinitlist.push_back(initlist.str());

    std::ostringstream update;
    update << "\t" <<  name << ".update(this, share, s." << name << ");";
    cupdate.push_back(update.str());    

    cnames.push_back(name);

    for (int i = 0; i < a.size(); ++i) {
      std::ostringstream init;
      init << "\tconst int arrGlb" << i << "[][2] = {";
      int n = 0;
      for (SetVarGlbRanges it(a[i]); it(); ++it, ++n) {
	if (n) init << ",";
	init << "{" << it.min() << "," << it.max() << "}";
      }
      init << "};\n";
      init << "\tconst int arrLub" << i << "[][2] = {";
      int m = 0;
      for (SetVarLubRanges it(a[i]); it(); ++it, ++m) {
	if(m) init << ",";
	init << "{" << it.min() << "," << it.max() << "}";
      }
      init << "};\n\t";
      init << name << "[" << i << "] = SetVar(this, IntSet(arrGlb"
	   << i << ", " << n << "), IntSet(arrLub"
	   << i << ", " << m << "), " << a[i].cardMin()
	   << ", " << a[i].cardMax() << ");";
      cinit.push_back(init.str());
    }
  }
#endif

  void log(const std::string hlog, const std::string clog = "") {
    if (!do_logging) return;
    hops.push_back(hlog);
    if (clog != "")
      cops.push_back(clog);
  }

  void assign(std::string name, int val) {
    if (!do_logging) return;
    std::ostringstream h;
    h << "Assign val=" << val << " to " << name;
    hops.push_back(h.str());

    std::ostringstream c;
    c << "\trel(this, " << name << ", IRT_EQ, " << val << ");";
    cops.push_back(c.str());
  }

  void assign(std::string name, const IntSet& val) {
    if (!do_logging) return;
    std::ostringstream h;
    h << "Assign val=" << val << " to " << name;
    hops.push_back(h.str());

    std::ostringstream c;

    c << "\t{ const int arr[][2] = {";
    int n = 0;
    for (IntSetRanges it(val); it(); ++it, ++n) {
      if(n) c << ",";
      c << "{" << it.min() << "," << it.max() << "}";
    }
    c << "};\n";
    if (n==0) {
      c << "\t  IntSet is(IntSet::empty);\n";
    } else {
      c << "\t  IntSet is(arr, " << n << ");\n";
    }
    c << "\t dom(this, " << name << ", SRT_EQ, " << "is); }";
    cops.push_back(c.str());
  }

  void prune(const IntVar& v, std::string name, IntRelType irt, int val) {
    if (!do_logging) return;
    std::ostringstream h;
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

    std::ostringstream c;
    switch (irt) {
    case IRT_NQ:
      c << "\trel(this, " << name << ", IRT_NQ, " << val << ");";
      break;
    case IRT_LE:
      c << "\trel(this, " << name << ", IRT_LE, " << val << ");";
      break;
    case IRT_GR:
      c << "\trel(this, " << name << ", IRT_GR, " << val << ");";
      break;
    default: break;
    }
    cops.push_back(c.str());
  }

#ifdef GECODE_HAVE_SET_VARS
  void prune(const SetVar& v, std::string name, SetRelType srt, int val) {
    if(!do_logging) return;
    std::ostringstream h;
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

    std::ostringstream c;
    switch(srt) {
    case SRT_SUP:
      c << "\tdom(this, " << name << ", SRT_SUP, " << val << ");";
      break;
    case SRT_DISJ:
      c << "\tdom(this, " << name << ", SRT_DISJ, " << val << ");";
      break;
    default: break;
    }
    cops.push_back(c.str());
  }

  void prune(const SetVar& v, std::string name,
	     unsigned int cardMin, unsigned int cardMax) {
    if(!do_logging) return;
    std::ostringstream h;
    h << "Prune cardinality of " << name << " to "
      << cardMin << "/" << cardMax;
    hops.push_back(h.str());

    std::ostringstream c;
    c << "\tcardinality(this, " << name << ", "
      << cardMin << ", " << cardMax << ");";
    cops.push_back(c.str());
  }
#endif

  void prune_result(const IntVar& v) {
    if (!do_logging) return;
    std::ostringstream h;
    h << "\tPruning resulted in domain " << v;
    hops.push_back(h.str());
  }

#ifdef GECODE_HAVE_SET_VARS
  void prune_result(const SetVar& v) {
    if (!do_logging) return;
    std::ostringstream h;
    h << "\tPruning resulted in domain " << v;
    hops.push_back(h.str());
  }
#endif

  void flush() {
    if (!do_logging) return;
    hops.push_back("Flush caches");
    cops.push_back("\tflush();");
  }

  void fixpoint() {
    if (!do_logging) return;
    hops.push_back("Compute fixpoint");
    cops.push_back("\tstatus(tmp);");
  }

  void print(const IntVarArray& a, const char* name) {
    if (!do_logging) return;
    std::ostringstream h;
    if (name) h << name << "[] = ";
    h << "{";
    for (int i = 0; i < a.size(); ++i) {
      h << a[i];
      if (i != a.size()-1) h << ", ";
    }
    hops.push_back(h.str());
    cops.push_back("\tprint();");
  }

#ifdef GECODE_HAVE_SET_VARS
  void print(const SetVarArray& a, const char* name) {
    if (!do_logging) return;
    std::ostringstream h;
    if (name) h << name << "[] = ";
    h << "{";
    for (int i = 0; i < a.size(); ++i) {
      h << a[i];
      if (i != a.size()-1) h << ", ";
    }
    hops.push_back(h.str());
    cops.push_back("\tprint();");
  }
#endif

}

// STATISTICS: test-core
