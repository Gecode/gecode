/* Experimental binary linear presolve and redundant cuts.
 * SPDX-License-Identifier: MIT
 */
#ifndef GECODE_MINIMODEL_LP_STRENGTHENING_HPP
#define GECODE_MINIMODEL_LP_STRENGTHENING_HPP

#include <gecode/minimodel/lp-model.hpp>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

namespace Gecode { namespace Experimental { namespace LpRelaxation {
namespace Strengthening {

struct Options {
  bool gcd = true;
  bool fixings = true;
  bool pairs = true;
  bool cliques = true;
  bool covers = true;
  std::size_t max_cuts = 256;
  std::size_t max_graph_variables = 128;
  std::size_t max_pair_checks = 200000;
  std::size_t max_clique_seeds = 128;
  std::size_t max_clique_size = 64;
  std::size_t max_cover_cuts_per_row = 4;
  std::size_t max_cover_terms = 512;
  std::size_t max_cover_starts = 4;
};

struct Stats {
  std::size_t rows_before = 0, rows_after = 0;
  std::size_t gcd_rows = 0, duplicates_removed = 0;
  std::size_t tautologies_removed = 0;
  std::size_t pair_checks = 0, pair_conflicts = 0;
  std::size_t fixing_cuts = 0, pair_cuts = 0;
  std::size_t clique_cuts = 0, cover_cuts = 0;
  std::size_t arithmetic_rejections = 0, cover_rows_skipped = 0;
  bool graph_skipped = false, pair_limit_reached = false;
  bool cut_limit_reached = false, infeasible = false;
  std::size_t cuts_added(void) const {
    return fixing_cuts + pair_cuts + clique_cuts + cover_cuts;
  }
};

struct Result {
  LinearModel model;
  Stats stats;
};

namespace Detail {
using Integer = std::int64_t;
using Row = std::vector<Integer>;

inline bool add(Integer a, Integer b, Integer& out) {
  if ((b > 0 && a > std::numeric_limits<Integer>::max()-b) ||
      (b < 0 && a < std::numeric_limits<Integer>::min()-b))
    return false;
  out=a+b;
  return true;
}
inline bool subtract(Integer a, Integer b, Integer& out) {
  if ((b > 0 && a < std::numeric_limits<Integer>::min()+b) ||
      (b < 0 && a > std::numeric_limits<Integer>::max()+b))
    return false;
  out=a-b;
  return true;
}

struct Term {
  // 2*j means x[j]; 2*j+1 means 1-x[j].
  std::size_t literal;
  Integer weight;
};
struct PackingRow {
  std::vector<Term> terms;
  Integer capacity;
};
enum class Kind { Original, Fixing, Pair, Clique, Cover };

class Builder {
  const LinearModel& original;
  const Options& options;
  const std::size_t n;
  std::map<Row,Integer> rows;
  // Keep surviving originals in their input order and append generated cuts.
  // Map iterators remain valid when new rows are inserted or bounds tightened.
  std::vector<std::map<Row,Integer>::const_iterator> row_order;
  Stats stats;

  void contradiction(void) {
    row_order.clear();
    rows.clear();
    row_order.push_back(rows.emplace(Row(n,0),1).first);
    stats.infeasible=true;
  }

  // Insert an equivalent normalized original row or a proved redundant cut.
  bool insert(Row row, Integer rhs, Kind kind) {
    if (stats.infeasible)
      return false;
    Integer divisor=0, minimum=0, maximum=0;
    for (Integer coefficient : row) {
      // Input validation and generated +/-1 cuts exclude INT64_MIN here.
      if (coefficient < -1000000000LL || coefficient > 1000000000LL) {
        ++stats.arithmetic_rejections;
        return false;
      }
      divisor=std::gcd(divisor,coefficient < 0 ? -coefficient : coefficient);
      if (!add(minimum,std::min<Integer>(0,coefficient),minimum) ||
          !add(maximum,std::max<Integer>(0,coefficient),maximum)) {
        ++stats.arithmetic_rejections;
        return false;
      }
    }
    if (rhs > maximum) {
      contradiction();
      return false;
    }
    if (rhs <= minimum) {
      if (kind==Kind::Original)
        ++stats.tautologies_removed;
      return false;
    }
    if (options.gcd && divisor > 1) {
      for (Integer& coefficient : row)
        coefficient/=divisor;
      // Mathematical ceiling for a positive divisor, including negative rhs.
      const Integer remainder=rhs%divisor;
      rhs/=divisor;
      if (remainder > 0 && !add(rhs,1,rhs)) {
        ++stats.arithmetic_rejections;
        return false;
      }
      if (kind==Kind::Original)
        ++stats.gcd_rows;
    }
    if (rhs < -1000000000LL || rhs > 1000000000LL) {
      ++stats.arithmetic_rejections;
      return false;
    }
    auto found=rows.find(row);
    if (found!=rows.end()) {
      if (kind==Kind::Original)
        ++stats.duplicates_removed;
      if (found->second >= rhs)
        return false;
    }
    if (kind!=Kind::Original && stats.cuts_added() >= options.max_cuts) {
      stats.cut_limit_reached=true;
      return false;
    }
    if (found==rows.end()) {
      row_order.push_back(rows.emplace(std::move(row),rhs).first);
    } else
      found->second=rhs;
    switch (kind) {
    case Kind::Original: break;
    case Kind::Fixing: ++stats.fixing_cuts; break;
    case Kind::Pair: ++stats.pair_cuts; break;
    case Kind::Clique: ++stats.clique_cuts; break;
    case Kind::Cover: ++stats.cover_cuts; break;
    }
    return true;
  }

  // Translate sum(literals)<=limit into a row in the original x variables.
  bool cardinality(const std::vector<std::size_t>& literals,
                   Integer limit, Kind kind) {
    Row row(n,0);
    Integer rhs;
    if (!subtract(0,limit,rhs)) {
      ++stats.arithmetic_rejections;
      return false;
    }
    for (std::size_t literal : literals) {
      const bool complement=(literal&1U)!=0;
      if (!add(row[literal/2],complement ? 1 : -1,row[literal/2]) ||
          (complement && !add(rhs,1,rhs))) {
        ++stats.arithmetic_rejections;
        return false;
      }
    }
    return insert(std::move(row),rhs,kind);
  }

  Result finish(void) {
    Result result;
    result.model.c=original.c;
    result.model.b.reserve(rows.size());
    for (const auto& position : row_order) {
      const auto& entry=*position;
      result.model.a.insert(result.model.a.end(),entry.first.begin(),entry.first.end());
      result.model.b.push_back(entry.second);
    }
    stats.rows_after=result.model.b.size();
    result.stats=stats;
    return result;
  }

public:
  Builder(const LinearModel& input,const Options& settings)
    : original(input),options(settings),n(input.c.size()) {
    validate_model(input);
    stats.rows_before=input.b.size();
  }

  Result run(void) {
    for (std::size_t i=0; i<original.b.size(); ++i) {
      Row row(original.a.begin()+i*n,original.a.begin()+(i+1)*n);
      const auto rejected=stats.arithmetic_rejections;
      insert(std::move(row),original.b[i],Kind::Original);
      // Never silently remove an original constraint if arithmetic fails.
      if (stats.arithmetic_rejections!=rejected)
        throw std::overflow_error("Binary presolve arithmetic");
      if (stats.infeasible)
        return finish();
    }
    if (options.max_cuts==0)
      return finish();

    std::vector<PackingRow> packing;
    packing.reserve(rows.size());
    for (const auto& position : row_order) {
      const auto& entry=*position;
      PackingRow converted;
      converted.capacity=0;
      bool valid=true;
      for (std::size_t j=0; j<n; ++j) {
        const Integer coefficient=entry.first[j];
        if (coefficient==0)
          continue;
        const bool complement=coefficient>0;
        const Integer weight=complement ? coefficient : -coefficient;
        converted.terms.push_back({2*j+(complement ? 1 : 0),weight});
        if (complement && !add(converted.capacity,weight,converted.capacity))
          valid=false;
      }
      if (!subtract(converted.capacity,entry.second,converted.capacity))
        valid=false;
      if (!valid) {
        ++stats.arithmetic_rejections;
        continue;
      }
      // Ax>=b is exactly sum |a[j]|*literal[j] <= sum(a[j]>0)a[j]-b.
      if (converted.capacity<0) {
        contradiction();
        return finish();
      }
      packing.push_back(std::move(converted));
    }

    std::vector<unsigned char> forced_zero(2*n,0);
    if (options.fixings) {
      for (const auto& row : packing)
        for (const Term& term : row.terms)
          if (term.weight>row.capacity) {
            forced_zero[term.literal]=1;
            if (forced_zero[term.literal^1U]) {
              contradiction();
              return finish();
            }
            cardinality({term.literal},0,Kind::Fixing);
          }
    }
    if (stats.infeasible)
      return finish();

    const bool use_graph=(options.pairs || options.cliques) &&
      n<=options.max_graph_variables;
    std::vector<std::pair<std::size_t,std::size_t>> conflicts;
    if ((options.pairs || options.cliques) && !use_graph)
      stats.graph_skipped=true;
    if (use_graph) {
      const std::size_t vertices=2*n;
      if (vertices!=0 && vertices>std::numeric_limits<std::size_t>::max()/vertices)
        throw std::overflow_error("Binary conflict graph dimensions");
      std::vector<unsigned char> adjacency(vertices*vertices,0);
      std::vector<std::size_t> degree(vertices,0);
      auto edge=[&](std::size_t u,std::size_t v,bool derived) {
        if (adjacency[u*vertices+v])
          return;
        adjacency[u*vertices+v]=adjacency[v*vertices+u]=1;
        ++degree[u]; ++degree[v];
        if (derived) {
          conflicts.emplace_back(std::min(u,v),std::max(u,v));
          ++stats.pair_conflicts;
        }
      };
      for (std::size_t j=0; j<n; ++j)
        edge(2*j,2*j+1,false); // A literal and its complement never both hold.
      bool stop_pairs=false;
      for (const auto& row : packing) {
        for (std::size_t i=0; i<row.terms.size() && !stop_pairs; ++i) {
          if (forced_zero[row.terms[i].literal]) continue;
          for (std::size_t j=i+1; j<row.terms.size(); ++j) {
            if (forced_zero[row.terms[j].literal]) continue;
            if (stats.pair_checks >= options.max_pair_checks) {
              stats.pair_limit_reached=true;
              stop_pairs=true;
              break;
            }
            ++stats.pair_checks;
            Integer weight;
            if (!add(row.terms[i].weight,row.terms[j].weight,weight)) {
              ++stats.arithmetic_rejections;
              continue;
            }
            if (weight>row.capacity)
              edge(row.terms[i].literal,row.terms[j].literal,true);
          }
        }
        if (stop_pairs) break;
      }
      if (options.cliques && options.max_clique_size>=3) {
        std::vector<std::size_t> order(vertices);
        std::iota(order.begin(),order.end(),0);
        std::sort(order.begin(),order.end(),[&](std::size_t u,std::size_t v) {
          return degree[u]!=degree[v] ? degree[u]>degree[v] : u<v;
        });
        const std::size_t seeds=std::min(vertices,options.max_clique_seeds);
        for (std::size_t s=0; s<seeds && !stats.infeasible; ++s) {
          const std::size_t seed=order[s];
          if (forced_zero[seed]) continue;
          std::vector<std::size_t> clique{seed};
          for (std::size_t candidate : order) {
            if (candidate==seed || forced_zero[candidate]) continue;
            bool compatible=true;
            for (std::size_t member : clique)
              if (!adjacency[candidate*vertices+member]) {
                compatible=false; break;
              }
            if (compatible) {
              clique.push_back(candidate);
              if (clique.size()>=options.max_clique_size) break;
            }
          }
          if (clique.size()>=3)
            cardinality(clique,1,Kind::Clique);
        }
      }
    }
    if (stats.infeasible)
      return finish();

    if (options.covers) {
      for (const auto& row : packing) {
        if (stats.infeasible) break;
        if (row.terms.size()>options.max_cover_terms) {
          ++stats.cover_rows_skipped;
          continue;
        }
        std::vector<Term> terms;
        for (const Term& term : row.terms)
          if (!forced_zero[term.literal]) terms.push_back(term);
        if (terms.empty() ||
            std::all_of(terms.begin(),terms.end(),[](const Term& t){return t.weight==1;}))
          continue; // Unit-weight subset covers are already implied by the LP row.
        const std::size_t before=stats.cover_cuts;
        for (unsigned int policy=0; policy<3; ++policy) {
          std::vector<Term> order=terms;
          std::sort(order.begin(),order.end(),[&](const Term& u,const Term& v) {
            if (policy!=2 && u.weight!=v.weight)
              return policy==0 ? u.weight>v.weight : u.weight<v.weight;
            return u.literal<v.literal;
          });
          const std::size_t starts=std::min(order.size(),options.max_cover_starts);
          for (std::size_t start=0; start<starts; ++start) {
            if (stats.cover_cuts-before >= options.max_cover_cuts_per_row)
              break;
            Integer weight=0;
            bool valid=true;
            std::vector<Term> cover;
            for (std::size_t k=0; k<order.size() && weight<=row.capacity; ++k) {
              const Term& term=order[(start+k)%order.size()];
              cover.push_back(term);
              if (!add(weight,term.weight,weight)) {valid=false; break;}
            }
            if (!valid) {++stats.arithmetic_rejections; continue;}
            if (weight<=row.capacity) continue;
            std::sort(cover.begin(),cover.end(),[](const Term& u,const Term& v) {
              return u.weight!=v.weight ? u.weight<v.weight : u.literal<v.literal;
            });
            std::vector<std::size_t> literals;
            for (const Term& term : cover) {
              Integer without;
              if (!subtract(weight,term.weight,without)) {valid=false; break;}
              if (without>row.capacity) weight=without;
              else literals.push_back(term.literal);
            }
            if (!valid) {++stats.arithmetic_rejections; continue;}
            // The retained subset itself must still be a cover.
            if (weight>row.capacity && !literals.empty())
              cardinality(literals,static_cast<Integer>(literals.size())-1,Kind::Cover);
          }
        }
      }
    }
    if (stats.infeasible)
      return finish();
    if (options.pairs)
      for (const auto& pair : conflicts) {
        cardinality({pair.first,pair.second},1,Kind::Pair);
        if (stats.infeasible) break;
      }
    return finish();
  }
};
} // namespace Detail

/**
 * Return a binary-equivalent model with identical objective/variable mapping.
 *
 * GCD rounding is valid because each row activity is integral. Pair and clique
 * cuts come only from proved literal conflicts; cover cuts come only from
 * subsets whose nonnegative packing weights exceed the capacity. Complements
 * are translated back to the original x variables. These cuts can remove
 * fractional LP points, but do not remove any feasible binary assignment.
 *
 * All candidate arithmetic is checked. Original rows are never silently
 * dropped on arithmetic failure. Limits can weaken the strengthening, but
 * cannot change feasibility. max_cuts=0 retains only normalization/deduplication.
 * A proved contradiction is returned as the row 0>=1.
 */
inline Result strengthen(const LinearModel& model,const Options& options=Options()) {
  return Detail::Builder(model,options).run();
}

} // namespace Strengthening
}}}
#endif
