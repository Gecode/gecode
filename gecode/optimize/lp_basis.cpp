#include <gecode/optimize/lp_basis_detail.hpp>
#include <gecode/optimize/validate.hpp>
#include <cmath>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
void tick(const SolveBudget* budget) {
  // Admission does not mutate any backend. The solve wrapper translates this
  // finite-work interruption into the shared budget's exact stop reason.
  if (budget && budget->expired()) throw ModelError("Basis admission interrupted by solve budget");
}
void status(LpBasisStatus value,double lower,double upper) {
  switch(value) {
  case LpBasisStatus::Basic: case LpBasisStatus::NonbasicUnspecified: return;
  case LpBasisStatus::Lower:
    if(std::isfinite(lower))return;
    throw ModelError("Lower basis status requires a finite lower bound");
  case LpBasisStatus::Upper:
    if(std::isfinite(upper))return;
    throw ModelError("Upper basis status requires a finite upper bound");
  case LpBasisStatus::Zero:
    if(!std::isfinite(lower)&&!std::isfinite(upper))return;
    throw ModelError("Zero basis status requires a free row or column");
  }
  throw ModelError("Unknown original LP basis status");
}
bool same(Variable a,Variable b){return a.model_id==b.model_id&&a.id==b.id;}
bool same(Constraint a,Constraint b){return a.model_id==b.model_id&&a.id==b.id;}
bool terms(const std::vector<Term>& a,const std::vector<Term>& b,const SolveBudget& budget){
  if(a.size()!=b.size())return false;
  for(std::size_t i=0;i<a.size();++i){
    if(!(i%256))tick(&budget);
    if(!same(a[i].variable,b[i].variable)||a[i].coefficient!=b[i].coefficient)return false;
  }
  return true;
}
}
void LpBasisSolveOptions::validate() const {
  observations.validate();
  if(!basis)throw ModelError("solve_lp_with_basis requires an owning basis");
  if(!observations.solve.primal_start.empty())
    throw ModelError("A basis and primal_start cannot be submitted together");
}
std::shared_ptr<const LpBasis> make_lp_basis(const LpBasisData& data){
  return Detail::LpBasisAccess::create(data,LpBasisOrigin::Caller);
}
std::shared_ptr<const LpBasis> make_lp_basis(const LpObservations& observed){
  if(observed.basis().state!=LpObservationState::Available)
    throw ModelError("Cannot construct a basis from an unavailable observation group");
  LpBasisData data;data.source=observed.source();
  data.rows.reserve(observed.rows().size());data.columns.reserve(observed.columns().size());
  for(const auto& row:observed.rows())data.rows.push_back(row.basis);
  for(const auto& col:observed.columns())data.columns.push_back(col.basis);
  return Detail::LpBasisAccess::create(std::move(data),LpBasisOrigin::Observations);
}
namespace Detail {
void LpBasisAccess::validate_statuses(const ModelSnapshot& model,
    const std::vector<std::optional<LpBasisStatus>>& rows,
    const std::vector<std::optional<LpBasisStatus>>& columns,const SolveBudget* budget){
  if(rows.size()!=model.rows.size()||columns.size()!=model.variables.size())
    throw ModelError("Basis dimensions must exactly match original row/column slots");
  std::size_t basics=0,active_rows=0;
  for(std::size_t i=0;i<model.variables.size();++i){
    if(!(i%256))tick(budget);
    const auto& v=model.variables[i];
    if(columns[i].has_value()!=v.active)throw ModelError("Basis column presence differs from original active mask");
    if(!v.active)continue;
    if(v.type!=VariableType::Continuous)throw ModelError("LP basis source must contain only active Continuous variables");
    status(*columns[i],v.lower,v.upper);basics+=*columns[i]==LpBasisStatus::Basic;
  }
  for(std::size_t i=0;i<model.rows.size();++i){
    if(!(i%256))tick(budget);
    const auto& row=model.rows[i];
    if(rows[i].has_value()!=row.active)throw ModelError("Basis row presence differs from original active mask");
    if(!row.active)continue;
    if(row.terms.empty())throw ModelError("LP basis submission does not support active constant rows");
    ++active_rows;status(*rows[i],row.lower,row.upper);basics+=*rows[i]==LpBasisStatus::Basic;
  }
  for(const auto& record:model.indicators)if(record.active)throw ModelError("LP basis source cannot have active indicators");
  for(const auto& record:model.globals)if(record.active)throw ModelError("LP basis source cannot have active globals");
  if(basics!=active_rows)throw ModelError("Number of basic entities must equal the number of active rows");
  tick(budget);
}
std::shared_ptr<const LpBasis> LpBasisAccess::create(LpBasisData data,LpBasisOrigin origin){
  validate_structure(data.source);validate_statuses(data.source,data.rows,data.columns);
  auto basis=std::shared_ptr<LpBasis>(new LpBasis);basis->data_=std::move(data);basis->origin_=origin;
  return basis;
}
void LpBasisAccess::compatible(const LpBasis& basis,const ModelSnapshot& next,const SolveBudget& budget){
  const auto& old=basis.source();
  auto mismatch=[](){throw ModelError("Basis requires identical original owner, revision and active source content");};
  if(old.model_id!=next.model_id||old.revision!=next.revision||old.variables.size()!=next.variables.size()||old.rows.size()!=next.rows.size())mismatch();
  validate_statuses(next,basis.rows(),basis.columns(),&budget);
  for(std::size_t i=0;i<next.variables.size();++i){
    if(!(i%256))tick(&budget);
    const auto& a=old.variables[i];const auto& b=next.variables[i];
    if(!same(a.variable,b.variable)||a.active!=b.active)mismatch();
    if(a.active&&(a.type!=b.type||a.lower!=b.lower||a.upper!=b.upper||a.name!=b.name))mismatch();
  }
  for(std::size_t i=0;i<next.rows.size();++i){
    if(!(i%256))tick(&budget);
    const auto& a=old.rows[i];const auto& b=next.rows[i];
    if(!same(a.constraint,b.constraint)||a.active!=b.active)mismatch();
    if(a.active&&(a.lower!=b.lower||a.upper!=b.upper||a.name!=b.name||!terms(a.terms,b.terms,budget)))mismatch();
  }
  if(old.objective.sense!=next.objective.sense||old.objective.offset!=next.objective.offset||
     !terms(old.objective.terms,next.objective.terms,budget))mismatch();
  tick(&budget);
}
}
}}
