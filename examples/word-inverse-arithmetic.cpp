/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>

#include "../benchmarks/word/driver-output.hpp"

#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

using namespace Gecode;

namespace {
  struct InvalidNumber {};

  template<class Unsigned>
  Unsigned parse_unsigned(const char* text, int base) {
    errno = 0;
    char* end = nullptr;
    const unsigned long long value = std::strtoull(text,&end,base);
    const bool has_prefix = (text[0] >= '0' && text[0] <= '9') ||
      (text[0] == '+');
    if (!has_prefix || errno != 0 || end == text || *end != '\0' ||
        value > std::numeric_limits<Unsigned>::max())
      throw InvalidNumber();
    return static_cast<Unsigned>(value);
  }

  enum Operation { MULT, DIVMOD, PRODUCT_MOD };
  enum Overflow { NONE, MULT_SIGNED, MULT_UNSIGNED, DIV_SIGNED };

  struct Case {
    const char* id; Operation operation; unsigned int width;
    WordValue xmin, xmax, ymin, ymax, zmin, zmax, rmin, rmax;
    int modulus; bool is_signed; bool guard; Overflow overflow;
  };

  const Case cases[] = {
    {"mult-free-w4",MULT,4,0,15,0,15,6,6,0,0,0,false,true,NONE},
    {"mult-tight-w8",MULT,8,0,31,13,13,143,143,0,0,0,false,true,NONE},
    {"mult-signed-min-neg1-w8",MULT,8,128,128,255,255,128,128,0,0,0,true,true,MULT_SIGNED},
    {"mult-wrap-w64",MULT,64,~WordValue(0),~WordValue(0),~WordValue(0),~WordValue(0),1,1,0,0,0,false,true,MULT_UNSIGNED},
    {"divmod-free-w3",DIVMOD,3,0,7,0,7,0,7,0,7,0,false,true,NONE},
    {"divmod-tight-w8",DIVMOD,8,100,140,7,9,15,15,1,1,0,false,true,NONE},
    {"divmod-zero-w64",DIVMOD,64,~WordValue(0)-1,~WordValue(0)-1,0,0,~WordValue(0),~WordValue(0),~WordValue(0)-1,~WordValue(0)-1,0,false,true,NONE},
    {"divmod-signed-min-neg1-w8",DIVMOD,8,128,128,255,255,128,128,0,0,0,true,true,DIV_SIGNED},
    {"divmod-signed-zero-w8",DIVMOD,8,128,128,0,0,1,1,128,128,0,true,true,NONE},
    {"product-mod-free-w5",PRODUCT_MOD,5,0,31,0,7,0,0,0,0,5,false,true,NONE},
    {"product-mod-tight-w5",PRODUCT_MOD,5,0,31,7,7,4,4,0,0,13,false,true,NONE},
    {"product-mod-wide-w64",PRODUCT_MOD,64,~WordValue(0),~WordValue(0),~WordValue(0),~WordValue(0),225,225,0,0,2147483646,false,true,NONE},
    {"product-mod-wrong-w8",PRODUCT_MOD,8,7,7,7,7,5,5,0,0,13,false,true,NONE},
    {"product-mod-disabled",PRODUCT_MOD,8,2,2,3,3,4,4,0,0,13,false,false,NONE},
    {"product-mod-disabled-zero",PRODUCT_MOD,8,2,2,3,3,4,4,0,0,0,false,false,NONE}
  };

  /// Enumerate arithmetic operands with explicit signed and overflow semantics.
  class Model : public Space {
  public:
    WordVar x, y, z, r;
    Model(const Case& c, bool lsb)
      : x(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.xmin,c.xmax),
        y(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.ymin,c.ymax),
        z(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.zmin,c.zmax),
        r(*this,c.width,c.is_signed ? WDT_SIGNED : WDT_UNSIGNED,c.rmin,c.rmax) {
      post_operation(c);
      WordVarArgs variables = c.operation == DIVMOD ?
        WordVarArgs({x,y,z,r}) : WordVarArgs({x,y,z});
      if (lsb)
        branch(*this,variables,WORD_VAR_NONE(),WORD_VAL_LSB());
      else
        branch(*this,variables,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    }
    Model(Model& s) : Space(s) {
      x.update(*this,s.x); y.update(*this,s.y); z.update(*this,s.z); r.update(*this,s.r);
    }
    Space* copy(void) override { return new Model(*this); }
  private:
    void post_operation(const Case& c) {
      if (c.operation == MULT) mult(*this,x,y,z);
      else if (c.operation == DIVMOD) {
        if (c.is_signed) { signed_div(*this,x,y,z); signed_rem(*this,x,y,r); }
        else divmod(*this,x,y,z,r);
      } else {
        IntVar modulus(*this,c.modulus,c.modulus);
        BoolVar guard(*this,c.guard ? 1 : 0,c.guard ? 1 : 0);
        product_mod(*this,x,y,modulus,z,Reify(guard,RM_IMP));
      }
      if (c.overflow != NONE) {
        BoolVar flag(*this,1,1);
        const WordOverflowType type = c.overflow == MULT_SIGNED ? WOF_MULT_SIGNED :
          (c.overflow == MULT_UNSIGNED ? WOF_MULT_UNSIGNED : WOF_DIV_SIGNED);
        overflow(*this,x,type,y,flag);
      }
    }

  };
}

namespace {
  struct Request {
    const Case* spec=nullptr;
    unsigned int batch=1;
    bool lsb=false;
  };
  struct Result {
    std::vector<std::vector<WordValue> > rows;
    std::uint64_t total_solutions=0;
  };
  bool parse_request(int argc, char* argv[], Request& request) {
    if ((argc != 3 && argc != 5 && argc != 7) || std::string(argv[1]) != "--case") return false;

    for (int i=3; i<argc; i+=2) {
      if (i+1 == argc) return false;
      if (std::string(argv[i]) == "--search" &&
          (std::string(argv[i+1]) == "lsb" || std::string(argv[i+1]) == "split-min"))
        request.lsb=std::string(argv[i+1]) == "lsb";
      else if (std::string(argv[i]) == "--batch")
        request.batch=parse_unsigned<unsigned int>(argv[i+1],10);
      else return false;
    }
    if (request.batch == 0U) return false;

    for (const Case& c : cases) if (c.id == std::string(argv[2])) request.spec=&c;
    if (request.spec == nullptr) return false;
    return true;
  }

  Result enumerate(const Request& request) {
    Result result;
    for (unsigned int trial=0; trial<request.batch; trial++) {
      std::unique_ptr<Model> root(new Model(*request.spec,request.lsb));
      DFS<Model> engine(root.get());
      root.reset();
      while (std::unique_ptr<Model> solution{engine.next()}) {
        result.total_solutions++;
        if (trial == 0U) {
          std::vector<WordValue> row={solution->x.val(),solution->y.val(),solution->z.val()};
          if (request.spec->operation == DIVMOD) row.push_back(solution->r.val());
          result.rows.push_back(row);
        }
      }
    }
    return result;
  }

  void write_result(const Request& request, const Result& result) {
    const Case* selected=request.spec;
    const unsigned int batch=request.batch;
    const auto& rows=result.rows;
    std::cout << "{\"schema_version\":1,\"semantic_status\":\""
              << (rows.empty() ? "unsat" : "sat") << "\",\"solutions\":" << rows.size()
              << ",\"batch\":" << batch << ",\"batch_solutions\":" << result.total_solutions
              << ",\"decision_variables\":[\"x\",\"y\",\""
              << (selected->operation == DIVMOD ? "quotient\",\"remainder" : "result")
              << "\"],\"projections\":";
    WordDriver::write_rows(std::cout,rows);
    std::cout << "}\n";
  }
}

int main(int argc, char* argv[]) {
  Request request;
  try {
    if (!parse_request(argc,argv,request)) return 2;
  } catch (const InvalidNumber&) {
    return 2;
  }
  write_result(request,enumerate(request));
  return 0;
}

// STATISTICS: example-any
