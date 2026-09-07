#include <gecode/optimize/quadratic_bound.hpp>
#include <cassert>
#include <cfenv>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
using namespace Gecode::Optimize;
using namespace Gecode::Optimize::Detail;
namespace {
// Independent exact signed dyadic reference: arbitrary-width base-2^32 integer.
// No floating arithmetic is used to compare the exact sum/product with endpoints.
struct Dyadic {
  int sign=0, exponent=0;
  std::vector<std::uint32_t> words;
  explicit Dyadic(double d=0) {
    assert(std::isfinite(d));std::uint64_t bits;std::memcpy(&bits,&d,8);
    const auto e=(bits>>52)&2047, fraction=bits&((std::uint64_t{1}<<52)-1);
    const auto mantissa=e?fraction+(std::uint64_t{1}<<52):fraction;
    if(!mantissa)return;
    sign=(bits>>63)?-1:1;exponent=e?static_cast<int>(e)-1023-52:-1074;
    words={static_cast<std::uint32_t>(mantissa),static_cast<std::uint32_t>(mantissa>>32)};trim();
  }
  void trim(){while(!words.empty()&&!words.back())words.pop_back();if(words.empty())sign=0;}
  std::vector<std::uint32_t> shifted(int shift)const {
    assert(shift>=0);if(!sign)return {};
    std::vector<std::uint32_t> out(words.size()+shift/32+1,0);
    const unsigned r=shift%32;std::uint64_t carry=0;
    for(std::size_t i=0;i<words.size();++i){const std::uint64_t z=(std::uint64_t(words[i])<<r)|carry;out[i+shift/32]=static_cast<std::uint32_t>(z);carry=z>>32;}
    out[words.size()+shift/32]=static_cast<std::uint32_t>(carry);while(!out.empty()&&!out.back())out.pop_back();return out;
  }
};
int magcmp(const std::vector<std::uint32_t>& a,const std::vector<std::uint32_t>& b){
  if(a.size()!=b.size())return a.size()<b.size()?-1:1;
  for(std::size_t i=a.size();i--;)if(a[i]!=b[i])return a[i]<b[i]?-1:1;return 0;
}
int compare(const Dyadic& a,const Dyadic& b){
  if(a.sign!=b.sign)return a.sign<b.sign?-1:1;if(!a.sign)return 0;
  const int e=std::min(a.exponent,b.exponent);return a.sign*magcmp(a.shifted(a.exponent-e),b.shifted(b.exponent-e));
}
Dyadic add(const Dyadic& a,const Dyadic& b){
  if(!a.sign)return b;if(!b.sign)return a;Dyadic out;out.exponent=std::min(a.exponent,b.exponent);
  auto x=a.shifted(a.exponent-out.exponent),y=b.shifted(b.exponent-out.exponent);out.sign=a.sign;
  if(a.sign==b.sign){const auto n=std::max(x.size(),y.size());x.resize(n);y.resize(n);out.words.resize(n+1);std::uint64_t c=0;
    for(std::size_t i=0;i<n;++i){const auto z=std::uint64_t(x[i])+y[i]+c;out.words[i]=static_cast<std::uint32_t>(z);c=z>>32;}out.words[n]=static_cast<std::uint32_t>(c);
  }else{if(magcmp(x,y)<0){x.swap(y);out.sign=b.sign;}y.resize(x.size());out.words.resize(x.size());std::uint64_t borrow=0;
    for(std::size_t i=0;i<x.size();++i){const auto sub=std::uint64_t(y[i])+borrow;out.words[i]=static_cast<std::uint32_t>(std::uint64_t(x[i])-sub);borrow=std::uint64_t(x[i])<sub;}}
  out.trim();return out;
}
Dyadic multiply(const Dyadic& a,const Dyadic& b){
  Dyadic out;if(!a.sign||!b.sign)return out;out.sign=a.sign*b.sign;out.exponent=a.exponent+b.exponent;out.words.resize(a.words.size()+b.words.size());
  for(std::size_t i=0;i<a.words.size();++i){std::uint64_t carry=0;for(std::size_t j=0;j<b.words.size();++j){const auto v=std::uint64_t(a.words[i])*b.words[j]+out.words[i+j]+carry;out.words[i+j]=static_cast<std::uint32_t>(v);carry=v>>32;}out.words[i+b.words.size()]=static_cast<std::uint32_t>(carry);}out.trim();return out;
}
void enclosed(QpInterval interval,const Dyadic& exact){assert(compare(Dyadic(interval.lower),exact)<=0);assert(compare(Dyadic(interval.upper),exact)>=0);}
}
int main(){
#if defined(__FAST_MATH__) || defined(GECODE_QUADRATIC_EXPECT_UNSUPPORTED_ARITHMETIC)
  assert(!quadratic_arithmetic_supported());
  QuadraticModel m;auto x=m.add_continuous(-1,1);m.minimize_squares({{{{x,1}},0,1,""}});
  assert(!quadratic_bound(m.snapshot(),{0},{0},{}).normalized_lower);
  std::cout<<"quadratic fast-math proof boundary rejected\n";return 0;
#else
  assert(quadratic_arithmetic_supported());std::mt19937_64 rng(813);std::size_t checked=0;
  for(int i=0;i<12000;++i){
    const double a=std::ldexp(double(static_cast<std::int64_t>(rng()%2000001)-1000000),int(rng()%2050)-1074);
    const double b=std::ldexp(double(static_cast<std::int64_t>(rng()%2000001)-1000000),int(rng()%2050)-1074);
    if(!std::isfinite(a)||!std::isfinite(b))continue;
    try{enclosed(qp_add(qp_point(a),qp_point(b)),add(Dyadic(a),Dyadic(b)));++checked;}catch(const std::overflow_error&){}
    try{enclosed(qp_multiply(qp_point(a),qp_point(b)),multiply(Dyadic(a),Dyadic(b)));++checked;}catch(const std::overflow_error&){}
  }
  const double tiny=std::numeric_limits<double>::denorm_min();
  enclosed(qp_multiply(qp_point(tiny),qp_point(0.5)),multiply(Dyadic(tiny),Dyadic(0.5)));
  enclosed(qp_add(qp_point(1e16),qp_point(1)),add(Dyadic(1e16),Dyadic(1)));
  enclosed(qp_add(qp_point(-1e16),qp_point(-1)),add(Dyadic(-1e16),Dyadic(-1)));
  for(int k=0;k<400;++k){
    QuadraticModel m;const double lo=-3,hi=4;
    auto x=m.add_continuous(lo,hi);const double a=int(rng()%9)-4,b=int(rng()%9)-4,w=1+int(rng()%4),t=int(rng()%11)-5;
    const double c=int(rng()%9)-4,d=int(rng()%9)-4;
    m.minimize_squares({{{{x,a}},b,w,""}},{{x,c}},1e16);
    m.add_row({{x,2}},-4,6);const auto q=m.snapshot();
    const auto got=quadratic_bound(q,{1},{t},{d});assert(got.normalized_lower&&got.gap_upper);
    // Exact tangent/row bound formula for this one-variable instance.
    const double residual=c+2*w*t*a-2*d;
    const double constant=2*w*t*b-w*t*t+d*(d>=0?-4:6);
    auto exact=add(Dyadic(1e16),add(Dyadic(constant),multiply(Dyadic(residual),Dyadic(residual>=0?lo:hi))));
    assert(compare(Dyadic(*got.normalized_lower),exact)<=0);
    const double primal=c+w*(a+b)*(a+b);
    const double dual=constant+residual*(residual>=0?lo:hi);
    assert(compare(Dyadic(*got.gap_upper),Dyadic(primal-dual))>=0);
    // Also check the claimed global bound at every feasible integer point.
    for(int value=-2;value<=3;++value){auto objective=add(Dyadic(1e16),Dyadic(c*value+w*(a*value+b)*(a*value+b)));assert(compare(Dyadic(*got.normalized_lower),objective)<=0);}
  }
  QuadraticModel m;auto x=m.add_continuous(-1,1);m.minimize_squares({{{{x,1}},0,1,""}});const auto q=m.snapshot();
  const int old=std::fegetround();assert(std::fesetround(FE_UPWARD)==0);assert(!quadratic_arithmetic_supported());assert(!quadratic_bound(q,{0},{0},{}).normalized_lower);assert(std::fesetround(old)==0);
  assert(!quadratic_bound(q,{0},{std::numeric_limits<double>::infinity()},{}).normalized_lower);
  assert(!quadratic_bound(q,{0},{1e308},{}).normalized_lower);
  SolveOptions o;o.time_limit_seconds=0;SolveBudget budget(o);assert(!quadratic_bound(q,{0},{0},{},&budget).normalized_lower);
  std::cout<<"quadratic bound: "<<checked<<" exact dyadic operations, 400 exact bound formulas\n";
#endif
}
