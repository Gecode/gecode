/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/support.hh>
#include <gecode/gist/node.hh>
#include <gecode/gist/visualnode.hh>
#include <cassert>

namespace Gecode { namespace Gist {

    void
    Node::set_number_of_children(unsigned int n, NodeAllocator& na) {
        assert(get_tag() == UNDET);
        switch (n) {
        case 0:
            set_tag(LEAF);
            break;
        case 1:
            children_or_first_child =
                reinterpret_cast<void*>(static_cast<Support::uptrint_t>(na.allocate(get_index(na))) << 2);
            no_of_children = 1;
            set_tag(TWO_CHILDREN);
            break;
        case 2: {
            int idx = get_index(na);
            children_or_first_child = reinterpret_cast<void*>(static_cast<Support::uptrint_t>(na.allocate(idx)) << 2);
            no_of_children = -na.allocate(idx);
            set_tag(TWO_CHILDREN);
        } break;
        default: {
            int idx = get_index(na);
            no_of_children = n;
            int* children = heap.alloc<int>(n);
            children_or_first_child = static_cast<void*>(children);
            set_tag(MORE_CHILDREN);
            for (unsigned int i = n; i--;)
                children[i] = na.allocate(idx);
        }
        }
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
