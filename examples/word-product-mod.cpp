/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>
#include <cstring>
#include <iostream>

using namespace Gecode;

class ProductModCase : public Space {
public:
  WordVar x, y, result;
  IntVar modulus;
  ProductModCase(bool bounded)
    : x(*this,9,WDT_UNSIGNED,10U,20U),
      y(*this,9,WDT_UNSIGNED,10U,20U),
      result(*this,9,WDT_UNSIGNED,0U,511U), modulus(*this,509,509) {
    if (bounded) {
      product_mod(*this,x,y,modulus,result);
    } else {
      WordVar cx(*this,9), cy(*this,9), cr(*this,9);
      rel(*this,cx,WRT_EQ,x); rel(*this,cy,WRT_EQ,y);
      rel(*this,cr,WRT_EQ,result);
      product_mod(*this,cx,cy,modulus,cr);
    }
    branch(*this,result,WORD_VAL_SPLIT_MIN());
    WordVarArgs operands={x,y};
    branch(*this,operands,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
  }
  ProductModCase(ProductModCase& s) : Space(s) {
    x.update(*this,s.x); y.update(*this,s.y);
    result.update(*this,s.result); modulus.update(*this,s.modulus);
  }
  virtual Space* copy(void) { return new ProductModCase(*this); }
};

int main(int argc, char* argv[]) {
  bool bounded=true;
  if (argc == 3 && std::strcmp(argv[1],"--variant") == 0) {
    if (std::strcmp(argv[2],"bounded") == 0) bounded=true;
    else if (std::strcmp(argv[2],"compact") == 0) bounded=false;
    else return 2;
  } else if (argc != 1) {
    std::cerr << "usage: word-product-mod --variant compact|bounded\n";
    return 2;
  }
  ProductModCase* root=new ProductModCase(bounded);
  DFS<ProductModCase> search(root); delete root;
  unsigned long long int solutions=0, checksum=0;
  while (ProductModCase* solution=search.next()) {
    solutions++;
    checksum += (solution->x.val()<<18) ^ (solution->y.val()<<9) ^
      solution->result.val();
    delete solution;
  }
  const Search::Statistics stats=search.statistics();
  std::cout << "{\"variant\":\"" << (bounded ? "bounded" : "compact")
            << "\",\"solutions\":" << solutions
            << ",\"checksum\":" << checksum
            << ",\"nodes\":" << stats.node
            << ",\"failures\":" << stats.fail
            << ",\"propagations\":" << stats.propagate << "}\n";
  return (solutions == 121U) && (checksum == 476747865ULL) ? 0 : 1;
}
