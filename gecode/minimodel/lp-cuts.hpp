/* Exact scoped cover-cut records and bounded deterministic separation.
 * Experimental, opt-in; no native or numerical backend integration.
 * SPDX-License-Identifier: MIT
 */
#ifndef GECODE_MINIMODEL_LP_CUTS_HPP
#define GECODE_MINIMODEL_LP_CUTS_HPP

#include <gecode/minimodel/lp-strengthening.hpp>
#include <memory>
#include <stdexcept>

namespace Gecode { namespace Experimental { namespace LpRelaxation { namespace Cuts {

using Integer=std::int64_t;

/// Copies share one immutable identity; rebuilding an equal model creates another.
class SourceModel {
  std::shared_ptr<const BoundedIntegerModel> data_;
  static std::shared_ptr<const BoundedIntegerModel> snapshot(BoundedIntegerModel input) {
    validate_integer_model(input);
    return std::make_shared<const BoundedIntegerModel>(std::move(input));
  }
public:
  explicit SourceModel(BoundedIntegerModel input):data_(snapshot(std::move(input))) {}
  const BoundedIntegerModel& model() const {
    if (!data_) throw std::invalid_argument("cut source is moved from");
    return *data_;
  }
  bool same_identity(const SourceModel& other) const noexcept {
    return data_ && data_==other.data_;
  }
};

struct Box {
  std::vector<Integer> lower,upper;
};
enum class ScopeKind { Global, Local };
struct Scope {
  ScopeKind kind=ScopeKind::Global;
  Box box;
  static Scope global() {return {};}
  static Scope local(Box value) {return {ScopeKind::Local,std::move(value)};}
};

/// Untrusted proof request. The checker derives weights/signs from the source row.
struct CoverProof {
  std::size_t row=0;
  std::vector<std::size_t> columns;
  Scope scope;
};
/// Canonical original-variable inequality sum(coefficient[k]*x[column[k]])<=upper.
struct SparseCut {
  std::vector<std::size_t> column;
  std::vector<Integer> coefficient;
  Integer upper=0;
};

namespace Detail {
struct LimitReached {};
struct Meter {
  std::size_t used=0,limit=std::numeric_limits<std::size_t>::max();
  void tick() {
    if (used==limit) throw LimitReached{};
    ++used;
  }
};
// A fixed merge schedule makes work-limit prefixes independent of std::sort's
// implementation. Both comparisons and output entries consume work units.
template<class T,class Less>
inline void sort(std::vector<T>& values,Less less,Meter& work) {
  const auto n=values.size();
  if (n<2) return;
  std::vector<T> scratch(n);
  for (std::size_t width=1;width<n;) {
    for (std::size_t first=0;first<n;) {
      const auto middle=first+std::min(width,n-first);
      const auto end=middle+std::min(width,n-middle);
      auto left=first,right=middle;
      for (auto out=first;out<end;++out) {
        bool from_right=left==middle;
        if (left<middle && right<end) {work.tick();from_right=less(values[right],values[left]);}
        work.tick();scratch[out]=from_right?values[right++]:values[left++];
      }
      first=end;
    }
    values.swap(scratch);
    if (width>n/2) break;
    width*=2;
  }
}
// Reuse the established strengthening arithmetic and literal representation.
using Term=Strengthening::Detail::Term;
using PackingRow=Strengthening::Detail::PackingRow;
inline Integer add(Integer a,Integer b) {
  Integer out;
  if (!Strengthening::Detail::add(a,b,out)) throw std::overflow_error("cover addition overflow");
  return out;
}
inline Integer subtract(Integer a,Integer b) {
  Integer out;
  if (!Strengthening::Detail::subtract(a,b,out)) throw std::overflow_error("cover subtraction overflow");
  return out;
}
inline Integer times_positive(Integer value,Integer positive) {
  if (positive<=0) throw std::invalid_argument("cover multiplier must be positive");
  if ((value>0 && value>std::numeric_limits<Integer>::max()/positive) ||
      (value<0 && value<std::numeric_limits<Integer>::min()/positive))
    throw std::overflow_error("cover multiplication overflow");
  return value*positive;
}
inline bool valid_box(const BoundedIntegerModel& model,const Box& box,Meter& work) {
  const auto n=model.lower.size();
  if (box.lower.size()!=n || box.upper.size()!=n) return false;
  for (std::size_t j=0;j<n;++j) {
    work.tick();
    if (box.lower[j]<model.lower[j] || box.upper[j]>model.upper[j] || box.lower[j]>box.upper[j]) return false;
  }
  return true;
}
struct Bounds {
  const std::vector<Integer>& lower;
  const std::vector<Integer>& upper;
};
inline Bounds bounds(const BoundedIntegerModel& model,const Scope& scope,Meter& work) {
  if (scope.kind==ScopeKind::Global) {
    if (!scope.box.lower.empty() || !scope.box.upper.empty())
      throw std::invalid_argument("global cover proof cannot carry local bounds");
    return {model.lower,model.upper};
  }
  if (scope.kind!=ScopeKind::Local || !valid_box(model,scope.box,work))
    throw std::invalid_argument("local cover scope must be a nonempty box inside the source domains");
  return {scope.box.lower,scope.box.upper};
}
inline bool contains(const Box& outer,const Box& inner) {
  for (std::size_t j=0;j<outer.lower.size();++j)
    if (inner.lower[j]<outer.lower[j] || inner.upper[j]>outer.upper[j]) return false;
  return true;
}
inline bool covers_scope(const Scope& outer,const Scope& inner) {
  if (outer.kind==ScopeKind::Global) return true;
  if (inner.kind==ScopeKind::Global) return false;
  return contains(outer.box,inner.box);
}
struct UnsupportedRow : std::invalid_argument {
  UnsupportedRow():std::invalid_argument("cover row has a free nonbinary variable in its certified scope") {}
};
inline PackingRow packing(const BoundedIntegerModel& model,std::size_t row,
                          const Bounds& box,Meter& work) {
  const auto& matrix=model.linear;
  if (row>=matrix.b.size()) throw std::invalid_argument("cover proof source row is out of range");
  PackingRow result;
  result.capacity=subtract(0,matrix.b[row]);
  for (auto k=matrix.row_start[row];k<matrix.row_start[row+1];++k) {
    work.tick();
    const auto j=matrix.column[k];const Integer a=matrix.a[k];
    const Integer weight=a>0?a:subtract(0,a);
    if (box.lower[j]==box.upper[j]) {
      Integer fixed=times_positive(box.lower[j],weight);
      if (a<0) fixed=subtract(0,fixed);
      result.capacity=add(result.capacity,fixed);
      continue;
    }
    if (box.lower[j]<0 || box.upper[j]>1) throw UnsupportedRow{};
    const bool complement=a>0;
    result.terms.push_back({2*j+(complement?1U:0U),weight});
    if (complement) result.capacity=add(result.capacity,weight);
  }
  return result;
}
struct Verifier;
inline bool same_row(const SparseCut& a,const SparseCut& b) {
  return a.upper==b.upper && a.column==b.column && a.coefficient==b.coefficient;
}
inline bool same_row(const SparseCut& a,const SparseCut& b,Meter& work) {
  work.tick();
  if (a.upper!=b.upper || a.column.size()!=b.column.size()) return false;
  for (std::size_t i=0;i<a.column.size();++i) {
    work.tick();
    if (a.column[i]!=b.column[i] || a.coefficient[i]!=b.coefficient[i]) return false;
  }
  return true;
}
}

/// No constructor accepts an arbitrary inequality or unverified proof fields.
class VerifiedCut {
  SourceModel source_;
  CoverProof proof_;
  SparseCut inequality_;
  Integer capacity_,weight_;
  VerifiedCut(SourceModel source,CoverProof proof,SparseCut row,Integer capacity,Integer weight)
    : source_(std::move(source)),proof_(std::move(proof)),inequality_(std::move(row)),
      capacity_(capacity),weight_(weight) {}
  friend struct Detail::Verifier;
public:
  const SourceModel& source() const {return source_;}
  const CoverProof& proof() const {return proof_;}
  const SparseCut& inequality() const {return inequality_;}
  Integer capacity() const {return capacity_;}
  Integer cover_weight() const {return weight_;}
  bool applies_to(const SourceModel& source,const Box& box) const {
    if (!source_.same_identity(source)) return false;
    Detail::Meter work;
    if (!Detail::valid_box(source_.model(),box,work)) return false;
    return proof_.scope.kind==ScopeKind::Global || Detail::contains(proof_.scope.box,box);
  }
};

namespace Detail {
struct Verifier {
  static VerifiedCut make(const SourceModel& source,const CoverProof& claim,Meter& work) {
    const auto& model=source.model();
    const auto box=bounds(model,claim.scope,work);
    auto row=packing(model,claim.row,box,work);
    if (claim.columns.size()>row.terms.size())
      throw std::invalid_argument("cover proof selects too many free source terms");
    auto columns=claim.columns;
    Detail::sort(columns,[](std::size_t a,std::size_t b){return a<b;},work);
    SparseCut cut;cut.upper=static_cast<Integer>(columns.size())-1;
    Integer weight=0;std::size_t position=0;
    for (std::size_t i=0;i<columns.size();++i) {
      work.tick();
      if (i && columns[i]==columns[i-1]) throw std::invalid_argument("cover proof repeats a variable");
      while (position<row.terms.size() && row.terms[position].literal/2<columns[i]) {
        work.tick();++position;
      }
      if (position==row.terms.size() || row.terms[position].literal/2!=columns[i])
        throw std::invalid_argument("cover proof selects an absent or fixed source variable");
      const auto& term=row.terms[position];const bool complement=(term.literal&1U)!=0;
      weight=add(weight,term.weight);
      cut.column.push_back(columns[i]);cut.coefficient.push_back(complement?-1:1);
      if (complement) cut.upper=subtract(cut.upper,1);
    }
    if (weight<=row.capacity)
      throw std::invalid_argument("selected terms do not form a strict knapsack cover");
    CoverProof proof=claim;proof.columns=std::move(columns);
    return VerifiedCut(source,std::move(proof),std::move(cut),row.capacity,weight);
  }
};
}

/// Independently reconstruct and verify a proof, regardless of any candidate point.
inline VerifiedCut verify_cover(const SourceModel& source,const CoverProof& proof) {
  Detail::Meter work;
  return Detail::Verifier::make(source,proof,work);
}

struct PoolLimits {
  std::size_t max_cuts=64,max_nonzeros=4096,max_scope_values=16384;
};
enum class InsertStatus { Inserted, Replaced, Duplicate, Capacity, ForeignSource };

/** A bounded pool of already verified immutable records for one source identity.
 * Identical inequalities may share wider independently proved scopes. Two
 * incomparable local boxes are never replaced by their union or interval hull.
 */
class CutPool {
  SourceModel source_;
  PoolLimits limits_;
  std::vector<VerifiedCut> cuts_;
  std::size_t nonzeros_=0,scope_values_=0;
  static bool accumulate(std::size_t value,std::size_t limit,std::size_t& total) {
    if (value>limit-total) return false;
    total+=value;return true;
  }
public:
  explicit CutPool(SourceModel source,PoolLimits limits={})
    : source_(std::move(source)),limits_(limits) {(void)source_.model();}
  const std::vector<VerifiedCut>& records() const {return cuts_;}
  std::size_t nonzeros() const {return nonzeros_;}
  std::size_t scope_values() const {return scope_values_;}
  InsertStatus insert(const VerifiedCut& candidate) {
    if (!source_.same_identity(candidate.source())) return InsertStatus::ForeignSource;
    std::vector<unsigned char> remove(cuts_.size(),0);std::size_t removed=0;
    for (std::size_t i=0;i<cuts_.size();++i) {
      if (!Detail::same_row(cuts_[i].inequality(),candidate.inequality())) continue;
      if (Detail::covers_scope(cuts_[i].proof().scope,candidate.proof().scope)) return InsertStatus::Duplicate;
      if (Detail::covers_scope(candidate.proof().scope,cuts_[i].proof().scope)) {remove[i]=1;++removed;}
    }
    std::size_t count=0,terms=0,scopes=0;
    const auto fits=[&](const VerifiedCut& cut) {
      return accumulate(1,limits_.max_cuts,count) &&
        accumulate(cut.inequality().column.size(),limits_.max_nonzeros,terms) &&
        accumulate(cut.proof().scope.box.lower.size(),limits_.max_scope_values,scopes) &&
        accumulate(cut.proof().scope.box.upper.size(),limits_.max_scope_values,scopes);
    };
    if (!fits(candidate)) return InsertStatus::Capacity;
    for (std::size_t i=0;i<cuts_.size();++i) if (!remove[i] && !fits(cuts_[i])) return InsertStatus::Capacity;
    std::vector<VerifiedCut> next;
    for (std::size_t i=0;i<cuts_.size();++i) if (!remove[i]) next.push_back(cuts_[i]);
    next.push_back(candidate); // Allocation failure leaves the existing pool unchanged.
    cuts_.swap(next);nonzeros_=terms;scope_values_=scopes;
    return removed?InsertStatus::Replaced:InsertStatus::Inserted;
  }
  /// Returned values retain their own source/scope; later pool edits cannot alter them.
  std::vector<VerifiedCut> applicable(const Box& box) const {
    std::vector<VerifiedCut> result;
    for (const auto& cut:cuts_) if (cut.applies_to(source_,box)) result.push_back(cut);
    return result;
  }
};

/// Exact rational selection point. It is not proof of a cut's validity.
struct FractionalPoint {
  std::vector<Integer> numerator;
  Integer denominator=1048576;
};
struct SeparationOptions {
  std::size_t max_work=100000,max_rows=256,max_terms_per_row=512;
  std::size_t max_cuts=32,max_cuts_per_row=4,max_starts_per_row=4;
};
struct SeparationStats {
  std::size_t work=0,rows=0,unsupported_rows=0,oversized_rows=0;
  std::size_t arithmetic_rejections=0,duplicate_cuts=0;
  bool work_limit=false,row_limit=false,cut_limit=false;
};
struct SeparationResult {
  std::vector<VerifiedCut> cuts;
  SeparationStats stats;
};

namespace Detail {
inline void point_in_box(const FractionalPoint& point,const Bounds& box,Meter& work) {
  if (point.denominator<=0 || point.numerator.size()!=box.lower.size())
    throw std::invalid_argument("cover selection point requires matching coordinates and a positive denominator");
  for (std::size_t j=0;j<point.numerator.size();++j) {
    work.tick();
    const auto lower=times_positive(box.lower[j],point.denominator);
    const auto upper=times_positive(box.upper[j],point.denominator);
    if (point.numerator[j]<lower || point.numerator[j]>upper)
      throw std::invalid_argument("cover selection point is outside its certified scope box");
  }
}
inline Integer literal_value(const Term& term,const FractionalPoint& point) {
  return term.literal&1U ? subtract(point.denominator,point.numerator[term.literal/2])
                        : point.numerator[term.literal/2];
}
inline bool violated(const std::vector<Term>& cover,const FractionalPoint& point,Meter& work) {
  Integer activity=0;
  for (const auto& term:cover) {work.tick();activity=add(activity,literal_value(term,point));}
  return activity>times_positive(static_cast<Integer>(cover.size())-1,point.denominator);
}
}

/** Deterministic greedy covers with exact proof reconstruction and point checks.
 * Reuses the established strengthening rule: grow a cover, remove any light
 * term while its remainder still exceeds capacity, then emit cardinality <=k-1.
 * Work caps can miss cuts; an empty result makes no separation-completeness claim.
 */
inline SeparationResult separate_covers(const SourceModel& source,const FractionalPoint& point,
                                        const Scope& scope=Scope::global(),const SeparationOptions& options={}) {
  SeparationResult result;Detail::Meter work{0,options.max_work};
  if (!options.max_cuts) {result.stats.cut_limit=true;return result;}
  try {
    const auto& model=source.model();const auto& matrix=model.linear;
    const auto box=Detail::bounds(model,scope,work);
    Detail::point_in_box(point,box,work);
    for (std::size_t i=0;i<matrix.b.size();++i) {
      if (result.stats.rows==options.max_rows) {result.stats.row_limit=true;break;}
      work.tick();++result.stats.rows;
      if (matrix.row_start[i+1]-matrix.row_start[i]>options.max_terms_per_row) {
        ++result.stats.oversized_rows;continue;
      }
      Detail::PackingRow row;
      try {row=Detail::packing(model,i,box,work);}
      catch (const Detail::UnsupportedRow&) {++result.stats.unsupported_rows;continue;}
      catch (const std::overflow_error&) {++result.stats.arithmetic_rejections;continue;}
      const auto before=result.cuts.size();
      const auto append=[&](const std::vector<Detail::Term>& cover) {
        if (!Detail::violated(cover,point,work)) return;
        CoverProof claim;claim.row=i;claim.scope=scope;
        for (const auto& term:cover) {work.tick();claim.columns.push_back(term.literal/2);}
        auto cut=Detail::Verifier::make(source,claim,work);
        for (const auto& prior:result.cuts) {
          if (Detail::same_row(prior.inequality(),cut.inequality(),work)) {++result.stats.duplicate_cuts;return;}
        }
        result.cuts.push_back(std::move(cut));
      };
      if (!options.max_cuts_per_row) continue;
      try {
        if (row.capacity<0) append({});
        else for (unsigned policy=0;policy<3 && result.cuts.size()<options.max_cuts &&
                            result.cuts.size()-before<options.max_cuts_per_row;++policy) {
          auto order=row.terms;
          Detail::sort(order,[&](const Detail::Term& a,const Detail::Term& b) {
            if (policy==0) {
              const auto av=Detail::literal_value(a,point),bv=Detail::literal_value(b,point);
              if (av!=bv) return av>bv;
            }
            if (policy!=2 && a.weight!=b.weight) return a.weight>b.weight;
            return a.literal<b.literal;
          },work);
          const auto starts=std::min(order.size(),options.max_starts_per_row);
          for (std::size_t start=0;start<starts && result.cuts.size()<options.max_cuts &&
                                     result.cuts.size()-before<options.max_cuts_per_row;++start) {
            Integer weight=0;std::vector<Detail::Term> cover;
            for (std::size_t k=0;k<order.size() && weight<=row.capacity;++k) {
              work.tick();const auto& term=order[(start+k)%order.size()];
              weight=Detail::add(weight,term.weight);cover.push_back(term);
            }
            if (weight<=row.capacity) continue;
            Detail::sort(cover,[](const Detail::Term& a,const Detail::Term& b) {
              return a.weight!=b.weight?a.weight<b.weight:a.literal<b.literal;
            },work);
            std::vector<Detail::Term> minimal;
            for (const auto& term:cover) {
              work.tick();const auto remaining=Detail::subtract(weight,term.weight);
              if (remaining>row.capacity) weight=remaining;else minimal.push_back(term);
            }
            append(minimal);
          }
        }
      } catch (const std::overflow_error&) {++result.stats.arithmetic_rejections;}
      if (result.cuts.size()==options.max_cuts) {result.stats.cut_limit=true;break;}
    }
  } catch (const Detail::LimitReached&) {result.stats.work_limit=true;}
  result.stats.work=work.used;
  return result;
}

}}}}
#endif
