/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Raphael Reischuk <reischuk@cs.uni-sb.de>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Raphael Reischuk, 2008
 *     Guido Tack, 2008
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

#include <gecode/driver.hh>
#include <gecode/int.hh>

#include <fstream>
#include <string>
#include <vector>

using namespace Gecode;

/** \brief Options for %SAT problems
 *
 * \relates SAT
 */
class SatOptions : public Options {
public:
  /// Name of the DIMACS file to parse
  std::string filename;
  /// Initialize options with file name \a s
  SatOptions(const char* s)
    : Options(s) {}
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    // Parse regular options
    Options::parse(argc,argv);
    // Filename, should be at position 1
    if (argc == 1) {
      help();
      exit(1);
    }
    filename = argv[1];
    argc--;
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(string) " << std::endl
              << "\t\tdimacs file to parse (.cnf)" << std::endl;
  }
};

/**
 * \brief %Example: CNF SAT solver
 *
 * SAT finds assignments of Boolean variables such
 * that a set of clauses is satisfied or shows that
 * no such assignment exists.
 *
 * This example parses a dimacs CNF file in which
 * the constraints are specified. For each line of
 * the file a clause propagator is posted.
 *
 * Format of dimacs CNF files:
 *
 * A dimacs file starts with comments (each line
 * starts with c). The number of variables and the
 * number of clauses is defined by the line
 *
 *    p cnf \<variables\> \<clauses\>
 *
 * Each of the subsequent lines specifies a clause.
 * A positive literal is denoted by a positive
 * integer, a negative literal is denoted by the
 * corresponding negative integer. Each line is
 * terminated by 0.
 *
 * c sample CNF file
 * p cnf 3 2
 * 3 -1 0
 * 1 2 -1 0
 *
 * Benchmarks on satlib.org, for instance,
 * are in the dimacs CNF format.
 *
 * \ingroup Example
 */
class Sat : public Script {
private:
  /// The Boolean variables
  BoolVarArray x;
public:
  /// The actual problem
  Sat(const SatOptions& opt) {
    parseDIMACS(opt.filename.c_str());
    branch(*this, x, INT_VAR_NONE, INT_VAL_MIN);
  }

  /// Constructor for cloning
  Sat(bool share, Sat& s) : Script(share,s) {
    x.update(*this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Sat(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "solution:\n" << x << std::endl;
  }

  /// Post constraints according to DIMACS file \a f
  void parseDIMACS(const char* f) {
    int variables = 0;
    int clauses = 0;
    std::ifstream dimacs(f);
    if (!dimacs) {
      std::cerr << "error: file '" << f << "' not found" << std::endl;
      exit(1);
    }
    std::cout << "Solving problem from DIMACS file '" << f << "'"
              << std::endl;
    std::string line;
    int c = 0;
    while (dimacs.good()) {
      std::getline(dimacs,line);
      // Comments (ignore them)
      if (line[0] == 'c' || line == "") {
      }
      // Line has format "p cnf <variables> <clauses>"
      else if (variables == 0 && clauses == 0 &&
               line[0] == 'p' && line[1] == ' ' &&
               line[2] == 'c' && line[3] == 'n' &&
               line[4] == 'f' && line[5] == ' ') {
        int i = 6;
        while (line[i] >= '0' && line[i] <= '9') {
          variables = 10*variables + line[i] - '0';
          i++;
        }
        i++;
        while (line[i] >= '0' && line[i] <= '9') {
          clauses = 10*clauses + line[i] - '0';
          i++;
        }
        std::cout << "(" << variables << " variables, "
                  << clauses << " clauses)" << std::endl << std::endl;
        // Add variables to array
        x = BoolVarArray(*this, variables, 0, 1);
      }
      // Parse regular clause
      else if (variables > 0 &&
      ((line[0] >= '0' && line[0] <= '9') || line[0] == '-' || line[0] == ' ')) {
        c++;
        std::vector<int> pos;
        std::vector<int> neg;
        int i = 0;
        while (line[i] != 0) {
          if (line[i] == ' ') {
            i++;
            continue;
          }
          bool positive = true;
          if (line[i] == '-') {
            positive = false;
            i++;
          }
          int value = 0;
          while (line[i] >= '0' && line[i] <= '9') {
            value = 10 * value + line[i] - '0';
            i++;
          }
          if (value != 0) {
            if (positive)
              pos.push_back(value-1);
            else
              neg.push_back(value-1);
            i++;
          }
        }

        // Create positive BoolVarArgs
        BoolVarArgs positives(pos.size());
        for (int i=pos.size(); i--;)
          positives[i] = x[pos[i]];

        BoolVarArgs negatives(neg.size());
        for (int i=neg.size(); i--;)
          negatives[i] = x[neg[i]];

        // Post propagators
        clause(*this, BOT_OR, positives, negatives, 1);
      }
      else {
        std::cerr << "format error in dimacs file" << std::endl;
        std::cerr << "context: '" << line << "'" << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
    dimacs.close();
    if (clauses != c) {
      std::cerr << "error: number of specified clauses seems to be wrong."
                << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
};


/** \brief Main-function
 *  \relates SAT
 */
int main(int argc, char* argv[]) {

  SatOptions opt("SAT");
  opt.parse(argc,argv);

  // Check whether all arguments are successfully parsed
  if (argc > 1) {
    std::cerr << "Could not parse all arguments." << std::endl;
    opt.help();
    std::exit(EXIT_FAILURE);
  }

  // Run SAT solver
  Script::run<Sat,DFS,SatOptions>(opt);
  return 0;
}

// STATISTICS: example-any
