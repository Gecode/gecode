/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-08-04 16:06:52 +0200 (Fri, 04 Aug 2006) $ by $Author: schulte $
 *     $Revision: 3517 $
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

#ifndef GECODE_GIST_GIST_HH
#define GECODE_GIST_GIST_HH

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GIST_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_GIST
#define GECODE_GIST_EXPORT __declspec( dllexport )
#else
#define GECODE_GIST_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_GIST_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_GIST_EXPORT

#endif
#endif

class QWidget;

#include "gecode/gist/better.hh"

namespace Gecode { namespace Gist {
  class Inspector {
  public:
    virtual void inspect(Space* node) = 0;
    virtual ~Inspector(void);
  };
  
  template <class S>
  class PrintingInspector : public Inspector {
  public:
    virtual void inspect(Space* node) {
      dynamic_cast<S*>(node)->print();
    }
  };
  
  int explore(Space* root, Better* b, Inspector* gi);
  QWidget* exploreWidget(QWidget* parent, 
                         Space* root, Better* b, Inspector* gi);
  }	

  int explore(Space* root, Gist::Inspector* gi = 0);
  void exploreWidget(QWidget* parent, Space* root, Gist::Inspector* gi = 0);

  template <class S>
  int exploreBest(S* root, Gist::Inspector* gi = 0);

  template <class S>
  void exploreBestWidget(QWidget* parent, S* root, Gist::Inspector* gi = 0);

}

#include "gecode/gist/gist.icc"

#endif
