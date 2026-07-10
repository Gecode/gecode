/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Nathan Tran <kieron.qtran@gmail.com>
 *
 *  Copyright:
 *     Jip J. Dekker, 2023
 *     Nathan Tran, 2025
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

#include "test/flatzinc.hh"

#include "gecode/flatzinc/restart-random.hpp"

namespace Test { namespace FlatZinc {

  namespace {
    /// Check that reversed uniform integer ranges fail during parsing
    class UniformIntInvalidRange : public Base {
    public:
      /// Create and register test
      UniformIntInvalidRange(void)
        : Base("FlatZinc::on_restart::uniform_int_invalid_range") {}
      /// Perform test
      virtual bool run(void) {
        std::stringstream source(R"FZN(
predicate gecode_on_restart_uniform_int(int: low,int: high,var int: out);
var 0..1: y;
constraint gecode_on_restart_uniform_int(1,0,y);
solve satisfy;
)FZN");
        Gecode::FlatZinc::Printer p;
        std::ostringstream err;
        Gecode::FlatZinc::FlatZincSpace* fg =
          Gecode::FlatZinc::parse(source, p, err);
        const std::string message = err.str();
        return fg == nullptr &&
          message.find("gecode_on_restart_uniform_int") != std::string::npos &&
          message.find("low (1) <= high (0)") != std::string::npos;
      }
    };

    /// Scripted generator for testing wide restart integer sampling
    class ScriptedRandom {
    private:
      const unsigned int* values;
      unsigned int size;
      unsigned int next;
    public:
      /// Initialize with the values to return
      ScriptedRandom(const unsigned int* values0, unsigned int size0)
        : values(values0), size(size0), next(0) {}
      /// Return the next scripted 31-bit chunk
      unsigned int operator ()(unsigned int n) {
        if ((n != (1U << 31)) || (next >= size))
          return 0;
        return values[next++];
      }
      /// Stub for the narrow-path overload
      unsigned long long int operator ()(unsigned long long int) {
        return 0;
      }
      /// Return whether all scripted chunks were consumed
      bool done(void) const {
        return next == size;
      }
    };

    /// Test wide restart integer range endpoints deterministically
    class WideUniformInt : public Base {
    public:
      /// Create and register test
      WideUniformInt(void)
        : Base("FlatZinc::on_restart::uniform_int_wide_endpoints") {}
      /// Perform test
      virtual bool run(void) {
        {
          const unsigned int chunks[] = {
            (1U << 31) - 1U, (1U << 31) - 1U, 1U, 0U
          };
          ScriptedRandom random(chunks, 4);
          const unsigned long long int width = (1ULL << 31) + 1ULL;
          const unsigned long long int offset =
            Gecode::FlatZinc::Internal::uniform_int_offset(random,width);
          if ((offset != width - 1ULL) ||
              (static_cast<long long int>(INT_MIN) +
               static_cast<long long int>(offset) != 0) || !random.done())
            return false;
        }
        {
          const unsigned int chunks[] = {1U, (1U << 31) - 1U};
          ScriptedRandom random(chunks, 2);
          const unsigned long long int width = 1ULL << 32;
          const unsigned long long int offset =
            Gecode::FlatZinc::Internal::uniform_int_offset(random,width);
          if ((offset != width - 1ULL) ||
              (static_cast<long long int>(INT_MIN) +
               static_cast<long long int>(offset) != INT_MAX) || !random.done())
            return false;
        }
        return true;
      }
    };

    /// Helper class to create and register tests
    class Create {
    public:

      /// Perform creation and registration
      Create(void) {
        (void) new FlatZincTest("on_restart::last_val_int",
R"FZN(
predicate gecode_on_restart_status(var int: s);
predicate gecode_on_restart_last_val_int(var int: input,var int: out);
predicate int_lin_eq_imp(array [int] of int: as,array [int] of var int: bs,int: c,var bool: r);
predicate int_eq_imp(var int: a,var int: b,var bool: r);
array [1..2] of int: X_INTRODUCED_15_ = [1,-1];
var -1..4: x:: output_var;
var int: y:: output_var;
var 0..4: X_INTRODUCED_1_ ::var_is_introduced :: is_defined_var;
var 1..5: X_INTRODUCED_3_ ::var_is_introduced ;
var bool: X_INTRODUCED_4_ ::var_is_introduced :: is_defined_var;
var int: X_INTRODUCED_6_ ::var_is_introduced ;
var bool: X_INTRODUCED_16_ ::var_is_introduced :: is_defined_var;
array [1..1] of var int: X_INTRODUCED_2_ ::var_is_introduced  = [x];
constraint int_abs(x,X_INTRODUCED_1_):: ctx_pos:: defines_var(X_INTRODUCED_1_);
constraint gecode_on_restart_status(X_INTRODUCED_3_);
constraint gecode_on_restart_last_val_int(y,X_INTRODUCED_6_);
constraint bool_clause([X_INTRODUCED_4_,X_INTRODUCED_16_],[]);
constraint int_eq_imp(y,0,X_INTRODUCED_4_);
constraint int_eq_reif(X_INTRODUCED_3_,1,X_INTRODUCED_4_):: defines_var(X_INTRODUCED_4_);
constraint int_lin_eq_imp(X_INTRODUCED_15_,[y,X_INTRODUCED_6_],1,X_INTRODUCED_16_):: defines_var(X_INTRODUCED_16_);
solve :: int_search(X_INTRODUCED_2_,input_order,indomain_min,complete) maximize X_INTRODUCED_1_;
)FZN",
R"OUT(x = -1;
y = 0;
----------
x = 2;
y = 1;
----------
x = 3;
y = 2;
----------
x = 4;
y = 3;
----------
==========
)OUT", true, {"--restart", "constant", "--restart-base", "100"});

        (void) new FlatZincTest("on_restart::uniform_int_upper_bound",
R"FZN(
predicate gecode_on_restart_uniform_int(int: low,int: high,var int: out);
var 0..1: y:: output_var;
constraint gecode_on_restart_uniform_int(0,1,y);
constraint int_eq(y,1);
solve satisfy;
)FZN",
R"OUT(y = 1;
----------
)OUT", true, {"--restart", "constant", "--restart-base", "100", "--seed", "2"});
      }
    };

    Create c;
    UniformIntInvalidRange invalid_range;
    WideUniformInt w;
  }

}}

// STATISTICS: test-flatzinc
