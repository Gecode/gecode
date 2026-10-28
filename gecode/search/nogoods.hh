/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2013
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

#ifndef GECODE_SEARCH_NOGOODS_HH
#define GECODE_SEARCH_NOGOODS_HH

#include <gecode/search.hh>

namespace Gecode { namespace Search {

    /// Class for a sentinel no-good literal
    class GECODE_VTABLE_EXPORT NoNGL : public NGL {
    public:
        /// Constructor for creation
        NoNGL(void);
        /// Constructor for creation
        NoNGL(Space& home);
        /// Constructor for cloning \a ngl
        NoNGL(Space& home, NoNGL& ngl);
        /// Subscribe propagator \a p to all views of the no-good literal
        void subscribe(Space& home, Propagator& p) override;
        /// Schedule propagator \a p for all views of the no-good literal
        void reschedule(Space& home, Propagator& p) override;
        /// Cancel propagator \a p from all views of the no-good literal
        void cancel(Space& home, Propagator& p) override;
        /// Test the status of the no-good literal
        NGL::Status status(const Space& home) const override;
        /// Propagate the negation of the no-good literal
        ExecStatus prune(Space& home) override;
        /// Create copy
        NGL* copy(Space& home) override;
    };

    /// No-good propagator
    class GECODE_SEARCH_EXPORT NoGoodsProp : public Propagator {
    protected:
        /// Root of no-good literal tree
        NGL* root;
        /// Number of no-good literals with subscriptions
        unsigned int n;
        /// Constructor for creation
        NoGoodsProp(Space& home, NGL* root);
        /// Constructor for cloning \a p
        NoGoodsProp(Space& home, NoGoodsProp& p);

    public:
        /// Perform copying during cloning
        Actor* copy(Space& home) override;
        /// Const function (defined as low unary)
        PropCost cost(const Space& home, const ModEventDelta& med) const override;
        /// Schedule function
        void reschedule(Space& home) override;
        /// Perform propagation
        ExecStatus propagate(Space& home, const ModEventDelta& med) override;
        /// Post propagator for path \a p
        template <class Path>
        static ExecStatus post(Space& home, const Path& p);
        /// Delete propagator and return its size
        size_t dispose(Space& home) override;
    };

}} // namespace Gecode::Search

#include <gecode/search/nogoods.hpp>

#endif

// STATISTICS: search-other
