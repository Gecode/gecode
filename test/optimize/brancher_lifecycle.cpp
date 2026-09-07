/* Brancher list sentinels remain ActorLinks, including cloning and deletion. */
#include <gecode/int.hh>
#include <gecode/search.hh>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace Gecode;
namespace {
void check(bool okay) { if (!okay) throw std::runtime_error("brancher lifecycle regression"); }
class State : public Space {
public:
  IntVarArray x;
  explicit State(int size) : x(*this,size,0,1) {
    for(int i=0;i<size;++i) branch(*this,x[i],INT_VAL_MIN());
  }
  State(State& other) : Space(other) { x.update(*this,other.x); }
  Space* copy() override { return new State(*this); }
  std::vector<unsigned int> ids() {
    std::vector<unsigned int> result;
    for(Space::Branchers b(*this);b();++b) result.push_back(b.brancher().id());
    return result;
  }
  unsigned int propagators() {
    unsigned int count=0;
    for(Space::Propagators p(*this);p();++p) { (void)&p.propagator();++count; }
    return count;
  }
  unsigned int idle_propagators() {
    unsigned int count=0;
    for(Space::IdlePropagators p(*this);p();++p) { (void)&p.propagator();++count; }
    return count;
  }
};
}
int main() {
  State empty(0);
  check(empty.ids().empty() && empty.status()==SS_SOLVED && empty.choice()==nullptr);
  check(empty.propagators()==0 && empty.idle_propagators()==0);
  std::unique_ptr<State> empty_clone(static_cast<State*>(empty.clone()));
  check(empty_clone->ids().empty() && empty_clone->status()==SS_SOLVED);

  State original(3);check(original.status()==SS_BRANCH);
  const auto original_ids=original.ids();check(original_ids.size()==3);
  std::unique_ptr<const Choice> first(original.choice());
  std::unique_ptr<State> child(static_cast<State*>(original.clone()));
  child->commit(*first,0);check(child->status()==SS_BRANCH && child->x[0].val()==0);
  std::unique_ptr<const Choice> second(child->choice());
  check(child->ids()==std::vector<unsigned int>({original_ids[1],original_ids[2]}));
  Archive archive;second->archive(archive);
  std::unique_ptr<const Choice> restored(child->choice(archive));
  std::unique_ptr<State> grandchild(static_cast<State*>(child->clone()));
  grandchild->commit(*restored,1);
  check(grandchild->status()==SS_BRANCH && grandchild->x[1].val()==1);
  std::unique_ptr<const Choice> third(grandchild->choice());
  grandchild->commit(*third,0);check(grandchild->status()==SS_SOLVED);
  check(grandchild->choice()==nullptr && grandchild->ids().empty());

  // Look up a later brancher, then wrap around to an earlier one without
  // calling choice(), which would delete the exhausted actors.
  std::unique_ptr<State> wrap(static_cast<State*>(original.clone()));
  wrap->commit(*third,0);wrap->commit(*first,0);
  check(wrap->x[0].val()==0 && wrap->x[2].val()==0 && wrap->status()==SS_BRANCH);
  std::unique_ptr<State> missing_middle(static_cast<State*>(original.clone()));
  BrancherGroup middle;middle.move(*missing_middle,original_ids[1]);middle.kill(*missing_middle);
  bool no_middle=false;
  try { missing_middle->commit(*second,0); } catch(const SpaceNoBrancher&) { no_middle=true; }
  check(no_middle && missing_middle->ids().size()==2);
  missing_middle->commit(*third,0);missing_middle->commit(*first,0);
  check(missing_middle->status()==SS_SOLVED);

  std::unique_ptr<State> killed(static_cast<State*>(original.clone()));
  BrancherGroup removal;
  for (auto id : {original_ids[1],original_ids[0],original_ids[2]}) {
    removal.move(*killed,id);removal.kill(*killed);
  }
  check(killed->ids().empty() && killed->status()==SS_SOLVED);
  bool missing=false;
  try { killed->commit(*first,0); } catch(const SpaceNoBrancher&) { missing=true; }
  check(missing);
  std::unique_ptr<State> killed_clone(static_cast<State*>(killed->clone()));
  check(killed_clone->status()==SS_SOLVED && killed_clone->ids().empty());

  State failed(2);failed.fail();
  check(failed.status()==SS_FAILED && failed.choice()==nullptr && failed.ids().empty());
  State constrained(3);rel(constrained,constrained.x[0],IRT_LQ,constrained.x[1]);
  check(constrained.propagators()>0);
  check(constrained.status()==SS_BRANCH && constrained.idle_propagators()>0);
  Search::Options options;options.threads=1;options.c_d=3;options.a_d=2;
  DFS<State> search(&original,options);unsigned int seen=0,mask=0;
  while(std::unique_ptr<State> solution{search.next()}) {
    ++seen;mask |= 1U << (solution->x[0].val()+2*solution->x[1].val()+4*solution->x[2].val());
  }
  check(seen==8 && mask==255 && !search.stopped());
}
