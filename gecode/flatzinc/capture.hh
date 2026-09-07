/* Owning FlatZinc parser records. No Optimize or solver-state dependencies. */
#ifndef GECODE_FLATZINC_CAPTURE_HH
#define GECODE_FLATZINC_CAPTURE_HH

#include <gecode/support/config.hpp>
#include <gecode/flatzinc/capture-records.hh>
#include <istream>

// Same shared-library boundary as the native FlatZinc entry points, without
// including native Space/AST implementation headers in this record API.
#if !defined(GECODE_STATIC_LIBS) && (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))
# ifdef GECODE_BUILD_FLATZINC
#  define GECODE_CAPTURE_EXPORT __declspec(dllexport)
# else
#  define GECODE_CAPTURE_EXPORT __declspec(dllimport)
# endif
#elif defined(GECODE_GCC_HAS_CLASS_VISIBILITY)
# define GECODE_CAPTURE_EXPORT __attribute__((visibility("default")))
#else
# define GECODE_CAPTURE_EXPORT
#endif

namespace Gecode { namespace FlatZinc { namespace Capture {

/** Capture without constructing a native Space or invoking any registry poster.
 * Input size and nesting are bounded before parsing; declared bulk allocation
 * and captured record counts have explicit limits. No solver is called.
 * Allocation failure may throw std::bad_alloc. All other input failures have
 * a typed Result and never publish partial records.
 */
GECODE_CAPTURE_EXPORT Result parse(std::istream& input, const Options& options = {});
GECODE_CAPTURE_EXPORT Result parse_string(const std::string& input, const Options& options = {});

}}}
#undef GECODE_CAPTURE_EXPORT
#endif
