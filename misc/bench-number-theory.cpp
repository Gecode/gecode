// Copyright (c) 2026 Mikael Zayenz Lagerkvist
// SPDX-License-Identifier: MIT
#include <gecode/int.hh>
#include <gecode/search.hh>
#include <chrono>
#include <iostream>
#include <string>
using namespace Gecode;
class Model : public Space {
public:
  IntVarArray x;
  Model(int n, const std::string& pattern, bool decomposition, bool search)
    : x(*this,n,0,1) {
    if (pattern == "repeated")
      for (int i=1; i<n; i++) x[i]=x[0];
    IntVar y=pattern == "alias" ? x[0] : IntVar(*this,0,search ? 0 : 1);
    if (decomposition) {
      IntVar p=x[0];
      for (int i=1; i<n; i++) {
        IntVar q=i == n-1 ? y : IntVar(*this,0,1);
        mult(*this,p,x[i],q);
        p=q;
      }
    } else product(*this,x,y);
    if (search) branch(*this,x,INT_VAR_NONE(),INT_VAL_MIN());
  }
  Model(Model& s) : Space(s) { x.update(*this,s.x); }
  Space* copy() override { return new Model(*this); }
};
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  int n=std::stoi(argv[1]), repeats=std::stoi(argv[4]);
  std::string pattern=argv[2]; bool decomposition=std::string(argv[3]) == "chain";
  for (int i=0; i<20; i++) { Model m(n,pattern,decomposition,false); m.status(); }
  auto start=std::chrono::steady_clock::now();
  for (int i=0; i<repeats; i++) {
    Model m(n,pattern,decomposition,false);
    if (m.status() == SS_FAILED) return 3;
  }
  double us=std::chrono::duration<double,std::micro>
    (std::chrono::steady_clock::now()-start).count()/repeats;
  unsigned long nodes=0, solutions=0;
  if (n<=12) {
    Model m(n,pattern,decomposition,true);
    DFS<Model> engine(&m);
    while (Model* s=engine.next()) { solutions++; delete s; }
    nodes=engine.statistics().node;
  }
  std::cout << us << ',' << nodes << ',' << solutions << '\n';
}
