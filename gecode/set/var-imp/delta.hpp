/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *     Guido Tack, 2007
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

namespace Gecode { namespace Set {

    forceinline
    SetDelta::SetDelta()
        : _glb_min(1), _glb_max(0), _lub_min(1), _lub_max(0) {}

    forceinline
    SetDelta::SetDelta(int glb_min, int glb_max, int lub_min, int lub_max)
        : _glb_min(glb_min), _glb_max(glb_max), _lub_min(lub_min), _lub_max(lub_max) {}

    forceinline int
    SetDelta::glb_min() const {
        return _glb_min;
    }
    forceinline int
    SetDelta::glb_max() const {
        return _glb_max;
    }
    forceinline int
    SetDelta::lub_min() const {
        return _lub_min;
    }
    forceinline int
    SetDelta::lub_max() const {
        return _lub_max;
    }
    forceinline bool
    SetDelta::glb_any() const {
        return _glb_min > _glb_max;
    }
    forceinline bool
    SetDelta::lub_any() const {
        return _lub_min > _lub_max;
    }

}} // namespace Gecode::Set

// STATISTICS: set-var
