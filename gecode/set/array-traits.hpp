/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode {

    class SetVarArgs;
    class SetVarArray;

    /// Traits of %VarArgArray&lt;%SetVar>
    template <>
    class ArrayTraits<VarArgArray<SetVar>> {
    public:
        using StorageType = SetVarArgs;
        using ValueType = SetVar;
        using ArgsType = SetVarArgs;
    };

    /// Traits of %VarArray&lt;%SetVar>
    template <>
    class ArrayTraits<VarArray<SetVar>> {
    public:
        using StorageType = SetVarArray;
        using ValueType = SetVar;
        using ArgsType = SetVarArgs;
    };

    /// Traits of %SetVarArray
    template <>
    class ArrayTraits<SetVarArray> {
    public:
        using StorageType = SetVarArray;
        using ValueType = SetVar;
        using ArgsType = SetVarArgs;
    };
    /// Traits of %SetVarArgs
    template <>
    class ArrayTraits<SetVarArgs> {
    public:
        using StorageType = SetVarArgs;
        using ValueType = SetVar;
        using ArgsType = SetVarArgs;
    };

} // namespace Gecode

// STATISTICS: set-other
