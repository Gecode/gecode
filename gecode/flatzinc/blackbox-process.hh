/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
 */
#ifndef GECODE_FLATZINC_BLACKBOX_PROCESS_HH
#define GECODE_FLATZINC_BLACKBOX_PROCESS_HH

#include <string>
#include <vector>

#ifdef GECODE_HAS_THREADS
#include <thread>
#endif

namespace Gecode::FlatZinc {

    /// Platform process session used by the executable blackbox backend.
    class BlackBoxProcessSession {
    protected:
#ifdef GECODE_HAS_THREADS
        std::thread::id owner;
#endif
        // NOLINTNEXTLINE(modernize-use-equals-default)
        BlackBoxProcessSession()
#ifdef GECODE_HAS_THREADS
            : owner(std::this_thread::get_id())
#endif
        {
        }

    public:
        virtual ~BlackBoxProcessSession() = default;
        bool
        owned_by_current_thread() const {
#ifdef GECODE_HAS_THREADS
            return owner == std::this_thread::get_id();
#else
            return true;
#endif
        }
        virtual std::string exchange(const std::string& request) = 0;
    };

    /// Create the process implementation selected for the target platform.
    BlackBoxProcessSession* create_blackbox_process(const std::string& program, const std::vector<std::string>& args);

} // namespace Gecode::FlatZinc

#endif

// STATISTICS: flatzinc-other
