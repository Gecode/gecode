/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
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

using namespace Gecode;

/**
 * \brief %Example: Golden spiral
 *
 * The Golden Spiral is a logarithmic spiral whose growth factor
 * is the golden ratio \f$\phi=1,618\f$.
 *
 * It is defined by the polar equation:
 * \f[
 * r = ae^{b\theta}
 * \f]
 * where
 * \f[
 * \operatorname{abs}(b) = \frac{\operatorname{ln}(\phi)}{\frac{\pi}{2}}
 * \f]
 *
 * To get cartesian coordinates, it can be solved for \f$x\f$
 * and \f$y\f$ in terms of \f$r\f$ and \f$\theta\f$.
 * By setting \f$a=1\f$, it yields to the equation:
 *
 * \f[
 * r = e^{0.30649\times\theta}
 * \f]
 * with
 * \f[
 * x=r\operatorname{cos}(\theta), \quad y=r\operatorname{sin}(\theta)
 * \f]
 *
 * The tuple \f$(r,\theta)\f$ is related to the position for
 * \f$x\f$ and \f$y\f$ on the curve.  \f$r\f$ and \f$\theta\f$
 * are positive numbers.
 *
 * To get reasonable interval starting sizes, \f$x\f$ and \f$y\f$
 * are restricted to \f$[-20;20]\f$.
 *
 * \ingroup Example
 */
class GoldenSpiral : public FloatMaximizeScript {
protected:
  /// The numbers
  FloatVarArray f;
public:
  /// Actual model
  GoldenSpiral(const Options& opt)
    : FloatMaximizeScript(opt), f(*this,4,-20,20) {
    // Post equation
    FloatVar theta = f[0];
    FloatVar r = f[3];
    FloatVar x = f[1];
    FloatVar y = f[2];
    rel(*this, theta >= 0);
    rel(*this, r >= 0);
    rel(*this, r*cos(theta) == x);
    rel(*this, r*sin(theta) == y);
    rel(*this, exp(0.30649*theta) == r);

    branch(*this,theta,FLOAT_VAL_SPLIT_MIN());
  }
  /// Constructor for cloning \a p
  GoldenSpiral(GoldenSpiral& p)
    : FloatMaximizeScript(p) {
    f.update(*this, p.f);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new GoldenSpiral(*this);
  }
  /// Cost function
  virtual FloatVar cost(void) const {
    return f[0];
  }
  /// Print solution coordinates
  virtual void print(std::ostream& os) const {
    os << "XY " << f[1].med() << " " << f[2].med()
       << std::endl;
  }

};

/** \brief Main-function
 *  \relates GoldenSpiral
 */
int main(int argc, char* argv[]) {
  Options opt("GoldenSpiral");
  opt.solutions(0);
  opt.step(0.1);
  opt.parse(argc,argv);
  FloatMaximizeScript::run<GoldenSpiral,BAB,Options>(opt);
  return 0;
}

// STATISTICS: example-any
