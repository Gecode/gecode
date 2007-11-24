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

#include "gecode/gist/postscript/postscript.hh"
#include "gecode/gist/nodecursor.hh"
#include "gecode/gist/nodevisitor.hh"
#include <list>

namespace Gecode { namespace Gist {
  
  class PSColor {
  public:
    double r,g,b;
    PSColor(double r0, double g0, double b0);
    PSColor(void);
  };
  
  PSColor::PSColor(double r0, double g0, double b0) : r(r0), g(g0), b(b0) {}
  PSColor::PSColor(void) : r(0.0), g(0.0), b(0.0) {}

  class PSBoundingBox {
  public:
    int minx, maxx, miny, maxy;
    PSBoundingBox(int minx0, int maxx0, int miny0, int maxy0);
    void merge(const PSBoundingBox& b);
    void translate(double scale, double translatex, double translatey);
  };
  
  PSBoundingBox::PSBoundingBox(int minx0, int maxx0, int miny0, int maxy0) {
    minx = minx0;
    maxx = maxx0;
    miny = miny0;
    maxy = maxy0;    
  }
  
  void
  PSBoundingBox::merge(const PSBoundingBox& b) {
    if (b.minx < minx) minx = b.minx;
    if (b.maxx > maxx) maxx = b.maxx;
    if (b.miny < miny) miny = b.miny;
    if (b.maxy > maxy) maxy = b.maxy;    
  }
  
  void
  PSBoundingBox::translate(double scale, double translatex, double translatey) {
    minx = (int) (((double)minx)*scale+translatex);
    maxx = (int) (((double)maxx)*scale+translatex);
    miny = (int) (((double)miny)*scale+translatey);
    maxy = (int) (((double)maxy)*scale+translatey);    
  }
  
  class Path {
  public:
    virtual void emit(std::ostream& out) = 0;
    virtual PSBoundingBox bb(void) = 0;
    virtual ~Path(void);
  };
  
  Path::~Path(void) {}
  
  class Triangle : public Path {
  private:
    int c1x, c1y, c2x, c2y, c3x, c3y;
    PSColor outline;
    PSColor fill;
  
  public:
    Triangle(int c1x0, int c1y0, int c2x0, int c2y0, int c3x0, int c3y0,
             const PSColor& outline0, const PSColor& fill0);
    virtual void emit(std::ostream& out);
    virtual PSBoundingBox bb(void);
  };

  Triangle::Triangle(int c1x0, int c1y0, int c2x0, int c2y0, int c3x0, int c3y0,
                      const PSColor& outline0, const PSColor& fill0)
    : c1x(c1x0), c1y(c1y0), c2x(c2x0), c2y(c2y0), c3x(c3x0), c3y(c3y0),
      outline(outline0), fill(fill0) {}

  void
  Triangle::emit(std::ostream& out) {
    out << "gsave\n";
    out << c1x << " " << c1y << " moveto "<<c2x<<" "<<c2y<<" lineto ";
    out << c3x<<" "<<c3y<<" lineto "<<c1x<<" "<<c1y<<" lineto\n";
    out << fill.r<<" "<<fill.g<<" "<<fill.b<<" setrgbcolor\n";
    out << "eofill\n";
    out << c1x<<" "<<c1y<<" moveto "<<c2x<<" "<<c2y<<" lineto ";
    out << c3x<<" "<<c3y<<" lineto "<<c1x<<" "<<c1y<<" lineto\n";
    out << "1 setlinejoin 1 setlinecap\n";
    out << "1 setlinewidth\n";
    out << "[] 0 setdash\n";
    out << outline.r<<" "<<outline.g<<" "<<outline.b<<" setrgbcolor\n";
    out << "stroke\n";
    out << "grestore\n";    
  }

  PSBoundingBox
  Triangle::bb(void) {
    return PSBoundingBox(std::min(std::min(c1x,c2x),c3x),
                         std::max(std::max(c1x,c2x),c3x),
                         std::min(std::min(c1y,c2y),c3y),
                         std::max(std::max(c1y,c2y),c3y));
   }
  
  class Circle : public Path {
  private:
    int cx, cy, r;
    PSColor outline;
    PSColor fill;

  public:  
    Circle(int cx0, int cy0, int r0, const PSColor& outline0, const PSColor& fill0);
    virtual void emit(std::ostream& out);
    virtual PSBoundingBox bb(void);
  };

  Circle::Circle(int cx0, int cy0, int r0, const PSColor& outline0, const PSColor& fill0)
    : cx(cx0), cy(cy0), r(r0), outline(outline0), fill(fill0) {}

  void
  Circle::emit(std::ostream& out) {
    out << "gsave\n";
    out << "matrix currentmatrix\n";
    out << cx << ".0 " << cy << ".0 translate " << r << ".0 " << r << 
      ".0 scale 1 0 moveto 0 0 1 0 360 arc\n";
    out << "setmatrix\n";
    out << fill.r << " " << fill.g << " " << fill.b << " setrgbcolor\n";
    out << "fill\n";
    out << "matrix currentmatrix\n";
    out << cx << ".0 " << cy << ".0 translate " << r << ".0 " << r << 
      ".0 scale 1 0 moveto 0 0 1 0 360 arc\n";
    out << "setmatrix\n";
    out << "0 setlinejoin 2 setlinecap\n";
    out << "1 setlinewidth\n";
    out << "[] 0 setdash\n";
    out << outline.r << " " << outline.g << " " << outline.b << " setrgbcolor\n";
    out << "stroke\n";
    out << "grestore\n";    
  }
  
  PSBoundingBox
  Circle::bb(void) {
    return PSBoundingBox(cx-r,cx+r,cy-r,cy+r);    
  }
  
  class Diamond : public Path {
  private:
    int cx, cy, r;
    PSColor outline;
    PSColor fill;

  public:  
    Diamond(int cx0, int cy0, int r0, const PSColor& outline0, const PSColor& fill0);
    
    virtual void emit(std::ostream& out);
    virtual PSBoundingBox bb(void);
  };
  
  Diamond::Diamond(int cx0, int cy0, int r0, const PSColor& outline0, const PSColor& fill0)
    : cx(cx0), cy(cy0), r(r0), outline(outline0), fill(fill0) {}

  void
  Diamond::emit(std::ostream& out) {
    double c1x = (double) cx;
    double c1y = (double) cy;
    double c2x = c1x + (double) r;
    double c2y = c1y + (double) r;
    double c3x = c1x;
    double c3y = c1y + (double) (2*r);
    double c4x = c1x - (double) r;
    double c4y = c2y;

    out << "gsave\n";
    out << c1x << " " << c1y << " moveto " << c2x << " " << c2y << " lineto ";
    out << c3x << " " << c3y << " lineto " << c4x << " " << c4y << " lineto ";
    out << c1x << " " << c1y << " lineto\n";
    out << fill.r << " " << fill.g << " " << fill.b << " setrgbcolor\n";
    out << "eofill\n";
    out << c1x << " " << c1y << " moveto " << c2x << " " << c2y << " lineto ";
    out << c3x << " " << c3y << " lineto " << c4x << " " << c4y << " lineto ";
    out << c1x << " " << c1y << " lineto\n";
    out << "1 setlinejoin 1 setlinecap\n";
    out << "1 setlinewidth\n";
    out << "[] 0 setdash\n";
    out << outline.r << " " << outline.g << " " << outline.b << " setrgbcolor\n";
    out << "stroke\n";
    out << "grestore\n";
  }
  
  PSBoundingBox
  Diamond::bb(void) {
    return PSBoundingBox(cx-r,cx+r,cy-r,cy+r);
  }

  class Rectangle : public Path {
    int c1x, c1y, c2x, c2y;
    PSColor outline;
    PSColor fill;

  public:
    Rectangle(int c1x0, int c1y0, int c2x0, int c2y0,
              const PSColor& outline0, const PSColor& fill0);
    
    virtual void emit(std::ostream& out);
    virtual PSBoundingBox bb(void);
  };

  Rectangle::Rectangle(int c1x0, int c1y0, int c2x0, int c2y0,
                         const PSColor& outline0, const PSColor& fill0)
    : c1x(c1x0), c1y(c1y0), c2x(c2x0), c2y(c2y0),
      outline(outline0), fill(fill0) {}
  
  void
  Rectangle::emit(std::ostream& out) {
    out << "gsave\n";
    out << c1x << ".0 " << c1y << ".0 moveto " << c1x << ".0 " << c2y << ".0 lineto ";
    out << c2x << ".0 " << c2y << ".0 lineto " << c2x << ".0 " << c1y << 
      ".0 lineto " << "closepath\n";
    out << fill.r << " " << fill.g << " " << fill.b << " setrgbcolor\n";
    out << "fill\n";
    out << c1x << ".0 " << c1y << ".0 moveto " << c1x << ".0 " << c2y << ".0 lineto ";
    out << c2x << ".0 " << c2y << ".0 lineto " << c2x << ".0 " << c1y << 
      ".0 lineto " << "closepath\n";
    out << "1 setlinejoin 1 setlinecap\n";
    out << "1 setlinewidth\n";
    out << "[] 0 setdash\n";
    out << outline.r << " " << outline.g << " " << outline.b << " setrgbcolor\n";
    out << "stroke\n";
    out << "grestore\n";    
  }
  
  PSBoundingBox
  Rectangle::bb(void) {
    return PSBoundingBox(std::min(c1x,c2x), std::max(c1x,c2x),
                          std::min(c1y,c2y), std::max(c1y,c2y));    
  }

  class Line : public Path {
  private:
    int ax, ay, bx, by;
    PSColor color;
  public:
    Line(int ax0, int ay0, int bx0, int by0, const PSColor& color0);
    
    virtual void emit(std::ostream& out);
    virtual PSBoundingBox bb(void);
  };

  Line::Line(int ax0, int ay0, int bx0, int by0, const PSColor& color0)
    : ax(ax0), ay(ay0), bx(bx0), by(by0), color(color0) {}
    
  void
  Line::emit(std::ostream& out) {
    out << "gsave\n";
    out << ax << ".0 " << ay << ".0 moveto\n";
    out << bx << ".0 " << by << ".0 lineto\n";
    out << "0 setlinecap\n";
    out << "1 setlinejoin\n";
    out << "1 setlinewidth\n";
    out << "[] 0 setdash\n";
    out << color.r << " " << color.g << " " << color.b << " setrgbcolor\n";
    out << "stroke\n";
    out << "grestore\n";    
  }

  PSBoundingBox
  Line::bb(void) {
    return PSBoundingBox(std::min(ax,bx), std::max(ax,bx),
                          std::min(ay,by), std::max(ay,by));    
  }

  class PostscriptCursor : public NodeCursor<VisualNode> {
  protected:
    static const int verticalOffset = 38;
    static const PSColor black;
    static const PSColor blue;
    static const PSColor white;
    static const PSColor red;
    static const PSColor green;
    static const PSColor lightgray;
    
    int x, y;
    
    std::list<Path*>& ps;

    void drawHidden(int x, int y, bool solved, bool open);
    void drawFailed(int x, int y);
    void drawSolved(int x, int y);
    void drawChoice(int x, int y);
    void drawUndetermined(int x, int y);
  public:
    PostscriptCursor(VisualNode* tree, std::list<Path*>& ps0);
    void moveUpwards(void);
    bool mayMoveDownwards(void);
    void moveDownwards(void);
    void moveSidewards(void);        
    
    void processCurrentNode(void);
  };

  const PSColor
  PostscriptCursor::black(0.0,0.0,0.0);
  const PSColor
  PostscriptCursor::blue(0.0,0.36,0.63);
  const PSColor
  PostscriptCursor::white(1.0,1.0,1.0);
  const PSColor
  PostscriptCursor::red(0.85,0.14,0.11);
  const PSColor
  PostscriptCursor::green(0.04,0.46,0.27);
  const PSColor
  PostscriptCursor::lightgray(0.92,0.92,0.92);

  PostscriptCursor::PostscriptCursor(VisualNode* tree, std::list<Path*>& ps0)
    : NodeCursor<VisualNode>(tree), x(0), y(0), ps(ps0) {}

  void
  PostscriptCursor::moveUpwards() {
    x = x - node()->getOffset();
    y = y - verticalOffset;
    NodeCursor<VisualNode>::moveUpwards();
  }
      
  bool
  PostscriptCursor::mayMoveDownwards() {
      return (NodeCursor<VisualNode>::mayMoveDownwards() &&
              ! node()->isHidden() );
  }
  
  void
  PostscriptCursor::moveDownwards() {
      NodeCursor<VisualNode>::moveDownwards();
      x = x + node()->getOffset();
      y = y + verticalOffset;
  }
  
  void
  PostscriptCursor::moveSidewards() {
      x = x - node()->getOffset();
      NodeCursor<VisualNode>::moveSidewards();
      x = x + node()->getOffset();
  }

  void
  PostscriptCursor::drawHidden(int x, int y, bool solved, bool) {
    PSColor fill = solved ? green : red;
    ps.push_back(new Triangle(x,-y+10,x-16,-y-38,x+16,-y-38,black,fill));
  }

  void
  PostscriptCursor::drawFailed(int x, int y) {
    ps.push_back(new Rectangle(x-7,-y+7,x+7,-y-7,black,red));
  }

  void
  PostscriptCursor::drawSolved(int x, int y) {
    ps.push_back(new Diamond(x, -y-10, 10, black, green));
  }

   void
  PostscriptCursor::drawChoice(int x, int y) {
    ps.push_back(new Circle(x, -y, 10, black, blue));
  }

   void
  PostscriptCursor::drawUndetermined(int x, int y) {
    ps.push_back(new Circle(x, -y, 10, black, white));
  }

  void
  PostscriptCursor::processCurrentNode(void) {
    VisualNode* currentNode = node();
    int parentX = (x - currentNode->getOffset());
    int myx = x;

    if (! currentNode->isRoot()) {
      if (currentNode->isHidden()) {
        ps.push_back(new Line(myx, -y+10, parentX, -y+28, black));
      } else {
        switch(currentNode->getStatus()) {
        case SOLVED:
        case UNDETERMINED:
        case BRANCH:
          ps.push_back(new Line(myx, -y+10, parentX, -y+28, black));
          break;
        case FAILED:
          ps.push_back(new Line(myx, -y+7, parentX, -y+28, black));
          break;
        }
      }
    }
    if (currentNode->isHidden()) {
      drawHidden(myx, y,
                  currentNode->hasSolvedChildren(),
                  currentNode->isOpen());
    } else {
      switch(currentNode->getStatus()) {
      case UNDETERMINED:
        drawUndetermined(myx,y);
        break;
      case FAILED:
        drawFailed(myx,y);
        break;
      case SOLVED:
        drawSolved(myx,y);
        break;
      case BRANCH:
        drawChoice(myx,y);
        break;
      }
    }    
  }
  
  void
  Postscript::output(std::ostream& out, VisualNode* tree) {
    PSBoundingBox b(100000,0,100000,0);

    double scale = 0.72;
    double translatex = 306.0;
    double translatey = 396.0;
    
    int translatexI = 311;
    int translateyI = 390;
    
    std::list<Path*> p;
    PostscriptCursor cursor(tree, p);
    PreorderNodeVisitor<PostscriptCursor> visitor(cursor);
    while (visitor.next());
    
    for (std::list<Path*>::iterator i=p.begin(); i != p.end(); ++i) {
        b.merge((*i)->bb());
    }
    b.translate(scale, translatex, translatey);

    out << "%!PS-Adobe-3.0 EPSF-3.0\n";
    out << "%%Creator: Gecode/J PS library\n";
    out << "%%For: Who knows?\n";
    out << "%%Title: figure\n";
    out << "%%CreationDate: Fri Jul 16 13:58:16 2004\n";
    out << "%%BoundingBox: " << b.minx << " " << b.miny << " " << 
        b.maxx+10 << " " << b.maxy << "\n";
    out << "%%Pages: 1\n";
    out << "%%DocumentData: Clean7Bit\n";
    out << "%%Orientation: Portrait\n";
    out << "%%EndComments\n\n";
    out << "%%BeginProlog\n";
    out << "50 dict begin\n\n";
    out << "%%EndProlog\n";
    out << "%%Page: 1 1\n";
    out << "save\n";
    out << translatexI << " " << translateyI << " translate\n";
    out << scale << " " << scale << " scale\n";

    for (std::list<Path*>::iterator i=p.begin(); i != p.end(); ++i) {
        (*i)->emit(out);
        delete *i;
    }
    out << "restore showpage\n\n" << 
              "%%Trailer\n" << 
              "end\n" << 
              "%%EOF\n";
  }
  
}}
