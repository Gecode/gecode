/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#ifndef GECODE_FLATZINC_BRANCH_HH
#define GECODE_FLATZINC_BRANCH_HH

#include <gecode/int.hh>
#include <gecode/int/branch.hh>
#include <gecode/flatzinc.hh>

namespace Gecode { namespace FlatZinc {

    /// Which integer or Boolean variable to select for branching
    class IntBoolVarBranch : public VarBranch<IntVar> {
    public:
        /// Which variable selection
        enum class Select {
            SEL_AFC_MAX,         ///< With largest accumulated failure count
            SEL_ACTION_MAX,      ///< With highest action
            SEL_CHB_MAX,         ///< With highest CHB Q-score
            SEL_AFC_SIZE_MAX,    ///< With largest accumulated failure count divided by domain size
            SEL_ACTION_SIZE_MAX, ///< With largest action divided by domain size
            SEL_CHB_SIZE_MAX     ///< With largest CHB Q-score divided by domain size
        };
        using enum Select;

    protected:
        /// Which variable to select
        Select s;
        /// Integer AFC
        IntAFC int_afc_data;
        /// Boolean AFC
        BoolAFC bool_afc_data;
        /// Integer action
        IntAction int_action_data;
        /// Boolean action
        BoolAction bool_action_data;
        /// Integer CHB
        IntCHB int_chb_data;
        /// Boolean CHB
        BoolCHB bool_chb_data;

    public:
        /// Initialize with selection strategy \a s and decay factor \a d
        IntBoolVarBranch(Select s, double d);
        /// Initialize with selection strategy \a s and AFC \a i and \a b
        IntBoolVarBranch(Select s, const IntAFC& i, const BoolAFC& b);
        /// Initialize with selection strategy \a s and action \a i and \a b
        IntBoolVarBranch(Select s, const IntAction& i, const BoolAction& b);
        /// Initialize with selection strategy \a s and CHB \a i and \a b
        IntBoolVarBranch(Select s, const IntCHB& i, const BoolCHB& b);
        /// Return selection strategy
        Select select() const;
        /// Return integer AFC
        IntAFC int_afc() const;
        /// Return Boolean AFC
        BoolAFC bool_afc() const;
        /// Return integer action
        IntAction int_action() const;
        /// Return Boolean action
        BoolAction bool_action() const;
        /// Return integer CHB
        IntCHB int_chb() const;
        /// Return Boolean AFC
        BoolCHB bool_chb() const;
        /// Expand AFC, action, and CHB
        void expand(Home home, const IntVarArgs& x, const BoolVarArgs& y);
    };

    /// Variable selection for both integer and Boolean variables
    //@{
    /// Select variable with largest accumulated failure count
    IntBoolVarBranch int_bool_var_afc_max(double d = 1.0);
    /// Select variable with largest accumulated failure count
    IntBoolVarBranch int_bool_var_afc_max(const IntAFC& ia, const BoolAFC& ba);
    /// Select variable with highest action
    IntBoolVarBranch int_bool_var_action_max(double d = 1.0);
    /// Select variable with highest action
    IntBoolVarBranch int_bool_var_action_max(const IntAction& ia, const BoolAction& ba);
    /// Select variable with largest CHB Q-score
    IntBoolVarBranch int_bool_var_chb_max(double d = 1.0);
    /// Select variable with largest CHB Q-score
    IntBoolVarBranch int_bool_var_chb_max(const IntCHB& ic, const BoolCHB& bc);
    /// Select variable with largest accumulated failure count divided by domain size
    IntBoolVarBranch int_bool_var_afc_size_max(double d = 1.0);
    /// Select variable with largest accumulated failure count divided by domain size
    IntBoolVarBranch int_bool_var_afc_size_max(const IntAFC& ia, const BoolAFC& ba);
    /// Select variable with largest action divided by domain size
    IntBoolVarBranch int_bool_var_action_size_max(double d = 1.0);
    /// Select variable with largest action divided by domain size
    IntBoolVarBranch int_bool_var_action_size_max(const IntAction& ia, const BoolAction& ba);
    /// Select variable with largest CHB Q-score divided by domain size
    IntBoolVarBranch int_bool_var_chb_size_max(double d = 1.0);
    /// Select variable with largest CHB Q-score divided by domain size
    IntBoolVarBranch int_bool_var_chb_size_max(const IntCHB& ic, const BoolCHB& bc);
    //@}

    /// Select by maximal AFC
    class MeritMaxAFC {
    protected:
        /// Integer AFC information
        IntAFC int_afc_data;
        /// Boolean AFC information
        BoolAFC bool_afc_data;

    public:
        /// Constructor for initialization
        MeritMaxAFC(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxAFC(Space& home, MeritMaxAFC& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// Select by maximal AFC over size
    class MeritMaxAFCSize {
    protected:
        /// Integer AFC information
        IntAFC int_afc_data;
        /// Boolean AFC information
        BoolAFC bool_afc_data;

    public:
        /// Constructor for initialization
        MeritMaxAFCSize(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxAFCSize(Space& home, MeritMaxAFCSize& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// Select by maximal Action
    class MeritMaxAction {
    protected:
        /// Integer Action information
        IntAction int_action_data;
        /// Boolean Action information
        BoolAction bool_action_data;

    public:
        /// Constructor for initialization
        MeritMaxAction(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxAction(Space& home, MeritMaxAction& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// Select by maximal Action over size
    class MeritMaxActionSize {
    protected:
        /// Integer Action information
        IntAction int_action_data;
        /// Boolean Action information
        BoolAction bool_action_data;

    public:
        /// Constructor for initialization
        MeritMaxActionSize(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxActionSize(Space& home, MeritMaxActionSize& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// Select by maximal CHB
    class MeritMaxCHB {
    protected:
        /// Integer CHB information
        IntCHB int_chb_data;
        /// Boolean CHB information
        BoolCHB bool_chb_data;

    public:
        /// Constructor for initialization
        MeritMaxCHB(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxCHB(Space& home, MeritMaxCHB& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// Select by maximal CHB over size
    class MeritMaxCHBSize {
    protected:
        /// Integer CHB information
        IntCHB int_chb_data;
        /// Boolean CHB information
        BoolCHB bool_chb_data;

    public:
        /// Constructor for initialization
        MeritMaxCHBSize(Space& home, const IntBoolVarBranch& ibvb);
        /// Constructor for cloning
        MeritMaxCHBSize(Space& home, MeritMaxCHBSize& m);
        /// Return merit
        double operator()(Int::IntView x, int i) const;
        /// Return merit
        double operator()(Int::BoolView x, int i) const;
        /// Dispose
        void dispose();
    };

    /// %Choice storing position and value
    class GECODE_VTABLE_EXPORT PosIntChoice : public Choice {
    private:
        /// Position of view to assign
        int _pos;
        /// Value to assign to
        int _val;

    public:
        /// Initialize choice for brancher \a b, number of alternatives \a a, position \a p, and value \a n
        PosIntChoice(const Brancher& b, unsigned int a, int p, int n);
        /// Return position of view to assign
        int pos() const;
        /// Return value to assign to
        int val() const;
        /// Archive into \a e
        void archive(Archive& e) const override;
    };

    /// Base-class for brancher for integer and Boolean views
    class IntBoolBrancherBase : public Brancher {
    protected:
        /// Integer views to branch on
        ViewArray<Int::IntView> x;
        /// Boolean views to branch on
        ViewArray<Int::BoolView> y;
        /// Unassigned views start here (might be in \a x or \a y)
        mutable int start;
        /// Integer value selection and commit object
        ValSelCommitBase<Int::IntView, int>* xvsc;
        /// Boolean value selection and commit object
        ValSelCommitBase<Int::BoolView, int>* yvsc;
        /// Constructor for cloning \a b
        IntBoolBrancherBase(Space& home, IntBoolBrancherBase& b);
        /// Constructor for creation
        IntBoolBrancherBase(Home home, ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                            ValSelCommitBase<Int::IntView, int>* xvsc, ValSelCommitBase<Int::BoolView, int>* yvsc);

    public:
        /// Check status of brancher, return true if alternatives left
        bool status(const Space& home) const override;
        /// Return choice
        const Choice* choice(Space& home) override = 0;
        /// Return choice
        const Choice* choice(const Space& home, Archive& e) override;
        /// Perform commit for choice \a c and alternative \a b
        ExecStatus commit(Space& home, const Choice& c, unsigned int b) override;
        /// Create no-good literal for choice \a c and alternative \a b
        NGL* ngl(Space& home, const Choice& c, unsigned int b) const override;
        /// Print branch for choice \a c and alternative \a b
        void print(const Space& home, const Choice& c, unsigned int b, std::ostream& o) const override;
        /// Delete brancher and return its size
        size_t dispose(Space& home) override;
    };

    /// Brancher for integer and Boolean views
    template <class Merit>
    class IntBoolBrancher : public IntBoolBrancherBase {
    protected:
        /// Selection by maximal merit
        Merit merit;
        /// Constructor for cloning \a b
        IntBoolBrancher(Space& home, IntBoolBrancher& b);
        /// Constructor for creation
        IntBoolBrancher(Home home, ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y, Merit& m,
                        ValSelCommitBase<Int::IntView, int>* xvsc, ValSelCommitBase<Int::BoolView, int>* yvsc);

    public:
        /// Return choice
        const Choice* choice(Space& home) override;
        /// Perform cloning
        Actor* copy(Space& home) override;
        /// Post brancher
        static void post(Home home, ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y, Merit& m,
                         ValSelCommitBase<Int::IntView, int>* xvsc, ValSelCommitBase<Int::BoolView, int>* yvsc);
        /// Delete brancher and return its size
        size_t dispose(Space& home) override;
    };

    /// Map respective integer value selection to Boolean value selection
    BoolValBranch i2b(const IntValBranch& ivb);

    /// Branch function for integer and Boolean variables
    GECODE_FLATZINC_EXPORT void branch(Home home, const IntVarArgs& x, const BoolVarArgs& y, IntBoolVarBranch vars,
                                       const IntValBranch& vals);

}} // namespace Gecode::FlatZinc

#include <gecode/flatzinc/branch.hpp>

#endif

// STATISTICS: flatzinc-branch
