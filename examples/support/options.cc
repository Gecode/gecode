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

#include "examples/support.hh"

void 
StringOption::add(int v, const char* o, const char* h) {
  Value* n = new Value;
  n->val  = v;
  n->opt  = o;
  n->help = h;
  n->next = NULL;
  if (fst == NULL) {
    fst = lst = n;
  } else {
    lst->next = n; lst = n;
  }
}

bool
StringOption::parse(int argc, char* argv[], int& i) {
  using namespace std;
  if (strcmp(argv[i],opt))
    return false;

  if (++i == argc) {
    cerr << "Missing argument for option \"" << opt << "\"" << endl;
    exit(EXIT_FAILURE);
  }

  for (Value* v = fst; v != NULL; v = v->next)
    if (!strcmp(argv[i],v->opt)) {
      cur = v->val; i++;
      return true;
    }

  cerr << "Wrong argument \"" << argv[i] 
       << "\" for option \"" << opt << "\"" 
       << endl;
  exit(EXIT_FAILURE);
}

void 
StringOption::help(void) {
  using namespace std;
  if (fst == NULL)
    return;
  cerr << '\t' << opt << " (";
  const char* d = NULL;
  for (Value* v = fst; v != NULL; v = v->next) {
    cerr << v->opt << ((v->next != NULL) ? ", " : "");
    if (v->val == cur)
      d = v->opt;
  }
  cerr << ")";
  if (d != NULL) 
    cerr << " default: " << d;
  cerr << endl << "\t\t" << exp << endl;
  for (Value* v = fst; v != NULL; v = v->next)
    if (v->help != NULL)
      cerr << "\t\t  " << v->opt << ": " << v->help << endl;
}



bool
UIntOption::parse(int argc, char* argv[], int& i) {
  using namespace std;
  if (strcmp(argv[i],opt))
    return false;

  if (++i == argc) {
    cerr << "Missing argument for option \"" << opt << "\"" << endl;
    exit(EXIT_FAILURE);
  }
  cur = atoi(argv[i++]);
  return true;
}

void 
UIntOption::help(void) {
  using namespace std;
  cerr << '\t' << opt << " (unsigned int) default: " << cur << endl
       << "\t\t" << exp << endl;
}



Options::Options(const char* n)
  : 
    fails(-1),
    time(-1),
    naive(false),
    size(0),
#ifdef GECODE_HAVE_CPLTSET_VARS
    initvarnum(100),
    initcache(100),
    scl(SCL_DEF),
#endif
    fst(NULL), lst(NULL),

    _name(n),

    _model("-model","model variants"),
    _propagation("-propagation","propagation variants"),
    _icl("-icl","integer consistency level",ICL_DEF),
    _branching("-branching","branching variants"),
    
    _search("-search","search engine variants"),
    _solutions("-solutions","number of solutions (0 = all)",1),
    _c_d("-c_d","recompution copy distance",Search::Config::c_d),
    _a_d("-a_d","recompution adaption distance",Search::Config::a_d),

    _mode("-mode","how to execute example",EM_SOLUTION),
    _iterations("-iterations","iterations per sample (time mode)"),
    _samples("-samples","how many samples (time mode)")

{

  _icl.add(ICL_DEF, "def"); _icl.add(ICL_VAL, "val");
  _icl.add(ICL_BND, "bnd"); _icl.add(ICL_DOM, "dom");

  _mode.add(EM_SOLUTION, "solution"); 
  _mode.add(EM_TIME, "time");
  _mode.add(EM_STAT, "stat");

  add(&_model); add(&_propagation); add(&_icl); add(&_branching); 

  add(&_search); add(&_solutions); add(&_c_d); add(&_a_d);

  add(&_mode); add(&_iterations); add(&_samples);

}

namespace {

#ifdef GECODE_HAVE_CPLTSET_VARS
  const char* scl2str[] =
    { "bnd_bdd", "bnd_sbr", "spl", "crd", "lex", "dom", "def" };
#endif

}

void
Options::parse(int argc, char* argv[]) {
  using namespace std;
  int i = 1;
  const char* e = NULL;
  while (i < argc) {
    if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help")) {
      cerr << "Options for " << name() << ":" << endl;
      for (BaseOption* o = fst; o != NULL; o = o->next)
        o->help();
      cerr 
#ifdef GECODE_HAVE_CPLTSET_VARS
	   << "\t-scl (def,bnd_bdd, bnd_sbr,,spl,crd,lex,dom) default: " << scl2str[scl]
	   << endl
	   << "\t\tbdd consistency level" << endl
	   << "\t-ivn (unsigned int) default: " << initvarnum
	   << endl
	   << "\t\tinitial number of bdd nodes in the table" << endl
	   << "\t-ics (unsigned int) default: " << initcache
	   << endl
	   << "\t\tinitial cachesize for bdd operations" << endl
#endif

           << "\t-fails (unsigned int) default: "
           << (fails<0 ? "(no limit) " : "") << fails << endl
           << "\t\tset number of fails before stopping (solution-mode)"
           << endl

           << "\t-time (unsigned int) default: "
           << (time<0 ? "(no limit) " : "") << time << endl
           << "\t\tset time before stopping (solution-mode)" << endl

           << "\t(unsigned int) default: " << size << endl
           << "\t\twhich version/size for example" << endl;
      exit(EXIT_SUCCESS);
    }
  redo:
    for (BaseOption* o = fst; o != NULL; o = o->next)
      if ((i < argc) && o->parse(argc,argv,i))
        goto redo;
    if (i >= argc)
      break;
#ifdef GECODE_HAVE_CPLTSET_VARS
    else if (!strcmp(argv[i],"-scl")) {
      if (++i == argc) goto missing;
      if (!strcmp(argv[i],"def")) {
	scl = SCL_DEF;
      } else if (!strcmp(argv[i],"bnd_bdd")) {
	scl = SCL_BND_BDD;
      } else if (!strcmp(argv[i],"bnd_sbr")) {
	scl = SCL_BND_SBR;
      } else if (!strcmp(argv[i],"spl")) {
	scl = SCL_SPL;
      } else if (!strcmp(argv[i],"crd")) {
	scl = SCL_CRD;
      } else if (!strcmp(argv[i],"lex")) {
	scl = SCL_LEX;
      } else if (!strcmp(argv[i],"dom")) {
	scl = SCL_DOM;
      } else {
	e = "expecting: def, bnd_bdd, bnd_sbr, spl, crd, lex or dom";
	goto error;
      }
    } else if (!strcmp(argv[i],"-ivn")) {
      if (++i == argc) goto missing;
      initvarnum = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-ics")) {
      if (++i == argc) goto missing;
      initcache = atoi(argv[i]);
    }
#endif 
    if (!strcmp(argv[i],"-fails")) {
      if (++i == argc) goto missing;
      fails = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-time")) {
      if (++i == argc) goto missing;
      time = atoi(argv[i]);
    } else {
      char* unused;
      size = strtol(argv[i], &unused, 10);
      if ('\0' != *unused) {
        i++;
        goto error;
      }
    }
    i++;
  }
  return;
 missing:
  e = "missing argument";
 error:
  cerr << "Erroneous argument (" << argv[i-1] << ")" << endl;
  if (e) cerr << e << endl;
  exit(EXIT_FAILURE);
}

// STATISTICS: example-any
