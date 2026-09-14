/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/word.hh>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <cstdlib>

using namespace Gecode;

enum ActorKind { AK_DIVIDES, AK_GCD, AK_PRODUCT_MOD };

static WordValue mask_for(unsigned int width) {
  return (width == 64U) ? ~WordValue(0) : (WordValue(1)<<width)-1U;
}

class ProgressionClosure : public Space {
public:
  WordVar divisor, dividend, gcd_result, one, mod_result;
  IntVar modulus;
  BoolVar divisible;
  ProgressionClosure(unsigned int width, unsigned int fixed,
                     ActorKind kind)
    : divisor(*this,width,WDT_UNSIGNED,
              (WordValue(1)<<fixed)-1U,(WordValue(1)<<fixed)-1U),
      dividend(*this,width,0U,mask_for(width)&~((WordValue(1)<<fixed)-1U),
               WDT_UNSIGNED,WordValue(1)<<fixed,
               ((WordValue(1)<<fixed)-1U)*(WordValue(1)<<fixed)),
      gcd_result(*this,width,WDT_UNSIGNED,
                 (WordValue(1)<<fixed)-1U,(WordValue(1)<<fixed)-1U),
      one(*this,width,WDT_UNSIGNED,1U,1U),
      mod_result(*this,width,WDT_UNSIGNED,0U,0U),
      modulus(*this,static_cast<int>((WordValue(1)<<fixed)-1U),
              static_cast<int>((WordValue(1)<<fixed)-1U)),
      divisible(*this,1,1) {
    switch (kind) {
    case AK_DIVIDES:
      divides(*this,divisor,dividend,Reify(divisible)); break;
    case AK_GCD:
      gcd(*this,divisor,dividend,gcd_result); break;
    case AK_PRODUCT_MOD:
      product_mod(*this,one,dividend,modulus,mod_result); break;
    default: GECODE_NEVER;
    }
  }
  ProgressionClosure(ProgressionClosure& s) : Space(s) {
    divisor.update(*this,s.divisor); dividend.update(*this,s.dividend);
    gcd_result.update(*this,s.gcd_result); one.update(*this,s.one);
    mod_result.update(*this,s.mod_result); modulus.update(*this,s.modulus);
    divisible.update(*this,s.divisible);
  }
  virtual Space* copy(void) { return new ProgressionClosure(*this); }
};

int main(int argc, char* argv[]) {
  if (argc != 3) return 2;
  const unsigned int width=static_cast<unsigned int>(std::strtoul(argv[1],nullptr,10));
  if ((width < 2U) || (width > 64U)) return 2;
  ActorKind kind;
  if (std::strcmp(argv[2],"divides") == 0)
    kind=AK_DIVIDES;
  else if (std::strcmp(argv[2],"gcd") == 0)
    kind=AK_GCD;
  else if (std::strcmp(argv[2],"product_mod") == 0)
    kind=AK_PRODUCT_MOD;
  else
    return 2;
  const unsigned int fixed=std::min(width/2U,30U);
  ProgressionClosure space(width,fixed,kind);
  const bool ok=(space.status() != SS_FAILED) && space.dividend.assigned();
  std::cout << "{\"width\":" << width
            << ",\"actor\":\"" << argv[2] << "\""
            << ",\"fixed_bits\":" << fixed
            << ",\"minimum\":" << space.dividend.minimum()
            << ",\"maximum\":" << space.dividend.maximum()
            << ",\"assigned\":" << (space.dividend.assigned() ? "true" : "false")
            << ",\"propagators\":" << PropagatorGroup::all.size(space)
            << "}\n";
  return ok ? 0 : 1;
}
