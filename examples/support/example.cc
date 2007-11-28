/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
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

#include "examples/support.hh"

double
am(double t[], int n) {
  if (n < 1)
    return 0.0;
  double s = 0;
  for (int i=n; i--; )
    s += t[i];
  return s / n;
}

double
dev(double t[], int n) {
  if (n < 2)
    return 0.0;
  double m = am(t,n);
  double s = 0.0;
  for (int i=n; i--; ) {
    double d = t[i]-m;
    s += d*d;
  }
  return sqrt(s / (n-1)) / m;
}

void
Example::sac_collect_vars() {
  // Collect variables in VarMap
  Reflection::VarMap vm;
  for (Reflection::SpecIter si = actorSpecs(vm); si(); ++si) {}
  Support::Symbol vti_bool("VTI_BOOL");
  Support::Symbol vti_int("VTI_INT");
  for (Reflection::VarMapIter vmi(vm); vmi(); ++vmi) {
    if (vmi.spec().vti() == vti_bool) {
      BoolVar bv(Int::BoolView(static_cast<Int::BoolVarImp*>(vmi.var())));
      _sac_bva.add(this, bv);
    } else if (vmi.spec().vti() == vti_int) {
      IntVar iv(Int::IntView(static_cast<Int::IntVarImp*>(vmi.var())));
      _sac_iva.add(this, iv);
    }
  }
}

void
Example::sac_remove_vars() {
  // ToDo: Empty _sac_iva and _sac_bva since they are no longer needed.
}

bool
Example::sac(unsigned long int& p) {
  if (status(p) == SS_FAILED) return false;

  bool modified = false;
  for (int i = _sac_bva.size(); i--; ) {
    for (int val = 0; val <= 1; ++val) {
      if (_sac_bva[i].assigned()) break;
      Example* e = static_cast<Example*>(this->clone());
      rel(e, e->_sac_bva[i], IRT_EQ, val);
      if (e->status(p) == SS_FAILED) {
        modified = true;
        rel(this, this->_sac_bva[i], IRT_NQ, val);
        if (status(p) == SS_FAILED)
          return false;
      }
      delete e;
    }
  }

  for (int i = _sac_iva.size(); i--; ) {
    if (_sac_iva[i].assigned()) continue;
    IntArgs nq(_sac_iva[i].size());
    int nnq = 0;
    IntVarValues vv(_sac_iva[i]);
    while (vv()) {
      Example* e = static_cast<Example*>(this->clone());
      rel(e, e->_sac_iva[i], IRT_EQ, vv.val());
      if (e->status() == SS_FAILED) {
        nq[nnq++] = vv.val();
      }
      delete e;
      ++vv;
    }
    if (nnq > 0) modified = true;
    for (; nnq--; ) {
      rel(this, _sac_iva[i], IRT_NQ, nq[nnq]);
    }
    if (status(p) == SS_FAILED)
      return false;
  }

  return modified;
}

// STATISTICS: example-any
