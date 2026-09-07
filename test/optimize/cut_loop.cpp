// Independent finite original-model oracle and actual numerical LP selection.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/minimodel/lp-cut-loop.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <thread>
#ifdef GECODE_LP_CUT_LOOP_TEST_NATIVE
#include <gecode/search.hh>
#endif

namespace LP=Gecode::Experimental::LpRelaxation;
namespace Cuts=LP::Cuts;
using I=std::int64_t;
using Model=LP::BoundedIntegerModel;
static std::size_t runs=0,points=0;
#ifdef GECODE_LP_CUT_LOOP_TEST_HOOKS
static thread_local std::map<std::string,unsigned> events;
static thread_local std::string failed_event;
static thread_local unsigned failed_occurrence=0,event_occurrence=0;
static thread_local bool allocation_failure=false;
static thread_local bool cancel_on_event=false,cancelled_on_event=false;
namespace Gecode { namespace Experimental { namespace LpRelaxation { namespace Cuts {
void root_cut_loop_test_event(const char* name) {
  ++events[name];
  if(failed_event!=name || ++event_occurrence!=failed_occurrence)return;
  if(cancel_on_event){cancelled_on_event=true;return;}
  if(allocation_failure)throw std::bad_alloc();
  throw std::runtime_error("injected backend/augmentation error");
}
}}}}
#endif

static Model model(LP::LinearModel dense,std::vector<I> lower,std::vector<I> upper) {
  return {LP::sparse_model(dense),std::move(lower),std::move(upper)};
}
static Model fixture() {return model({{-3,-3},{-5},{-2,-2}},{0,0},{1,1});}
static bool same(const Model& a,const Model& b) {
  return a.lower==b.lower && a.upper==b.upper && a.linear.a==b.linear.a &&
    a.linear.b==b.linear.b && a.linear.c==b.linear.c &&
    a.linear.column==b.linear.column && a.linear.row_start==b.linear.row_start;
}
static bool feasible(const Model& m,const std::vector<I>& x) {
  for (std::size_t j=0;j<x.size();++j) if (x[j]<m.lower[j] || x[j]>m.upper[j]) return false;
  for (std::size_t i=0;i<m.linear.b.size();++i) {
    I sum=0;
    for (auto k=m.linear.row_start[i];k<m.linear.row_start[i+1];++k)
      sum+=m.linear.a[k]*x[m.linear.column[k]];
    if (sum<m.linear.b[i]) return false;
  }
  return true;
}
static I objective(const Model& m,const std::vector<I>& x) {
  I sum=0;for(std::size_t j=0;j<x.size();++j) sum+=m.linear.c[j]*x[j];return sum;
}
static void provenance(const Cuts::SourceModel& source,const Cuts::SourceModel& augmented,
                       const std::vector<Cuts::VerifiedCut>& cuts) {
  const auto& original=source.model();const auto& m=augmented.model();
  assert(m.lower==original.lower && m.upper==original.upper && m.linear.c==original.linear.c);
  const auto rows=original.linear.b.size(),nnz=original.linear.a.size();
  assert(m.linear.b.size()==rows+cuts.size());
  assert(std::equal(original.linear.row_start.begin(),original.linear.row_start.end(),m.linear.row_start.begin()));
  assert(std::equal(original.linear.a.begin(),original.linear.a.end(),m.linear.a.begin()));
  assert(std::equal(original.linear.column.begin(),original.linear.column.end(),m.linear.column.begin()));
  assert(std::equal(original.linear.b.begin(),original.linear.b.end(),m.linear.b.begin()));
  std::size_t start=nnz;
  for (std::size_t i=0;i<cuts.size();++i) {
    const auto& cut=cuts[i];assert(cut.source().same_identity(source));
    assert(cut.proof().scope.kind==Cuts::ScopeKind::Global);
    const auto independently=Cuts::verify_cover(source,cut.proof());
    assert(independently.inequality().column==cut.inequality().column &&
           independently.inequality().coefficient==cut.inequality().coefficient &&
           independently.inequality().upper==cut.inequality().upper);
    const auto& row=cut.inequality();
    assert(m.linear.row_start[rows+i]==start && m.linear.b[rows+i]==-row.upper);
    for(std::size_t k=0;k<row.column.size();++k,++start)
      assert(m.linear.column[start]==row.column[k] && m.linear.a[start]==-row.coefficient[k]);
  }
  assert(start==m.linear.a.size() && m.linear.row_start.back()==start);
  LP::validate_integer_model(m);
}
static void evidence(const Cuts::SourceModel& source,const Cuts::RootLoopOptions& options,
                     const Cuts::RootLoopResult& result) {
  ++runs;assert(result.original.same_identity(source));
  assert(result.stats.work<=options.max_work && result.stats.rounds<=options.max_rounds);
  assert(result.cuts.size()<=options.pool.max_cuts);
  provenance(source,result.model,result.cuts);
  if(result.backend) assert(same(result.backend->model,result.model.model()));
  if(result.best_bound) {
    const auto& best=*result.best_bound;
    assert(best.bound.valid && best.bound.certificate && !best.bound.primal_suggestion);
    provenance(source,best.model,best.cuts);
    I checked=0;
    assert(best.bound.certificate->lower_bound_integer(source.model().lower,source.model().upper,checked));
    assert(checked==best.bound.lower_bound);
  }
  const auto& original=source.model();std::vector<I> x(original.lower.size());
  std::optional<I> optimum;
  const auto visit=[&](const auto& self,std::size_t j)->void {
    if(j==x.size()) {
      ++points;const bool valid=feasible(original,x);
      assert(valid==feasible(result.model.model(),x));
      if(result.best_bound) assert(valid==feasible(result.best_bound->model.model(),x));
      if(valid) {const auto value=objective(original,x);if(!optimum || value<*optimum) optimum=value;}
      return;
    }
    for(I value=original.lower[j];value<=original.upper[j];++value){x[j]=value;self(self,j+1);}
  };
  visit(visit,0);
  if(optimum && result.best_bound) assert(result.best_bound->bound.lower_bound<=*optimum);
}
static void actual_lp_and_ownership() {
  auto input=fixture();Cuts::SourceModel source(input);
  auto old=std::make_shared<LP::BoundedIntegerBackend>(input);
  const auto old_bound=old->bound(input.lower,input.upper,true,true);
  assert(old_bound.valid && old_bound.lower_bound==-3 && old_bound.primal_suggestion &&
    std::abs(old_bound.lp_objective+10.0/3.0)<1e-8);
  assert(!old->bound(input.lower,input.upper).primal_suggestion);
  auto copied=old_bound;(*copied.primal_suggestion)[0]=999;
  assert((*old_bound.primal_suggestion)[0]<=1);
  input.linear.a[0]=11;input.upper[0]=0;
  assert(source.model().linear.a[0]==-3 && old->model.upper[0]==1);
  Cuts::RootLoopOptions options;
  auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::NoNewCuts);
  assert(result.stats.lp_calls>=2 && result.stats.augmentations==1 && result.cuts.size()==1);
  assert(result.best_bound && result.best_bound->bound.lower_bound==-2);
  assert(result.best_bound->model.model().linear.b.size()==2);
  assert(old->model.linear.b.size()==1 && old->bound({0,0},{1,1},true).lower_bound==-3);
  I checked=0;assert(old_bound.certificate->lower_bound_integer({0,0},{1,1},checked) && checked==-3);
  auto historical=result;result.cuts.clear();result.backend.reset();
  assert(historical.cuts.size()==1 && historical.backend->model.linear.b.size()==2);
  LP::SparseBackend binary(source.model().linear);
  assert(binary.bound({0,0},{1,1},true,true).primal_suggestion);
  assert(!binary.bound({0,0},{1,1},true).primal_suggestion);
}
static void rationalization() {
  Cuts::SourceModel source(model({{},{},{0,0,0}},{-2,0,2},{2,1,2}));
  const auto projected=Cuts::rational_selection_point(source,{-3,1.0001,99},8);
  assert(projected.projected_coordinates==3 && projected.point.numerator==std::vector<I>({-16,8,16}));
  const auto rounded=Cuts::rational_selection_point(source,{-0.0625,0.0625,2},8);
  assert(!rounded.projected_coordinates && rounded.point.numerator==std::vector<I>({-1,1,16}));
  for(I d:{I(0),I(-1),I(3),I(2097152)}) {
    bool rejected=false;try{(void)Cuts::rational_selection_point(source,{0,0,2},d);}
    catch(const std::invalid_argument&){rejected=true;}assert(rejected);
  }
  for(double bad:{std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
    bool rejected=false;try{(void)Cuts::rational_selection_point(source,{bad,0,2});}
    catch(const std::invalid_argument&){rejected=true;}assert(rejected);
  }
  bool rejected=false;try{(void)Cuts::rational_selection_point(source,{0});}
  catch(const std::invalid_argument&){rejected=true;}assert(rejected);
  const auto limit=Gecode::Int::Limits::max;
  Cuts::SourceModel endpoints(model({{},{},{0,0}},{-limit,limit},{-limit,limit}));
  const auto exact=Cuts::rational_selection_point(endpoints,{-double(limit),double(limit)});
  assert(exact.point.numerator==std::vector<I>({-I(limit)*1048576,I(limit)*1048576}));
}
static void budgets() {
  Cuts::SourceModel source(fixture());Cuts::RootLoopOptions options;
  options.max_rounds=0;auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::RoundLimit && !result.backend && !result.best_bound);
  options={};options.max_rounds=1;result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::RoundLimit && result.cuts.size()==1);
  assert(result.best_bound->bound.lower_bound==-3 && result.best_bound->cuts.empty());
  assert(result.model.model().linear.b.size()==2 && result.best_bound->model.model().linear.b.size()==1);
  for(std::size_t work=0;work<650;++work) {
    options={};options.max_work=work;
    const auto limited=Cuts::root_cover_loop(source,options);evidence(source,options,limited);
    assert(limited.completion==Cuts::RootLoopCompletion::WorkLimit ||
           limited.completion==Cuts::RootLoopCompletion::NoNewCuts);
    if(limited.completion==Cuts::RootLoopCompletion::WorkLimit && limited.best_bound)
      assert(limited.best_bound->bound.lower_bound<=-2);
  }
  for(unsigned kind=0;kind<6;++kind) {
    options={};
    if(kind==0)options.max_columns=1;
    if(kind==1)options.max_rows=1;
    if(kind==2)options.max_nonzeros=3;
    if(kind==3)options.pool.max_cuts=0;
    if(kind==4)options.pool.max_nonzeros=1;
    if(kind==5)options.max_rows=0;
    result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
    assert(result.completion==Cuts::RootLoopCompletion::StorageLimit && result.cuts.empty());
  }
  options={};options.separation.max_rows=0;result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::SeparationLimit);
  options={};options.separation.max_work=0;result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::WorkLimit);
  options={};options.separation.max_cuts=0;result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::SeparationLimit);
  options={};options.deadline=std::chrono::steady_clock::now();result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::TimeLimit && !result.backend);
  options={};options.stop_requested=[](){return true;};result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  assert(result.completion==Cuts::RootLoopCompletion::Cancelled && !result.backend);
  std::size_t checkpoints=0;
  options={};options.stop_requested=[&](){++checkpoints;return false;};
  result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
  const auto total=checkpoints;
  for(std::size_t stop=1;stop<=total;++stop) for(unsigned mode=0;mode<4;++mode) {
    checkpoints=0;options={};
    options.stop_requested=[&]() {
      if(++checkpoints!=stop)return false;
      if(mode==1)throw std::runtime_error("callback failure");
      if(mode==2)throw std::bad_alloc();
      if(mode==3){options.deadline=std::chrono::steady_clock::now();return false;}
      return true;
    };
    const auto partial=Cuts::root_cover_loop(source,options);evidence(source,options,partial);
    assert(checkpoints==stop);
    assert(partial.completion==(mode==3?Cuts::RootLoopCompletion::TimeLimit:
      mode?Cuts::RootLoopCompletion::CallbackError:Cuts::RootLoopCompletion::Cancelled));
  }
}
static void edge_models() {
  for(const auto& value:{
      model({{1},{0},{1}},{0},{1}), // Integral LP; no violated cover.
      model({{-1,-1},{-3},{-1,-1}},{0,0},{3,3}), // Unsupported free general terms.
      model({{},{},{0}},{-1},{1}), // No matrix, no raw LP suggestion.
      model({{},{1},{}},{},{}), // Exact contradiction, no floating inference.
      model({{1,-1},{-1},{0,0}},{-2,0},{-2,1}), // Fixed negative integer substitution.
      model({{1},{2},{0}},{0},{1}) // Floating infeasible status must not prune.
    }) {
    Cuts::SourceModel source(value);Cuts::RootLoopOptions options;
    const auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
    if(value.upper==std::vector<I>({3,3})) assert(result.stats.unsupported_rows>0 && result.cuts.empty());
    if(value.linear.a.empty()) assert(result.completion==Cuts::RootLoopCompletion::NoPrimalSuggestion);
  }
  std::mt19937 random(419);
  for(unsigned example=0;example<180;++example) {
    const std::size_t n=2+random()%2,m=1+random()%3;LP::LinearModel dense;
    for(std::size_t k=0;k<n*m;++k)dense.a.push_back(I(random()%7)-3);
    for(std::size_t i=0;i<m;++i)dense.b.push_back(I(random()%9)-5);
    for(std::size_t j=0;j<n;++j)dense.c.push_back(I(random()%7)-3);
    std::vector<I> lo(n,0),hi(n,1);
    if(example%5==0)lo[0]=hi[0]=-2;
    Cuts::SourceModel source(model(dense,lo,hi));
    std::optional<I> prior;
    for(std::size_t rounds=1;rounds<=3;++rounds) {
      Cuts::RootLoopOptions options;options.max_rounds=rounds;
      const auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
      if(result.best_bound) {
        if(prior)assert(result.best_bound->bound.lower_bound>=*prior);
        prior=result.best_bound->bound.lower_bound;
      }
    }
  }
}
static void concurrency() {
  Cuts::SourceModel source(fixture());Cuts::RootLoopOptions options;
  std::optional<Cuts::RootLoopResult> a,b;
  std::thread one([&](){a=Cuts::root_cover_loop(source,options);});
  std::thread two([&](){b=Cuts::root_cover_loop(source,options);});
  one.join();two.join();evidence(source,options,*a);evidence(source,options,*b);
  assert(a->best_bound->bound.lower_bound==-2 && b->best_bound->bound.lower_bound==-2);
  assert(!a->model.same_identity(b->model) && a->cuts[0].source().same_identity(b->cuts[0].source()));
  auto backend=a->backend;bool first=true,second=true;
  std::thread low([&](){for(unsigned i=0;i<10;++i){const auto r=backend->bound({0,0},{0,1},true,true);first&=r.valid && r.lower_bound==-2;}});
  std::thread high([&](){for(unsigned i=0;i<10;++i){const auto r=backend->bound({1,0},{1,0},true,true);second&=r.valid && r.lower_bound==-2;}});
  low.join();high.join();assert(first && second);
}
#ifdef GECODE_LP_CUT_LOOP_TEST_HOOKS
static void faults() {
  Cuts::SourceModel source(fixture());Cuts::RootLoopOptions options;
  events.clear();(void)Cuts::root_cover_loop(source,options);const auto counts=events;
  for(const auto& event:counts)for(unsigned occurrence=1;occurrence<=event.second;++occurrence)
    for(unsigned mode=0;mode<3;++mode) {
      failed_event=event.first;failed_occurrence=occurrence;event_occurrence=0;allocation_failure=mode==1;
      cancel_on_event=mode==2;cancelled_on_event=false;
      options.stop_requested=[](){return cancelled_on_event;};
      auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
      assert(event_occurrence==occurrence);
      assert(result.completion==(mode==2?Cuts::RootLoopCompletion::Cancelled:
        mode==1?Cuts::RootLoopCompletion::AllocationFailure:Cuts::RootLoopCompletion::BackendError));
      if(event.first=="augmentation_publication")assert(result.cuts.empty() && result.best_bound->bound.lower_bound==-3);
      if(event.first=="after_augmentation_publication")assert(result.cuts.size()==1 && result.best_bound->bound.lower_bound==-3);
      if(event.first=="bound_publication" && occurrence==2)assert(result.cuts.size()==1 && result.best_bound->bound.lower_bound==-3);
    }
  // Explicit last-round interruption: retain its already-published augmentation.
  failed_event="after_augmentation_publication";failed_occurrence=1;event_occurrence=0;
  cancel_on_event=true;cancelled_on_event=false;options.max_rounds=1;
  const auto last=Cuts::root_cover_loop(source,options);evidence(source,options,last);
  assert(last.completion==Cuts::RootLoopCompletion::Cancelled && last.cuts.size()==1);
  failed_event.clear();cancel_on_event=false;cancelled_on_event=false;
}
#endif
#ifdef GECODE_LP_CUT_LOOP_TEST_NATIVE
class NativeModel final:public Gecode::Space {
public:
  Gecode::IntVarArray x;
  Gecode::IntVar cost;
  explicit NativeModel(const Model& input):x(*this,static_cast<int>(input.lower.size())),
      cost(*this,Gecode::Int::Limits::min,Gecode::Int::Limits::max) {
    for(int j=0;j<x.size();++j)x[j]=Gecode::IntVar(*this,static_cast<int>(input.lower[j]),static_cast<int>(input.upper[j]));
    LP::post_native_integer(*this,x,cost,input);
    Gecode::branch(*this,x,Gecode::INT_VAR_SIZE_MIN(),Gecode::INT_VAL_MIN());
  }
  NativeModel(NativeModel& other):Gecode::Space(other){x.update(*this,other.x);cost.update(*this,other.cost);}
  Gecode::Space* copy() override{return new NativeModel(*this);}
};
static void native_consumption() {
  for(const auto& input:{fixture(),model({{2,-3,-3},{-7},{0,-2,-2}},{-1,0,0},{-1,1,1}),
                        model({{},{1},{}},{},{})}) {
    Cuts::SourceModel source(input);Cuts::RootLoopOptions options;
    auto result=Cuts::root_cover_loop(source,options);evidence(source,options,result);
    auto root=std::make_unique<NativeModel>(result.model.model());
    Gecode::DFS<NativeModel> search(root.get());root.reset();
    std::size_t count=0;
    while(auto* raw=search.next()) {
      std::unique_ptr<NativeModel> solution(raw);std::vector<I> x;
      for(int j=0;j<solution->x.size();++j)x.push_back(solution->x[j].val());
      assert(feasible(input,x) && solution->cost.val()==objective(input,x));++count;
    }
    std::size_t expected=0;std::vector<I> x(input.lower.size());
    const auto visit=[&](const auto& self,std::size_t j)->void {
      if(j==x.size()){expected+=feasible(input,x);return;}
      for(I value=input.lower[j];value<=input.upper[j];++value){x[j]=value;self(self,j+1);}
    };
    visit(visit,0);assert(count==expected);
  }
}
#endif
int main() {
  actual_lp_and_ownership();rationalization();budgets();edge_models();
#ifdef GECODE_LP_CUT_LOOP_TEST_HOOKS
  faults(); // Hook instrumentation is coordinator-only; concurrent runs follow.
#endif
  concurrency();
#ifdef GECODE_LP_CUT_LOOP_TEST_NATIVE
  native_consumption();
#endif
  std::cout<<"PASS root cover loop: "<<runs<<" configurations, "<<points
    <<" exact feasible-set checks; LP strengthening, budgets, immutable provenance and concurrency\n";
}
