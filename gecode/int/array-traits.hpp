/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

    class IntVarArgs;
    class IntVarArray;
    class BoolVarArgs;
    class BoolVarArray;
    class IntArgs;

    /// Traits of %IntVarArgs
    template <>
    class ArrayTraits<VarArgArray<IntVar>> {
    public:
        using StorageType = IntVarArgs;
        using ValueType = IntVar;
        using ArgsType = IntVarArgs;
    };

    /// Traits of %IntVarArray
    template <>
    class ArrayTraits<VarArray<IntVar>> {
    public:
        using StorageType = IntVarArray;
        using ValueType = IntVar;
        using ArgsType = IntVarArgs;
    };

    /// Traits of %BoolVarArgs
    template <>
    class ArrayTraits<VarArgArray<BoolVar>> {
    public:
        using StorageType = BoolVarArgs;
        using ValueType = BoolVar;
        using ArgsType = BoolVarArgs;
    };

    /// Traits of %BoolVarArray
    template <>
    class ArrayTraits<VarArray<BoolVar>> {
    public:
        using StorageType = BoolVarArray;
        using ValueType = BoolVar;
        using ArgsType = BoolVarArgs;
    };

    /// Traits of %IntArgs
    template <>
    class ArrayTraits<ArgArray<int>> {
    public:
        using StorageType = IntArgs;
        using ValueType = int;
        using ArgsType = IntArgs;
    };

    /// Traits of %IntSetArgs
    template <>
    class ArrayTraits<ArgArray<IntSet>> {
    public:
        using StorageType = IntSetArgs;
        using ValueType = IntSet;
        using ArgsType = IntSetArgs;
    };

    /// Traits of %IntVarArray
    template <>
    class ArrayTraits<IntVarArray> {
    public:
        using StorageType = IntVarArray;
        using ValueType = IntVar;
        using ArgsType = IntVarArgs;
    };

    /// Traits of %IntVarArgs
    template <>
    class ArrayTraits<IntVarArgs> {
    public:
        using StorageType = IntVarArgs;
        using ValueType = IntVar;
        using ArgsType = IntVarArgs;
    };

    /// Traits of %IntArgs
    template <>
    class ArrayTraits<IntArgs> {
    public:
        using StorageType = IntArgs;
        using ValueType = int;
        using ArgsType = IntArgs;
    };

    /// Traits of %BoolVarArray
    template <>
    class ArrayTraits<BoolVarArray> {
    public:
        using StorageType = BoolVarArray;
        using ValueType = BoolVar;
        using ArgsType = BoolVarArgs;
    };

    /// Traits of %BoolVarArgs
    template <>
    class ArrayTraits<BoolVarArgs> {
    public:
        using StorageType = BoolVarArgs;
        using ValueType = BoolVar;
        using ArgsType = BoolVarArgs;
    };

} // namespace Gecode

// STATISTICS: int-other
