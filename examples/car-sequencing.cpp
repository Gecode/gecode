/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2009
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <iomanip>

using namespace Gecode;

// Problems
namespace {
    // Problem data
    extern const int* problems[];
    // Number of problems
    extern const unsigned int n_problems;
} // namespace

namespace {
    /**
     * \brief %Options for car sequencing problems
     *
     * \relates CarSequence
     */
    class CarOptions : public SizeOptions {
    private:
        /// Max slack parameter
        Driver::UnsignedIntOption _max_stalls;

    public:
        /// Initialize options for example with name \a s
        CarOptions(const char* s) : SizeOptions(s), _max_stalls("maxstall", "Maximum numbere of stalls", 30) {
            // Add options
            add(_max_stalls);
        }
        /// Parse options from arguments \a argv (number is \a argc)
        void
        parse(int& argc, char* argv[]) {
            SizeOptions::parse(argc, argv);
        }
        /// Get max stalls
        int
        max_stalls(void) const {
            return _max_stalls.value();
        }
    };

    /**
     * \brief Propagator that pushes all occurrences of a value to the end.
     *
     * This propagator uses a variable array \f$x=\langle
     * x_1,x_2,\ldots,x_n\rangle\f$, a variable \f$y\f$, and a value
     * \f$val\f$. It It makes sure that the last \f$y\f$ variables of
     * \f$x\f$ are assigned the value, and that the value does not
     * appear in the rest of the array. Furthermore, the constraint
     * ensure that \$fval\$f isnot adjacent to \$fval-1\$f.
     *
     * Since the propagator is custom-made for the car sequencing
     * example, it relies on the fact that the value will be equal to
     * the upper bound to speed up computation. For example, it can
     * safely rely on only subscribing to bound events.
     *
     * \relates CarSequence
     */
    template <class View>
    class PushToEnd : public NaryOnePropagator<View, Int::PC_INT_BND> {
    protected:
        using NaryOnePropagator<View, Int::PC_INT_BND>::x;
        using NaryOnePropagator<View, Int::PC_INT_BND>::y;
        int val;

        /// Constructor for cloning \a p
        PushToEnd(Space& home, PushToEnd& p);
        /// Constructor for posting
        PushToEnd(Space& home, ViewArray<View>& x0, View y0, int val0);

    public:
        /// Constructor for rewriting \a p during cloning
        PushToEnd(Space& home, Propagator& p, ViewArray<View>& x0, View y0, int val0);
        /// Copy propagator during cloning
        Actor* copy(Space& home) override;
        /// Perform propagation
        ExecStatus propagate(Space& home, const ModEventDelta& med) override;
        /// Post propagator
        static ExecStatus post(Space& home, ViewArray<View>& x0, View y0, int val0);
    };

    template <class View>
    inline PushToEnd<View>::PushToEnd(Space& home, ViewArray<View>& x0, View y0, int val0)
        : NaryOnePropagator<View, Int::PC_INT_BND>(home, x0, y0), val(val0) {}

    template <class View>
    ExecStatus
    PushToEnd<View>::post(Space& home, ViewArray<View>& x0, View y0, int val0) {
        (void)new (home) PushToEnd<View>(home, x0, y0, val0);
        return ES_OK;
    }

    template <class View>
    inline PushToEnd<View>::PushToEnd(Space& home, PushToEnd<View>& p)
        : NaryOnePropagator<View, Int::PC_INT_BND>(home, p), val(p.val) {}

    template <class View>
    inline PushToEnd<View>::PushToEnd(Space& home, Propagator& p, ViewArray<View>& x0, View y0, int val0)
        : NaryOnePropagator<View, Int::PC_INT_BND>(home, p, x0, y0), val(val0) {}

    template <class View>
    Actor*
    PushToEnd<View>::copy(Space& home) {
        return new (home) PushToEnd<View>(home, *this);
    }

    template <class View>
    ExecStatus
    PushToEnd<View>::propagate(Space& home, const ModEventDelta&) {
        // Find number of required positions
        int min = 0;
        for (int i = x.size(); i-- && x[i].min() >= val - 1;) {
            ++min;
        }
        // Find number of possible positions
        int max = 0;
        {
            int i = x.size();
            while (i--) {
                if (x[i].max() != val)
                    break;
                ++max;
                if (max >= y.max())
                    break;
            }
            // No variables later than max can have value val
            while (i--) {
                GECODE_ME_CHECK(x[i].le(home, val));
            }
        }

        // Constrain y to be in {min..max}
        GECODE_ME_CHECK(y.gq(home, min));
        GECODE_ME_CHECK(y.lq(home, max));

        // At least the y.min() last values have value val
        for (int i = 0, pos = x.size() - 1; i < y.min(); ++i, --pos) {
            GECODE_ME_CHECK(x[pos].eq(home, val));
        }

        return y.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    /** \brief Post PushToEnd propagator.
     */
    void
    push_to_end(Space& home, IntVarArgs x, IntVar y, int val) {
        ViewArray<Int::IntView> vx(home, x);
        Int::IntView vy(y);
        GECODE_ES_FAIL(PushToEnd<Int::IntView>::post(home, vx, vy, val));
    }

} // namespace

/**
 * \brief %Example: Car sequencing
 *
 * See problem 1 at http://www.csplib.org/.
 *
 * This model uses extra stall-slots instead of violations, as proposed
 * in "Combining Forces to Solve the Car Sequencing Problem", Perron
 * and Shaw, CPAIOR 2004.
 *
 * \ingroup Example
 */
class CarSequencing : public Script {
public:
    /// Branching variants
    enum {
        BRANCH_INORDER, ///< Branch from left to right
        BRANCH_MIDDLE   ///< Branch from middle out
    };
    /// Propagation variants
    enum {
        PROP_REGULAR, ///< Use regular constraints
        PROP_CUSTOM   ///< Use custom constraint
    };

protected:
    /// Problem number
    const int problem;
    /// Number of cars
    const int car_count;
    /// Number of options
    const int option_count;
    /// Number of classes
    const int class_count;
    /// Maximum number of stalls
    const int max_stalls;
    /// Stall number
    const int stall_value;
    /// End number
    const int end_value;
    /// Number of stalls (cost to minimize)
    IntVar stall_count;
    /// Number of end markers
    IntVar end_count;
    /// Sequence of cars produced
    IntVarArray s;

public:
    /// Initial model
    CarSequencing(const CarOptions& opt)
        : Script(opt), problem(opt.size()), car_count(problems[problem][0]), option_count(problems[problem][1]),
          class_count(problems[problem][2]), max_stalls(opt.max_stalls()), stall_value(class_count),
          end_value(class_count + 1), stall_count(*this, 0, max_stalls), end_count(*this, 0, max_stalls),
          s(*this, car_count + max_stalls, 0, class_count + 1) {
        // Read problem
        const int* problem_cursor = problems[problem] + 3;

        // Sequence requirements for the options.
        IntArgs max(option_count), block(option_count);
        for (int i = 0; i < option_count; ++i) {
            max[i] = *problem_cursor++;
        }
        for (int i = 0; i < option_count; ++i) {
            block[i] = *problem_cursor++;
        }
        // Number of cars per class
        IntArgs ncc(class_count);
        // What classes require an option
        IntSetArgs classes(option_count);
        int** class_data = new int*[option_count];
        for (int i = option_count; i--;)
            class_data[i] = new int[class_count];
        int* n = new int[option_count];
        for (int i = option_count; i--;)
            n[i] = 0;
        // Read data
        for (int c = 0; c < class_count; ++c) {
            problem_cursor++;
            ncc[c] = *problem_cursor++;
            for (int o = 0; o < option_count; ++o) {
                if (*problem_cursor++) {
                    class_data[o][n[o]++] = c;
                }
            }
        }
        // Transfer specification data to int-sets
        for (int o = option_count; o--;) {
            classes[o] = IntSet(class_data[o], n[o]);
            delete[] class_data[o];
        }
        delete[] class_data;
        delete[] n;

        // Count the cars
        {
            IntSetArgs c(class_count + 2);
            for (int i = class_count; i--;) {
                c[i] = IntSet(ncc[i], ncc[i]);
            }
            c[stall_value] = IntSet(0, max_stalls);
            c[end_value] = IntSet(0, max_stalls);
            count(*this, s, c);
        }

        // Count number of end and stalls
        count(*this, s, stall_value, IRT_EQ, stall_count);
        count(*this, s, end_value, IRT_EQ, end_count);
        rel(*this, stall_count + end_count == max_stalls);

        // Make sure nothing is overloaded
        IntSet one(1, 1);
        for (int o = option_count; o--;) {
            // sb[i] reflects if car s[i] has option o
            BoolVarArgs sb(s.size());
            for (int i = s.size(); i--;) {
                BoolVar b(*this, 0, 1);
                dom(*this, s[i], classes[o], b);
                sb[i] = b;
            }
            sequence(*this, sb, one, block[o], 0, max[o]);
        }

        // End-markers located at end only
        switch (opt.propagation()) {
        case PROP_REGULAR: {
            IntArgs class_values(class_count), class_or_stall_values(class_count + 1);
            for (int i = class_count; i--;) {
                class_values[i] = i;
                class_or_stall_values[i] = i;
            }
            class_or_stall_values[class_count] = stall_value;
            REG r = *REG(class_values) + REG(class_or_stall_values) + *REG(end_value);
            extensional(*this, s, r);
            for (int pos = s.size() - 1, i = 0; i < max_stalls; ++i, --pos) {
                rel(*this, (end_count > i) >> (s[pos] == end_value));
            }
        } break;
        case PROP_CUSTOM: {
            push_to_end(*this, s, end_count, end_value);
        } break;
        }

        // Branching
        switch (opt.branching()) {
        case BRANCH_INORDER:
            branch(*this, s, INT_VAR_NONE(), INT_VAL_MIN());
            break;
        case BRANCH_MIDDLE: {
            IntVarArgs m(s.size());
            int mid = s.size() / 2;
            int pos = 0;
            m[pos++] = s[mid];
            for (int i = 1; i <= m.size() / 2; ++i) {
                if (mid - i >= 0)
                    m[pos++] = s[mid - i];
                if (mid + i < s.size())
                    m[pos++] = s[mid + i];
            }
            assert(pos == m.size());
            branch(*this, m, INT_VAR_NONE(), INT_VAL_MIN());
            break;
        }
        }
    }

    /// Return cost
    void
    constrain(const Space& _best) override {
        const CarSequencing& best = static_cast<const CarSequencing&>(_best);
        rel(*this, stall_count, IRT_LT, best.stall_count.val());
    }

    /// Print solution
    void
    print(std::ostream& os) const override {
        int width = class_count > 9 ? 2 : 1;
        const char* space = class_count > 9 ? " " : "";
        os << "Stall slots=" << stall_count << ", End slots=" << end_count << std::endl;
        int i = 0;
        for (; i < s.size(); ++i) {
            if (s[i].assigned()) {
                int v = s[i].val();
                if (v == end_value)
                    break;
                if (v == stall_value)
                    os << space << "_ ";
                else
                    os << std::setw(width) << v << " ";
            } else {
                os << space << "? ";
            }
            if ((i + 1) % 20 == 0)
                os << std::endl;
        }
        if (i % 20)
            os << std::endl;
        os << std::endl;
    }

    /// Constructor for cloning \a s
    CarSequencing(CarSequencing& cs)
        : Script(cs), problem(cs.problem), car_count(cs.car_count), option_count(cs.option_count),
          class_count(cs.class_count), max_stalls(cs.max_stalls), stall_value(cs.stall_value), end_value(cs.end_value) {
        stall_count.update(*this, cs.stall_count);
        end_count.update(*this, cs.end_count);
        s.update(*this, cs.s);
    }
    /// Copy during cloning
    Space*
    copy(void) override {
        return new CarSequencing(*this);
    }
};

/** \brief Main-function
 *  \relates CarSequencing
 */
int
main(int argc, char* argv[]) {
    CarOptions opt("CarSequencing");
    opt.solutions(0);
    opt.size(0);
    opt.branching(CarSequencing::BRANCH_INORDER);
    opt.branching(CarSequencing::BRANCH_INORDER, "inorder");
    opt.branching(CarSequencing::BRANCH_MIDDLE, "middle");
    opt.propagation(CarSequencing::PROP_CUSTOM);
    opt.propagation(CarSequencing::PROP_REGULAR, "regular");
    opt.propagation(CarSequencing::PROP_CUSTOM, "custom");
    opt.parse(argc, argv);
    if (opt.size() >= n_problems) {
        std::cerr << "Error: size must be between 0 and " << n_problems - 1 << std::endl;
        return 1;
    }

    Script::run<CarSequencing, BAB, CarOptions>(opt);
    return 0;
}

namespace {
    /// Problems from CSPLib

    /// Simple initial example
    const int p0[] = {10, 5, 6, 1, 2, 1, 2, 1, 2, 3, 3, 5, 5, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 2,
                      2,  0, 1, 0, 0, 1, 3, 2, 0, 1, 0, 1, 0, 4, 2, 1, 0, 1, 0, 0, 5, 2, 1, 1, 0, 0, 0};

    // ---------------------------------
    //  Problem 4/72  (Regin & Puget   // 1)
    // ---------------------------------
    const int p1[] = {
        100, 5, 22, 1, 2, 1, 2,  1, 2, 3, 3, 5, 5, 0,  6, 1, 0, 0, 1, 0, 1,  10, 1, 1, 1, 0, 0, 2,
        2,   1, 1,  0, 0, 1, 3,  2, 0, 1, 1, 0, 0, 4,  8, 0, 0, 0, 1, 0, 5,  15, 0, 1, 0, 0, 0, 6,
        1,   0, 1,  1, 1, 0, 7,  5, 0, 0, 1, 1, 0, 8,  2, 1, 0, 1, 1, 0, 9,  3,  0, 0, 1, 0, 0, 10,
        2,   1, 0,  1, 0, 0, 11, 1, 1, 1, 1, 0, 1, 12, 8, 0, 1, 0, 1, 0, 13, 3,  1, 0, 0, 1, 1, 14,
        10,  1, 0,  0, 0, 0, 15, 4, 0, 1, 0, 0, 1, 16, 4, 0, 0, 0, 0, 1, 17, 2,  1, 0, 0, 0, 1, 18,
        4,   1, 1,  0, 0, 0, 19, 6, 1, 1, 0, 1, 0, 20, 1, 1, 0, 1, 0, 1, 21, 1,  1, 1, 1, 1, 1,
    };

    // --------------------------------
    //  Problem 6/76, (Regin & Puget   // 2)
    // --------------------------------
    const int p2[] = {
        100, 5, 22, 1, 2, 1, 2,  1, 2, 3, 3, 5, 5, 0,  13, 1, 0, 0, 0, 0, 1,  8,  0, 0, 0, 1, 0, 2,
        7,   0, 1,  0, 0, 0, 3,  1, 1, 0, 0, 1, 0, 4,  12, 0, 0, 1, 0, 0, 5,  5,  0, 1, 0, 1, 0, 6,
        5,   0, 0,  1, 1, 0, 7,  6, 0, 1, 1, 0, 0, 8,  3,  1, 0, 0, 0, 1, 9,  12, 1, 1, 0, 0, 0, 10,
        8,   1, 1,  0, 1, 0, 11, 2, 1, 0, 0, 1, 1, 12, 2,  1, 1, 1, 0, 0, 13, 1,  0, 1, 0, 1, 1, 14,
        4,   1, 0,  1, 0, 0, 15, 4, 0, 1, 0, 0, 1, 16, 1,  1, 1, 0, 1, 1, 17, 2,  1, 0, 1, 1, 0, 18,
        1,   0, 0,  0, 0, 1, 19, 1, 1, 1, 1, 1, 0, 20, 1,  1, 1, 0, 0, 1, 21, 1,  0, 1, 1, 1, 0,
    };

    // ---------------------------------
    //  Problem 10/93, (Regin & Puget   // 3)
    // ---------------------------------
    const int p3[] = {
        100, 5, 25, 1,  2,  1,  2, 1, 2, 3,  3,  5,  5,  0, 7, 1,  0,  0,  1,  0,  1, 11, 1, 1,  0,  0,  0,
        2,   1, 0,  1,  1,  1,  1, 3, 3, 1,  0,  1,  0,  0, 4, 15, 0,  1,  0,  0,  0, 5,  2, 1,  0,  1,  1,
        0,   6, 8,  0,  1,  0,  1, 0, 7, 5,  0,  0,  1,  0, 0, 8,  3,  0,  0,  0,  1, 0,  9, 4,  0,  1,  1,
        1,   0, 10, 5,  1,  0,  0, 0, 0, 11, 2,  1,  1,  1, 0, 1,  12, 6,  0,  1,  1, 0,  0, 13, 2,  0,  0,
        1,   0, 1,  14, 2,  0,  1, 0, 0, 1,  15, 4,  1,  1, 1, 1,  0,  16, 3,  1,  0, 0,  0, 1,  17, 5,  1,
        1,   0, 1,  0,  18, 2,  1, 1, 1, 0,  0,  19, 4,  1, 1, 0,  0,  1,  20, 1,  1, 0,  0, 1,  1,  21, 1,
        1,   1, 0,  1,  1,  22, 1, 0, 1, 0,  1,  1,  23, 1, 0, 1,  1,  0,  1,  24, 2, 0,  0, 0,  0,  1,
    };

    // --------------
    //  Problem 16/81,
    // --------------
    const int p4[] = {
        100, 5, 26, 1, 2, 1, 2,  1, 2, 3, 3, 5, 5, 0,  10, 1, 0, 0, 0, 0, 1,  2,  0, 0, 0, 0, 1, 2,
        8,   0, 1,  0, 1, 0, 3,  8, 0, 0, 0, 1, 0, 4,  6,  0, 1, 1, 0, 0, 5,  11, 0, 1, 0, 0, 0, 6,
        3,   0, 0,  1, 0, 0, 7,  2, 0, 0, 1, 1, 0, 8,  7,  1, 1, 0, 0, 0, 9,  2,  1, 0, 0, 1, 1, 10,
        4,   1, 0,  1, 0, 0, 11, 7, 1, 0, 0, 1, 0, 12, 1,  1, 1, 1, 0, 1, 13, 3,  0, 1, 1, 1, 0, 14,
        4,   0, 1,  0, 0, 1, 15, 5, 1, 1, 1, 0, 0, 16, 2,  1, 1, 0, 0, 1, 17, 1,  1, 0, 1, 1, 1, 18,
        2,   1, 0,  1, 1, 0, 19, 3, 1, 0, 0, 0, 1, 20, 2,  0, 1, 1, 0, 1, 21, 1,  0, 1, 0, 1, 1, 22,
        3,   1, 1,  0, 1, 0, 23, 1, 0, 0, 1, 1, 1, 24, 1,  1, 1, 1, 1, 1, 25, 1,  1, 1, 1, 1, 0,
    };

    // ----------------------------------
    //  Problem 19/71,  (Regin & Puget   // 4)
    // ----------------------------------
    const int p5[] = {
        100, 5,  23, 1,  2, 1, 2, 1, 2,  3,  3,  5, 5,  0, 2, 0,  0,  0,  1, 1, 1, 2, 0,  0,  1,  0,  1,  2, 5,
        0,   1,  1,  1,  0, 3, 4, 0, 0,  0,  1,  0, 4,  4, 0, 1,  0,  1,  0, 5, 1, 1, 1,  0,  0,  1,  6,  3, 1,
        1,   1,  0,  1,  7, 4, 0, 0, 1,  0,  0,  8, 19, 0, 1, 0,  0,  0,  9, 7, 1, 1, 0,  1,  0,  10, 10, 1, 0,
        0,   0,  0,  11, 1, 0, 0, 1, 1,  0,  12, 5, 1,  1, 1, 1,  0,  13, 2, 1, 0, 1, 1,  0,  14, 6,  1,  1, 0,
        0,   0,  15, 4,  1, 1, 1, 0, 0,  16, 8,  1, 0,  0, 1, 0,  17, 1,  1, 0, 0, 0, 1,  18, 4,  0,  1,  1, 0,
        0,   19, 2,  0,  0, 0, 0, 1, 20, 4,  0,  1, 0,  0, 1, 21, 1,  1,  1, 0, 1, 1, 22, 1,  0,  1,  1,  0, 1,
    };

    const int* problems[] = {
        &p0[0], &p1[0], &p2[0], &p3[0], &p4[0], &p5[0],
    };

    /// The number of instances
    const unsigned int n_problems = sizeof(problems) / sizeof(int*);
}; // namespace

// STATISTICS: example-any
