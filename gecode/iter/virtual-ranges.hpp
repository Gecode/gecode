/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

namespace Gecode { namespace Iter { namespace Ranges { namespace Virt {

  /**
   * \brief Abstract base class for range iterators
   *
   * \ingroup FuncIterRangesVirt
   */
  class Iterator {
  public:
    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    virtual void operator ++(void) = 0;
    /// Test whether iterator is still at a range or done
    virtual bool operator ()(void) = 0;
    //@}
    /// \name Range access
    //@{
    /// Return smallest value of range
    virtual int min(void) const = 0;
    /// Return largest value of range
    virtual int max(void) const = 0;
    /// Return width of range (distance between minimum and maximum)
    virtual unsigned int width(void) const = 0;
    //@}

    /// Virtual destructor
    virtual ~Iterator(void);
  };

  forceinline
  Iterator::~Iterator(void) {}

  /**
   * \brief Adaptor class to virtualize any iterator
   *
   * Provides an iterator with virtual member functions for any template-based
   * iterator.
   *
   * \ingroup FuncIterRangesVirt
   */
  template<class I>
  class RangesTemplate : public Iterator {
  private:
    /// Check that \a I is a range iterator;
    IsRangeIter<I> _checkI;
  private:
    /// The template-based iterator
    I i;
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize with iterator \a i
    RangesTemplate(I& i);
    /// Initialize with iterator \a i
    void init(I& i);
    //@}
    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    virtual void operator ++(void);
    /// Test whether iterator is still at a range or done
    virtual bool operator ()(void);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    virtual int min(void) const;
    /// Return largest value of range
    virtual int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    virtual unsigned int width(void) const;
    //@}
  };

  template<class I>
  RangesTemplate<I>::RangesTemplate(I& i0) : i(i0) {}

  template<class I>
  void
  RangesTemplate<I>::init(I& i0) { i=i0; }

  template<class I>
  bool
  RangesTemplate<I>::operator ()(void) { return i(); }

  template<class I>
  void
  RangesTemplate<I>::operator ++(void) { ++i; }

  template<class I>
  int
  RangesTemplate<I>::min(void) const { return i.min(); }

  template<class I>
  int
  RangesTemplate<I>::max(void) const { return i.max(); }

  template<class I>
  unsigned int
  RangesTemplate<I>::width(void) const { return i.width(); }

}}}}

// STATISTICS: iter-any
