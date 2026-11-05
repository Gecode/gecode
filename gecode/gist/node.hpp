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

namespace Gecode { namespace Gist {

    template <class T>
    void
    NodeAllocatorBase<T>::allocate() {
        cur_b++;
        cur_t = 0;
        if (cur_b == n) {
            int oldn = n;
            n = static_cast<int>(n * 1.5 + 1.0);
            b = heap.realloc<Block*>(b, oldn, n);
        }
        b[cur_b] = static_cast<Block*>(heap.ralloc(sizeof(Block)));
    }

    template <class T>
    NodeAllocatorBase<T>::NodeAllocatorBase(bool bab) : _bab(bab) {
        b = heap.alloc<Block*>(10);
        n = 10;
        cur_b = -1;
        cur_t = node_block_size - 1;
    }

    template <class T>
    NodeAllocatorBase<T>::~NodeAllocatorBase() {
        for (int i = cur_b + 1; i--;)
            heap.rfree(b[i]);
        heap.free<Block*>(b, n);
    }

    template <class T>
    forceinline int
    NodeAllocatorBase<T>::allocate(int p) {
        cur_t++;
        if (cur_t == node_block_size)
            allocate();
        new (&b[cur_b]->b[cur_t]) T(p);
        b[cur_b]->best[cur_t] = -1;
        return cur_b * node_block_size + cur_t;
    }

    template <class T>
    forceinline int
    NodeAllocatorBase<T>::allocate(Space* root) {
        cur_t++;
        if (cur_t == node_block_size)
            allocate();
        new (&b[cur_b]->b[cur_t]) T(root);
        b[cur_b]->best[cur_t] = -1;
        return cur_b * node_block_size + cur_t;
    }

    template <class T>
    forceinline T*
    NodeAllocatorBase<T>::operator[](int i) const {
        assert(i / node_block_size < n);
        assert(i / node_block_size < cur_b || i % node_block_size <= cur_t);
        return &(b[i / node_block_size]->b[i % node_block_size]);
    }

    template <class T>
    forceinline T*
    NodeAllocatorBase<T>::best(int i) const {
        assert(i / node_block_size < n);
        assert(i / node_block_size < cur_b || i % node_block_size <= cur_t);
        int bi = b[i / node_block_size]->best[i % node_block_size];
        return bi == -1 ? nullptr : (*this)[bi];
    }

    template <class T>
    forceinline void
    NodeAllocatorBase<T>::set_best(int i, int best) {
        assert(i / node_block_size < n);
        assert(i / node_block_size < cur_b || i % node_block_size <= cur_t);
        b[i / node_block_size]->best[i % node_block_size] = best;
    }

    template <class T>
    forceinline bool
    NodeAllocatorBase<T>::bab() const {
        return _bab;
    }

    template <class T>
    forceinline bool
    NodeAllocatorBase<T>::show_labels() const {
        return !labels.isEmpty();
    }

    template <class T>
    bool
    NodeAllocatorBase<T>::has_label(T* n) const {
        return labels.contains(n);
    }

    template <class T>
    void
    NodeAllocatorBase<T>::set_label(T* n, const QString& l) {
        labels[n] = l;
    }

    template <class T>
    void
    NodeAllocatorBase<T>::clear_label(T* n) {
        labels.remove(n);
    }

    template <class T>
    QString
    NodeAllocatorBase<T>::get_label(T* n) const {
        return labels.value(n);
    }

    forceinline unsigned int
    Node::get_tag() const {
        return static_cast<unsigned int>(reinterpret_cast<Support::uptrint_t>(children_or_first_child) & 0b11);
    }

    forceinline void
    Node::set_tag(unsigned int tag) {
        assert(tag <= 0b11);
        assert(get_tag() == UNDET);
        children_or_first_child =
            Support::join_pointer(Support::clear_pointer_tag_bits<2>(children_or_first_child), tag);
    }

    forceinline void*
    Node::get_ptr() const {
        return Support::clear_pointer_tag_bits<2>(children_or_first_child);
    }

    forceinline int
    Node::get_first_child() const {
        return static_cast<int>(
            (reinterpret_cast<Support::uptrint_t>(children_or_first_child) & ~static_cast<Support::uptrint_t>(0b11)) >>
            2);
    }

    forceinline
    Node::Node(int p, bool failed)
        : parent(p) {
        children_or_first_child = nullptr;
        no_of_children = 0;
        set_tag(failed ? LEAF : UNDET);
    }

    forceinline int
    Node::get_parent() const {
        return parent;
    }

    forceinline VisualNode*
    Node::get_parent(const NodeAllocator& na) const {
        return parent < 0 ? nullptr : na[parent];
    }

    forceinline bool
    Node::is_undetermined() const {
        return get_tag() == UNDET;
    }

    forceinline int
    Node::get_child(int n) const {
        assert(get_tag() != UNDET && get_tag() != LEAF);
        if (get_tag() == TWO_CHILDREN) {
            assert(n != 1 || no_of_children <= 0);
            return n == 0 ? get_first_child() : -no_of_children;
        }
        assert(n < no_of_children);
        return static_cast<int*>(get_ptr())[n];
    }

    forceinline VisualNode*
    Node::get_child(const NodeAllocator& na, int n) const {
        return na[get_child(n)];
    }

    forceinline bool
    Node::is_root() const {
        return parent == -1;
    }

    forceinline unsigned int
    Node::get_number_of_children() const {
        switch (get_tag()) {
        case UNDET:
        case LEAF:
            return 0;
        case TWO_CHILDREN:
            return (no_of_children <= 0) ? 2 : 1;
        default:
            return static_cast<unsigned int>(no_of_children);
        }
    }

    forceinline int
    Node::get_index(const NodeAllocator& na) const {
        if (parent == -1)
            return 0;
        Node* p = na[parent];
        for (int i = p->get_number_of_children(); i--;)
            if (p->get_child(na, i) == this)
                return p->get_child(i);
        GECODE_NEVER;
        return -1;
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
