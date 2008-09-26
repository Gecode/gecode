/*
 * Example sat.cpp
 * Author: Raphael Reischuk (2008)
 * Read clauses from a dimacs file and post clause propagators.
 */

#include "examples/support.hh"
#include "gecode/minimodel.hh"
// #include "gecode/int/bool/clause.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

const bool USE_CLAUSE_PROPAGATOR = true;
const int READ_CLAUSES_FROM_FILE = 0;

const std::string defaultfile = "../dimacs/dimacs.cnf";

class SatOptions : public Options {
public:
  std::string filename;
  SatOptions(const char* s, std::string f) 
    : Options(s), filename(f) {}
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    // Parse regular options
    Options::parse(argc,argv);
    // No filename given
    if (argc < 2)
      return;
    // Filename given, should be at position 1
    filename = argv[1];
    argc--;
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(string) default: " << filename << std::endl
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
    // BoolVarArray for setting up branching
    BoolVarArray BA;
    std::string dimacsfile;

public:
  Sat(const SatOptions& opt) {
    dimacsfile = opt.filename;
    parseDIMACS(opt.filename.c_str()); 
    branch(*this, BA, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }
    

  // Constructor for Cloning
  Sat(bool share, Sat& s) : Example(share,s) {
    dimacsfile = s.dimacsfile;
    BA.update(*this, share, s.BA);
  }

  // Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Sat(share,*this);
  }

  // Print solution
  virtual void
  print(std::ostream& os) {
    os << "\tsolution: ";
    for (int i = 0; i < BA.size(); i++)
      os << BA[i].val();
    os << std::endl;
    checkDIMACS(dimacsfile.c_str());
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
        if (READ_CLAUSES_FROM_FILE > 0 && c > READ_CLAUSES_FROM_FILE) {
          std::getline(dimacs,line);
          continue;
        }
        // printf("%8d. clause:   %s\n",c,line.c_str());
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
        // Int::SharedBool::SharedClause::post(*this,positives,negatives);
        delete pos;
        delete neg;
      } 
      else {
        std::cerr << "format error in dimacs file" << std::endl;
        std::cerr << "line: '" << line << "'" << std::endl;
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

  bool
  checkDIMACS(const char* f) {
    int clauses = 0;
    std::ifstream dimacs(f);
    if (!dimacs) {
      std::cerr << "error: file '" << f << "' not found" << std::endl;
      exit(1);
    }
    std::string line;
    int c = 0;
    bool leave;
    while (dimacs.good()) {
      leave = false;
      std::getline(dimacs,line);
      if (line[0] == 'c' || line == "") {
        // Comments (ignore them)
      }
      else if (line[0] == 'p' && line[1] == ' ' &&
               line[2] == 'c' && line[3] == 'n' &&
               line[4] == 'f' && line[5] == ' ') {
        // Line has format "p cnf <variables> <clauses>"
        int i = 6;
        while (line[i] >= '0' && line[i] <= '9') {
          i++;
        }
        i++;
        while (line[i] >= '0' && line[i] <= '9') {
          clauses = 10*clauses + line[i] - '0';
          i++;
        } 
      }
      else if ((line[0] >= '0' && line[0] <= '9') || line[0] == '-' || line[0] == ' ') {
        // Parse regular clause
        c++;
        if (READ_CLAUSES_FROM_FILE > 0 && c > READ_CLAUSES_FROM_FILE) {
          continue;
        }
#ifdef CL_VERBOSE
        // printf("%8d. clause verifying:   %s\n",c,line.c_str());
#endif
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

        // Verify clause (positives)
        Li* tmp = pos->next();
        i = 0;
        while (tmp) {
          // printf ("pos %d: %s\n",tmp->n(),BA[tmp->n()].val() == 1 ? "yes" : "no");
          if (BA[tmp->n()].val() == 1) {
            leave = true;
            break;
          }
          tmp = tmp->next();
        }
        if (leave) {
          leave = false;
          delete pos;
          delete neg;
          continue;
        }

        // Verify clause (negatives)
        tmp = neg->next();
        i = 0;
        while (tmp) {
          // printf ("neg %d: %s\n",tmp->n(),BA[tmp->n()].val() == 0 ? "yes" : "no");
          if (BA[tmp->n()].val() == 0) {
            leave = true;
            break;
          }
          tmp = tmp->next();
        }
        if (leave) {
          leave = false;
          delete pos;
          delete neg;
          continue;
        }
        else {
          delete pos;
          delete neg;
          printf("error verifying %d. clause\n",c);
          return false;
        }
      }
      else {
        std::cerr << "format error in dimacs file" << std::endl;
        std::cerr << "line: " << line << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
    dimacs.close();
    printf("\nVerified %d/%d clauses: ok\n",c,clauses);
    return true;
  }

};


// main function
int main(int argc, char* argv[]) {
  
  SatOptions opt("SAT",defaultfile);
  opt.parse(argc,argv);
  
  if (argc > 1) {
    fprintf(stderr,"Could not parse all arguments.\n");
    opt.help();
    exit(1);
  }

  Example::run<Sat,DFS,SatOptions>(opt);
  return 0;
}


