/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>

#include <iostream>
#include <string>
#include <vector>

using namespace Gecode;

namespace {
  struct Case {
    const char* id; const char* operation; unsigned int width;
    WordValue xmin, xmax, ymin, ymax, zmin, zmax, rmin, rmax;
    int modulus; bool is_signed; bool guard; int overflow;
  };

  const Case cases[] = {
    {"mult-free-w4","mult",4,0,15,0,15,6,6,0,0,0,false,true,0},
    {"mult-tight-w8","mult",8,0,31,13,13,143,143,0,0,0,false,true,0},
    {"mult-signed-min-neg1-w8","mult",8,128,128,255,255,128,128,0,0,0,true,true,1},
    {"mult-wrap-w64","mult",64,~WordValue(0),~WordValue(0),~WordValue(0),~WordValue(0),1,1,0,0,0,false,true,2},
    {"divmod-free-w3","divmod",3,0,7,0,7,0,7,0,7,0,false,true,0},
    {"divmod-tight-w8","divmod",8,100,140,7,9,15,15,1,1,0,false,true,0},
    {"divmod-zero-w64","divmod",64,~WordValue(0)-1,~WordValue(0)-1,0,0,~WordValue(0),~WordValue(0),~WordValue(0)-1,~WordValue(0)-1,0,false,true,0},
    {"divmod-signed-min-neg1-w8","divmod",8,128,128,255,255,128,128,0,0,0,true,true,3},
    {"divmod-signed-zero-w8","divmod",8,128,128,0,0,1,1,128,128,0,true,true,0},
    {"product-mod-free-w5","product_mod",5,0,31,0,7,0,0,0,0,5,false,true,0},
    {"product-mod-tight-w5","product_mod",5,0,31,7,7,4,4,0,0,13,false,true,0},
    {"product-mod-wide-w64","product_mod",64,~WordValue(0),~WordValue(0),~WordValue(0),~WordValue(0),225,225,0,0,2147483646,false,true,0},
    {"product-mod-wrong-w8","product_mod",8,7,7,7,7,5,5,0,0,13,false,true,0},
    {"product-mod-disabled","product_mod",8,2,2,3,3,4,4,0,0,13,false,false,0},
    {"product-mod-disabled-zero","product_mod",8,2,2,3,3,4,4,0,0,0,false,false,0}
  };

  class Model : public Space {
  public:
    WordVar x, y, z, r;
    Model(const Case& c, bool lsb)
      : x(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.xmin,c.xmax),
        y(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.ymin,c.ymax),
        z(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.zmin,c.zmax),
        r(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.rmin,c.rmax) {
      if (std::string(c.operation) == "mult") mult(*this,x,y,z);
      else if (std::string(c.operation) == "divmod") {
        if (c.is_signed) { signed_div(*this,x,y,z); signed_rem(*this,x,y,r); }
        else divmod(*this,x,y,z,r);
      } else {
        IntVar modulus(*this,c.modulus,c.modulus);
        BoolVar guard(*this,c.guard ? 1 : 0,c.guard ? 1 : 0);
        product_mod(*this,x,y,modulus,z,Reify(guard,RM_IMP));
      }
      if (c.overflow != 0) {
        BoolVar flag(*this,1,1);
        const WordOverflowType type = c.overflow == 1 ? WOF_MULT_SIGNED :
          (c.overflow == 2 ? WOF_MULT_UNSIGNED : WOF_DIV_SIGNED);
        overflow(*this,x,type,y,flag);
      }
      WordVarArgs variables = std::string(c.operation) == "divmod" ?
        WordVarArgs({x,y,z,r}) : WordVarArgs({x,y,z});
      if (lsb)
        branch(*this,variables,WORD_VAR_NONE(),WORD_VAL_LSB());
      else
        branch(*this,variables,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    }
    Model(Model& s) : Space(s) {
      x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z); r.update(*this,s.r);
    }
    Space* copy(void) { return new Model(*this); }
  };
}

int main(int argc, char* argv[]) {
  if ((argc != 3 && argc != 5 && argc != 7) || std::string(argv[1]) != "--case") return 2;
  unsigned int batch=1U;
  std::string search="split-min";
  for (int i=3; i<argc; i+=2) {
    if (i+1 == argc) return 2;
    if (std::string(argv[i]) == "--search" &&
        (std::string(argv[i+1]) == "lsb" || std::string(argv[i+1]) == "split-min"))
      search=argv[i+1];
    else if (std::string(argv[i]) == "--batch")
      batch=static_cast<unsigned int>(std::strtoul(argv[i+1],nullptr,10));
    else return 2;
  }
  if (batch == 0U) return 2;
  const Case* selected=nullptr;
  for (const Case& c : cases) if (c.id == std::string(argv[2])) selected=&c;
  if (selected == nullptr) return 2;
  std::vector<std::vector<WordValue> > rows;
  std::uint64_t total_solutions=0;
  for (unsigned int trial=0; trial<batch; trial++) {
    Model* root=new Model(*selected,search == "lsb"); DFS<Model> engine(root); delete root;
    while (Model* solution=engine.next()) {
      total_solutions++;
      if (trial == 0U) {
        std::vector<WordValue> row={solution->x.val(),solution->y.val(),solution->z.val()};
        if (std::string(selected->operation) == "divmod") row.push_back(solution->r.val());
        rows.push_back(row);
      }
      delete solution;
    }
  }
  std::cout << "{\"schema_version\":1,\"semantic_status\":\""
            << (rows.empty() ? "unsat" : "sat") << "\",\"solutions\":" << rows.size()
            << ",\"batch\":" << batch << ",\"batch_solutions\":" << total_solutions
            << ",\"decision_variables\":[\"x\",\"y\",\""
            << (std::string(selected->operation) == "divmod" ? "quotient\",\"remainder" : "result")
            << "\"],\"projections\":[";
  for (std::size_t i=0; i<rows.size(); i++) {
    if (i) std::cout << ','; std::cout << '[';
    for (std::size_t j=0; j<rows[i].size(); j++) { if (j) std::cout << ','; std::cout << rows[i][j]; }
    std::cout << ']';
  }
  std::cout << "]}\n";
}

// STATISTICS: example-any
