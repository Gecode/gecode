/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2003
 *     Mikael Zayenz Lagerkvist, 2026
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

#ifndef GECODE_SEARCH_SEQ_PATH_HH
#define GECODE_SEARCH_SEQ_PATH_HH

#include <algorithm>

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/nogoods.hh>

namespace Gecode { namespace Search { namespace Seq {

    /**
     * \brief Depth-first path (stack of edges) supporting recomputation
     *
     * Maintains the invariant that it contains
     * the path of the space being currently explored. This
     * is required to support recomputation, of course.
     *
     * The path supports adaptive recomputation controlled
     * by the value of a_d: only if the recomputation
     * distance is at least this large, an additional
     * clone is created.
     *
     */
    template <class Tracer>
    class GECODE_VTABLE_EXPORT Path : public NoGoods {
        friend class Search::NoGoodsProp;

    public:
        /// Node identity type
        using ID = Tracer::ID;
        /// %Search tree edge for recomputation
        class Edge {
        protected:
            /// Space corresponding to this edge (might be nullptr)
            Space* _space;
            /// Current alternative
            unsigned int _alt;
            /// Choice
            const Choice* _choice;
            /// Node identifier
            ID _nid;

        public:
            /// Default constructor
            Edge();
            /// Edge for space \a s with clone \a c (possibly nullptr)
            Edge(Space* s, Space* c, unsigned int nid);

            /// Return space for edge
            Space* space() const;
            /// Set space to \a s
            void space(Space* s);

            /// Return choice
            const Choice* choice() const;

            /// Return number for alternatives
            unsigned int alt() const;
            /// Return true number for alternatives (excluding lao optimization)
            unsigned int true_alternative() const;
            /// Test whether current alternative is leftmost
            bool leftmost() const;
            /// Test whether current alternative is rightmost
            bool rightmost() const;
            /// Move to next alternative
            void next();
            /// Test whether current alternative was LAO
            bool lao() const;

            /// Return node identifier
            unsigned int nid() const;

            /// Free memory for edge
            void dispose();
        };

    protected:
        /// Stack to store edge information
        Support::DynamicStack<Edge, Heap> ds;
        /// Depth limit for no-good generation
        unsigned int _ngdl;

    public:
        /// Initialize with no-good depth limit \a l
        Path(unsigned int l);
        /// Return no-good depth limit
        unsigned int ngdl() const;
        /// Set no-good depth limit to \a l
        void ngdl(unsigned int l);
        /// Push space \a c (a clone of \a s or nullptr)
        const Choice* push(Worker& stat, Space* s, Space* c, unsigned int nid);
        /// Generate path for next node
        void next();
        /// Provide access to topmost edge
        Edge& top() const;
        /// Test whether path is empty
        bool empty() const;
        /// Return position on stack of last copy
        int lc() const;
        /// Unwind the stack up to position \a l (after failure)
        void unwind(int l, Tracer& t);
        /// Commit space \a s as described by stack entry at position \a i
        void commit(Space* s, int i) const;
        /// Recompute space according to path
        Space* recompute(unsigned int& d, unsigned int a_d, Worker& s, Tracer& t);
        /// Recompute space according to path
        Space* recompute(unsigned int& d, unsigned int a_d, Worker& s, const Space* best, int& mark, Tracer& t);
        /// Return number of entries on stack
        int entries() const;
        /// Reset stack
        void reset();
        /// Post no-goods
        void post(Space& home) const override;
    };

}}} // namespace Gecode::Search::Seq

#include <gecode/search/seq/path.hpp>

#endif

// STATISTICS: search-seq
