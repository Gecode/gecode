/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
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
#ifndef __GECODE_SEARCH_PARALLEL_LOG_HH__
#define __GECODE_SEARCH_PARALLEL_LOG_HH__

#include <gecode/support.hh>
#include <string>
#include <sstream>
#include <iostream>

namespace Gecode { namespace Support {
  class Log;

  /**
   * \brief Class to log object used to be printed
   *
   * \ingroup FuncSupport
   */
  class LogObject {
  friend class Log;
  private:
  public:
    /// Output buffer of object
    std::string m_buffer;
  public:
    /// Constructors
    LogObject(void);
    LogObject(const LogObject& lo);
    LogObject(bool b);
    LogObject(int i);
    LogObject(unsigned int u);
    LogObject(unsigned long int uil);
    LogObject(float f);
    LogObject(double d);
    LogObject(const char * str);
    LogObject(std::string std);

    /// Overload of += operator
    LogObject& operator +=(const LogObject& o);
  };

  forceinline
  LogObject::LogObject(void) { }

  forceinline
  LogObject::LogObject(const LogObject& lo) {
    m_buffer += lo.m_buffer;
  }

  forceinline
  LogObject::LogObject(bool b) {
    m_buffer += (b?"TRUE":"FALSE");
  }

  forceinline
  LogObject::LogObject(int i) {
    std::ostringstream oss;
    oss << i;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(unsigned int u) {
    std::ostringstream oss;
    oss << u;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(unsigned long int ul) {
    std::ostringstream oss;
    oss << ul;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(float f) {
    std::ostringstream oss;
    oss << f;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(double d) {
    std::ostringstream oss;
    oss << d;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(const char * str) {
    std::ostringstream oss;
    oss << str;
    m_buffer += oss.str();
  }

  forceinline
  LogObject::LogObject(std::string std) {
    m_buffer += std;
  }

  forceinline
  LogObject& LogObject::operator +=(const LogObject& o) {
    m_buffer += o.m_buffer;
    return *this;
  }

  template <class T>
  LogObject operator <<(LogObject lo, T e) {
    LogObject o1(lo);
    LogObject o2(e);
    o1 += o2;
    return o1;
  }

  /**
   * \brief Class to output log
   *
   * Support log from multi-thread search engines
   *
   * \ingroup FuncSupport
   */
  class Log {
  private:
    // Singleton instance
    static Log* m_pLog;
    /// Default constructor (disabled)
    Log(void) {}
    /// Copy constructor (disabled)
    Log(const Log&) {}
    /// Assignment operator (disabled)
    const Log& operator =(const Log&) { return *this; }
  protected:
    /// Mutex for access to output stream
    Support::Mutex _m_output;
  public:
    // Static method to get the instance
    static Log& instance() { assert(m_pLog); return *m_pLog; }

    // Write buffer of LogObject \a lo
    void write(LogObject lo);
    // Write buffer of LogObject \a lo, when thread \th is known
    void write(int th, LogObject lo);
  };

  forceinline
  void Log::write(LogObject lo) {
    _m_output.acquire();
    std::cout << lo.m_buffer << std::flush;
    _m_output.release();
  }

  forceinline
  void Log::write(int th, LogObject lo) {
    _m_output.acquire();
    std::cout << "[" << th << "] " << lo.m_buffer;
    _m_output.release();
  }

}}

#ifdef LOG_AUDIT
  #define LOG_OUT(X) Gecode::Support::Log::instance().write(Gecode::Support::LogObject() << "    " << X)
  #define LOG_OUTN(NTH,X) Gecode::Support::Log::instance().write(NTH,Gecode::Support::LogObject() << X)
#else
  #define LOG_OUT(X)
  #define LOG_OUTN(NTH,X)
#endif

#endif

// STATISTICS: support-any
