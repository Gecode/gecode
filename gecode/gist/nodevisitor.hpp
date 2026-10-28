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

    template <class Cursor>
    forceinline
    NodeVisitor<Cursor>::NodeVisitor(const Cursor& c0)
        : c(c0) {}

    template <class Cursor>
    forceinline void
    NodeVisitor<Cursor>::set_cursor(const Cursor& c0) {
        c = c0;
    }

    template <class Cursor>
    forceinline Cursor&
    NodeVisitor<Cursor>::get_cursor(void) {
        return c;
    }

    template <class Cursor>
    forceinline void
    PostorderNodeVisitor<Cursor>::move_to_leaf(void) {
        while (c.may_move_downwards()) {
            c.move_downwards();
        }
    }

    template <class Cursor>
    PostorderNodeVisitor<Cursor>::PostorderNodeVisitor(const Cursor& c0) : NodeVisitor<Cursor>(c0) {
        move_to_leaf();
    }

    template <class Cursor>
    forceinline bool
    PostorderNodeVisitor<Cursor>::next(void) {
        c.process_current_node();
        if (c.may_move_sidewards()) {
            c.move_sidewards();
            move_to_leaf();
        } else if (c.may_move_upwards()) {
            c.move_upwards();
        } else {
            return false;
        }
        return true;
    }

    template <class Cursor>
    forceinline void
    PostorderNodeVisitor<Cursor>::run(void) {
        while (next()) {
        }
    }

    template <class Cursor>
    forceinline bool
    PreorderNodeVisitor<Cursor>::backtrack(void) {
        while (!c.may_move_sidewards() && c.may_move_upwards()) {
            c.move_upwards();
        }
        if (!c.may_move_upwards()) {
            return false;
        } else {
            c.move_sidewards();
        }
        return true;
    }

    template <class Cursor>
    PreorderNodeVisitor<Cursor>::PreorderNodeVisitor(const Cursor& c0) : NodeVisitor<Cursor>(c0) {}

    template <class Cursor>
    forceinline bool
    PreorderNodeVisitor<Cursor>::next(void) {
        c.process_current_node();
        if (c.may_move_downwards()) {
            c.move_downwards();
        } else if (c.may_move_sidewards()) {
            c.move_sidewards();
        } else {
            return backtrack();
        }
        return true;
    }

    template <class Cursor>
    forceinline void
    PreorderNodeVisitor<Cursor>::run(void) {
        while (next()) {
        }
    }
}} // namespace Gecode::Gist

// STATISTICS: gist-any
