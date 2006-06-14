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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "examples/support.hh"

/*
 * Parses options for examples
 *
 */

static const char* icl2str[] =
  { "val", "bnd", "dom", "def" };

static const char* em2str[] =
  { "solution", "time", "stat" };

static const char* bool2str[] = 
  { "false", "true" };

void
Options::parse(int argc, char** argv) {
  using namespace std;
  int i = 1;
  const char* e = NULL;
  while (i < argc) {
    if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help")) {
      cerr << "Options for example " << name << ":" 
	   << endl
	   << "\t-icl (def,val,bnd,dom) default: " << icl2str[icl] 
	   << endl
	   << "\t\tinteger consistency level" << endl
	   << "\t-c_d (unsigned int) default: " << c_d << endl
	   << "\t\tcopying recomputation distance" << endl
	   << "\t-a_d (unsigned int) default: " << a_d << endl
	   << "\t\tadaption recomputation distance" << endl
	   << "\t-mode (solution, time, stat) default: "
	   << em2str[mode] << endl
	   << "\t\twhether to print solutions, measure time, "
	   << "or print statistics" << endl
	   << "\t-samples (unsigned int) default: " 
	   << samples << endl
	   << "\t\thow many samples (time-mode)" << endl
	   << "\t-iterations (unsigned int) default: " 
	   << iterations << endl
	   << "\t\thow many iterations per sample (time-mode)" << endl
	   << "\t-solutions (unsigned int) default: ";
      if (solutions == 0)
	cerr << "all (0)";
      else
	cerr << solutions;
      cerr << endl
	   << "\t\thow many solutions to search (solution-mode)" << endl
	   << "\t-naive default: " 
	   << bool2str[naive] << endl
	   << "\t\tuse naive version" << endl
	   << "\t-smart default: "
	   << bool2str[!naive] << endl
	   << "\t\tuse smart version" << endl
	   << "\t(unsigned int) default: " << size << endl
	   << "\t\twhich version/size for example" << endl;
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
    } else if (!strcmp(argv[i],"-c_d")) {
      if (++i == argc) goto missing;
      c_d = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-a_d")) {
      if (++i == argc) goto missing;
      a_d = atoi(argv[i]);
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
    } else if (!strcmp(argv[i],"-iterations")) {
      if (++i == argc) goto missing;
      iterations = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-naive")) {
      naive = true;
    } else if (!strcmp(argv[i],"-smart")) {
      naive = false;
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
  cerr << "Erroneous argument (" << argv[i-1] << ")" << endl
       << e << endl;
  exit(EXIT_FAILURE);
}

// STATISTICS: example-any

