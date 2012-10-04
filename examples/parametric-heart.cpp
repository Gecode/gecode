/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
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
#include <gecode/minimodel.hh>
#include <gecode/float.hh>
#include <gecode/float/linear.hh>

#include <iostream>

using namespace Gecode;

class ParametricHeart : public Script {
  FloatVarArray f;
public:
  double step;
  static FloatNum sMinValue;
  ParametricHeart(const Options& ) : f(*this,5,-20,20), step(0.1)
  {
	int q = 2;
	double p = 0.5;
	rel(*this, sqr(f[1]) + 2*sqr(f[2]-p*nroot(abs(f[1]),q)) == 1);
	rel(*this, f[0] == f[1]);
	step = 0.01;
    branch(*this,f[0],FLOAT_VAL_SPLIT_MIN());
	branch(*this,f[2],FLOAT_VAL_SPLIT_MIN());
  }

  ParametricHeart(bool share, ParametricHeart& p) : Script(share,p)
  {
    f.update(*this,share,p.f);
    step = p.step;
  }

  virtual Space* copy(bool share) { return new ParametricHeart(share,*this); }

  virtual void constrain(const Space& _b) {
    const ParametricHeart& b = static_cast<const ParametricHeart&>(_b);
	rel(*this, (f[0] >= (b.f[0].max()+step)) || (f[2] >= (b.f[2].max()+step))
											 || (f[2] <= (b.f[2].min()-step)));
  }

  virtual void
  print(std::ostream& os) const
  {
    os << "XY " << f[1].med() << " " << f[2].med();
    os << std::endl;
  }

};

double ParametricHeart::sMinValue = -20;

int main(int argc, char* argv[])
{
  Options opt("Coordinates of solutions of a parametric heart equation.");
  opt.parse(argc,argv);
  opt.solutions(0);
  Script::run<ParametricHeart,BAB,Options>(opt);
  return 0;
}

// STATISTICS: example-any
