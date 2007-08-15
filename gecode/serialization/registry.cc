/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include "gecode/serialization.hh"

#include "gecode/int.hh"
#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

#include "gecode/int/element.hh"

namespace Gecode { namespace Serialization {  

  namespace {

    class ArrayArgIter {
    private:
      Reflection::IntArrayArg& a;
      int n;
    public:
      ArrayArgIter(Reflection::IntArrayArg* a0) : a(*a0), n(0) {}
      bool operator()(void) { return n < a.size(); }
      void operator++(void) { n += 2; }
      int min(void) const { return a[n]; }
      int max(void) const { return a[n+1]; }
      unsigned int width(void) const { return max() - min() + 1; }
    };
    
    VarBase* createIntVar(Space* home, Reflection::VarSpec& spec) {
      ArrayArgIter ai(spec.dom()->toIntArray());
      return Int::IntView(IntVar(home, IntSet(ai))).variable();
    }
    VarBase* createBoolVar(Space* home, Reflection::VarSpec& spec) {
      return NULL;
    }
#ifdef GECODE_HAVE_SET_VARS
    VarBase* createSetVar(Space* home, Reflection::VarSpec& spec) {
      return NULL;
    }
#endif

    class VariableCreators {
    public:
        VariableCreators() {
        Reflection::registry.add(VTI_INT, createIntVar);
        Reflection::registry.add(VTI_BOOL, createBoolVar);
#ifdef GECODE_HAVE_SET_VARS
        Reflection::registry.add(VTI_SET, createSetVar);
#endif
      }
    };
    VariableCreators __variableCreators;
  }
    
}}

// STATISTICS: serialization-any
