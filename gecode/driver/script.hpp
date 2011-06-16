/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
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

#include <iostream>
#include <iomanip>

#ifndef GECODE_THREADS_WINDOWS
#include <csignal>
#endif

namespace Gecode { namespace Driver {

  /**
   * \brief Stop object based on nodes, failures, and time
   *
   */
  class Cutoff : public Search::Stop {
  private:
    Search::NodeStop* ns; ///< Used node stop object
    Search::FailStop* fs; ///< Used fail stop object
    Search::TimeStop* ts; ///< Used time stop object
    GECODE_DRIVER_EXPORT
    static bool sigint;   ///< Whether search was interrupted using Ctrl-C
    /// Initialize stop object
    Cutoff(unsigned int node, unsigned int fail, unsigned int time)
      : ns((node > 0) ? new Search::NodeStop(node) : NULL),
        fs((fail > 0) ? new Search::FailStop(fail) : NULL),
        ts((time > 0) ? new Search::TimeStop(time) : NULL) {
      sigint = false;
    }
  public:
    /// Reason why search has been stopped
    enum {
      SR_NODE = 1 << 0, ///< Node limit reached
      SR_FAIL = 1 << 1, ///< Fail limit reached
      SR_TIME = 1 << 2, ///< Time limit reached
      SR_INT  = 1 << 3  ///< Interrupted by user
    };
    /// Test whether search must be stopped
    virtual bool stop(const Search::Statistics& s, const Search::Options& o) {
      return
        sigint ||
        ((ns != NULL) && ns->stop(s,o)) ||
        ((fs != NULL) && fs->stop(s,o)) ||
        ((ts != NULL) && ts->stop(s,o));
    }
    /// Report reason why search has been stopped
    int reason(const Search::Statistics& s, const Search::Options& o) {
      return 
        (((ns != NULL) && ns->stop(s,o)) ? SR_NODE : 0) |
        (((fs != NULL) && fs->stop(s,o)) ? SR_FAIL : 0) |
        (((ts != NULL) && ts->stop(s,o)) ? SR_TIME : 0) |
        (sigint                          ? SR_INT  : 0);
    }
    /// Create appropriate stop-object
    static Search::Stop*
    create(unsigned int node, unsigned int fail, unsigned int time,
           bool intr) {
      if ( (!intr) && (node == 0) && (fail == 0) && (time == 0))
        return NULL;
      else
        return new Cutoff(node,fail,time);
    }
#ifdef GECODE_THREADS_WINDOWS
    /// Handler for catching Ctrl-C
    static BOOL interrupt(DWORD t) {
      if (t == CTRL_C_EVENT) {
        sigint = true;
        installCtrlHandler(false,true);
        return true;
      }
      return false;
    }
#else
    /// Handler for catching Ctrl-C
    static void
    interrupt(int) {
      sigint = true;
      installCtrlHandler(false,true);
    }
#endif
    /// Install handler for catching Ctrl-C
    static void installCtrlHandler(bool install, bool force=false) {
      if (force || !sigint) {
#ifdef GECODE_THREADS_WINDOWS
        SetConsoleCtrlHandler( (PHANDLER_ROUTINE) interrupt, install);
#else
        std::signal(SIGINT, install ? interrupt : SIG_DFL);
#endif
      }
    }
    /// Destructor
    ~Cutoff(void) {
      delete ns; delete fs; delete ts;
    }
  };

  /**
   * \brief Get time since start of timer and print user friendly time
   * information.
   */
  GECODE_DRIVER_EXPORT void 
  stop(Support::Timer& t, std::ostream& os);

  /**
   * \brief Compute arithmetic mean of \a n elements in \a t
   */
  GECODE_DRIVER_EXPORT double
  am(double t[], int n);
  
  /**
   * \brief Compute deviation of \a n elements in \a t
   */
  GECODE_DRIVER_EXPORT double
  dev(double t[], int n);
  
#ifdef GECODE_HAS_GIST
  
  /**
   * \brief Traits class for search engines
   */
  template<class Engine>
  class GistEngine {
  };
  
  /// Specialization for DFS
  template<typename S>
  class GistEngine<DFS<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::dfs(root, opt);
    }
  };
  
  /// Specialization for BAB
  template<typename S>
  class GistEngine<BAB<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::bab(root, opt);
    }
  };
  
  /// Specialization for Restart
  template<typename S>
  class GistEngine<Restart<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::bab(root, opt);
    }
  };
  
#endif

  template<class Space>
  template<class Script, template<class> class Engine, class Options>
  void
  ScriptBase<Space>::run(const Options& o) {
    using namespace std;
    try {
      switch (o.mode()) {
      case SM_GIST:
#ifdef GECODE_HAS_GIST
        {
          Gist::Print<Script> pi(o.name());
          Gist::VarComparator<Script> vc(o.name());
          Gist::Options opt;
          opt.inspect.click(&pi);
          opt.inspect.compare(&vc);
          opt.clone = false;
          opt.c_d   = o.c_d();
          opt.a_d   = o.a_d();
          for (int i=0; o.inspect.click(i) != NULL; i++)
            opt.inspect.click(o.inspect.click(i));
          for (int i=0; o.inspect.solution(i) != NULL; i++)
            opt.inspect.solution(o.inspect.solution(i));
          for (int i=0; o.inspect.move(i) != NULL; i++)
            opt.inspect.move(o.inspect.move(i));
          for (int i=0; o.inspect.compare(i) != NULL; i++)
            opt.inspect.compare(o.inspect.compare(i));
          Script* s = new Script(o);
          (void) GistEngine<Engine<Script> >::explore(s, opt);
        }
        break;
        // If Gist is not available, fall through
#endif
      case SM_SOLUTION:
        {
          cout << o.name() << endl;
          Support::Timer t;
          int i = o.solutions();
          t.start();
          Script* s = new Script(o);
          unsigned int n_p = s->propagators();
          unsigned int n_b = s->branchers();
          Search::Options so;
          so.threads = o.threads();
          so.c_d     = o.c_d();
          so.a_d     = o.a_d();
          so.stop    = Cutoff::create(o.node(),o.fail(), o.time(), 
                                      o.interrupt());
          so.clone   = false;
          if (o.interrupt())
            Cutoff::installCtrlHandler(true);
          Engine<Script> e(s,so);
          do {
            Script* ex = e.next();
            if (ex == NULL)
              break;
            ex->print(std::cout);
            delete ex;
          } while (--i != 0);
          if (o.interrupt())
            Cutoff::installCtrlHandler(false);
          Search::Statistics stat = e.statistics();
          cout << endl;
          if (e.stopped()) {
            cout << "Search engine stopped..." << endl
                 << "\treason: ";
            int r = static_cast<Cutoff*>(so.stop)->reason(stat,so);
            if (r & Cutoff::SR_INT)
              cout << "user interrupt " << endl;
            else {
              if (r & Cutoff::SR_NODE)
                cout << "node ";
              if (r & Cutoff::SR_FAIL)
                cout << "fail ";
              if (r & Cutoff::SR_TIME)
                cout << "time ";
              cout << "limit reached" << endl << endl;
            }
          }
          cout << "Initial" << endl
               << "\tpropagators: " << n_p << endl
               << "\tbranchers:   " << n_b << endl
               << endl
               << "Summary" << endl
               << "\truntime:      ";
          stop(t, cout);
          cout << endl
               << "\tsolutions:    "
               << ::abs(static_cast<int>(o.solutions()) - i) << endl
               << "\tpropagations: " << stat.propagate << endl
               << "\tnodes:        " << stat.node << endl
               << "\tfailures:     " << stat.fail << endl
               << "\tpeak depth:   " << stat.depth << endl
               << "\tpeak memory:  "
               << static_cast<int>((stat.memory+1023) / 1024) << " KB"
               << endl;
          delete so.stop;
        }
        break;
      case SM_STAT:
        {
          cout << o.name() << endl;
          Support::Timer t;
          int i = o.solutions();
          t.start();
          Script* s = new Script(o);
          unsigned int n_p = s->propagators();
          unsigned int n_b = s->branchers();
          Search::Options so;
          so.clone   = false;
          so.threads = o.threads();
          so.c_d     = o.c_d();
          so.a_d     = o.a_d();
          so.stop    = Cutoff::create(o.node(),o.fail(), o.time(),
                                      o.interrupt());
          if (o.interrupt())
            Cutoff::installCtrlHandler(true);
          Engine<Script> e(s,so);
          do {
            Script* ex = e.next();
            if (ex == NULL)
              break;
            delete ex;
          } while (--i != 0);
          if (o.interrupt())
            Cutoff::installCtrlHandler(false);
          Search::Statistics stat = e.statistics();
          cout << endl
               << "\tpropagators:  " << n_p << endl
               << "\tbranchers:    " << n_b << endl
               << "\truntime:      ";
          stop(t, cout);
          cout << endl
               << "\tsolutions:    "
               << ::abs(static_cast<int>(o.solutions()) - i) << endl
               << "\tpropagations: " << stat.propagate << endl
               << "\tnodes:        " << stat.node << endl
               << "\tfailures:     " << stat.fail << endl
               << "\tpeak depth:   " << stat.depth << endl
               << "\tpeak memory:  "
               << static_cast<int>((stat.memory+1023) / 1024) << " KB"
               << endl;
        }
        break;
      case SM_TIME:
        {
          cout << o.name() << endl;
          Support::Timer t;
          double* ts = new double[o.samples()];
          bool stopped = false;
          for (unsigned int s = o.samples(); !stopped && s--; ) {
            t.start();
            for (unsigned int k = o.iterations(); !stopped && k--; ) {
              unsigned int i = o.solutions();
              Script* s = new Script(o);
              Search::Options so;
              so.clone   = false;
              so.threads = o.threads();
              so.c_d     = o.c_d();
              so.a_d     = o.a_d();
              so.stop    = Cutoff::create(o.node(),o.fail(), o.time(), false);
              Engine<Script> e(s,so);
              do {
                Script* ex = e.next();
                if (ex == NULL)
                  break;
                delete ex;
              } while (--i != 0);
              if (e.stopped())
                stopped = true;
              delete so.stop;
            }
            ts[s] = t.stop() / o.iterations();
          }
          if (stopped) {
            cout << "\tSTOPPED" << endl;
          } else {
            double m = am(ts,o.samples());
            double d = dev(ts,o.samples()) * 100.0;
            cout << "\tRuntime: "
                 << setw(20) << right
                 << showpoint << fixed
                 << setprecision(6) << m << "ms"
                 << setprecision(2) << " (" << d << "% deviation)"
                 << endl;
          }
          delete [] ts;
        }
        break;
      }
    } catch (Exception e) {
      cerr << "Exception: " << e.what() << "." << endl
           << "Stopping..." << endl;
      exit(EXIT_FAILURE);
    }
  }

}}

// STATISTICS: driver-any
