/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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


/**
 * \brief %Example: Stress linear Boolean inequality
 *
 * \ingroup ExStress
 */
class StressLinearBool : public Example {
protected:
  /// Variables
  BoolVarArray x;
public:
  /// Constructor for the model
  StressLinearBool(const SizeOptions& opt)
    : x(*this,4*opt.size()+1,0,1) {

    linear(*this, x, IRT_GQ, 2*opt.size());

    for (unsigned int i=0; i<opt.size(); i++) {
      // Assign a variable
      rel(*this, x[opt.size()-1-i], IRT_EQ, 0);
      // Propagate
      (void) status();
      // Assign a variable
      rel(*this, x[opt.size()+i], IRT_EQ, 0);
      // Propagate
      (void) status();
    }

  }
  /// Constructor for cloning \a s
  StressLinearBool(bool share, StressLinearBool& s) : Example(share,s) {
    x.update(*this, share, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressLinearBool(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tx[" << x.size() << "] = " << x << std::endl;
  }
};

/** \brief Main-function
 *  \relates StressLinearBool
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("StressLinearBool");
  opt.size(1000);
  opt.parse(argc,argv);
  Example::run<StressLinearBool,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
