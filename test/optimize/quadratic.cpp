#include <gecode/optimize/quadratic.hpp>
#include <gecode/optimize/quadratic_bound.hpp>
#include <gecode/optimize/native.hpp>
#include <gecode/optimize/native_lp.hpp>
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/workflow.hpp>
#include <gecode/optimize/relaxation.hpp>
#include <gecode/optimize/pool.hpp>
#if __has_include(<gecode/optimize/presolve.hpp>)
#include <gecode/optimize/presolve.hpp>
#endif
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/diagnostics.hpp>
#include <gecode/optimize/validate.hpp>
#include <cassert>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <type_traits>
using namespace Gecode::Optimize;
namespace {
template<class T,template<class>class Expression,class=void>struct Callable:std::false_type{};
template<class T,template<class>class Expression>struct Callable<T,Expression,std::void_t<Expression<T>>>:std::true_type{};
template<class T>using LinearCall=decltype(solve(std::declval<T>()));
template<class T>using NativeCall=decltype(solve_native(std::declval<T>()));
template<class T>using HybridCall=decltype(solve_native_lp(std::declval<T>()));
template<class T>using FrontierCall=decltype(solve_native_search(std::declval<T>()));
template<class T>using PoolCall=decltype(solve_pool(std::declval<T>()));
template<class T>using RepairCall=decltype(relax_feasibility(std::declval<T>(),RelaxationOptions{}));
template<class T>using LexCall=decltype(solve_lexicographic(std::declval<T>(),std::vector<LexicographicObjective>{}));
template<class T>using ConflictCall=decltype(analyze_conflict(std::declval<T>()));
template<class T>using SessionCall=decltype(std::declval<SolveSession>().solve(std::declval<T>()));
template<class T>using ValidateCall=decltype(validate(std::declval<T>(),std::vector<double>{}));
template<class T>using WriteCall=decltype(write_model(std::declval<T>(),std::string{}));
#if __has_include(<gecode/optimize/presolve.hpp>)
template<class T>using PresolveCall=decltype(presolve_integer(std::declval<T>()));
#endif
template<class T>void type_boundary(){
  static_assert(!Callable<T,LinearCall>::value&&!Callable<T,NativeCall>::value&&!Callable<T,HybridCall>::value,"");
  static_assert(!Callable<T,FrontierCall>::value&&!Callable<T,PoolCall>::value&&!Callable<T,RepairCall>::value,"");
  static_assert(!Callable<T,LexCall>::value&&!Callable<T,ConflictCall>::value&&!Callable<T,SessionCall>::value,"");
  static_assert(!Callable<T,ValidateCall>::value&&!Callable<T,WriteCall>::value,"");
#if __has_include(<gecode/optimize/presolve.hpp>)
  static_assert(!Callable<T,PresolveCall>::value,"");
#endif
}
std::size_t solved=0;
void close(double a,double b,double t=1e-6){assert(std::abs(a-b)<=t);}
void optimum(const QuadraticResult& r,double expected){
  if(r.result.termination!=Termination::Optimal)std::cerr<<"Unexpected "<<to_string(r.result.termination)<<": "<<r.result.message<<" obj "<<r.result.objective.value_or(-999)<<" gap "<<r.checks.gap_upper_bound.value_or(-999)<<"\n";
  assert(r.result.termination==Termination::Optimal);assert(r.result.has_solution());close(*r.result.objective,expected);
  assert(r.checks.kkt_valid&&r.checks.bound_valid&&r.checks.gap_upper_bound);assert(*r.checks.gap_upper_bound<=1e-6);
  ++solved;
}
template<class F>void rejects(F f){bool thrown=false;try{f();}catch(const ModelError&){thrown=true;}assert(thrown);}
// Small exact rational KKT oracle. Test coefficients keep int64 arithmetic bounded.
struct Rat {
  std::int64_t n=0,d=1;
  Rat(std::int64_t a=0,std::int64_t b=1):n(a),d(b){assert(b);if(d<0){n=-n;d=-d;}auto g=std::gcd(n,d);n/=g;d/=g;}
  double value()const{return double(n)/d;}
};
Rat operator+(Rat a,Rat b){return {a.n*b.d+b.n*a.d,a.d*b.d};}
Rat operator-(Rat a,Rat b){return {a.n*b.d-b.n*a.d,a.d*b.d};}
Rat operator*(Rat a,Rat b){return {a.n*b.n,a.d*b.d};}
Rat operator/(Rat a,Rat b){return {a.n*b.d,a.d*b.n};}
bool operator<(Rat a,Rat b){return a.n*b.d<b.n*a.d;}
bool operator<=(Rat a,Rat b){return !(b<a);}
struct Point{Rat x,y;};
struct Halfspace{int x,y,b;};
// Objective x²+y²+(a*x+b*y+c)²+d*x+e*y, strictly convex.
Rat oracle(int a,int b,int c,int d,int e,const std::vector<Halfspace>& rows){
  const Rat q00=2*(1+a*a),q01=2*a*b,q11=2*(1+b*b),p0=2*a*c+d,p1=2*b*c+e;
  std::vector<Point> points;
  const auto det=q00*q11-q01*q01;
  points.push_back({(q01*p1-q11*p0)/det,(q01*p0-q00*p1)/det});
  for(const auto& r:rows){
    Point base=r.x?Point{Rat(r.b,r.x),0}:Point{0,Rat(r.b,r.y)};
    const Rat tx=-r.y,ty=r.x;
    const Rat curvature=q00*tx*tx+Rat(2)*q01*tx*ty+q11*ty*ty;
    const Rat slope=tx*(q00*base.x+q01*base.y+p0)+ty*(q01*base.x+q11*base.y+p1);
    const Rat t=(Rat(0)-slope)/curvature;points.push_back({base.x+t*tx,base.y+t*ty});
  }
  for(std::size_t i=0;i<rows.size();++i)for(std::size_t j=0;j<i;++j){
    const auto& r=rows[i];const auto& s=rows[j];const int determinant=r.x*s.y-r.y*s.x;
    if(determinant)points.push_back({Rat(r.b*s.y-r.y*s.b,determinant),Rat(r.x*s.b-r.b*s.x,determinant)});
  }
  std::optional<Rat> best;
  for(auto p:points){bool feasible=true;for(auto r:rows)feasible &= Rat(r.x)*p.x+Rat(r.y)*p.y<=Rat(r.b);if(!feasible)continue;
    const auto z=Rat(a)*p.x+Rat(b)*p.y+Rat(c);
    const auto f=p.x*p.x+p.y*p.y+z*z+Rat(d)*p.x+Rat(e)*p.y;
    if(!best||f<*best)best=f;
  }
  assert(best);return *best;
}
#ifdef GECODE_QUADRATIC_TEST_HOOKS
int fake=0;
std::string cancel_event;
std::string throw_event;
std::shared_ptr<CancellationToken> token;
#endif
}
#ifdef GECODE_QUADRATIC_TEST_HOOKS
namespace Gecode{namespace Optimize{namespace Detail{
double quadratic_test_regularization=0;
void quadratic_test_event(const char* event){if(cancel_event==event&&token)token->cancel();if(throw_event==event)throw std::bad_alloc();}
bool quadratic_test_oracle(const QuadraticSnapshot& q,const QuadraticOptions&,QuadraticRaw& raw){
  if(!fake)return false;
  raw.model_id=q.id();raw.revision=q.revision();for(auto& v:q.variables())raw.active_variables.push_back(v.active);
  raw.values.assign(q.variables().size(),0);raw.residual_values.assign(q.squares().size(),0);
  raw.row_duals.assign(q.rows().size(),0);raw.column_duals.assign(q.variables().size(),0);
  raw.value_valid=true;raw.dual_valid=true;raw.objective=q.linear_part().offset;raw.termination=Termination::Optimal;
  if(fake==2)raw.values[0]=0.25;
  if(fake==3)++raw.model_id;
  if(fake==4)++raw.revision;
  if(fake==5)raw.active_variables.clear();
  if(fake==6)raw.objective=1;
  if(fake==7)raw.residual_values[0]=1;
  if(fake==8)raw.column_duals[0]=1;
  if(fake==9)raw.dual_valid=false;
  if(fake==10){raw.termination=Termination::Infeasible;raw.value_valid=false;}
  if(fake==11)raw.termination=Termination::Unbounded;
  if(fake==12)raw.termination=Termination::IterationLimit;
  if(fake==13)raw.objective=std::numeric_limits<double>::quiet_NaN();
  if(fake==14)raw.values[0]=std::numeric_limits<double>::quiet_NaN();
  if(fake==15){raw.values[0]=1;raw.residual_values[0]=1;raw.objective=q.linear_part().offset+1;raw.column_duals[0]=2;}
  return true;
}
}}}
#endif
int main(){
  type_boundary<const QuadraticModel&>();type_boundary<const QuadraticSnapshot&>();
  static_assert(!std::is_convertible<QuadraticSnapshot,ModelSnapshot>::value,"");
  static_assert(!std::is_convertible<QuadraticModel,Model&>::value,"");
  using Ordinary=SolveResult(*)(const ModelSnapshot&,const SolveOptions&);
  static_assert(!std::is_invocable<Ordinary,const QuadraticSnapshot&,const SolveOptions&>::value,"");
  static_assert(!std::is_constructible<SolveSession,QuadraticSnapshot>::value,"");
  QuadraticModel m;const auto x=m.add_continuous(-2,2);const auto dead=m.add_continuous(-1,1);m.remove(dead);
  m.minimize_squares({{{{x,2},{x,-1},{x,0}},0,1,"square"}});
  const auto historical=m.snapshot();const auto rev=m.revision();assert(historical.squares()[0].terms[0].coefficient==1);
  rejects([&]{m.remove(x);});assert(m.revision()==rev);
  rejects([&]{m.minimize_squares({{{{x,1}},0,-1,""}});});assert(m.revision()==rev);
  rejects([&]{m.add_continuous(0,std::numeric_limits<double>::infinity());});assert(m.revision()==rev);
  QuadraticModel foreign;auto alien=foreign.add_continuous(-1,1);
  rejects([&]{m.minimize_squares({{{{alien,1}},0,1,""}});});assert(m.revision()==rev);
  m.minimize_squares({{{{x,1}},-1,1,""}});assert(m.revision()==rev+1);assert(historical.squares()[0].offset==0);
  auto snap=historical;auto moved_snapshot=std::move(snap);assert(snap.id()==moved_snapshot.id());
  QuadraticModel moved=std::move(m);assert(m.id()==0);rejects([&]{m.snapshot();});assert(moved.id()==historical.id());
  auto check=validate_quadratic(historical,{0,std::numeric_limits<double>::quiet_NaN()});assert(check.primal_valid&&check.objective_valid);
  assert(!validate_quadratic(historical,{3,0}).primal_valid);
  QuadraticOptions o;o.solve.time_limit_seconds=0;assert(solve_quadratic(historical,o).result.termination==Termination::TimeLimit);
  o={};o.solve.cancellation=std::make_shared<CancellationToken>();o.solve.cancellation->cancel();assert(solve_quadratic(historical,o).result.termination==Termination::Cancelled);
  o={};o.optimality_tolerance=-1;assert(solve_quadratic(historical,o).result.termination==Termination::InvalidModel);
  for(int k=0;k<5;++k){o={};if(k==0)o.solve.backend=Backend::Native;if(k==1)o.solve.guarantee=Guarantee::Exact;if(k==2)o.solve.node_limit=0;if(k==3)o.solve.threads=2;if(k==4)o.solve.primal_start={{x,0}};assert(solve_quadratic(historical,o).result.termination==Termination::Unsupported);}
  const int rounding=std::fegetround();assert(std::fesetround(FE_DOWNWARD)==0);assert(solve_quadratic(historical).result.termination==Termination::Unsupported);assert(std::fesetround(rounding)==0);
#ifdef GECODE_QUADRATIC_TEST_HOOKS
  QuadraticModel fm;auto fx=fm.add_continuous(-2,2);fm.minimize_squares({{{{fx,1}},0,1,""}});
  for(fake=1;fake<=14;++fake){auto r=solve_quadratic(fm);if(fake==1)optimum(r,0);else if(fake==12){assert(r.result.termination==Termination::IterationLimit);assert(r.result.has_solution());}else assert(r.result.termination==Termination::NumericalFailure);}
  // Large constants cannot make a nonoptimal point pass the closure/KKT gate.
  fm.minimize_squares({{{{fx,1}},0,1,""}},{},1e16);fake=15;assert(solve_quadratic(fm).result.termination==Termination::NumericalFailure);
  fake=1;
  for(const char* name:{"after_preflight","after_validation"}){token=std::make_shared<CancellationToken>();cancel_event=name;o={};o.solve.cancellation=token;auto r=solve_quadratic(fm,o);assert(r.result.termination==Termination::Cancelled);assert(!r.result.has_solution());assert(!r.checks.bound_valid);}
  cancel_event.clear();token.reset();fake=0;
  fake=1;throw_event="after_validation";const auto failed=solve_quadratic(fm);
  assert(failed.result.termination==Termination::MemoryLimit&&!failed.result.has_solution());
  assert(failed.result.model_id==fm.id()&&failed.result.revision==fm.revision());throw_event.clear();fake=0;
#endif
  if(!quadratic_capabilities().available){auto r=solve_quadratic(historical);assert(r.result.termination==Termination::Unsupported);assert(!r.result.has_solution());std::cout<<"quadratic foundation/backend-off passed\n";return 0;}
  optimum(solve_quadratic(historical),0);optimum(solve_quadratic(moved),0);
  const auto old_result=solve_quadratic(historical);close(old_result.result.value(x),0);rejects([&]{old_result.result.value(dead);});rejects([&]{old_result.result.value(alien);});
  std::mt19937 rng(119);
#ifdef GECODE_QUADRATIC_TEST_HOOKS
  unsigned perturbed_accepted=0,perturbed_rejected=0;
#endif
  for(int k=0;k<180;++k){
    const int a=int(rng()%5)-2,b=int(rng()%5)-2,c=int(rng()%5)-2,d=int(rng()%5)-2,e=int(rng()%5)-2;
    std::vector<Halfspace> rows={{1,0,2},{-1,0,2},{0,1,2},{0,-1,2},{1,1,1},{-1,2,2}};
    const auto exact=oracle(a,b,c,d,e,rows);
    for(int sense=0;sense<2;++sense){QuadraticModel q;auto u=q.add_continuous(-2,2),v=q.add_continuous(-2,2);
      for(std::size_t i=4;i<rows.size();++i)q.add_row({{u,double(rows[i].x)},{v,double(rows[i].y)}},-std::numeric_limits<double>::infinity(),rows[i].b);
      std::vector<WeightedSquare> squares={{{{u,1}},0,1,"x"},{{{v,1}},0,1,"y"},{{{u,double(a)},{v,double(b)}},double(c),1,"coupled"}};
      if(sense)q.maximize_concave_squares(squares,{{u,double(-d)},{v,double(-e)}},7);else q.minimize_squares(squares,{{u,double(d)},{v,double(e)}},7);
      optimum(solve_quadratic(q),7+(sense?-1:1)*exact.value());
#ifdef GECODE_QUADRATIC_TEST_HOOKS
      Detail::quadratic_test_regularization=1e-7;
      const auto perturbed=solve_quadratic(q);
      if(perturbed.result.termination==Termination::Optimal){
        close(*perturbed.result.objective,7+(sense?-1:1)*exact.value());
        assert(perturbed.checks.kkt_valid&&perturbed.checks.bound_valid);++perturbed_accepted;
      }else{
        assert(perturbed.result.termination==Termination::NumericalFailure || perturbed.result.termination==Termination::IterationLimit);
        ++perturbed_rejected;
      }
      Detail::quadratic_test_regularization=0;
#endif
    }
  }
  // Singular, equality-constrained, constant, fixed, and ill-conditioned cases.
  for(int k=0;k<6;++k){QuadraticModel q;auto u=q.add_continuous(-4,4),v=q.add_continuous(-4,4);
    if(k==0)q.minimize_squares({{{{u,1},{v,1}},-2,1,"rank-one"}});
    if(k==1){q.minimize_squares({{{{u,1}},0,1,""},{{{v,1}},0,1,""}});q.add_row({{u,1},{v,1}},2,2);}
    if(k==2)q.minimize_squares({{{},3,2,"constant"}});
    if(k==3){q.set_bounds(u,2,2);q.minimize_squares({{{{u,1}},0,1,""}});}
    if(k==4)q.minimize_squares({{{{u,1}},-1,1e-6,""},{{{v,1}},-2,1e6,""}});
    if(k==5)q.minimize_squares({},{{u,1}},5);
    optimum(solve_quadratic(q),k==1?2:k==2?18:k==3?4:k==5?1:0);
  }
  QuadraticModel empty;empty.minimize_squares({}, {}, 7);optimum(solve_quadratic(empty),7);
  QuadraticModel constant_only;constant_only.minimize_squares({{{},3,2,"constant"}});
  optimum(solve_quadratic(constant_only),18);
  for(int variant=0;variant<3;++variant){
    QuadraticModel q;auto a=q.add_continuous(-3,3),b=q.add_continuous(-3,3);
    q.add_row({{a,1},{b,1}},1,1);
    std::vector<WeightedSquare> squares={{{{a,1}},0,1,"a"},{{{b,1}},0,1,"b"}};
    if(variant==1)squares[0].terms[0].coefficient=-1;
    if(variant==2){squares[0].weight=0.5;squares.push_back(squares[0]);}
    q.minimize_squares(squares);optimum(solve_quadratic(q),0.5);
  }
  QuadraticModel offset_model;auto offset_x=offset_model.add_continuous(-1,1);
  offset_model.minimize_squares({{{{offset_x,1}},-1,1,""}}, {{offset_x,2}}, 1e16);
  const auto offset_result=solve_quadratic(offset_model);optimum(offset_result,1e16);
  assert(*offset_result.result.absolute_gap>=1);
  assert(*offset_result.checks.gap_upper_bound<=1e-6);
  assert(*offset_result.result.best_bound<*offset_result.result.objective);
  empty.add_row({},1,2);auto infeasible=solve_quadratic(empty);assert(infeasible.result.termination==Termination::Infeasible&&!infeasible.result.has_solution());
  QuadraticModel bad;auto u=bad.add_continuous(-1,1);bad.minimize_squares({{{{u,1}},0,1,""}});bad.add_row({{u,1}},2,3);assert(solve_quadratic(bad).result.termination==Termination::Infeasible);
  QuadraticModel tiny;tiny.minimize_squares({{{},0,1e-15,""}});assert(solve_quadratic(tiny).result.termination==Termination::Unsupported);
  o={};o.max_auxiliary_variables=0;assert(solve_quadratic(historical,o).result.termination==Termination::Unsupported);
  o={};o.max_lifted_nonzeros=0;assert(solve_quadratic(historical,o).result.termination==Termination::Unsupported);
  o={};o.iteration_limit=0;auto limited=solve_quadratic(moved,o);assert(limited.result.termination==Termination::IterationLimit || limited.result.termination==Termination::Optimal);
#ifdef GECODE_QUADRATIC_TEST_HOOKS
  std::cout<<"regularization exact-oracle acceptance: zero360/360, vendor default "<<perturbed_accepted<<"/360, rejected "<<perturbed_rejected<<"\n";
  unsigned zero_accepted=0, zero_rejected=0, default_accepted=0, default_rejected=0;
  for(double reg:{0.0,1e-7})for(int k=0;k<6;++k){
    Detail::quadratic_test_regularization=reg;
    QuadraticModel q;auto u=q.add_continuous(-4,4),v=q.add_continuous(-4,4);
    const double weight=k==0?1:k==1?1e-2:k==2?1e-4:k==3?1e-6:k==4?1e-8:1e-10;
    q.minimize_squares({{{{u,1},{v,1}},-2,weight,"singular-scaled"}});
    auto r=solve_quadratic(q);assert(r.regularization==reg);
    std::cout<<"regularization="<<reg<<" weight="<<weight<<" status="<<to_string(r.result.termination)<<" gap="<<r.checks.gap_upper_bound.value_or(-1)<<" obj="<<r.result.objective.value_or(-1)<<"\n";
    if(reg==0){if(r.result.termination==Termination::Optimal){optimum(r,0);++zero_accepted;}else ++zero_rejected;}
    else if(r.result.termination==Termination::Optimal)++default_accepted;
    else{assert(r.result.termination==Termination::NumericalFailure || r.result.termination==Termination::IterationLimit);++default_rejected;}
  }
  Detail::quadratic_test_regularization=0;
  std::cout<<"regularization original-model acceptance: zero "<<zero_accepted<<"/6 (failed "<<zero_rejected<<"), vendor default "<<default_accepted<<"/6, rejected "<<default_rejected<<"\n";
  for(const char* name:{"before_backend","after_backend"}){token=std::make_shared<CancellationToken>();cancel_event=name;o={};o.solve.cancellation=token;auto r=solve_quadratic(moved,o);assert(r.result.termination==Termination::Cancelled);assert(!r.result.has_solution());}
#endif
  std::cout<<"quadratic: "<<solved<<" analytic/rational oracle solves passed\n";
}
