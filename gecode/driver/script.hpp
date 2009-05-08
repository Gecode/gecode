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

#include <gecode/support/timer.hh>

#include <iostream>
#include <iomanip>

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
    /// Initialize stop object
    Cutoff(unsigned int node, unsigned int fail, unsigned int time)
      : ns((node > 0) ? new Search::NodeStop(node) : NULL),
        fs((fail > 0) ? new Search::FailStop(fail) : NULL),
        ts((time > 0) ? new Search::TimeStop(time) : NULL) {}
  public:
    /// Test whether search must be stopped
    virtual bool stop(const Search::Statistics& s, const Search::Options& o) {
      return
        ((ns != NULL) && ns->stop(s,o)) ||
        ((fs != NULL) && fs->stop(s,o)) ||
        ((ts != NULL) && ts->stop(s,o));
    }
    /// Create appropriate stop-object
    static Search::Stop*
    create(unsigned int node, unsigned int fail, unsigned int time) {
      if ((node == 0) && (fail == 0) && (time == 0))
        return NULL;
      else
        return new Cutoff(node,fail,time);
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
  template <class Engine>
  class GistEngine {
  };
  
  /// Specialization for DFS
  template <typename S>
  class GistEngine<DFS<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::dfs(root, opt);
    }
  };
  
  /// Specialization for LDS
  template <typename S>
  class GistEngine<LDS<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::dfs(root, opt);
    }
  };
  
  /// Specialization for BAB
  template <typename S>
  class GistEngine<BAB<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::bab(root, opt);
    }
  };
  
  /// Specialization for Restart
  template <typename S>
  class GistEngine<Restart<S> > {
  public:
    static void explore(S* root, const Gist::Options& opt) {
      (void) Gist::bab(root, opt);
    }
  };
  
#endif

  template <class Space>
  template <class Script, template<class> class Engine, class Options>
  void
  ScriptBase<Space>::run(const Options& o) {
    using namespace std;
    try {
      switch (o.mode()) {
      case SM_GIST:
#ifdef GECODE_HAS_GIST
        {
          Gist::Print<Script> pi(o.name());
          Gist::Options opt;
          opt.inspect.click(&pi);
          opt.clone = false;
          opt.c_d   = o.c_d();
          opt.a_d   = o.a_d();
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
          unsigned int n_b = s->branchings();
          Search::Options so;
          so.threads = o.threads();
          so.c_d     = o.c_d();
          so.a_d     = o.a_d();
          so.stop    = Cutoff::create(o.node(),o.fail(), o.time());
          so.clone   = false;
          Engine<Script> e(s,so);
          do {
            Script* ex = e.next();
            if (ex == NULL)
              break;
            ex->print(std::cout);
            delete ex;
          } while (--i != 0);
          Search::Statistics stat = e.statistics();
          cout << endl;
          cout << "Initial" << endl
               << "\tpropagators:  " << n_p << endl
               << "\tbranchings:   " << n_b << endl
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
          unsigned int n_b = s->branchings();
          Search::Options so;
          so.clone   = false;
          so.threads = o.threads();
          so.c_d     = o.c_d();
          so.a_d     = o.a_d();
          so.stop    = Cutoff::create(o.node(),o.fail(), o.time());
          Engine<Script> e(s,so);
          do {
            Script* ex = e.next();
            if (ex == NULL)
              break;
            delete ex;
          } while (--i != 0);
          Search::Statistics stat = e.statistics();
          cout << endl
               << "\tpropagators:  " << n_p << endl
               << "\tbranchings:   " << n_b << endl
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
          for (unsigned int s = o.samples(); s--; ) {
            t.start();
            for (unsigned int k = o.iterations(); k--; ) {
              unsigned int i = o.solutions();
              Script* s = new Script(o);
              Search::Options so;
              so.clone   = false;
              so.threads = o.threads();
              so.c_d     = o.c_d();
              so.a_d     = o.a_d();
              so.stop    = Cutoff::create(o.node(),o.fail(), o.time());
              Engine<Script> e(s,so);
              do {
                Script* ex = e.next();
                if (ex == NULL)
                  break;
                delete ex;
              } while (--i != 0);
            }
            ts[s] = t.stop() / o.iterations();
          }
          double m = am(ts,o.samples());
          double d = dev(ts,o.samples()) * 100.0;
          delete[] ts;
          cout << "\tRuntime: "
               << setw(20) << right
               << showpoint << fixed
               << setprecision(6) << m << "ms"
               << setprecision(2) << " (" << d << "% deviation)"
               << endl;
        }
        break;
      }
    } catch (Exception e) {
      cout << "Exception: " << e.what() << "." << endl
           << "Stopping..." << endl;
    }
  }

}}

// STATISTICS: driver-any
