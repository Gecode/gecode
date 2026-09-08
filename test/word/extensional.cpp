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
#include <algorithm>
#include <memory>

namespace Test { namespace Word { namespace Extensional {
  using namespace Gecode;
  typedef std::vector<std::vector<WordValue>> Rows;

  class TableSpace : public Space {
  public:
    WordVarArray x;
    TableSpace(const std::vector<unsigned int>& widths, const Rows& rows,
               const std::vector<WordDomainType>& kinds, bool alias=false)
      : x(*this,static_cast<int>(widths.size())) {
      for (int i=0; i<x.size(); i++) x[i]=WordVar(*this,widths[i],kinds[i]);
      if (alias) x[1]=x[0];
      extensional(*this,x,WordTupleSet(widths,rows));
    }
    TableSpace(TableSpace& s) : Space(s) { x.update(*this,s.x); }
    Space* copy(void) { return new TableSpace(*this); }
  };

  class Relation : public Test {
  public:
    Relation(void) : Test("Extensional::Multiply",3,Domain(3,0,7)) {}
    bool solution(const Assignment& a) const { return a[2] == ((a[0]*a[1])&7); }
    void post(Space& home, WordVarArray& x) {
      Rows rows;
      for (WordValue a=0; a<8; a++)
        for (WordValue b=0; b<8; b++) rows.push_back({a,b,(a*b)&7});
      extensional(home,x,WordTupleSet({3,3,3},rows));
    }
  } relation;

  class Hull : public Base {
    static bool check(TableSpace& s, const Rows& rows) {
      Rows compatible;
      for (const auto& row : rows) {
        bool ok=true;
        for (int i=0; i<s.x.size(); i++) {
          ok=ok && s.x[i].in(row[i]);
          for (int j=0; j<i; j++)
            if (Gecode::Word::WordView(s.x[i]) == Gecode::Word::WordView(s.x[j]))
              ok=ok && (row[i] == row[j]);
        }
        if (ok) compatible.push_back(row);
      }
      if (s.status() == SS_FAILED) return compatible.empty();
      if (compatible.empty()) return false;
      for (int i=0; i<s.x.size(); i++) {
        WordValue lo=~WordValue(0), hi=0, first=~WordValue(0), last=0;
        // Independent rank mapping: flip the sign bit for signed order.
        WordValue flip=s.x[i].domain_type()==WDT_SIGNED ?
          WordValue(1)<<(s.x[i].width()-1) : 0;
        for (const auto& row : compatible) {
          lo &= row[i]; hi |= row[i];
          first=std::min(first,row[i]^flip); last=std::max(last,row[i]^flip);
        }
        if ((s.x[i].lo()!=lo) || (s.x[i].hi()!=hi)) return false;
        if (s.x[i].bounded() &&
            ((s.x[i].minimum() != (first^flip)) ||
             (s.x[i].maximum() != (last^flip)))) return false;
      }
      return true;
    }
    static bool oracle(void) {
      Rows rows;
      // Multiple bitmap blocks, sparse holes, and duplicate rows.
      for (WordValue a=0; a<8; a++)
        for (WordValue b=0; b<8; b++)
          for (WordValue c=0; c<8; c++)
            if (((a*3+b*5+c*7)%5)==0) rows.push_back({a,b,c});
      rows.push_back(rows.front());
      unsigned int state=42;
      auto next=[&]() { state=1664525U*state+1013904223U; return state; };
      for (int types=0; types<27; types++) {
        std::vector<WordDomainType> kinds;
        int k=types;
        for (int i=0; i<3; i++,k/=3)
          kinds.push_back(k%3==0 ? WDT_CUBE : k%3==1 ? WDT_UNSIGNED : WDT_SIGNED);
        for (int trial=0; trial<24; trial++) {
          TableSpace s({3,3,3},rows,kinds,trial%4==0);
          if (!check(s,rows)) return false;
          for (int step=0; step<5; step++) {
            if (s.failed()) break;
            std::unique_ptr<TableSpace> clone(static_cast<TableSpace*>(s.clone()));
            int i=next()%3;
            auto prune=[&](TableSpace& target, bool alternate) {
              Gecode::Word::WordView v(target.x[i]);
              ModEvent me;
              if (v.bounded() && step%2==0) {
                WordValue middle=(v.rank_minimum()+v.rank_maximum())/2;
                me=v.narrow_rank_range(target,alternate ? middle : v.rank_minimum(),
                                       alternate ? v.rank_maximum() : middle);
              } else {
                WordValue bit=WordValue(1)<<(next()%3);
                me=v.narrow(target,alternate ? v.lo()|bit : v.lo(),
                           alternate ? v.hi() : v.hi()&~bit);
              }
              if (me_failed(me)) target.fail();
            };
            prune(*clone,true);
            if (!clone->failed() && !check(*clone,rows)) return false;
            prune(s,false);
            if (!s.failed() && !check(s,rows)) return false;
          }
        }
      }
      return true;
    }
    static bool exhaustive(void) {
      // Every pair of width-2 cubes, for all nine domain-type pairs.
      Rows rows={{0,0},{0,3},{1,2},{2,1},{3,0},{3,3}};
      for (WordDomainType a : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED})
        for (WordDomainType b : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED})
          for (PartialDomains p(2); p(); ++p)
            for (PartialDomains q(2); q(); ++q) {
              TableSpace s({2,2},rows,{a,b});
              dom(s,s.x[0],p.domain().lo(),p.domain().hi());
              dom(s,s.x[1],q.domain().lo(),q.domain().hi());
              if (!check(s,rows)) return false;
            }
      // Changes to an interval without any change to its cube must wake us.
      Rows bounds={{0,0},{3,1},{4,2},{7,3}};
      TableSpace s({3,2},bounds,{WDT_UNSIGNED,WDT_CUBE});
      if (!check(s,bounds)) return false;
      Gecode::Word::WordView v(s.x[0]);
      if (me_failed(v.narrow_rank_range(s,1,6)) || v.lo()!=0 || v.hi()!=7)
        return false;
      return check(s,bounds) && s.x[0].minimum()==3 && s.x[0].maximum()==4;
    }
    static bool edges(void) {
      Rows wide={{0,0},{WordValue(1)<<63,1},{~WordValue(0),3}};
      TableSpace s({64,2},wide,{WDT_SIGNED,WDT_UNSIGNED});
      if (!check(s,wide)) return false;
      dom(s,s.x[1],1);
      if (!check(s,wide) || !s.x[0].assigned() ||
          s.x[0].val() != (WordValue(1)<<63)) return false;
      TableSpace empty({1},{},{WDT_CUBE});
      if (empty.status()!=SS_FAILED) return false;
      TableSpace false0({}, {}, {}), true0({}, {{}}, {});
      if (false0.status()!=SS_FAILED || true0.status()==SS_FAILED) return false;
      try { WordTupleSet bad({0},{{0}}); return false; }
      catch (const Gecode::Word::OutOfLimits&) {}
      try { WordTupleSet bad({1},{{2}}); return false; }
      catch (const Gecode::Word::OutOfLimits&) {}
      try { WordTupleSet bad({1},{{}}); return false; }
      catch (const Gecode::Word::OutOfLimits&) {}
      try { extensional(s,s.x,WordTupleSet({64},{{0}})); return false; }
      catch (const Gecode::Word::OutOfLimits&) {}
      try { extensional(s,s.x,WordTupleSet({63,2},{{0,0}})); return false; }
      catch (const Gecode::Word::WidthMismatch&) {}
      return true;
    }
    static bool search(void) {
      Rows rows;
      for (WordValue a=0; a<16; a++)
        for (WordValue b=0; b<16; b++) rows.push_back({a,b,(a+b)&15});
      for (unsigned int distance : {1U,8U}) {
        TableSpace root({4,4,4},rows,{WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED});
        branch(root,root.x,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
        Search::Options options; options.c_d=distance; options.a_d=distance;
        DFS<TableSpace> dfs(&root,options);
        Rows found;
        while (TableSpace* solution=dfs.next()) {
          std::unique_ptr<TableSpace> guard(solution);
          if (PropagatorGroup::all.size(*solution)!=0) return false;
          found.push_back({solution->x[0].val(),solution->x[1].val(),solution->x[2].val()});
        }
        std::sort(found.begin(),found.end());
        if (found != rows) return false;
      }
      return true;
    }
  public:
    Hull(void) : Base("Word::Extensional::Hull") {}
    bool run(void) { return exhaustive() && oracle() && edges() && search(); }
  } hull;
}}}

// STATISTICS: test-word
