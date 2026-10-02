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

// Knapsack-augmented overload checking from Cloutier and Quimper,
// "Augmenting the Cumulative Overload Check with Integral Resource Usage
// Reasoning", CP 2026, https://doi.org/10.4230/LIPIcs.CP.2026.13.
// Task-count and two-word capacity limits bound the quadratic work and
// transient Region storage; unsupported shapes retain existing propagation.

namespace Gecode { namespace Int { namespace Cumulative {

  namespace Kaoc {

    struct Bits {
      unsigned long long low;
      unsigned long long high;
    };

    forceinline void
    add(Bits& bits, int demand, int capacity) {
      unsigned long long shifted_low = 0ULL;
      unsigned long long shifted_high = 0ULL;
      if (demand < 64) {
        shifted_low = bits.low << demand;
        shifted_high = bits.high << demand;
        if (demand != 0)
          shifted_high |= bits.low >> (64-demand);
      } else if (demand < 128) {
        shifted_high = bits.low << (demand-64);
      }
      bits.low |= shifted_low;
      bits.high |= shifted_high;
      if (capacity < 63)
        bits.low &= (1ULL << (capacity+1))-1ULL;
      if (capacity < 64) {
        bits.high = 0ULL;
      } else if (capacity < 127) {
        bits.high &= (1ULL << (capacity-63))-1ULL;
      }
    }

    forceinline int
    most_significant(unsigned long long word) {
      assert(word != 0ULL);
      int bit = 0;
      if (word >> 32) { word >>= 32; bit += 32; }
      if (word >> 16) { word >>= 16; bit += 16; }
      if (word >> 8)  { word >>= 8;  bit += 8; }
      if (word >> 4)  { word >>= 4;  bit += 4; }
      if (word >> 2)  { word >>= 2;  bit += 2; }
      if (word >> 1)  {             bit += 1; }
      return bit;
    }

    forceinline unsigned long long
    through(int bit) {
      assert((bit >= 0) && (bit < 64));
      return bit == 63 ? ~0ULL : (1ULL << (bit+1))-1ULL;
    }

    forceinline int
    available(const Bits& bits, int residual) {
      if (residual <= 0)
        return 0;
      residual = (std::min)(residual,127);
      if (residual >= 64) {
        const int high_bit = residual-64;
        const unsigned long long high = bits.high & through(high_bit);
        if (high != 0ULL)
          return 64+most_significant(high);
      }
      const int low_bit = (std::min)(residual,63);
      const unsigned long long low = bits.low & through(low_bit);
      return low == 0ULL ? 0 : most_significant(low);
    }

    template<class Tasks>
    bool
    conflict(Tasks& tasks, int capacity) {
      const int n = tasks.size();
      if ((n < 4) || (n > 64) ||
          (capacity < 2) || (capacity > 127))
        return false;
      bool non_unit = false;
      for (int i=0; i<n; i++) {
        if ((tasks[i].c() <= 0) || (tasks[i].c() > capacity))
          return false;
        non_unit = non_unit || (tasks[i].c() != 1);
      }
      if (!non_unit)
        return false;

      Region region;
      int* order = region.alloc<int>(n);
      int* times = region.alloc<int>(4*n);
      for (int i=0; i<n; i++) {
        order[i] = i;
        times[4*i] = tasks[i].est();
        times[4*i+1] = tasks[i].ect();
        times[4*i+2] = tasks[i].lst();
        times[4*i+3] = tasks[i].lct();
      }
      std::sort(order,order+n,[&](int left, int right) {
        if (tasks[left].lct() != tasks[right].lct())
          return tasks[left].lct() < tasks[right].lct();
        return left < right;
      });
      std::sort(times,times+4*n);
      int events = 0;
      for (int i=0; i<4*n; i++)
        if ((events == 0) || (times[i] != times[events-1]))
          times[events++] = times[i];
      if (events < 2)
        return false;

      int* fixed = region.alloc<int>(events);
      int* required = region.alloc<int>(events);
      Bits* bits = region.alloc<Bits>(events);
      for (int point=0; point<events; point++) {
        fixed[point] = 0;
        required[point] = 0;
        bits[point].low = 1ULL;
        bits[point].high = 0ULL;
        for (int task=0; task<n; task++)
          if ((tasks[task].lst() <= times[point]) &&
              (times[point] < tasks[task].ect()))
            fixed[point] += tasks[task].c();
        if (fixed[point] > capacity)
          return true;
      }

      int earliest = tasks[order[0]].est();
      for (int prefix=0; prefix<n; prefix++) {
        const int task = order[prefix];
        earliest = (std::min)(earliest,tasks[task].est());
        for (int point=0; point<events; point++) {
          const int time = times[point];
          const bool compulsory =
            (tasks[task].lst() <= time) && (time < tasks[task].ect());
          if ((tasks[task].est() <= time) &&
              (time < tasks[task].lct()) && !compulsory)
            add(bits[point],tasks[task].c(),capacity);
          if ((tasks[task].est() <= time) &&
              (time < (std::min)(tasks[task].ect(),tasks[task].lst())))
            required[point] += tasks[task].c();
        }

        const int finish = tasks[task].lct();
        long long overflow = 0;
        for (int point=0; point+1<events; point++) {
          const int left = times[point];
          if ((left < earliest) || (left >= finish))
            continue;
          const int right = (std::min)(times[point+1],finish);
          const int usable = available(bits[point],capacity-fixed[point]);
          overflow = (std::max)(0LL,overflow+
            (static_cast<long long>(right)-left) *
            static_cast<long long>(required[point]-usable));
        }
        if (overflow > 0)
          return true;
      }
      return false;
    }

  }

  template<class Task>
  forceinline ExecStatus
  supported_knapsack_overload(TaskArray<Task>& tasks, int capacity) {
    return Kaoc::conflict(tasks,capacity) ? ES_FAILED : ES_OK;
  }

  forceinline ExecStatus
  knapsack_overload(Space&, int c, TaskArray<ManFixPTask>& t) {
    return supported_knapsack_overload(t,c);
  }
  forceinline ExecStatus
  knapsack_overload(Space&, int c, TaskArray<ManFixPSETask>& t) {
    return supported_knapsack_overload(t,c);
  }
  forceinline ExecStatus
  knapsack_overload(Space&, int c, TaskArray<OptFixPTask>& t) {
    return supported_knapsack_overload(t,c);
  }
  forceinline ExecStatus
  knapsack_overload(Space&, int c, TaskArray<OptFixPSETask>& t) {
    return supported_knapsack_overload(t,c);
  }

}}}

// STATISTICS: int-prop
