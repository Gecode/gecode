// Certified conditional bounds and throttled LP propagation.
// Feasibility and objective oracles enumerate assignments independently.
#include <gecode/minimodel/lp-relaxation.hpp>
#include <gecode/search.hh>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace LP=Gecode::Experimental::LpRelaxation;
namespace Cert=Gecode::Experimental::LpCertificate;
using I=std::int64_t;
using V=std::vector<I>;
static void require(bool condition,const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct Instance { LP::LinearModel model; V lower,upper; };
static I cost(const Instance& p,unsigned mask) {
  I value=0;
  for (std::size_t j=0;j<p.model.c.size();++j) value+=p.model.c[j]*((mask>>j)&1U);
  return value;
}
static std::set<unsigned> oracle(const Instance& p,I cutoff) {
  std::set<unsigned> result;
  const auto n=p.model.c.size();
  for (unsigned mask=0;mask<(1U<<n);++mask) {
    bool feasible=cost(p,mask)<=cutoff;
    for (std::size_t j=0;j<n;++j) {
      const I value=(mask>>j)&1U;
      feasible&=p.lower[j]<=value && value<=p.upper[j];
    }
    for (std::size_t i=0;feasible && i<p.model.b.size();++i) {
      I sum=0;
      for (std::size_t j=0;j<n;++j) sum+=p.model.a[i*n+j]*((mask>>j)&1U);
      feasible=sum>=p.model.b[i];
    }
    if (feasible) result.insert(mask);
  }
  return result;
}
static int endpoint(const LP::LinearModel& p,bool upper) {
  I result=0;
  for (I c:p.c) result+=upper ? std::max<I>(0,c) : std::min<I>(0,c);
  return static_cast<int>(result);
}
static Instance random_instance(std::mt19937& random,unsigned id) {
  Instance p;
  const std::size_t n=2+random()%7,m=random()%7;
  p.model.c.resize(n);p.model.b.resize(m);p.model.a.resize(n*m);
  p.lower.resize(n);p.upper.resize(n);
  V witness(n);
  for (std::size_t j=0;j<n;++j) {
    const unsigned domain=random()%10;
    p.lower[j]=domain==0 ? 1:0;p.upper[j]=domain==1 ? 0:1;
    witness[j]=p.lower[j]==p.upper[j] ? p.lower[j]:random()%2;
    p.model.c[j]=static_cast<int>(random()%21)-10;
  }
  for (std::size_t i=0;i<m;++i) {
    I activity=0;
    for (std::size_t j=0;j<n;++j) {
      p.model.a[i*n+j]=static_cast<int>(random()%13)-6;
      activity+=p.model.a[i*n+j]*witness[j];
    }
    p.model.b[i]=id%2 ? static_cast<int>(random()%25)-12 : activity-random()%5;
  }
  return p;
}

static void check_certificate() {
  Cert::Certificate empty;
  I bound=999;
  std::vector<unsigned char> forbidden={7};
  require(!empty.filter({0},{1},0,bound,forbidden) && bound==999 && forbidden[0]==7,
          "uninitialized certificate changed outputs");
  require(Cert::prepare({-1},{-1},{-2},{0.5},empty),"signed preparation");
  require(empty.filter({0},{1},-1,bound,forbidden) && bound==-2 && forbidden[0]==1,
          "signed conditional ceiling");
  require(!empty.filter({-1},{1},-1,bound,forbidden) && bound==-2 && forbidden[0]==1,
          "invalid binary box changed outputs");
  require(!Cert::prepare({1},{1},{1},{std::numeric_limits<double>::infinity()},empty),
          "nonfinite multiplier accepted");
  require(empty.lower_bound({0},{1},bound) && bound==-2,"failed prepare replaced certificate");
  V huge(32,std::numeric_limits<I>::max());
  require(!Cert::prepare(huge,huge,{1},std::vector<double>(32,1e12),empty),
          "128-bit overflow accepted");
  require(!Cert::prepare({1,2},{1},{1},{1},empty),"bad dense dimensions accepted");

  std::mt19937 random(987651);
  for (unsigned id=0;id<1000;++id) {
    const Instance p=random_instance(random,id);
    std::vector<double> duals(p.model.b.size());
    const double candidates[]={-2,-0.5,0,0.1,0.5,0.99999999,1,1.5,3};
    for (double& d:duals) d=candidates[random()%9];
    Cert::Certificate prepared;
    require(Cert::prepare(p.model.a,p.model.b,p.model.c,duals,prepared),"tiny prepare failed");
    I old_bound,new_bound;
    require(Cert::lower_bound(p.model.a,p.model.b,p.model.c,p.lower,p.upper,duals,old_bound) &&
            prepared.lower_bound(p.lower,p.upper,new_bound) && old_bound==new_bound,
            "prepared bound differs from legacy certificate");
    for (I cutoff:{I(-7),I(0),I(9)}) {
      require(prepared.filter(p.lower,p.upper,cutoff,new_bound,forbidden),"tiny filter failed");
      require(new_bound==old_bound,"filter changed ordinary certificate");
      for (unsigned mask:oracle(p,cutoff)) {
        require(new_bound<=cost(p,mask),"bound exceeds a feasible assignment cost");
        for (std::size_t j=0;j<p.model.c.size();++j)
          require(!(forbidden[j]&(1U<<((mask>>j)&1U))),"conditional bound removed feasible bit");
      }
      // Compare every conditional result to the independently retained old
      // checker evaluated with an explicit fixed box, including negative costs.
      for (std::size_t j=0;j<p.model.c.size();++j) {
        if (p.lower[j]==p.upper[j]) {require(forbidden[j]==0,"assigned bit filtered");continue;}
        for (I value:{I(0),I(1)}) {
          V l=p.lower,u=p.upper;l[j]=u[j]=value;
          I forced;
          require(Cert::lower_bound(p.model.a,p.model.b,p.model.c,l,u,duals,forced),"forced bound failed");
          require(bool(forbidden[j]&(1U<<value))==(forced>cutoff),"conditional algebra mismatch");
        }
      }
    }
  }
}

class Problem : public Gecode::Space {
public:
  Gecode::IntVarArray x;
  Gecode::IntVar z;
  Problem(const Instance& p,I cutoff,const std::shared_ptr<LP::Backend>& backend,
          const LP::Options* options)
      :x(*this,static_cast<int>(p.model.c.size()),0,1),
       z(*this,endpoint(p.model,false),endpoint(p.model,true)) {
    for(int j=0;j<x.size();++j)
      Gecode::dom(*this,x[j],static_cast<int>(p.lower[j]),static_cast<int>(p.upper[j]));
    if(options) LP::binary_linear_minimize(*this,x,z,backend,*options);
    else LP::post_native(*this,x,z,p.model);
    Gecode::rel(*this,z,Gecode::IRT_LQ,static_cast<int>(cutoff));
    Gecode::branch(*this,x,Gecode::INT_VAR_NONE(),Gecode::INT_VAL_MIN());
  }
  Problem(Problem& other):Gecode::Space(other) {x.update(*this,other.x);z.update(*this,other.z);}
  Gecode::Space* copy() override {return new Problem(*this);}
  void constrain(const Gecode::Space& best) override {
    Gecode::rel(*this,z,Gecode::IRT_LE,static_cast<const Problem&>(best).z.val());
  }
  unsigned mask() const {
    unsigned result=0;
    for(int j=0;j<x.size();++j) {require(x[j].assigned(),"returned unassigned bit");result|=unsigned(x[j].val())<<j;}
    return result;
  }
};

static Instance triangle(bool negative=false) {
  Instance p{{{1,1,0,0, 0,1,1,0, 1,0,1,0},{1,1,1},{1,1,1,2}},
             {0,0,0,0},{1,1,1,1}};
  if(negative) {
    for(I& a:p.model.a) a=-a;
    for(I& b:p.model.b) b=-b;
    for(I& c:p.model.c) c=-c;
  }
  return p;
}

static void check_deductions_and_schedule() {
  for(bool negative:{false,true}) {
    const Instance p=triangle(negative);
    auto backend=std::make_shared<LP::Backend>(p.model);
    LP::Options bounds,fixing;fixing.reduced_cost_fixing=true;
    const I cutoff=negative ? -3:2;
    {
      Problem baseline(p,cutoff,backend,&bounds);
      require(baseline.status()!=Gecode::SS_FAILED && !baseline.x[3].assigned(),
              "structured fixing already performed by bound-only baseline");
      Problem improved(p,cutoff,backend,&fixing);
      require(improved.status()!=Gecode::SS_FAILED && improved.x[3].assigned() &&
              improved.x[3].val()==(negative ? 1:0),"conditional bound missed structured fixing");
    }
    require(backend->statistics().variable_fixings>0,"fixing statistics not updated");
    require(backend.use_count()==1,"actor leaked backend after fixing");
  }
  {
    const Instance p=triangle();
    auto backend=std::make_shared<LP::Backend>(p.model);
    LP::Options options;options.frequency=LP::Frequency::Root;options.reduced_cost_fixing=true;
    Problem root(p,5,backend,&options);
    require(root.status()!=Gecode::SS_FAILED && !root.x[3].assigned(),"uncut root changed");
    const auto calls=backend->statistics().lp_calls;
    std::unique_ptr<Problem> left(static_cast<Problem*>(root.clone()));
    std::unique_ptr<Problem> right(static_cast<Problem*>(root.clone()));
    Gecode::rel(*left,left->z,Gecode::IRT_LQ,2);
    require(left->status()!=Gecode::SS_FAILED && left->x[3].val()==0,
            "objective-only event did not reuse root certificate");
    Gecode::rel(*right,right->x[3],Gecode::IRT_EQ,1);
    Gecode::rel(*right,right->z,Gecode::IRT_LQ,4);
    require(right->status()!=Gecode::SS_FAILED && right->z.min()==4,
            "sibling root certificate used another box's scalar bound");
    require(backend->statistics().lp_calls==calls,"root-only fixing performed another LP solve");
  }
  {
    // Keep a nonzero redundant row so this fixture exercises LP scheduling.
    // A zero matrix correctly uses the checked box bound without an LP call.
    Instance p{{V(6,1),{0},V(6,0)},V(6,0),V(6,1)};
    auto backend=std::make_shared<LP::Backend>(p.model);
    LP::Options options;options.reduced_cost_fixing=true;options.assignment_interval=4;
    Problem root(p,0,backend,&options);
    require(root.status()!=Gecode::SS_FAILED,"throttling root failed");
    const auto initial=backend->statistics().lp_calls;
    require(initial==1,"root LP missing");
    for(int j=0;j<4;++j) {
      Gecode::rel(root,root.x[j],Gecode::IRT_EQ,0);
      require(root.status()!=Gecode::SS_FAILED,"throttled descendant failed");
      require(backend->statistics().lp_calls==initial+(j==3 ? 1:0),"assignment interval not respected");
    }
    bool rejected=false;options.assignment_interval=0;
    try {Problem bad(p,0,backend,&options);} catch(const std::invalid_argument&) {rejected=true;}
    require(rejected,"zero assignment interval accepted");
  }
}

static void check_search() {
  std::vector<Instance> cases={triangle(),triangle(true),{{{}, {}, {}},{},{}}};
  std::mt19937 random(928371);
  while(cases.size()<100) cases.push_back(random_instance(random,cases.size()));
  const std::vector<LP::Options> policies={
    {LP::Frequency::EveryNode,false,1}, {LP::Frequency::EveryNode,true,1},
    {LP::Frequency::EveryNode,true,4}, {LP::Frequency::Root,true,1},
    {LP::Frequency::EveryNode,false,4}};
  unsigned runs=0;
  for(const Instance& p:cases) {
    auto backend=std::make_shared<LP::Backend>(p.model);
    const int lo=endpoint(p.model,false),hi=endpoint(p.model,true);
    for(I cutoff:{I(hi),I(lo+(hi-lo)/2),I(lo)}) {
      const auto expected=oracle(p,cutoff);
      I optimum=std::numeric_limits<I>::max();
      for(unsigned mask:expected) optimum=std::min(optimum,cost(p,mask));
      for(unsigned distance:{1U,8U,32U})
        for(std::size_t mode=0;mode<=policies.size();++mode) {
          const LP::Options* policy=mode ? &policies[mode-1]:nullptr;
          Gecode::Search::Options options;options.threads=1;options.c_d=distance;options.a_d=16;
          {
            auto root=std::make_unique<Problem>(p,cutoff,backend,policy);
            Gecode::DFS<Problem> engine(root.get(),options);root.reset();
            std::set<unsigned> actual;
            while(Problem* raw=engine.next()) {
              std::unique_ptr<Problem> solution(raw);
              const auto mask=solution->mask();
              require(expected.count(mask) && solution->z.val()==cost(p,mask),"DFS solution invalid");
              require(actual.insert(mask).second,"duplicate DFS solution");
            }
            require(!engine.stopped() && actual==expected,"conditional propagation changed full solution set");
          }
          {
            auto root=std::make_unique<Problem>(p,cutoff,backend,policy);
            Gecode::BAB<Problem> engine(root.get(),options);root.reset();
            I best=std::numeric_limits<I>::max();
            while(Problem* raw=engine.next()) {
              std::unique_ptr<Problem> solution(raw);
              const auto mask=solution->mask();
              require(expected.count(mask) && solution->z.val()==cost(p,mask) && cost(p,mask)<best,
                      "BAB solution invalid");
              best=cost(p,mask);
            }
            require(!engine.stopped() && best==optimum,"conditional propagation changed optimum");
          }
          require(backend.use_count()==1,"cloned actor leaked backend");
          runs+=2;
        }
    }
  }
  std::cout<<"PASS reduced-cost certificates: 1000 randomized algebra/oracle cases; "
           <<cases.size()<<" models and "<<runs<<" exhaustive cutoff/DFS/BAB/policy/recompute configurations; "
           <<"positive/negative fixing, objective-only reuse, sibling restoration, exact interval\n";
}

int main() {
  try {check_certificate();check_deductions_and_schedule();check_search();return 0;}
  catch(const std::exception& error) {std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
