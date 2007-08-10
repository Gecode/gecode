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

#include <iostream>
#include <iomanip>

#include <cstdlib>
#include <cstring>

void 
StringOption::add(int v, const char* o, const char* h) {
  Value* n = new Value;
  n->val  = v;
  n->opt  = o;
  n->help = h;
  n->next = NULL;
  if (fst == NULL) {
    fst = n;
  } else {
    lst->next = n; 
  }
  lst = n;
}

bool
StringOption::parse(int argc, char* argv[], int& i) {
  if (strcmp(argv[i],opt))
    return false;
  if (++i == argc) {
    std::cerr << "Missing argument for option \"" << opt << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
  for (Value* v = fst; v != NULL; v = v->next)
    if (!strcmp(argv[i],v->opt)) {
      cur = v->val; i++;
      return true;
    }
  std::cerr << "Wrong argument \"" << argv[i] 
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
    Value* n = v->next;
    delete v;
    v = n;
  }
}


bool
UIntOption::parse(int argc, char* argv[], int& i) {
  if (strcmp(argv[i],opt))
    return false;
  if (++i == argc) {
    std::cerr << "Missing argument for option \"" << opt << "\"" 
              << std::endl;
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
  : fst(NULL), lst(NULL),

    _name(n),

    _model("-model","model variants"),
    _propagation("-propagation","propagation variants"),
    _icl("-icl","integer consistency level",ICL_DEF),
    _branching("-branching","branching variants"),
    
    _search("-search","search engine variants"),
    _solutions("-solutions","number of solutions (0 = all)",1),
    _c_d("-c-d","recompution copy distance",Search::Config::c_d),
    _a_d("-a-d","recompution adaption distance",Search::Config::a_d),
    _fail("-fail","failure cutoff (0 = none, solution mode)"),
    _time("-time","time (in ms) cutoff (0 = none, solution mode)"),

    _mode("-mode","how to execute example",EM_SOLUTION),
    _iterations("-iterations","iterations per sample (time mode)"),
    _samples("-samples","how many samples (time mode)") ,


    naive(false),
    size(0)
{

  _icl.add(ICL_DEF, "def"); _icl.add(ICL_VAL, "val");
  _icl.add(ICL_BND, "bnd"); _icl.add(ICL_DOM, "dom");

  _mode.add(EM_SOLUTION, "solution"); 
  _mode.add(EM_TIME, "time");
  _mode.add(EM_STAT, "stat");

  add(_model); add(_propagation); add(_icl); add(_branching); 
  add(_search); add(_solutions); add(_c_d); add(_a_d); add(_fail); add(_time);
  add(_mode); add(_iterations); add(_samples);
}

void
Options::parse(int argc, char* argv[]) {
  int i = 1;
  while (i < argc) {
    if (!strcmp(argv[i],"-help") || 
        !strcmp(argv[i],"--help") ||
        !strcmp(argv[i],"-?")) {
      std::cerr << "Options for " << name() << ":" << std::endl
                << "\t-help, --help, -?" << std::endl
                << "\t\tprint this help message" << std::endl;
      for (BaseOption* o = fst; o != NULL; o = o->next)
        o->help();
      std::cerr << "\t(unsigned int) default: " << size << std::endl
                << "\t\twhich version/size for example" << std::endl;


      exit(EXIT_SUCCESS);
    }
  redo:
    for (BaseOption* o = fst; o != NULL; o = o->next)
      if ((i < argc) && o->parse(argc,argv,i))
        goto redo;
    if (i >= argc)
      break;
    char* unused;
    size = strtol(argv[i], &unused, 10);
    if ('\0' != *unused) {
      i++;
      goto error;
    }
    i++;
  }
  return;
 error:
  std::cerr << "Erroneous argument (" << argv[i-1] << ")" << std::endl;
  exit(EXIT_FAILURE);
}

// STATISTICS: example-any
