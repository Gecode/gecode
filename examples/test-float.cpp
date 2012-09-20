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
#include <gecode/float/arithmetic.hh>
#include <gecode/float/transcendental.hh>
#include <gecode/float/trigonometric.hh>

#include <iostream>

//#define BOXES

using namespace Gecode;

class TestFloat : public Script {
  IntVar ivar;
  BoolVar bvar;
  FloatVarArray f;
public:
  double step;
  static FloatNum sMinValue;
  TestFloat(const Options& ) : ivar(*this,-5,5), bvar(*this,0,1), f(*this,5,-20,20), step(0.1)
  {
#ifndef BOXES
    branch(*this,f[0],FLOAT_VAL_SPLIT_MIN());
    //branch(*this,f[0],FLOAT_VAL_SPLIT_RND(),ValBranchOptions::time());
#endif

//    Int::IntView iv(ivar);
//    Float::FloatView x0(f[0]);
//    Float::MinusView mx0(f[0]);
//    Float::FloatView x1(f[1]);
//    Float::OffsetView o1(f[1],2);
//    Float::MinusView mx1(f[1]);
//    Float::FloatView x2(f[2]);
//    Float::MinusView mx2(f[2]);
//    Float::FloatView x3(f[3]);
//    Float::MinusView mx3(f[3]);
//    Float::FloatView x4(f[4]);
//    Float::MinusView mx4(f[4]);
//    Float::FloatView x5(f[5]);
//    Float::MinusView mx5(f[5]);
//    Float::Linear::LqBin<Float::FloatView,Float::MinusView>::post(*this,x0,mx1,0);
//    Float::Linear::GqBin<Float::FloatView,Float::MinusView>::post(*this,x0,mx1,0);
//    Float::Linear::EqBin<Float::FloatView,Float::MinusView>::post(*this,x0,mx1,0);
//    Float::Arithmetic::Mult<Float::FloatView>::post(*this,x0,x0,x1);
//    Float::Arithmetic::Div<Float::FloatView,Float::FloatView,Float::FloatView>::post(*this,x2,x0,x1);
//    Float::Arithmetic::Div<Float::FloatView,Float::FloatView,Float::FloatView>::post(*this,x2,x3,x4);
//    Float::Arithmetic::Sqr<Float::FloatView>::post(*this,x0,x1);
//    Float::Arithmetic::Sqrt<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Arithmetic::Abs<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Arithmetic::NthRoot<Float::FloatView,Float::FloatView>::post(*this,x0,x1,3);
//    Float::Arithmetic::Min<Float::FloatView,Float::FloatView,Float::FloatView>::post(*this,x0,x2,x1);
//    Float::Arithmetic::Max<Float::FloatView,Float::FloatView,Float::FloatView>::post(*this,x0,x2,x1);
//    Float::Arithmetic::Channel<Float::FloatView,Int::IntView>::post(*this,x0,iv);
//    Float::Transcendental::Exp<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Transcendental::Log<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::ASin<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::Sin<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::ACos<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::Cos<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::ATan<Float::FloatView,Float::FloatView>::post(*this,x0,x1);
//    Float::Trigonometric::Tan<Float::FloatView,Float::FloatView>::post(*this,x0,x1);

//    rel(*this, f[0] <= f[1]);
//    rel(*this, f[0] >= f[1]);
//    rel(*this, f[0] == f[1]);
//    rel(*this, f[0] * f[0] == f[1]);
//    rel(*this, f[2] / f[0] == f[1]);
//    rel(*this, f[2] / f[3] == f[4]);
//    rel(*this, sqr(f[0]) == f[1]);
//    rel(*this, sqrt(f[0]) == f[1]);
//    rel(*this, abs(f[0]) == f[1]);
//    rel(*this, nroot(f[0],3) == f[1]);
//    rel(*this, min(f[0],f[2]) == f[1]);
//    rel(*this, max(f[0],f[2]) == f[1]);
//    ivar = channel(*this,f[0]); rel(*this, ivar >= -5); rel(*this, ivar <= 5);
//    rel(*this, exp(f[1]) == f[2]);
//    rel(*this, log(f[0]) == f[1]);
//    rel(*this, asin(f[0]) == f[1]);
//    rel(*this, sin(f[0]) == f[2]);
//    rel(*this, acos(f[0]) == f[1]);
//    rel(*this, cos(f[0]) == f[1]);
//    rel(*this, atan(f[1]) == f[2]);
//    rel(*this, tan(f[1]) == f[2]);
//    rel(*this, (f[0] == f[1]));

//    rel(*this, bvar == 1);
//    rel(*this, bvar == (f[0] <= f[1]));
//    rel(*this, (f[0] == f[1]));
//    rel(*this, !(f[0] == f[1]));
//    rel(*this, f[0] == -20.0);
//    rel(*this, (sin(f[0]) == f[1]));
//    Float::Rel::ReLq<Float::FloatView,Int::BoolView,RM_EQV>::post(*this,Float::FloatView(f[0]),Float::FloatView(f[1]),Int::BoolView(bvar));

//      // Exemple 1 (spirale d'or)
//      rel(*this, f[0] >= 0);
//      rel(*this, f[4] >= 0);
//      rel(*this, f[4]*cos(f[0]) == f[1]);
//      rel(*this, f[4]*sin(f[0]) == f[2]);
//      rel(*this, exp(0.306349*f[0]) == f[4]);

//       // Exemple 1-2 (spirale d'archimède)
//       rel(*this, f[0] >= 0);
//       rel(*this, f[0] <= 6*FloatVal::pi());
//       rel(*this, f[4] >= 0);
//       rel(*this, f[4]*cos(f[0]) == f[1]);
//       rel(*this, f[4]*sin(f[0]) == f[2]);
//       rel(*this, f[4] == f[0]);

//      // Exemple 2 (ellipse)
//      int a = 2, b = 3;
//      rel(*this, a*sin(f[0]) == f[1]);
//      rel(*this, b*cos(f[0]) == f[2]);

//    // Exemple 3 (cycloide de Pascal)
//    double r = 0.5;
//    rel(*this, r*(f[0]-sin(f[0])) == f[1]);
//    rel(*this, r*(1-cos(f[0])) == f[2]);

//    // Exemple 4 (coeur paramétrique)
//    rel(*this, sin(f[0])*cos(f[0])*log(abs(f[0])) == f[1]);
//    rel(*this, sqrt(abs(f[0]))*cos(f[0]) == f[2]);
//    rel(*this, f[0] == FloatVal(-1,1));
//    step = 0.001;

//    // Exemple 5 (coeur cartésien)
//    int q = 2;
//    double p = 0.5;
//    rel(*this, sqr(f[1]) + 2*sqr(f[2]-p*nroot(abs(f[1]),q)) == 1);
//    rel(*this, f[0] == f[1]);
//    step = 0.01; 
//    branch(*this,f[2],FLOAT_VAL_SPLIT_MIN());
//    // Mettre le rel avec f[0] et f[2] dans le constrain

//    // Exemple 6 (Folium de Descartes)
//    rel(*this, 3*f[0]/(1+pow(f[0],3)) == f[1]);
//    rel(*this, 3*sqr(f[0])/(1+pow(f[0],3)) == f[2]);
//    rel(*this, pow(f[1],3) + pow(f[2],3)  == 3 * f[1] * f[2]);
//    rel(*this, f[1] == FloatVal(-1,2));

     // Exemple 7 (rosace)
     rel(*this, f[0] >= 0);
     rel(*this, f[0] <= 2*FloatVal::pi());
     rel(*this, f[4] >= 0);
#ifdef GECODE_HAS_MPFR
     rel(*this, f[4]*cos(f[0]) == f[1]);
     rel(*this, f[4]*sin(f[0]) == f[2]);
     rel(*this, f[4] == 2*cos(10*f[0]));
#endif
     step = 0.001;

#ifdef BOXES
    rel(*this, f[0] == FloatVal(sMinValue,sMinValue+step));
#endif
  }

  TestFloat(bool share, TestFloat& p) : Script(share,p)
  {
    ivar.update(*this,share,p.ivar);
    bvar.update(*this,share,p.bvar);
    f.update(*this,share,p.f);
    step = p.step;
  }

  virtual Space* copy(bool share) { return new TestFloat(share,*this); }

  virtual void constrain(const Space& _b) {
    const TestFloat& b = static_cast<const TestFloat&>(_b);
//     rel(*this, (f[0] >= (b.f[0].max()+step)) || (f[2] >= (b.f[2].max()+step))
//                                              || (f[2] <= (b.f[2].min()-step)));
    rel(*this, f[0] >= (b.f[0].max()+step));
  }

  virtual void
  print(std::ostream& os) const
  {
//  ./examples/test-float | grep FV | ploticus -prefab lines data=- pointsym=none x=2 y=3
#ifndef BOXES
    os << "FV " << f[1].med() << " " << f[2].med();
//    if (v0.min() == v1.min()) os << " MIN == "; else os << " MIN != " ;
//    if (v0.max() == v1.max()) os << " MAX == "; else os << " MAX != " ;
#else
//  ./examples/test-float | grep FV | ploticus -prefab lines data=- pointsym=none x=2 y=3
//  os << "FV " << f[0].min() << " " << f[1].max() << "\n";
//    os << "FV " << f[0].max() << " " << f[1].max() << "\n";
//    os << "FV " << f[0].max() << " " << f[1].min() << "\n";
//    os << "FV " << f[0].min() << " " << f[1].min() << "\n";
//    os << "FV " << f[0].min() << " " << f[1].max() << "\nFV \n";
    os << "FV " << f[1].min() << " " << f[2].max() << "\n";
    os << "FV " << f[1].max() << " " << f[2].max() << "\n";
    os << "FV " << f[1].max() << " " << f[2].min() << "\n";
    os << "FV " << f[1].min() << " " << f[2].min() << "\n";
    os << "FV " << f[1].min() << " " << f[2].max() << "\nFV \n";
#endif

    os << std::endl;
  }

};

double TestFloat::sMinValue = -20;

int main(int argc, char* argv[])
{
  Options opt("Float test program");
  opt.parse(argc,argv);
#ifndef BOXES
  opt.solutions(0);
  Script::run<TestFloat,BAB,Options>(opt);
#else
  opt.solutions(1);
  while (TestFloat::sMinValue < 20)
  {
    std::cout << "MinFloat: " << TestFloat::sMinValue << std::endl;
    TestFloat tf(opt);
    tf.status();
    if (!tf.failed()) tf.print(std::cout);
    TestFloat::sMinValue += tf.step;
  }
#endif
  return 0;
}

// STATISTICS: example-any
