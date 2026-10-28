/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *     Mikael Zayenz Lagerkvist, 2009
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

#include <gecode/flatzinc/registry-internal.hh>
#include <gecode/minimodel.hh>

namespace Gecode { namespace FlatZinc { namespace RegistryDetail {

  void post_cumulatives(FlatZincSpace& s, const ConExpr& ce,
                        AST::Node* ann) {
    if (ce.size() == 6 || ce.size() == 7) {
      // The seven-argument interface supplies end variables explicitly.
      int offset = ce.size() == 7 ? 1 : 0;
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      IntVarArgs duration = s.arg2intvarargs(ce[1]);
      IntVarArgs resources = s.arg2intvarargs(ce[2+offset]);
      IntVarArgs machine = s.arg2intvarargs(ce[3+offset]);
      IntArgs bound = s.arg2intargs(ce[4+offset]);
      bool upper = ce[5+offset]->getBool();
      int n = start.size();

      IntVarArgs end;
      if (offset) {
        end = s.arg2intvarargs(ce[2]);
      } else {
        end = IntVarArgs(n);
        for (int i=n; i--;)
          end[i] = expr(s, start[i] + duration[i]);
      }

      if (duration.assigned()) {
        IntArgs durationI(n);
        for (int i = n; (i--) != 0;) {
          durationI[i] = duration[i].val();
        }
        if (machine.assigned()) {
          IntArgs machineI(n);
          for (int i = n; (i--) != 0;) {
            machineI[i] = machine[i].val();
          }
          if (resources.assigned()) {
            IntArgs resourcesI(n);
            for (int i = n; (i--) != 0;) {
              resourcesI[i] = resources[i].val();
            }
            cumulatives(s, machineI, start, durationI, end, resourcesI, bound, upper, s.ann2ipl(ann));
          } else {
            cumulatives(s, machineI, start, durationI, end, resources, bound, upper, s.ann2ipl(ann));
          }
        } else if (resources.assigned()) {
          IntArgs resourcesI(n);
          for (int i = n; (i--) != 0;) {
            resourcesI[i] = resources[i].val();
          }
          cumulatives(s, machine, start, durationI, end, resourcesI, bound, upper, s.ann2ipl(ann));
        } else {
          cumulatives(s, machine, start, durationI, end, resources, bound, upper, s.ann2ipl(ann));
        }
      } else {
        if (machine.assigned()) {
          IntArgs machineI(n);
          for (int i = n; (i--) != 0;) {
            machineI[i] = machine[i].val();
          }
          if (resources.assigned()) {
            IntArgs resourcesI(n);
            for (int i = n; (i--) != 0;) {
              resourcesI[i] = resources[i].val();
            }
            cumulatives(s, machineI, start, duration, end, resourcesI, bound, upper, s.ann2ipl(ann));
          } else {
            cumulatives(s, machineI, start, duration, end, resources, bound, upper, s.ann2ipl(ann));
          }
        } else if (resources.assigned()) {
          IntArgs resourcesI(n);
          for (int i = n; (i--) != 0;) {
            resourcesI[i] = resources[i].val();
          }
          cumulatives(s, machine, start, duration, end, resourcesI, bound, upper, s.ann2ipl(ann));
        } else {
          cumulatives(s, machine, start, duration, end, resources, bound, upper, s.ann2ipl(ann));
        }
      }
      return;
    }

    IntVarArgs start = s.arg2intvarargs(ce[0]);
    IntVarArgs duration = s.arg2intvarargs(ce[1]);
    IntVarArgs height = s.arg2intvarargs(ce[2]);
    int n = start.size();
    IntVar bound = s.arg2IntVar(ce[3]);

    if (n==0)
      return;

    if (n == 1) {
      rel(s, (duration[0] > 0) >> (height[0] <= bound));
      return;
    }

    bool nonzeroDuration = true;
    for (int i=0; i<n; i++) {
      if (duration[i].min() <= 0) {
        nonzeroDuration = false;
        break;
      }
    }

    int minHeight = std::min(height[0].min(),height[1].min());
    int minHeight2 = std::max(height[0].min(),height[1].min());
    for (int i=2; i<n; i++) {
      if (height[i].min() < minHeight) {
        minHeight2 = minHeight;
        minHeight = height[i].min();
      } else if (height[i].min() < minHeight2) {
        minHeight2 = height[i].min();
      }
    }
    bool disjunctive = nonzeroDuration && (
     (minHeight > bound.max()/2) ||
     (minHeight2 > bound.max()/2 && minHeight+minHeight2>bound.max()));
    if (disjunctive) {
      rel(s, bound >= max(height));
      // Unary
      if (duration.assigned()) {
        IntArgs durationI(n);
        for (int i=n; i--;)
          durationI[i] = duration[i].val();
        unshare(s,start);
        unary(s,start,durationI,s.ann2ipl(ann));
      } else {
        IntVarArgs end(n);
        for (int i=n; i--;)
          end[i] = expr(s,start[i]+duration[i]);
        unshare(s,start);
        unary(s,start,duration,end,s.ann2ipl(ann));
      }
    } else if (nonzeroDuration && height.assigned()) {
      IntArgs heightI(n);
      for (int i=n; i--;)
        heightI[i] = height[i].val();
      if (duration.assigned()) {
        IntArgs durationI(n);
        for (int i=n; i--;)
          durationI[i] = duration[i].val();
        cumulative(s, bound, start, durationI, heightI, s.ann2ipl(ann));
      } else {
        IntVarArgs end(n);
        for (int i = n; i--; )
          end[i] = expr(s,start[i]+duration[i]);
        cumulative(s, bound, start, duration, end, heightI, s.ann2ipl(ann));
      }
    } else if (nonzeroDuration && bound.assigned()) {
      IntArgs machine = IntArgs::create(n,0,0);
      IntArgs limit({bound.val()});
      IntVarArgs end(n);
      for (int i=n; i--;)
        end[i] = expr(s,start[i]+duration[i]);
      cumulatives(s, machine, start, duration, end, height, limit, true,
                  s.ann2ipl(ann));
    } else {
      int min = Gecode::Int::Limits::max;
      int max = Gecode::Int::Limits::min;
      IntVarArgs end(start.size());
      for (int i = start.size(); i--; ) {
        min = std::min(min, start[i].min());
        max = std::max(max, start[i].max() + duration[i].max());
        end[i] = expr(s, start[i] + duration[i]);
      }
      for (int time = min; time < max; ++time) {
        IntVarArgs x(start.size());
        for (int i = start.size(); i--; ) {
          IntVar overlaps = channel(s, expr(s, (start[i] <= time) &&
                                               (time < end[i])));
          x[i] = expr(s, overlaps * height[i]);
        }
        linear(s, x, IRT_LQ, bound);
      }
    }
  }

}}}

// STATISTICS: flatzinc-any
