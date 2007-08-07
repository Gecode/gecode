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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/config.icc"
#ifdef GECODE_HAVE_BOOST_SERIALIZATION

#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "gecode/serialization.hh"

namespace Gecode {

  void boostTextSerialization(std::ostream& out,
    const std::vector<Reflection::ActorSpec*>& actors) {
    boost::archive::text_oarchive oa(out);
    oa << actors;
  }

  void boostTextSerialization(std::istream& in,
    std::vector<Reflection::ActorSpec*>& actors) {
    boost::archive::text_iarchive ia(in);
    ia >> actors;
  }

}

#endif // GECODE_HAVE_BOOST_SERIALIZATION

// STATISTICS: serialization-any
