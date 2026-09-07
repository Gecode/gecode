/* Independent exact vertex oracle for small boxed two-variable equality LPs.
 * It compares objectives at enumerated vertices, never uses a simplex tableau.
 * Fixtures keep operands small enough for checked int64 arithmetic. */
#ifndef TEST_OPTIMIZE_LP_SENSITIVITY_ORACLE_HPP
#define TEST_OPTIMIZE_LP_SENSITIVITY_ORACLE_HPP
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <optional>
#include <vector>
namespace SensitivityOracle {
struct Q {
  std::int64_t n=0,d=1;
  Q(std::int64_t a=0,std::int64_t b=1):n(a),d(b){assert(b);if(d<0){n=-n;d=-d;}auto g=std::gcd(n,d);n/=g;d/=g;}
  double value() const{return static_cast<double>(n)/d;}
};
inline std::int64_t product(std::int64_t a,std::int64_t b){assert(std::abs(a)<=100000000&&std::abs(b)<=100000000);return a*b;}
inline Q operator+(Q a,Q b){return {product(a.n,b.d)+product(b.n,a.d),product(a.d,b.d)};}
inline Q operator-(Q a,Q b){return {product(a.n,b.d)-product(b.n,a.d),product(a.d,b.d)};}
inline Q operator*(Q a,Q b){return {product(a.n,b.n),product(a.d,b.d)};}
inline Q operator/(Q a,Q b){assert(b.n);return {product(a.n,b.d),product(a.d,b.n)};}
inline bool operator<(Q a,Q b){return product(a.n,b.d)<product(b.n,a.d);}
inline bool operator==(Q a,Q b){return a.n==b.n&&a.d==b.d;}
struct Range {std::optional<Q> lower,upper;};
struct BoxLine {
  Q a,b,rhs,lx,ux,ly,uy,cx,cy;
  std::array<Q,2> reference;
  bool maximize=false;
  std::vector<std::array<Q,2>> vertices() const {
    std::vector<std::array<Q,2>> out;
    auto add=[&](Q x,Q y){if(x<lx||ux<x||y<ly||uy<y)return;for(auto v:out)if(v[0]==x&&v[1]==y)return;out.push_back({x,y});};
    for(Q x:{lx,ux})if(b.n)add(x,(rhs-a*x)/b);
    for(Q y:{ly,uy})if(a.n)add((rhs-b*y)/a,y);
    assert(!out.empty());return out;
  }
  Range objective(std::size_t j) const {
    Range out;const Q sign=maximize?Q(-1):Q(1);
    for(auto v:vertices()){
      const Q change=sign*(v[j]-reference[j]);
      const Q other=sign*((j?cx:cy)*(v[1-j]-reference[1-j]));
      if(!change.n){assert(!(other<Q(0)));continue;}
      const Q threshold=(Q(0)-other)/change;
      if(Q(0)<change){if(!out.lower||*out.lower<threshold)out.lower=threshold;}
      else if(!out.upper||threshold<*out.upper)out.upper=threshold;
    }return out;
  }
  bool optimal_at(Q coefficient,std::size_t j) const {
    const Q sign=maximize?Q(-1):Q(1);
    for(auto v:vertices()) {
      const Q difference=coefficient*(v[j]-reference[j])+
        (j?cx:cy)*(v[1-j]-reference[1-j]);
      if(sign*difference<Q(0))return false;
    }return true;
  }
  Range equality_with_x_fixed() const {
    Q first=a*reference[0]+b*ly,last=a*reference[0]+b*uy;
    return first<last?Range{first,last}:Range{last,first};
  }
};
}
#endif
