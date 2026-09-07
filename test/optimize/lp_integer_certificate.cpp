// Exact finite integer box bounds and interval cuts; independent of both solvers.
#include <gecode/minimodel/lp-certificate.hpp>
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

namespace C=Gecode::Experimental::LpCertificate;
using I=std::int64_t;
using V=std::vector<I>;
static void check(bool okay,const char* message) {if(!okay)throw std::runtime_error(message);}
struct Matrix {
  std::vector<std::size_t> start{0},column;
  V value;std::size_t n=0;
  C::SparseMatrixView view() const {return {start,column,value,n};}
};
static Matrix sparse(const V& a,std::size_t m,std::size_t n) {
  Matrix matrix;matrix.n=n;
  for(std::size_t i=0;i<m;++i) {
    for(std::size_t j=0;j<n;++j) if(a[i*n+j]) {
      matrix.column.push_back(j);matrix.value.push_back(a[i*n+j]);
    }
    matrix.start.push_back(matrix.value.size());
  }
  return matrix;
}
static void edges() {
  for(I cost:{I(2),I(-2)}) for(I cutoff:{I(3),I(-3),I(-4)}) {
    C::Certificate certificate;
    check(C::prepare({}, {}, {cost}, {},certificate),"prepare no-row integer objective");
    C::IntegerFilterResult result;
    check(certificate.filter_integer({-5},{5},cutoff,result) && !result.infeasible,
          "integer signed division failed");
    const I upper=cutoff==3?1:-2;
    check(result.lower==V({cost>0?-5:-upper}) && result.upper==V({cost>0?upper:5}),
          "floor/ceil sign or exact-divisibility error");
    check(result.lower_bound==-10,"signed integer box bound");
    I legacy=443;
    check(!certificate.lower_bound({-5},{5},legacy) && legacy==443,"legacy binary method widened silently");
  }
  C::Certificate constant;
  check(C::prepare({2},{3},{1},{0.5},constant),"zero-residual multiplier preparation");
  C::IntegerFilterResult empty;
  check(constant.filter_integer({-3},{5},1,empty) && empty.infeasible && empty.lower_bound==2,
        "constant cutoff infeasibility or integer ceiling");
  check(constant.filter_integer({-3},{5},2,empty) && !empty.infeasible,
        "constant cutoff equality rejected");
  C::Certificate none;
  check(C::prepare({}, {}, {}, {},none),"empty model preparation");
  check(none.filter_integer({}, {},-1,empty) && empty.infeasible,"zero-dimensional box cutoff");
  check(none.filter_integer({}, {},0,empty) && !empty.infeasible && empty.lower.empty(),"empty feasible model");
  C::IntegerFilterResult saved{991,{992},{993},true};
  empty=saved;
  check(!constant.filter_integer({2},{1},0,empty) && empty.lower_bound==991 && empty.lower==V({992}) &&
        empty.upper==V({993}) && empty.infeasible,"empty interval failure changed output");
  I bound=444;
  check(!constant.lower_bound_integer({}, {},bound) && bound==444,"integer bound dimension failure");
  const auto maximum=std::numeric_limits<I>::max(),minimum=std::numeric_limits<I>::min();
  check(C::integer_lower_bound({}, {},{1},{minimum},{minimum},{},bound) && bound==minimum,"minimum integer endpoint");
  check(C::integer_lower_bound({}, {},{1},{maximum},{maximum},{},bound) && bound==maximum,"maximum integer endpoint");
  bound=444;
  check(!C::integer_lower_bound({}, {},{-1},{minimum},{minimum},{},bound) && bound==444,
        "unrepresentable objective accepted");
  check(C::integer_lower_bound({}, {},{minimum},{1},{1},{},bound) && bound==minimum,"minimum integer coefficient");
  check(C::integer_lower_bound({}, {},{maximum},{1},{1},{},bound) && bound==maximum,"maximum integer coefficient");

  C::Certificate large;
  check(C::prepare({maximum},{0},{0},{1e8},large),"large residual preparation");
  bound=444;
  check(!large.lower_bound_integer({0},{maximum},bound) && bound==444,"residual*endpoint overflow accepted");
  empty=saved;
  check(!large.filter_integer({0},{maximum},0,empty) && empty.lower_bound==991 && empty.lower==V({992}),
        "overflowing interval filter changed output");
  // Preparation and the integer box numerator fit; the conditional cutoff
  // subtraction does not. Reject the whole cut rather than wrap or saturate.
  const V huge={minimum,minimum,minimum,minimum+1};
  check(C::prepare(huge,huge,{0},std::vector<double>(4,std::ldexp(1.0,42)),large),
        "near-int128-limit affine preparation");
  check(large.lower_bound_integer({1},{2},bound) && bound==0,"cancelled large integer numerator");
  empty=saved;
  check(!large.filter_integer({1},{2},maximum,empty) && empty.lower_bound==991 && empty.upper==V({993}),
        "conditional subtraction overflow accepted or output changed");
}
static void oracles() {
  std::mt19937 random(139572);
  unsigned feasible_models=0,cut_solutions=0,tightened=0;
  for(unsigned trial=0;trial<3000;++trial) {
    const std::size_t n=1+random()%4,m=random()%5;
    V a(n*m),b(m),c(n),lower(n),upper(n),witness(n);
    std::vector<double> duals(m);
    for(std::size_t j=0;j<n;++j) {
      lower[j]=static_cast<int>(random()%7)-4;upper[j]=lower[j]+random()%4;
      witness[j]=lower[j]+random()%(upper[j]-lower[j]+1);
      c[j]=static_cast<int>(random()%11)-5;
    }
    for(std::size_t i=0;i<m;++i) {
      I activity=0;
      for(std::size_t j=0;j<n;++j) {
        a[i*n+j]=random()%3?0:static_cast<int>(random()%11)-5;
        activity+=a[i*n+j]*witness[j];
      }
      b[i]=trial%2?static_cast<int>(random()%21)-10:activity-static_cast<int>(random()%5);
      duals[i]=(static_cast<int>(random()%41)-10)/7.0;
    }
    const auto matrix=sparse(a,m,n);
    C::Certificate dense,csr;
    check(C::prepare(a,b,c,duals,dense) && C::prepare(matrix.view(),b,c,duals,csr),"small integer preparation");
    I db=0,sb=0,direct=0;
    check(dense.lower_bound_integer(lower,upper,db) && csr.lower_bound_integer(lower,upper,sb) && db==sb &&
          C::integer_lower_bound(matrix.view(),b,c,lower,upper,duals,direct) && direct==db,"integer dense/CSR equivalence");
    const I cutoff=static_cast<int>(random()%41)-20;
    C::IntegerFilterResult dcut,scut;
    check(dense.filter_integer(lower,upper,cutoff,dcut) && csr.filter_integer(lower,upper,cutoff,scut),"small integer filtering");
    check(dcut.lower_bound==scut.lower_bound && dcut.lower==scut.lower && dcut.upper==scut.upper &&
          dcut.infeasible==scut.infeasible,"dense/CSR integer cuts disagree");
    bool feasible=false,within_cutoff=false;
    V assignment(n);
    const auto enumerate=[&](auto&& self,std::size_t j)->void {
      if(j<n) {for(I value=lower[j];value<=upper[j];++value){assignment[j]=value;self(self,j+1);}return;}
      for(std::size_t i=0;i<m;++i) {
        I activity=0;for(std::size_t k=0;k<n;++k)activity+=a[i*n+k]*assignment[k];
        if(activity<b[i])return;
      }
      feasible=true;
      I objective=0;for(std::size_t k=0;k<n;++k)objective+=c[k]*assignment[k];
      check(db<=objective,"integer certificate exceeds a feasible objective");
      if(objective>cutoff)return;
      within_cutoff=true;++cut_solutions;
      check(!scut.infeasible,"integer cut falsely proved infeasibility");
      for(std::size_t k=0;k<n;++k)
        check(scut.lower[k]<=assignment[k] && assignment[k]<=scut.upper[k],"integer interval removed feasible cutoff solution");
    };
    enumerate(enumerate,0);
    if(feasible)++feasible_models;
    if(scut.infeasible)check(!within_cutoff,"integer empty-cut oracle");
    else for(std::size_t j=0;j<n;++j) {
      check(scut.lower[j]>=lower[j] && scut.upper[j]<=upper[j] && scut.lower[j]<=scut.upper[j],"cuts loosened/inverted box");
      tightened+=scut.lower[j]!=lower[j] || scut.upper[j]!=upper[j];
    }
    // The same certificate also evaluates a looser, signed sibling box.
    for(std::size_t j=0;j<n;++j){--lower[j];++upper[j];}
    check(dense.lower_bound_integer(lower,upper,db) && csr.lower_bound_integer(lower,upper,sb) && db==sb,
          "integer certificate sibling reuse");
  }
  check(feasible_models>=1500 && cut_solutions>0 && tightened>0,"integer oracle did not exercise valid cuts");
  std::cout<<"PASS 3000 integer box/cut dense-CSR oracles; "<<tightened<<" interval cuts, "<<cut_solutions<<" cutoff witnesses\n";
}
int main() {
 try {
  if(!C::supported) {
    I bound=123;check(!C::integer_lower_bound({}, {}, {}, {}, {}, {},bound) && bound==123,"unsupported fallback");
    std::cout<<"PASS integer unsupported-arithmetic fallback\n";return 0;
  }
  edges();oracles();
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
