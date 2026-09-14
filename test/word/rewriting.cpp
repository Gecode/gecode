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
#include <gecode/search.hh>
#include <gecode/word/arithmetic.hh>
#include <memory>

namespace Test { namespace Word {

  namespace {
    enum Operation { ADD, SUB, MULT, NEG, DIV, MOD, SDIV, SREM, SMOD,
                     BOTH, CARRY, BORROW, NARY, PRODUCT };

    class RewriteSpace : public Gecode::Space {
    public:
      Gecode::WordVarArray x;
      Gecode::BoolVar flag;
      Gecode::IntVar modulus;
      unsigned int width;
      Gecode::WordDomainType kind;
      RewriteSpace(unsigned int w, Gecode::WordDomainType k)
        : x(*this,4,w,k), flag(*this,0,1), modulus(*this,101,101), width(w), kind(k) {}
      RewriteSpace(RewriteSpace& s) : Gecode::Space(s), width(s.width), kind(s.kind) {
        x.update(*this,s.x); flag.update(*this,s.flag); modulus.update(*this,s.modulus);
      }
      virtual Gecode::Space* copy(void) { return new RewriteSpace(*this); }
      Gecode::WordValue encode(long long value) const {
        return Gecode::WordValue(value) & x[0].mask();
      }
      void bounds(int i, long long lo, long long hi) {
        using namespace Gecode;
        rel(*this,x[i],kind == WDT_SIGNED ? WRT_SGQ : WRT_UGQ,width,encode(lo));
        rel(*this,x[i],kind == WDT_SIGNED ? WRT_SLQ : WRT_ULQ,width,encode(hi));
      }
      bool range(int i, long long lo, long long hi) const {
        return x[i].minimum() == encode(lo) && x[i].maximum() == encode(hi);
      }
      void post(Operation op, bool alias=false) {
        using namespace Gecode;
        WordVar y=alias ? x[0] : x[1];
        switch (op) {
        case ADD: add(*this,x[0],y,x[2]); break;
        case SUB: sub(*this,x[0],y,x[2]); break;
        case MULT: mult(*this,x[0],y,x[2]); break;
        case NEG: neg(*this,x[0],x[2]); break;
        case DIV: div(*this,x[0],y,x[2]); break;
        case MOD: mod(*this,x[0],y,x[2]); break;
        case SDIV: signed_div(*this,x[0],y,x[2]); break;
        case SREM: signed_rem(*this,x[0],y,x[2]); break;
        case SMOD: signed_mod(*this,x[0],y,x[2]); break;
        case BOTH: divmod(*this,x[0],y,x[2],x[3]); break;
        case CARRY: add(*this,x[0],y,x[2],flag); break;
        case BORROW: sub(*this,x[0],y,x[2],flag); break;
        case NARY: {
          WordVarArgs input; input << x[0] << y << x[2];
          add(*this,input,x[3]); break;
        }
        case PRODUCT: product_mod(*this,x[0],y,modulus,x[2]); break;
        }
      }
      void tighten(Operation op) {
        switch (op) {
        case ADD: case CARRY: bounds(0,0,40); bounds(1,0,40); break;
        case SUB: case BORROW: bounds(0,60,100); bounds(1,0,40); break;
        case MULT: case PRODUCT: bounds(0,0,10); bounds(1,0,10); break;
        case NEG: bounds(0,10,40); break;
        case DIV: case MOD: case BOTH: bounds(0,100,120); bounds(1,5,10); break;
        case SDIV: case SREM: case SMOD: bounds(0,-100,-80); bounds(1,5,10); break;
        case NARY: bounds(0,0,20); bounds(1,0,20); bounds(2,0,20); break;
        }
      }
      bool expected(Operation op) const {
        switch (op) {
        case ADD: return range(2,0,80);
        case SUB: return range(2,20,100);
        case MULT: case PRODUCT: return range(2,0,100);
        case NEG: return range(2,-40,-10);
        case DIV: return range(2,10,24);
        case MOD: return range(2,0,9);
        case SDIV: return range(2,-20,-8);
        case SREM: return range(2,-9,0);
        case SMOD: return range(2,0,9);
        case BOTH: return range(2,10,24) && range(3,0,9);
        case CARRY: return range(2,0,80) && flag.zero();
        case BORROW: return range(2,20,100) && flag.zero();
        case NARY: return range(3,0,60);
        }
        return false;
      }
    };

    bool supported(Operation op, Gecode::WordDomainType kind) {
      using namespace Gecode;
      if (op == NEG || op == SDIV || op == SREM || op == SMOD) return kind == WDT_SIGNED;
      if (op == DIV || op == MOD || op == BOTH || op == CARRY || op == BORROW || op == PRODUCT)
        return kind == WDT_UNSIGNED;
      return true;
    }

    class KernelSupport : public Base {
    public:
      KernelSupport(void) : Base("Word::Arithmetic::KernelSupport") {}
      virtual bool run(void) {
        using namespace Gecode;
        using namespace Gecode::Word::Arithmetic;
        RewriteSpace home(2,WDT_UNSIGNED);
        // Every two-bit cube, all five operand partitions, both operations,
        // and each nonempty terminal carry/borrow domain. The oracle checks
        // the exact supported bit projection, not only assigned solutions.
        const int partition[5][3]={{0,1,2},{0,0,2},{0,1,0},{0,1,1},{0,0,0}};
        for (unsigned int a=0; a<9; a++)
          for (unsigned int b=0; b<9; b++)
            for (unsigned int c=0; c<9; c++)
              for (const auto& map : partition)
                for (bool subtract : {false,true})
                  for (unsigned int terminal=1; terminal<=3; terminal++) {
                    BoundLocalDomain d[3];
                    const unsigned int code[3]={a,b,c};
                    for (int i=0; i<3; i++)
                      d[i]=BoundLocalDomain{2,WDT_UNSIGNED,0,3,0,3,true};
                    for (int i=0; i<3; i++) {
                      unsigned int state=code[i];
                      for (unsigned int bit=0; bit<2; bit++,state/=3) {
                        if (state%3 == 0) d[map[i]].hi &= ~(WordValue(1)<<bit);
                        if (state%3 == 1) d[map[i]].lo |= WordValue(1)<<bit;
                      }
                    }
                    bool empty=false;
                    for (int i=0; i<3; i++) empty |= (d[i].lo & ~d[i].hi) != 0;
                    if (empty) continue;
                    WordValue lo[3]={3,3,3}, hi[3]={0,0,0};
                    unsigned int expected_final=0;
                    for (unsigned int x=0; x<4; x++)
                      for (unsigned int y=0; y<4; y++) {
                        const unsigned int z=(subtract ? x-y : x+y)&3U;
                        const unsigned int carry=subtract ? (x<y) : ((x+y)>>2);
                        if ((terminal & (1U<<carry)) == 0) continue;
                        const unsigned int values[3]={x,y,z};
                        bool supported=true;
                        for (int i=0; i<3; i++) {
                          supported &= (values[i]&d[map[i]].lo)==d[map[i]].lo &&
                            (values[i]&~d[map[i]].hi)==0;
                          for (int j=0; j<i; j++)
                            supported &= map[i]!=map[j] || values[i]==values[j];
                        }
                        if (!supported) continue;
                        expected_final |= 1U<<carry;
                        for (int i=0; i<3; i++) {
                          lo[i] &= values[i]; hi[i] |= values[i];
                        }
                      }
                    BoundLocalView x(d[map[0]]), y(d[map[1]]), z(d[map[2]]);
                    unsigned int final=0;
                    const ExecStatus status=subtract ?
                      sub_narrow(home,x,y,z,terminal,final) :
                      add_narrow(home,x,y,z,terminal,final);
                    if ((status == ES_FAILED) != (expected_final == 0)) return false;
                    if (status == ES_FAILED) continue;
                    if (final != expected_final) return false;
                    for (int i=0; i<3; i++)
                      if (d[map[i]].lo != lo[i] || d[map[i]].hi != hi[i]) return false;
                  }
        return true;
      }
    } kernel_support;

    class Rewriting : public Base {
      template<class View>
      static bool nary_aliases(Gecode::WordDomainType kind) {
        using namespace Gecode;
        using namespace Gecode::Word::Arithmetic;
        RewriteSpace s(8,kind);
        s.bounds(0,10,50);
        dom(s,s.x[1],0);
        ViewArray<View> input(s,5);
        input[0]=input[2]=input[4]=View(s.x[1]);
        input[1]=input[3]=View(s.x[0]);
        View result(s.x[3]);
        // Exactly two unassigned aliases remain among assigned entries.
        if (!nary_add_shared(input,result)) return false;
        input[3]=View(s.x[2]);
        if (nary_add_shared(input,result)) return false;
        if (!nary_add_shared(input,View(s.x[0]))) return false;
        input[3]=View(s.x[0]);
        if (BoundNaryAdd<View>::post(s,input,result,0) == ES_FAILED ||
            s.status() == SS_FAILED || !s.range(3,20,100)) return false;
        std::unique_ptr<RewriteSpace> clone(static_cast<RewriteSpace*>(s.clone()));
        clone->bounds(3,100,100);
        if (clone->status() == SS_FAILED || !clone->range(0,50,50) ||
            !s.range(0,10,50)) return false;
        const WordValue lo=clone->x[0].lo(), hi=clone->x[0].hi();
        StatusStatistics repeated;
        clone->status(repeated);
        return repeated.propagate == 0 && clone->x[0].lo() == lo &&
          clone->x[0].hi() == hi;
      }
      bool lifecycle(void) const {
        using namespace Gecode;
        for (unsigned int width : {8U,16U,64U})
          for (WordDomainType kind : {WDT_UNSIGNED,WDT_SIGNED})
            for (int operation=ADD; operation<=PRODUCT; operation++) {
              const Operation op=static_cast<Operation>(operation);
              if (!supported(op,kind)) continue;
              for (bool early : {false,true}) for (bool clone : {false,true}) {
                std::unique_ptr<RewriteSpace> s(new RewriteSpace(width,kind));
                if (early) s->tighten(op);
                s->post(op);
                if (s->status() == SS_FAILED) return false;
                if (clone) s.reset(static_cast<RewriteSpace*>(s->clone()));
                if (!early) s->tighten(op);
                if (s->status() == SS_FAILED || !s->expected(op)) return false;
                StatusStatistics repeated; s->status(repeated);
                if (repeated.propagate != 0) return false;
              }
            }
        return true;
      }
      bool bound_events(void) const {
        using namespace Gecode;
        RewriteSpace a(8,WDT_UNSIGNED); a.post(ADD); a.status();
        a.bounds(0,0,100); a.bounds(1,0,100);
        if (a.status() == SS_FAILED || !a.range(2,0,200)) return false;
        const WordValue hi=a.x[0].hi();
        a.bounds(0,0,90); a.bounds(1,0,90);
        if (a.x[0].hi() != hi || a.status() == SS_FAILED || !a.range(2,0,180)) return false;
        RewriteSpace n(8,WDT_SIGNED); n.post(NEG); n.status();
        n.bounds(0,-127,127);
        if (n.status() == SS_FAILED || !n.range(2,-127,127)) return false;
        for (Operation op : {DIV,MOD,BOTH}) {
          RewriteSpace d(8,WDT_UNSIGNED); d.post(op); d.status();
          d.bounds(0,0,100); d.status();
          d.bounds(1,1,255);
          if (d.status() == SS_FAILED || d.x[2].maximum() > 100) return false;
        }
        return true;
      }
      bool corners(void) const {
        using namespace Gecode;
        // Each contradiction survives the cube filter and fails in a numeric rewrite.
        for (Operation op : {ADD,SUB,MULT}) {
          RewriteSpace s(8,WDT_UNSIGNED); s.post(op); s.status();
          if (op == ADD) { s.bounds(0,0,40); s.bounds(1,0,40); s.bounds(2,81,100); }
          else if (op == SUB) { s.bounds(0,100,140); s.bounds(1,50,60); s.bounds(2,91,100); }
          else { s.bounds(0,0,10); s.bounds(1,0,10); s.bounds(2,101,120); }
          if (s.status() != SS_FAILED) return false;
        }
        for (Operation op : {ADD,SUB,MULT,DIV,MOD,SDIV,SREM,SMOD,BOTH}) {
          const WordDomainType kind=(op == SDIV || op == SREM || op == SMOD) ? WDT_SIGNED : WDT_UNSIGNED;
          RewriteSpace root(8,kind); root.post(op,true); root.status();
          std::unique_ptr<RewriteSpace> s(static_cast<RewriteSpace*>(root.clone()));
          s->bounds(0,3,3);
          if (s->status() == SS_FAILED || !s->x[2].assigned()) return false;
          const WordValue expected=(op == ADD) ? 6 : (op == MULT) ? 9 :
            (op == DIV || op == SDIV || op == BOTH) ? 1 : 0;
          if (s->x[2].val() != expected) return false;
          if (op == BOTH && (!s->x[3].assigned() || s->x[3].val() != 0)) return false;
        }
        for (Operation op : {DIV,MOD,SDIV,SREM,SMOD,BOTH}) {
          WordDomainType kind=(op == SDIV || op == SREM || op == SMOD) ? WDT_SIGNED : WDT_UNSIGNED;
          RewriteSpace s(64,kind); s.post(op); s.status(); s.bounds(0,13,13); s.bounds(1,0,0);
          if (s.status() == SS_FAILED || !s.x[2].assigned()) return false;
          WordValue expected=(op == DIV || op == SDIV || op == BOTH) ? ~WordValue(0) : 13;
          if (s.x[2].val() != expected || (op == BOTH && s.x[3].val() != 13)) return false;
        }
        RewriteSpace minimum(64,WDT_SIGNED); minimum.post(NEG); minimum.status();
        rel(minimum,minimum.x[0],WRT_EQ,64,WordValue(1)<<63);
        if (minimum.status() == SS_FAILED || minimum.x[2].val() != (WordValue(1)<<63)) return false;
        for (Operation op : {CARRY,BORROW}) {
          RewriteSpace s(8,WDT_UNSIGNED); s.post(op); s.status(); rel(s,s.flag,IRT_EQ,1);
          if (op == CARRY) { s.bounds(0,200,220); s.bounds(1,100,110); }
          else { s.bounds(0,10,20); s.bounds(1,100,120); }
          if (s.status() == SS_FAILED || !s.range(2,op == CARRY ? 44 : 146,op == CARRY ? 74 : 176)) return false;
        }
        // Historical constants wrap, but their canonical folded constant is 44.
        RewriteSpace folded(8,WDT_UNSIGNED);
        WordVarArgs input; input << WordVar(folded,8,WDT_UNSIGNED,200,200)
          << WordVar(folded,8,WDT_UNSIGNED,100,100) << folded.x[0];
        add(folded,input,folded.x[2]); folded.status(); folded.bounds(0,0,10);
        if (folded.status() == SS_FAILED || !folded.range(2,44,54)) return false;
        return true;
      }
      bool nary_fixpoint(void) const {
        using namespace Gecode;
        unsigned long long state=20260916U;
        const auto next=[&state]() -> unsigned int {
          state=state*6364136223846793005ULL+1442695040888963407ULL;
          return static_cast<unsigned int>(state >> 32);
        };
        // Reposting the same local filter after status must find no missed
        // bit deductions created by the preceding numeric synchronization.
        for (unsigned int trial=0; trial<600; trial++) {
          RewriteSpace s(8,WDT_UNSIGNED);
          for (int i=0; i<3; i++) {
            const unsigned int lo=next()%16, hi=lo+next()%(32-lo);
            s.bounds(i,lo,hi);
            const WordValue fixed=next()&3U;
            dom(s,s.x[i],fixed,63U);
          }
          s.bounds(3,next()%16,16+next()%80);
          if (s.failed()) continue;
          const bool alias=(trial%3)==0;
          s.post(NARY,alias);
          if (s.status() == SS_FAILED) continue;
          WordValue before[4][4];
          for (int i=0; i<4; i++) {
            before[i][0]=s.x[i].lo(); before[i][1]=s.x[i].hi();
            before[i][2]=s.x[i].minimum(); before[i][3]=s.x[i].maximum();
          }
          s.post(NARY,alias);
          if (s.status() == SS_FAILED) return false;
          for (int i=0; i<4; i++)
            if (before[i][0]!=s.x[i].lo() || before[i][1]!=s.x[i].hi() ||
                before[i][2]!=s.x[i].minimum() || before[i][3]!=s.x[i].maximum())
              return false;
        }
        return true;
      }
    public:
      Rewriting(void) : Base("Word::Arithmetic::RewritingLifecycle") {}
      virtual bool run(void) {
        return lifecycle() && bound_events() && corners() && nary_fixpoint() &&
          nary_aliases<Gecode::Word::UnsignedWordView>(Gecode::WDT_UNSIGNED) &&
          nary_aliases<Gecode::Word::SignedWordView>(Gecode::WDT_SIGNED);
      }
    } rewriting;
  }
}}

// STATISTICS: test-word
