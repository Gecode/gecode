/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#ifndef GECODE_SEARCH_SUPPORT_HH
#define GECODE_SEARCH_SUPPORT_HH

#include <gecode/search.hh>

namespace Gecode::Search {

    /// Clone space \a s depending on options \a o
    forceinline Space* snapshot(Space* s, const Options& o);

    /// Virtualize a worker to an engine
    template <class Worker>
    class WorkerToEngine : public Engine {
    protected:
        /// The worker to wrap into an engine
        Worker w;

    public:
        /// Initialization
        WorkerToEngine(Space* s, const Options& o);
        /// Return next solution (nullptr, if none exists or search has been stopped)
        Space* next() override;
        /// Return statistics
        Search::Statistics statistics() const override;
        /// Check whether engine has been stopped
        bool stopped() const override;
        /// Constrain future solutions to be better than \a b
        void constrain(const Space& b) override;
        /// Reset engine to restart at space \a s
        void reset(Space* s) override;
        /// Return no-goods
        NoGoods& nogoods() override;
    };

    forceinline Space*
    snapshot(Space* s, const Options& o) {
        return o.clone ? s->clone() : s;
    }

    template <class Worker>
    WorkerToEngine<Worker>::WorkerToEngine(Space* s, const Options& o) : Engine(o, 1U), w(s, o) {}
    template <class Worker>
    Space*
    WorkerToEngine<Worker>::next() {
        return w.next();
    }
    template <class Worker>
    Search::Statistics
    WorkerToEngine<Worker>::statistics() const {
        return w.statistics();
    }
    template <class Worker>
    bool
    WorkerToEngine<Worker>::stopped() const {
        return w.stopped();
    }
    template <class Worker>
    void
    WorkerToEngine<Worker>::reset(Space* s) {
        w.reset(s);
    }
    template <class Worker>
    void
    WorkerToEngine<Worker>::constrain(const Space& b) {
        w.constrain(b);
    }
    template <class Worker>
    NoGoods&
    WorkerToEngine<Worker>::nogoods() {
        return w.nogoods();
    }

} // namespace Gecode::Search

#endif

// STATISTICS: search-other
