// Complete original integer solution sets and optima against independent enumeration.
#include <gecode/minimodel/lp-relaxation.hpp>
#include <gecode/search.hh>
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <type_traits>

namespace LP=Gecode::Experimental::LpRelaxation;
using I=std::int64_t;
using Assignment=std::vector<int>;
static void check(bool okay,const char* message) {if(!okay)throw std::runtime_error(message);}
struct Instance {
  LP::BoundedIntegerModel model;
  std::vector<Assignment> domains;
};
enum class Mode {Native,Root,Node,RootTight,NodeTight,Throttled};
static I value(const LP::BoundedIntegerModel& model,const Assignment& x) {
  I result=0;for(std::size_t j=0;j<x.size();++j)result+=model.linear.c[j]*x[j];return result;
}
static std::set<Assignment> enumerate(const Instance& instance) {
  const auto& m=instance.model.linear;Assignment current(m.c.size());std::set<Assignment> result;
  const auto visit=[&](auto&& self,std::size_t j)->void {
    if(j<current.size()){for(int v:instance.domains[j]){current[j]=v;self(self,j+1);}return;}
    for(std::size_t k=0;k<current.size();++k)
      if(current[k]<instance.model.lower[k] || current[k]>instance.model.upper[k])return;
    for(std::size_t i=0;i<m.b.size();++i) {
      I sum=0;for(auto k=m.row_start[i];k<m.row_start[i+1];++k)sum+=m.a[k]*current[m.column[k]];
      if(sum<m.b[i])return;
    }
    result.insert(current);
  };visit(visit,0);return result;
}
class Problem : public Gecode::Space {
public:
  Gecode::IntVarArray x;Gecode::IntVar z;
  Problem(const Instance& instance,Mode mode,const std::shared_ptr<LP::BoundedIntegerBackend>& backend,
          unsigned interval=4)
    : x(*this,static_cast<int>(instance.model.linear.c.size())),z(*this,Gecode::Int::Limits::min,Gecode::Int::Limits::max) {
    for(int j=0;j<x.size();++j) {
      const auto& domain=instance.domains[j];
      x[j]=Gecode::IntVar(*this,Gecode::IntSet(domain.data(),static_cast<int>(domain.size())));
    }
    if(mode==Mode::Native) LP::post_native_integer(*this,x,z,instance.model);
    else {
      LP::IntegerOptions options;
      options.frequency=mode==Mode::Root || mode==Mode::RootTight ? LP::Frequency::Root : LP::Frequency::EveryNode;
      options.bound_tightening=mode==Mode::RootTight || mode==Mode::NodeTight || mode==Mode::Throttled;
      options.bound_change_interval=mode==Mode::Throttled?interval:1;
      LP::integer_linear_minimize(*this,x,z,backend,options);
    }
    Gecode::branch(*this,x,Gecode::INT_VAR_SIZE_MIN(),Gecode::INT_VAL_MIN());
  }
  Problem(Problem& other):Gecode::Space(other){x.update(*this,other.x);z.update(*this,other.z);}
  Gecode::Space* copy() override{return new Problem(*this);}
  void constrain(const Gecode::Space& best) override {
    Gecode::rel(*this,z,Gecode::IRT_LE,static_cast<const Problem&>(best).z.val());
  }
  Assignment assignment() const {
    Assignment result;for(int j=0;j<x.size();++j){check(x[j].assigned(),"unassigned integer witness");result.push_back(x[j].val());}
    return result;
  }
};
static void searches(const Instance& instance,Mode mode,const std::shared_ptr<LP::BoundedIntegerBackend>& backend,
                      unsigned distance,const std::set<Assignment>& expected) {
  Gecode::Search::Options options;options.threads=1;options.c_d=distance;options.a_d=3;
  {
    auto root=std::make_unique<Problem>(instance,mode,backend);
    Gecode::DFS<Problem> search(root.get(),options);root.reset();std::set<Assignment> actual;
    while(std::unique_ptr<Problem> found{search.next()}) {
      auto assignment=found->assignment();
      check(expected.count(assignment)==1 && actual.insert(assignment).second,"integer DFS infeasible/duplicate witness");
      check(found->z.assigned() && found->z.val()==value(instance.model,assignment),"integer DFS objective mismatch");
    }
    check(!search.stopped() && actual==expected,"integer propagation changed complete solution set");
  }
  check(backend.use_count()==1,"integer DFS backend leak");
  {
    auto root=std::make_unique<Problem>(instance,mode,backend);
    Gecode::BAB<Problem> search(root.get(),options);root.reset();I best=std::numeric_limits<I>::max();
    while(std::unique_ptr<Problem> found{search.next()}) {
      const auto assignment=found->assignment();const I cost=value(instance.model,assignment);
      check(expected.count(assignment)==1 && cost<best && found->z.val()==cost,"integer BAB witness/objective mismatch");
      best=cost;
    }
    I optimum=std::numeric_limits<I>::max();for(const auto& assignment:expected)optimum=std::min(optimum,value(instance.model,assignment));
    check(!search.stopped() && best==optimum,"integer BAB optimum mismatch");
  }
  check(backend.use_count()==1,"integer BAB backend leak");
}
static Instance instance(LP::LinearModel dense,std::vector<I> lower,std::vector<I> upper) {
  Instance result{{LP::sparse_model(dense),std::move(lower),std::move(upper)},{}};
  for(std::size_t j=0;j<result.model.lower.size();++j) {
    Assignment domain;for(I v=result.model.lower[j];v<=result.model.upper[j];++v)domain.push_back(static_cast<int>(v));
    result.domains.push_back(std::move(domain));
  }
  return result;
}
static std::vector<Instance> cases() {
  std::vector<Instance> result;
  result.push_back(instance({{1,1,0,0,1,1,1,0,1},{3,3,3},{1,1,1}},{0,0,0},{3,3,3}));
  result.push_back(instance({{-1,-1,0,0,-1,-1,-1,0,-1},{1,1,1},{-1,-1,-1}},{-3,-3,-3},{1,1,1}));
  result.push_back(instance({{1,-1,2,-2},{0,1},{-2,3}},{-2,-2},{2,2}));
  result.push_back(instance({{0,0},{1},{-1,2}},{-2,1},{2,3}));
  result.push_back(instance({{},{},{-3,2}},{-2,-2},{2,3}));
  result.push_back(instance({{},{},{}},{},{}));
  result.push_back(instance({{1,-1},{0},{-1,2}},{-2,-3},{-2,-3}));
  auto holes=instance({{1,-1},{0},{-3,2}},{-3,-3},{3,3});holes.domains={{-3,0,3},{-2,2}};result.push_back(holes);
  result.push_back(instance({{1000000000,-1000000000},{1000000000},{0,0}},{-1,-1},{1,1}));
  const I maximum=Gecode::Int::Limits::max;
  result.push_back({{LP::sparse_model({{-1},{1000000000},{0}}),{-maximum},{maximum}},
                    {{static_cast<int>(-maximum),0,static_cast<int>(maximum)}}});
  result.push_back({{LP::sparse_model({{},{},{1}}),{-maximum/2},{maximum/2}},
                    {{static_cast<int>(-maximum/2),0,static_cast<int>(maximum/2)}}});
  auto intersected=instance({{1,1},{0},{-1,2}},{-2,-1},{2,3});
  intersected.domains={{-4,-1,0,1,4},{-2,0,2,5}};result.push_back(intersected);
  auto empty_intersection=instance({{},{},{1}},{0},{1});
  empty_intersection.domains={{-2,-1}};result.push_back(empty_intersection);
  std::mt19937 random(113955);
  while(result.size()<80) {
    const auto n=1+random()%4,m=random()%5;
    LP::LinearModel dense;dense.a.resize(n*m);dense.b.resize(m);dense.c.resize(n);
    std::vector<I> lo(n),hi(n),witness(n);
    for(std::size_t j=0;j<n;++j) {
      lo[j]=static_cast<int>(random()%5)-3;hi[j]=lo[j]+random()%4;
      witness[j]=lo[j]+random()%(hi[j]-lo[j]+1);dense.c[j]=static_cast<int>(random()%9)-4;
    }
    for(std::size_t i=0;i<m;++i) {
      I activity=0;for(std::size_t j=0;j<n;++j){dense.a[i*n+j]=random()%3?0:static_cast<int>(random()%9)-4;activity+=dense.a[i*n+j]*witness[j];}
      dense.b[i]=result.size()%2?static_cast<int>(random()%15)-7:activity-static_cast<int>(random()%4);
    }
    auto model=instance(dense,lo,hi);
    for(auto& domain:model.domains) if(domain.size()>2 && random()%3==0)domain.erase(domain.begin()+1);
    result.push_back(std::move(model));
  }
  return result;
}
static std::uint64_t policies() {
  // The three covering rows jointly imply a fractional cost of 4.5.
  // Native row-by-row propagation cannot derive the fourth variable's cut.
  const auto cutting=instance({{1,1,0,0,0,1,1,0,1,0,1,0},{3,3,3},{1,1,1,2}},
                              {0,0,0,-1},{3,3,3,3});
  auto cut_backend=std::make_shared<LP::BoundedIntegerBackend>(cutting.model);
  auto cut=std::make_unique<Problem>(cutting,Mode::RootTight,cut_backend);
  Gecode::rel(*cut,cut->z,Gecode::IRT_LQ,6);
  check(cut->status()==Gecode::SS_BRANCH && cut->x[3].min()==-1 && cut->x[3].max()==0,
        "integer certificate failed a strict nonbinary interval cut");
  check(cut_backend->statistics().variable_bound_tightenings>0,"integer interval tightening not recorded");
  {
    auto expected=enumerate(cutting);
    for(auto it=expected.begin();it!=expected.end();)
      if(value(cutting.model,*it)>6)it=expected.erase(it);else ++it;
    Gecode::DFS<Problem> search(cut.get());cut.reset();std::set<Assignment> actual;
    while(std::unique_ptr<Problem> found{search.next()})actual.insert(found->assignment());
    check(!search.stopped() && actual==expected,"integer interval cut removed a cutoff-feasible assignment");
  }
  check(cut_backend.use_count()==1,"integer cutting policy lifecycle");
  const auto plain=instance({{},{},{1,2,3}},{-4,-4,-4},{4,4,4});
  for(unsigned interval:{1U,2U}) {
    auto backend=std::make_shared<LP::BoundedIntegerBackend>(plain.model);
    auto root=std::make_unique<Problem>(plain,Mode::Throttled,backend,interval);
    check(root->status()==Gecode::SS_BRANCH && backend->statistics().valid_bounds==1,"integer root scheduling");
    Gecode::rel(*root,root->x[0],Gecode::IRT_GQ,-2);
    check(root->status()==Gecode::SS_BRANCH && !root->x[0].assigned(),"nonassignment bound change");
    check(backend->statistics().valid_bounds==(interval==1?2U:1U),"integer scheduling counted assignments instead of bounds");
    Gecode::rel(*root,root->x[1],Gecode::IRT_GQ,-2);
    check(root->status()==Gecode::SS_BRANCH && backend->statistics().valid_bounds==(interval==1?3U:2U),"integer bound interval throttling");
    const auto calls=backend->statistics().valid_bounds;
    Gecode::rel(*root,root->z,Gecode::IRT_LQ,23);
    check(root->status()==Gecode::SS_BRANCH && backend->statistics().valid_bounds==calls,"objective-only change reoptimized LP");
    auto clone=std::unique_ptr<Problem>(static_cast<Problem*>(root->clone()));
    check(clone->status()==Gecode::SS_BRANCH,"integer cloned bound policy");
    clone.reset();root.reset();check(backend.use_count()==1,"integer policy lifecycle");
  }
  return cut_backend->statistics().variable_bound_tightenings;
}
int main() {
 try {
  static_assert(!std::is_base_of<LP::SparseBackend,LP::BoundedIntegerBackend>::value,"integer backend must not enter binary API by upcast");
  const auto models=cases();unsigned runs=0,feasible=0;std::uint64_t bounds=0,tightened=0;
  for(const auto& model:models) {
    const auto expected=enumerate(model);feasible+=!expected.empty();
    auto backend=std::make_shared<LP::BoundedIntegerBackend>(model.model);
    std::weak_ptr<LP::BoundedIntegerBackend> weak=backend;
    for(auto mode:{Mode::Native,Mode::Root,Mode::Node,Mode::RootTight,Mode::NodeTight,Mode::Throttled})
      for(unsigned distance:{1U,8U}){searches(model,mode,backend,distance,expected);runs+=2;}
    bounds+=backend->statistics().valid_bounds;tightened+=backend->statistics().variable_bound_tightenings;
    backend.reset();check(weak.expired(),"integer backend outlived searches");
  }
  tightened+=policies();check(feasible>0 && feasible<models.size() && bounds>0 && tightened>0,"integer integration coverage missing");
  std::cout<<"PASS "<<models.size()<<" integer models, "<<runs<<" DFS/BAB oracle configurations; "<<bounds<<" certificates, "<<tightened<<" interval tightenings; holes, clones, policy and lifecycle checks\n";
 } catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
