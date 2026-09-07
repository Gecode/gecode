// Independent correctness checks; no Gecode or LP library is needed to link.
#include <gecode/minimodel/lp-certificate.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace Cert = Gecode::Experimental::LpCertificate;
using I = std::int64_t;
using V = std::vector<I>;

static void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

static void expect_bound(const V& A, const V& b, const V& c,
                         const V& lower, const V& upper,
                         const std::vector<double>& duals, I expected) {
  I result=987654321;
  require(Cert::lower_bound(A,b,c,lower,upper,duals,result),
          "expected a valid certificate");
  require(result==expected,"incorrect certified lower bound");
}

static void expect_rejected(const V& A, const V& b, const V& c,
                            const V& lower, const V& upper,
                            const std::vector<double>& duals) {
  I result=987654321;
  require(!Cert::lower_bound(A,b,c,lower,upper,duals,result),
          "invalid or overflowing certificate was accepted");
  require(result==987654321,"failure changed the output");
}

// Exhaustively compute the true minimum without using the certificate formula.
static bool brute_min(const V& A, const V& b, const V& c,
                      const V& lower, const V& upper, I& optimum) {
  bool found=false;
  const std::size_t n=c.size();
  for (unsigned int mask=0; mask<(1U<<n); ++mask) {
    bool feasible=true;
    for (std::size_t j=0; j<n; ++j) {
      const I x=(mask>>j)&1U;
      if (x<lower[j] || x>upper[j])
        feasible=false;
    }
    for (std::size_t i=0; feasible && i<b.size(); ++i) {
      I sum=0;
      for (std::size_t j=0; j<n; ++j)
        sum+=A[i*n+j]*((mask>>j)&1U);
      if (sum<b[i])
        feasible=false;
    }
    if (feasible) {
      I objective=0;
      for (std::size_t j=0; j<n; ++j)
        objective+=c[j]*((mask>>j)&1U);
      if (!found || objective<optimum)
        optimum=objective;
      found=true;
    }
  }
  return found;
}

int main(void) {
  try {
    if (!Cert::supported) {
      expect_rejected({}, {}, {}, {}, {}, {});
      std::cout << "PASS unsupported-arithmetic fallback preserves output\n";
      return EXIT_SUCCESS;
    }

    // Fractional lower bounds must be rounded upward for integer objectives.
    expect_bound({2},{1},{1},{0},{1},{0.5},1);
    expect_bound({-2},{-1},{-1},{0},{1},{0.5},0);
    // Signed costs, fixed variables, empty models, and no dual information.
    expect_bound({}, {}, {5,-2}, {1,0}, {1,1}, {}, 3);
    expect_bound({}, {}, {-5,2}, {0,0}, {1,1}, {}, -5);
    expect_bound({}, {}, {}, {}, {}, {}, 0);
    expect_bound({2},{1},{1},{0},{1},{-100.0},0);
    expect_bound({2},{1},{1},{0},{1},{-1e308},0);
    // An arbitrary enormous nonnegative multiplier need not be LP optimal.
    expect_bound({1},{0},{1},{0},{1},{1000.0},-999);
    // Near quantization boundary and tiny positive multipliers are safe.
    expect_bound({0},{0},{0},{0},{1},
                 {std::nextafter(std::ldexp(1.0,43),0.0)},0);
    expect_bound({1},{1},{1},{0},{1},
                 {std::numeric_limits<double>::denorm_min()},0);

    std::vector<I> quantized={123};
    require(Cert::quantize({0.5,-2.0,0.0},quantized),
            "finite multiplier conversion failed");
    require(quantized==V({Cert::scale/2,0,0}),"wrong multiplier conversion");
    const V saved=quantized;
    require(!Cert::quantize({std::numeric_limits<double>::quiet_NaN()},quantized),
            "NaN multiplier accepted");
    require(quantized==saved,"failed quantization changed the output");

    for (double bad : {std::numeric_limits<double>::quiet_NaN(),
                       std::numeric_limits<double>::infinity(),
                       -std::numeric_limits<double>::infinity(),
                       std::ldexp(1.0,43),
                       std::numeric_limits<double>::max()})
      expect_rejected({1},{1},{1},{0},{1},{bad});
    expect_rejected({}, {1}, {1}, {0}, {1}, {0.5}); // Matrix dimensions
    expect_rejected({1}, {1}, {1}, {}, {1}, {0.5}); // Bounds dimensions
    expect_rejected({1}, {1}, {1}, {0}, {1}, {});   // Dual dimensions
    expect_rejected({1}, {1}, {1}, {-1}, {1}, {0});
    expect_rejected({1}, {1}, {1}, {1}, {0}, {0});
    expect_rejected({1}, {1}, {1}, {0}, {2}, {0});
    const I largest=std::numeric_limits<I>::max();
    const I smallest=std::numeric_limits<I>::min();
    // Exact result overflow, reduced-cost accumulation overflow, y*b overflow.
    expect_rejected({}, {}, {largest,largest}, {1,1}, {1,1}, {});
    expect_rejected({}, {}, {smallest,smallest}, {1,1}, {1,1}, {});
    expect_rejected(V(5,largest),V(5,0),{0},{0},{1},
                    std::vector<double>(5,std::ldexp(1.0,42)));
    expect_rejected(V(5,0),V(5,largest),{0},{0},{1},
                    std::vector<double>(5,std::ldexp(1.0,42)));
    expect_bound({}, {}, {largest}, {1}, {1}, {}, largest);
    expect_bound({}, {}, {smallest}, {1}, {1}, {}, smallest);

    std::mt19937 random(712367);
    unsigned int feasible_cases=0;
    unsigned int infeasible_cases=0;
    const unsigned int trials=3000;
    for (unsigned int trial=0; trial<trials; ++trial) {
      const std::size_t n=1+random()%7;
      const std::size_t m=random()%9;
      V A(n*m), b(m), c(n), lower(n), upper(n), witness(n);
      std::vector<double> duals(m);
      for (std::size_t j=0; j<n; ++j) {
        const unsigned int domain=random()%3;
        lower[j]=(domain==2) ? 1 : 0;
        upper[j]=(domain==1) ? 0 : 1;
        witness[j]=(lower[j]==upper[j]) ? lower[j] : random()%2;
        c[j]=static_cast<int>(random()%23)-11;
      }
      for (std::size_t i=0; i<m; ++i) {
        I at_witness=0;
        for (std::size_t j=0; j<n; ++j) {
          A[i*n+j]=static_cast<int>(random()%15)-7;
          at_witness+=A[i*n+j]*witness[j];
        }
        // Half the models are constructed with a known feasible witness.
        b[i]=(trial%2==0) ? at_witness-static_cast<int>(random()%5)
                           : static_cast<int>(random()%31)-15;
        // Deliberately arbitrary candidate duals, including wrong signs.
        duals[i]=(static_cast<int>(random()%81)-20)/7.0;
      }
      I bound=0;
      require(Cert::lower_bound(A,b,c,lower,upper,duals,bound),
              "small valid arithmetic was rejected");
      I optimum=0;
      if (brute_min(A,b,c,lower,upper,optimum)) {
        ++feasible_cases;
        require(bound<=optimum,"certificate exceeds the true integer optimum");
      } else {
        ++infeasible_cases;
      }
    }
    require(feasible_cases>=trials/2,"random witness construction failed");
    require(infeasible_cases>0,"infeasible random models were not exercised");
    std::cout << "PASS certificate edge cases and " << trials
              << " exhaustive randomized models (" << feasible_cases
              << " feasible, " << infeasible_cases << " infeasible)\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
