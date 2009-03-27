/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_DRIVER_HH__
#define __GECODE_DRIVER_HH__

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#ifdef GECODE_HAS_GIST
#include <gecode/gist.hh>
#endif

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_DRIVER
#define GECODE_DRIVER_EXPORT __declspec( dllexport )
#else
#define GECODE_DRIVER_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_DRIVER_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_DRIVER_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_DRIVER
#define GECODE_LIBRARY_NAME "Driver"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode::Driver
 * \brief Script commandline driver
 *
 * The Gecode::Driver namespace contains support for passing common
 * commandline options and for scripts that use the commandline
 * options.
 *
 */

namespace Gecode {


  /**
   * \defgroup TaskDriverCmd Commandline options for running scripts
   * \ingroup TaskDriver
   */

  /**
   * \brief Different modes for executing scripts
   * \ingroup TaskDriverCmd
   */
  enum ScriptMode {
    SM_SOLUTION, ///< Print solution and some statistics
    SM_TIME,     ///< Measure average runtime
    SM_STAT      ///< Print statistics for script
#ifdef GECODE_HAS_GIST
    ,
    SM_GIST      ///< Run script in Gist
#endif
  };

  class Options;

  namespace Driver {
    /**
     * \brief Base class for options
     *
     */
    class GECODE_DRIVER_EXPORT BaseOption {
      friend class Options;
    protected:
      const char* opt;  ///< String for option (including hyphen)
      const char* exp;  ///< Short explanation
      BaseOption* next; ///< Next option
    public:
      /// Initialize for option \a o and explanation \a e
      BaseOption(const char* o, const char* e);
      /// Parse option at first position and possibly delete
      virtual bool parse(int& argc, char* argv[]) = 0;
      /// Print help text
      virtual void help(void) = 0;
      /// Destructor
      virtual ~BaseOption(void);
    };
    
    /**
     * \brief String-valued option
     *
     */
    class GECODE_DRIVER_EXPORT StringOption : public BaseOption {
    protected:
      /// Option value
      class Value {
      public:
        int         val;  ///< Value for an option value
        const char* opt;  ///< String for option value
        const char* help; ///< Optional help text
        Value*      next; ///< Next option value
      };
      int    cur; ///< Current value
      Value* fst; ///< First option value
      Value* lst; ///< Last option value
    public:
      /// Initialize for option \a o and explanation \a e and default value \a v
      StringOption(const char* o, const char* e, int v=0);
      /// Set default value to \a v
      void value(int v);
      /// Return current option value
      int value(void) const;
      /// Add option value for value \a v, string \a o, and help text \a h
      void add(int v, const char* o, const char* h = NULL);
      /// Parse option at first position
      virtual bool parse(int& argc, char* argv[]);
      /// Print help text
      virtual void help(void);
      /// Destructor
      virtual ~StringOption(void);
    };
  

    /**
     * \brief Integer option
     *
     */
    class GECODE_DRIVER_EXPORT IntOption : public BaseOption {
    protected:
      int cur; ///< Current value
    public:
      /// Initialize for option \a o and explanation \a e and default value \a v
      IntOption(const char* o, const char* e, int v=0);
      /// Set default value to \a v
      void value(int v);
      /// Return current option value
      int value(void) const;
      /// Parse option at first position
      virtual bool parse(int& argc, char* argv[]);
      /// Print help text
      virtual void help(void);
    };

    /**
     * \brief Unsigned integer option
     *
     */
    class GECODE_DRIVER_EXPORT UnsignedIntOption : public BaseOption {
    protected:
      unsigned int cur; ///< Current value
    public:
      /// Initialize for option \a o and explanation \a e and default value \a v
      UnsignedIntOption(const char* o, const char* e, unsigned int v=0);
      /// Set default value to \a v
      void value(unsigned int v);
      /// Return current option value
      unsigned int value(void) const;
      /// Parse option at first position
      virtual bool parse(int& argc, char* argv[]);
      /// Print help text
      virtual void help(void);
    };

  }
  
  /**
   * \brief Options for scripts
   * \ingroup TaskDriverCmd
   */
  class GECODE_DRIVER_EXPORT Options {
  protected:
    Driver::BaseOption* fst;   ///< First registered option
    Driver::BaseOption* lst;   ///< Last registered option
    const char* _name; ///< Script name
    
    /// \name Model options
    //@{
    Driver::StringOption _model;       ///< General model options
    Driver::StringOption _propagation; ///< Propagation options
    Driver::StringOption _icl;         ///< Integer consistency level
    Driver::StringOption _branching;   ///< Branching options
    //@}
    
    /// \name Search options
    //@{
    Driver::StringOption      _search;    ///< Search options
    Driver::UnsignedIntOption _solutions; ///< How many solutions
    Driver::UnsignedIntOption _c_d;       ///< Copy recomputation distance
    Driver::UnsignedIntOption _a_d;       ///< Adaptive recomputation distance
    Driver::UnsignedIntOption _node;      ///< Cutoff for number of nodes
    Driver::UnsignedIntOption _fail;      ///< Cutoff for number of failures
    Driver::UnsignedIntOption _time;      ///< Cutoff for time
    //@}
    
    /// \name Execution options
    //@{
    Driver::StringOption      _mode;       ///< Script mode to run
    Driver::UnsignedIntOption _samples;    ///< How many samples
    Driver::UnsignedIntOption _iterations; ///< How many iterations per sample
    //@}

  public:
    /// Initialize options for script with name \a s
    Options(const char* s);
    /// Add new option \a o
    void add(Driver::BaseOption& o);
    /// Print help text
    virtual void help(void);
    /// Parse options from arguments \a argv (number is \a argc)
    void parse(int& argc, char* argv[]);
    
    /// Return name of script
    const char* name(void) const;
    
    /// \name Model options
    //@{
    /// Set default model value
    void model(int v);
    /// Add model option value for value \a v, string \a o, and help \a h
    void model(int v, const char* o, const char* h = NULL);
    /// Return model value
    int model(void) const;
    
    /// Set default propagation value
    void propagation(int v);
    /// Add propagation option value for value \a v, string \a o, and help \a h
    void propagation(int v, const char* o, const char* h = NULL);
    /// Return propagation value
    int propagation(void) const;
    
    /// Set default integer consistency level
    void icl(IntConLevel i);
    /// Return integer consistency level
    IntConLevel icl(void) const;
    
    /// Set default branching value
    void branching(int v);
    /// Add branching option value for value \a v, string \a o, and help \a h
    void branching(int v, const char* o, const char* h = NULL);
    /// Return branching value
    int branching(void) const;
    //@}
    
    /// \name Search options
    //@{
    /// Set default search value
    void search(int v);
    /// Add search option value for value \a v, string \a o, and help \a h
    void search(int v, const char* o, const char* h = NULL);
    /// Return search value
    int search(void) const;
    
    /// Set default number of solutions to search for
    void solutions(unsigned int n);
    /// Return number of solutions to search for
    unsigned int solutions(void) const;

    /// Set default copy recomputation distance
    void c_d(unsigned int d);
    /// Return copy recomputation distance
    unsigned int c_d(void) const;
    
    /// Set default adaptive recomputation distance
    void a_d(unsigned int d);
    /// Return adaptive recomputation distance
    unsigned int a_d(void) const;
    
    /// Set default node cutoff
    void node(unsigned int n);
    /// Return node cutoff
    unsigned int node(void) const;
    
    /// Set default failure cutoff
    void fail(unsigned int n);
    /// Return failure cutoff
    unsigned int fail(void) const;
    
    /// Set default time cutoff
    void time(unsigned int t);
    /// Return time cutoff
    unsigned int time(void) const;
    //@}

    /// \name Execution options
    //@{
    /// Set default mode
    void mode(ScriptMode em);
    /// Return mode
    ScriptMode mode(void) const;
    
    /// Set default number of iterations
    void iterations(unsigned int i);
    /// Return number of iterations
    unsigned int iterations(void) const;
    
    /// Set default number of samples
    void samples(unsigned int s);
    /// Return number of samples
    unsigned int samples(void) const;
    //@}
    
    /// Destructor
    virtual ~Options(void);
  };

  /**
   * \brief Options for scripts with additional size parameter
   * \ingroup TaskDriverCmd
   */
  class GECODE_DRIVER_EXPORT SizeOptions : public Options {
  protected:
    unsigned int _size; ///< Size value
  public:
    /// Initialize options for script with name \a s
    SizeOptions(const char* s);
    /// Print help text
    virtual void help(void);
    /// Parse options from arguments \a argv (number is \a argc)
    void parse(int& argc, char* argv[]);
    
    /// Set default size
    void size(unsigned int s);
    /// Return size
    unsigned int size(void) const;
  };

}

#include <gecode/driver/options.hpp>

namespace Gecode {

  namespace Driver {
    /**
     * \brief Parametric base-class for scripts
     *
     * All scripts must inherit from this class
     *  - adds printing to scripts
     *  - run allows to execute scripts
     */
    template <class BaseSpace>
    class ScriptBase : public BaseSpace {
    public:
      /// Default constructor
      ScriptBase(void) {}
      /// Constructor used for cloning
      ScriptBase(bool share, ScriptBase& e) : BaseSpace(share,e) {}
      /// Print a solution to \a os
      virtual void print(std::ostream& os) const { (void) os; }
      /// Run script with search engine \a Engine and options \a opt
      template <class Script, template<class> class Engine, class Options>
      static void run(const Options& opt);
    private:
      /// Catch wrong definitions of copy constructor
      explicit ScriptBase(ScriptBase& e);
    };
  }
  
  /**
   * \defgroup TaskDriverScript Script classes
   * \ingroup TaskDriver
   */

  /**
   * \brief Base-class for scripts
   * \ingroup TaskDriverScript
   */
  typedef Driver::ScriptBase<Space> Script;
  /**
   * \brief Base-class for scripts for finding solution of lowest cost
   * \ingroup TaskDriverScript
   */
  typedef Driver::ScriptBase<MinimizeSpace> MinimizeScript;
  /**
   * \brief Base-class for scripts for finding solution of highest cost
   * \ingroup TaskDriverScript
   */
  typedef Driver::ScriptBase<MaximizeSpace> MaximizeScript;

}

#include <gecode/driver/script.hpp>

#endif

// STATISTICS: driver-any
