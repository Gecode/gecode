/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/minimodel.hh>
#include <gecode/word.hh>
#include <chrono>
#include <cstring>
#include <iostream>

using namespace Gecode;

enum Formulation { F_VAL, F_BND, F_INT };

class RegisterAllocation : public Space {
public:
  Formulation formulation;
  WordVarArray words;
  IntVarArray integers;

  RegisterAllocation(unsigned int per_bank, Formulation f)
    : formulation(f), words(*this,2*per_bank),
      integers(*this,(f == F_INT) ? 2*per_bank : 0,0,127) {
    for (unsigned int i=0; i<2*per_bank; i++) {
      const bool upper=i >= per_bank;
      const WordValue lo=upper ? 0x40U : 0U;
      const WordValue hi=upper ? 0x70U : 0x30U;
      if (f == F_INT) {
        words[i]=WordVar(*this,8,lo,hi);
        integers[i]=IntVar(*this,static_cast<int>(lo),static_cast<int>(hi));
        channel(*this,words[i],integers[i],WDT_UNSIGNED);
      } else {
        words[i]=WordVar(*this,8,lo,hi,WDT_UNSIGNED,lo,hi | 0x0fU);
      }
    }
    if (f == F_INT) {
      distinct(*this,integers,IPL_BND);
      branch(*this,integers,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
    } else {
      distinct(*this,words,(f == F_BND) ? IPL_BND : IPL_VAL);
      branch(*this,words,WORD_VAR_SIZE_MIN(),WORD_VAL_LSB());
    }
  }
  RegisterAllocation(RegisterAllocation& s)
    : Space(s), formulation(s.formulation) {
    words.update(*this,s.words); integers.update(*this,s.integers);
  }
  virtual Space* copy(void) { return new RegisterAllocation(*this); }
  std::uint64_t checksum(void) const {
    std::uint64_t sum=0;
    for (int i=0; i<words.size(); i++)
      sum += static_cast<std::uint64_t>(i+1)*words[i].val();
    return sum;
  }
};

class WideRoot : public Space {
public:
  WordVarArray words;
  IntVarArray integers;
  WideRoot(unsigned int n, Formulation f, unsigned int width)
    : words(*this,n), integers(*this,(f == F_INT) ? n : 0,0,
        (width == 30U) ? ((1<<30)-1) : ((1<<width)-1)) {
    WordValue maximum=(WordValue(1)<<width)-1U;
    for (unsigned int i=0; i<n; i++) {
      words[i]=(f == F_INT) ? WordVar(*this,width) :
        WordVar(*this,width,WDT_UNSIGNED,0,maximum);
      if (f == F_INT)
        channel(*this,words[i],integers[i],WDT_UNSIGNED);
    }
    if (f == F_INT)
      distinct(*this,integers,IPL_BND);
    else
      distinct(*this,words,(f == F_BND) ? IPL_BND : IPL_VAL);
  }
  WideRoot(WideRoot& s) : Space(s) {
    words.update(*this,s.words); integers.update(*this,s.integers);
  }
  virtual Space* copy(void) { return new WideRoot(*this); }
};

int main(int argc, char* argv[]) {
  if (argc != 5) return 2;
  Formulation formulation;
  if (!std::strcmp(argv[2],"value")) formulation=F_VAL;
  else if (!std::strcmp(argv[2],"bounds")) formulation=F_BND;
  else if (!std::strcmp(argv[2],"int-channel")) formulation=F_INT;
  else return 2;
  const unsigned int size=static_cast<unsigned int>(std::strtoul(argv[3],nullptr,10));
  const unsigned int iterations=static_cast<unsigned int>(std::strtoul(argv[4],nullptr,10));
  if ((size == 0U) || (iterations == 0U)) return 2;

  std::uint64_t solutions=0, checksum=0, nodes=0, failures=0, propagations=0;
  auto start=std::chrono::steady_clock::now();
  if (!std::strcmp(argv[1],"register")) {
    for (unsigned int trial=0; trial<iterations; trial++) {
      RegisterAllocation* root=new RegisterAllocation(size,formulation);
      StatusStatistics rs; if (root->status(rs) == SS_FAILED) return 1;
      DFS<RegisterAllocation> search(root); delete root;
      while (RegisterAllocation* solution=search.next()) {
        solutions++; checksum += solution->checksum(); delete solution;
      }
      Search::Statistics s=search.statistics();
      nodes += s.node; failures += s.fail; propagations += rs.propagate+s.propagate;
    }
  } else if (!std::strcmp(argv[1],"wide")) {
    const unsigned int width=(size > 64U) ? 30U : 16U;
    for (unsigned int trial=0; trial<iterations; trial++) {
      WideRoot root(size,formulation,width); StatusStatistics s;
      if (root.status(s) == SS_FAILED) return 1;
      propagations += s.propagate;
      checksum += (formulation == F_INT) ?
        static_cast<std::uint64_t>(root.integers[0].min()+
                                   root.integers[size-1].max()) :
        root.words[0].minimum()+root.words[size-1].maximum();
    }
  } else return 2;
  double seconds=std::chrono::duration<double>(
    std::chrono::steady_clock::now()-start).count();
  std::cout << "{\"status\":\"ok\",\"seconds\":" << seconds
            << ",\"solutions\":" << solutions << ",\"checksum\":" << checksum
            << ",\"nodes\":" << nodes << ",\"failures\":" << failures
            << ",\"propagations\":" << propagations
            << ",\"iterations\":" << iterations << "}\n";
  return 0;
}
