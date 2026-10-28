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

#ifndef GECODE_FLATZINC_REGISTRY_INTERNAL_HH
#define GECODE_FLATZINC_REGISTRY_INTERNAL_HH

#include <gecode/flatzinc/registry.hh>

namespace Gecode { namespace FlatZinc { namespace RegistryDetail {

  void post_cumulatives(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann);

  void post_int_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann);
  void post_int_in_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann);

#ifdef GECODE_HAS_SET_VARS
  void register_set(Registry& r);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
  void register_float(Registry& r);
#endif

}}}

#endif

// STATISTICS: flatzinc-any
