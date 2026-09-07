// Explicit bounded-integer import, sibling restoration and range proof gates.
#include <gecode/minimodel/lp-backend.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace LP=Gecode::Experimental::LpRelaxation;
using I=std::int64_t;
static void check(bool okay,const char* message) {if(!okay)throw std::runtime_error(message);}
static LP::BoundedIntegerModel model(LP::LinearModel dense,std::vector<I> lo,std::vector<I> hi) {
  return {LP::sparse_model(dense),std::move(lo),std::move(hi)};
}
static void invalid_models() {
  const auto original=model({{1,1},{-1},{2,9}},{-3,-3},{3,3});
  for(unsigned bad=0;bad<10;++bad) {
    auto m=original;
    if(bad==0)m.lower.pop_back();
    if(bad==1)m.upper[0]=m.lower[0]-1;
    if(bad==2)m.lower[0]=static_cast<I>(Gecode::Int::Limits::min)-1;
    if(bad==3)m.upper[0]=static_cast<I>(Gecode::Int::Limits::max)+1;
    if(bad==4)m.linear.column[1]=m.linear.column[0];
    if(bad==5)m.linear.a[0]=0;
    if(bad==6)m.linear.b[0]=1000000001LL;
    if(bad==7)m.linear.a[0]=1000000000LL; // Native row activity overflow.
    if(bad==8)m.linear.c[0]=1000000000LL; // Native objective equality overflow.
    if(bad==9)m.linear.row_start.back()=1;
    bool validator=false,backend=false;
    try {LP::validate_integer_model(m);}catch(const std::invalid_argument&){validator=true;}
    try {LP::BoundedIntegerBackend unused(m);}catch(const std::invalid_argument&){backend=true;}
    check(validator && backend,"unsafe integer model accepted");
  }
  const I maximum=Gecode::Int::Limits::max;
  LP::validate_integer_model(model({{1},{0},{0}},{-maximum},{maximum}));
  LP::validate_integer_model(model({{},{},{1}},{-maximum/2},{maximum/2}));
  auto excess=model({{},{},{1}},{0},{maximum/2+1});bool rejected=false;
  try {LP::validate_integer_model(excess);}catch(const std::invalid_argument&){rejected=true;}
  check(rejected,"objective equality envelope boundary not enforced");
}
int main() {
 try {
  static_assert(!std::is_base_of<LP::SparseBackend,LP::BoundedIntegerBackend>::value,
                "integer workspace must not enter strict binary API");
  invalid_models();
  LP::BoundedIntegerBackend triangle(model({{1,1,0,0,1,1,1,0,1},{3,3,3},{1,1,1}},
                                           {0,0,0},{3,3,3}));
  const auto t=triangle.bound({0,0,0},{3,3,3},true);
  check(t.valid && t.lower_bound==5 && std::abs(t.lp_objective-4.5)<1e-8 && t.certificate,
        "fractional integer LP certificate ceiling");
  I reevaluated=0;
  check(t.certificate->lower_bound_integer({-3,-3,-3},{3,3,3},reevaluated) && reevaluated==5,
        "integer certificate not independently reusable on a looser signed box");

  auto source=model({{1,1},{-1},{2,9}},{-3,-3},{3,3});
  LP::BoundedIntegerBackend sibling(source);
  source.linear.a[0]=7;source.lower[0]=0;source.upper[1]=0;
  check(sibling.model.linear.a[0]==1 && sibling.model.lower[0]==-3 && sibling.model.upper[1]==3,
        "input mutation changed immutable integer model");
  auto a=sibling.bound({-3,2},{-3,2},true),b=sibling.bound({2,-3},{2,-3},true);
  auto restored=sibling.bound({-3,-3},{3,3},true);
  check(a.valid && a.lower_bound==12 && b.valid && b.lower_bound==-23 &&
        restored.valid && restored.lower_bound==-23,"signed sibling bounds were not fully restored");
  (void)sibling.bound({-3,-3},{-3,-3});
  restored=sibling.bound({-3,-3},{3,3});
  check(restored.valid && restored.lower_bound==-23 && sibling.statistics().infeasible_status>0,
        "integer workspace did not recover after numerical infeasibility");
  check(!sibling.bound({-4,-3},{3,3}).valid && !sibling.bound({-3,-3},{3,4}).valid &&
        !sibling.bound({-3},{3}).valid && !sibling.bound({3,-3},{2,3}).valid,
        "integer workspace accepted invalid/outside-original boxes");
  bool first_ok=true,second_ok=true;
  std::thread first([&]{for(int i=0;i<40;++i){const auto r=sibling.bound({-3,2},{-3,2});first_ok&=r.valid && r.lower_bound==12;}});
  std::thread second([&]{for(int i=0;i<40;++i){const auto r=sibling.bound({2,-3},{2,-3});second_ok&=r.valid && r.lower_bound==-23;}});
  first.join();second.join();check(first_ok && second_ok,"serialized integer sibling calls changed bounds");

  LP::BoundedIntegerBackend empty(model({{},{},{}},{},{}));
  const auto e=empty.bound({},{});check(e.valid && e.lower_bound==0,"empty integer box");
  LP::BoundedIntegerBackend zero(model({{0,0},{1},{-3,4}},{-2,-3},{2,3}));
  const auto z=zero.bound({-2,-3},{2,3},true);
  check(z.valid && z.lower_bound==-18 && zero.statistics().lp_calls==0,
        "zero matrix skipped original feasibility or attempted invalid HiGHS import");
  LP::Backend binary({{},{},{1}});
  check(!binary.bound({-1},{1}).valid && !binary.bound({0},{2}).valid,"legacy binary contract widened silently");
  std::cout<<"PASS integer backend: signed restored siblings, 80 serialized calls, exact ceiling, "
               "original-domain gates, zero matrices and native activity boundaries\n";
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
