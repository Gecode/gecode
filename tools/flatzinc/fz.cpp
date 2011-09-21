/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#include <iostream>
#include <fstream>
#include <gecode/flatzinc.hh>

using namespace std;
using namespace Gecode;

int main(int argc, char** argv) {
  
  Support::Timer t_total;
  t_total.start();
  FlatZinc::FlatZincOptions opt("Gecode/FlatZinc");
  opt.parse(argc, argv);
  
  if (argc!=2) {
    cerr << "Usage: " << argv[0] << " [options] <file>" << endl;
    cerr << "       " << argv[0] << " -help for more information" << endl;
    exit(EXIT_FAILURE);
  }
  
  const char* filename = argv[1];
  opt.name(filename);
  
  FlatZinc::Printer p;
  FlatZinc::FlatZincSpace* fg = NULL;
  if (!strcmp(filename, "-")) {
    fg = FlatZinc::parse(cin, p);
  } else {
    fg = FlatZinc::parse(filename, p);
  }

  if (fg) {
    
    fg->createBranchers(fg->solveAnnotations(), opt.seed(), false, std::cerr);
    fg->shrinkArrays(p);
    if (opt.output()) {
      std::ofstream os(opt.output());
      if (!os.good()) {
        std::cerr << "Could not open file " << opt.output() << " for output."
                  << std::endl;
        exit(EXIT_FAILURE);
      }      
      fg->run(os, p, opt, t_total);
      os.close();
    } else {
      fg->run(std::cout, p, opt, t_total);
    }
  } else {
    exit(EXIT_FAILURE);    
  }
  delete fg;
  
  return 0;
}

// STATISTICS: flatzinc-any
