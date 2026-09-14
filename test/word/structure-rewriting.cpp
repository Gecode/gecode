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

#include "test/word.hh"
#include <gecode/search.hh>
#include <gecode/word/structure.hh>
#include <memory>

namespace Test { namespace Word { namespace StructureRewriting {
  using namespace Gecode;

  class S : public Space {
  public:
    WordVar x,a,y;
    BoolVar b;
    S(unsigned int width=8, WordDomainType kind=WDT_UNSIGNED)
      : x(*this,width,kind),a(*this,width,kind),y(*this,width,kind),b(*this,0,1) {}
    S(S& s) : Space(s) {
      x.update(*this,s.x);a.update(*this,s.a);y.update(*this,s.y);b.update(*this,s.b);
    }
    Space* copy(void) { return new S(*this); }
  };

  void range(S& s, WordVar x, WordValue lo, WordValue hi) {
    rel(s,x,WRT_UGQ,x.width(),lo);rel(s,x,WRT_ULQ,x.width(),hi);
  }
  bool same(WordVar x, WordVar y) {
    return x.lo()==y.lo() && x.hi()==y.hi() &&
      x.minimum()==y.minimum() && x.maximum()==y.maximum();
  }

  // The generic view deliberately retains the closure loop, providing a
  // reference against which to compare the disjoint-cube fast path.
  class LoopView : public Gecode::Word::WordView {
  public:
    explicit LoopView(WordVar x) : Gecode::Word::WordView(x) {}
  };

  class ProjectionSpace : public Space {
  public:
    WordVar x,y;
    ProjectionSpace(unsigned int wx, unsigned int wy,
                    WordValue lx, WordValue hx, WordValue ly, WordValue hy)
      : x(*this,wx,lx,hx),y(*this,wy,ly,hy) {}
    ProjectionSpace(ProjectionSpace& s) : Space(s) {
      x.update(*this,s.x);y.update(*this,s.y);
    }
    Space* copy(void) { return new ProjectionSpace(*this); }
  };

  class CubeProjection : public Base {
    static WordValue image(Gecode::Word::Structure::FixedOp op, WordValue x,
                           unsigned int wx, unsigned int wy,
                           unsigned int a, unsigned int b) {
      using namespace Gecode::Word::Structure;
      const WordValue mask=(WordValue(1)<<wy)-1;
      switch (op) {
      case FO_EXTRACT: return (x>>a)&mask;
      case FO_REPEAT: {
        WordValue y=0;
        for (unsigned int i=0; i<a; i++) y|=x<<(i*b);
        return y;
      }
      case FO_ZERO_EXTEND: return x;
      case FO_SIGN_EXTEND:
        return (x&(WordValue(1)<<(wx-1))) ? x|(mask^((WordValue(1)<<wx)-1)) : x;
      case FO_SHIFT_LEFT: return a>=wx ? 0 : (x<<a)&mask;
      case FO_LOGICAL_SHIFT_RIGHT: return a>=wx ? 0 : x>>a;
      case FO_ARITHMETIC_SHIFT_RIGHT: {
        const long long value=(x&(WordValue(1)<<(wx-1))) ?
          static_cast<long long>(x)-(1LL<<wx) : static_cast<long long>(x);
        // Euclidean floor division gives sign extension without relying on
        // implementation-defined right shifts of negative signed values.
        const long long divisor=1LL<<std::min(a,wx);
        return WordValue(value>=0 ? value/divisor : -((-value+divisor-1)/divisor))&mask;
      }
      case FO_ROTATE_LEFT:
        a%=wx;return a==0 ? x : ((x<<a)|(x>>(wx-a)))&mask;
      case FO_ROTATE_RIGHT:
        a%=wx;return a==0 ? x : ((x>>a)|(x<<(wx-a)))&mask;
      default: GECODE_NEVER;
      }
      return 0;
    }
  public:
    CubeProjection(void) : Base("Word::Structure::Rewriting::CubeProjection") {}
    bool run(void) {
      using namespace Gecode::Word;
      using namespace Gecode::Word::Structure;
      S cube(3,WDT_CUBE),bounded(3,WDT_UNSIGNED);
      WordView cx(cube.x),cy(cube.y),bx(bounded.x),by(bounded.y);
      ConstWordView constant(3,0);
      if (!fixed_projection_one_pass(cx,cy) ||
          fixed_projection_one_pass(cx,cx) ||
          fixed_projection_one_pass(bx,by) ||
          fixed_projection_one_pass(cx,bx) ||
          fixed_projection_one_pass(LoopView(cube.x),LoopView(cube.y)) ||
          !fixed_projection_one_pass(constant,cx) ||
          !fixed_projection_one_pass(cx,constant) ||
          fixed_projection_one_pass(constant,bx) ||
          fixed_projection_one_pass(bx,constant)) return false;
      for (int operation=FO_EXTRACT; operation<=FO_ROTATE_RIGHT; operation++) {
        const FixedOp op=static_cast<FixedOp>(operation);
        const unsigned int wx=(op==FO_REPEAT || op==FO_ZERO_EXTEND ||
                               op==FO_SIGN_EXTEND) ? 2 : 3;
        const unsigned int wy=op==FO_EXTRACT ? 2 : wx==2 ? 4 : 3;
        const unsigned int amounts=op==FO_EXTRACT ? 2 :
          (op>=FO_SHIFT_LEFT ? 5 : 1);
        const WordValue mx=(WordValue(1)<<wx)-1,my=(WordValue(1)<<wy)-1;
        for (unsigned int amount=0; amount<amounts; amount++) {
          const unsigned int a=op==FO_REPEAT ? 2 : amount,b=wx;
          for (WordValue lx=0; lx<=mx; lx++)
            for (WordValue hx=lx; hx<=mx; hx++) {
              if (lx&~hx) continue;
              for (WordValue ly=0; ly<=my; ly++)
                for (WordValue hy=ly; hy<=my; hy++) {
                  if (ly&~hy) continue;
                  ProjectionSpace fast(wx,wy,lx,hx,ly,hy),loop(wx,wy,lx,hx,ly,hy);
                  const ExecStatus ef=Fixed<WordView,WordView>::narrow(
                    fast,WordView(fast.x),WordView(fast.y),op,a,b);
                  const ExecStatus el=Fixed<LoopView,LoopView>::narrow(
                    loop,LoopView(loop.x),LoopView(loop.y),op,a,b);
                  bool supported=false;WordValue ox=mx,px=0,oy=my,py=0;
                  for (WordValue x=0; x<=mx; x++) {
                    if ((x&lx)!=lx || (x&~hx)) continue;
                    const WordValue y=image(op,x,wx,wy,a,b);
                    if ((y&ly)!=ly || (y&~hy)) continue;
                    supported=true;ox&=x;px|=x;oy&=y;py|=y;
                  }
                  if ((ef==ES_FAILED)!=(el==ES_FAILED) ||
                      (ef==ES_FAILED)==supported) return false;
                  if (supported && (fast.x.lo()!=ox || fast.x.hi()!=px ||
                      fast.y.lo()!=oy || fast.y.hi()!=py ||
                      fast.x.lo()!=loop.x.lo() || fast.x.hi()!=loop.x.hi() ||
                      fast.y.lo()!=loop.y.lo() || fast.y.hi()!=loop.y.hi())) return false;
                }
            }
        }
      }
      return true;
    }
  } cube_projection;

  class LateFixed : public Base {
  public:
    LateFixed(void) : Base("Word::Structure::Rewriting::LateFixed") {}
    bool run(void) {
      S closure(4);range(closure,closure.y,3,12);
      shift_left(closure,closure.x,1,closure.y);
      if (closure.status()==SS_FAILED) return false;
      dom(closure,closure.x,0,13);
      if (closure.status()==SS_FAILED || !(closure.x.lo()&4)) return false;
      S right(4);range(right,right.y,3,6);
      logical_shift_right(right,right.x,1,right.y);
      if (right.status()==SS_FAILED) return false;
      dom(right,right.x,0,13);
      if (right.status()==SS_FAILED || !(right.x.lo()&8)) return false;
      S s;shift_left(s,s.x,1,s.y);
      if (s.status()==SS_FAILED) return false;
      std::unique_ptr<S> sibling(static_cast<S*>(s.clone()));
      range(s,s.x,20,30);
      if (s.status()==SS_FAILED || s.y.minimum()!=40 || s.y.maximum()!=60)
        return false;
      S fresh;range(fresh,fresh.x,20,30);shift_left(fresh,fresh.x,1,fresh.y);
      if (fresh.status()==SS_FAILED || !same(s.x,fresh.x) || !same(s.y,fresh.y))
        return false;
      std::unique_ptr<S> after(static_cast<S*>(s.clone()));
      dom(*after,after->x,25);
      if (after->status()==SS_FAILED || !after->y.assigned() || after->y.val()!=50 ||
          s.y.assigned() || sibling->y.maximum()==60) return false;
      range(s,s.y,0,39);
      if (s.status()!=SS_FAILED) return false;
      range(*sibling,sibling->x,200,210);
      dom(*sibling,sibling->x,205);
      if (sibling->status()==SS_FAILED || !sibling->y.assigned() ||
          sibling->y.val()!=154) return false;
      for (unsigned int width : {1U,4U,64U})
        for (unsigned int amount : {0U,width-1,width,width+1}) {
          S edge(width);const WordValue value=1;
          shift_left(edge,edge.x,amount,edge.y);dom(edge,edge.x,value);
          const WordValue expected=amount>=width ? 0 :
            (value<<amount)&Gecode::Word::width_mask(width);
          if (edge.status()==SS_FAILED || !edge.y.assigned() ||
              edge.y.val()!=expected) return false;
        }
      return true;
    }
  } late_fixed;

  class LateVariable : public Base {
  public:
    LateVariable(void) : Base("Word::Structure::Rewriting::LateVariable") {}
    bool run(void) {
      S s;range(s,s.x,20,40);range(s,s.a,0,3);shift_left(s,s.x,s.a,s.y);
      if (s.status()==SS_FAILED) return false;
      const WordValue lo=s.a.lo(),hi=s.a.hi();
      std::unique_ptr<S> sibling(static_cast<S*>(s.clone()));
      range(s,s.a,0,2);
      // This change must wake the adaptive actor even though no bit changes.
      if (s.a.lo()!=lo || s.a.hi()!=hi || s.status()==SS_FAILED ||
          s.y.minimum()!=20 || s.y.maximum()!=160) return false;
      S fresh;range(fresh,fresh.x,20,40);range(fresh,fresh.a,0,2);
      shift_left(fresh,fresh.x,fresh.a,fresh.y);
      if (fresh.status()==SS_FAILED || !same(s.x,fresh.x) ||
          !same(s.a,fresh.a) || !same(s.y,fresh.y)) return false;
      dom(*sibling,sibling->a,1);range(*sibling,sibling->x,20,30);
      if (sibling->status()==SS_FAILED || sibling->y.minimum()!=40 ||
          sibling->y.maximum()!=60) return false;
      S late;range(late,late.a,0,7);shift_left(late,late.x,late.a,late.y);
      if (late.status()==SS_FAILED) return false;
      dom(late,late.a,1);
      if (late.status()==SS_FAILED) return false;
      range(late,late.x,20,30);
      if (late.status()==SS_FAILED || late.y.minimum()!=40 || late.y.maximum()!=60)
        return false;
      S overshift;shift_left(overshift,overshift.x,overshift.a,overshift.y);
      if (overshift.status()==SS_FAILED) return false;
      dom(overshift,overshift.a,255);
      return overshift.status()!=SS_FAILED && overshift.y.assigned() &&
        overshift.y.val()==0;
    }
  } late_variable;

  class Aliases : public Base {
  public:
    Aliases(void) : Base("Word::Structure::Rewriting::Aliases") {}
    bool run(void) {
      for (WordDomainType kind : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED})
        for (int mode=0;mode<5;mode++)
          for (bool late : {false,true})
            for (WordValue x=0;x<8;x++)
              for (WordValue a=0;a<8;a++)
                for (WordValue y=0;y<8;y++) {
                  S s(3,kind);
                  if (mode==1 || mode==4) s.a=s.x;
                  if (mode==2 || mode==4) s.y=s.x;
                  if (mode==3) s.y=s.a;
                  const bool consistent=(mode!=1 || x==a) &&
                    (mode!=2 || x==y) && (mode!=3 || a==y) &&
                    (mode!=4 || (x==a && a==y));
                  const WordValue value=a>=3 ? 0 : (x<<a)&7;
                  const bool expected=consistent && value==y;
                  if (late) {
                    shift_left(s,s.x,s.a,s.y);
                    if (s.status()==SS_FAILED) return false;
                  }
                  dom(s,s.x,x);dom(s,s.a,a);dom(s,s.y,y);
                  if (!late) shift_left(s,s.x,s.a,s.y);
                  if ((s.status()!=SS_FAILED)!=expected) return false;
                }
      return true;
    }
  } aliases;

  class Constants : public Base {
  public:
    Constants(void) : Base("Word::Conditional::BoundedConstants") {}
    bool run(void) {
      S s;range(s,s.x,20,30);ite(s,s.b,8,200,s.x,s.y);
      if (s.status()==SS_FAILED || s.y.minimum()!=20 || s.y.maximum()!=200)
        return false;
      rel(s,s.b,IRT_EQ,0);
      if (s.status()==SS_FAILED) return false;
      range(s,s.x,20,23);
      if (s.status()==SS_FAILED || s.y.maximum()!=23) return false;
      S other;range(other,other.x,20,30);ite(other,other.b,other.x,8,3,other.y);
      if (other.status()==SS_FAILED || other.y.minimum()!=3 || other.y.maximum()!=30)
        return false;
      S signed_case(8,WDT_SIGNED);
      rel(signed_case,signed_case.x,WRT_SGQ,8,5);
      rel(signed_case,signed_case.x,WRT_SLQ,8,10);
      ite(signed_case,signed_case.b,8,236,signed_case.x,signed_case.y);
      if (signed_case.status()==SS_FAILED || signed_case.y.minimum()!=236 ||
          signed_case.y.maximum()!=10) return false;
      S alias;range(alias,alias.x,5,10);ite(alias,alias.b,8,3,alias.x,alias.x);
      if (alias.status()==SS_FAILED || !alias.b.assigned() || alias.b.val()!=0)
        return false;
      std::unique_ptr<S> copy(static_cast<S*>(other.clone()));
      rel(*copy,copy->b,IRT_EQ,0);
      return copy->status()!=SS_FAILED && copy->y.assigned() && copy->y.val()==3 &&
        !other.y.assigned();
    }
  } constants;

}}}

// STATISTICS: test-word
