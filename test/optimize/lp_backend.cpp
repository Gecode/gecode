#include <gecode/minimodel/lp-backend.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <type_traits>

using namespace Gecode::Experimental::LpRelaxation;

static void check(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}

static void sparse_checks() {
  const LinearModel input{{1,1,0, 0,1,1, 1,0,1},{1,1,1},{1,1,1}};
  Backend dense(input);
  SparseBackend sparse(sparse_model(input));
  check(sparse.model.nonzeros()==6 && sparse.model.row_start.size()==4,"CSR storage");
  for (unsigned box=0;box<27;++box) {
    std::vector<std::int64_t> lo(3),hi(3); auto remaining=box;
    for (unsigned j=0;j<3;++j) {
      const auto domain=remaining%3; remaining/=3;
      lo[j]=domain==1; hi[j]=domain!=2;
    }
    const auto d=dense.bound(lo,hi,true),s=sparse.bound(lo,hi,true);
    check(d.valid==s.valid,"sparse/dense bound validity");
    if (d.valid) {
      check(d.lower_bound==s.lower_bound && s.certificate,"sparse/dense bound value");
      std::int64_t restored=0,original=0;
      check(s.certificate->lower_bound({0,0,0},{1,1,1},restored) &&
            d.certificate->lower_bound({0,0,0},{1,1,1},original) && restored==original,
            "sparse certificate sibling reuse");
    }
  }
  // Input containers remain mutable. Each constructor takes a fresh snapshot;
  // changing one never changes already published dense/sparse backend models.
  auto changed=input;
  Backend before(changed); changed.a[0]=0;
  Backend after(changed);
  check(before.model.a[0]==1 && after.model.a[0]==0 &&
        before.SparseBackend::model.nonzeros()==6 && after.SparseBackend::model.nonzeros()==5,
        "legacy mutable model reused stale CSR");
  auto csr=sparse_model(input);
  SparseBackend immutable(csr); csr.a[0]=7;
  check(immutable.model.a[0]==1,"sparse input mutation changed published model");
  for (unsigned bad=0;bad<5;++bad) {
    auto invalid=sparse_model(input);
    if (bad==0) invalid.row_start.clear();
    if (bad==1) invalid.column[1]=invalid.column[0];
    if (bad==2) invalid.a[0]=0;
    if (bad==3) invalid.column[0]=3;
    if (bad==4) invalid.a[0]=1000000001LL;
    bool rejected=false;
    try { SparseBackend unused(std::move(invalid)); }
    catch (const std::invalid_argument&) { rejected=true; }
    check(rejected,"invalid sparse backend input accepted");
  }
  SparseLinearModel zero;
  zero.row_start={0,0}; zero.b={1}; zero.c={-3,4};
  SparseBackend constant(zero);
  const auto box=constant.bound({0,0},{1,1},true);
  check(box.valid && box.lower_bound==-3 && constant.statistics().lp_calls==0,
        "zero matrix numerical assertion or unsupported infeasibility inference");
  SparseBackend sibling(sparse_model({{1,1},{1},{2,9}}));
  bool first_ok=true,second_ok=true;
  std::thread first([&] { for (int i=0;i<20;++i) {
    auto r=sibling.bound({0,1},{0,1}); first_ok &= r.valid && r.lower_bound==9;
  } });
  std::thread second([&] { for (int i=0;i<20;++i) {
    auto r=sibling.bound({1,0},{1,0}); second_ok &= r.valid && r.lower_bound==2;
  } });
  first.join(); second.join();
  check(first_ok && second_ok,"sparse serialized sibling restoration");
  static_assert(std::is_base_of<SparseBackend,Backend>::value,"legacy sparse backend conversion");
}

int main() {
  sparse_checks();
  // Fractional triangle-cover optimum is 1.5, hence integer lower bound 2.
  Backend triangle({{1,1,0, 0,1,1, 1,0,1}, {1,1,1}, {1,1,1}});
  auto t = triangle.bound({0,0,0},{1,1,1});
  check(t.valid && t.lower_bound == 2, "triangle bound");
  check(std::abs(t.lp_objective-1.5) < 1e-8, "row-dual sign/LP objective");
  // Force one sibling to a high bound, then loosen it on another sibling.
  Backend sibling({{1,1}, {1}, {2,9}});
  auto a = sibling.bound({0,1},{0,1});
  auto b = sibling.bound({1,0},{1,0});
  auto c = sibling.bound({0,0},{1,1});
  check(a.valid && a.lower_bound == 9, "first sibling");
  check(b.valid && b.lower_bound == 2, "second sibling");
  check(c.valid && c.lower_bound == 2, "restored bounds");
  // An infeasibility status is only a statistic. HiGHS may still supply
  // multipliers, in which case only the exact checker can accept a bound.
  (void) sibling.bound({0,0},{0,0});
  auto recovered = sibling.bound({0,0},{1,1});
  check(recovered.valid && recovered.lower_bound == 2, "recovery after infeasibility");
  check(sibling.statistics().infeasible_status >= 1, "infeasibility statistics");
  check(!sibling.bound({-1,0},{1,1}).valid, "nonbinary rejection");
  check(!sibling.bound({0},{1}).valid, "dimension rejection");
  bool threw = false;
  try { Backend bad({{1}, {1}, {1,2}}); } catch (const std::invalid_argument&) { threw=true; }
  check(threw,"invalid dense model rejection");
  Backend empty({{}, {}, {}});
  check(empty.bound({},{}).valid,"empty model");
  Backend box({{}, {}, {-3,4}});
  check(box.bound({0,0},{1,1}).lower_bound == -3,"negative-cost empty-row box");
  // One shared backend is serialized across callers; every bound restored.
  bool first_ok = true, second_ok = true;
  std::thread first([&] { for(int i=0;i<20;++i) { auto r=sibling.bound({0,1},{0,1}); first_ok &= r.valid && r.lower_bound==9; } });
  std::thread second([&] { for(int i=0;i<20;++i) { auto r=sibling.bound({1,0},{1,0}); second_ok &= r.valid && r.lower_bound==2; } });
  first.join(); second.join();
  check(first_ok && second_ok,"serialized concurrent sibling calls");
  const auto stats=sibling.statistics();
  check(stats.lp_calls==45 && stats.valid_bounds+stats.rejected==47 && stats.rejected>=2,"statistics");
  std::cout << "{\"status\":\"passed\",\"highs_version\":\"" << highsVersion()
            << "\",\"checks\":14,\"serialized_thread_calls\":40,"
            << "\"cases\":[\"triangle_dual_sign_and_exact_bound\","
            << "\"sibling_bound_restoration\",\"recovery_after_infeasible_lp\","
            << "\"binary_and_dimension_checks\",\"empty_models\","
            << "\"serialized_callers\",\"sparse_equivalence_and_corruption\"],\"lp_calls\":" << stats.lp_calls
            << ",\"valid_bounds\":" << stats.valid_bounds
            << ",\"rejected\":" << stats.rejected << "}\n";
}
