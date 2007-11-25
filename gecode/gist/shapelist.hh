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

#ifndef GECODE_GIST_SHAPELIST_HH
#define GECODE_GIST_SHAPELIST_HH

#include <vector>
#include <iostream>

namespace Gecode { namespace Gist {

  class BoundingBox {
  public:
    int left;
    int right;
    int depth;
    
    BoundingBox(int l, int r, int d);
  };

  class Extent {
  public:
    int l;
    int r;
    
    Extent(void);
    Extent(int l0, int r0);
    Extent(int width);
    
    void extend(int deltaL, int deltaR);
    void move(int delta);
  };

  class Shape {
  private:
    std::vector<Extent> shape;
    Shape(const Shape&);
  public:
    Shape(void);
    Shape(Extent e);
    Shape(Extent e, const Shape* subShape);
    Shape(const Shape* subShape);
    
    int depth(void) const;
    void add(Extent e);
    Extent get(int i);
    void extend(int deltaL, int deltaR);
    void move(int delta);
    bool getExtentAtDepth(int depth, Extent& extent);
    BoundingBox getBoundingBox(void);
    
    void print(void);
  };
  
  class ShapeList {
  private:
    std::vector<Shape*> shapes;
    int minimalSeparation;
    std::vector<int> offsets;

    int getAlpha(Shape* shape1, Shape* shape2);
    static Shape* merge(Shape* shape1, Shape* shape2, int alpha);
  public:
    ShapeList(int length, int minSeparation);
    
    Shape* getMergedShape(void);
    int getOffsetOfChild(int i);
    
    Shape*& operator[](int i);
  };
  
}}

#endif

// STATISTICS: gist-any
