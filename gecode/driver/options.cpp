/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
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

#include <gecode/driver.hh>

#include <iostream>
#include <iomanip>

#include <cstdlib>
#include <cstring>

namespace Gecode {

  namespace Driver {

    /*
     * Option baseclass
     *
     */
    char*
    BaseOption::strdup(const char* s) {
      if (s == NULL)
        return NULL;
      char* d = heap.alloc<char>(static_cast<unsigned long int>(strlen(s)+1));
      (void) strcpy(d,s);
      return d;
    }

    void
    BaseOption::strdel(const char* s) {
      if (s == NULL)
        return;
      heap.rfree(const_cast<char*>(s));
    }

    BaseOption::BaseOption(const char* o, const char* e)
      : opt(strdup(o)), exp(strdup(e)) {}

    BaseOption::~BaseOption(void) {
      strdel(opt);
      strdel(exp);
    }


    bool
    StringValueOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt))
        return false;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
      cur = strdup(argv[2]);
      // Remove options
      argc -= 2;
      for (int i=1; i<argc; i++)
        argv[i] = argv[i+2];
      return true;
    }
    
    void
    StringValueOption::help(void) {
      using namespace std;
      cerr << '\t' << opt << " (string) default: " 
           << ((cur == NULL) ? "NONE" : cur) << endl
           << "\t\t" << exp << endl;
    }
  
    StringValueOption::~StringValueOption(void) {
      strdel(cur);
    }
  


    void
    StringOption::add(int v, const char* o, const char* h) {
      Value* n = new Value;
      n->val  = v;
      n->opt  = strdup(o);
      n->help = strdup(h);
      n->next = NULL;
      if (fst == NULL) {
        fst = n;
      } else {
        lst->next = n;
      }
      lst = n;
    }
    
    bool
    StringOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt))
        return false;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
      for (Value* v = fst; v != NULL; v = v->next)
        if (!strcmp(argv[2],v->opt)) {
          cur = v->val;
          // Remove options
          argc -= 2;
          for (int i=1; i<argc; i++)
            argv[i] = argv[i+2];
          return true;
        }
      std::cerr << "Wrong argument \"" << argv[2]
                << "\" for option \"" << opt << "\""
                << std::endl;
      exit(EXIT_FAILURE);
    }
    
    void
    StringOption::help(void) {
      if (fst == NULL)
        return;
      std::cerr << '\t' << opt << " (";
      const char* d = NULL;
      for (Value* v = fst; v != NULL; v = v->next) {
        std::cerr << v->opt << ((v->next != NULL) ? ", " : "");
        if (v->val == cur)
          d = v->opt;
      }
      std::cerr << ")";
      if (d != NULL)
        std::cerr << " default: " << d;
      std::cerr << std::endl << "\t\t" << exp << std::endl;
      for (Value* v = fst; v != NULL; v = v->next)
        if (v->help != NULL)
          std::cerr << "\t\t  " << v->opt << ": " << v->help << std::endl;
    }
    
    StringOption::~StringOption(void) {
      Value* v = fst;
      while (v != NULL) {
        strdel(v->opt);
        strdel(v->help);
        Value* n = v->next;
        delete v;
        v = n;
      }
    }
    
    
    bool
    IntOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt))
        return false;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
      cur = atoi(argv[2]);
      // Remove options
      argc -= 2;
      for (int i=1; i<argc; i++)
        argv[i] = argv[i+2];
      return true;
    }
    
    void
    IntOption::help(void) {
      using namespace std;
      cerr << '\t' << opt << " (int) default: " << cur << endl
           << "\t\t" << exp << endl;
    }
  

    bool
    UnsignedIntOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt))
        return false;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
      cur = static_cast<unsigned int>(atoi(argv[2]));
      // Remove options
      argc -= 2;
      for (int i=1; i<argc; i++)
        argv[i] = argv[i+2];
      return true;
    }
    
    void
    UnsignedIntOption::help(void) {
      using namespace std;
      cerr << '\t' << opt << " (unsigned int) default: " << cur << endl
           << "\t\t" << exp << endl;
    }
  

    bool
    DoubleOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt))
        return false;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
      cur = atof(argv[2]);
      // Remove options
      argc -= 2;
      for (int i=1; i<argc; i++)
        argv[i] = argv[i+2];
      return true;
    }
    
    void
    DoubleOption::help(void) {
      using namespace std;
      cerr << '\t' << opt << " (double) default: " << cur << endl
           << "\t\t" << exp << endl;
    }

    bool
    BoolOption::parse(int& argc, char* argv[]) {
      if ((argc < 2) || strcmp(argv[1],opt)) {
        return false;
      }
      // Remove options
      argc--;
      for (int i=1; i<argc; i++)
        argv[i] = argv[i+1];
      cur = true;
      return true;
    }

    void 
    BoolOption::help(void) {
      using namespace std;
      cerr << '\t' << opt << endl << "\t\t" << exp << endl;
    }

  
  }

  BaseOptions::BaseOptions(const char* n)
    : fst(NULL), lst(NULL), 
      _name(Driver::BaseOption::strdup(n)) {}

  void
  BaseOptions::name(const char* n) {
    Driver::BaseOption::strdel(_name);
    _name = Driver::BaseOption::strdup(n);
  }

  void
  BaseOptions::help(void) {
    std::cerr << "Gecode configuration information:" << std::endl
              << " - Version: " << GECODE_VERSION << std::endl
              << " - Variable types: ";
#ifdef GECODE_HAS_INT_VARS
    std::cerr << "BoolVar IntVar ";
#endif
#ifdef GECODE_HAS_SET_VARS
    std::cerr << "SetVar";
#endif
    std::cerr << std::endl
              << " - Thread support: ";
#ifdef GECODE_HAS_THREADS
    if (Support::Thread::npu() == 1)
      std::cerr << "enabled (1 processing unit)";
    else
      std::cerr << "enabled (" << Support::Thread::npu() 
                << " processing units)";
#else
    std::cerr << "disabled";
#endif
    std::cerr << std::endl
              << " - Gist support: ";
#ifdef GECODE_HAS_GIST
    std::cerr << "enabled";
#else
    std::cerr << "disabled";
#endif
    std::cerr << std::endl << std::endl
              << "Options for " << name() << ":" << std::endl
              << "\t-help, --help, -?" << std::endl
              << "\t\tprint this help message" << std::endl;
    for (Driver::BaseOption* o = fst; o != NULL; o = o->next)
      o->help();
  }

  void
  BaseOptions::parse(int& argc, char* argv[]) {
  next:
    for (Driver::BaseOption* o = fst; o != NULL; o = o->next)
      if (o->parse(argc,argv))
        goto next;
    if (argc < 2)
      return;
    if (!strcmp(argv[1],"-help") || !strcmp(argv[1],"--help") ||
        !strcmp(argv[1],"-?")) {
      help();
      exit(EXIT_SUCCESS);
    }
    return;
  }
  
  BaseOptions::~BaseOptions(void) {
    Driver::BaseOption::strdel(_name);
  }


  Options::Options(const char* n)
    : BaseOptions(n),
      
      _model("-model","model variants"),
      _symmetry("-symmetry","symmetry variants"),
      _propagation("-propagation","propagation variants"),
      _icl("-icl","integer consistency level",ICL_DEF),
      _branching("-branching","branching variants"),
      
      _search("-search","search engine variants"),
      _solutions("-solutions","number of solutions (0 = all)",1),
      _threads("-threads","number of threads (0 = #processing units)",
               Search::Config::threads),
      _c_d("-c-d","recomputation commit distance",Search::Config::c_d),
      _a_d("-a-d","recomputation adaptation distance",Search::Config::a_d),
      _node("-node","node cutoff (0 = none, solution mode)"),
      _fail("-fail","failure cutoff (0 = none, solution mode)"),
      _time("-time","time (in ms) cutoff (0 = none, solution mode)"),
      _interrupt("-interrupt","whether to catch Ctrl-C (true) or not (false)", true),
      
      _mode("-mode","how to execute script",SM_SOLUTION),
      _samples("-samples","how many samples (time mode)",1),
      _iterations("-iterations","iterations per sample (time mode)",1)
  {
    
    _icl.add(ICL_DEF, "def"); _icl.add(ICL_VAL, "val");
    _icl.add(ICL_BND, "bnd"); _icl.add(ICL_DOM, "dom");
    
    _mode.add(SM_SOLUTION, "solution");
    _mode.add(SM_TIME, "time");
    _mode.add(SM_STAT, "stat");
    _mode.add(SM_GIST, "gist");
    
    _interrupt.add(false, "false");
    _interrupt.add(true, "true");
    
    add(_model); add(_symmetry); add(_propagation); add(_icl); 
    add(_branching);
    add(_search); add(_solutions); add(_threads); add(_c_d); add(_a_d);
    add(_node); add(_fail); add(_time); add(_interrupt);
    add(_mode); add(_iterations); add(_samples);
  }

  
  SizeOptions::SizeOptions(const char* e)
    : Options(e), _size(0) {}
  
  void
  SizeOptions::help(void) {
    Options::help();
    std::cerr << "\t(unsigned int) default: " << size() << std::endl
              << "\t\twhich version/size for script" << std::endl;
  }

  void
  SizeOptions::parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 2)
      return;
    size(static_cast<unsigned int>(atoi(argv[1])));
  }



  InstanceOptions::InstanceOptions(const char* e)
    : Options(e), _inst(NULL) {}
  
  void
  InstanceOptions::instance(const char* s) {
    Driver::BaseOption::strdel(_inst);
    _inst = strdup(s);
  }

  void
  InstanceOptions::help(void) {
    Options::help();
    std::cerr << "\t(string) default: " << instance() << std::endl
              << "\t\twhich instance for script" << std::endl;
  }

  void
  InstanceOptions::parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 2)
      return;
    instance(argv[1]);
  }

  InstanceOptions::~InstanceOptions(void) {
    Driver::BaseOption::strdel(_inst);
  }

}

// STATISTICS: driver-any
