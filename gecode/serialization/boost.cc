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

#include "gecode/support.hh"
#ifdef GECODE_HAS_BOOST_SERIALIZATION

#include <fstream>

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#pragma GCC visibility push(default)
#endif

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#pragma GCC visibility pop
#endif

#include "gecode/serialization.hh"

namespace Gecode {

  void boostTextSerialization(std::ostream& out,
    const std::vector<Reflection::VarSpec>& vars,
    const std::vector<Reflection::ActorSpec>& actors) {
    boost::archive::text_oarchive oa(out);
    oa << vars;
    oa << actors;
  }

  void boostTextSerialization(std::istream& in,
    std::vector<Reflection::VarSpec>& vars,
    std::vector<Reflection::ActorSpec>& actors) {
    boost::archive::text_iarchive ia(in);
    ia >> vars;
    ia >> actors;
  }

}

#endif // GECODE_HAS_BOOST_SERIALIZATION

// STATISTICS: serialization-any
