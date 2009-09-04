/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2006-12-11 03:27:31 +1100 (Mon, 11 Dec 2006) $ by $Author: schulte $
 *     $Revision: 4024 $
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

#ifndef __FLATZINC_VARSPEC__HH__
#define __FLATZINC_VARSPEC__HH__

#include <gecode/flatzinc/option.hh>
#include <gecode/flatzinc/ast.hh>
#include <vector>
#include <iostream>

namespace Gecode { namespace FlatZinc {

  class Equal {
  public:
    const int v;
    Equal(int v0) : v(v0) {}
  };

  class VarSpec {
  public:
    bool introduced;
    VarSpec(bool introduced0) : introduced(introduced0) {}
    virtual ~VarSpec(void) {}
    virtual void print(void) = 0;
  };

  class IntVarSpec : public VarSpec {
  public:
    bool alias;
    bool singleton;
    int i;
    Option<AST::SetLit* > domain;
    IntVarSpec(const Option<AST::SetLit* >& d, bool introduced)
    : VarSpec(introduced), alias(false), singleton(false), domain(d) {}
    IntVarSpec(int i0, bool introduced) : VarSpec(introduced),
      alias(false), singleton(true), i(i0) {}
    IntVarSpec(const Equal& eq, bool introduced) :
      VarSpec(introduced), alias(true), i(eq.v) {}
    virtual void print(void) {
      std::cout << "i(";
      if (alias) {
        std::cout << "-> " << i;
      } else if (singleton) {
        std::cout << i;
      }
      else if (domain()) {
        AST::SetLit* sl = domain.some();
        if (sl->interval) {
          std::cout << sl->min << ".." << sl->max;
        } else {
          for (unsigned int i=0; i<sl->s.size(); i++)
            std::cout << sl->s[i] << ", ";
        }
      } else {
        std::cout << "..";
      }
      std::cout << ")";
    }
    ~IntVarSpec(void) {
      if (!alias && !singleton && domain())
        delete domain.some();
    }
  };

  class BoolVarSpec : public VarSpec {
  public:
    bool alias;
    bool singleton;
    int i;
    Option<AST::SetLit* > domain;
    BoolVarSpec(Option<AST::SetLit* >& d, bool introduced)
    : VarSpec(introduced), alias(false), singleton(false), domain(d) {}
    BoolVarSpec(bool b, bool introduced)
    : VarSpec(introduced), alias(false), singleton(true), i(b) {}
    BoolVarSpec(const Equal& eq, bool introduced)
    : VarSpec(introduced), alias(true), i(eq.v) {}
    virtual void print(void) {
      std::cout << "b(";
      if (alias) {
        std::cout << "-> " << i;
      } else if (singleton) {
        std::cout << (i ? "true" : "false");
      }
      else if (domain()) {
        AST::SetLit* sl = domain.some();
        if (sl->interval) {
          std::cout << sl->min << ".." << sl->max;
        } else {
          for (unsigned int i=0; i<sl->s.size(); i++)
            std::cout << (sl->s[i] ? "true" : "false") << ", ";
        }
      } else {
        std::cout << "..";
      }
      std::cout << ")";
    }
    ~BoolVarSpec(void) {
      if (!alias && !singleton && domain())
        delete domain.some();
    }
  };

  class FloatVarSpec : public VarSpec {
  public:
    bool alias;
    bool singleton;
    double i;
    Option<std::vector<double>* > domain;
    FloatVarSpec(Option<std::vector<double>* >& d, bool introduced)
    : VarSpec(introduced), alias(false), singleton(false), domain(d) {}
    FloatVarSpec(bool b, bool introduced)
    : VarSpec(introduced), alias(false), singleton(true), i(b) {}
    FloatVarSpec(const Equal& eq, bool introduced)
    : VarSpec(introduced), alias(true), i(eq.v) {}
    virtual void print(void) {}
    ~FloatVarSpec(void) {
      if (!alias && !singleton && domain())
        delete domain.some();
    }
  };

  class SetVarSpec : public VarSpec {
  public:
    bool alias;
    bool singleton;
    int i;
    Option<AST::SetLit*> upperBound;
    SetVarSpec(bool introduced)
    : VarSpec(introduced),alias(false), singleton(false),
      upperBound(Option<AST::SetLit* >::none()) {}
    SetVarSpec(const Option<AST::SetLit* >& v, bool introduced)
    : VarSpec(introduced), alias(false), singleton(false), upperBound(v) {}
    SetVarSpec(AST::SetLit* v, bool introduced)
    : VarSpec(introduced), alias(false), singleton(true), 
      upperBound(Option<AST::SetLit*>::some(v)) {}
    SetVarSpec(const Equal& eq, bool introduced)
    : VarSpec(introduced), alias(true), i(eq.v) {}
    virtual void print(void) {}
    ~SetVarSpec(void) {
      if (!alias && upperBound())
        delete upperBound.some();
    }
  };

}}
#endif

// STATISTICS: flatzinc-any
