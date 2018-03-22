/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include <gecode/int.hh>

namespace Gecode { namespace Int {

  OutOfLimits::OutOfLimits(const char* l)
    : Exception(l,"Number out of limits") {}

  VariableEmptyDomain::VariableEmptyDomain(const char* l)
    : Exception(l,"Attempt to create variable with empty domain") {}

  NotZeroOne::NotZeroOne(const char* l)
    : Exception(l,"Not a zero/one integer value") {}

  TooFewArguments::TooFewArguments(const char* l)
    : Exception(l,"Passed argument array has too few elements") {}

  ArgumentSizeMismatch::ArgumentSizeMismatch(const char* l)
    : Exception(l,"Sizes of argument arrays mismatch") {}

  ArgumentSame::ArgumentSame(const char* l)
    : Exception(l,"Argument array contains same variable multiply") {}

  UnknownRelation::UnknownRelation(const char* l)
    : Exception(l,"Unknown relation type") {}

  UnknownOperation::UnknownOperation(const char* l)
    : Exception(l,"Unknown operation type") {}

  IllegalOperation::IllegalOperation(const char* l)
    : Exception(l,"Illegal operation type") {}

  UnknownBranching::UnknownBranching(const char* l)
    : Exception(l,"Unknown branching type") {}

  UnknownReifyMode::UnknownReifyMode(const char* l)
    : Exception(l,"Unknown reification mode") {}

  ValOfUnassignedVar::ValOfUnassignedVar(const char* l)
    : Exception(l,"Attempt to access value of unassigned variable") {}

  UninitializedTupleSet::UninitializedTupleSet(const char* l)
    : Exception(l,"Attempt to use uninitialized tuple set") {}

  NotYetFinalized::NotYetFinalized(const char* l)
    : Exception(l,"Tuple set not yet finalized") {}

  AlreadyFinalized::AlreadyFinalized(const char* l)
    : Exception(l,"Tuple set already finalized") {}

  LDSBUnbranchedVariable::LDSBUnbranchedVariable(const char* l)
    : Exception(l,"Variable in symmetry not branched on") {}

  LDSBBadValueSelection::LDSBBadValueSelection(const char* l)
    : Exception(l,"Value selection incomatible with LDSB") {}

}}

// STATISTICS: int-other

