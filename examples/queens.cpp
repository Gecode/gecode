/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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
#include <gecode/minimodel.hh>

/**
 * \brief %Example: n-%Queens puzzle
 *
 * Place n queens on an n times n chessboard such that they do not
 * attack each other.
 *
 * \ingroup ExProblem
 *
 */
class Queens : public Example {
protected:
  /// Position of queens on boards
  IntVarArray q;
public:
  /// Propagation to use for model
  enum {
    PROP_BINARY,  ///< Use only binary disequality constraints
    PROP_MIXED,   ///< Use single distinct and binary disequality constraints
    PROP_DISTINCT ///< Use three distinct constraints
  };
  /// The actual problem
  Queens(const SizeOptions& opt)
    : q(*this,opt.size(),0,opt.size()-1) {
    const int n = q.size();
    switch (opt.propagation()) {
    case PROP_BINARY:
      for (int i = 0; i<n; i++)
        for (int j = i+1; j<n; j++) {
          post(*this, q[i] != q[j]);
          post(*this, q[i]+i != q[j]+j);
          post(*this, q[i]-i != q[j]-j);
        }
      break;
    case PROP_MIXED:
      for (int i = 0; i<n; i++)
        for (int j = i+1; j<n; j++) {
          post(*this, q[i]+i != q[j]+j);
          post(*this, q[i]-i != q[j]-j);
        }
      distinct(*this, q, opt.icl());
      break;
    case PROP_DISTINCT:
      {
        IntArgs c(n);
        for (int i = n; i--; ) c[i] = i;
        distinct(*this, c, q, opt.icl());
        for (int i = n; i--; ) c[i] = -i;
        distinct(*this, c, q, opt.icl());
      }
      distinct(*this, q, opt.icl());
      break;
    }
    branch(*this, q, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }

  /// Constructor for cloning \a s
  Queens(bool share, Queens& s) : Example(share,s) {
    q.update(*this, share, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Queens(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\t";
    for (int i = 0; i < q.size(); i++) {
      os << q[i] << ", ";
      if ((i+1) % 10 == 0)
        os << std::endl << "\t";
    }
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates Queens
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Queens");
  opt.iterations(500);
  opt.size(100);
  opt.propagation(Queens::PROP_DISTINCT);
  opt.propagation(Queens::PROP_BINARY, "binary",
                      "only binary disequality constraints");
  opt.propagation(Queens::PROP_MIXED, "mixed",
                      "single distinct and binary disequality constraints");
  opt.propagation(Queens::PROP_DISTINCT, "distinct",
                      "three distinct constraints");
  opt.parse(argc,argv);
  Example::run<Queens,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

