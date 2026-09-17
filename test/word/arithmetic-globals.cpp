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
 */

#include "test/word.hh"
#include <gecode/word/arithmetic/global-integer.hpp>
#include <memory>
#include <limits>
#include <chrono>
#include <iostream>

namespace Test { namespace Word { namespace ArithmeticGlobals {
  using namespace Gecode;
  namespace E=Gecode;

  enum Kind { PRODUCT, DIVMOD, LINEAR, RADIX, IMAGE, PRODUCT_ALIAS,
              CANCEL_ZERO, DIVMOD_ALIAS, LINEAR_ALIAS, RADIX_ALIAS, IMAGE_ALIAS,
              LINEAR_DIVMOD, QUANTIZE };

  class Semantics : public Test {
    Kind kind;
    IntPropLevel options;
    bool signed_values;
    WordDomainType storage;
  public:
    Semantics(Kind k,const char* name,int arity,bool sign,bool knapsack,WordDomainType policy)
      : Test(std::string("ArithmeticGlobals::")+name+(sign ? "::Signed" : "::Unsigned")+
             (knapsack ? "::Advanced" : "::Basic")+
             (policy==WDT_CUBE ? "::Cube" : policy==WDT_SIGNED ? "::SignedDomain" : "::UnsignedDomain"),
             arity,Domain(sign ? 3 : 2,0,sign ? 7 : 3)),kind(k),
        options(knapsack ? IPL_BASIC_ADVANCED : IPL_BASIC),signed_values(sign),storage(policy) {}
    virtual bool solution(const Assignment& assignment) const {
      std::vector<int> a(assignment.size());
      for (int i=0;i<assignment.size();i++) {
        int value=static_cast<int>(assignment[i]);
        a[i]=signed_values && (value&4) ? value-8 : value;
      }
      int x=a[0],y=a[1];
      switch (kind) {
      case PRODUCT: return 2*x*y==a[2]*a[3];
      case DIVMOD: return a[2]>0 && a[4]>=0 && a[4]<a[2] && x*y==a[2]*a[3]+a[4];
      case LINEAR: return x-2*y+a[2]==1 && (x+y-2)%3==0 &&
        x+a[2]>=1 && x+a[2]<=4;
      case RADIX: return x>=0 && x<2 && y>=0 && y<3 && a[2]==3*x+y;
      case IMAGE: return a[2]==2*x-y+1 && a[3]==x+y;
      case PRODUCT_ALIAS: return x*x==y;
      case CANCEL_ZERO: return x*y==x*a[2];
      case DIVMOD_ALIAS: return false;
      case LINEAR_ALIAS: return x-y==1;
      case RADIX_ALIAS: return x>=0 && x<2 && y==3*x;
      case IMAGE_ALIAS: return x==2*x-y+1;
      case LINEAR_DIVMOD: return a[2]>0 && a[4]>=0 && a[4]<a[2] &&
        1+2*x-y==a[2]*a[3]+a[4];
      case QUANTIZE: return y>=x && y-x<3 && (y-1)%3==0;
      }
      return false;
    }
    virtual void post(Space& home,WordVarArray& source) {
      WordVarArgs x(source);
      const unsigned int width=signed_values ? 3 : 2;
      const WordValue sign=WordValue(1)<<(width-1), mask=(sign<<1)-1;
      WordVarArgs v(x.size());
      for (int i=0;i<x.size();i++) {
        if (storage!=WDT_CUBE) {
          x[i]=WordVar(home,width,storage,storage==WDT_SIGNED ? sign : 0,
                       storage==WDT_SIGNED ? sign-1 : mask);
          Gecode::rel(home,source[i],WRT_EQ,x[i]);
        }
        v[i]=x[i];
      }
      WordVarArgs a,b;
      switch (kind) {
      case PRODUCT:
        a<<v[0]<<v[1]; b<<v[2]<<v[3]; E::product_balance(home,a,2,b,1,options); break;
      case DIVMOD:
        a<<v[0]<<v[1]; E::product_divmod(home,a,v[2],v[3],v[4],options); break;
      case LINEAR:
        a<<v[0]<<v[1]<<v[2]; E::linear_system(home,a,
          {E::WordLinearRow::equal({1,-2,1},1),E::WordLinearRow::congruence({1,1,0},2,3),
           E::WordLinearRow::range({1,0,1},1,4)},options); break;
      case RADIX:
        a<<v[0]<<v[1]; E::mixed_radix(home,a,{2,3},v[2],options); break;
      case IMAGE:
        a<<v[0]<<v[1]; b<<v[2]<<v[3];
        E::bounded_image(home,a,{2,-1,1,1},{1,0},b,options); break;
      case PRODUCT_ALIAS:
        a<<v[0]<<v[0]; b<<v[1]; E::product_balance(home,a,1,b,1,options); break;
      case CANCEL_ZERO:
        a<<v[0]<<v[1]; b<<v[0]<<v[2]; E::product_balance(home,a,1,b,1,options); break;
      case DIVMOD_ALIAS:
        a<<v[0]; E::product_divmod(home,a,v[1],v[0],v[1],options); break;
      case LINEAR_ALIAS:
        a<<v[0]<<v[0]<<v[1]; E::linear_system(home,a,{E::WordLinearRow::equal({3,-2,-1},1)},options); break;
      case RADIX_ALIAS:
        a<<v[0]<<v[0]; E::mixed_radix(home,a,{2,2},v[1],options); break;
      case IMAGE_ALIAS:
        a<<v[0]<<v[1]; b<<v[0];
        E::bounded_image(home,a,{2,-1},{1},b,options); break;
      case LINEAR_DIVMOD:
        a<<v[0]<<v[1]; E::linear_divmod(home,{2,-1},a,1,v[2],v[3],v[4],options); break;
      case QUANTIZE:
        E::quantize_up(home,v[0],3,1,v[1],options); break;
      }
    }
  };
  class Create {
  public:
    Create(void) {
      for (WordDomainType policy : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED})
        for (bool knapsack : {false,true}) {
        bool sign=policy==WDT_SIGNED;
        (void) new Semantics(PRODUCT,"Product",4,sign,knapsack,policy);
        (void) new Semantics(DIVMOD,"ProductDivmod",5,sign,knapsack,policy);
        (void) new Semantics(LINEAR,"LinearSystem",3,sign,knapsack,policy);
        (void) new Semantics(RADIX,"MixedRadix",3,sign,knapsack,policy);
        (void) new Semantics(IMAGE,"BoundedImage",4,sign,knapsack,policy);
        (void) new Semantics(PRODUCT_ALIAS,"RepeatedFactor",2,sign,knapsack,policy);
        (void) new Semantics(CANCEL_ZERO,"ZeroCancellation",3,sign,knapsack,policy);
        (void) new Semantics(DIVMOD_ALIAS,"DivisorRemainderAlias",2,sign,knapsack,policy);
        (void) new Semantics(LINEAR_ALIAS,"LinearAlias",2,sign,knapsack,policy);
        (void) new Semantics(RADIX_ALIAS,"RadixAlias",2,sign,knapsack,policy);
        (void) new Semantics(IMAGE_ALIAS,"ImageAlias",2,sign,knapsack,policy);
        (void) new Semantics(LINEAR_DIVMOD,"LinearDivmod",5,sign,knapsack,policy);
        (void) new Semantics(QUANTIZE,"Quantize",2,sign,knapsack,policy);
      }
    }
  } create;

  // Same Base/Space pattern as bounded.cpp for persistent interval and wide
  // domain tests, which the common cube-domain fixture cannot represent.
  class Model : public Space {
  public:
    WordVarArray x;
    Model(int n,unsigned int width=64) : x(*this,n,width,WDT_UNSIGNED,0,
      Gecode::Word::width_mask(width)) {}
    Model(Model& s) : Space(s) { x.update(*this,s.x); }
    virtual Space* copy(void) { return new Model(*this); }
    bool range(int i,WordValue lo,WordValue hi) {
      if (x[i].domain_type()==WDT_SIGNED)
        return !me_failed(Gecode::Word::SignedWordView(x[i]).narrow_range(*this,
          Gecode::Word::rank(WDT_SIGNED,x[i].width(),lo),
          Gecode::Word::rank(WDT_SIGNED,x[i].width(),hi)));
      return !me_failed(Gecode::Word::UnsignedWordView(x[i]).narrow_range(*this,lo,hi));
    }
    bool bits(int i,WordValue lo,WordValue hi) {
      return !me_failed(Gecode::Word::WordView(x[i]).narrow(*this,lo,hi));
    }
  };
  class Rules : public Base {
    bool integer(void) {
      using namespace Gecode::Word::Arithmetic::Global;
      for (int a=-71;a<=71;a++) for (int b=-17;b<=17;b++) if (b) {
        Integer x=Integer::signed_value(a),y=Integer::signed_value(b);
        if (x+y!=Integer::signed_value(a+b) || x*y!=Integer::signed_value(a*b) ||
            x/y!=Integer::signed_value(a/b) || x%y!=Integer::signed_value(a%b)) return false;
        int f=a/b,c=f;
        if (a%b) { if ((a<0)!=(b<0)) f--; else c++; }
        if (floor_div(x,y)!=Integer::signed_value(f) ||
            ceil_div(x,y)!=Integer::signed_value(c)) return false;
      }
      const Integer m(~WordValue(0)),two(2),one(1);
      // Carry, borrow, high-product and division checks across a 64-bit limb.
      const WordValue values[]={0,1,0xffffffffULL,0x100000000ULL,
        0x8000000000000000ULL,~WordValue(0)};
      const Integer radix=Integer::power_two(64);
      for (WordValue a : values) for (WordValue b : values) {
        Integer x(a),y(b);
        if (mod(x+y,radix).word()!=a+b || mod(x*y,radix).word()!=a*b ||
            (x+y)-y!=x || (x-y)+y!=x) return false;
        if (b && ((x/y).word()!=a/b || (x%y).word()!=a%b)) return false;
      }
      if ((m*m)/radix!=m-one || (m*m)%radix!=one) return false;
      Integer p=m*m*m*m;
      return p.bits()==256 && p/m==m*m*m && (p+m)%m==Integer() &&
        (p-one)/(m-one)==m*m*m+m*m+m+one &&
        mod(Integer::signed_value(-1),m)==m-one &&
        mod(inverse(two,m)*two,m)==one &&
        Integer::signed_value(std::numeric_limits<std::int64_t>::min()).abs()==
          Integer(WordValue(1)<<63);
    }
    bool deductions(void) {
      IntPropLevel o=IPL_DEF;
      {
        Model s(3); s.range(0,3,9); s.range(1,4,4); s.range(2,20,24);
        WordVarArgs a,b; a<<s.x[0]<<s.x[1]; b<<s.x[2]; E::product_balance(s,a,1,b,1,o);
        if (s.status()==SS_FAILED || s.x[0].minimum()!=5 || s.x[0].maximum()!=6 ||
            (s.x[2].hi()&3)!=0) return false;
        std::unique_ptr<Model> clone(static_cast<Model*>(s.clone()));
        clone->range(2,24,24);
        if (clone->status()==SS_FAILED || clone->x[0].val()!=6 || s.x[0].assigned()) return false;
      }
      {
        Model s(5); s.range(0,1000,2000); s.range(1,8,8); s.range(2,64,64);
        s.range(3,150,150); s.range(4,0,0);
        WordVarArgs a; a<<s.x[0]<<s.x[1]; E::product_divmod(s,a,s.x[2],s.x[3],s.x[4],o);
        if (s.status()==SS_FAILED || !s.x[0].assigned() ||
            s.x[0].val()!=1200 || PropagatorGroup::all.size(s)!=0)
          return false;
      }
      {
        Model s(2); WordVarArgs a; a<<s.x[0]<<s.x[1];
        E::linear_system(s,a,{E::WordLinearRow::equal({1,1},10),E::WordLinearRow::equal({1,-1},1)},IPL_ADVANCED);
        if (s.status()!=SS_FAILED) return false; // 2*x=11 from two rows.
      }
      {
        Model s(5); s.range(1,8,8); s.range(2,64,64);
        WordVarArgs a; a<<s.x[0]<<s.x[1]; E::product_divmod(s,a,s.x[2],s.x[3],s.x[4],o);
        if (s.status()==SS_FAILED || (s.x[4].hi()&7)!=0 || s.x[4].maximum()!=56) return false;
      }
      {
        Model s(1); WordVarArgs a; a<<s.x[0];
        E::linear_system(s,a,{E::WordLinearRow::congruence({1},0,6),E::WordLinearRow::congruence({1},1,9)},o);
        if (s.status()!=SS_FAILED) return false;
      }
      {
        Model s(1); s.range(0,0,100); WordVarArgs a; a<<s.x[0];
        E::linear_system(s,a,{E::WordLinearRow::congruence({1},2,6),E::WordLinearRow::congruence({1},5,9)},o);
        if (s.status()==SS_FAILED || s.x[0].minimum()!=14 || s.x[0].maximum()!=86) return false;
      }
      {
        Model s(3,16); WordVarArgs a; a<<s.x[0]<<s.x[1];
        s.bits(2,0xA05,0xFFF); E::mixed_radix(s,a,{16,256},s.x[2],o);
        if (s.status()==SS_FAILED || (s.x[0].lo()&10)!=10 || (s.x[1].lo()&5)!=5) return false;
      }
      {
        Model s(3); s.range(2,34,38); WordVarArgs a; a<<s.x[0]<<s.x[1];
        E::mixed_radix(s,a,{4,10},s.x[2],o);
        if (s.status()==SS_FAILED || s.x[0].val()!=3 ||
            s.x[1].minimum()!=4 || s.x[1].maximum()!=8) return false;
      }
      {
        Model s(2); s.range(1,20,30); WordVarArgs a,b; a<<s.x[0]; b<<s.x[1];
        E::bounded_image(s,a,{6},{2},b,o);
        if (s.status()==SS_FAILED || s.x[0].minimum()!=3 || s.x[0].maximum()!=4 ||
            s.x[1].minimum()!=20 || s.x[1].maximum()!=26) return false;
      }
      return true;
    }
    bool wide(void) {
      const WordValue m=~WordValue(0); IntPropLevel o=IPL_DEF;
      for (bool valid : {false,true}) {
        Model s(2); s.range(0,m,m); s.range(1,valid ? m : m-1,valid ? m : m-1);
        WordVarArgs a,b; a<<s.x[0]<<s.x[0]<<s.x[0]; b<<s.x[1]<<s.x[1]<<s.x[1];
        E::product_balance(s,a,1,b,1,o);
        if ((s.status()!=SS_FAILED)!=valid) return false;
      }
      {
        Model s(4); s.range(0,m,m); s.range(1,m,m); s.range(2,m,m); s.range(3,0,0);
        WordVarArgs a; a<<s.x[0]<<s.x[1]; E::product_divmod(s,a,s.x[1],s.x[2],s.x[3],o);
        if (s.status()==SS_FAILED) return false; // 128-bit dividend, 64-bit result.
      }
      {
        Model s(2); s.range(0,WordValue(1)<<63,WordValue(1)<<63); s.range(1,0,0);
        WordVarArgs a,b; a<<s.x[0]; b<<s.x[1]; E::product_balance(s,a,2,b,1,o);
        if (s.status()!=SS_FAILED) return false; // No wrapping to zero.
      }
      {
        Model s(2); WordVarArgs a; a<<s.x[0]<<s.x[1];
        E::mixed_radix(s,a,{m,m},s.x[1],o);
        if (s.status()==SS_FAILED || s.x[0].val()!=0) return false;
      }
      {
        Model s(1); WordVarArgs empty,a; a<<s.x[0];
        E::product_balance(s,empty,1,a,1,o);
        if (s.status()==SS_FAILED || s.x[0].val()!=1) return false;
      }
      {
        Model s(2); s.x[0]=WordVar(s,1,WDT_UNSIGNED,0,1);
        WordVarArgs a,b; a<<s.x[0]; b<<s.x[1]; s.range(1,m,m);
        E::product_balance(s,a,m,b,1,o);
        if (s.status()==SS_FAILED || !s.x[0].assigned() || s.x[0].val()!=1) return false;
      }
      {
        Model s(1); WordVarArgs a; a<<s.x[0];
        E::product_balance(s,a,m,a,m-1,o);
        if (s.status()==SS_FAILED || !s.x[0].assigned() || s.x[0].val()!=0) return false;
      }
      {
        Model s(3); WordVarArgs a; a<<s.x[0];
        E::product_divmod(s,a,s.x[1],s.x[2],s.x[1],o);
        if (s.status()!=SS_FAILED) return false;
      }
      {
        Model s(3); s.x[0]=WordVar(s,2,WDT_UNSIGNED,0,3);
        s.x[1]=WordVar(s,6,WDT_UNSIGNED,0,63);
        s.x[2]=WordVar(s,10,WDT_UNSIGNED,191,191);
        WordVarArgs a; a<<s.x[0]<<s.x[1]; E::mixed_radix(s,a,{4,64},s.x[2],o);
        if (s.status()==SS_FAILED || s.x[0].val()!=2 || s.x[1].val()!=63) return false;
      }
      return true;
    }
  public:
    Rules(void) : Base("Word::ArithmeticGlobals::BoundedAndWideRules") {}
    virtual bool run(void) { return integer() && deductions() && wide(); }
  } rules;

  class SignedEdges : public Base {
    static void signed_variable(Model& s,int i,std::int64_t l,std::int64_t u) {
      s.x[i]=WordVar(s,64,WDT_SIGNED,static_cast<WordValue>(l),static_cast<WordValue>(u));
    }
  public:
    SignedEdges(void) : Base("Word::ArithmeticGlobals::SignedEdges") {}
    virtual bool run(void) {
      const auto minimum=std::numeric_limits<std::int64_t>::min();
      const auto maximum=std::numeric_limits<std::int64_t>::max();
      {
        Model s(3); signed_variable(s,0,-20,-3); s.range(1,4,4);
        signed_variable(s,2,-40,-20);
        WordVarArgs a,b; a<<s.x[0]<<s.x[1]; b<<s.x[2]; E::product_balance(s,a,1,b,1);
        if (s.status()==SS_FAILED || s.x[0].minimum()!=WordValue(-10) ||
            s.x[0].maximum()!=WordValue(-5) || (s.x[2].hi()&3)!=0) return false;
        std::unique_ptr<Model> clone(static_cast<Model*>(s.clone()));
        clone->range(2,WordValue(-24),WordValue(-24));
        if (clone->status()==SS_FAILED || !clone->x[0].assigned() ||
            clone->x[0].val()!=WordValue(-6) || s.x[0].assigned()) return false;
      }
      {
        Model s(4); signed_variable(s,0,-7,-7); s.range(1,3,3);
        signed_variable(s,2,minimum,maximum); signed_variable(s,3,minimum,maximum);
        WordVarArgs a; a<<s.x[0]; E::product_divmod(s,a,s.x[1],s.x[2],s.x[3]);
        if (s.status()==SS_FAILED || !s.x[2].assigned() || s.x[2].val()!=WordValue(-3) ||
            !s.x[3].assigned() || s.x[3].val()!=2) return false;
      }
      {
        Model s(4); signed_variable(s,0,-4,-4); s.range(1,4,4);
        signed_variable(s,2,minimum,maximum); signed_variable(s,3,minimum,maximum);
        WordVarArgs a; a<<s.x[0]; E::linear_divmod(s,{2},a,-1,s.x[1],s.x[2],s.x[3]);
        if (s.status()==SS_FAILED || !s.x[2].assigned() || s.x[2].val()!=WordValue(-3) ||
            !s.x[3].assigned() || s.x[3].val()!=3) return false;
      }
      {
        Model s(2); signed_variable(s,0,-7,-7); signed_variable(s,1,minimum,maximum);
        E::quantize_up(s,s.x[0],4,0,s.x[1]);
        if (s.status()==SS_FAILED || !s.x[1].assigned() || s.x[1].val()!=WordValue(-4)) return false;
      }
      for (bool fits : {false,true}) {
        Model s(3); signed_variable(s,0,minimum,minimum); signed_variable(s,1,-1,-1);
        if (!fits) signed_variable(s,2,minimum,maximum);
        WordVarArgs a,b; a<<s.x[0]<<s.x[1]; b<<s.x[2]; E::product_balance(s,a,1,b,1);
        bool failed=s.status()==SS_FAILED;
        if (fits ? failed || !s.x[2].assigned() || s.x[2].val()!=(WordValue(1)<<63) : !failed)
          return false;
      }
      {
        Model s(2); signed_variable(s,0,-20,-1); signed_variable(s,1,minimum,maximum);
        WordVarArgs a,b; a<<s.x[0]; b<<s.x[1];
        E::bounded_image(s,a,{3},{2},b);
        if (s.status()==SS_FAILED || s.x[1].minimum()!=WordValue(-58) ||
            s.x[1].maximum()!=WordValue(-1)) return false;
        s.range(1,WordValue(-13),WordValue(-13));
        if (s.status()==SS_FAILED || !s.x[0].assigned() || s.x[0].val()!=WordValue(-5)) return false;
      }
      {
        Model s(1); signed_variable(s,0,-100,-1); WordVarArgs a; a<<s.x[0];
        E::linear_system(s,a,{E::WordLinearRow::congruence({1},2,6),E::WordLinearRow::congruence({1},5,9)});
        if (s.status()==SS_FAILED || s.x[0].minimum()!=WordValue(-94) ||
            s.x[0].maximum()!=WordValue(-4)) return false;
      }
      {
        Model s(1); s.x[0]=WordVar(s,8,WDT_SIGNED,255,255); WordVarArgs a; a<<s.x[0];
        E::linear_system(s,a,{E::WordLinearRow::congruence({1},-1,~WordValue(0))});
        if (s.status()==SS_FAILED) return false;
      }
      return true;
    }
  } signed_edges;

  class Knapsack : public Base {
  public:
    Knapsack(void) : Base("Word::ArithmeticGlobals::IntegralSums") {}
    virtual bool run(void) {
      {
        Model s(3); for (int i=0;i<3;i++) s.range(i,0,1);
        WordVarArgs a(s.x); E::linear_system(s,a,{E::WordLinearRow::equal({3,4,5},6)});
        // Bounds and gcds admit this row; only the selected integral stage
        // detects its missing sum. Default posting must leave that stage off.
        if (s.status()==SS_FAILED) return false;
        for (int i=0;i<3;i++) if (s.x[i].assigned()) return false;
        E::linear_system(s,a,{E::WordLinearRow::equal({3,4,5},6)},IPL_BASIC_ADVANCED);
        if (s.status()!=SS_FAILED) return false;
      }
      for (bool sign : {false,true}) for (bool feasible : {false,true}) {
        Model s(3);
        for (int i=0;i<3;i++)
          if (sign) s.x[i]=WordVar(s,1,WDT_SIGNED,1,0);
          else s.range(i,0,1);
        WordVarArgs a(s.x); int rhs=feasible ? 7 : 6;
        if (sign) rhs=-rhs;
        E::linear_system(s,a,{E::WordLinearRow::equal({3,4,5},rhs)},IPL_BASIC_ADVANCED);
        bool failed=s.status()==SS_FAILED;
        if (!feasible) { if (!failed) return false; continue; }
        if (failed) return false;
        for (int i=0;i<3;i++)
          if (!s.x[i].assigned() || s.x[i].val()!=WordValue(i<2)) return false;
      }
      {
        Model s(4); for (int i=0;i<4;i++) s.range(i,0,1);
        WordVarArgs a(s.x); E::linear_system(s,a,{E::WordLinearRow::range({64,17,23,21},60,70)},
                                            IPL_BASIC_ADVANCED);
        if (s.status()==SS_FAILED) return false;
        // Both 64 and 17+23+21=61 are supported, across the limb boundary.
        for (int i=0;i<4;i++) if (s.x[i].assigned()) return false;
      }
      return true;
    }
  } knapsack;

  class PropagationLevels : public Base {
    class PostTracer : public Tracer {
    public:
      unsigned int posts=0;
      virtual void propagate(const Space&,const PropagateTraceInfo&) {}
      virtual void commit(const Space&,const CommitTraceInfo&) {}
      virtual void post(const Space&,const PostTraceInfo&) { posts++; }
    };
  public:
    PropagationLevels(void) : Base("Word::ArithmeticGlobals::PropagationLevels") {}
    virtual bool run(void) {
      for (IntPropLevel ipl : {IPL_DEF,IPL_VAL,IPL_BND,IPL_DOM,IPL_BASIC,
                              IPL_ADVANCED,IPL_BASIC_ADVANCED,
                              static_cast<IntPropLevel>(IPL_DOM|IPL_ADVANCED)}) {
        bool advanced=(ba(ipl)&IPL_ADVANCED)!=0;
        {
          Model s(4); for (int i=0;i<4;i++) s.range(i,0,1);
          // Each original row admits both values of every variable. Eliminating
          // their common columns exposes 2*w=1 only in the advanced package.
          E::linear_system(s,WordVarArgs(s.x),{E::WordLinearRow::equal({1,1,1,1},2),
                           E::WordLinearRow::equal({1,1,1,-1},1)},ipl);
          if ((s.status()==SS_FAILED)!=advanced) return false;
        }
        {
          Model s(3); for (int i=0;i<3;i++) s.range(i,0,1);
          E::linear_system(s,WordVarArgs(s.x),{E::WordLinearRow::equal({3,4,5},6)},ipl);
          if ((s.status()==SS_FAILED)!=advanced) return false;
        }
      }
      for (int api=0;api<7;api++) {
        PostTracer tracer;
        Model s(4); trace(s,TE_POST,tracer);
        WordVarArgs a,b; a<<s.x[0]; b<<s.x[1];
        unsigned int before=tracer.posts;
        switch (api) {
        case 0: E::product_balance(s,a,1,b,1); break;
        case 1: E::product_divmod(s,a,s.x[1],s.x[2],s.x[3]); break;
        case 2: E::linear_divmod(s,{1},a,0,s.x[1],s.x[2],s.x[3]); break;
        case 3: E::quantize_up(s,s.x[0],4,0,s.x[1]); break;
        case 4: E::linear_system(s,a,{E::WordLinearRow::equal({1},3)}); break;
        case 5: E::mixed_radix(s,a,{4},s.x[1]); break;
        case 6: E::bounded_image(s,a,{1},{0},b); break;
        }
        if (tracer.posts!=before+1) return false;
      }
      return true;
    }
  } propagation_levels;

  class BoundedSoundness : public Base {
  public:
    BoundedSoundness(void) : Base("Word::ArithmeticGlobals::BoundedSoundness") {}
    virtual bool run(void) {
      unsigned long long state=20260916;
      auto next=[&]() -> unsigned int {
        state=state*6364136223846793005ULL+1442695040888963407ULL;
        return static_cast<unsigned int>(state>>32);
      };
      for (Kind kind : {PRODUCT,DIVMOD,LINEAR,RADIX,IMAGE,LINEAR_DIVMOD,QUANTIZE})
        for (unsigned int trial=0;trial<150;trial++) {
          int n=(kind==DIVMOD || kind==LINEAR_DIVMOD) ? 5 :
            (kind==PRODUCT || kind==IMAGE) ? 4 : kind==QUANTIZE ? 2 : 3;
          Model s(n,3);
          for (int i=0;i<n;i++)
            if ((trial/3)%3==1 || ((trial/3)%3==2 && i%2))
              s.x[i]=WordVar(s,3,WDT_SIGNED,4,3);
          bool initial=true;
          for (int i=0;i<n;i++) {
            WordValue l=next()%8,u=next()%8; if (l>u) std::swap(l,u);
            WordValue sign=s.x[i].domain_type()==WDT_SIGNED ? 4 : 0;
            initial&=s.range(i,l^sign,u^sign);
            if (!initial) break;
            WordValue witness=(l+next()%(u-l+1))^sign,free=next()%8;
            WordValue hi=witness|free,lo=witness&~free;
            initial&=s.bits(i,lo,hi);
            if (!initial) break;
          }
          if (!initial) continue;
          bool alias=(trial%3)==0;
          if (alias) s.x[1]=s.x[0];
          WordVarArgs v(s.x);
          std::vector<std::vector<WordValue>> values(n),supports;
          size_t count=1;
          for (int i=0;i<n;i++) {
            for (WordValue v=0;v<8;v++) if (s.x[i].in(v)) values[i].push_back(v);
            count*=values[i].size();
          }
          const int coefficient=static_cast<int>(next()%7)-3;
          const int rhs=static_cast<int>(next()%15)-7;
          const WordValue radix=1+next()%7,modulus=1+next()%9;
          for (size_t t=0;t<count;t++) {
            size_t k=t; std::vector<WordValue> encoded(n); std::vector<int> a(n);
            for (int i=0;i<n;i++) {
              encoded[i]=values[i][k%values[i].size()]; k/=values[i].size();
              a[i]=static_cast<int>(encoded[i]);
              if (v[i].domain_type()==WDT_SIGNED && (a[i]&4)) a[i]-=8;
            }
            if (alias && encoded[0]!=encoded[1]) continue;
            int x=a[0],y=a[1]; bool ok=false;
            switch (kind) {
            case PRODUCT: ok=2*a[0]*a[1]==a[2]*a[3]; break;
            case DIVMOD: ok=a[2]>0 && a[4]>=0 && a[4]<a[2] && a[0]*a[1]==a[2]*a[3]+a[4]; break;
            case LINEAR: {
              int v=coefficient*x-2*y+static_cast<int>(a[2])-rhs;
              ok=v==0 && (x+y-static_cast<int>(radix))%static_cast<int>(modulus)==0;
              break;
            }
            case RADIX: ok=a[0]>=0 && a[0]<3 && a[1]>=0 && a[1]<static_cast<int>(radix) &&
              a[2]==a[0]*static_cast<int>(radix)+a[1]; break;
            case IMAGE: ok=static_cast<int>(a[2])==coefficient*x-y+rhs && a[3]==a[0]+a[1]; break;
            case LINEAR_DIVMOD: ok=a[2]>0 && a[4]>=0 && a[4]<a[2] &&
              rhs+coefficient*x-y==static_cast<int>(a[2]*a[3]+a[4]); break;
            case QUANTIZE: ok=a[1]>=a[0] && a[1]-a[0]<static_cast<int>(radix) &&
              a[1]%static_cast<int>(radix)==0; break;
            default: return false;
            }
            if (ok) supports.push_back(encoded);
          }
          WordVarArgs a,b; IntPropLevel options=trial%2 ? IPL_BASIC_ADVANCED : IPL_BASIC;
          switch (kind) {
          case PRODUCT:
            a<<v[0]<<v[1]; b<<v[2]<<v[3]; E::product_balance(s,a,2,b,1,options); break;
          case DIVMOD:
            a<<v[0]<<v[1]; E::product_divmod(s,a,v[2],v[3],v[4],options); break;
          case LINEAR:
            a<<v[0]<<v[1]<<v[2]; E::linear_system(s,a,
              {E::WordLinearRow::equal({coefficient,-2,1},rhs),E::WordLinearRow::congruence({1,1,0},radix,modulus)},options); break;
          case RADIX:
            a<<v[0]<<v[1]; E::mixed_radix(s,a,{3,radix},v[2],options); break;
          case IMAGE:
            a<<v[0]<<v[1]; b<<v[2]<<v[3];
            E::bounded_image(s,a,{coefficient,-1,1,1},{rhs,0},b,options); break;
          case LINEAR_DIVMOD:
            a<<v[0]<<v[1]; E::linear_divmod(s,{coefficient,-1},a,rhs,v[2],v[3],v[4],options); break;
          case QUANTIZE: E::quantize_up(s,v[0],radix,0,v[1],options); break;
          default: return false;
          }
          bool failed=s.status()==SS_FAILED;
          for (const auto& tuple : supports) {
            if (failed) return false;
            for (int i=0;i<n;i++) if (!s.x[i].in(tuple[i])) return false;
          }
          if (!failed) {
            bool assigned=true;
            for (int i=0;i<n;i++) assigned&=s.x[i].assigned();
            if (assigned && supports.empty()) return false;
          }
        }
      return true;
    }
  } bounded_soundness;


  class Staging : public Base {
    class StageTracer : public Tracer {
    public:
      bool basic=false,advanced=false;
      virtual void propagate(const Space& home,const PropagateTraceInfo& info) {
        if (!info.propagator() || info.status()==PropagateTraceInfo::FAILED) return;
        const auto cost=info.propagator()->cost(home,
          Gecode::Word::WordView::med(Gecode::Word::ME_WORD_DOM)).ac;
        basic|=cost==PropCost::linear(PropCost::LO,4).ac;
        advanced|=cost==PropCost::quadratic(PropCost::HI,4).ac;
      }
      virtual void commit(const Space&,const CommitTraceInfo&) {}
      virtual void post(const Space&,const PostTraceInfo&) {}
    };
  public:
    Staging(void) : Base("Word::ArithmeticGlobals::Staging") {}
    virtual bool run(void) {
      for (unsigned int width : {8U,16U,64U}) {
        Model s(3,width); s.range(2,0,0);
        // The third column keeps the derived row nonconstant at posting.
        // Original rows otherwise require unit bound updates until failure.
        E::linear_system(s,WordVarArgs(s.x),{E::WordLinearRow::equal({1,-1,0},0),
          E::WordLinearRow::equal({1,-1,2},1)},IPL_FULL);
        StatusStatistics stats;
        auto start=std::chrono::steady_clock::now();
        if (s.status(stats)!=SS_FAILED || stats.propagate>4) return false;
        if (opt.log) std::cout << "\nstaging " << width << ' ' << stats.propagate
          << ' ' << std::chrono::duration<double,std::micro>(
            std::chrono::steady_clock::now()-start).count() << '\n';
      }
      {
        Model s(2);
        E::linear_system(s,WordVarArgs(s.x),{E::WordLinearRow::equal({1,-1},0),
          E::WordLinearRow::equal({1,-1},1)},IPL_FULL);
        // A constant derived contradiction can be rejected during posting.
        if (!s.failed()) return false;
      }
      {
        StageTracer tracer;
        Model s(4); for (int i=0;i<4;i++) s.range(i,0,1);
        trace(s,TE_PROPAGATE,tracer);
        E::linear_system(s,WordVarArgs(s.x),{E::WordLinearRow::range({64,17,23,21},60,70)},IPL_FULL);
        if (s.status()==SS_FAILED || !tracer.basic || !tracer.advanced) return false;
      }
      return true;
    }
  } staging;

  class RowPosting : public Base {
  public:
    RowPosting(void) : Base("Word::ArithmeticGlobals::RowPosting") {}
    virtual bool run(void) {
      for (unsigned int n : {128U,256U,512U,1024U}) {
        Model s(1);
        E::WordLinearRowArgs rows;
        for (unsigned int i=0; i<n; i++)
          rows << E::WordLinearRow::congruence({1},0,2);
        auto start=std::chrono::steady_clock::now();
        E::linear_system(s,WordVarArgs(s.x),rows,IPL_FULL);
        if (opt.log) std::cout << "\nposting " << n << ' '
          << std::chrono::duration<double,std::micro>(
            std::chrono::steady_clock::now()-start).count() << '\n';
        if (s.status()==SS_FAILED || (s.x[0].hi()&1)) return false;
      }
      {
        Model s(4); for (int i=0;i<4;i++) s.range(i,0,1);
        // Ineligible rows must not exhaust the equality-pair allowance.
        E::WordLinearRowArgs rows;
        for (unsigned int i=0; i<1024; i++)
          rows << E::WordLinearRow::range({1,0,0,0},0,1);
        rows << E::WordLinearRow::equal({1,1,1,1},2)
             << E::WordLinearRow::equal({1,1,1,-1},1);
        E::linear_system(s,WordVarArgs(s.x),rows,IPL_FULL);
        if (s.status()!=SS_FAILED) return false;
      }
      return true;
    }
  } row_posting;
  class Interpretation : public Base {
  public:
    Interpretation(void) : Base("Word::ArithmeticGlobals::Interpretation") {}
    virtual bool run(void) {
      for (WordDomainType policy : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED}) {
        const bool sign=policy==WDT_SIGNED;
        Model s(1,8);
        s.x[0]=policy==WDT_CUBE ? WordVar(s,8,255,255) :
          WordVar(s,8,policy,255,255);
        E::linear_system(s,WordVarArgs(s.x),
          {E::WordLinearRow::equal({1},sign ? -1 : 255)});
        if (s.status()==SS_FAILED || !s.x[0].assigned() ||
            s.x[0].val()!=255)
          return false;
      }
      {
        Model s(1);
        E::product_balance(s,WordVarArgs(),~WordValue(0),
                           WordVarArgs(s.x),1);
        if (s.status()==SS_FAILED || !s.x[0].assigned() ||
            s.x[0].val()!=~WordValue(0))
          return false;
      }
      {
        Model s(1);
        s.x[0]=WordVar(s,64,WDT_SIGNED);
        E::linear_system(s,WordVarArgs(s.x),
          {E::WordLinearRow::equal({1},-1)});
        if (s.status()==SS_FAILED || !s.x[0].assigned() ||
            s.x[0].val()!=~WordValue(0))
          return false;
      }
      return true;
    }
  } interpretation;
  class Contracts : public Base {
  public:
    Contracts(void) : Base("Word::ArithmeticGlobals::Contracts") {}
    virtual bool run(void) {
      {
        Model s(0); WordVarArgs empty;
        E::product_balance(s,empty,1,empty,2);
        if (s.status()!=SS_FAILED) return false;
      }
      {
        Model s(0); WordVarArgs empty;
        E::linear_system(s,empty,{E::WordLinearRow::equal({},1)});
        if (s.status()!=SS_FAILED) return false;
      }
      {
        Model s(0); WordVarArgs empty;
        E::product_balance(s,empty,1,empty,1);
        if (s.status()==SS_FAILED || PropagatorGroup::all.size(s)!=0) return false;
      }
      for (unsigned int which=0;which<6;which++) {
        Model s(2); WordVarArgs a; a<<s.x[0]; bool caught=false;
        try {
          switch (which) {
          case 0: E::linear_system(s,a,{E::WordLinearRow::congruence({1},0,0)}); break;
          case 1: E::linear_system(s,a,{E::WordLinearRow::equal({1,2},0)}); break;
          case 2: E::mixed_radix(s,a,{0},s.x[1]); break;
          case 3: E::bounded_image(s,a,{1,2},{0},a); break;
          case 4: E::quantize_up(s,s.x[0],0,0,s.x[1]); break;
          case 5: E::quantize_up(s,s.x[0],4,4,s.x[1]); break;
          }
        } catch (const Gecode::Word::OutOfLimits&) { caught=true; }
        if (!caught) return false;
      }
      return true;
    }
  } contracts;
}}}

// STATISTICS: test-word
