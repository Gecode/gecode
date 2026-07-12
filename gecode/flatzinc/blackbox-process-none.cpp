/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
 */
#include <gecode/flatzinc/blackbox-process.hh>
#include <gecode/flatzinc.hh>

#if !defined(_WIN32) && !defined(GECODE_HAS_POSIX_BLACKBOX_EXEC)
namespace Gecode { namespace FlatZinc {

BlackBoxProcessSession*
create_blackbox_process(const std::string&, const std::vector<std::string>&) {
  throw Error("BlackBoxExec",
              "Persistent process blackboxes are not supported on this "
              "platform");
}

}}
#endif
