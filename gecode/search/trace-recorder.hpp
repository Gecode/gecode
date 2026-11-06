/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

namespace Gecode::Search {

    /// Recorder for engine events (for access control)
    class WrapTraceRecorder {
    public:
        /// Register engine
        static void engine(SearchTracer* tracer, SearchTracer::EngineType t, unsigned int n);
    };

    /// Simple recorder for a search tracer
    class TraceRecorder {
    protected:
        /// The actual tracer
        SearchTracer& tracer;
        /// The engine id
        unsigned int _eid;
        /// The worker id
        unsigned int _wid;
        /// The next free node id
        unsigned int _nid;

    public:
        /// Class for storing identifiers
        class ID {
        protected:
            /// The actual identifier
            unsigned int _id;

        public:
            /// Do not initialize
            ID() = default;
            /// Initialize
            ID(unsigned int id);
            /// Access
            operator unsigned int() const;
        };
        /// Initialize
        TraceRecorder(SearchTracer* t);
        /// Register engine
        void engine(SearchTracer::EngineType t, unsigned int n);
        /// Register worker
        void worker();
        /// Generate new node id
        unsigned int nid();
        /// Return worker id
        unsigned int wid() const;
        /// The workers goes to a next round (restart or next iteration in LDS)
        void round();
        /// The engine skips an edge
        void skip(const SearchTracer::EdgeInfo& ei);
        /// The engine creates a new node with information \a ei and \a ni
        void node(const SearchTracer::EdgeInfo& ei, const SearchTracer::NodeInfo& ni);
        /// The worker is done
        void done();
        /// Whether this is a real tracer
        operator bool() const;
    };

    /// Recorder for a search tracer with edge information
    class EdgeTraceRecorder : public TraceRecorder {
    protected:
        /// Edge information
        SearchTracer::EdgeInfo _ei;

    public:
        /// Initialize
        EdgeTraceRecorder(SearchTracer* t);
        /// Invalidate edge information
        void invalidate();
        /// Provide access to edge information
        SearchTracer::EdgeInfo* ei();
    };

    /// Empty trace recorder
    class NoTraceRecorder {
    public:
        /// Class for storing node identifiers
        class ID {
        public:
            /// Do not initialize
            ID() = default;
            /// Initialize
            ID(unsigned int id);
            /// Access
            operator unsigned int() const;
        };
        /// Initialize
        NoTraceRecorder(SearchTracer* t);
        /// Register engine
        void engine(SearchTracer::EngineType t, unsigned int n);
        /// Register worker
        void worker();
        /// Return worker id
        unsigned int wid() const;
        /// Generate new node id
        unsigned int nid();
        /// Invalidate edge information
        void invalidate();
        /// Provide access to edge information
        SearchTracer::EdgeInfo* ei();
        /// The workers goes to a next round (restart or next iteration in LDS)
        void round();
        /// The engine skips an edge
        void skip(const SearchTracer::EdgeInfo& ei);
        /// The engine creates a new node with information \a ei and \a ni
        void node(const SearchTracer::EdgeInfo& ei, const SearchTracer::NodeInfo& ni);
        /// The worker is done
        void done();
        /// Whether this is a real tracer
        operator bool() const;
    };

    /*
     * Recorder for engine events
     *
     */
    forceinline void
    WrapTraceRecorder::engine(SearchTracer* tracer, SearchTracer::EngineType t, unsigned int n) {
        if (tracer)
            tracer->engine(t, n);
    }

    /*
     * Simple trace recorder for a search tracer
     *
     */

    forceinline
    TraceRecorder::ID::ID(unsigned int id)
        : _id(id) {}

    forceinline TraceRecorder::ID::
    operator unsigned int() const {
        return _id;
    }

    forceinline
    TraceRecorder::TraceRecorder(SearchTracer* t)
        : tracer(*t), _eid(0U), _wid(0U), _nid(0U) {}

    forceinline void
    TraceRecorder::engine(SearchTracer::EngineType t, unsigned int n) {
        tracer.engine(t, n);
    }

    forceinline void
    TraceRecorder::worker() {
        tracer.worker(_wid, _eid);
    }

    forceinline unsigned int
    TraceRecorder::nid() {
        return _nid++;
    }

    forceinline unsigned int
    TraceRecorder::wid() const {
        return _wid;
    }

    forceinline void
    TraceRecorder::round() {
        tracer._round(_eid);
    }

    forceinline void
    TraceRecorder::skip(const SearchTracer::EdgeInfo& ei) {
        tracer._skip(ei);
    }

    forceinline void
    TraceRecorder::node(const SearchTracer::EdgeInfo& ei, const SearchTracer::NodeInfo& ni) {
        tracer._node(ei, ni);
    }

    forceinline void
    TraceRecorder::done() {
        tracer.worker();
    }

    forceinline TraceRecorder::
    operator bool() const {
        return true;
    }

    /*
     * Recorder for a search tracer with edge information
     *
     */
    forceinline
    EdgeTraceRecorder::EdgeTraceRecorder(SearchTracer* t)
        : TraceRecorder(t) {}

    forceinline void
    EdgeTraceRecorder::invalidate() {
        _ei.invalidate();
    }

    forceinline SearchTracer::EdgeInfo*
    EdgeTraceRecorder::ei() {
        return &_ei;
    }

    /*
     * Empty trace recorder
     *
     */

    forceinline
    NoTraceRecorder::ID::ID(unsigned int) {}

    forceinline NoTraceRecorder::ID::
    operator unsigned int() const {
        return 0U;
    }

    forceinline
    NoTraceRecorder::NoTraceRecorder(SearchTracer*) {}

    forceinline void
    NoTraceRecorder::engine(SearchTracer::EngineType, unsigned int) {}

    forceinline void
    NoTraceRecorder::worker() {}

    forceinline unsigned int
    NoTraceRecorder::wid() const {
        return 0U;
    }

    forceinline unsigned int
    NoTraceRecorder::nid() {
        return 0U;
    }

    forceinline void
    NoTraceRecorder::invalidate() {}

    forceinline SearchTracer::EdgeInfo*
    NoTraceRecorder::ei() {
        return nullptr;
    }

    forceinline void
    NoTraceRecorder::round() {}

    forceinline void
    NoTraceRecorder::skip(const SearchTracer::EdgeInfo&) {}

    forceinline void
    NoTraceRecorder::node(const SearchTracer::EdgeInfo&, const SearchTracer::NodeInfo&) {}

    forceinline void
    NoTraceRecorder::done() {}

    forceinline NoTraceRecorder::
    operator bool() const {
        return false;
    }

} // namespace Gecode::Search

// STATISTICS: search-trace
