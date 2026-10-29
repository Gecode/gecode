/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Raphael Reischuk <reischuk@cs.uni-sb.de>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Raphael Reischuk, 2008
 *     Guido Tack, 2008
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <string>

using namespace Gecode;

namespace {

  /**
   * \brief Reader spec for DIMACS CNF files
   *
   * See Instance::Spec for the spec language.
   *
   * \relates Sat
   */
  const char* instance_spec = R"(format dimacs-cnf

comment "c"
stop "%"                    # SATLIB files end with "%" and "0"
line "p cnf {vars:0..gecode.int_max} {clauses:0..}"
repeat clauses {
  list clause: -vars..vars until 0   # the terminating 0 is not stored
}
)";

}

/**
 * \brief %Example: CNF SAT solver
 *
 * SAT finds assignments of Boolean variables such
 * that a set of clauses is satisfied or shows that
 * no such assignment exists.
 *
 * The clauses are read from a DIMACS CNF file with a
 * reader spec, and a clause propagator is posted for each
 * of them.
 *
 * Format of dimacs CNF files:
 *
 * A dimacs file starts with comments (each line
 * starts with c). The number of variables and the
 * number of clauses is defined by the line
 *
 *    p cnf \<variables\> \<clauses\>
 *
 * Then follow the clauses. A positive literal is denoted
 * by a positive integer, a negative literal is denoted by
 * the corresponding negative integer. Each clause is
 * terminated by 0, and clauses may span several lines.
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
  Sat(const InstanceOptions& opt)
    : Script(opt) {
    Instance::Data d = opt.data();
    x = BoolVarArray(*this, static_cast<int>(d.integer("vars")), 0, 1);
    for (int c=0; c<d.size("clause"); c++) {
      IntArgs literals = d.ints("clause", {c});
      BoolVarArgs positives, negatives;
      for (int l : literals)
        if (l > 0)
          positives << x[l-1];
        else
          negatives << x[-l-1];
      clause(*this, BOT_OR, positives, negatives, 1);
    }
    branch(*this, x, BOOL_VAR_AFC_MAX(), BOOL_VAL_MIN());
  }

  /// Constructor for cloning
  Sat(Sat& s) : Script(s) {
    x.update(*this, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new Sat(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "solution:\n" << x << std::endl;
  }
};


/** \brief Main-function
 *  \relates SAT
 */
int main(int argc, char* argv[]) {

  InstanceOptions opt("SAT", instance_spec);
  opt.parse(argc,argv);
  if (argc > 2) {
    std::cerr << "Error: unexpected arguments after the DIMACS file" << std::endl;
    return 1;
  }

  if (!opt.is_file()) {
    if (opt.instance() == nullptr)
      std::cerr << "Error: no DIMACS file given" << std::endl;
    else
      std::cerr << "Error: cannot read file '" << opt.instance() << "'" << std::endl;
    return 1;
  }
  try {
    // Read here to report errors before solving starts
    (void) opt.data();
  } catch (const Instance::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // Run SAT solver
  Script::run<Sat,DFS,InstanceOptions>(opt);
  return 0;
}

// STATISTICS: example-any
