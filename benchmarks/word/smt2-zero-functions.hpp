/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef GECODE_BENCHMARK_WORD_SMT2_ZERO_FUNCTIONS_HPP
#define GECODE_BENCHMARK_WORD_SMT2_ZERO_FUNCTIONS_HPP

#include <gecode/minimodel.hh>
#include <gecode/word.hh>
#include <stdexcept>
#include <vector>

namespace WordSmt2 {

  /// Finite interpretations of SMT Int's two underspecified zero-divisor
  /// functions. This construction-time registry need not survive Space clones:
  /// the posted congruence constraints and ordinary branchers do survive them.
  class IntegerZeroFunctions {
  public:
    enum Operation { DIVIDE, MODULO };
  private:
    struct Entry {
      Operation operation;
      Gecode::WordVar argument, result;
    };
    std::vector<Entry> entries;
  public:
    Gecode::WordExpr value(Gecode::Space& home, Operation operation,
                          const Gecode::WordExpr& argument,
                          Gecode::WordDomainType policy) {
      using namespace Gecode;
      if (home.failed()) return WordExpr(64,0);
      WordVar a=argument.post(home,policy);
      for (const Entry& entry:entries)
        if (entry.operation == operation &&
            (entry.argument.varimp() == a.varimp() ||
             (entry.argument.assigned() && a.assigned() &&
              entry.argument.val() == a.val())))
          return WordExpr(entry.result);
      // Ackermann constraints are quadratic in the number of distinct calls.
      // Refuse excessive construction rather than silently weaken congruence.
      if (entries.size() >= 256)
        throw std::runtime_error("reader zero-divisor function limit is 256");
      WordVar result(home,64,policy);
      for (const Entry& entry:entries) {
        if (entry.operation != operation) continue;
        if (entry.argument.assigned() && a.assigned() &&
            entry.argument.val() != a.val()) continue;
        rel(home,!word_rel(WordExpr(a),WRT_EQ,WordExpr(entry.argument),policy) ||
                 word_rel(WordExpr(result),WRT_EQ,WordExpr(entry.result),policy));
      }
      entries.push_back(Entry{operation,a,result});
      return WordExpr(result);
    }

    /// Append these variables to the reader's ordinary input branchers.
    /// Connected free function results must be assigned before reporting sat.
    Gecode::WordVarArgs variables(void) const {
      Gecode::WordVarArgs result;
      for (const Entry& entry:entries) result << entry.result;
      return result;
    }
  };

}

#endif
