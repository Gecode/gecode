/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
// Standalone sequential CADDS study; no production engine or Model dependency.
#include "examples/tsptw-transition.hh"
#include <gecode/search/seq/choice-expansion.hh>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

using namespace Gecode;
using Owned = std::unique_ptr<Space>;
using Clock = std::chrono::steady_clock;

// Constant-time counts include seeds, pending/active/in-flight states,
// incumbents, and publications owned by the caller. These are not byte counts.
class State : public TSPTW::Transition {
public:
  static inline unsigned long live=0, peak=0, clones=0;
  void created() { peak=std::max(peak,++live); }
  State(std::shared_ptr<const TSPTW::Instance> data,
        std::shared_ptr<TSPTW::Measurements> measurements)
    : Transition(std::move(data),LAZY,OFF,std::move(measurements)) { created(); }
  State(State& s) : Transition(s) { clones++; created(); }
  Space* copy() override { return new State(*this); }
  ~State() override { --live; }
};

struct Step {
  enum Kind { SOLUTION, SUSPENDED, EXHAUSTED } kind;
  Owned solution;
};
struct Entry {
  Owned space;
  std::int64_t bound, rank;
  unsigned long long tie, revision;
};
struct Fragment {
  unsigned depth;
  std::vector<Entry> sources, staged;
  size_t source=0;
  unsigned page_survivors=0;
  unsigned long long source_revision=0;
  // Destroy the Choice continuation before the sources.
  Search::Seq::ChoiceExpansion expansion;
  explicit Fragment(unsigned d) : depth(d) {}
};

// advance: finish the active continuation, classify each child, finish the
// frozen fragment, install its successors, then select the next deepest layer.
// The cursor control instead descends after each page of B surviving children.
class Study {
  bool cadds, optimize;
  unsigned width;
  Search::HeuristicGuidance guidance;
  Search::Options options;
  Search::Worker worker;
  Owned incumbent, root_goal;
  std::int64_t incumbent_bound=0;
  unsigned long long revision=0, tie=0;
  std::vector<std::vector<Entry>> layers;
  std::unique_ptr<Fragment> active;
  std::vector<std::unique_ptr<Fragment>> stack;
  unsigned long pending=0, staged=0, sources=0;

  Entry entry(Owned s, std::int64_t bound) {
    const auto rank=guidance.rank ? guidance.rank(*s) : bound;
    return {std::move(s),bound,rank,tie++,revision};
  }
  void sample() {
    peak_pending=std::max(peak_pending,pending);
    peak_staged=std::max(peak_staged,staged);
    peak_sources=std::max(peak_sources,sources);
  }
  Owned publish(Owned s) {
    if (optimize) {
      const auto value=guidance.bound(*s);
      if (incumbent && value>=incumbent_bound) return {};
      incumbent.reset(s->clone());
      incumbent_bound=value;
      revision++;
    }
    return s;
  }
  bool select_fragment() {
    size_t d=layers.size();
    while (d && layers[d-1].empty()) --d;
    if (!d) return false;
    --d;
    auto& layer=layers[d];
    const size_t count=std::min<size_t>(width,layer.size());
    std::partial_sort(layer.begin(),layer.begin()+count,layer.end(),
      [](const Entry& a,const Entry& b) {
        return a.rank<b.rank || (a.rank==b.rank && a.tie<b.tie);
      });
    active=std::make_unique<Fragment>(static_cast<unsigned>(d));
    for (size_t k=0;k<count;k++) active->sources.push_back(std::move(layer[k]));
    layer.erase(layer.begin(),layer.begin()+count);
    pending-=count; sources+=count; fragments++; sample();
    return true;
  }
  void finish_fragment() {
    const unsigned d=active->depth+1;
    if (layers.size()<=d) layers.resize(d+1);
    // The deepest-layer rule makes this layer empty at fragment start.
    if (!layers[d].empty()) throw std::runtime_error("nonempty successor layer");
    pending+=active->staged.size(); staged-=active->staged.size();
    layers[d]=std::move(active->staged);
    active.reset(); sample();
  }
  void descend_page(Fragment& f) {
    auto child=std::make_unique<Fragment>(f.depth+1);
    staged-=f.staged.size(); sources+=f.staged.size();
    child->sources=std::move(f.staged); f.page_survivors=0;
    stack.push_back(std::move(child)); fragments++; sample();
  }
public:
  unsigned long commits=0, pruned=0, suspensions=0, fragments=0;
  unsigned long peak_pending=0, peak_staged=0, peak_sources=0;

  Study(Owned root, bool full_fragment, bool optimization, unsigned b,
        Search::HeuristicGuidance g, Search::Options opt={})
    : cadds(full_fragment),optimize(optimization),width(b),
      guidance(std::move(g)),options(opt) {
    if (!width) throw std::runtime_error("zero width");
    if (guidance.sense!=Search::HeuristicGuidance::MINIMIZE)
      throw std::runtime_error("study supports minimization");
    worker.node++;
    const auto status=root->status(worker);
    if (status==SS_FAILED) { worker.fail++; return; }
    if (status==SS_SOLVED) { root_goal=std::move(root); return; }
    const auto bound=guidance.bound(*root);
    if (cadds) {
      layers.resize(1); layers[0].push_back(entry(std::move(root),bound)); pending=1;
    } else {
      auto f=std::make_unique<Fragment>(0);
      f->sources.push_back(entry(std::move(root),bound)); sources=1;
      stack.push_back(std::move(f));
    }
    sample();
  }
  Search::Statistics statistics() const { return worker; }
  bool done() const {
    return !root_goal && !active && stack.empty() && pending==0;
  }
  Step advance(unsigned commit_quota=std::numeric_limits<unsigned>::max()) {
    worker.start();
    if (root_goal) return {Step::SOLUTION,publish(std::move(root_goal))};
    unsigned work=0;
    for (;;) {
      if (cadds) {
        if (!active && !select_fragment()) return {Step::EXHAUSTED,{}};
      } else if (stack.empty()) return {Step::EXHAUSTED,{}};
      Fragment& f=cadds ? *active : *stack.back();
      // A published goal can fill the cursor page without a branching child.
      if (!cadds && f.page_survivors==width) {
        if (!f.staged.empty()) descend_page(f); else f.page_survivors=0;
        continue;
      }
      if (!f.expansion) {
        while (f.source<f.sources.size() && !f.sources[f.source].space) f.source++;
        if (f.source==f.sources.size()) {
          if (cadds) finish_fragment();
          else if (!f.staged.empty()) descend_page(f);
          else stack.pop_back();
          continue;
        }
        Entry& selected=f.sources[f.source++];
        if (optimize && incumbent && selected.bound>=incumbent_bound) {
          selected.space.reset(); sources--; pruned++; continue;
        }
        f.source_revision=selected.revision;
        f.expansion.select(std::move(selected.space));
      }
      if (worker.stop(options) || work==commit_quota) {
        suspensions++; return {Step::SUSPENDED,{}};
      }
      const bool owned_source=static_cast<bool>(f.expansion);
      auto child=f.expansion.next(worker,options,f.depth+1,f.source_revision,
        optimize ? incumbent.get() : nullptr,revision);
      if (owned_source && !f.expansion) { sources--; sample(); }
      if (child.state==Search::Seq::ExpansionStep::SUSPENDED) {
        suspensions++; return {Step::SUSPENDED,{}};
      }
      if (child.state==Search::Seq::ExpansionStep::EXHAUSTED) continue;
      if (child.state==Search::Seq::ExpansionStep::CHILD) {
        commits++; work++;
      }
      if (child.status==SS_FAILED) { worker.fail++; continue; }
      const auto bound=guidance.bound(*child.space);
      if (optimize && incumbent && bound>=incumbent_bound) {
        pruned++; continue;
      }
      if (!cadds) f.page_survivors++;
      if (child.status==SS_SOLVED) {
        if (auto result=publish(std::move(child.space)))
          return {Step::SOLUTION,std::move(result)};
      } else {
        f.staged.push_back(entry(std::move(child.space),bound)); staged++; sample();
      }
    }
  }
};

static void numbers(const std::vector<int>& v) {
  std::cout<<'[';
  for (size_t k=0;k<v.size();k++) std::cout<<(k?",":"")<<v[k];
  std::cout<<']';
}
static void require(bool condition,const char* message) {
  if (!condition) throw std::runtime_error(message);
}

// Original choices change enumeration after a cut. Branching nodes 1 and 2
// expose the two-parent fragment schedule; goals occur at different depths.
class Witness : public Space {
public:
  int node=0,ceiling=100,cost=0;
  std::vector<int> path;
  static inline std::vector<int> expanded;
  class Branch : public Brancher {
    bool prepared=false;
    class Decision : public Choice {
    public:
      std::vector<int> actions;
      Decision(const Brancher& b,std::vector<int> a)
        : Choice(b,static_cast<unsigned>(a.size())),actions(std::move(a)) {}
      void archive(Archive& e) const override {
        Choice::archive(e); e<<static_cast<unsigned>(actions.size());
        for (int a:actions) e<<a;
      }
    };
  public:
    Branch(Home h) : Brancher(h) {}
    Branch(Space& h,Branch& b) : Brancher(h,b),prepared(b.prepared) {}
    bool status(const Space& h) const override {
      return static_cast<const Witness&>(h).node<10;
    }
    bool prepare(Space&,Search::Statistics&,const Search::Options&) override {
      if (!prepared) { prepared=true; return false; }
      return true;
    }
    const Choice* choice(Space& h) override {
      const auto& s=static_cast<const Witness&>(h);
      std::vector<int> a;
      if (s.node==0) { a={9,1,2}; if (s.ceiling<=9) a.erase(a.begin()); }
      else if (s.node==1) a={3,10};
      else if (s.node==2) a={4};
      else if (s.node==3) a={7};
      else a={5};
      expanded.push_back(s.node);
      return new Decision(*this,std::move(a));
    }
    const Choice* choice(const Space&,Archive& e) override {
      unsigned n; e>>n; std::vector<int> a(n); for (int& v:a) e>>v;
      return new Decision(*this,std::move(a));
    }
    ExecStatus commit(Space& h,const Choice& c,unsigned a) override {
      auto& s=static_cast<Witness&>(h);
      const int action=static_cast<const Decision&>(c).actions[a];
      s.path.push_back(action); prepared=false;
      if (action==10) return ES_FAILED;
      if (action>=1 && action<=4) { s.node=action; return ES_OK; }
      s.node=10; s.cost=action==9?9:(action==5?5:3);
      return s.cost<s.ceiling ? ES_OK : ES_FAILED;
    }
    Actor* copy(Space& h) override { return new(h) Branch(h,*this); }
    size_t dispose(Space& h) override { (void)Brancher::dispose(h); return sizeof(*this); }
  };
  explicit Witness(int state=0) : node(state) { (void)new(*this)Branch(*this); }
  Witness(Witness& s) : Space(s),node(s.node),ceiling(s.ceiling),cost(s.cost),path(s.path) {}
  Space* copy() override { return new Witness(*this); }
  void constrain(const Space& s) override {
    ceiling=static_cast<const Witness&>(s).cost;
    if (node>=10 && cost>=ceiling) fail();
  }
};

static void witnesses() {
  Search::HeuristicGuidance g{Search::HeuristicGuidance::MINIMIZE,
    [](const Space& s)->std::int64_t {
      const auto& w=static_cast<const Witness&>(s); return w.node>=10?w.cost:0;
    },[](const Space& s)->std::int64_t {
      const auto& w=static_cast<const Witness&>(s); return w.node==1 || w.node==3?100:0;
    }};
  std::vector<int> schedule;
  struct Observation { bool cadds; unsigned width; std::vector<int> costs; };
  std::vector<Observation> observations;
  unsigned pauses=0;
  for (bool cadds:{true,false}) for (unsigned width:{1u,4u}) {
    Witness::expanded.clear();
    Study coverage(std::make_unique<Witness>(),cadds,false,width,g);
    std::set<std::vector<int>> goals;
    for (;;) {
      auto step=coverage.advance(1);
      if (step.kind==Step::EXHAUSTED) break;
      if (step.kind==Step::SUSPENDED) { pauses++; continue; }
      goals.insert(static_cast<const Witness&>(*step.solution).path);
    }
    require(goals==std::set<std::vector<int>>{{9},{1,3,7},{2,4,5}},"coverage");
    require(coverage.statistics().fail==1 && coverage.done(),"exhaustion");
    if (cadds && width==4) {
      schedule=Witness::expanded;
      const auto second=std::find(schedule.begin(),schedule.end(),1);
      const auto successor=std::find(schedule.begin(),schedule.end(),4);
      require(second<successor,"fragment descended before second source");
      require(schedule==std::vector<int>{0,2,1,4,3},"fragment witness order");
    }
    Study optimization(std::make_unique<Witness>(),cadds,true,width,g);
    std::vector<int> costs;
    for (;;) {
      auto step=optimization.advance(1);
      if (step.kind==Step::EXHAUSTED) break;
      if (step.kind==Step::SOLUTION) costs.push_back(static_cast<Witness&>(*step.solution).cost);
    }
    require(!costs.empty() && costs.front()==9 && costs.back()==3 &&
      std::adjacent_find(costs.begin(),costs.end(),std::less_equal<int>())==costs.end(),
      "strict improvement or misleading rank");
    observations.push_back({cadds,width,std::move(costs)});
  }
  Search::NodeStop stop(0); Search::Options options; options.stop=&stop;
  Study resumed(std::make_unique<Witness>(),true,false,4,g,options);
  require(resumed.advance().kind==Step::SUSPENDED && !resumed.done(),"ordinary stop");
  stop.limit(1000);
  require(resumed.advance().kind==Step::SUSPENDED,"preparation suspension");
  // Stop after commits while the original root continuation is still active.
  require(resumed.advance(1).kind==Step::SOLUTION,"shallow goal");
  stop.limit(resumed.statistics().node-1);
  require(resumed.advance().kind==Step::SUSPENDED && !resumed.done(),"midfragment stop");
  stop.limit(1000);
  unsigned returns=1;
  for (;;) {
    auto step=resumed.advance(1);
    if (step.kind==Step::EXHAUSTED) break;
    if (step.kind==Step::SOLUTION) returns++;
  }
  require(returns==3 && resumed.done(),"stop resume lost work");
  auto failed=std::make_unique<Witness>(); failed->fail();
  Study empty(std::move(failed),true,false,4,g);
  require(empty.advance().kind==Step::EXHAUSTED,"failed root");
  Study solved(std::make_unique<Witness>(10),true,true,4,g);
  require(solved.advance().kind==Step::SOLUTION &&
          solved.advance().kind==Step::EXHAUSTED,"solved root");
  // Directly establish the fresh-choice index shift, using clone-related Spaces.
  Search::Statistics stat; Search::Options opt;
  auto source=std::make_unique<Witness>(); (void)source->status();
  require(!source->prepare(stat,opt) && source->prepare(stat,opt),"prepare");
  std::unique_ptr<const Choice> original(source->choice());
  auto cut=std::unique_ptr<Witness>(static_cast<Witness*>(source->clone()));
  cut->ceiling=6; (void)cut->status();
  std::unique_ptr<const Choice> fresh(cut->choice());
  auto wrong=std::unique_ptr<Witness>(static_cast<Witness*>(cut->clone()));
  wrong->commit(*fresh,1); (void)wrong->status();
  auto right=std::unique_ptr<Witness>(static_cast<Witness*>(source->clone()));
  right->commit(*original,1); right->ceiling=6; (void)right->status();
  require(wrong->path==std::vector<int>{2} && right->path==std::vector<int>{1},"choice index shift");
  std::cout<<"{\"kind\":\"witness\",\"coverage_configurations\":4,\"goals\":3,"
    <<"\"prepare_and_slice_suspensions\":"<<pauses
    <<",\"ordinary_stop_resume\":true,\"failed_solved_roots\":true,"
    <<"\"choice_index_shift\":true,\"full_fragment_expansion_order\":";
  numbers(schedule); std::cout<<",\"optimization_sequences\":[";
  for (size_t k=0;k<observations.size();k++) {
    const auto& observation=observations[k];
    std::cout<<(k?",":"")<<"{\"engine\":\""<<(observation.cadds?"cadds":"cursor")
      <<"\",\"width\":"<<observation.width<<",\"costs\":";
    numbers(observation.costs); std::cout<<'}';
  }
  std::cout<<"]}\n";
}

static double elapsed(Clock::time_point start) {
  return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}
static void solution(const State& s,bool coverage,Clock::time_point start) {
  std::cout<<"{\"kind\":\""<<(coverage?"coverage":"solution")
    <<"\",\"scaled_cost\":"<<s.cost()<<",\"elapsed_ms\":"<<elapsed(start)<<",\"tour\":";
  numbers(s.tour()); std::cout<<"}\n"<<std::flush;
}
static void statistics(const Search::Statistics& s) {
  std::cout<<"{\"node\":"<<s.node<<",\"fail\":"<<s.fail<<",\"depth\":"<<s.depth
    <<",\"propagate\":"<<s.propagate<<",\"restart\":"<<s.restart<<"}";
}

int main(int argc,char** argv) {
  try {
    if (argc==2 && std::string(argv[1])=="witness") { witnesses(); return 0; }
    if (argc!=6) throw std::runtime_error("engine(cadds|cursor|cabs) mode(opt|coverage) width time-ms instance");
    const std::string engine=argv[1],mode=argv[2];
    require(engine=="cadds" || engine=="cursor" || engine=="cabs","engine");
    require(mode=="opt" || mode=="coverage","mode");
    require(engine!="cabs" || mode=="opt","native CABS is optimization only");
    const unsigned width=std::stoul(argv[3]); require(width==1 || width==4,"width");
    const unsigned time_ms=std::stoul(argv[4]);
    const auto start=Clock::now();
    auto data=std::make_shared<const TSPTW::Instance>(TSPTW::read(argv[5]));
    auto measurements=std::make_shared<TSPTW::Measurements>();
    auto guidance=TSPTW::Transition::guidance(TSPTW::Transition::MIN_ARCS,measurements);
    Search::TimeStop stop(time_ms); Search::Options opt;
    opt.clone=false; opt.threads=1; if (time_ms) opt.stop=&stop;
    Search::Statistics counts;
    bool stopped=false;
    int best=-1; unsigned publications=0;
    unsigned long commits=0,pending=0,staged=0,sources=0,fragments=0,pruned=0;
    {
      if (engine=="cabs") {
        CABS<State> search(new State(data,measurements),guidance,opt);
        while (std::unique_ptr<State> s{search.next()}) {
          best=s->cost(); publications++; solution(*s,false,start);
        }
        stopped=search.stopped(); counts=search.statistics();
      } else {
        Study study(std::make_unique<State>(data,measurements),engine=="cadds",mode=="opt",width,guidance,opt);
        for (;;) {
          auto step=study.advance();
          if (step.kind==Step::EXHAUSTED) break;
          if (step.kind==Step::SUSPENDED) { stopped=true; break; }
          auto& s=static_cast<const State&>(*step.solution);
          best=best<0?s.cost():std::min(best,s.cost()); publications++;
          solution(s,mode=="coverage",start);
        }
        counts=study.statistics(); commits=study.commits; pending=study.peak_pending;
        staged=study.peak_staged; sources=study.peak_sources;
        fragments=study.fragments; pruned=study.pruned;
      }
    }
    require(State::live==0,"leaked live states");
    const char* termination=stopped?"stopped":(mode=="coverage"?"exhausted":(best<0?"infeasible":"optimal"));
    std::cout<<"{\"kind\":\"summary\",\"engine\":\""<<engine<<"\",\"termination\":\""<<termination
      <<"\",\"elapsed_ms\":"<<elapsed(start)<<",\"scale\":"<<data->scale<<",\"best_scaled_cost\":";
    if (best<0) std::cout<<"null"; else std::cout<<best;
    std::cout<<",\"publications\":"<<publications<<",\"statistics\":"; statistics(counts);
    std::cout<<",\"peak_live_spaces\":"<<State::peak<<",\"clones\":"<<State::clones;
    if (engine!="cabs") std::cout<<",\"commits\":"<<commits<<",\"peak_pending_nodes\":"<<pending
      <<",\"peak_staged_nodes\":"<<staged<<",\"peak_source_nodes\":"<<sources
      <<",\"fragments_or_pages\":"<<fragments<<",\"bound_pruned\":"<<pruned;
    std::cout<<",\"measurements\":{\"bound_calls\":"<<measurements->bound_calls
      <<",\"bound_ms\":"<<measurements->bound_ms<<",\"inference_calls\":"<<measurements->inference_calls
      <<",\"inference_ms\":"<<measurements->inference_ms<<",\"propagation_ms\":"<<measurements->propagation_ms
      <<"}}\n";
  } catch (const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
