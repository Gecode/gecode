/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

namespace Gecode { namespace Int { namespace BinPacking {

  /*
   * Item
   *
   */
  forceinline
  Item::Item(void)
    : s(0) {}
  forceinline
  Item::Item(IntView b, int s0)
    : DerivedView<IntView>(b), s(s0) {}

  forceinline IntView
  Item::bin(void) const {
    return x;
  }
  forceinline
  void Item::bin(IntView b) {
    x = b;
  }
  forceinline int
  Item::size(void) const {
    return s;
  }
  forceinline void
  Item::size(int s0) {
    s = s0;
  }

  forceinline void
  Item::update(Space& home, Item& i) {
    x.update(home,i.x);
    s = i.s;
  }


  forceinline bool
  operator ==(const Item& i, const Item& j) {
    return (i.bin() == j.bin()) && (i.size() == j.size());
  }
  forceinline bool
  operator !=(const Item& i, const Item& j) {
    return !(i == j);
  }

  /// For sorting according to size
  forceinline bool
  operator <(const Item& i, const Item& j) {
    return i.size() > j.size();
  }


  /*
   * Size set
   *
   */
  forceinline
  SizeSet::SizeSet(void) {}
  forceinline
  SizeSet::SizeSet(Region& region, int n_max)
    : n(0), t(0), s(region.alloc<int>(n_max)) {}
  forceinline void
  SizeSet::add(int s0) {
    t += s0; s[n++] = s0;
  }
  forceinline int
  SizeSet::card(void) const {
    return n;
  }
  forceinline int
  SizeSet::total(void) const {
    return t;
  }
  forceinline int
  SizeSet::operator [](int i) const {
    return s[i];
  }

  forceinline
  SizeSetMinusOne::SizeSetMinusOne(void) {}
  forceinline
  SizeSetMinusOne::SizeSetMinusOne(Region& region, int n_max)
    : SizeSet(region,n_max), p(-1) {}
  forceinline void
  SizeSetMinusOne::minus(int s0) {
    // This rests on the fact that items are removed in order
    do
      p++;
    while (s[p] > s0);
    assert(p < n);
  }
  forceinline int
  SizeSetMinusOne::card(void) const {
    assert(p >= 0);
    return n - 1;
  }
  forceinline int
  SizeSetMinusOne::total(void) const {
    assert(p >= 0);
    return t - s[p];
  }
  forceinline int
  SizeSetMinusOne::operator [](int i) const {
    assert(p >= 0);
    return s[(i < p) ? i : i+1];
  }



  /*
   * Packing propagator
   *
   */

  forceinline
  Pack::Pack(Home home, ViewArray<OffsetView>& l0, ViewArray<Item>& bs0)
    : Propagator(home), l(l0), bs(bs0), t(0) {
    l.subscribe(home,*this,PC_INT_BND);
    bs.subscribe(home,*this,PC_INT_DOM);
    for (int i=0; i<bs.size(); i++)
      t += bs[i].size();
  }

  forceinline
  Pack::Pack(Space& home, Pack& p)
    : Propagator(home,p), t(p.t) {
    l.update(home,p.l);
    bs.update(home,p.bs);
  }

  forceinline size_t
  Pack::dispose(Space& home) {
    l.cancel(home,*this,PC_INT_BND);
    bs.cancel(home,*this,PC_INT_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class SizeSet>
  forceinline bool
  Pack::nosum(const SizeSet& s, int a, int b, int& ap, int& bp) {
    if ((a <= 0) || (b >= s.total()))
      return false;
    int n=s.card()-1;
    int sc=0;
    int kp=0;
    while (sc + s[n-kp] < a) {
      sc += s[n-kp];
      kp++;
    }
    int k=0;
    int sa=0, sb = s[n-kp];
    while ((sa < a) && (sb <= b)) {
      sa += s[k++];
      if (sa < a) {
        kp--;
        sb += s[n-kp];
        sc -= s[n-kp];
        while (sa + sc >= a) {
          kp--;
          sc -= s[n-kp];
          sb += s[n-kp] - s[n-kp-k-1];
        }
      }
    }
    ap = sa + sc; bp = sb;
    return sa < a;
  }

  template<class SizeSet>
  forceinline bool
  Pack::nosum(const SizeSet& s, int a, int b) {
    int ap, bp;
    return nosum(s, a, b, ap, bp);
  }

  forceinline int 
  Pack::fCCM1(int w, int l, int c)
  {
    // Conditions
    int const c0 = 2 * w > c;  // x > c / 2
    int const c1 = 2 * w == c; // x == c / 2
    int const c2 = 2 * w < c;  // x < c / 2

    // Values
    int const v0 = 2 * ((c / l) - ((c - w) / l));
    int const v1 = c / l;
    int const v2 = 2 * (w / l);

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int 
  Pack::fMT(int w, int l, int c)
  {
    // Conditions
    int const c0 = w < l;
    int const c1 = l <= w and w <= c - l;
    int const c2 = c - l < w;

    // Values
    int const v0 = 0;
    int const v1 = w;
    int const v2 = c;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int 
  Pack::fBJ1(int w, int l, int c)
  {
    // Auxiliary values
    int const p = l - (c % l);

    // Conditions
    int const c0 = w % l <= c % l;
    int const c1 = w % l > c % l;

    // Values
    int const v0 = (w / l) * p;
    int const v1 = (w / l) * p + (w % l) - (c % l);

    return (c0 * v0) + (c1 * v1);
  }

  forceinline int 
  Pack::fVB2Base(int w, int l, int c)
  {
      int v = ceil_div_pp(l * w, c);
      return v > 0 ? v - 1 : 0;
  }

  forceinline int 
  Pack::fVB2(int w, int l, int c)
  {
    int const c0 = 2 * w > c;
    int const c1 = 2 * w == c;
    int const c2 = 2 * w < c;

    int const t0 = fVB2Base(c, l, c);
    int const t1 = fVB2Base(w, l, c);
    int const t2 = fVB2Base(c - w, l, c);

    int const v0 = 2 * t0 - 2 * t2;
    int const v1 = t0;
    int const v2 = 2 * t1;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int 
  Pack::fFS1(int w, int l, int c)
  {
    // Conditions
    int c0 = w * (l + 1) % c == 0;
    int c1 = w * (l + 1) % c != 0;

    // Values
    int v0 = w * l;
    int v1 = ((w * (l + 1)) / c) * c;

    return (c0 * v0) + (c1 * v1);
  }

  forceinline int 
  Pack::fRAD2Base(int w, int l, int c)
  {
    // Conditions
    int const c0 = w < l;
    int const c1 = l <= w and w <= c - 2 * l;
    int const c2 = c - 2 * l < w and w < 2 * l;

    // Values
    int const v0 = 0;
    int const v1 = c / 3;
    int const v2 = c / 2;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int 
  Pack::fRAD2(int w, int l, int c)
  {
    // Conditions
    int const c0 = w < 2 * l;
    int const c1 = 2 * l <= w;

    // Values
    int const v0 = fRAD2Base(w, l, c);
    int const v1 = c - fRAD2Base(c - w, l, c);

    return (c0 * v0) + (c1 * v1);
  }

  forceinline LambdaRange Pack::lCCM1(int c) {return {1, c / 2};};

  forceinline LambdaRange Pack::lMT(int c)   {return {0, c / 2};}; // The 0 value is included to calculate the L0 bound

  forceinline LambdaRange Pack::lBJ1(int c)  {return {1, c};}

  forceinline LambdaRange Pack::lVB2(int c)  {return {2, c};};

  forceinline LambdaRange Pack::lFS1(int c)  {return {1, 100};};

  forceinline LambdaRange Pack::lRAD2(int c) {return {c / 4 + 1, c / 3};};

  forceinline LambdaRange 
  Pack::sanitizeLambdaRange(LambdaRange lambdaRange, int nNotZeroWeights, int maxWeight)
  {
    if (nNotZeroWeights * maxWeight != 0)
    {
        int lMax = std::min(std::numeric_limits<int>::max() / (nNotZeroWeights * maxWeight), lambdaRange.max);
        return {lambdaRange.min, lMax};
    }
    else
    {
        return {0,-1};
    }
  }

  template<int f(int,int,int)>
  forceinline int 
  Pack::calcDffLowerboundSingleLambda(const IntDynamicArray& weights, int capacity, int lambda)
  {
      // Transform and sum the weights
      int sumTransformedWeights = 0;
      int nWeights = weights.capacity();
      for (int wIdx = 0; wIdx < nWeights; wIdx += 1)
      {
          sumTransformedWeights += f(weights[wIdx], lambda, capacity);
      }

      // Transform the capacity
      int transformedCapacity = f(capacity, lambda, capacity);

      // Lowerbound
      return ceil_div_pp(sumTransformedWeights, transformedCapacity);
  }

  template<int f(int,int,int), LambdaRange l(int)>
  forceinline int 
  Pack::calcDffLowerbound(const IntDynamicArray& weights, int capacity, int nNotZeroWeights, int maxWeight, bool sanitize)
  {
      int fLowerbound = 0;
      LambdaRange lambdaRange = l(capacity);
      lambdaRange = sanitize ? sanitizeLambdaRange(lambdaRange, nNotZeroWeights, maxWeight) : lambdaRange;
      int lStep = ceil_div_pp(lambdaRange.max - lambdaRange.min + 1, nLambdaSamples + 1);
      for (int lambda = lambdaRange.min + lStep; lambda < lambdaRange.max; lambda += lStep)
      {
          int lowerbound = calcDffLowerboundSingleLambda<f>(weights, capacity, lambda);
          fLowerbound = std::max(fLowerbound, lowerbound);
      }
      return fLowerbound;
  }

}}}

// STATISTICS: int-prop

