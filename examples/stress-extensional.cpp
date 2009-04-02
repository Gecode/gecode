/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

using namespace Gecode;

/**
 * \brief %Example: Stress extensional propagator
 *
 * Creates a huge DFA specifying that all values are pairwise distinct
 * and use the DFA for propagation.
 *
 * \ingroup ExStress
 */
class StressExtensional : public Script {
protected:
  /// Variables
  IntVarArray x;
public:
  /// Type definition as abbreviation
  typedef Support::DynamicArray<DFA::Transition,Heap> Transitions;
  /// Create all states for permutation of symbols
  void
  permute(int i_state, int i, int n, int p,
          Transitions& t, int& n_t, int& n_state) {
    if (i<n-1) {
      for (int j=n; j--; )
        if (!(p & (1 << j))) {
          t[n_t].i_state = i_state;
          t[n_t].symbol  = j;
          t[n_t].o_state = ++n_state;
          n_t++;
          permute(n_state,i+1,n,p | (1 << j),
                  t,n_t,n_state);
        }
    } else {
      for (int j=n; j--; )
        if (!(p & (1 << j))) {
          t[n_t].i_state = i_state;
          t[n_t].symbol  = j;
          t[n_t].o_state = 1;
          n_t++;
        }
    }
  }
  /// Construct a distinct %DFA for \a n values
  DFA
  construct(int n) {
    Transitions t(heap);
    int n_t     = 0;
    int n_state = 1;
    permute(0,0,n,0,t,n_t,n_state);
    t[n_t].i_state = -1;
    int f[] = {1,-1};
    DFA d(0,t,f);
    return d;
  }
  /// The actual problem
  StressExtensional(const SizeOptions& opt)
    : x(*this,4*opt.size(),0,opt.size()) {
    DFA a = construct(opt.size());
    IntVarArgs y(opt.size());
    for (unsigned int i=0; i <= 3*opt.size(); i++) {
      for (int j = opt.size(); j--; )
        y[j] = x[i+j];
      extensional(*this, y, a);
    }
    for (int j=opt.size(); j--; )
      y[j] = x[j];
    branch(*this, y, INT_VAR_NONE, INT_VAL_MED);
  }
  /// Constructor for cloning \a s
  StressExtensional(bool share, StressExtensional& s) : Script(share,s) {
    x.update(*this, share, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressExtensional(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tx[" << x.size() << "] = " << x << std::endl;
  }
};

/** \brief Main-function
 *  \relates StressExtensional
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("StressExtensional");
  opt.size(7);
  opt.parse(argc,argv);
  Script::run<StressExtensional,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
