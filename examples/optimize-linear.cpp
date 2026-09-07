#include <gecode/optimize/solve.hpp>
#include <iostream>
#include <limits>

int main() {
  namespace O = Gecode::Optimize;
  O::Model model;
  const auto open = model.add_binary("open");
  const auto quantity = model.add_continuous(0.0, 100.0, "quantity");
  model.add_row({{quantity, 1.0}, {open, -100.0}},
                -std::numeric_limits<double>::infinity(), 0.0, "capacity");
  model.add_row({{quantity, 1.0}}, 40.0,
                std::numeric_limits<double>::infinity(), "demand");
  model.minimize({{open, 12.0}, {quantity, 0.5}});
  O::SolveOptions options;
  options.time_limit_seconds = 30;
  auto result = O::solve(model, options);
  std::cout << result.backend << ' ' << result.backend_version << ": "
            << O::to_string(result.termination) << '\n';
  if (!result.has_solution()) {
    std::cout << result.message << '\n';
    return 1;
  }
  std::cout << "open=" << result.value(open) << " quantity=" << result.value(quantity)
            << " objective=" << *result.objective << '\n';
  return result.termination == O::Termination::Optimal ? 0 : 1;
}
