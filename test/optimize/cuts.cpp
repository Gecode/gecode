// Independent exhaustive validity/scope oracles; no LP solver or search calls.
#include <gecode/minimodel/lp-cuts.hpp>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <type_traits>

namespace LP=Gecode::Experimental::LpRelaxation;
namespace C=LP::Cuts;
using I=std::int64_t;
using Point=std::vector<I>;
static void check(bool okay,const char* message) {if(!okay)throw std::runtime_error(message);}
template<class Function> static void rejected(Function function,const char* message) {
  bool failed=false;
  try {function();}catch(const std::invalid_argument&){failed=true;}catch(const std::overflow_error&){failed=true;}
  check(failed,message);
}
static LP::BoundedIntegerModel model(LP::LinearModel dense,Point lower,Point upper) {
  return {LP::sparse_model(dense),std::move(lower),std::move(upper)};
}
static C::Box box(const LP::BoundedIntegerModel& source) {return {source.lower,source.upper};}
static bool in_box(const Point& point,const C::Box& bounds) {
  for(std::size_t j=0;j<point.size();++j)
    if(point[j]<bounds.lower[j] || point[j]>bounds.upper[j])return false;
  return true;
}
static I activity(const C::SparseCut& cut,const Point& point) {
  I value=0;for(std::size_t k=0;k<cut.column.size();++k)value+=cut.coefficient[k]*point[cut.column[k]];return value;
}
static std::vector<Point> enumerate(const LP::BoundedIntegerModel& source) {
  const auto& m=source.linear;Point point(m.c.size());std::vector<Point> found;
  const auto visit=[&](auto&& self,std::size_t column)->void {
    if(column<point.size()) {
      for(I value=source.lower[column];value<=source.upper[column];++value) {point[column]=value;self(self,column+1);}
      return;
    }
    for(std::size_t row=0;row<m.b.size();++row) {
      I value=0;for(auto k=m.row_start[row];k<m.row_start[row+1];++k)value+=m.a[k]*point[m.column[k]];
      if(value<m.b[row])return;
    }
    found.push_back(point);
  };visit(visit,0);return found;
}
static std::size_t assert_valid(const C::VerifiedCut& cut,const C::SourceModel& source,
                               const std::vector<Point>& points) {
  std::size_t excluded_siblings=0;
  const auto& inequality=cut.inequality();
  check(inequality.column.size()==inequality.coefficient.size(),"cut term dimensions");
  for(std::size_t i=0;i<inequality.column.size();++i) {
    check(!i || inequality.column[i]>inequality.column[i-1],"cut is not canonical");
    check(inequality.coefficient[i]==-1 || inequality.coefficient[i]==1,"cover coefficients are not literal signs");
  }
  for(const auto& point:points) {
    const bool inside=cut.proof().scope.kind==C::ScopeKind::Global || in_box(point,cut.proof().scope.box);
    check(cut.applies_to(source,{point,point})==inside,"cut scope differs from independent box containment");
    if(inside)check(activity(inequality,point)<=inequality.upper,"cover removed a feasible original integer assignment");
    else if(activity(inequality,point)>inequality.upper)++excluded_siblings;
  }
  const auto rechecked=C::verify_cover(source,cut.proof());
  check(rechecked.inequality().column==inequality.column && rechecked.inequality().coefficient==inequality.coefficient &&
        rechecked.inequality().upper==inequality.upper && rechecked.capacity()==cut.capacity() &&
        rechecked.cover_weight()==cut.cover_weight(),"stored cover proof does not reconstruct its cut");
  return excluded_siblings;
}
static std::string fingerprint(const C::SeparationResult& result) {
  std::ostringstream text;
  for(const auto& cut:result.cuts) {
    text<<cut.proof().row<<':'<<static_cast<int>(cut.proof().scope.kind)<<':'<<cut.inequality().upper<<';';
    for(auto v:cut.inequality().column)text<<v<<',';
    text<<'|';for(auto v:cut.inequality().coefficient)text<<v<<',';
    text<<'|';for(auto v:cut.proof().scope.box.lower)text<<v<<',';
    text<<'|';for(auto v:cut.proof().scope.box.upper)text<<v<<',';
    text<<'\n';
  }
  const auto& s=result.stats;
  text<<s.work<<':'<<s.rows<<':'<<s.unsupported_rows<<':'<<s.oversized_rows<<':'
      <<s.arithmetic_rejections<<':'<<s.duplicate_cuts<<':'<<s.work_limit<<':'<<s.row_limit<<':'<<s.cut_limit;
  return text.str();
}

static void proof_edges() {
  static_assert(!std::is_default_constructible<C::VerifiedCut>::value,"unverified default cut must not exist");
  auto input=model({{-3,-3,0},{-5},{0,0,0}},{0,0,0},{1,1,1});
  C::SourceModel source(input),identical(input);auto shared=source;
  auto cut=C::verify_cover(source,{0,{1,0},C::Scope::global()});
  check(cut.capacity()==5 && cut.cover_weight()==6 && cut.inequality().upper==1 &&
        cut.inequality().column==std::vector<std::size_t>({0,1}),"strict cover or canonical ordering");
  check(cut.applies_to(shared,box(input)) && !cut.applies_to(identical,box(input)),"source identity was structural or lost on copy");
  input.linear.a[0]=-1;input.upper[0]=3;
  check(source.model().linear.a[0]==-3 && source.model().upper[0]==1,"source snapshot changed after input mutation");
  auto moved=std::move(source);
  check(cut.applies_to(moved,box(moved.model())) && !cut.applies_to(source,box(moved.model())),"source move identity");
  rejected([&]{(void)source.model();},"moved source accepted");
  assert_valid(cut,moved,enumerate(moved.model()));
  auto row_copy=cut.inequality();row_copy.upper=100;
  check(cut.inequality().upper==1,"mutable inequality copy modified the verified record");
  const auto retained=[] {
    C::SourceModel temporary(model({{-3,-3},{-5},{0,0}},{0,0},{1,1}));
    return C::verify_cover(temporary,{0,{0,1},{}});
  }();
  assert_valid(retained,retained.source(),enumerate(retained.source().model()));
  for(const auto& claim:std::vector<C::CoverProof>{
        {0,{0},{}},{0,{0,0},{}},{0,{0,2},{}},{0,{99},{}},{99,{},{}},
        {0,{0,1},{C::ScopeKind::Global,{{0,0,0},{1,1,0}}}},
        {0,{0,1},C::Scope::local({{0,0},{1,1}})},
        {0,{0,1},C::Scope::local({{-1,0,0},{1,1,1}})},
        {0,{0,1},C::Scope::local({{1,0,0},{0,1,1}})},
        {0,{0,1},{static_cast<C::ScopeKind>(-1),{}}}})
    rejected([&]{(void)C::verify_cover(moved,claim);},"forged or insufficient cover proof accepted");
  check(!cut.applies_to(moved,{{0},{1}}) && !cut.applies_to(moved,{{0,0,0},{2,1,1}}),"invalid application box accepted");

  C::SourceModel signed_row(model({{3,-3,3},{1},{0,0,0}},{0,0,0},{1,1,1}));
  auto signed_cut=C::verify_cover(signed_row,{0,{0,1},{}});
  check(signed_cut.inequality().coefficient==Point({-1,1}) && signed_cut.inequality().upper==0,
        "signed-literal translation lost a complement constant");
  assert_valid(signed_cut,signed_row,enumerate(signed_row.model()));
  for(I coefficient:{I(2),I(-2)}) {
    const I fixed=coefficient>0?2:-2;
    C::SourceModel fixed_row(model({{-3,-3,coefficient},{-1},{0,0,0}},{0,0,fixed},{1,1,fixed}));
    const auto fixed_cut=C::verify_cover(fixed_row,{0,{0,1},{}});
    check(fixed_cut.capacity()==5 && fixed_cut.inequality().upper==1,"fixed signed integer contribution");
    assert_valid(fixed_cut,fixed_row,enumerate(fixed_row.model()));
    rejected([&]{(void)C::verify_cover(fixed_row,{0,{0,2},{}});},"fixed variable accepted as free cover literal");
  }
  C::SourceModel fixed_zero(model({{3,-3},{-1},{0,0}},{0,0},{0,1}));
  auto zero_cut=C::verify_cover(fixed_zero,{0,{1},{}});
  check(zero_cut.capacity()==1 && zero_cut.inequality().upper==0,"fixed positive coefficient included twice");
  assert_valid(zero_cut,fixed_zero,enumerate(fixed_zero.model()));
  C::SourceModel contradiction(model({{},{1},{}},{},{}));
  auto impossible=C::verify_cover(contradiction,{0,{},{}});
  check(impossible.inequality().column.empty() && impossible.inequality().upper==-1,"empty cover contradiction");
  for(I rhs:{I(0),I(-1)}) {
    C::SourceModel constant(model({{},{rhs},{}},{},{}));
    rejected([&]{(void)C::verify_cover(constant,{0,{},{}});},"nonnegative capacity accepted an empty cover");
  }
  auto corrupt=model({{-3,-3},{-5},{0,0}},{0,0},{1,1});corrupt.linear.column[1]=0;
  rejected([&]{C::SourceModel invalid(corrupt);},"malformed sparse source accepted");
  corrupt=model({{-3},{-1},{0}},{0},{1});corrupt.linear.a[0]=std::numeric_limits<I>::min();
  rejected([&]{C::SourceModel invalid(corrupt);},"unsupported source arithmetic accepted");
}

static void scope_and_pool() {
  // In z=1, x+y<=1 is valid. In z=0, x=y=1 is feasible and violates it.
  C::SourceModel source(model({{-2,-2,-2,0},{-4},{0,0,0,0}},{0,0,0,0},{1,1,1,1}));
  auto parent=box(source.model());parent.lower[2]=1;
  auto left=parent,right=parent;left.upper[3]=0;right.lower[3]=1;
  const auto a=C::verify_cover(source,{0,{0,1},C::Scope::local(left)});
  const auto b=C::verify_cover(source,{0,{1,0},C::Scope::local(right)});
  const auto broad=C::verify_cover(source,{0,{0,1},C::Scope::local(parent)});
  check(assert_valid(a,source,enumerate(source.model()))>0,"local fixture cannot expose a sibling leak");
  auto sibling=box(source.model());sibling.upper[2]=0;
  check(!a.applies_to(source,right) && !a.applies_to(source,sibling) && broad.applies_to(source,left),"local cut leaked a sibling");
  rejected([&]{(void)C::verify_cover(source,{0,{0,1},{}});},"local proof silently became global");
  C::CutPool pool(source);
  check(pool.insert(a)==C::InsertStatus::Inserted && pool.insert(b)==C::InsertStatus::Inserted &&
        pool.records().size()==2,"incomparable scopes were merged");
  check(pool.applicable(parent).empty() && pool.applicable(left).size()==1 && pool.applicable(sibling).empty(),
        "pool applied a union of unproved scopes");
  auto historical=pool.applicable(left);
  check(pool.insert(broad)==C::InsertStatus::Replaced && pool.records().size()==1 &&
        pool.scope_values()==8 && pool.nonzeros()==2,"independently proved broader local scope did not replace narrower cuts");
  check(pool.insert(a)==C::InsertStatus::Duplicate && !historical[0].applies_to(source,right),
        "duplicate insertion widened a historical record");
  C::SourceModel foreign(source.model());
  auto foreign_cut=C::verify_cover(foreign,{0,{0,1},C::Scope::local(parent)});
  check(pool.insert(foreign_cut)==C::InsertStatus::ForeignSource && pool.records().size()==1,"pool accepted a foreign model");
  for(const auto limits:std::vector<C::PoolLimits>{{0,100,100},{2,1,100},{2,100,7}}) {
    C::CutPool limited(source,limits);
    check(limited.insert(broad)==C::InsertStatus::Capacity && limited.records().empty() &&
          limited.nonzeros()==0 && limited.scope_values()==0,"capacity rejection partially mutated pool");
  }
  C::SourceModel global_source(model({{-3,-3,0},{-5},{0,0,0}},{0,0,0},{1,1,1}));
  auto local=box(global_source.model());local.upper[2]=0;
  auto global=C::verify_cover(global_source,{0,{0,1},{}});
  C::CutPool replacement(global_source,{1,2,6});
  check(replacement.insert(C::verify_cover(global_source,{0,{0,1},C::Scope::local(local)}))==C::InsertStatus::Inserted,
        "local initial record insertion");
  check(replacement.insert(global)==C::InsertStatus::Replaced && replacement.scope_values()==0,
        "global proof replacement failed at pool capacity");
  check(replacement.insert(global)==C::InsertStatus::Duplicate,"global canonical duplicate");
}

static void separation_edges() {
  C::SourceModel source(model({{-3,-3,0,-3,0,-3},{-5,-5},{0,0,0}},{0,0,0},{1,1,1}));
  const C::FractionalPoint point{{3,3,3},4};
  // Each original packing row has activity 4.5<=5, but its cover has 1.5>1.
  const auto complete=C::separate_covers(source,point);
  check(complete.cuts.size()==2 && !complete.stats.work_limit,"fractional cover point was not separated");
  for(const auto& cut:complete.cuts) {
    check(activity(cut.inequality(),point.numerator)>cut.inequality().upper*point.denominator,
          "separator returned an unviolated cut");
    assert_valid(cut,source,enumerate(source.model()));
  }
  // Verification accepts a valid cut independently of whether this point violates it.
  const auto valid=C::verify_cover(source,{0,{0,1},{}});
  check(C::separate_covers(source,{{0,0,0},1}).cuts.empty() && valid.inequality().upper==1,
        "validity was confused with candidate-point violation");
  for(std::size_t budget=0;budget<300;++budget) {
    C::SeparationOptions options;options.max_work=budget;
    const auto a=C::separate_covers(source,point,{},options),b=C::separate_covers(source,point,{},options);
    check(a.stats.work<=budget && fingerprint(a)==fingerprint(b),"separation work budget or deterministic prefix");
    for(const auto& cut:a.cuts)assert_valid(cut,source,enumerate(source.model()));
  }
  C::SeparationOptions options;options.max_rows=0;
  check(C::separate_covers(source,point,{},options).stats.row_limit,"row limit ignored");
  options={};options.max_cuts=0;
  check(C::separate_covers(source,point,{},options).stats.cut_limit,"zero cut limit ignored");
  options={};options.max_cuts=1;
  const auto one=C::separate_covers(source,point,{},options);
  check(one.cuts.size()==1 && one.stats.cut_limit,"output cut limit ignored");
  options={};options.max_terms_per_row=1;
  const auto skipped=C::separate_covers(source,point,{},options);
  check(skipped.cuts.empty() && skipped.stats.oversized_rows==2,"row-term storage cap ignored");
  options={};options.max_cuts_per_row=0;
  check(C::separate_covers(source,point,{},options).cuts.empty(),"per-row zero cut limit ignored");
  options={};options.max_starts_per_row=0;
  check(C::separate_covers(source,point,{},options).cuts.empty(),"zero greedy starts ignored");
  C::SourceModel general(model({{-3,-3,2},{-1},{0,0,0}},{0,0,-2},{1,1,2}));
  rejected([&]{(void)C::verify_cover(general,{0,{0,1},{}});},"free general integer row treated as binary");
  const auto unsupported=C::separate_covers(general,{{1,1,0},2});
  check(unsupported.cuts.empty() && unsupported.stats.unsupported_rows==1,"unsupported row not reported");
  auto local=box(general.model());local.lower[2]=local.upper[2]=2;
  const auto localized=C::separate_covers(general,{{3,3,8},4},C::Scope::local(local));
  check(!localized.cuts.empty(),"fixed general integer substitution did not permit binary separation");
  for(const auto& cut:localized.cuts)assert_valid(cut,general,enumerate(general.model()));
  for(const auto& bad:std::vector<C::FractionalPoint>{{{0,0,0},0},{{0,0},1},{{-1,0,0},1},{{2,0,0},1}})
    rejected([&]{(void)C::separate_covers(source,bad);},"invalid selection point accepted");
  const I maximum=std::numeric_limits<I>::max();
  const auto overflow=C::separate_covers(source,{{maximum,maximum,maximum},maximum});
  check(overflow.cuts.empty() && overflow.stats.arithmetic_rejections>0,"overflowing rational separation was not rejected");
  C::SourceModel impossible(model({{},{1},{}},{},{}));
  const auto empty=C::separate_covers(impossible,{{},1});
  check(empty.cuts.size()==1 && empty.cuts[0].inequality().upper==-1,"constant contradiction not separated");

  // The source and query remain sparse; even coordinate validation obeys work.
  const std::size_t n=16384;
  LP::BoundedIntegerModel large;
  large.linear.row_start={0,2};large.linear.column={0,n-1};large.linear.a={-3,-3};large.linear.b={-5};
  large.linear.c.resize(n);large.lower.resize(n);large.upper.assign(n,1);
  C::SourceModel big(large);options={};options.max_work=100;
  const auto bounded=C::separate_covers(big,{Point(n,1),2},{},options);
  check(bounded.stats.work==100 && bounded.stats.work_limit && bounded.stats.rows==0 && bounded.cuts.empty(),
        "large source coordinate scan ignored separator budget");
  auto global=C::verify_cover(big,{0,{0,n-1},{}});
  C::CutPool sparse_pool(big);
  check(sparse_pool.insert(global)==C::InsertStatus::Inserted && sparse_pool.nonzeros()==2,
        "sparse cut pool retained dense coefficients");
}

static void exhaustive_oracles() {
  std::mt19937 random(975522);
  std::size_t verified=0,rejected_proofs=0,separated=0,sibling_witnesses=0;
  for(unsigned trial=0;trial<750;++trial) {
    const auto n=1+random()%4,m=1+random()%3;
    LP::LinearModel dense;dense.c.resize(n);dense.a.resize(n*m);dense.b.resize(m);
    Point lo(n),hi(n);
    for(std::size_t j=0;j<n;++j) {
      const auto kind=random()%5;
      if(kind<3){lo[j]=0;hi[j]=1;}
      else if(kind==3){lo[j]=hi[j]=static_cast<int>(random()%5)-2;}
      else {lo[j]=-2;hi[j]=2;}
    }
    for(auto& a:dense.a)a=random()%3?static_cast<int>(random()%11)-5:0;
    for(auto& b:dense.b)b=static_cast<int>(random()%15)-7;
    C::SourceModel source(model(dense,lo,hi));const auto feasible=enumerate(source.model());
    auto local=box(source.model());
    for(std::size_t j=0;j<n;++j)if(random()%2) {
      local.lower[j]=lo[j]+random()%(hi[j]-lo[j]+1);
      local.upper[j]=local.lower[j]+random()%(hi[j]-local.lower[j]+1);
    }
    for(const auto& scope:std::vector<C::Scope>{{},C::Scope::local(local)}) {
      for(std::size_t row=0;row<m;++row)for(unsigned mask=0;mask<(1U<<n);++mask) {
        C::CoverProof proof;proof.row=row;proof.scope=scope;
        for(std::size_t j=0;j<n;++j)if(mask&(1U<<j))proof.columns.push_back(j);
        try {
          const auto cut=C::verify_cover(source,proof);
          ++verified;sibling_witnesses+=assert_valid(cut,source,feasible);
        }catch(const std::invalid_argument&){++rejected_proofs;}
      }
      const auto& lower=scope.kind==C::ScopeKind::Global?lo:local.lower;
      const auto& upper=scope.kind==C::ScopeKind::Global?hi:local.upper;
      Point numerators(n);for(std::size_t j=0;j<n;++j)numerators[j]=lower[j]+upper[j];
      const auto result=C::separate_covers(source,{numerators,2},scope);
      check(result.stats.work<=C::SeparationOptions{}.max_work,"random separator exceeded budget");
      for(const auto& cut:result.cuts) {
        ++separated;sibling_witnesses+=assert_valid(cut,source,feasible);
        check(activity(cut.inequality(),numerators)>2*cut.inequality().upper,"random cut was not exactly violated");
      }
    }
  }
  check(verified>100 && rejected_proofs>100 && separated>100 && sibling_witnesses>0,"cover oracle coverage missing");
  std::cout<<"PASS 750 scoped integer models; "<<verified<<" independently verified covers, "<<rejected_proofs
           <<" rejected proofs, "<<separated<<" separated cuts, "<<sibling_witnesses<<" excluded sibling witnesses\n";
}
int main() {
  try {proof_edges();scope_and_pool();separation_edges();exhaustive_oracles();}
  catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
