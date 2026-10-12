/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
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


#include <gecode/int/min-distance.hh>

#include <utility>

namespace Gecode {

  IntDistance::Data::Data(int n0, const IntArgs& m)
    : n(n0), matrix(m) {}

  IntDistance::Data::Data(int n0, std::function<int(int,int)> f)
    : n(n0), function(std::move(f)) {}

  IntDistance::Data::~Data(void) {}

  IntDistance::IntDistance(int n, const IntArgs& matrix) {
    Int::Limits::positive(n,"IntDistance");
    if (static_cast<long long int>(n)*n != matrix.size())
      throw Int::ArgumentSizeMismatch("IntDistance");
    for (int a=0; a<n; a++)
      for (int b=0; b<n; b++) {
        Int::Limits::nonnegative(matrix[a*n+b],"IntDistance");
        if (((a == b) && (matrix[a*n+b] != 0)) ||
            (matrix[a*n+b] != matrix[b*n+a]))
          throw Int::IllegalOperation("IntDistance");
      }
    object(new Data(n,matrix));
  }

  IntDistance::IntDistance(int n, std::function<int(int,int)> f) {
    Int::Limits::positive(n,"IntDistance");
    if (!f)
      throw InvalidFunction("IntDistance");
    object(new Data(n,std::move(f)));
  }

}

namespace Gecode { namespace Int { namespace MinDistance {

  /// Validate the organization and number of positions
  void check(const IntVarArgs& x, MinDistancePropKind kind) {
    if (x.size() < 2)
      throw TooFewArguments("Int::minimum_distance");
    if ((kind != MDP_DECOMPOSED) && (kind != MDP_SINGLE))
      throw IllegalOperation("Int::minimum_distance");
    Limits::nonnegative(static_cast<long long int>(x.size())*(x.size()-1)/2,
                        "Int::minimum_distance");
  }

  /// Shared posting for the two public overloads
  ExecStatus post(Home home, const IntVarArgs& x, IntView z,
                  const IntDistance& d, const IntSharedArray& r,
                  MinDistancePropKind kind, IntPropLevel ipl) {
    ViewArray<IntView> views(home,x);
    for (int i=0; i<views.size(); i++) {
      GECODE_ME_CHECK(views[i].gq(home,0));
      GECODE_ME_CHECK(views[i].le(home,d.size()));
    }
    GECODE_ME_CHECK(z.gq(home,0));
    if (views.assigned())
      return assigned(home,views,z,d,r);
    if (kind == MDP_DECOMPOSED) {
      for (int i=0; i<views.size(); i++)
        for (int j=i+1; j<views.size(); j++)
          GECODE_ES_CHECK(Pair::post(home,views[i],views[j],z,d,
                                    requirement(r,views.size(),i,j)));
      GECODE_ES_CHECK(Finalize::post(home,views,z,d,r));
    } else {
      GECODE_ES_CHECK(AdvisorForward::post(home,views,z,d,r));
    }
    if (ba(ipl) != IPL_BASIC)
      GECODE_ES_CHECK(Matching::post(home,views,z,d));
    return ES_OK;
  }

}}}

namespace Gecode {

  void minimum_distance(Home home, const IntVarArgs& x, IntVar z,
                        const IntDistance& d, IntPropLevel ipl,
                        MinDistancePropKind kind) {
    Int::MinDistance::check(x,kind);
    GECODE_POST;
    IntSharedArray r;
    GECODE_ES_FAIL(Int::MinDistance::post(home,x,z,d,r,kind,ipl));
  }

  void minimum_distance(Home home, const IntVarArgs& x, IntVar z,
                        const IntDistance& d, const IntArgs& r,
                        IntPropLevel ipl, MinDistancePropKind kind) {
    Int::MinDistance::check(x,kind);
    if (static_cast<long long int>(x.size())*x.size() != r.size())
      throw Int::ArgumentSizeMismatch("Int::minimum_distance");
    for (int i=0; i<x.size(); i++)
      for (int j=0; j<x.size(); j++) {
        Int::Limits::nonnegative(r[i*x.size()+j],"Int::minimum_distance");
        if (((i == j) && (r[i*x.size()+j] != 0)) ||
            (r[i*x.size()+j] != r[j*x.size()+i]))
          throw Int::IllegalOperation("Int::minimum_distance");
      }
    GECODE_POST;
    IntSharedArray requirements(r);
    GECODE_ES_FAIL(Int::MinDistance::post(home,x,z,d,requirements,kind,ipl));
  }

}

// STATISTICS: int-post
