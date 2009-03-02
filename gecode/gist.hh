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

#ifndef __GECODE_GIST_HH__
#define __GECODE_GIST_HH__

#include <gecode/kernel.hh>
#include <gecode/search.hh>

/*
 * Configure linking
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

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_GIST_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_GIST_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_GIST
#define GECODE_LIBRARY_NAME "Gist"
#include <gecode/support/auto-link.hpp>
#endif

class QWidget;

#include <string>
#include <sstream>

namespace Gecode {

  /**
   * \namespace Gecode::Gist
   * \brief The %Gecode Interactive %Search Tool
   *
   * The Gecode::Gist namespace contains the %Gecode Interactive %Search Tool,
   * a Qt-based graphical search engine.
   *
   */
  namespace Gist {

    /** \brief Abstract base class for Inspectors
     *
     * An inspector provides a virtual method that is called
     * when a node in the search tree is inspected (e.g. by
     * double-clicking)
     *
     * \ingroup TaskGist
     */
    class GECODE_GIST_EXPORT Inspector {
    public:
      /// Call-back function
      virtual void inspect(const Space& node) = 0;
      /// Name of the inspector
      virtual std::string name(void);
      /// Destructor
      virtual ~Inspector(void);
    };
    
    /// An inspector base class for simple text output
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
      /// Name of the inspector
      virtual std::string name(void);
    };
    
    /// An inspector for printing simple text output
    template <class S>
    class Print : public TextInspector {
    public:
      /// Constructor
      Print(const std::string& name);
      /// Use the print method of the template class S to print a space
      virtual void inspect(const Space& node);
    };
    
    /**
     * \brief Options for %Gist
     *
     * Note that the \a d and \a stop fields of the Search::Options class
     * are not used by Gist.
     *
     */
    class Options : public Search::Options {
    public:
      struct {
        /// Inspector that reacts on node double clicks
        Inspector* click;
        /// Inspector that reacts on each new solution that is found
        Inspector* solution;
      } inspect;
      /// Default options
      GECODE_GIST_EXPORT static const Options def;
      /// Initialize with default values
      Options(void);
    };

    
    /// Create a new stand-alone Gist for \a root using \a bab
    GECODE_GIST_EXPORT int 
    explore(Space* root, bool bab, const Options& opt);
    
    /**
     * \brief Create a new stand-alone Gist for \a root
     * \ingroup TaskGist
     */
    int 
    dfs(Space* root, const Gist::Options& opt = Gist::Options::def);
    
    /**
     * \brief Create a new stand-alone Gist for branch-and-bound search of \a root
     * \ingroup TaskGist
     */
    int 
    bab(Space* root, const Gist::Options& opt = Gist::Options::def);
    
  }	

}

#include <gecode/gist/gist.hpp>

#endif

// STATISTICS: gist-any
