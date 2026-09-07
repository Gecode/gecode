#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>

// Deterministic allocation failures exercise the public all-or-nothing boundary.
namespace Allocation {
long fail_after = -1;
std::size_t maximum = std::numeric_limits<std::size_t>::max(), largest = 0;
}
void* operator new(std::size_t bytes) {
  Allocation::largest = std::max(Allocation::largest, bytes);
  if (bytes > Allocation::maximum || Allocation::fail_after == 0) throw std::bad_alloc();
  if (Allocation::fail_after > 0) --Allocation::fail_after;
  if (void* memory = std::malloc(bytes ? bytes : 1)) return memory;
  throw std::bad_alloc();
}
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();
std::string fingerprint(const Model& model) {
  const auto source = model.snapshot(); std::ostringstream out;
  out.precision(17); out << source.model_id << ':' << source.revision;
  for (const auto& v : source.variables)
    out << '/' << v.variable.model_id << ':' << v.variable.id << ':' << int(v.type)
        << ':' << v.lower << ':' << v.upper << ':' << v.active << ':' << v.name;
  for (const auto& r : source.rows) {
    out << '/' << r.constraint.model_id << ':' << r.constraint.id << ':' << r.lower << ':' << r.upper << ':' << r.active << ':' << r.name;
    for (const auto& t : r.terms) out << ':' << t.variable.model_id << ',' << t.variable.id << ',' << t.coefficient;
  }
  out << '/' << int(source.objective.sense) << ':' << source.objective.offset;
  for (const auto& t : source.objective.terms) out << ':' << t.variable.id << ',' << t.coefficient;
  return out.str();
}
template<class Action> void rejects_unchanged(Model& model, Action action) {
  const auto before = fingerprint(model);
  bool caught = false;
  try { action(); } catch (const ModelError&) { caught = true; }
  assert(caught && fingerprint(model) == before);
}

void scalar_and_bulk_equivalence() {
  Model model;
  const auto removed = model.add_integer(0, 1); model.remove(removed);
  const auto old = model.snapshot(); const auto revision = model.revision();
  const std::vector<VariableSpec> specs{
    {VariableType::Integer, -3, 7, "integer"}, {VariableType::Continuous, -inf, inf, "free"},
    {VariableType::Binary, .2, .8, "empty integer domain"},
    {VariableType::SemiContinuous, 2, 9, "semi"}, {VariableType::SemiInteger, 2.5, 7, "semi integer"}};
  const auto variables = model.add_variables(specs);
  assert(model.revision() == revision + 1 && variables.size() == specs.size());
  for (std::size_t i = 0; i < variables.size(); ++i) {
    const auto& value = model.variable(variables[i]);
    assert(variables[i].model_id == model.id() && variables[i].id == i + 1);
    assert(value.type == specs[i].type && value.lower == specs[i].lower && value.upper == specs[i].upper && value.name == specs[i].name);
  }
  assert(old.variables.size() == 1 && !old.variables[0].active);
  const std::vector<RowSpec> rows{
    {{{variables[0], 1e16}, {variables[1], 2}, {variables[0], 1}, {variables[0], -1e16}}, -3, 8, "compensated"},
    {{}, -inf, inf, "empty"}, {{{variables[4], 3}, {variables[4], -3}}, 1, inf, "contradiction"}};
  const auto before = model.revision(); const auto handles = model.add_rows(rows);
  assert(model.revision() == before + 1 && handles.size() == 3);
  assert(model.row(handles[0]).terms.size() == 2 && model.row(handles[0]).terms[0].coefficient == 1);
  assert(model.row(handles[1]).terms.empty() && model.row(handles[2]).terms.empty());
  Model scalar; const auto sv = scalar.add_variables(specs);
  for (std::size_t i = 0; i < rows.size(); ++i) {
    auto terms = rows[i].terms; for (auto& term : terms) term.variable = sv[term.variable.id - 1];
    const auto handle = scalar.add_row(terms, rows[i].lower, rows[i].upper, rows[i].name);
    const auto& actual = model.row(handles[i]); const auto& expected = scalar.row(handle);
    assert(actual.lower == expected.lower && actual.upper == expected.upper && actual.name == expected.name);
    assert(actual.terms.size() == expected.terms.size());
    for (std::size_t k = 0; k < actual.terms.size(); ++k)
      assert(actual.terms[k].coefficient == expected.terms[k].coefficient && actual.terms[k].variable.id == expected.terms[k].variable.id + 1);
  }
  validate_structure(model.snapshot());
  const auto unchanged = fingerprint(model);
  assert(model.add_variables({}).empty() && model.add_rows({}).empty() && model.add_rows_sparse({}).empty());
  assert(fingerprint(model) == unchanged);
}

void csr_semantics_and_rejections() {
  Model model;
  auto vars = model.add_variables({{VariableType::Integer, -2, 2, "x"}, {VariableType::Integer, 0, 3, "y"}});
  auto gone = model.add_integer(0, 1); model.remove(gone);
  const auto removed = model.add_row({}, -inf, inf); model.remove(removed);
  SparseRowBatch rows;
  rows.columns = {vars[1], vars[0]}; // explicit mapping differs from slot order
  rows.row_start = {0, 4, 4, 5}; rows.column = {1, 0, 1, 1, 0};
  rows.coefficient = {1e16, 2, 1, -1e16, -3}; rows.lower = {-1, -inf, -9}; rows.upper = {4, inf, 0};
  rows.names = {"row", "empty", "bound"};
  const auto revision = model.revision(); const auto added = model.add_rows_sparse(rows);
  assert(model.revision() == revision + 1 && added.size() == 3 && added[0].id == 1);
  assert(model.row(added[0]).terms.size() == 2 && model.row(added[0]).terms[0].variable == vars[0]);
  assert(model.row(added[0]).terms[0].coefficient == 1 && model.row(added[0]).terms[1].coefficient == 2);
  for (int x = -2; x <= 2; ++x) for (int y = 0; y <= 3; ++y) {
    const auto checked = validate(model.snapshot(), {double(x), double(y), 0}, 0, 0);
    assert(checked.valid == (-1 <= x + 2*y && x + 2*y <= 4));
  }
  const auto bad = [&](auto change) { auto malformed = rows; change(malformed); rejects_unchanged(model, [&] { model.add_rows_sparse(malformed); }); };
  bad([](auto& r) { r.row_start.clear(); }); bad([](auto& r) { r.row_start[0] = 1; });
  bad([](auto& r) { r.row_start.back() = 4; }); bad([](auto& r) { r.row_start[1] = 6; });
  bad([](auto& r) { r.row_start[2] = 3; }); bad([](auto& r) { r.upper.pop_back(); });
  bad([](auto& r) { r.names.pop_back(); }); bad([](auto& r) { r.column.pop_back(); });
  bad([](auto& r) { r.column.back() = 2; }); bad([](auto& r) { r.coefficient.back() = inf; });
  bad([](auto& r) { r.lower.back() = 1; }); bad([](auto& r) { r.columns.push_back(r.columns[0]); });
  bad([&](auto& r) { r.columns.push_back(gone); });
  Model foreign; const auto other = foreign.add_binary(); bad([&](auto& r) { r.columns.push_back(other); });
  rejects_unchanged(model, [&] { model.add_rows({{{{vars[0], 1}}, 0, 1, {}}, {{{other, 0}}, 0, 1, {}}}); });
  rejects_unchanged(model, [&] { model.add_variables({{VariableType::Integer, 0, 1, {}}, {VariableType::Binary, -1, 1, {}}}); });
  rejects_unchanged(model, [&] { model.add_variables({{static_cast<VariableType>(-1), 0, 1, {}}}); });
  Model moved = std::move(model);
  bool caught = false; try { model.add_variables({}); } catch (const ModelError&) { caught = true; } assert(caught);
  assert(moved.row(added[0]).constraint.model_id == moved.id());
}

void allocation_atomicity() {
  for (int operation = 0; operation < 3; ++operation) {
    bool succeeded = false; unsigned failures = 0;
    for (long stop = 0; stop < 100 && !succeeded; ++stop) {
      Model model; auto x = model.add_integer(0, 3);
      auto original = model.add_row({{x, 2}}, 0, 6, "existing"); model.minimize({{x, 1}}, 2);
      const auto* variable_view = &model.variable(x); const auto* row_view = &model.row(original);
      const auto before = fingerprint(model);
      std::vector<VariableSpec> variables(3, {VariableType::Integer, -2, 2, std::string(100, 'v')});
      std::vector<RowSpec> rows(3, {{{x, 2}, {x, -1}}, 0, 6, std::string(100, 'r')});
      SparseRowBatch sparse; sparse.columns = {x}; sparse.row_start = {0, 1, 2, 3}; sparse.column = {0, 0, 0};
      sparse.coefficient = {1, 2, 3}; sparse.lower = {0, 0, 0}; sparse.upper = {3, 6, 9}; sparse.names = {std::string(100, 's'), "b", "c"};
      Allocation::fail_after = stop;
      try {
        if (operation == 0) (void) model.add_variables(variables);
        else if (operation == 1) (void) model.add_rows(rows);
        else (void) model.add_rows_sparse(sparse);
        Allocation::fail_after = -1; succeeded = true;
      } catch (const std::bad_alloc&) {
        Allocation::fail_after = -1; ++failures;
        assert(fingerprint(model) == before);
        assert(&model.variable(x) == variable_view && &model.row(original) == row_view);
      }
    }
    assert(succeeded && failures > 3);
  }
}

void sparse_scale() {
  constexpr std::size_t n = 16384;
  Model model; std::vector<VariableSpec> specs(n, {VariableType::Integer, 0, 1, {}});
  Allocation::largest = 0; Allocation::maximum = 4 * 1024 * 1024;
  const auto variables = model.add_variables(specs);
  SparseRowBatch input; input.columns = variables; input.row_start.resize(n + 1);
  input.column.resize(n); input.coefficient.assign(n, 1); input.lower.assign(n, 0); input.upper.assign(n, 1);
  for (std::size_t i = 0; i <= n; ++i) input.row_start[i] = i;
  for (std::size_t i = 0; i < n; ++i) input.column[i] = n - 1 - i;
  const auto rows = model.add_rows_sparse(input);
  Allocation::maximum = std::numeric_limits<std::size_t>::max();
  assert(rows.size() == n && model.revision() == 2);
  const auto snapshot = model.snapshot(); std::size_t nonzeros = 0;
  for (const auto& row : snapshot.rows) nonzeros += row.terms.size();
  assert(nonzeros == n && Allocation::largest < 4 * 1024 * 1024);
}
}
int main() {
  scalar_and_bulk_equivalence(); csr_semantics_and_rejections(); allocation_atomicity(); sparse_scale();
  std::cout << "Atomic bulk/CSR equivalence, allocation failure and 16K sparse construction pass\n";
}
