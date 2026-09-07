// Private native storage admission, shared only with its arithmetic tests.
#ifndef GECODE_OPTIMIZE_NATIVE_REGULAR_LIMITS_HPP
#define GECODE_OPTIMIZE_NATIVE_REGULAR_LIMITS_HPP
#include <cstdint>
#include <limits>
namespace Gecode { namespace Optimize { namespace Detail {
inline const char* native_regular_size_error(std::uint64_t words,std::uint64_t states,
                                             std::uint64_t transitions,std::uint64_t symbols,
                                             bool short_symbols) noexcept {
  constexpr auto imax=static_cast<std::uint64_t>(std::numeric_limits<int>::max());
  constexpr auto umax=static_cast<std::uint64_t>(std::numeric_limits<unsigned int>::max());
  // DFA init and layer allocation both need signed-int count+1 sentinels.
  if(words>=imax||!states||states>=imax||transitions>=imax)
    return "Native regular word/state/transition count exceeds signed array limits";
  // LayeredGraph::initialize initializes int(max_states)*(word_length+1).
  if(states>imax/(words+1))
    return "Native regular layer-state product exceeds signed integer limits";
  if(words&&transitions>umax/words)
    return "Native regular layer-edge total exceeds unsigned integer limits";
  // DFA::fill allocates 1<<n_log, the power of two strictly above symbols.
  if(symbols>=((imax/2)+1))
    return "Native regular alphabet hash exceeds positive signed power-of-two limits";
  // The short-valued graph stores its per-layer support count in ushort.
  if(short_symbols&&symbols>std::numeric_limits<unsigned short>::max())
    return "Native regular short-symbol support count exceeds native storage";
  return nullptr;
}
}}}
#endif
