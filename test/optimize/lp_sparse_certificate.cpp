// Sparse certificate trust boundary, dense equivalence and deterministic work.
// Standalone C++17; no native solver or numerical backend dependency.
#include <gecode/minimodel/lp-certificate.hpp>
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace C = Gecode::Experimental::LpCertificate;
using I = std::int64_t;
using V = std::vector<I>;
static void check(bool condition,const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct Matrix {
  std::vector<std::size_t> start{0},column;
  V value;
  std::size_t n=0;
  C::SparseMatrixView view() const { return {start,column,value,n}; }
};
static Matrix sparse(const V& a,std::size_t m,std::size_t n) {
  Matrix result; result.n=n;
  for (std::size_t i=0;i<m;++i) {
    for (std::size_t j=0;j<n;++j)
      if (a[i*n+j]) { result.column.push_back(j); result.value.push_back(a[i*n+j]); }
    result.start.push_back(result.value.size());
  }
  return result;
}
static void corruption() {
  Matrix unit; unit.n=2; unit.start={0,2}; unit.column={0,1}; unit.value={1,2};
  C::Certificate kept;
  check(C::prepare(unit.view(),{1},{1,2},{1},kept),"initial certificate");
  const std::vector<Matrix> bad={
    {{},{0,1},{1,2},2}, // Missing row offsets.
    {{1,2},{0,1},{1,2},2},
    {{0,1},{0,1},{1,2},2}, // Wrong terminal offset.
    {{0,3},{0,1},{1,2},2},
    {{0,2},{0},{1,2},2},
    {{0,2},{0,2},{1,2},2}, // Out of range.
    {{0,2},{0,0},{1,2},2}, // Duplicate column.
    {{0,2},{1,0},{1,2},2}, // Unsorted column.
    {{0,2},{0,1},{1,0},2}, // Explicit zero.
    {{0,2},{0,1},{1,2},3}, // Mismatched width.
  };
  for (const auto& matrix:bad) {
    C::PreparationStats work{73,74,75};
    check(!C::prepare(matrix.view(),{1},{1,2},{1},kept,&work),"malformed CSR accepted");
    check(work.rows_visited==73 && work.nonzero_products==74 && work.residuals_initialized==75,
          "failed preparation changed work output");
    I bound=0;
    check(kept.lower_bound({0,0},{1,1},bound) && bound==1,"failed preparation replaced certificate");
    bound=991;
    check(!C::lower_bound(matrix.view(),{1},{1,2},{0,0},{1,1},{1},bound) && bound==991,
          "failed sparse bound changed output");
  }
  Matrix decreasing{{0,2,1,2},{0,1},{1,1},2};
  check(!C::valid_sparse(decreasing.view(),3,2),"decreasing row offsets accepted");
  check(!C::valid_sparse(unit.view(),std::numeric_limits<std::size_t>::max(),2),
        "row count overflow accepted");
  for (double dual:{std::numeric_limits<double>::infinity(),
                    std::numeric_limits<double>::quiet_NaN(),std::ldexp(1.0,43)}) {
    check(!C::prepare(unit.view(),{1},{1,2},{dual},kept),"corrupt dual accepted");
  }
  Matrix overflow{{0,1,2,3,4,5},{0,0,0,0,0},V(5,std::numeric_limits<I>::max()),1};
  check(!C::prepare(overflow.view(),V(5,0),{0},std::vector<double>(5,std::ldexp(1.0,42)),kept),
        "sparse residual overflow accepted");
  check(!C::prepare(overflow.view(),V(5,std::numeric_limits<I>::max()),{0},
                    std::vector<double>(5,std::ldexp(1.0,42)),kept),"sparse constant overflow accepted");
}
static void equivalence() {
  std::mt19937 random(357187);
  for (unsigned trial=0;trial<3000;++trial) {
    const std::size_t n=random()%8,m=random()%9;
    V a(m*n),b(m),c(n),lower(n),upper(n);
    std::vector<double> dual(m);
    for (auto& coefficient:a) coefficient=random()%4 ? 0 : static_cast<int>(random()%15)-7;
    for (auto& rhs:b) rhs=static_cast<int>(random()%15)-7;
    for (auto& cost:c) cost=static_cast<int>(random()%21)-10;
    for (auto& q:dual) q=(static_cast<int>(random()%83)-20)/11.0;
    for (std::size_t j=0;j<n;++j) {
      const auto domain=random()%4;
      lower[j]=domain==1; upper[j]=domain!=2;
    }
    const Matrix matrix=sparse(a,m,n);
    C::Certificate dense,csr;
    C::PreparationStats work;
    check(C::prepare(a,b,c,dual,dense) && C::prepare(matrix.view(),b,c,dual,csr,&work),
          "valid dense/sparse preparation failed");
    check(work.rows_visited==m && work.residuals_initialized==n && work.nonzero_products<=matrix.value.size(),
          "sparse work exceeds rows+columns+nnz");
    const I upper_objective=static_cast<int>(random()%31)-15;
    I db=0,sb=0;
    std::vector<unsigned char> df,sf;
    check(dense.filter(lower,upper,upper_objective,db,df) &&
          csr.filter(lower,upper,upper_objective,sb,sf) && db==sb && df==sf,
          "dense/sparse bound or filtering disagreement");
    I direct=992;
    check(C::lower_bound(matrix.view(),b,c,lower,upper,dual,direct) && direct==db,
          "sparse direct/prepared bound disagreement");
    // Independent enumeration: every feasible assignment obeys the bound,
    // and every feasible assignment within the cutoff survives filtering.
    for (unsigned mask=0;mask<(1U<<n);++mask) {
      bool feasible=true; I cost=0;
      for (std::size_t j=0;j<n;++j) {
        const I x=(mask>>j)&1U;
        if (x<lower[j] || x>upper[j]) feasible=false;
        cost+=c[j]*x;
      }
      for (std::size_t i=0;i<m;++i) {
        I activity=0;
        for (std::size_t j=0;j<n;++j) activity+=a[i*n+j]*((mask>>j)&1U);
        if (activity<b[i]) feasible=false;
      }
      if (!feasible) continue;
      check(db<=cost,"sparse certificate exceeds a feasible objective");
      if (cost<=upper_objective)
        for (std::size_t j=0;j<n;++j)
          check(!(sf[j] & (1U<<((mask>>j)&1U))),"sparse filtering removed feasible assignment");
    }
    // The same immutable affine certificate may be applied to a looser sibling.
    lower.assign(n,0); upper.assign(n,1);
    check(dense.lower_bound(lower,upper,db) && csr.lower_bound(lower,upper,sb) && db==sb,
          "sparse sibling box disagreement");
  }
}
static void scaling() {
  std::size_t previous_bytes=0;
  for (std::size_t n:{std::size_t(32768),std::size_t(65536)}) {
    Matrix matrix; matrix.n=n;
    matrix.start.reserve(n+1); matrix.column.reserve(n); matrix.value.reserve(n);
    for (std::size_t i=0;i<n;++i) {
      matrix.column.push_back(i); matrix.value.push_back(1); matrix.start.push_back(i+1);
    }
    // A corresponding dense int64 matrix would need 8/32 GiB respectively.
    const auto bytes=matrix.start.capacity()*sizeof(std::size_t)+
      matrix.column.capacity()*sizeof(std::size_t)+matrix.value.capacity()*sizeof(I);
    check(bytes<=32*n+16,"CSR storage is not proportional to nnz+rows");
    if (previous_bytes) check(bytes<=2*previous_bytes,"doubling sparse dimensions grew storage superlinearly");
    previous_bytes=bytes;
    C::Certificate certificate; C::PreparationStats work;
    check(C::prepare(matrix.view(),V(n,1),V(n,1),std::vector<double>(n,1),certificate,&work),
          "large sparse certificate failed");
    check(work.rows_visited==n && work.nonzero_products==n && work.residuals_initialized==n,
          "large sparse preparation performed more than linear arithmetic");
    I bound=0;
    check(certificate.lower_bound(V(n,0),V(n,1),bound) && bound==static_cast<I>(n),
          "large sparse certificate bound");
  }
}
int main() {
  try {
    if (!C::supported) {
      Matrix matrix; I bound=123;
      check(!C::lower_bound(matrix.view(),{},{},{},{},{},bound) && bound==123,
            "unsupported arithmetic produced a bound");
      std::cout << "PASS sparse unsupported-arithmetic fallback\n"; return 0;
    }
    corruption(); equivalence(); scaling();
    std::cout << "PASS sparse CSR corruption/overflow, 3000 dense/sparse oracle/filter cases, "
                 "32768/65536 diagonal storage and operation scaling\n";
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
