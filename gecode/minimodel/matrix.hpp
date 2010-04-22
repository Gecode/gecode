/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *
 *  Bugfixes provided by:
 *     Olof Sivertsson <olof@olofsivertsson.com>
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

#include <algorithm>

namespace Gecode {

  template<class A>
  inline
  Slice<A>::Slice(Matrix<A>& a, int fc, int tc, int fr, int tr)
    : _r(0), _fc(fc), _tc(tc), _fr(fr), _tr(tr) {
    if (tc > a.width() || tr > a.height())
      throw MiniModel::ArgumentOutOfRange("Slice::Slice");
    if (fc >= tc || fr >= tr) {
      _fc=0; _tc=0; _fr=0; _tr=0;
      return;
    }

    _r = args_type((tc-fc)*(tr-fr));

    int i = 0;
    for (int h = fr; h < tr; h++)
      for (int w = fc; w < tc; w++)
        _r[i++] = a(w, h);
  }

  template<class A>
  Slice<A>&
  Slice<A>::reverse(void) {
    for (int i = 0; i < _r.size()/2; i++)
      std::swap(_r[i], _r[_r.size()-i-1]);
    return *this;
  }

  template<class A>
  forceinline
  Slice<A>::operator args_type(void) {
    return _r;
  }
  template<class A>
  forceinline
  Slice<A>::operator Matrix<args_type>(void) {
    return Matrix<args_type>(_r, _tc-_fc, _tr-_fr);
  }
  template<class A>
  forceinline
  Slice<A>::operator const args_type(void) const {
    return _r;
  }
  template<class A>
  forceinline
  Slice<A>::operator const Matrix<args_type>(void) const {
    return Matrix<args_type>(_r, _tc-_fc, _tr-_fr);
  }

  template<class A>
  typename Slice<A>::args_type
  operator+(const Slice<A>& x, const Slice<A>& y) {
    typename Slice<A>::args_type xx = x;
    typename Slice<A>::args_type yy = y;
    return xx+yy;
  }

  template<class A>
  typename Slice<A>::args_type
  operator+(const Slice<A>& x, const typename ArrayTraits<A>::args_type& y) {
    typename Slice<A>::args_type xx = x;
    return xx+y;
  }

  template<class A>
  typename Slice<A>::args_type
  operator+(const typename ArrayTraits<A>::args_type& x, const Slice<A>& y) {
    typename Slice<A>::args_type yy = y;
    return x+yy;
  }

  template<class A>
  typename Slice<A>::args_type
  operator+(const Slice<A>& x, const typename ArrayTraits<A>::value_type& y) {
    typename Slice<A>::args_type xx = x;
    return xx+y;
  }

  template<class A>
  typename Slice<A>::args_type
  operator+(const typename ArrayTraits<A>::value_type& x, const Slice<A>& y) {
    typename Slice<A>::args_type yy = y;
    return x+yy;
  }

  template<class A>
  forceinline
  Matrix<A>::Matrix(A a, int w, int h)
    : _a(a), _w(w), _h(h) {
    if ((_w * _h) != _a.size())
      throw MiniModel::ArgumentSizeMismatch("Matrix::Matrix(A, w, h)");
  }

  template<class A>
  forceinline
  Matrix<A>::Matrix(A a, int n)
    : _a(a), _w(n), _h(n) {
    if (n*n != _a.size())
      throw MiniModel::ArgumentSizeMismatch("Matrix::Matrix(A, n)");
  }

  template<class A>
  forceinline int
  Matrix<A>::width(void) const  { return _w; }
  template<class A>
  forceinline int
  Matrix<A>::height(void) const { return _h; }
  template<class A>
  inline typename Matrix<A>::args_type const
  Matrix<A>::get_array(void) const {
    return args_type(_a);
  }

  template<class A>
  forceinline typename Matrix<A>::value_type&
  Matrix<A>::operator ()(int c, int r) {
    if ((c >= _w) || (r >= _h))
      throw MiniModel::ArgumentOutOfRange("Matrix::operator ()");
    return _a[r*_w + c];
  }

  template<class A>
  forceinline Slice<A>
  Matrix<A>::slice(int fc, int tc, int fr, int tr) {
    return Slice<A>(*this, fc, tc, fr, tr);
  }

  template<class A>
  forceinline Slice<A>
  Matrix<A>::row(int r) {
    return slice(0, width(), r, r+1);
  }

  template<class A>
  forceinline Slice<A>
  Matrix<A>::col(int c) {
    return slice(c, c+1, 0, height());
  }

  forceinline void
  element(Home home, const Matrix<IntArgs>& m, IntVar x, IntVar y,  
          IntVar z, IntConLevel icl) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z, icl);
  }
  forceinline void
  element(Home home, const Matrix<IntArgs>& m, IntVar x, IntVar y,  
          BoolVar z, IntConLevel icl) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z, icl);
  }
  forceinline void
  element(Home home, const Matrix<IntVarArgs>& m, IntVar x, IntVar y,  
          IntVar z, IntConLevel icl) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z, icl);
  }
  forceinline void
  element(Home home, const Matrix<BoolVarArgs>& m, IntVar x, IntVar y,  
          BoolVar z, IntConLevel icl) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z, icl);
  }

#ifdef GECODE_HAS_SET_VARS
  forceinline void
  element(Home home, const Matrix<IntSetArgs>& m, IntVar x, IntVar y,  
          SetVar z) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z);
  }
  forceinline void
  element(Home home, const Matrix<SetVarArgs>& m, IntVar x, IntVar y,  
          SetVar z) {
    element(home, m.get_array(), x, m.width(), y, m.height(), z);
  }
#endif

}

// STATISTICS: minimodel-any
