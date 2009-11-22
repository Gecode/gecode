/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     David Rijsman <David.Rijsman@quintiq.com>
 *
 *  Copyright:
 *     David Rijsman, 2009
 *
 *  Last modified:
 *     $Date$
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

#include <gecode/int/sequence.hh>

namespace Gecode {

  using namespace Int;

  void
  sequence(Home home, const IntVarArgs &x, const IntSet &s, 
           int q, int l, int u ,IntConLevel) {
    if ( x.size() == 0 )
      throw TooFewArguments("Int::sequence");

    Limits::check(s.min(),"Int::sequence");
    Limits::check(s.max(),"Int::sequence");
    Limits::check(q,"Int::sequence");
    Limits::check(l,"Int::sequence");
    Limits::check(u,"Int::sequence");

    if ( x.same(home) )
      throw ArgumentSame("Int::sequence");

    if ( q < 1 || q > x.size() )
      throw Exception("Int::sequence","1<=q<=n, invalid");

    if ( l < 0 || u > q || u < l )
      throw Exception("Int::sequence","0<=l<=u<=q, invalid");

    if ( home.failed() ) return;

    if ( 0 == l && q == u ) return;

    if ( 0 == u ) {
      for ( int i=x.size(); i--; ) {
        IntView xv(x[i]);
        IntSetRanges ris(s);
        GECODE_ME_FAIL(home,xv.minus_r(home,ris,false));
      }
      return;
    }

    if ( s.width() * l > static_cast<unsigned int>(q) ) {
      home.fail();
      return;
    }

    if ( l == q ) {
       for ( int i=x.size(); i--; ) {
        IntView xv(x[i]);
        IntSetRanges ris(s);
        GECODE_ME_FAIL(home,xv.inter_r(home,ris,false));
      }
      return;
    }

    ViewArray<IntView> xv(home,x);

    IntSetValues vs(s);
  
    while ( vs() ) {
      GECODE_ES_FAIL(home,(Sequence::post_sequence<IntView,int>(home,xv,vs.val(),q,l,u)));
      ++vs;
    }
  }

  void
  sequence(Home home, const BoolVarArgs &x, bool s, 
           int q, int l, int u, IntConLevel) {
      if ( x.size() == 0 )
        throw TooFewArguments("Int::sequence");

      Limits::check(q,"Int::sequence");
      Limits::check(l,"Int::sequence");
      Limits::check(u,"Int::sequence");

      if ( x.same(home) )
        throw ArgumentSame("Int::sequence");

      if ( q < 1 || q > x.size() )
        throw Exception("Int::sequence","1<=q<=n, invalid");

      if ( l < 0 || u > q || u < l )
        throw Exception("Int::sequence","0<=l<=u<=q, invalid");

      if ( home.failed() ) return;

      if ( 0 == u ) {
        for ( int i=x.size(); i--; ) {
          BoolView xv(x[i]);
          GECODE_ME_FAIL(home,xv.nq(home,s));
        }
        return;
      }

      if ( s * l > q ) {
        home.fail();
        return;
      }

      if ( l == q ) {
        for ( int i=x.size(); i--; ) {
          BoolView xv(x[i]);
          GECODE_ME_FAIL(home,xv.eq(home,s));
        }
        return;
      }

      if ( 0 == l && q == u ) return;

      ViewArray<BoolView> xv(home,x);

      GECODE_ES_FAIL(home,(Sequence::post_sequence<BoolView,bool>(home,xv,s,q,l,u)));
  }
}

// STATISTICS: int-post
