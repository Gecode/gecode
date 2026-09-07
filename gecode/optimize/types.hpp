/* Shared types for the additive optimization API. */
#ifndef GECODE_OPTIMIZE_TYPES_HPP
#define GECODE_OPTIMIZE_TYPES_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

namespace Gecode { namespace Optimize {

using ModelId = std::uint64_t;
using Revision = std::uint64_t;
enum class VariableType { Continuous, Integer, Binary, SemiContinuous, SemiInteger };
enum class ObjectiveSense { Minimize, Maximize };
enum class Guarantee { Numerical, Exact, Certified };
enum class Backend { Auto, Highs, Native };

struct Variable {
  ModelId model_id = 0;
  std::uint64_t id = 0;
  bool operator==(const Variable& other) const noexcept {
    return model_id == other.model_id && id == other.id;
  }
  bool operator!=(const Variable& other) const noexcept { return !(*this == other); }
};
struct Constraint {
  ModelId model_id = 0;
  std::uint64_t id = 0;
};
class ModelError : public std::invalid_argument {
public:
  explicit ModelError(const std::string& message) : std::invalid_argument(message) {}
};

}}
#endif
