/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

using namespace Gecode;

class ScatterGather : public Space {
  static WordVarArray lengths(Space& home, int n, WordDomainType kind) {
    return kind == WDT_CUBE ? WordVarArray(home,n,12,0U,0xfffU) :
      WordVarArray(home,n,12,kind,64U,256U);
  }
  static WordVar sum(Space& home, WordValue v, WordDomainType kind) {
    return kind == WDT_CUBE ? WordVar(home,12,v,v) :
      WordVar(home,12,kind,v,v);
  }
public:
  WordVarArray length;
  WordVar total;
  ScatterGather(int n, WordValue value, WordDomainType kind)
    : length(lengths(*this,n,kind)), total(sum(*this,value,kind)) {
    for (int i=0; i<n; i++) {
      dom(*this,length[i],0U,0xff0U);
      if (kind == WDT_CUBE) {
        rel(*this,length[i],WRT_UGQ,12,64U);
        rel(*this,length[i],WRT_ULQ,12,256U);
      }
    }
    for (int i=1; i<n; i++)
      rel(*this,length[i-1],WRT_ULQ,length[i]);
    WordVarArgs a(n);
    for (int i=0; i<n; i++) a[i]=length[i];
    add(*this,a,total);
    branch(*this,length,WORD_VAR_NONE(),
           kind == WDT_CUBE ? WORD_VAL_LSB() : WORD_VAL_SPLIT_MIN());
  }
  ScatterGather(ScatterGather& s) : Space(s) {
    length.update(*this,s.length); total.update(*this,s.total);
  }
  Space* copy(void) { return new ScatterGather(*this); }
};

int main(int argc, char* argv[]) {
  int n=4; WordValue total=0; bool total_set=false;
  WordDomainType kind=WDT_UNSIGNED;
  for (int i=1; i<argc; i++) {
    if ((std::strcmp(argv[i],"--segments") == 0) && (++i<argc))
      n=std::atoi(argv[i]);
    else if ((std::strcmp(argv[i],"--variant") == 0) && (++i<argc))
      kind=(std::strcmp(argv[i],"compact") == 0) ? WDT_CUBE : WDT_UNSIGNED;
    else if ((std::strcmp(argv[i],"--total") == 0) && (++i<argc)) {
      total=static_cast<WordValue>(std::strtoull(argv[i],nullptr,10));
      total_set=true;
    }
    else { std::cerr << "usage: word-nary-add --segments 4|6|8 --variant compact|bounded [--total value]\n"; return 2; }
  }
  if (((n != 4) && (n != 6) && (n != 8)) || (total > 0xfffU)) return 2;
  if (!total_set)
    total=static_cast<WordValue>(160*n);
  ScatterGather* root=new ScatterGather(n,total,kind);
  DFS<ScatterGather> search(root); delete root;
  unsigned long long count=0, checksum=0;
  std::vector<WordValue> first;
  while (ScatterGather* s=search.next()) {
    count++;
    for (int i=0; i<n; i++) {
      checksum += static_cast<unsigned long long>(i+1)*s->length[i].val();
      if (count == 1) first.push_back(s->length[i].val());
    }
    delete s;
  }
  Search::Statistics stats=search.statistics();
  std::cout << "{\"schema_version\":1,\"status\":\"ok\",\"segments\":"
            << n << ",\"total\":" << total << ",\"variant\":\""
            << (kind == WDT_CUBE ? "compact" : "bounded")
            << "\",\"solutions\":" << count << ",\"checksum\":" << checksum
            << ",\"semantic_status\":\"" << (count ? "sat" : "unsat") << "\""
            << ",\"decision_variables\":[";
  for (int i=0; i<n; i++) {
    if (i != 0) std::cout << ',';
    std::cout << "\"length[" << i << "]\"";
  }
  std::cout << "],\"projections\":[";
  if (!first.empty()) {
    std::cout << '[';
    for (std::size_t i=0; i<first.size(); i++) {
      if (i != 0) std::cout << ',';
      std::cout << first[i];
    }
    std::cout << ']';
  }
  std::cout << "]"
            << ",\"nodes\":" << stats.node << ",\"failures\":" << stats.fail
            << ",\"propagations\":" << stats.propagate << "}\n";
  return 0;
}
