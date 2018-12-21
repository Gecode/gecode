/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode { namespace Int { namespace Branch {

  template<class View>
  forceinline
  ValCommitEq<View>::ValCommitEq
  (Space& home, const ValBranch<ValCommitEq<View>::Var>& vb)
    : ValCommit<View,int>(home,vb) {}
  template<class View>
  forceinline
  ValCommitEq<View>::ValCommitEq(Space& home, ValCommitEq& vc)
    : ValCommit<View,int>(home,vc) {}
  template<class View>
  forceinline ModEvent
  ValCommitEq<View>::commit(Space& home, unsigned int a, View x, int, int n) {
    return (a == 0) ? x.eq(home,n) : x.nq(home,n);
  }
  template<class View>
  forceinline NGL*
  ValCommitEq<View>::ngl(Space& home, unsigned int a, View x, int n) const {
    if (a == 0)
      return new (home) EqNGL<View>(home,x,n);
    else
      return NULL;
  }
  template<class View>
  forceinline void
  ValCommitEq<View>::print(const Space&, unsigned int a, View, int i,
                           int n, std::ostream& o) const {
    o << "var[" << i << "] "
      << ((a == 0) ? "=" : "!=") << " " << n;
  }

  template<class View>
  forceinline
  ValCommitLq<View>::ValCommitLq
  (Space& home, const ValBranch<ValCommitLq<View>::Var>& vb)
    : ValCommit<View,int>(home,vb) {}
  template<class View>
  forceinline
  ValCommitLq<View>::ValCommitLq(Space& home, ValCommitLq& vc)
    : ValCommit<View,int>(home,vc) {}
  template<class View>
  forceinline ModEvent
  ValCommitLq<View>::commit(Space& home, unsigned int a, View x, int, int n) {
    return (a == 0) ? x.lq(home,n) : x.gr(home,n);
  }
  template<class View>
  forceinline NGL*
  ValCommitLq<View>::ngl(Space& home, unsigned int a, View x, int n) const {
    if (a == 0)
      return new (home) LqNGL<View>(home,x,n);
    else
      return NULL;
  }
  template<class View>
  forceinline void
  ValCommitLq<View>::print(const Space&, unsigned int a, View, int i,
                           int n, std::ostream& o) const {
    o << "var[" << i << "] "
      << ((a == 0) ? "<=" : ">") << " " << n;
  }

  template<class View>
  forceinline
  ValCommitGq<View>::ValCommitGq
  (Space& home, const ValBranch<ValCommitGq<View>::Var>& vb)
    : ValCommit<View,int>(home,vb) {}
  template<class View>
  forceinline
  ValCommitGq<View>::ValCommitGq(Space& home, ValCommitGq& vc)
    : ValCommit<View,int>(home,vc) {}
  template<class View>
  forceinline ModEvent
  ValCommitGq<View>::commit(Space& home, unsigned int a, View x, int, int n) {
    return (a == 0) ? x.gq(home,n) : x.le(home,n);
  }
  template<class View>
  forceinline NGL*
  ValCommitGq<View>::ngl(Space& home, unsigned int a, View x, int n) const {
    if (a == 0)
      return new (home) GqNGL<View>(home,x,n);
    else
      return NULL;
  }
  template<class View>
  forceinline void
  ValCommitGq<View>::print(const Space&, unsigned int a, View, int i,
                           int n, std::ostream& o) const {
    o << "var[" << i << "] "
      << ((a == 0) ? ">=" : "<") << " " << n;
  }

  template<class View>
  forceinline
  ValCommitGr<View>::ValCommitGr
  (Space& home, const ValBranch<ValCommitGr<View>::Var>& vb)
    : ValCommit<View,int>(home,vb) {}
  template<class View>
  forceinline
  ValCommitGr<View>::ValCommitGr(Space& home, ValCommitGr& vc)
    : ValCommit<View,int>(home,vc) {}
  template<class View>
  forceinline ModEvent
  ValCommitGr<View>::commit(Space& home, unsigned int a, View x, int, int n) {
    return (a == 0) ? x.gr(home,n) : x.lq(home,n);
  }
  template<class View>
  forceinline NGL*
  ValCommitGr<View>::ngl(Space& home, unsigned int a, View x, int n) const {
    if (a == 0)
      return new (home) GqNGL<View>(home,x,n+1);
    else
      return NULL;
  }
  template<class View>
  forceinline void
  ValCommitGr<View>::print(const Space&, unsigned int a, View, int i,
                           int n, std::ostream& o) const {
    o << "var[" << i << "] "
      << ((a == 0) ? ">" : "<=") << " " << n;
  }

}}}

// STATISTICS: int-branch

