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

#include "gecode/gist/dotnode.hh"

using namespace std;

namespace Gecode { namespace Gist {

  DotNode::DotNode(int alternative, BestSpace* b) : SpaceNode(alternative, b) {}
  
  DotNode::DotNode(Space* root, Better* b) : SpaceNode(root, b) {}
  
  int
  DotNode::printDotInternal(int& count) {
    switch (getStatus()) {
    case UNDETERMINED:
      std::cout << "\tnode" << count << " [shape=circle, label=\"\", height=1, width=1];" << std::endl;
      return count;
      break;
    case FAILED:
      std::cout << "\tnode" << count << " [shape=box, label=\"\", height=.6, width=.6, style=filled, fillcolor=red];" << std::endl;
      return count;
      break;
    case SOLVED:
      std::cout << "\tnode" << count << " [shape=diamond, label=\"\", height=.8, width=.8, style=filled, fillcolor=green];" << std::endl;
      return count;
      break;
    case BRANCH:
      {
        std::cout << "\tnode" << count << " [shape=circle, label=\"\", height=.7, width=.7, style=filled, fillcolor=blue];" << std::endl;
        int mycount = count;
        for (unsigned int i=0; i<children.size(); i++) {
          count++;
          int childi =
            (static_cast<DotNode*>(children[i]))->printDotInternal(count);
          std::cout << "\t\"node" << mycount << "\" -> \"node" << childi << "\";"
                    << std::endl;
        }
        return mycount;
      }
    default:
      assert(false);
    }    
  }
  
  void
  DotNode::printDot(void) {
    std::cout << "digraph tree {" << std::endl;
    int count = 0;
    (void) printDotInternal(count);
    std::cout << "}" << std::endl;
  }
  
  SpaceNode*
  DotNode::createChild(int alternative) {
    return new DotNode(alternative, curBest);
  }

}}

// STATISTICS: gist-any
