/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

namespace Gecode { namespace Int {

  forceinline void
  Event::init(Event::Type e0, int t0, int i0) {
    ei=static_cast<unsigned int>(e0 | (i0 << 3)); t=t0;
  }

  forceinline Event::Type
  Event::type(void) const {
    return static_cast<Type>(ei & 7);
  }
  forceinline int
  Event::time(void) const {
    return t;
  }
  forceinline int
  Event::idx(void) const {
    return static_cast<int>(ei >> 3);;
  }

  forceinline bool
  Event::operator <(const Event& e) const {
    if (time() == e.time())
      return type() < e.type();
    return time() < e.time();
  }


  template<class Char, class Traits>
  inline std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Event& e) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '[';
    switch (e.type()) {
    case Event::LRT: s << "LRT"; break;
    case Event::LCT: s << "LCT"; break;
    case Event::EST: s << "EST"; break;
    case Event::ZRO: s << "ZRO"; break;
    case Event::ERT: s << "ERT"; break;
    default: GECODE_NEVER;
    }
    s << ',' << e.time() << ',' << e.idx() << ']';
    return os << s.str();
  }


  template<class Task>
  forceinline Event*
  Event::events(Region& r, const TaskArray<Task>& t, bool& assigned) {
    Event* e = r.alloc<Event>(4*t.size()+1);

    // Initialize events
    assigned=true;
    bool required=false;

    int n=0;
    for (int i=0; i<t.size(); i++)
      if (t[i].assigned()) {
        // Only add required part
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

    if (!required)
      return NULL;

    // Sort events
    Support::quicksort(e, n);

    // Write end marker
    e[n++].init(Event::END,Limits::infinity,0);

    return e;
  }

  template<class Task>
  forceinline Event*
  Event::events(Region& r, const TaskArray<Task>& t) {
    Event* e = r.alloc<Event>(2*t.size()+1);

    // Only add assigned and mandatory tasks
    int n=0;
    for (int i=0; i<t.size(); i++)
      if (t[i].assigned() && t[i].mandatory()) {
        if (t[i].pmin() > 0) {
          e[n++].init(Event::ERT,t[i].lst(),i);
          e[n++].init(Event::LRT,t[i].ect(),i);
        } else if (t[i].pmax() == 0) {
          e[n++].init(Event::ZRO,t[i].lst(),i);
        }
      } else {
        assert(!t[i].excluded());
        return NULL;
      }

    // Sort events
    Support::quicksort(e, n);

    // Write end marker
    e[n++].init(Event::END,Limits::infinity,0);

    return e;
  }

}}

// STATISTICS: int-prop
