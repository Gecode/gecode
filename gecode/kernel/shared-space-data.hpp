/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

namespace Gecode { namespace Kernel {

  /// Class to store data shared among several spaces
  class SharedSpaceData : public SharedHandle {
  public:
    /// The shared data
    class Data : public SharedHandle::Object {
    public:
      /// The shared memory area
      SharedMemory sm;
      /// The global propagator information
      GPI gpi;
      /// Default constructor
      Data(void);
      /// Destructor
      virtual ~Data(void);
    };
    /// Initialize
    SharedSpaceData(void);
    /// Initialize from \a ssd
    SharedSpaceData(const SharedSpaceData& ssd);
    /// Assignment operator
    SharedSpaceData& operator =(const SharedSpaceData& r);
    /// Delete
    ~SharedSpaceData(void);
    /// Provide access
    Data& data(void) const;
  };



  forceinline
  SharedSpaceData::Data::Data(void) {}

  forceinline
  SharedSpaceData::Data::~Data(void) {}



  forceinline
  SharedSpaceData::SharedSpaceData(void)
    : SharedHandle(new Data) {}

  forceinline
  SharedSpaceData::SharedSpaceData(const SharedSpaceData& ssd)
    : SharedHandle(ssd) {}

  forceinline SharedSpaceData&
  SharedSpaceData::operator =(const SharedSpaceData& ssd) {
    (void) SharedHandle::operator =(ssd);
    return *this;
  }

  forceinline
  SharedSpaceData::~SharedSpaceData(void) {}

  forceinline SharedSpaceData::Data&
  SharedSpaceData::data(void) const {
    return static_cast<Data&>(*object());
  }

}}

// STATISTICS: kernel-memory
