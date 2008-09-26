/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Raphael Reischuk <raphael@ps.uni-sb.de>
 *
 *  Copyright:
 *     Raphael Reischuk, 2008
 *
 *  Last modified:
 *     $Date: 2008-09-03 14:14:11 +0200 (Mi, 03 Sep 2008) $ by $Author: tack $
 *     $Revision: 7787 $
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

#include "examples/support.hh"
#include "gecode/minimodel.hh"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

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
 *    p cnf <variables> <clauses>
 *
 * Each of the subsequent lines specifies a clause.
 * A positive literal is denoted by the a positive
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
 * \ingroup ExProblem
 */


class SatOptions : public Options {
public:
  std::string filename;
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


class Li {
private:
  int _n;
  Li* _next;
public:
  Li() {_next = NULL; _n = 0;}
  Li(Li* l, int n) {_next = l; _n = n;}
  void add(int n) {_next = new Li(_next, n); _n++;} 
  Li* next() {return _next;}
  int n(void) {return _n;}
  int size(void) {return _n;}
  ~Li() {delete _next;}
};


class Sat : public Example {
private:
    /// BoolVarArray \a BA for branching
    BoolVarArray BA;

public:
  /// The actual problem
  Sat(const SatOptions& opt) {
    parseDIMACS(opt.filename.c_str()); 
    branch(*this, BA, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }

  /// Constructor for Cloning
  Sat(bool share, Sat& s) : Example(share,s) {
    BA.update(*this, share, s.BA);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Sat(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\tsolution: ";
    for (int i = 0; i < BA.size(); i++)
      os << BA[i].val();
    os << std::endl;
  }


  forceinline void 
  parseDIMACS(const char* f) {
    int variables = 0;
    int clauses = 0;
    std::ifstream dimacs(f);
    if (!dimacs) {
      std::cerr << "error: file '" << f << "' not found" << std::endl;
      exit(1);
    }
    printf("+---------------------------------------------------+\n");
    std::cout << "| Parsing file '" << f << "'" << std::endl;
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
        printf("+---------------------------------------------------|\n");
        printf("|%8d variables                                 |\n",variables);
        printf("|%8d clauses                                   |\n",clauses);
        printf("+---------------------------------------------------+\n");
        // Add variables to Array (for branching)
        BA = BoolVarArray(*this, 0);
        BoolVar bv;
        for (int i=0; i<variables; i++) {
          bv = BoolVar(*this, 0, 1);
          BA.add(*this, bv);
        }
      }
      // Parse regular clause
      else if (variables > 0 && 
      ((line[0] >= '0' && line[0] <= '9') || line[0] == '-' || line[0] == ' ')) {
        c++;
        Li* pos = new Li();
        Li* neg = new Li();
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
              pos->add(value-1);
            else
              neg->add(value-1);
            i++;
          }
        }
        
        // Create positive BoolVarArgs
        BoolVarArgs positives(pos->size());
        Li* tmp = pos->next();
        i = 0;
        while (tmp) {
          positives[i++] = BA[tmp->n()];
          tmp = tmp->next();
        }

        // Create negative BoolVarArgs
        BoolVarArgs negatives(neg->size());
        tmp = neg->next();
        i = 0;
        while (tmp) {
          negatives[i++] = BA[tmp->n()];
          tmp = tmp->next();
        }
        
        // Post propagators
        clause(*this, BOT_OR, positives, negatives, 1);
        delete pos;
        delete neg;
      } 
      else {
        std::cerr << "format error in dimacs file" << std::endl;
        std::cerr << "context: '" << line << "'" << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
    dimacs.close();
    if (clauses != c) {
      printf("error: number of specified clauses seems to be wrong.\n");
      exit(1);
    }
    printf("| Parsing done.                                     |\n");
    printf("*---------------------------------------------------*\n");
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
    fprintf(stderr,"Could not parse all arguments.\n");
    opt.help();
    exit(1);
  }
  
  // Run SAT solver
  Example::run<Sat,DFS,SatOptions>(opt);
  return 0;
}


