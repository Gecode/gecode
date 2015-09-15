/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
 *     Guido Tack, 2010
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

namespace Gecode { namespace Int { namespace Unary {

  /// Event for task
  class Event {
  public:
    /// Event type for task with order in which they are processed
    enum Type {
      LRT = 0, ///< Latest required time of task
      LCT = 1, ///< Latest completion time of task
      EST = 2, ///< Earliest start time of task
      ZRO = 3, ///< Zero-length task start time
      ERT = 4, ///< Earliest required time of task
      END = 5  ///< End marker
    };
    Type e; ///< Type of event
    int t;  ///< Time of event
    int i;  ///< Number of task
    /// Initialize event
    void init(Type e, int t, int i);
    /// Order among events
    bool operator <(const Event& e) const;
  };

  /// Print event \a e on stream \a os
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Event& e);

  forceinline void
  Event::init(Event::Type e0, int t0, int i0) {
    e=e0; t=t0; i=i0;
  }

  forceinline bool
  Event::operator <(const Event& e) const {
    if (this->t == e.t)
      return this->e < e.e;
    return this->t < e.t;
  }

  template<class Char, class Traits>
  inline std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Event& e) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '[';
    switch (e.e) {
    case Event::LRT: s << "LRT"; break;
    case Event::LCT: s << "LCT"; break;
    case Event::EST: s << "EST"; break;
    case Event::ZRO: s << "ZRO"; break;
    case Event::ERT: s << "ERT"; break;
    default: GECODE_NEVER;
    }
    s << ',' << e.t << ',' << e.i << ']';
    return os << s.str();
  }




  // Basic propagation (timetabling)
  template<class Task>
  ExecStatus
  basic(Space& home, bool& subsumed, TaskArray<Task>& t) {
#ifdef PRINTIT
    std::cout << "basic(" << t << ")" << std::endl;
#endif
    subsumed = false;

    Region r(home);

    Event* e = r.alloc<Event>(4*t.size()+1);

    // Initialize events
    bool assigned=true;
    {
      bool required=false;
      int n=0;
      for (int i=t.size(); i--; ) 
        if (t[i].assigned()) {
          // Only add required part
          //          e[n++].init(Event::EST,t[i].est(),i);
          //          e[n++].init(Event::LCT,t[i].lct(),i);
          if (t[i].pmin() > 0) {
            required = true;
            e[n++].init(Event::ERT,t[i].lst(),i);
            e[n++].init(Event::LRT,t[i].ect(),i);
          } else if (t[i].pmax() == 0) {
            required = true;
            e[n++].init(Event::ZRO,t[i].lst(),i);
          }
        } else {
          assigned = false;
          e[n++].init(Event::EST,t[i].est(),i);
          e[n++].init(Event::LCT,t[i].lct(),i);
          // Check whether task has required part
          if (t[i].lst() < t[i].ect()) {
            required = true;
            e[n++].init(Event::ERT,t[i].lst(),i);
            e[n++].init(Event::LRT,t[i].ect(),i);
          }
        }
      
      // Check whether no task has a required part
      if (!required) {
        subsumed = assigned;
        return ES_FIX;
      }
      
      // Write end marker
      e[n++].init(Event::END,Limits::infinity,-1);
      
      // Sort events
      Support::quicksort(e, n);
    }

    // Whether resource is free
    bool free = true;
    // Set of current but not required tasks
    Support::BitSet<Region> tasks(r,static_cast<unsigned int>(t.size()));

    // Process events
    while (e->e != Event::END) {
      // Current time
      int time = e->t;

      // Process events for completion of required part
      for ( ; (e->t == time) && (e->e == Event::LRT); e++) {
        if (t[e->i].mandatory()) {
          tasks.set(static_cast<unsigned int>(e->i)); 
          free = true;
        }
#ifdef PRINTIT
        std::cout << *e << " free=" << free << " tasks={";
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          std::cout << j.val() << ",";
        std::cout << "}" << std::endl;
#endif
      }
      // Process events for completion of task
      for ( ; (e->t == time) && (e->e == Event::LCT); e++) {
        tasks.clear(static_cast<unsigned int>(e->i));
#ifdef PRINTIT
        std::cout << *e << " free=" << free << " tasks={";
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          std::cout << j.val() << ",";
        std::cout << "}" << std::endl;
#endif
      }
      // Process events for start of task
      for ( ; (e->t == time) && (e->e == Event::EST); e++) {
        tasks.set(static_cast<unsigned int>(e->i));
#ifdef PRINTIT
        std::cout << *e << " free=" << free << " tasks={";
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          std::cout << j.val() << ",";
        std::cout << "}" << std::endl;
#endif
      }
      // Process events for zero-length task
      for ( ; (e->t == time) && (e->e == Event::ZRO); e++) {
#ifdef PRINTIT
        std::cout << *e << " free=" << free << " tasks={";
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          std::cout << j.val() << ",";
        std::cout << "}" << std::endl;
#endif
        if (!free)
          return ES_FAILED;
        assert(free);
      }

      // norun start time for 0-length tasks
      int zltime = time;
      // Process events for start of required part
      for ( ; (e->t == time) && (e->e == Event::ERT); e++) {
        if (t[e->i].mandatory()) {
          tasks.clear(static_cast<unsigned int>(e->i)); 
          if (!free)
            return ES_FAILED;
          free = false;
          zltime = time+1;
        } else if (t[e->i].optional() && !free) {
          GECODE_ME_CHECK(t[e->i].excluded(home));
        }
#ifdef PRINTIT
        std::cout << *e << " free=" << free << " tasks={";
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          std::cout << j.val() << ",";
        std::cout << "}" << std::endl;
#endif
      }

      if (!free)
        for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
             j(); ++j) 
          // Task j cannot run from time to next time - 1
          if (t[j.val()].mandatory()) {
            if (t[j.val()].pmin() > 0) {

#ifdef PRINTIT
              std::cout << "\tNorun (" << j.val() << ") = [" 
                        << time << ".." << e->t-1 << "]" << std::endl;
#endif
              GECODE_ME_CHECK(t[j.val()].norun(home, time, e->t - 1));
            } else {
#ifdef PRINTIT
              std::cout << "\tNorun (" << j.val() << ") = [" 
                        << zltime << ".." << e->t-1 << "]" << std::endl;
#endif
              GECODE_ME_CHECK(t[j.val()].norun(home, zltime, e->t - 1));
            }
          }

    }

    subsumed = assigned;
    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
