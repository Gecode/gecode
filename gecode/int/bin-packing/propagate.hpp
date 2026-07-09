/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Fabio Tardivo <ftardivo@nmsu.edu>
 *
 *  Copyright:
 *     Fabio Tardivo, 2024
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
  Pack::f_ccm1(int w, int l, int c) {
    // Conditions
    int const c0 = w > c - w;  // x > c / 2
    int const c1 = w == c - w; // x == c / 2
    int const c2 = w < c - w;  // x < c / 2

    // Values
    int const v0 = 2 * ((c / l) - ((c - w) / l));
    int const v1 = c / l;
    int const v2 = 2 * (w / l);

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int
  Pack::f_mt(int w, int l, int c) {
    // Conditions
    int const c0 = w < l;
    int const c1 = (l <= w) && (w <= c - l);
    int const c2 = c - l < w;

    // Values
    int const v0 = 0;
    int const v1 = w;
    int const v2 = c;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int
  Pack::f_bj1(int w, int l, int c) {
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
  Pack::f_vb2_base(int w, int l, int c) {
    int v = ceil_div_pp(l * w, c);
    return v > 0 ? v - 1 : 0;
  }

  forceinline int
  Pack::f_vb2(int w, int l, int c) {
    int const c0 = w > c - w;
    int const c1 = w == c - w;
    int const c2 = w < c - w;

    int const t0 = f_vb2_base(c, l, c);
    int const t1 = f_vb2_base(w, l, c);
    int const t2 = f_vb2_base(c - w, l, c);

    int const v0 = 2 * t0 - 2 * t2;
    int const v1 = t0;
    int const v2 = 2 * t1;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int
  Pack::f_fs1(int w, int l, int c) {
    // Conditions
    int const c0 = w * (l + 1) % c == 0;
    int const c1 = w * (l + 1) % c != 0;

    // Values
    int const v0 = w * l;
    int const v1 = ((w * (l + 1)) / c) * c;

    return (c0 * v0) + (c1 * v1);
  }

  forceinline int
  Pack::f_rad2_base(int w, int l, int c) {
    // Conditions
    int const c0 = w < l;
    int const c1 = (l <= w) && (w <= c - 2 * l);
    int const c2 = (c - 2 * l < w) && (w < 2 * l);

    // Values
    int const v0 = 0;
    int const v1 = c / 3;
    int const v2 = c / 2;

    return (c0 * v0) + (c1 * v1) + (c2 * v2);
  }

  forceinline int
  Pack::f_rad2(int w, int l, int c) {
    // Conditions
    int const c0 = w < 2 * l;
    int const c1 = 2 * l <= w;

    // Values
    int const v0 = f_rad2_base(w, l, c);
    int const v1 = c - f_rad2_base(c - w, l, c);

    return (c0 * v0) + (c1 * v1);
  }

  forceinline LambdaRange
  Pack::l_ccm1(int c) {
    return {1, c / 2};
  }

  forceinline LambdaRange
  Pack::l_mt(int c) {
    return {0, c / 2};
  }

  forceinline LambdaRange
  Pack::l_bj1(int c) {
    return {1, c};
  }

  forceinline LambdaRange
  Pack::l_vb2(int c) {
    return {2, c};
  }

  forceinline LambdaRange
  Pack::l_fs1(int) {
    return {1, 100};
  }

  forceinline LambdaRange
  Pack::l_rad2(int c) {
    return {c / 4 + 1, c / 3};
  }

  forceinline LambdaRange
  Pack::sanitize_lambda_range(LambdaRange lambda_range,
                              int n_not_zero_weights, int max_weight) {
    long long int n = static_cast<long long int>(n_not_zero_weights) *
      static_cast<long long int>(max_weight);
    if (n != 0) {
      int l_max =
        std::min(static_cast<int>(std::numeric_limits<int>::max() / n),
                 lambda_range.max);
      return {lambda_range.min, l_max};
    } else {
      return {0, -1};
    }
  }

  template<int f(int,int,int)>
  forceinline int
  Pack::calc_dff_lower_bound_single_lambda(const int* weights, int n_weights,
                                           int capacity, int lambda) {
    // Transform and sum the weights
    long long int sum_transformed_weights = 0;
    for (int w_idx = 0; w_idx < n_weights; w_idx += 1)
      sum_transformed_weights += f(weights[w_idx], lambda, capacity);

    // Transform the capacity
    int transformed_capacity = f(capacity, lambda, capacity);

    // Lower bound
    return static_cast<int>
      (ceil_div_pp(sum_transformed_weights,
                   static_cast<long long int>(transformed_capacity)));
  }

  template<int f(int,int,int), LambdaRange l(int)>
  forceinline int
  Pack::calc_dff_lower_bound(const int* weights, int n_weights,
                             int capacity, int n_not_zero_weights,
                             int max_weight, bool sanitize) {
    LambdaRange lambda_range = l(capacity);
    lambda_range =
      sanitize ? sanitize_lambda_range(lambda_range, n_not_zero_weights,
                                       max_weight) :
                 lambda_range;
    if (lambda_range.min >= lambda_range.max)
      return 0;

    int lower_bound = 0;
    int l_step =
      ceil_div_pp(lambda_range.max - lambda_range.min + 1,
                  n_lambda_samples + 1);
    for (int lambda = lambda_range.min + l_step;
         lambda < lambda_range.max; ) {
      int cur_lower_bound =
        calc_dff_lower_bound_single_lambda<f>(weights, n_weights,
                                              capacity, lambda);
      lower_bound = std::max(lower_bound, cur_lower_bound);
      if (lambda > lambda_range.max - l_step)
        break;
      lambda += l_step;
    }
    return lower_bound;
  }

}}}

// STATISTICS: int-prop
