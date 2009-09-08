/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

  forceinline
  Extent::Extent(void) : l(-1), r(-1) {}

  forceinline
  Extent::Extent(int l0, int r0) : l(l0), r(r0) {}

  inline
  Extent::Extent(int width) {
    int halfWidth = width / 2;
    l = 0 - halfWidth;
    r = 0 + halfWidth;
  }

  inline void
  Extent::extend(int deltaL, int deltaR) {
    l += deltaL; r += deltaR;
  }

  inline void
  Extent::move(int delta) {
    l += delta; r += delta;
  }

  forceinline const Extent&
  Shape::operator [](int i) const {
    assert(i < _depth);
    return shape[i];
  }

  forceinline Extent&
  Shape::operator [](int i) {
    assert(i < _depth);
    return shape[i];
  }

  inline Shape*
  Shape::allocate(int d) {
    Shape* ret =
      static_cast<Shape*>(heap.ralloc(sizeof(Shape) +
                                         (d-1)*sizeof(Extent)));
    ret->_depth = d;
    return ret;
  }

  inline Shape*
  Shape::allocate(Extent e) {
    Shape* ret = Shape::allocate(1);
    (*ret)[0] = e;
    return ret;
  }

  forceinline void
  Shape::deallocate(Shape* shape) {
    if (shape != hidden && shape != leaf)
      heap.rfree(shape);
  }

  forceinline int
  Shape::depth(void) const { return _depth; }

  forceinline
  BoundingBox::BoundingBox(int l, int r)
   : left(l), right(r) {}

  forceinline bool
  VisualNode::isHidden(void) {
    return (nstatus & (1<<(HIDDEN-1))) != 0;
  }

  forceinline void
  VisualNode::setHidden(bool h) {
    if (h)
      nstatus |= 1<<(HIDDEN-1);
    else
      nstatus &= ~(1<<(HIDDEN-1));
  }

  forceinline int
  VisualNode::getOffset(void) { return offset; }

  forceinline void
  VisualNode::setOffset(int n) { offset = n; }

  forceinline bool
  VisualNode::isDirty(void) {
    return (nstatus & (1<<(DIRTY-1))) != 0;
  }

  forceinline void
  VisualNode::setDirty(bool d) {
    if (d)
      nstatus |= 1<<(DIRTY-1);
    else
      nstatus &= ~(1<<(DIRTY-1));
  }

  forceinline bool
  VisualNode::childrenLayoutIsDone(void) {
    return (nstatus & (1<<(CHILDRENLAYOUTDONE-1))) != 0;
  }

  forceinline void
  VisualNode::setChildrenLayoutDone(bool d) {
    if (d)
      nstatus |= 1<<(CHILDRENLAYOUTDONE-1);
    else
      nstatus &= ~(1<<(CHILDRENLAYOUTDONE-1));
  }

  forceinline bool
  VisualNode::isMarked(void) {
    return (nstatus & (1<<(MARKED-1))) != 0;
  }

  forceinline void
  VisualNode::setMarked(bool m) {
    if (m)
      nstatus |= 1<<(MARKED-1);
    else
      nstatus &= ~(1<<(MARKED-1));
  }

  forceinline bool
  VisualNode::isOnPath(void) {
    return (nstatus & (1<<(ONPATH-1))) != 0;
  }

  forceinline void
  VisualNode::setOnPath(bool b) {
    if (b)
      nstatus |= 1<<(ONPATH-1);
    else
      nstatus &= ~(1<<(ONPATH-1));
  }

  forceinline Shape*
  VisualNode::getShape(void) { return shape; }

  forceinline void
  VisualNode::setBoundingBox(BoundingBox b) { box = b; }

  forceinline BoundingBox
  VisualNode::getBoundingBox(void) { return box; }

  forceinline VisualNode*
  VisualNode::getParent() {
    return static_cast<VisualNode*>(SpaceNode::getParent());
  }

  forceinline VisualNode*
  VisualNode::getChild(int i) {
    return static_cast<VisualNode*>(SpaceNode::getChild(i));
  }

}}

// STATISTICS: gist-any
