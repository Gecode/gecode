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

#ifndef GECODE_GIST_GIST_HH
#define GECODE_GIST_GIST_HH

/*
 * Support for DLLs under Windows
 *
 */

#include "gecode/config.hh"

#if !defined(GIST_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_GIST
#define GECODE_GIST_EXPORT __declspec( dllexport )
#else
#define GECODE_GIST_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define GECODE_GIST_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_GIST_EXPORT

#endif
#endif

class QWidget;

#include "gecode/gist/better.hh"
#include <string>
#include <sstream>

namespace Gecode { 
  
  /** 
   * \namespace Gecode::Gist
   * \brief The Gecode Interactive Search Tool
   * 
   * The Gecode::Gist namespace contains the %Gecode Interactive Search Tool,
   * a Qt-based graphical search engine.
   *
   */
  
  namespace Gist {
  /** \brief Abstract base class for Inspectors
    *
    * An inspector provides a virtual method that is called
    * when a node in the search tree is inspected (e.g. by
    * double-clicking)
    */
  class GECODE_GIST_EXPORT Inspector {
  public:
    /// Call-back function
    virtual void inspect(Space* node) = 0;
    /// Destructor
    virtual ~Inspector(void);
  };
  
  /// \brief An inspector base class for simple text output
  class GECODE_GIST_EXPORT TextInspector : public Inspector {
  private:
    class TextInspectorImpl;
    /// The implementation object
    TextInspectorImpl *t;
    /// The name of the inspector
    std::string n;
  protected:
    /// Initialize the implementation object
    void init(void);
    /// Get the stream that is used to output text
    std::ostream& getStream(void);
    /// Add html text \a s to the output
    void addHtml(const char* s);
  public:
    /// Constructor
    TextInspector(const std::string& name);
    /// Destructor
    virtual ~TextInspector(void);
  };
  
  /// \brief An inspector for printing simple text output
  template <class S>
  class PrintingInspector : public TextInspector {
  public:
    /// Constructor
    PrintingInspector(const std::string& name);
    /// Use the print method of the template class S to print a space
    virtual void inspect(Space* node);
  };
  
  /// Create a new stand-alone Gist for \a root using \a b and \a gi
  GECODE_GIST_EXPORT
  int explore(Space* root, Better* b, Inspector* gi);
  
  /// Create a new Gist widget with parent \a parent for \a root using \a b and \a gi
  GECODE_GIST_EXPORT
  QWidget* exploreWidget(QWidget* parent, 
                         Space* root, Better* b, Inspector* gi);
  }	

  /// Create a new stand-alone Gist for \a root using \a gi
  GECODE_GIST_EXPORT
  int explore(Space* root, Gist::Inspector* gi = 0);
  /// Create a new Gist widget with parent \a parent for \a root and \a gi
  GECODE_GIST_EXPORT
  void exploreWidget(QWidget* parent, Space* root, Gist::Inspector* gi = 0);

  /// Create a new stand-alone Gist for branch-and-bound search of \a root, using \a gi
  template <class S>
  int exploreBest(S* root, Gist::Inspector* gi = 0);

  /// Create a new Gist widget with parent \a parent for branch-and-bound search of \a root, using \a gi
  template <class S>
  void exploreBestWidget(QWidget* parent, S* root, Gist::Inspector* gi = 0);

}

#include "gecode/gist/gist.icc"

#endif

// STATISTICS: gist-any
