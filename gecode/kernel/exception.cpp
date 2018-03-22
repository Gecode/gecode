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

#include <gecode/kernel.hh>

namespace Gecode {

  SpaceFailed::SpaceFailed(const char* l)
    : Exception(l,"Attempt to invoke operation on failed space") {}

  SpaceNotStable::SpaceNotStable(const char* l)
    : Exception(l,"Attempt to invoke operation on not stable space") {}

  SpaceNotCloned::SpaceNotCloned(const char* l)
    : Exception(l,"Copy constructor of space did not call base class copy constructor") {}

  SpaceNoBrancher::SpaceNoBrancher(const char* l)
    : Exception(l,"Attempt to commit with no brancher") {}

  SpaceIllegalAlternative::SpaceIllegalAlternative(const char* l)
    : Exception(l,"Attempt to commit with illegal alternative") {}

  TooManyGroups::TooManyGroups(const char* l)
    : Exception(l,"Too many groups created") {}

  UnknownPropagator::UnknownPropagator(const char* l)
    : Exception(l,"Unknown propagator (illegal id)") {}

  TooManyBranchers::TooManyBranchers(const char* l)
    : Exception(l,"Too many branchers created") {}

  UnknownBrancher::UnknownBrancher(const char* l)
    : Exception(l,"Unknown brancher (illegal id)") {}

  UninitializedAFC::UninitializedAFC(const char* l)
    : Exception(l,"Uninitialized AFC information for branching") {}

  UninitializedAction::UninitializedAction(const char* l)
    : Exception(l,"Uninitialized action information for branching") {}

  UninitializedCHB::UninitializedCHB(const char* l)
    : Exception(l,"Uninitialized CHB information for branching") {}

  UninitializedRnd::UninitializedRnd(const char* l)
    : Exception(l,"Uninitialized random generator for branching") {}

  IllegalDecay::IllegalDecay(const char* l)
    : Exception(l,"Illegal decay factor") {}

  InvalidFunction::InvalidFunction(const char* l)
    : Exception(l,"Invalid function") {}

  MoreThanOneTracer::MoreThanOneTracer(const char* l)
    : Exception(l,"Attempt create more than one non-variable tracer") {}

}

// STATISTICS: kernel-other
