/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/minimodel.hh>
#include <gecode/word.hh>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

using namespace Gecode;

enum Formulation { F_VAL, F_BND, F_INT };

class RegisterAllocation : public Space {
public:
  Formulation formulation;
  WordVarArray words;
  IntVarArray integers;

  RegisterAllocation(unsigned int per_bank, unsigned int slots, Formulation f)
    : formulation(f), words(*this,2*per_bank),
      integers(*this,(f == F_INT) ? 2*per_bank : 0,0,127) {
    for (unsigned int i=0; i<2*per_bank; i++) {
      const bool upper=i >= per_bank;
      const WordValue lo=upper ? 0x40U : 0U;
      const WordValue hi=lo+0x10U*(slots-1U);
      const WordValue cube_hi=lo | 0x30U;
      if (f == F_INT) {
        words[i]=WordVar(*this,8,lo,cube_hi);
        integers[i]=IntVar(*this,static_cast<int>(lo),static_cast<int>(hi));
        channel(*this,words[i],integers[i],WDT_UNSIGNED);
      } else {
        words[i]=WordVar(*this,8,lo,cube_hi,WDT_UNSIGNED,lo,cube_hi | 0x0fU);
      }
      rel(*this,words[i],WRT_ULQ,8,hi);
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
  std::vector<unsigned int> projection(void) const {
    std::vector<unsigned int> values;
    for (int i=0; i<words.size(); i++)
      values.push_back(static_cast<unsigned int>(words[i].val()));
    return values;
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
  if ((argc < 5) || (argc > 7)) return 2;
  Formulation formulation;
  if (!std::strcmp(argv[2],"value")) formulation=F_VAL;
  else if (!std::strcmp(argv[2],"bounds")) formulation=F_BND;
  else if (!std::strcmp(argv[2],"int-channel")) formulation=F_INT;
  else return 2;
  const unsigned int size=static_cast<unsigned int>(std::strtoul(argv[3],nullptr,10));
  const unsigned int iterations=static_cast<unsigned int>(std::strtoul(argv[4],nullptr,10));
  const unsigned int slots=(argc >= 6) ?
    static_cast<unsigned int>(std::strtoul(argv[5],nullptr,10)) : 4U;
  const bool projections=(argc == 7) && !std::strcmp(argv[6],"projections");
  if ((size == 0U) || (iterations == 0U) ||
      (slots == 0U) || (slots > 4U) ||
      ((argc == 7) && !projections)) return 2;

  std::uint64_t solutions=0, checksum=0, nodes=0, failures=0, propagations=0;
  auto start=std::chrono::steady_clock::now();
  if (!std::strcmp(argv[1],"register")) {
    std::vector<std::vector<unsigned int> > rows;
    for (unsigned int trial=0; trial<iterations; trial++) {
      RegisterAllocation* root=new RegisterAllocation(size,slots,formulation);
      StatusStatistics rs;
      const SpaceStatus root_status=root->status(rs);
      DFS<RegisterAllocation> search(root_status == SS_FAILED ? nullptr : root);
      delete root;
      while (RegisterAllocation* solution=search.next()) {
        solutions++; checksum += solution->checksum();
        if (projections && (trial == 0U)) rows.push_back(solution->projection());
        delete solution;
      }
      Search::Statistics s=search.statistics();
      nodes += s.node; failures += s.fail; propagations += rs.propagate+s.propagate;
    }
    std::cout << "{\"status\":\"ok\",\"semantic_status\":\""
              << (solutions ? "sat" : "unsat") << "\",\"solutions\":" << solutions
              << ",\"decision_variables\":[";
    for (unsigned int i=0; i<2U*size; i++) {
      if (i) std::cout << ',';
      std::cout << "\"address[" << i << "]\"";
    }
    std::cout << "],\"projections\":[";
    for (std::size_t i=0; i<rows.size(); i++) {
      if (i) std::cout << ',';
      std::cout << '[';
      for (std::size_t j=0; j<rows[i].size(); j++) {
        if (j) std::cout << ',';
        std::cout << rows[i][j];
      }
      std::cout << ']';
    }
    double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
    std::cout << "],\"seconds\":" << seconds << ",\"checksum\":" << checksum
              << ",\"nodes\":" << nodes << ",\"failures\":" << failures
              << ",\"propagations\":" << propagations
              << ",\"iterations\":" << iterations << "}\n";
    return 0;
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
