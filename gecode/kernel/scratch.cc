/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include "gecode/kernel.hh"

namespace Gecode {

  void*
  Scratch::heap_alloc(size_t s) {
    void* p = heap.malloc(s);
    switch (++n_malloc) {
    case 0: 
      GECODE_NEVER;
    case 1:
      at.two.fst = p; break;
    case 2:
      at.two.snd = p; break;
    case 3:
      {
        void** b = static_cast<void**>(heap.malloc(sizeof(void*) * 8));
        b[0]=at.two.fst; b[1]=at.two.snd; b[2]=p;
        at.any.l_malloc = 8;
        at.any.b_malloc = b;
      }
      break;
    default:
      if (at.any.l_malloc == n_malloc) {
        unsigned int l = (3 * n_malloc) / 2;
        at.any.b_malloc = 
          static_cast<void**>(heap.realloc(at.any.b_malloc,
                                              l*sizeof(void*)));
        at.any.l_malloc = l;
      }
      at.any.b_malloc[n_malloc-1] = p;
      break;
    }
    return p;
  }

  void
  Scratch::heap_free(void) {
    switch (n_malloc) {
    case 0:
      GECODE_NEVER;
    case 2:
      heap.free(at.two.snd);
      // Fall through
    case 1:
      heap.free(at.two.fst);
      break;
    default:
      for (unsigned int i=n_malloc; i--; )
        heap.free(at.any.b_malloc[i]);
      heap.free(at.any.b_malloc);
      break;
    }
  }

}

// STATISTICS: kernel-other
