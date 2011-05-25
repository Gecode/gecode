/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Gregory Crosswhite <gcross@phys.washington.edu>
 *
 *  Copyright:
 *     Gregory Crosswhite, 2011
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include "test/test.hh"

/// Check the test result and handle failed test
#define CHECK_TEST(T,M)                                         \
if (opt.log)                                                    \
  olog << ind(3) << "Check: " << (M) << std::endl;              \
if (!(T)) {                                                     \
  problem = (M); goto failed;                         \
}

/// Start new test
#define START_TEST(T)                                           \
  if (opt.log) {                                                \
     olog.str("");                                              \
     olog << ind(2) << "Testing: " << (T) << std::endl;         \
  }                                                             \
  test = (T);

namespace Test { 

  /// Tests for arrays
  namespace Array {

  /// Test name prefix
  static const std::string prefix("Array::Iterator::");

  /// %Base class for testing iterators
  class Iterator : public Test::Base {
  protected:
    /// Maximum array size
    static const int n = 16;
    /// Initialize test
    Iterator(const std::string& name) : Test::Base(prefix + name) {}
    /// Perform actual tests
    template<class Array> bool runTestForArray(Array& a) {
      // Test/problem information.
      const char* test    = "NONE";
      const char* problem = "NONE";
      // Constant reference to the array
      const Array& const_a = a;

      START_TEST("Iteration");
      {
        typedef typename Array::reference reference;
        typedef typename Array::pointer pointer;
        typedef typename Array::iterator iterator;
        const iterator begin = a.begin(), end = a.end();
        CHECK_TEST(end-begin==a.size(),"Distance != size");
        int index = 0;
        iterator iter = begin;
        for(; iter != end; ++iter, ++index) {
          reference ref = *iter;
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&a[index],"Iterator points to the wrong element (going forward)");
        }
        CHECK_TEST(index==a.size(),"Iteration covered the wrong number of elements (going forward)");
        for(; iter != begin; --iter, --index) {
          reference ref = *(iter-1);
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&a[index-1],"Iterator points to the wrong element (going backwards)");
        }
        CHECK_TEST(index==0,"Iteration covered the wrong number of elements (going backward)");
      }
      START_TEST("Read-only iteration");
      {
        typedef typename Array::const_reference reference;
        typedef typename Array::const_pointer pointer;
        typedef typename Array::const_iterator iterator;
        const iterator begin = const_a.begin(), end = const_a.end();
        CHECK_TEST(end-begin==const_a.size(),"Distance != size");
        int index = 0;
        iterator iter = begin;
        for(; iter != end; ++iter, ++index) {
          reference ref = *iter;
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&const_a[index],"Iterator points to the wrong element (going forward)");
        }
        CHECK_TEST(index==const_a.size(),"Iteration covered the wrong number of elements (going forward)");
        for(; iter != begin; --iter, --index) {
          reference ref = *(iter-1);
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&const_a[index-1],"Iterator points to the wrong element (going backwards)");
        }
        CHECK_TEST(index==0,"Iteration covered the wrong number of elements (going backward)");
      }

      START_TEST("Reverse iteration");
      {
        typedef typename Array::reference reference;
        typedef typename Array::pointer pointer;
        typedef typename Array::reverse_iterator iterator;
        const iterator begin = a.rbegin(), end = a.rend();
        CHECK_TEST(end-begin==a.size(),"Distance != size");
        int index = a.size();
        iterator iter = begin;
        for(; iter != end; ++iter, --index) {
          reference ref = *iter;
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&a[index-1],"Iterator points to the wrong element (going forward)");
        }
        CHECK_TEST(index==0,"Iteration covered the wrong number of elements (going forward)");
        for(; iter != begin; --iter, ++index) {
          reference ref = *(iter-1);
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&a[index],"Iterator points to the wrong element (going backwards)");
        }
        CHECK_TEST(index==a.size(),"Iteration covered the wrong number of elements (going backward)");
      }

      START_TEST("Reverse read-only iteration");
      {
        typedef typename Array::const_reference reference;
        typedef typename Array::const_pointer pointer;
        typedef typename Array::const_reverse_iterator iterator;
        const iterator begin = const_a.rbegin(), end = const_a.rend();
        CHECK_TEST(end-begin==const_a.size(),"Distance != size");
        int index = a.size();
        iterator iter = begin;
        for(; iter != end; ++iter, --index) {
          reference ref = *iter;
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&const_a[index-1],"Iterator points to the wrong element (going forward)");
        }
        CHECK_TEST(index==0,"Iteration covered the wrong number of elements (going forward)");
        for(; iter != begin; --iter, ++index) {
          reference ref = *(iter-1);
          const pointer ptr = &ref;
          CHECK_TEST(ptr==&const_a[index],"Iterator points to the wrong element (going backwards)");
        }
        CHECK_TEST(index==a.size(),"Iteration covered the wrong number of elements (going backward)");
      }

      return true;
    failed:
      if (opt.log)
        olog << "FAILURE" << std::endl
        << ind(1) << "Test:       " << test << std::endl
        << ind(1) << "Problem:    " << problem << std::endl;
      return false;
    }
  };

  /// Test space
  class TestSpace : public Gecode::Space {
  public:
    TestSpace(void) : Space() {}
    TestSpace(bool share, TestSpace& s) : Space(share,s) {}
    virtual Space* copy(bool share) {
      return new TestSpace(share,*this);
    }
  };

  /// %Class for testing the VarArray iterator
  class VarArrayIterator : public Iterator {
  protected:
    /// Maximum array size
    static const int n = 16;
    /// Array type being tested
    typedef Gecode::VarArray<Gecode::IntVar> Array;
  public:
    /// Initialize test
    VarArrayIterator(void) : Iterator("VarArray") {}
    /// Perform actual tests
    bool run(void) {
      // Space for the test
      TestSpace s;
      // VarArray for the test
      Array a(s,rand(n));
      // Run the iterator test
      return runTestForArray(a);
    }
  } varArrayIteratorTest;

  /// %Class for testing the VarArgs iterator
  class VarArgsIterator : public Iterator {
  protected:
    /// Maximum array size
    static const int n = 16;
    /// Array type being tested
    typedef Gecode::ArgArrayBase<int> Array;
  public:
    /// Initialize test
    VarArgsIterator(void) : Iterator("VarArgs") {}
    /// Perform actual tests
    bool run(void) {
      // Space for the test
      TestSpace s;
      // VarArray for the test
      Array a(rand(n));
      // Run the iterator test
      return runTestForArray(a);
    }
  } varArgsIteratorTest;

  /// %Class for testing the ViewArray iterator
  class ViewArrayIterator : public Iterator {
  protected:
    /// Maximum array size
    static const int n = 16;
    /// Array type being tested
    typedef Gecode::ViewArray<Gecode::IntVar> Array;
  public:
    /// Initialize test
    ViewArrayIterator(void) : Iterator("ViewArray") {}
    /// Perform actual tests
    bool run(void) {
      // Space for the test
      TestSpace s;
      // VarArray for the test
      Array a(s,rand(n));
      // Run the iterator test
      return runTestForArray(a);
    }
  } viewArrayIteratorTest;

  /// %Class for testing the SharedArray iterator
  class SharedArrayIterator : public Iterator {
  protected:
    /// Maximum array size
    static const int n = 16;
    /// Array type being tested
    typedef Gecode::SharedArray<int> Array;
  public:
    /// Initialize test
    SharedArrayIterator(void) : Iterator("SharedArray") {}
    /// Perform actual tests
    bool run(void) {
      // SharedArray for the test
      Array a(rand(n));
      // Run the iterator test
      return runTestForArray(a);
    }
  } sharedArrayIteratorTest;

}}

// STATISTICS: test-kernel
