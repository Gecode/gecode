#include <gecode/optimize/solve.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace O = Gecode::Optimize;

static std::string json(const std::string& value) {
  std::ostringstream out;
  out << '"';
  for (unsigned char c : value) {
    switch (c) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (c<32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c) << std::dec;
        else out << c;
    }
  }
  out << '"';
  return out.str();
}

static double number(const std::string& text) {
  std::size_t used=0;
  const double value=std::stod(text,&used);
  if (used!=text.size() || !std::isfinite(value) || value<0)
    throw std::invalid_argument("Option requires a nonnegative finite number");
  return value;
}

static void optional(const std::optional<double>& value) {
  if (value && std::isfinite(*value)) std::cout << *value;
  else std::cout << "null";
}

int main(int argc, char** argv) {
  const auto begin=std::chrono::steady_clock::now();
  const auto elapsed=[&] { return std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count(); };
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
  try {
    if (argc<2) throw std::invalid_argument("Usage: optimize-file MODEL.lp|MODEL.mps [--time-limit SECONDS] [--relative-gap GAP] [--absolute-gap GAP] [--seed N]");
    O::SolveOptions options;
    for (int arg=2; arg<argc; arg+=2) {
      if (arg+1>=argc) throw std::invalid_argument("Missing option value");
      const std::string key=argv[arg];
      const double value=number(argv[arg+1]);
      if (key=="--time-limit") options.time_limit_seconds=value;
      else if (key=="--relative-gap") options.relative_gap=value;
      else if (key=="--absolute-gap") options.absolute_gap=value;
      else if (key=="--seed" && std::floor(value)==value && value<=std::numeric_limits<int>::max())
        options.random_seed=static_cast<int>(value);
      else throw std::invalid_argument("Unknown or invalid option: "+key);
    }
    options.validate();
    auto model=O::read_model(argv[1]);
    const double import_seconds=elapsed();
    if (std::isfinite(options.time_limit_seconds))
      options.time_limit_seconds=std::max(0.0,options.time_limit_seconds-import_seconds);
    const auto result=O::solve(model,options);
    std::cout << "{\"schema_version\":1,\"model\":" << json(argv[1])
              << ",\"backend\":" << json(result.backend)
              << ",\"backend_version\":" << json(result.backend_version)
              << ",\"guarantee\":\"numerical\",\"termination\":" << json(O::to_string(result.termination))
              << ",\"message\":" << json(result.message)
              << ",\"solution_validated\":" << (result.has_solution()?"true":"false")
              << ",\"objective\":";
    optional(result.objective);
    std::cout << ",\"best_bound\":"; optional(result.best_bound);
    std::cout << ",\"absolute_gap\":"; optional(result.absolute_gap);
    std::cout << ",\"relative_gap\":"; optional(result.relative_gap);
    std::cout << ",\"native_backend_gap\":"; optional(result.native_backend_gap);
    std::cout << ",\"import_seconds\":" << import_seconds
              << ",\"solve_seconds\":" << result.elapsed_seconds
              << ",\"elapsed_seconds\":" << elapsed() << ",\"values\":[";
    for (std::size_t i=0; i<result.values.size(); ++i) {
      if (i) std::cout << ',';
      optional(result.values[i]);
    }
    std::cout << "],\"variable_names\":[";
    const auto snapshot=model.snapshot();
    for (std::size_t i=0; i<snapshot.variables.size(); ++i) {
      if (i) std::cout << ',';
      std::cout << json(snapshot.variables[i].name);
    }
    std::cout << "]}\n";
    if (result.termination==O::Termination::Optimal || result.termination==O::Termination::Infeasible
        || result.termination==O::Termination::Unbounded) return 0;
    if (result.termination==O::Termination::InvalidModel || result.termination==O::Termination::Unsupported
        || result.termination==O::Termination::BackendError || result.termination==O::Termination::NumericalFailure) return 2;
    return 1;
  } catch (const std::exception& error) {
    std::cout << "{\"schema_version\":1,\"termination\":\"input_error\",\"message\":" << json(error.what())
              << ",\"elapsed_seconds\":" << elapsed() << "}\n";
    return 2;
  }
}
