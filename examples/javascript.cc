/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
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

#include "examples/support.hh"
#include "gecode/serialization.hh"
#include <fstream>
#include <string>

/**
 * \brief Options for examples with additional file name parameter
 *
 */
class FileNameOptions : public Options {
protected:
  const char* _file; ///< File name value
public:
  /// Initialize options for example with name \a n
  FileNameOptions(const char* n);
  /// Print help text
  virtual void help(void);
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]);

  /// Set file name
  void file(const char* f);
  /// Return file name
  const char* file(void) const;
};

inline void
FileNameOptions::file(const char* f) { _file = f; }

inline const char*
FileNameOptions::file(void) const {
  return _file;
}

FileNameOptions::FileNameOptions(const char* n) 
  : Options(n), _file(NULL) {}

void
FileNameOptions::help(void) {
  Options::help();
  std::cerr << "\t(string)" << std::endl
            << "\t\tJavaScript source file to parse" << std::endl;
}

void
FileNameOptions::parse(int& argc, char* argv[]) {
  Options::parse(argc,argv);
  if (argc < 2)
    return;
  file(argv[1]);
}

/**
 * \brief %Example: Gecode/JavaScript interpreter
 *
 * Executes a JavaScript program, builds the Gecode model that it contains and
 * runs a search.
 *
 * \ingroup ExProblem
 *
 */
class JavaScript : public Example {
protected:
  /// Variables
  VarArray<Reflection::Var> x;
public:
  /// The actual problem
  JavaScript(const FileNameOptions& opt) : x(this, 0) {
    if (opt.file() == NULL) {
      throw Exception("JavaScript", "no file given");
    }
    std::ifstream programFile(opt.file());
    std::string program;
    if (programFile.fail()) {
      throw Exception("JavaScript", "error reading file");      
    }
    while (!programFile.eof()) {
      std::string line;
      std::getline(programFile, line);
      program += line+"\n";
    }
    programFile.close();
    fromJavaScript(this, program);
    Reflection::VarMap vm;
    for (Reflection::ActorSpecIter si(this, vm); si(); ++si) {
      (void) si.actor();
    }
    for (Reflection::VarMapIter vmi(vm); vmi(); ++vmi) {
      x.add(this, Reflection::Var(vmi.varImpBase(), vmi.spec().vti()));
    }
  }

  /// Constructor for cloning \a s
  JavaScript(bool share, JavaScript& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new JavaScript(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    for (int i=0; i<x.size(); i++) {
      os << x[i] << (i < x.size() - 1 ? "," : "");
    }
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates Queens
 */
int
main(int argc, char* argv[]) {
  FileNameOptions opt("JavaScript");
  opt.parse(argc,argv);
  Gecode::Serialization::initRegistry();
  Example::run<JavaScript,DFS,FileNameOptions>(opt);
  return 0;
}

// STATISTICS: example-any
