/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

    char*
    BaseOption::stredup(const char* s) {
      if (s == NULL)
        return NULL;
      char* d = heap.alloc<char>(static_cast<unsigned long int>(strlen(s)+2));
      d[0] = '-';
      (void) strcpy(d+1,s);
      return d;
    }

    void
    BaseOption::strdel(const char* s) {
      if (s == NULL)
        return;
      heap.rfree(const_cast<char*>(s));
    }

    char*
    BaseOption::argument(int argc, char* argv[]) const {
      if (argc < 2)
        return NULL;
      const char* s = argv[1];
      if (s[0] == '-') {
        s++;
        if (s[0] == '-')
          s++;
      } else {
        return NULL;
      }
      if (strcmp(s,eopt))
        return NULL;
      if (argc == 2) {
        std::cerr << "Missing argument for option \"" << iopt << "\""
                  << std::endl;
        exit(EXIT_FAILURE);
      }
      return argv[2];
    }

    BaseOption::BaseOption(const char* o, const char* e)
      : eopt(strdup(o)), iopt(stredup(o)), exp(strdup(e)) {}

    BaseOption::~BaseOption(void) {
      strdel(eopt);
      strdel(iopt);
      strdel(exp);
    }


    StringValueOption::StringValueOption(const char* o, const char* e,
                                         const char* v)
      : BaseOption(o,e), cur(strdup(v)) {}
    void
    StringValueOption::value(const char* v) {
      strdel(cur);
      cur = strdup(v);
    }
    int
    StringValueOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        cur = strdup(a);
        return 2;
      }
      return 0;
    }
    void
    StringValueOption::help(void) {
      std::cerr << '\t' << iopt << " (string) default: "
                << ((cur == NULL) ? "NONE" : cur) << std::endl
                << "\t\t" << exp << std::endl;
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
    int
    StringOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        for (Value* v = fst; v != NULL; v = v->next)
          if (!strcmp(a,v->opt)) {
            cur = v->val;
            return 2;
          }
        std::cerr << "Wrong argument \"" << a
                  << "\" for option \"" << iopt << "\""
                  << std::endl;
        exit(EXIT_FAILURE);
      }
      return 0;
    }
    void
    StringOption::help(void) {
      if (fst == NULL)
        return;
      std::cerr << '\t' << iopt << " (";
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


    int
    IntOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        cur = atoi(a);
        return 2;
      }
      return 0;
    }

    void
    IntOption::help(void) {
      std::cerr << '\t' << iopt << " (int) default: " << cur << std::endl
                << "\t\t" << exp << std::endl;
    }


    int
    UnsignedIntOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        cur = static_cast<unsigned int>(atoi(a));
        return 2;
      }
      return 0;
    }

    void
    UnsignedIntOption::help(void) {
      std::cerr << '\t' << iopt << " (unsigned int) default: "
                << cur << std::endl
                << "\t\t" << exp << std::endl;
    }


    int
    DoubleOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        cur = atof(a);
        return 2;
      }
      return 0;
    }

    void
    DoubleOption::help(void) {
      using namespace std;
      cerr << '\t' << iopt << " (double) default: " << cur << endl
           << "\t\t" << exp << endl;
    }


    int
    BoolOption::parse(int argc, char* argv[]) {
      if (argc < 2)
        return 0;
      const char* s = argv[1];
      if (s[0] == '-') {
        s++;
        if (s[0] == '-')
          s++;
      } else {
        return 0;
      }
      if (strcmp(s,eopt))
        return 0;
      if (argc == 2) {
        // Option without argument
        cur = true;
        return 1;
      } else if (!strcmp(argv[2],"true") || !strcmp(argv[2],"1")) {
        cur = true;
        return 2;
      } else if (!strcmp(argv[2],"false") || !strcmp(argv[2],"0")) {
        cur = false;
        return 2;
      } else {
        // Option without argument
        cur = true;
        return 1;
      }
      return 0;
    }

    void
    BoolOption::help(void) {
      using namespace std;
      cerr << '\t' << iopt << " (optional: false, 0, true, 1) default: "
           << (cur ? "true" : "false") << endl
           << "\t\t" << exp << endl;
    }

    /*
     * Integer propagation level option
     *
     */
    IplOption::IplOption(IntPropLevel ipl)
      : BaseOption("ipl","integer propagation level (comma-separated list)"),
        cur(ipl) {}

    int
    IplOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        int b = IPL_DEF;
        int m = IPL_DEF;
        do {
          // Search for a comma
          char* c = a;
          while ((*c != ',') && (*c != 0))
            c++;
          unsigned int e = static_cast<unsigned int>(c-a);
          if      (!strncmp("def",a,e))      { b = IPL_DEF; }
          else if (!strncmp("val",a,e))      { b = IPL_VAL; }
          else if (!strncmp("bnd",a,e))      { b = IPL_BND; }
          else if (!strncmp("dom",a,e))      { b = IPL_DOM; }
          else if (!strncmp("basic",a,e))    { m |= IPL_BASIC; }
          else if (!strncmp("advanced",a,e)) { m |= IPL_ADVANCED; }
          else {
            std::cerr << "Wrong argument \"" << a
                      << "\" for option \"" << iopt << "\""
                      << std::endl;
            exit(EXIT_FAILURE);
          }

          if (*c == ',') a = c+1; else a = c;

        } while (*a != 0);

        cur = static_cast<IntPropLevel>(b | m);
        return 2;
      }
      return 0;
    }

    void
    IplOption::help(void) {
      using namespace std;
      cerr << '\t' << iopt
           << " (def,val,bnd,dom,basic,advanced)" << endl
           << "\t\tdefault: ";
      switch (vbd(cur)) {
      case IPL_DEF: cerr << "def"; break;
      case IPL_VAL: cerr << "val"; break;
      case IPL_BND: cerr << "bnd"; break;
      case IPL_DOM: cerr << "dom"; break;
      default: GECODE_NEVER;
      }
      if (cur & IPL_BASIC)    cerr << ",basic";
      if (cur & IPL_ADVANCED) cerr << ",advanced";
      cerr << endl << "\t\t" << exp << endl;
    }


    /*
     * Trace flag option
     *
     */
    TraceOption::TraceOption(int f)
      : BaseOption("trace","trace flags (comma-separated list)"),
        cur(f) {}

    int
    TraceOption::parse(int argc, char* argv[]) {
      if (char* a = argument(argc,argv)) {
        cur = 0;
        do {
          // Search for a comma
          char* c = a;
          while ((*c != ',') && (*c != 0))
            c++;
          unsigned int e = static_cast<unsigned int>(c-a);
          if      (!strncmp("init",a,e))      { cur |= TE_INIT; }
          else if (!strncmp("prune",a,e))     { cur |= TE_PRUNE; }
          else if (!strncmp("fix",a,e))       { cur |= TE_FIX; }
          else if (!strncmp("fail",a,e))      { cur |= TE_FAIL; }
          else if (!strncmp("done",a,e))      { cur |= TE_DONE ; }
          else if (!strncmp("propagate",a,e)) { cur |= TE_PROPAGATE; }
          else if (!strncmp("commit",a,e))    { cur |= TE_COMMIT; }
          else if (!strncmp("post",a,e))      { cur |= TE_POST; }
          else if (!strncmp("none",a,e) ||
                   !strncmp("false",a,e) ||
                   !strncmp("0",a,e))         { cur = 0; }
          else if (!strncmp("all",a,e) ||
                   !strncmp("1",a,e))         { cur = (TE_INIT |
                                                       TE_PRUNE |
                                                       TE_FIX |
                                                       TE_FAIL |
                                                       TE_DONE |
                                                       TE_PROPAGATE |
                                                       TE_COMMIT |
                                                       TE_POST); }
          else if (!strncmp("variable",a,e))  { cur = (TE_INIT |
                                                       TE_PRUNE |
                                                       TE_FIX |
                                                       TE_FAIL |
                                                       TE_DONE); }
          else if (!strncmp("general",a,e))   { cur = (TE_PROPAGATE |
                                                       TE_COMMIT |
                                                       TE_POST); }
          else {
            std::cerr << "Wrong argument \"" << a
                      << "\" for option \"" << iopt << "\""
                      << std::endl;
            exit(EXIT_FAILURE);
          }

          if (*c == ',') a = c+1; else a = c;

        } while (*a != 0);

        return 2;
      }
      return 0;
    }

    void
    TraceOption::help(void) {
      using namespace std;
      cerr << '\t' << iopt
           << " (init,prune,fix,fail,done,propagate,commit,post,none,all,variable,general)"
           << " default: ";
      if (cur == 0) {
        cerr << "none";
      } else if (cur == (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE |
                         TE_PROPAGATE | TE_COMMIT | TE_POST)) {
        cerr << "all";
      } else if (cur == (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE)) {
        cerr << "variable";
      } else if (cur == (TE_PROPAGATE | TE_COMMIT | TE_POST)) {
        cerr << "general";
      } else {
        int f = cur;
        if ((f & TE_INIT) != 0) {
          cerr << "init";
          f -= TE_INIT;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_PRUNE) != 0) {
          cerr << "prune";
          f -= TE_PRUNE;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_FIX) != 0) {
          cerr << "fix";
          f -= TE_FIX;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_FAIL) != 0) {
          cerr << "fail";
          f -= TE_FAIL;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_DONE) != 0) {
          cerr << "done";
          f -= TE_DONE;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_PROPAGATE) != 0) {
          cerr << "propagate";
          f -= TE_PROPAGATE;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_COMMIT) != 0) {
          cerr << "commit";
          f -= TE_COMMIT;
          if (f != 0) cerr << ',';
        }
        if ((f & TE_POST) != 0) {
          cerr << "post";
        }
      }
      cerr << endl << "\t\t" << exp << endl;
    }


  }

  void
  BaseOptions::add(Driver::BaseOption& o) {
    o.next = NULL;
    if (fst == NULL) {
      fst=&o;
    } else {
      lst->next=&o;
    }
    lst=&o;
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
    std::cerr << "SetVar ";
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    std::cerr << "FloatVar "
              << std::endl
              << " - Trigonometric and transcendental float constraints: ";
#ifdef GECODE_HAS_MPFR
    std::cerr  << "enabled";
#else
    std::cerr << "disabled";
#endif
#endif
    std::cerr << std::endl;
    std::cerr << " - Thread support: ";
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
    std::cerr << std::endl
              << " - CPProfiler support: ";
#ifdef GECODE_HAS_CPPROFILER
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
    int c = argc;
    char** v = argv;
  next:
    for (Driver::BaseOption* o = fst; o != NULL; o = o->next)
      if (int a = o->parse(c,v)) {
        c -= a; v += a;
        goto next;
      }
    if (c >= 2) {
      if (!strcmp(v[1],"-help") || !strcmp(v[1],"--help") ||
          !strcmp(v[1],"-?")) {
        help();
        exit(EXIT_SUCCESS);
      }
    }
    // Copy remaining arguments
    argc = c;
    for (int i=1; i<argc; i++)
      argv[i] = v[i];
    return;
  }

  BaseOptions::~BaseOptions(void) {
    Driver::BaseOption::strdel(_name);
  }


  Options::Options(const char* n)
    : BaseOptions(n),

      _model("model","model variants"),
      _symmetry("symmetry","symmetry variants"),
      _propagation("propagation","propagation variants"),
      _branching("branching","branching variants"),
      _decay("decay","decay factor",1.0),
      _seed("seed","random number generator seed",1U),
      _step("step","step distance for float optimization",0.0),

      _search("search","search engine variants"),
      _solutions("solutions","number of solutions (0 = all)",1),
      _threads("threads","number of threads (0 = #processing units)",
               Search::Config::threads),
      _c_d("c-d","recomputation commit distance",Search::Config::c_d),
      _a_d("a-d","recomputation adaptation distance",Search::Config::a_d),
      _d_l("d-l","discrepancy limit for LDS",Search::Config::d_l),
      _node("node","node cutoff (0 = none, solution mode)"),
      _fail("fail","failure cutoff (0 = none, solution mode)"),
      _time("time","time (in ms) cutoff (0 = none, solution mode)"),
      _assets("assets","#portfolio assets (#engines)",0),
      _slice("slice","portfolio slice (in #failures)",Search::Config::slice),
      _restart("restart","restart sequence type",RM_NONE),
      _r_base("restart-base","base for geometric restart sequence",
              Search::Config::base),
      _r_scale("restart-scale","scale factor for restart sequence",
               Search::Config::slice),
      _nogoods("nogoods","whether to use no-goods from restarts",false),
      _nogoods_limit("nogoods-limit","depth limit for no-good extraction",
                     Search::Config::nogoods_limit),
      _relax("relax","probability for relaxing variable", 0.0),
      _interrupt("interrupt","whether to catch Ctrl-C (true) or not (false)",
                 true),

      _mode("mode","how to execute script",SM_SOLUTION),
      _samples("samples","how many samples (time mode)",1),
      _iterations("iterations","iterations per sample (time mode)",1),
      _print_last("print-last",
                  "whether to only print the last solution (solution mode)",
                  false),
      _out_file("file-sol", "where to print solutions "
                "(supports stdout, stdlog, stderr)","stdout"),
      _log_file("file-stat", "where to print statistics "
                "(supports stdout, stdlog, stderr)","stdout"),
      _trace(0)

#ifdef GECODE_HAS_CPPROFILER
      ,
      _profiler_id("cpprofiler-id", "use this execution id with CP-profiler", 0),
      _profiler_port("cpprofiler-port", "connect to CP-profiler on this port",
                     Search::Config::cpprofiler_port),
      _profiler_info("cpprofiler-info", "send solution information to CP-profiler", false) 
#endif
  {

    _mode.add(SM_SOLUTION,   "solution");
    _mode.add(SM_TIME,       "time");
    _mode.add(SM_STAT,       "stat");
    _mode.add(SM_GIST,       "gist");
    _mode.add(SM_CPPROFILER, "cpprofiler");

    _restart.add(RM_NONE,"none");
    _restart.add(RM_CONSTANT,"constant");
    _restart.add(RM_LINEAR,"linear");
    _restart.add(RM_LUBY,"luby");
    _restart.add(RM_GEOMETRIC,"geometric");

    add(_model); add(_symmetry); add(_propagation); add(_ipl);
    add(_branching); add(_decay); add(_seed); add(_step);
    add(_search); add(_solutions); add(_threads); add(_c_d); add(_a_d);
    add(_d_l);
    add(_node); add(_fail); add(_time); add(_interrupt);
    add(_assets); add(_slice);
    add(_restart); add(_r_base); add(_r_scale);
    add(_nogoods); add(_nogoods_limit);
    add(_relax);
    add(_mode); add(_iterations); add(_samples); add(_print_last);
    add(_out_file); add(_log_file); add(_trace);
#ifdef GECODE_HAS_CPPROFILER
    add(_profiler_id);
    add(_profiler_port);
    add(_profiler_info);
#endif
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
    _inst = Driver::BaseOption::strdup(s);
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
