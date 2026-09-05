/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace Gecode;

namespace {

  struct Parameters {
    std::string id;
    unsigned int width;
    WordDomainType domain;
    WordValue x_min, x_max, y_min, y_max, result_min, result_max;
    int modulus;
  };

  unsigned long long
  parse_unsigned(const char* option, const char* value) {
    errno=0;
    char* end=nullptr;
    const unsigned long long result=std::strtoull(value,&end,10);
    if ((errno != 0) || (end == value) || (*end != '\0')) {
      std::cerr << "invalid value for " << option << "\n";
      std::exit(2);
    }
    return result;
  }

  long long
  parse_signed(const char* option, const char* value) {
    errno=0;
    char* end=nullptr;
    const long long result=std::strtoll(value,&end,10);
    if ((errno != 0) || (end == value) || (*end != '\0')) {
      std::cerr << "invalid value for " << option << "\n";
      std::exit(2);
    }
    return result;
  }

  const char*
  require_value(int& i, int argc, char* argv[]) {
    if (++i == argc) {
      std::cerr << "missing option value\n";
      std::exit(2);
    }
    return argv[i];
  }

  WordValue
  endpoint(const char* option, const std::string& value, unsigned int width,
           WordDomainType domain) {
    const WordValue mask=Word::width_mask(width);
    if (domain == WDT_UNSIGNED) {
      const unsigned long long parsed=parse_unsigned(option,value.c_str());
      if (parsed > mask) {
        std::cerr << option << " does not fit the Word width\n";
        std::exit(2);
      }
      return static_cast<WordValue>(parsed);
    }
    const long long parsed=parse_signed(option,value.c_str());
    const long long minimum=(width == 64) ? LLONG_MIN : -(1LL << (width-1));
    const long long maximum=(width == 64) ? LLONG_MAX : (1LL << (width-1))-1;
    if ((parsed < minimum) || (parsed > maximum)) {
      std::cerr << option << " does not fit the signed Word width\n";
      std::exit(2);
    }
    return static_cast<WordValue>(parsed) & mask;
  }

  class ProductModCase : public Space {
  public:
    WordVar x, y, result;
    IntVar modulus;
    ProductModCase(const Parameters& p, bool bounded)
      : x(*this,p.width,p.domain,p.x_min,p.x_max),
        y(*this,p.width,p.domain,p.y_min,p.y_max),
        result(*this,p.width,WDT_UNSIGNED,p.result_min,p.result_max),
        modulus(*this,p.modulus,p.modulus) {
      if (bounded) {
        product_mod(*this,x,y,modulus,result);
      } else {
        WordVar cx(*this,p.width), cy(*this,p.width), cr(*this,p.width);
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

}

int
main(int argc, char* argv[]) {
  bool bounded=true;
  std::string id="reduce-small", domain="unsigned";
  unsigned int width=9;
  std::string x_min="10", x_max="30", y_min="10", y_max="30";
  unsigned long long result_min=0, result_max=16, modulus=17;
  for (int i=1; i<argc; i++) {
    const char* option=argv[i];
    const char* value=require_value(i,argc,argv);
    if (std::strcmp(option,"--variant") == 0) {
      if (std::strcmp(value,"bounded") == 0) bounded=true;
      else if (std::strcmp(value,"compact") == 0) bounded=false;
      else return 2;
    } else if (std::strcmp(option,"--case-id") == 0) id=value;
    else if (std::strcmp(option,"--width") == 0)
      width=static_cast<unsigned int>(parse_unsigned(option,value));
    else if (std::strcmp(option,"--domain") == 0) domain=value;
    else if (std::strcmp(option,"--x-min") == 0) x_min=value;
    else if (std::strcmp(option,"--x-max") == 0) x_max=value;
    else if (std::strcmp(option,"--y-min") == 0) y_min=value;
    else if (std::strcmp(option,"--y-max") == 0) y_max=value;
    else if (std::strcmp(option,"--result-min") == 0)
      result_min=parse_unsigned(option,value);
    else if (std::strcmp(option,"--result-max") == 0)
      result_max=parse_unsigned(option,value);
    else if (std::strcmp(option,"--modulus") == 0)
      modulus=parse_unsigned(option,value);
    else {
      std::cerr << "unknown option: " << option << "\n";
      return 2;
    }
  }
  if ((width == 0) || (width > 64) ||
      ((domain != "unsigned") && (domain != "signed")) ||
      (modulus == 0) ||
      (modulus > static_cast<unsigned long long>(Int::Limits::max)) ||
      (result_min > result_max) || (result_max > Word::width_mask(width))) {
    std::cerr << "invalid benchmark parameters\n";
    return 2;
  }
  const WordDomainType domain_type = domain == "signed" ? WDT_SIGNED : WDT_UNSIGNED;
  Parameters parameters={id,width,domain_type,
    endpoint("--x-min",x_min,width,domain_type),
    endpoint("--x-max",x_max,width,domain_type),
    endpoint("--y-min",y_min,width,domain_type),
    endpoint("--y-max",y_max,width,domain_type),
    static_cast<WordValue>(result_min),static_cast<WordValue>(result_max),
    static_cast<int>(modulus)};

  ProductModCase* root=new ProductModCase(parameters,bounded);
  DFS<ProductModCase> search(root); delete root;
  unsigned long long solutions=0, checksum=0;
  std::vector<std::tuple<WordValue,WordValue,WordValue> > projections;
  while (ProductModCase* solution=search.next()) {
    const WordValue xv=solution->x.val(), yv=solution->y.val();
    const WordValue rv=solution->result.val();
    solutions++;
    projections.emplace_back(xv,yv,rv);
    checksum += (xv*1315423911ULL) ^ (yv*2654435761ULL) ^ rv;
    delete solution;
  }
  const Search::Statistics stats=search.statistics();
  std::cout << "{\"schema_version\":1,\"status\":\"ok\""
            << ",\"case_id\":\"" << id << "\""
            << ",\"variant\":\"" << (bounded ? "bounded" : "compact") << "\""
            << ",\"solutions\":" << solutions
            << ",\"checksum\":" << checksum
            << ",\"semantic_status\":\"" << (solutions ? "sat" : "unsat") << "\""
            << ",\"decision_variables\":[\"x\",\"y\",\"result\"]"
            << ",\"projections\":[";
  for (std::size_t i=0; i<projections.size(); i++) {
    if (i != 0) std::cout << ',';
    std::cout << '[' << std::get<0>(projections[i]) << ','
              << std::get<1>(projections[i]) << ','
              << std::get<2>(projections[i]) << ']';
  }
  std::cout << "]"
            << ",\"nodes\":" << stats.node
            << ",\"failures\":" << stats.fail
            << ",\"propagations\":" << stats.propagate << "}\n";
  return 0;
}
