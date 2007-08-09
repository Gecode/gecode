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
OptValue::add(int v, const char* o, const char* h) {
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
OptValue::parse(const char* o) {
  if (fst == NULL)
    return false;
  for (Value* v = fst; v != NULL; v = v->next)
    if (!strcmp(o,v->opt)) {
      cur = v->val;
      return true;
    }
  return false;
}

void 
OptValue::help(const char* o, const char* t) {
  using namespace std;
  if (fst == NULL)
    return;
  cerr << '\t' << o << " (";
  const char* d = NULL;
  for (Value* v = fst; v != NULL; v = v->next) {
    cerr << v->opt << ((v->next != NULL) ? ", " : "");
    if (v->val == cur)
      d = v->opt;
  }
  cerr << ")";
  if (d != NULL) 
    cerr << " default: " << d;
  cerr << endl << "\t\t" << t << endl;
  for (Value* v = fst; v != NULL; v = v->next)
    if (v->help != NULL)
      cerr << "\t\t  " << v->opt << ": " << v->help << endl;
}

namespace {

  const char* icl2str[] =
    { "val", "bnd", "dom", "def" };

  const char* em2str[] =
    { "solution", "time", "stat" };

  const char* bool2str[] =
    { "false", "true" };

#ifdef GECODE_HAVE_CPLTSET_VARS
  const char* scl2str[] =
    { "bnd_bdd", "bnd_sbr", "spl", "crd", "lex", "dom", "def" };
#endif

}

void
Options::parse(int argc, char** argv) {
  using namespace std;
  int i = 1;
  const char* e = NULL;
  while (i < argc) {
    if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help")) {
      cerr << "Options for " << name << ":"
           << endl

           << "\t-icl (def,val,bnd,dom) default: " << icl2str[icl] << endl
           << "\t\tinteger consistency level" << endl
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

           << "\t-c_d (unsigned int) default: " << c_d << endl
           << "\t\tcopying recomputation distance" << endl

           << "\t-a_d (unsigned int) default: " << a_d << endl
           << "\t\tadaptive recomputation distance" << endl

           << "\t-mode (solution, time, stat) default: "
           << em2str[mode] << endl
           << "\t\tprint solutions, measure time, or print statistics" << endl
        
           << "\t-samples (unsigned int) default: " << samples << endl
           << "\t\thow many samples (time-mode)" << endl

           << "\t-iterations (unsigned int) default: " << iterations << endl
           << "\t\thow many iterations per sample (time-mode)" << endl

           << "\t-solutions (unsigned int) default: "
           << ((solutions == 0) ? "all " : "") << solutions << endl
           << "\t\thow many solutions to search (solution-mode)" << endl

           << "\t-fails (unsigned int) default: "
           << (fails<0 ? "(no limit) " : "") << fails << endl
           << "\t\tset number of fails before stopping (solution-mode)"
           << endl

           << "\t-time (unsigned int) default: "
           << (time<0 ? "(no limit) " : "") << time << endl
           << "\t\tset time before stopping (solution-mode)" << endl

           << "\t(unsigned int) default: " << size << endl
           << "\t\twhich version/size for example" << endl;
      model.help("-model","model variants");
      propagation.help("-propagation","propagation variants");
      branching.help("-branching","branching variants");
      exit(EXIT_SUCCESS);
    } else if (!strcmp(argv[i],"-icl")) {
      if (++i == argc) goto missing;
      if (!strcmp(argv[i],"def")) {
        icl = ICL_DEF;
      } else if (!strcmp(argv[i],"val")) {
        icl = ICL_VAL;
      } else if (!strcmp(argv[i],"bnd")) {
        icl = ICL_BND;
      } else if (!strcmp(argv[i],"dom")) {
        icl = ICL_DOM;
      } else {
        e = "expecting: def, val, bnd, or dom";
        goto error;
      }
    } 
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
    else if (!strcmp(argv[i],"-c_d")) {
      if (++i == argc) goto missing;
      c_d = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-a_d")) {
      if (++i == argc) goto missing;
      a_d = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-quiet")) {
      quiet = true;
    } else if (!strcmp(argv[i],"-mode")) {
      if (++i == argc) goto missing;
      if (!strcmp(argv[i],"solution")) {
        mode = EM_SOLUTION;
      } else if (!strcmp(argv[i],"time")) {
        mode = EM_TIME;
      } else if (!strcmp(argv[i],"stat")) {
        mode = EM_STAT;
      } else {
        e = "expecting: solution, time, or stat";
        goto error;
      }
    } else if (!strcmp(argv[i],"-samples")) {
      if (++i == argc) goto missing;
      samples = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-solutions")) {
      if (++i == argc) goto missing;
      solutions = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-fails")) {
      if (++i == argc) goto missing;
      fails = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-time")) {
      if (++i == argc) goto missing;
      time = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-iterations")) {
      if (++i == argc) goto missing;
      iterations = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-model")) {
      if (++i == argc) goto missing;
      if (!model.parse(argv[i])) goto error;
    } else if (!strcmp(argv[i],"-propagation")) {
      if (++i == argc) goto missing;
      if (!propagation.parse(argv[i])) goto error;
    } else if (!strcmp(argv[i],"-branching")) {
      if (++i == argc) goto missing;
      if (!branching.parse(argv[i])) goto error;
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
