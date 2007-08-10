/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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

/*
 */

#ifndef __GECODE_INT_CUMULATIVES_HH__
#define __GECODE_INT_CUMULATIVES_HH__

#include "gecode/int.hh"

#include "gecode/support/shared-array.hh"
#include <vector>
#include <list>

namespace Gecode { namespace Int { namespace Cumulatives {

  /** \namespace Gecode::Int::Cumulatives
   *  \brief %Cumulatives propagators
   *
   * This namespace contains a propagator for the
   * cumulatives constraint as presented in
   \verbatim
   @inproceedings{DBLP:conf/cp/BeldiceanuC02,
     author    = {Nicolas Beldiceanu and Mats Carlsson},
     title     = {A New Multi-resource cumulatives Constraint
                  with Negative Heights.},
     booktitle = {CP},
     year      = {2002},
     pages     = {63-79},
     ee        = {http://link.springer.de/link/service/series/0558/
                  bibs/2470/24700063.htm},
     crossref  = {DBLP:conf/cp/2002},
     bibsource = {DBLP, http://dblp.uni-trier.de}
   }
   @proceedings{DBLP:conf/cp/2002,
     editor    = {Pascal Van Hentenryck},
     title     = {Principles and Practice of Constraint Programming -
                  CP 2002, 8th International Conference, CP 2002,
                  Ithaca, NY, USA, September 9-13, 2002, Proceedings},
     booktitle = {CP},
     publisher = {Springer},
     series    = {Lecture Notes in Computer Science},
     volume    = {2470},
     year      = {2002},
     isbn      = {3-540-44120-4},
     bibsource = {DBLP, http://dblp.uni-trier.de}
   }
   \endverbatim
   */


  /**
   * \brief %Propagator for the cumulatives constraint.
   *
   * This class implements Beldiceanu's and Carlsson's sweep-line
   * propagation algorithm for the cumulatives constraint.
   *
   * Requires \code #include "gecode/int/cumulatives.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class ViewM, class ViewD, class ViewH, class View>
  class Val : public Propagator {
    ViewArray<ViewM>  machine;
    ViewArray<View>   start;
    ViewArray<ViewD>  duration;
    ViewArray<View>   end;
    ViewArray<ViewH>  height;
    Support::SharedArray<int>  limit;
    const bool        at_most;

    Val(Space* home, bool share, Val<ViewM, ViewD, ViewH, View>& p);
    Val(Space* home, const ViewArray<ViewM>&, const ViewArray<View>&,
        const ViewArray<ViewD>&, const ViewArray<View>&,
        const ViewArray<ViewH>&, const IntArgs&, bool);

    ExecStatus prune(Space * home, int low, int up, int r,
                     int ntask, int sheight,
                     const std::vector<int>& contribution,
                     std::list<int>& prune_tasks);
  public:
    virtual size_t dispose(Space* home);
    virtual Actor*     copy(Space* home, bool share);
    virtual PropCost   cost(void) const;
    virtual ExecStatus propagate(Space* home);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static std::string name(void);
    static  ExecStatus post(Space* home, const ViewArray<ViewM>&,
                            const ViewArray<View>&, const ViewArray<ViewD>&,
                            const ViewArray<View>&, const ViewArray<ViewH>&,
                            const IntArgs&, bool);
  };


}}}

#include "gecode/int/cumulatives/val.icc"

#endif

// STATISTICS: int-prop

