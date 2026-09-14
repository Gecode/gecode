/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/word.hh>
#include <algorithm>
#include <chrono>
#include <iostream>

using namespace Gecode;

// A deliberately simple full-row-scan reference. Both paths compute the
// exact representable hull; tuple index construction is timed separately.
class Model : public Space {
public:
  WordVarArray x;
  Model(unsigned int width, WordDomainType kind, const WordTupleSet& table,
        bool compact, unsigned int seed) : x(*this,3,width,kind) {
    WordValue mask=(WordValue(1)<<width)-1;
    WordValue bits=mask & 0x55;
    WordValue value=(seed*37)&bits;
    dom(*this,x[0],value,mask & ~(bits^value));
    if (compact) extensional(*this,x,table);
  }
  Model(Model& s) : Space(s) { x.update(*this,s.x); }
  Space* copy(void) { return new Model(*this); }
  void scan(const WordTupleSet& table) {
    WordValue lo[3]={~WordValue(0),~WordValue(0),~WordValue(0)}, hi[3]={0,0,0};
    WordValue first[3]={~WordValue(0),~WordValue(0),~WordValue(0)}, last[3]={0,0,0};
    bool found=false;
    for (int t=0; t<table.tuples(); t++) {
      if (!x[0].in(table.value(t,0)) || !x[1].in(table.value(t,1)) ||
          !x[2].in(table.value(t,2))) continue;
      found=true;
      for (int i=0; i<3; i++) {
        WordValue v=table.value(t,i);
        lo[i] &= v; hi[i] |= v;
        WordValue r=v ^ (x[i].domain_type()==WDT_SIGNED ?
                          WordValue(1)<<(x[i].width()-1) : 0);
        first[i]=std::min(first[i],r); last[i]=std::max(last[i],r);
      }
    }
    if (!found) { fail(); return; }
    for (int i=0; i<3; i++) {
      dom(*this,x[i],lo[i],hi[i]);
      Word::WordView v(x[i]);
      if (v.bounded() && me_failed(v.narrow_rank_range(*this,first[i],last[i])))
        fail();
    }
  }
};

int main(void) {
  using Clock=std::chrono::steady_clock;
  for (unsigned int width : {4U,6U,8U}) {
    WordValue size=WordValue(1)<<width;
    std::vector<std::vector<WordValue>> rows;
    for (WordValue a=0; a<size; a++)
      for (WordValue b=0; b<size; b++) rows.push_back({a,b,(a*b)&(size-1)});
    auto start=Clock::now();
    WordTupleSet table({width,width,width},rows);
    auto setup=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-start).count();
    for (WordDomainType kind : {WDT_CUBE,WDT_UNSIGNED,WDT_SIGNED}) {
      // Check identical root hulls before collecting times.
      for (unsigned int seed=0; seed<16; seed++) {
        Model compact(width,kind,table,true,seed), scan(width,kind,table,false,seed);
        scan.scan(table);
        if (compact.status()!=scan.status()) return 1;
        for (int i=0; i<3; i++) {
          if (compact.x[i].lo()!=scan.x[i].lo() || compact.x[i].hi()!=scan.x[i].hi()) return 1;
          if (compact.x[i].bounded() &&
              (compact.x[i].minimum()!=scan.x[i].minimum() ||
               compact.x[i].maximum()!=scan.x[i].maximum())) return 1;
        }
      }
      for (int trial=0; trial<5; trial++)
        for (int form=0; form<2; form++) {
          bool compact=(form==(trial%2));
          start=Clock::now();
          for (unsigned int seed=0; seed<256; seed++) {
            Model model(width,kind,table,compact,seed);
            if (!compact) model.scan(table);
            if (model.status()==SS_FAILED) return 1;
          }
          auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-start).count();
          std::cout << "{\"width\":" << width << ",\"domain\":\""
                    << (kind==WDT_CUBE ? "cube" : kind==WDT_UNSIGNED ? "unsigned" : "signed")
                    << "\",\"compact\":" << (compact ? "true" : "false")
                    << ",\"trial\":" << trial << ",\"models\":256,\"setup_us\":" << setup
                    << ",\"elapsed_us\":" << elapsed << "}\n";
        }
    }
  }
}
