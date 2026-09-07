/* Pinned backend experiment: factor a supplied basis without optimization. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "Highs.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

static void near(double actual,double expected) {
  assert(std::isfinite(actual));
  assert(std::abs(actual-expected)<=1e-9*std::max(1.0,std::abs(expected)));
}
static HighsLp model(double first,double second,bool singular=false) {
  HighsLp lp;
  lp.num_col_=2;lp.num_row_=2;
  lp.col_cost_={1,2};lp.col_lower_={0,0};lp.col_upper_={10,10};
  lp.row_lower_={-100,-100};lp.row_upper_={100,100};
  lp.a_matrix_.format_=MatrixFormat::kColwise;
  lp.a_matrix_.start_={0,2,4};lp.a_matrix_.index_={0,1,0,1};
  lp.a_matrix_.value_={2*first,8*second,4*first,(singular?16:32)*second};
  return lp;
}
static void mixed(double first,double second) {
  Highs h;assert(h.setOptionValue("output_flag",false)==HighsStatus::kOk);
  assert(h.setOptionValue("threads",1)==HighsStatus::kOk);
  assert(h.passModel(model(first,second))==HighsStatus::kOk);
  HighsBasis b;b.alien=false;b.valid=true;b.useful=true;
  b.col_status={HighsBasisStatus::kBasic,HighsBasisStatus::kLower};
  b.row_status={HighsBasisStatus::kUpper,HighsBasisStatus::kBasic};
  assert(h.setBasis(b,"sensitivity factor-only experiment")==HighsStatus::kOk);
  assert(!h.hasInvert());
  std::vector<HighsInt> order(2);
  assert(h.getBasicVariables(order.data())==HighsStatus::kOk);
  assert(h.hasInvert());assert(order[0]==0&&order[1]==-2);
  assert(h.getBasis().col_status==b.col_status&&h.getBasis().row_status==b.row_status);
  // B = [ A_column_0, +e_1 ]. Logical coordinate z is -row activity.
  double rhs[]={6*first,40*second},answer[2]={};
  assert(h.getBasisSolve(rhs,answer)==HighsStatus::kOk);
  near(answer[0],3);near(answer[1],16*second);
  near(2*first*answer[0],rhs[0]);near(8*second*answer[0]+answer[1],rhs[1]);
  const double dual[]={4/first,3/second};
  const double trhs[]={32,3/second};
  assert(h.getBasisTransposeSolve(trhs,answer)==HighsStatus::kOk);
  near(answer[0],dual[0]);near(answer[1],dual[1]);
  near(2*first*answer[0]+8*second*answer[1],trhs[0]);near(answer[1],trhs[1]);
  // Original first-row RHS movement uses +e_0: z_0'=-1, x'=B^-1 e_0.
  const double direction_rhs[]={1,0};
  assert(h.getBasisSolve(direction_rhs,answer)==HighsStatus::kOk);
  near(2*first*answer[0],1);near(8*second*answer[0]+answer[1],0);
  assert(h.getModelStatus()==HighsModelStatus::kNotset);
  assert(!h.getInfo().valid);
}
int main() {
  mixed(1,1);mixed(1e-6,1e6);
  Highs h;assert(h.setOptionValue("output_flag",false)==HighsStatus::kOk);
  assert(h.passModel(model(1,1,true))==HighsStatus::kOk);
  HighsBasis b;b.alien=false;b.valid=true;b.useful=true;
  b.col_status={HighsBasisStatus::kBasic,HighsBasisStatus::kBasic};
  b.row_status={HighsBasisStatus::kLower,HighsBasisStatus::kUpper};
  assert(h.setBasis(b,"singular factor-only experiment")==HighsStatus::kOk);
  std::vector<HighsInt> order(2);
  assert(h.getBasicVariables(order.data())==HighsStatus::kError);
  assert(h.getBasis().col_status==b.col_status&&h.getBasis().row_status==b.row_status);
  assert(h.getModelStatus()==HighsModelStatus::kNotset);
  std::cout<<"factor-only normal/scaled signs and singular rejection passed\n";
}
