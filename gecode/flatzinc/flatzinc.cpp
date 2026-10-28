/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Gabriel Hjort Blindell <gabriel.hjort.blindell@gmail.com>
 *     Nathan Tran <kieron.qtran@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007-2012
 *     Gabriel Hjort Blindell, 2012
 *     Nathan Tran, 2025
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

#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/blackbox.hh>
#include <gecode/flatzinc/registry.hh>
#include <gecode/flatzinc/plugin.hh>
#include <gecode/flatzinc/branch.hh>

#include <gecode/search.hh>

#include <vector>
#include <string>
#include <sstream>
#include <limits>
#include <memory>
#include <unordered_set>

namespace std {

    /// Hashing for tuple sets
    template <>
    struct hash<Gecode::TupleSet> {
        /// Return hash key for \a x
        forceinline size_t
        operator()(const Gecode::TupleSet& x) const {
            return x.hash();
        }
    };

    /// Hashing for tuple sets
    template <>
    struct hash<Gecode::SharedArray<int>> {
        /// Return hash key for \a x
        forceinline size_t
        operator()(const Gecode::SharedArray<int>& x) const {
            size_t seed = static_cast<size_t>(x.size());
            for (int i = x.size(); i--;)
                Gecode::cmb_hash(seed, x[i]);
            return seed;
        }
    };

    /// Hashing for DFAs
    template <>
    struct hash<Gecode::DFA> {
        /// Return hash key for \a d
        forceinline size_t
        operator()(const Gecode::DFA& d) const {
            return d.hash();
        }
    };

} // namespace std

namespace Gecode { namespace FlatZinc {

    // Default random number generator
    Rnd defrnd(0);

    /**
     * \brief Branching on the introduced variables
     *
     * This brancher makes sure that when a solution is found for the model
     * variables, all introduced variables are either assigned, or the solution
     * can be extended to a solution of the introduced variables.
     *
     * The advantage over simply branching over the introduced variables is that
     * only one such extension will be searched for, instead of enumerating all
     * possible (equivalent) extensions.
     *
     */
    class AuxVarBrancher : public Brancher {
    protected:
        /// Flag whether brancher is done
        bool done;
        /// Construct brancher
        AuxVarBrancher(Home home, TieBreak<IntVarBranch> int_varsel0, IntValBranch int_valsel0,
                       TieBreak<BoolVarBranch> bool_varsel0, BoolValBranch bool_valsel0
#ifdef GECODE_HAS_SET_VARS
                       ,
                       SetVarBranch set_varsel0, SetValBranch set_valsel0
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                       ,
                       TieBreak<FloatVarBranch> float_varsel0, FloatValBranch float_valsel0
#endif
                       )
            : Brancher(home), done(false), int_varsel(int_varsel0), int_valsel(int_valsel0), bool_varsel(bool_varsel0),
              bool_valsel(bool_valsel0)
#ifdef GECODE_HAS_SET_VARS
              ,
              set_varsel(set_varsel0), set_valsel(set_valsel0)
#endif
#ifdef GECODE_HAS_FLOAT_VARS
              ,
              float_varsel(float_varsel0), float_valsel(float_valsel0)
#endif
        {
        }
        /// Copy constructor
        AuxVarBrancher(Space& home, AuxVarBrancher& b) : Brancher(home, b), done(b.done) {}

        /// %Choice that only signals failure or success
        class Choice : public Gecode::Choice {
        public:
            /// Whether brancher should fail
            bool fail;
            /// Initialize choice for brancher \a b
            Choice(const Brancher& b, bool fail0) : Gecode::Choice(b, 1), fail(fail0) {}
            /// Report size occupied
            virtual size_t
            size(void) const {
                return sizeof(Choice);
            }
            /// Archive into \a e
            void
            archive(Archive& e) const override {
                Gecode::Choice::archive(e);
                e.put(fail);
            }
        };

        TieBreak<IntVarBranch> int_varsel;
        IntValBranch int_valsel;
        TieBreak<BoolVarBranch> bool_varsel;
        BoolValBranch bool_valsel;
#ifdef GECODE_HAS_SET_VARS
        SetVarBranch set_varsel;
        SetValBranch set_valsel;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        TieBreak<FloatVarBranch> float_varsel;
        FloatValBranch float_valsel;
#endif

    public:
        /// Check status of brancher, return true if alternatives left.
        bool
        status(const Space& _home) const override {
            if (done)
                return false;
            const FlatZincSpace& home = static_cast<const FlatZincSpace&>(_home);
            for (int i = 0; i < home.iv_aux.size(); i++)
                if (!home.iv_aux[i].assigned())
                    return true;
            for (int i = 0; i < home.bv_aux.size(); i++)
                if (!home.bv_aux[i].assigned())
                    return true;
#ifdef GECODE_HAS_SET_VARS
            for (int i = 0; i < home.sv_aux.size(); i++)
                if (!home.sv_aux[i].assigned())
                    return true;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
            for (int i = 0; i < home.fv_aux.size(); i++)
                if (!home.fv_aux[i].assigned())
                    return true;
#endif
            // No non-assigned variables left
            return false;
        }
        /// Return choice
        Choice*
        choice(Space& home) override {
            done = true;
            FlatZincSpace& fzs = static_cast<FlatZincSpace&>(*home.clone());
            fzs.need_aux_vars = false;
            branch(fzs, fzs.iv_aux, int_varsel, int_valsel);
            branch(fzs, fzs.bv_aux, bool_varsel, bool_valsel);
#ifdef GECODE_HAS_SET_VARS
            branch(fzs, fzs.sv_aux, set_varsel, set_valsel);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
            branch(fzs, fzs.fv_aux, float_varsel, float_valsel);
#endif
            Search::Options opt;
            opt.clone = false;
            FlatZincSpace* sol = dfs(&fzs, opt);
            if (sol) {
                delete sol;
                return new Choice(*this, false);
            } else {
                return new Choice(*this, true);
            }
        }
        /// Return choice
        Choice*
        choice(const Space&, Archive& e) override {
            bool fail;
            e >> fail;
            return new Choice(*this, fail);
        }
        /// Perform commit for choice \a c
        ExecStatus
        commit(Space&, const Gecode::Choice& c, unsigned int) override {
            return static_cast<const Choice&>(c).fail ? ES_FAILED : ES_OK;
        }
        /// Print explanation
        void
        print(const Space&, const Gecode::Choice& c, unsigned int, std::ostream& o) const override {
            o << "FlatZinc(" << (static_cast<const Choice&>(c).fail ? "fail" : "ok") << ")";
        }
        /// Copy brancher
        Actor*
        copy(Space& home) override {
            return new (home) AuxVarBrancher(home, *this);
        }
        /// Post brancher
        static void
        post(Home home, TieBreak<IntVarBranch> int_varsel, IntValBranch int_valsel, TieBreak<BoolVarBranch> bool_varsel,
             BoolValBranch bool_valsel
#ifdef GECODE_HAS_SET_VARS
             ,
             SetVarBranch set_varsel, SetValBranch set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
             ,
             TieBreak<FloatVarBranch> float_varsel, FloatValBranch float_valsel
#endif
        ) {
            (void)new (home) AuxVarBrancher(home, int_varsel, int_valsel, bool_varsel, bool_valsel
#ifdef GECODE_HAS_SET_VARS
                                            ,
                                            set_varsel, set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                                            ,
                                            float_varsel, float_valsel
#endif
            );
        }
        /// Delete brancher and return its size
        size_t
        dispose(Space&) override {
            return sizeof(*this);
        }
    };

    class BranchInformationO : public SharedHandle::Object {
    private:
        struct BI {
            std::string r0;
            std::string r1;
            std::vector<std::string> n;
            BI(void) : r0(""), r1(""), n(0) {}
            BI(const std::string& r00, const std::string& r10, const std::vector<std::string>& n0)
                : r0(r00), r1(r10), n(n0) {}
        };
        std::vector<BI> v;
        BranchInformationO(std::vector<BI> v0) : v(v0) {}

    public:
        BranchInformationO(void) {}
        ~BranchInformationO(void) override {}
        virtual SharedHandle::Object*
        copy(void) const {
            return new BranchInformationO(v);
        }
        /// Add new brancher information
        void
        add(BrancherGroup bg, const std::string& rel0, const std::string& rel1, const std::vector<std::string>& n) {
            v.resize(std::max(static_cast<unsigned int>(v.size()), bg.id() + 1));
            v[bg.id()] = BI(rel0, rel1, n);
        }
        /// Output branch information
        void
        print(const Brancher& b, unsigned int a, int i, int n, std::ostream& o) const {
            const BI& bi = v[b.group().id()];
            o << bi.n[i] << " " << (a == 0 ? bi.r0 : bi.r1) << " " << n;
        }
#ifdef GECODE_HAS_FLOAT_VARS
        void
        print(const Brancher& b, unsigned int a, int i, const FloatNumBranch& nl, std::ostream& o) const {
            const BI& bi = v[b.group().id()];
            o << bi.n[i] << " " << (((a == 0) == nl.l) ? "<=" : ">=") << nl.n;
        }
#endif
    };

    BranchInformation::BranchInformation(void) : SharedHandle(nullptr) {}

    BranchInformation::BranchInformation(const BranchInformation& bi) : SharedHandle(bi) {}

    void
    BranchInformation::init(void) {
        assert(object() == nullptr);
        object(new BranchInformationO());
    }

    void
    BranchInformation::add(BrancherGroup bg, const std::string& rel0, const std::string& rel1,
                           const std::vector<std::string>& n) {
        static_cast<BranchInformationO*>(object())->add(bg, rel0, rel1, n);
    }
    void
    BranchInformation::print(const Brancher& b, unsigned int a, int i, int n, std::ostream& o) const {
        static_cast<const BranchInformationO*>(object())->print(b, a, i, n, o);
    }
#ifdef GECODE_HAS_FLOAT_VARS
    void
    BranchInformation::print(const Brancher& b, unsigned int a, int i, const FloatNumBranch& nl,
                             std::ostream& o) const {
        static_cast<const BranchInformationO*>(object())->print(b, a, i, nl, o);
    }
#endif
    template <class Var>
    void
    var_val_print(const Space& home, const Brancher& b, unsigned int a, Var, int i, const int& n, std::ostream& o) {
        static_cast<const FlatZincSpace&>(home).branch_info.print(b, a, i, n, o);
    }

#ifdef GECODE_HAS_FLOAT_VARS
    void
    var_val_print_f(const Space& home, const Brancher& b, unsigned int a, FloatVar, int i, const FloatNumBranch& nl,
                    std::ostream& o) {
        static_cast<const FlatZincSpace&>(home).branch_info.print(b, a, i, nl, o);
    }
#endif

    IntSet
    vs2is(IntVarSpec* vs) {
        if (vs->assigned) {
            return IntSet(vs->i, vs->i);
        }
        if (vs->domain()) {
            AST::SetLit* sl = vs->domain.some();
            if (sl->interval) {
                return IntSet(sl->min, sl->max);
            } else {
                int* newdom = heap.alloc<int>(static_cast<unsigned long int>(sl->s.size()));
                for (int i = sl->s.size(); i--;)
                    newdom[i] = sl->s[i];
                IntSet ret(newdom, sl->s.size());
                heap.free(newdom, static_cast<unsigned long int>(sl->s.size()));
                return ret;
            }
        }
        return IntSet(Int::Limits::min, Int::Limits::max);
    }

    int
    vs2bsl(BoolVarSpec* bs) {
        if (bs->assigned) {
            return bs->i;
        }
        if (bs->domain()) {
            AST::SetLit* sl = bs->domain.some();
            assert(sl->interval);
            return std::min(1, std::max(0, sl->min));
        }
        return 0;
    }

    int
    vs2bsh(BoolVarSpec* bs) {
        if (bs->assigned) {
            return bs->i;
        }
        if (bs->domain()) {
            AST::SetLit* sl = bs->domain.some();
            assert(sl->interval);
            return std::max(0, std::min(1, sl->max));
        }
        return 1;
    }

    TieBreak<IntVarBranch>
    annotation_to_int_var_branch(AST::Node* ann, Rnd rnd, double decay) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "input_order")
                return TieBreak<IntVarBranch>(INT_VAR_NONE());
            if (s->id == "first_fail")
                return TieBreak<IntVarBranch>(INT_VAR_SIZE_MIN());
            if (s->id == "anti_first_fail")
                return TieBreak<IntVarBranch>(INT_VAR_SIZE_MAX());
            if (s->id == "smallest")
                return TieBreak<IntVarBranch>(INT_VAR_MIN_MIN());
            if (s->id == "largest")
                return TieBreak<IntVarBranch>(INT_VAR_MAX_MAX());
            if (s->id == "occurrence")
                return TieBreak<IntVarBranch>(INT_VAR_DEGREE_MAX());
            if (s->id == "max_regret")
                return TieBreak<IntVarBranch>(INT_VAR_REGRET_MIN_MAX());
            if (s->id == "most_constrained")
                return TieBreak<IntVarBranch>(INT_VAR_SIZE_MIN(), INT_VAR_DEGREE_MAX());
            if (s->id == "random") {
                return TieBreak<IntVarBranch>(INT_VAR_RND(rnd));
            }
            if (s->id == "dom_w_deg") {
                return TieBreak<IntVarBranch>(INT_VAR_AFC_SIZE_MAX(decay));
            }
            if (s->id == "afc_min")
                return TieBreak<IntVarBranch>(INT_VAR_AFC_MIN(decay));
            if (s->id == "afc_max")
                return TieBreak<IntVarBranch>(INT_VAR_AFC_MAX(decay));
            if (s->id == "afc_size_min")
                return TieBreak<IntVarBranch>(INT_VAR_AFC_SIZE_MIN(decay));
            if (s->id == "afc_size_max") {
                return TieBreak<IntVarBranch>(INT_VAR_AFC_SIZE_MAX(decay));
            }
            if (s->id == "action_min")
                return TieBreak<IntVarBranch>(INT_VAR_ACTION_MIN(decay));
            if (s->id == "action_max")
                return TieBreak<IntVarBranch>(INT_VAR_ACTION_MAX(decay));
            if (s->id == "action_size_min")
                return TieBreak<IntVarBranch>(INT_VAR_ACTION_SIZE_MIN(decay));
            if (s->id == "action_size_max")
                return TieBreak<IntVarBranch>(INT_VAR_ACTION_SIZE_MAX(decay));
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return TieBreak<IntVarBranch>(INT_VAR_NONE());
    }

    IntValBranch
    annotation_to_int_val_branch(AST::Node* ann, std::string& r0, std::string& r1, Rnd rnd) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "indomain_min") {
                r0 = "=";
                r1 = "!=";
                return INT_VAL_MIN();
            }
            if (s->id == "indomain_max") {
                r0 = "=";
                r1 = "!=";
                return INT_VAL_MAX();
            }
            if (s->id == "indomain_median") {
                r0 = "=";
                r1 = "!=";
                return INT_VAL_MED();
            }
            if (s->id == "indomain_split") {
                r0 = "<=";
                r1 = ">";
                return INT_VAL_SPLIT_MIN();
            }
            if (s->id == "indomain_reverse_split") {
                r0 = ">";
                r1 = "<=";
                return INT_VAL_SPLIT_MAX();
            }
            if (s->id == "indomain_random") {
                r0 = "=";
                r1 = "!=";
                return INT_VAL_RND(rnd);
            }
            if (s->id == "indomain") {
                r0 = "=";
                r1 = "=";
                return INT_VALUES_MIN();
            }
            if (s->id == "indomain_middle") {
                std::cerr << "Warning, replacing unsupported annotation "
                          << "indomain_middle with indomain_median" << std::endl;
                r0 = "=";
                r1 = "!=";
                return INT_VAL_MED();
            }
            if (s->id == "indomain_interval") {
                std::cerr << "Warning, replacing unsupported annotation "
                          << "indomain_interval with indomain_split" << std::endl;
                r0 = "<=";
                r1 = ">";
                return INT_VAL_SPLIT_MIN();
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        r0 = "=";
        r1 = "!=";
        return INT_VAL_MIN();
    }

    IntAssign
    annotation_to_int_assign(AST::Node* ann, Rnd rnd) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "indomain_min")
                return INT_ASSIGN_MIN();
            if (s->id == "indomain_max")
                return INT_ASSIGN_MAX();
            if (s->id == "indomain_median")
                return INT_ASSIGN_MED();
            if (s->id == "indomain_random") {
                return INT_ASSIGN_RND(rnd);
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return INT_ASSIGN_MIN();
    }

    TieBreak<BoolVarBranch>
    annotation_to_bool_var_branch(AST::Node* ann, Rnd rnd, double decay) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if ((s->id == "input_order") || (s->id == "first_fail") || (s->id == "anti_first_fail") ||
                (s->id == "smallest") || (s->id == "largest") || (s->id == "max_regret"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_NONE());
            if ((s->id == "occurrence") || (s->id == "most_constrained"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_DEGREE_MAX());
            if (s->id == "random")
                return TieBreak<BoolVarBranch>(BOOL_VAR_RND(rnd));
            if ((s->id == "afc_min") || (s->id == "afc_size_min"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_AFC_MIN(decay));
            if ((s->id == "afc_max") || (s->id == "afc_size_max") || (s->id == "dom_w_deg"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_AFC_MAX(decay));
            if ((s->id == "action_min") && (s->id == "action_size_min"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_ACTION_MIN(decay));
            if ((s->id == "action_max") || (s->id == "action_size_max"))
                return TieBreak<BoolVarBranch>(BOOL_VAR_ACTION_MAX(decay));
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return TieBreak<BoolVarBranch>(BOOL_VAR_NONE());
    }

    BoolValBranch
    annotation_to_bool_val_branch(AST::Node* ann, std::string& r0, std::string& r1, Rnd rnd) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "indomain_min") {
                r0 = "=";
                r1 = "!=";
                return BOOL_VAL_MIN();
            }
            if (s->id == "indomain_max") {
                r0 = "=";
                r1 = "!=";
                return BOOL_VAL_MAX();
            }
            if (s->id == "indomain_median") {
                r0 = "=";
                r1 = "!=";
                return BOOL_VAL_MIN();
            }
            if (s->id == "indomain_split") {
                r0 = "<=";
                r1 = ">";
                return BOOL_VAL_MIN();
            }
            if (s->id == "indomain_reverse_split") {
                r0 = ">";
                r1 = "<=";
                return BOOL_VAL_MAX();
            }
            if (s->id == "indomain_random") {
                r0 = "=";
                r1 = "!=";
                return BOOL_VAL_RND(rnd);
            }
            if (s->id == "indomain") {
                r0 = "=";
                r1 = "=";
                return BOOL_VAL_MIN();
            }
            if (s->id == "indomain_middle") {
                std::cerr << "Warning, replacing unsupported annotation "
                          << "indomain_middle with indomain_median" << std::endl;
                r0 = "=";
                r1 = "!=";
                return BOOL_VAL_MIN();
            }
            if (s->id == "indomain_interval") {
                std::cerr << "Warning, replacing unsupported annotation "
                          << "indomain_interval with indomain_split" << std::endl;
                r0 = "<=";
                r1 = ">";
                return BOOL_VAL_MIN();
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        r0 = "=";
        r1 = "!=";
        return BOOL_VAL_MIN();
    }

    BoolAssign
    annotation_to_bool_assign(AST::Node* ann, Rnd rnd) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if ((s->id == "indomain_min") || (s->id == "indomain_median"))
                return BOOL_ASSIGN_MIN();
            if (s->id == "indomain_max")
                return BOOL_ASSIGN_MAX();
            if (s->id == "indomain_random") {
                return BOOL_ASSIGN_RND(rnd);
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return BOOL_ASSIGN_MIN();
    }

#ifdef GECODE_HAS_SET_VARS
    SetVarBranch
    annotation_to_set_var_branch(AST::Node* ann, Rnd rnd, double decay) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "input_order")
                return SET_VAR_NONE();
            if (s->id == "first_fail")
                return SET_VAR_SIZE_MIN();
            if (s->id == "anti_first_fail")
                return SET_VAR_SIZE_MAX();
            if (s->id == "smallest")
                return SET_VAR_MIN_MIN();
            if (s->id == "largest")
                return SET_VAR_MAX_MAX();
            if (s->id == "afc_min")
                return SET_VAR_AFC_MIN(decay);
            if (s->id == "afc_max")
                return SET_VAR_AFC_MAX(decay);
            if (s->id == "afc_size_min")
                return SET_VAR_AFC_SIZE_MIN(decay);
            if (s->id == "afc_size_max")
                return SET_VAR_AFC_SIZE_MAX(decay);
            if (s->id == "action_min")
                return SET_VAR_ACTION_MIN(decay);
            if (s->id == "action_max")
                return SET_VAR_ACTION_MAX(decay);
            if (s->id == "action_size_min")
                return SET_VAR_ACTION_SIZE_MIN(decay);
            if (s->id == "action_size_max")
                return SET_VAR_ACTION_SIZE_MAX(decay);
            if (s->id == "random") {
                return SET_VAR_RND(rnd);
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return SET_VAR_NONE();
    }

    SetValBranch
    annotation_to_set_val_branch(AST::Node* ann, std::string r0, std::string r1, Rnd rnd) {
        (void)rnd;
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "indomain_min") {
                r0 = "in";
                r1 = "not in";
                return SET_VAL_MIN_INC();
            }
            if (s->id == "indomain_max") {
                r0 = "in";
                r1 = "not in";
                return SET_VAL_MAX_INC();
            }
            if (s->id == "outdomain_min") {
                r1 = "in";
                r0 = "not in";
                return SET_VAL_MIN_EXC();
            }
            if (s->id == "outdomain_max") {
                r1 = "in";
                r0 = "not in";
                return SET_VAL_MAX_EXC();
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        r0 = "in";
        r1 = "not in";
        return SET_VAL_MIN_INC();
    }
#endif

#ifdef GECODE_HAS_FLOAT_VARS
    TieBreak<FloatVarBranch>
    annotation_to_float_var_branch(AST::Node* ann, Rnd rnd, double decay) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "input_order")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_NONE());
            if (s->id == "first_fail")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MIN());
            if (s->id == "anti_first_fail")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MAX());
            if (s->id == "smallest")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_MIN_MIN());
            if (s->id == "largest")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_MAX_MAX());
            if (s->id == "occurrence")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_DEGREE_MAX());
            if (s->id == "most_constrained")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MIN(), FLOAT_VAR_DEGREE_MAX());
            if (s->id == "random") {
                return TieBreak<FloatVarBranch>(FLOAT_VAR_RND(rnd));
            }
            if (s->id == "afc_min")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_MIN(decay));
            if (s->id == "afc_max")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_MAX(decay));
            if (s->id == "afc_size_min")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_SIZE_MIN(decay));
            if (s->id == "afc_size_max")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_SIZE_MAX(decay));
            if (s->id == "action_min")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTION_MIN(decay));
            if (s->id == "action_max")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTION_MAX(decay));
            if (s->id == "action_size_min")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTION_SIZE_MIN(decay));
            if (s->id == "action_size_max")
                return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTION_SIZE_MAX(decay));
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        return TieBreak<FloatVarBranch>(FLOAT_VAR_NONE());
    }

    FloatValBranch
    annotation_to_float_val_branch(AST::Node* ann, std::string r0, std::string r1) {
        if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
            if (s->id == "indomain_split") {
                r0 = "<=";
                r1 = ">";
                return FLOAT_VAL_SPLIT_MIN();
            }
            if (s->id == "indomain_reverse_split") {
                r1 = "<=";
                r0 = ">";
                return FLOAT_VAL_SPLIT_MAX();
            }
        }
        std::cerr << "Warning, ignored search annotation: ";
        ann->print(std::cerr);
        std::cerr << std::endl;
        r0 = "<=";
        r1 = ">";
        return FLOAT_VAL_SPLIT_MIN();
    }

#endif

    class FlatZincSpaceInitData {
    public:
        /// Hash table of tuple sets
        typedef std::unordered_set<TupleSet> TupleSetSet;
        /// Hash table of tuple sets
        TupleSetSet tuple_set_set;

        /// Hash table of shared integer arrays
        typedef std::unordered_set<SharedArray<int>> IntSharedArraySet;
        /// Hash table of shared integer arrays
        IntSharedArraySet int_shared_array_set;

        /// Hash table of DFAs
        typedef std::unordered_set<DFA> DFASet;
        /// Hash table of DFAs
        DFASet dfa_set;

        /// Opaque state shared by blackbox propagators in this model
        BlackBoxContextHandle black_box_context;

        /// Initialize
        FlatZincSpaceInitData(void) {}
    };

    FlatZincSpace::FlatZincSpace(FlatZincSpace& f)
        : Space(f), _init_data(nullptr), _random(f._random), _solve_annotations(nullptr), restart_data(f.restart_data),
          iv_boolalias(nullptr),
#ifdef GECODE_HAS_FLOAT_VARS
          step(f.step),
#endif
          need_aux_vars(f.need_aux_vars) {
        _opt_var = f._opt_var;
        _opt_var_is_int = f._opt_var_is_int;
        _method = f._method;
        _lns = f._lns;
        _lns_initial_solution = f._lns_initial_solution;
        branch_info = f.branch_info;
        iv.update(*this, f.iv);
        iv_lns.update(*this, f.iv_lns);
        int_var_count = f.int_var_count;

        on_restart_iv.update(*this, f.on_restart_iv);
        on_restart_bv.update(*this, f.on_restart_bv);
#ifdef GECODE_HAS_SET_VARS
        on_restart_sv.update(*this, f.on_restart_sv);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        on_restart_fv.update(*this, f.on_restart_fv);
#endif
        if (need_aux_vars) {
            IntVarArgs iva;
            for (int i = 0; i < f.iv_aux.size(); i++) {
                if (!f.iv_aux[i].assigned()) {
                    iva << IntVar();
                    iva[iva.size() - 1].update(*this, f.iv_aux[i]);
                }
            }
            iv_aux = IntVarArray(*this, iva);
        }

        bv.update(*this, f.bv);
        bool_var_count = f.bool_var_count;
        if (need_aux_vars) {
            BoolVarArgs bva;
            for (int i = 0; i < f.bv_aux.size(); i++) {
                if (!f.bv_aux[i].assigned()) {
                    bva << BoolVar();
                    bva[bva.size() - 1].update(*this, f.bv_aux[i]);
                }
            }
            bv_aux = BoolVarArray(*this, bva);
        }

#ifdef GECODE_HAS_SET_VARS
        sv.update(*this, f.sv);
        set_var_count = f.set_var_count;
        if (need_aux_vars) {
            SetVarArgs sva;
            for (int i = 0; i < f.sv_aux.size(); i++) {
                if (!f.sv_aux[i].assigned()) {
                    sva << SetVar();
                    sva[sva.size() - 1].update(*this, f.sv_aux[i]);
                }
            }
            sv_aux = SetVarArray(*this, sva);
        }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        fv.update(*this, f.fv);
        float_var_count = f.float_var_count;
        if (need_aux_vars) {
            FloatVarArgs fva;
            for (int i = 0; i < f.fv_aux.size(); i++) {
                if (!f.fv_aux[i].assigned()) {
                    fva << FloatVar();
                    fva[fva.size() - 1].update(*this, f.fv_aux[i]);
                }
            }
            fv_aux = FloatVarArray(*this, fva);
        }
#endif
    }

    FlatZincSpace::FlatZincSpace(Rnd& random)
        : _init_data(new FlatZincSpaceInitData), int_var_count(-1), bool_var_count(-1), float_var_count(-1),
          set_var_count(-1), _opt_var(-1), _opt_var_is_int(true), _lns(0), _lns_initial_solution(0), _random(random),
          _solve_annotations(nullptr), need_aux_vars(true) {
        branch_info.init();
    }

    BlackBoxContextHandle&
    BlackBoxAccess::context(FlatZincSpace& s) {
        assert(s._init_data != nullptr);
        return s._init_data->black_box_context;
    }

    void
    FlatZincSpace::init(int int_vars, int bool_vars, int set_vars, int float_vars) {
        (void)set_vars;
        (void)float_vars;

        int_var_count = 0;
        iv = IntVarArray(*this, int_vars);
        iv_introduced = std::vector<bool>(2 * int_vars);
        iv_boolalias = alloc<int>(int_vars + (int_vars == 0 ? 1 : 0));
        bool_var_count = 0;
        bv = BoolVarArray(*this, bool_vars);
        bv_introduced = std::vector<bool>(2 * bool_vars);
#ifdef GECODE_HAS_SET_VARS
        set_var_count = 0;
        sv = SetVarArray(*this, set_vars);
        sv_introduced = std::vector<bool>(2 * set_vars);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        float_var_count = 0;
        fv = FloatVarArray(*this, float_vars);
        fv_introduced = std::vector<bool>(2 * float_vars);
#endif
    }

    void
    FlatZincSpace::new_int_var(IntVarSpec* vs) {
        if (vs->alias) {
            iv[int_var_count++] = iv[vs->i];
        } else {
            IntSet dom(vs2is(vs));
            if (dom.size() == 0) {
                fail();
                return;
            } else {
                iv[int_var_count++] = IntVar(*this, dom);
            }
        }
        iv_introduced[2 * (int_var_count - 1)] = vs->introduced;
        iv_introduced[2 * (int_var_count - 1) + 1] = vs->func_dep;
        iv_boolalias[int_var_count - 1] = -1;
    }

    void
    FlatZincSpace::alias_bool2_int(int iv, int bv) {
        iv_boolalias[iv] = bv;
    }
    int
    FlatZincSpace::alias_bool2_int(int iv) {
        return iv_boolalias[iv];
    }

    void
    FlatZincSpace::new_bool_var(BoolVarSpec* vs) {
        if (vs->alias) {
            bv[bool_var_count++] = bv[vs->i];
        } else {
            bv[bool_var_count++] = BoolVar(*this, vs2bsl(vs), vs2bsh(vs));
        }
        bv_introduced[2 * (bool_var_count - 1)] = vs->introduced;
        bv_introduced[2 * (bool_var_count - 1) + 1] = vs->func_dep;
    }

#ifdef GECODE_HAS_SET_VARS
    void
    FlatZincSpace::new_set_var(SetVarSpec* vs) {
        if (vs->alias) {
            sv[set_var_count++] = sv[vs->i];
        } else if (vs->assigned) {
            assert(vs->upper_bound());
            AST::SetLit* vsv = vs->upper_bound.some();
            if (vsv->interval) {
                IntSet d(vsv->min, vsv->max);
                sv[set_var_count++] = SetVar(*this, d, d);
            } else {
                int* is = heap.alloc<int>(static_cast<unsigned long int>(vsv->s.size()));
                for (int i = vsv->s.size(); i--;)
                    is[i] = vsv->s[i];
                IntSet d(is, vsv->s.size());
                heap.free(is, static_cast<unsigned long int>(vsv->s.size()));
                sv[set_var_count++] = SetVar(*this, d, d);
            }
        } else if (vs->upper_bound()) {
            AST::SetLit* vsv = vs->upper_bound.some();
            if (vsv->interval) {
                IntSet d(vsv->min, vsv->max);
                sv[set_var_count++] = SetVar(*this, IntSet::empty, d);
            } else {
                int* is = heap.alloc<int>(static_cast<unsigned long int>(vsv->s.size()));
                for (int i = vsv->s.size(); i--;)
                    is[i] = vsv->s[i];
                IntSet d(is, vsv->s.size());
                heap.free(is, static_cast<unsigned long int>(vsv->s.size()));
                sv[set_var_count++] = SetVar(*this, IntSet::empty, d);
            }
        } else {
            sv[set_var_count++] = SetVar(*this, IntSet::empty, IntSet(Set::Limits::min, Set::Limits::max));
        }
        sv_introduced[2 * (set_var_count - 1)] = vs->introduced;
        sv_introduced[2 * (set_var_count - 1) + 1] = vs->func_dep;
    }
#else
    void
    FlatZincSpace::new_set_var(SetVarSpec*) {
        throw FlatZinc::Error("Gecode", "set variables not supported");
    }
#endif

#ifdef GECODE_HAS_FLOAT_VARS
    void
    FlatZincSpace::new_float_var(FloatVarSpec* vs) {
        if (vs->alias) {
            fv[float_var_count++] = fv[vs->i];
        } else {
            double dmin, dmax;
            if (vs->domain()) {
                dmin = vs->domain.some().first;
                dmax = vs->domain.some().second;
                if (dmin > dmax) {
                    fail();
                    return;
                }
            } else {
                dmin = Float::Limits::min;
                dmax = Float::Limits::max;
            }
            fv[float_var_count++] = FloatVar(*this, dmin, dmax);
        }
        fv_introduced[2 * (float_var_count - 1)] = vs->introduced;
        fv_introduced[2 * (float_var_count - 1) + 1] = vs->func_dep;
    }
#else
    void
    FlatZincSpace::new_float_var(FloatVarSpec*) {
        throw FlatZinc::Error("Gecode", "float variables not supported");
    }
#endif

    namespace {
        struct ConExprOrder {
            bool
            operator()(ConExpr* ce0, ConExpr* ce1) {
                return ce0->args->a.size() < ce1->args->a.size();
            }
        };
    } // namespace

    void
    FlatZincSpace::post_constraints(std::vector<ConExpr*>& ces) {
        ConExprOrder ceo;
        std::sort(ces.begin(), ces.end(), ceo);

        for (unsigned int i = 0; i < ces.size(); i++) {
            const ConExpr& ce = *ces[i];
            try {
                registry().post(*this, ce);
            } catch (Gecode::Exception& e) {
                throw FlatZinc::Error("Gecode", e.what(), ce.ann);
            } catch (AST::TypeError& e) {
                throw FlatZinc::Error("Type error", e.what(), ce.ann);
            }
            delete ces[i];
            ces[i] = nullptr;
        }
    }

    void
    flatten_annotations(AST::Array* ann, std::vector<AST::Node*>& out) {
        for (unsigned int i = 0; i < ann->a.size(); i++) {
            if (ann->a[i]->is_call("seq_search")) {
                AST::Call* c = ann->a[i]->get_call();
                if (c->args->is_array())
                    flatten_annotations(c->args->get_array(), out);
                else
                    out.push_back(c->args);
            } else {
                out.push_back(ann->a[i]);
            }
        }
    }

    void
    FlatZincSpace::create_branchers(Printer& p, AST::Node* ann, FlatZincOptions& opt, bool ignore_unknown,
                                    std::ostream& err) {
        int seed = opt.seed();
        double decay = opt.decay();
        Rnd rnd(static_cast<unsigned int>(seed));
        TieBreak<IntVarBranch> def_int_varsel = INT_VAR_AFC_SIZE_MAX(0.99);
        IntBoolVarBranch def_intbool_varsel = int_bool_var_afc_size_max(0.99);
        IntValBranch def_int_valsel = INT_VAL_MIN();
        std::string def_int_rel_left = "=";
        std::string def_int_rel_right = "!=";
        TieBreak<BoolVarBranch> def_bool_varsel = BOOL_VAR_AFC_MAX(0.99);
        BoolValBranch def_bool_valsel = BOOL_VAL_MIN();
        std::string def_bool_rel_left = "=";
        std::string def_bool_rel_right = "!=";
#ifdef GECODE_HAS_SET_VARS
        SetVarBranch def_set_varsel = SET_VAR_AFC_SIZE_MAX(0.99);
        SetValBranch def_set_valsel = SET_VAL_MIN_INC();
        std::string def_set_rel_left = "in";
        std::string def_set_rel_right = "not in";
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        TieBreak<FloatVarBranch> def_float_varsel = FLOAT_VAR_SIZE_MIN();
        FloatValBranch def_float_valsel = FLOAT_VAL_SPLIT_MIN();
        std::string def_float_rel_left = "<=";
        std::string def_float_rel_right = ">";
#endif

        std::vector<bool> iv_searched(iv.size());
        for (unsigned int i = iv.size(); i--;)
            iv_searched[i] = false;
        std::vector<bool> bv_searched(bv.size());
        for (unsigned int i = bv.size(); i--;)
            bv_searched[i] = false;
#ifdef GECODE_HAS_SET_VARS
        std::vector<bool> sv_searched(sv.size());
        for (unsigned int i = sv.size(); i--;)
            sv_searched[i] = false;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        std::vector<bool> fv_searched(fv.size());
        for (unsigned int i = fv.size(); i--;)
            fv_searched[i] = false;
#endif

        _lns = 0;
        if (ann) {
            std::vector<AST::Node*> flat_ann;
            if (ann->is_array()) {
                flatten_annotations(ann->get_array(), flat_ann);
            } else {
                flat_ann.push_back(ann);
            }

            for (unsigned int i = 0; i < flat_ann.size(); i++) {
                if (flat_ann[i]->is_call("restart_geometric")) {
                    AST::Call* call = flat_ann[i]->get_call("restart_geometric");
                    opt.restart(RM_GEOMETRIC);
                    AST::Array* args = call->get_args(2);
                    opt.restart_base(args->a[0]->get_float());
                    opt.restart_scale(args->a[1]->get_int());
                } else if (flat_ann[i]->is_call("restart_luby")) {
                    AST::Call* call = flat_ann[i]->get_call("restart_luby");
                    opt.restart(RM_LUBY);
                    opt.restart_scale(call->args->get_int());
                } else if (flat_ann[i]->is_call("restart_linear")) {
                    AST::Call* call = flat_ann[i]->get_call("restart_linear");
                    opt.restart(RM_LINEAR);
                    opt.restart_scale(call->args->get_int());
                } else if (flat_ann[i]->is_call("restart_constant")) {
                    AST::Call* call = flat_ann[i]->get_call("restart_constant");
                    opt.restart(RM_CONSTANT);
                    opt.restart_scale(call->args->get_int());
                } else if (flat_ann[i]->is_call("restart_none")) {
                    opt.restart(RM_NONE);
                } else if (flat_ann[i]->is_call("relax_and_reconstruct")) {
                    if (_lns != 0)
                        throw FlatZinc::Error("FlatZinc", "Only one relax_and_reconstruct annotation allowed");
                    AST::Call* call = flat_ann[i]->get_call("relax_and_reconstruct");
                    AST::Array* args;
                    if (call->args->get_array()->a.size() == 2) {
                        args = call->get_args(2);
                    } else {
                        args = call->get_args(3);
                    }
                    _lns = args->a[1]->get_int();
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_int())
                            k--;
                    iv_lns = IntVarArray(*this, k);
                    k = 0;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_int())
                            continue;
                        iv_lns[k++] = iv[vars->a[i]->get_int_var()];
                    }
                    if (args->a.size() == 3) {
                        AST::Array* initial = args->a[2]->get_array();
                        _lns_initial_solution = IntSharedArray(initial->a.size());
                        for (unsigned int i = initial->a.size(); i--;)
                            _lns_initial_solution[i] = initial->a[i]->get_int();
                    }
                } else if (flat_ann[i]->is_call("gecode_search")) {
                    AST::Call* c = flat_ann[i]->get_call();
                    branch_with_plugin(c->args);
                } else if (flat_ann[i]->is_call("int_search")) {
                    AST::Call* call = flat_ann[i]->get_call("int_search");
                    AST::Array* args = call->get_args(4);
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_int())
                            k--;
                    IntVarArgs va(k);
                    std::vector<std::string> names;
                    k = 0;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_int())
                            continue;
                        va[k++] = iv[vars->a[i]->get_int_var()];
                        iv_searched[vars->a[i]->get_int_var()] = true;
                        names.push_back(vars->a[i]->get_var_name());
                    }
                    std::string r0, r1;
                    {
                        BrancherGroup bg;
                        branch(bg(*this), va, annotation_to_int_var_branch(args->a[1], rnd, decay),
                               annotation_to_int_val_branch(args->a[2], r0, r1, rnd), nullptr, &var_val_print<IntVar>);
                        branch_info.add(bg, r0, r1, names);
                    }
                } else if (flat_ann[i]->is_call("int_assign")) {
                    AST::Call* call = flat_ann[i]->get_call("int_assign");
                    AST::Array* args = call->get_args(2);
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_int())
                            k--;
                    IntVarArgs va(k);
                    k = 0;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_int())
                            continue;
                        va[k++] = iv[vars->a[i]->get_int_var()];
                        iv_searched[vars->a[i]->get_int_var()] = true;
                    }
                    assign(*this, va, annotation_to_int_assign(args->a[1], rnd), nullptr, &var_val_print<IntVar>);
                } else if (flat_ann[i]->is_call("bool_search")) {
                    AST::Call* call = flat_ann[i]->get_call("bool_search");
                    AST::Array* args = call->get_args(4);
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_bool())
                            k--;
                    BoolVarArgs va(k);
                    k = 0;
                    std::vector<std::string> names;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_bool())
                            continue;
                        va[k++] = bv[vars->a[i]->get_bool_var()];
                        bv_searched[vars->a[i]->get_bool_var()] = true;
                        names.push_back(vars->a[i]->get_var_name());
                    }

                    std::string r0, r1;
                    {
                        BrancherGroup bg;
                        branch(bg(*this), va, annotation_to_bool_var_branch(args->a[1], rnd, decay),
                               annotation_to_bool_val_branch(args->a[2], r0, r1, rnd), nullptr,
                               &var_val_print<BoolVar>);
                        branch_info.add(bg, r0, r1, names);
                    }
                } else if (flat_ann[i]->is_call("int_default_search")) {
                    AST::Call* call = flat_ann[i]->get_call("int_default_search");
                    AST::Array* args = call->get_args(2);
                    def_int_varsel = annotation_to_int_var_branch(args->a[0], rnd, decay);
                    def_int_valsel = annotation_to_int_val_branch(args->a[1], def_int_rel_left, def_int_rel_right, rnd);
                } else if (flat_ann[i]->is_call("bool_default_search")) {
                    AST::Call* call = flat_ann[i]->get_call("bool_default_search");
                    AST::Array* args = call->get_args(2);
                    def_bool_varsel = annotation_to_bool_var_branch(args->a[0], rnd, decay);
                    def_bool_valsel =
                        annotation_to_bool_val_branch(args->a[1], def_bool_rel_left, def_bool_rel_right, rnd);
                } else if (flat_ann[i]->is_call("set_search")) {
#ifdef GECODE_HAS_SET_VARS
                    AST::Call* call = flat_ann[i]->get_call("set_search");
                    AST::Array* args = call->get_args(4);
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_set())
                            k--;
                    SetVarArgs va(k);
                    k = 0;
                    std::vector<std::string> names;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_set())
                            continue;
                        va[k++] = sv[vars->a[i]->get_set_var()];
                        sv_searched[vars->a[i]->get_set_var()] = true;
                        names.push_back(vars->a[i]->get_var_name());
                    }
                    std::string r0, r1;
                    {
                        BrancherGroup bg;
                        branch(bg(*this), va, annotation_to_set_var_branch(args->a[1], rnd, decay),
                               annotation_to_set_val_branch(args->a[2], r0, r1, rnd), nullptr, &var_val_print<SetVar>);
                        branch_info.add(bg, r0, r1, names);
                    }
#else
                    if (!ignoreUnknown) {
                        err << "Warning, ignored search annotation: ";
                        flatAnn[i]->print(err);
                        err << std::endl;
                    }
#endif
                } else if (flat_ann[i]->is_call("set_default_search")) {
#ifdef GECODE_HAS_SET_VARS
                    AST::Call* call = flat_ann[i]->get_call("set_default_search");
                    AST::Array* args = call->get_args(2);
                    def_set_varsel = annotation_to_set_var_branch(args->a[0], rnd, decay);
                    def_set_valsel = annotation_to_set_val_branch(args->a[1], def_set_rel_left, def_set_rel_right, rnd);
#else
                    if (!ignoreUnknown) {
                        err << "Warning, ignored search annotation: ";
                        flatAnn[i]->print(err);
                        err << std::endl;
                    }
#endif
                } else if (flat_ann[i]->is_call("float_default_search")) {
#ifdef GECODE_HAS_FLOAT_VARS
                    AST::Call* call = flat_ann[i]->get_call("float_default_search");
                    AST::Array* args = call->get_args(2);
                    def_float_varsel = annotation_to_float_var_branch(args->a[0], rnd, decay);
                    def_float_valsel =
                        annotation_to_float_val_branch(args->a[1], def_float_rel_left, def_float_rel_right);
#else
                    if (!ignoreUnknown) {
                        err << "Warning, ignored search annotation: ";
                        flatAnn[i]->print(err);
                        err << std::endl;
                    }
#endif
                } else if (flat_ann[i]->is_call("float_search")) {
#ifdef GECODE_HAS_FLOAT_VARS
                    AST::Call* call = flat_ann[i]->get_call("float_search");
                    AST::Array* args = call->get_args(5);
                    AST::Array* vars = args->a[0]->get_array();
                    int k = vars->a.size();
                    for (int i = vars->a.size(); i--;)
                        if (vars->a[i]->is_float())
                            k--;
                    FloatVarArgs va(k);
                    k = 0;
                    std::vector<std::string> names;
                    for (unsigned int i = 0; i < vars->a.size(); i++) {
                        if (vars->a[i]->is_float())
                            continue;
                        va[k++] = fv[vars->a[i]->get_float_var()];
                        fv_searched[vars->a[i]->get_float_var()] = true;
                        names.push_back(vars->a[i]->get_var_name());
                    }
                    std::string r0, r1;
                    {
                        BrancherGroup bg;
                        branch(bg(*this), va, annotation_to_float_var_branch(args->a[2], rnd, decay),
                               annotation_to_float_val_branch(args->a[3], r0, r1), nullptr, &var_val_print_f);
                        branch_info.add(bg, r0, r1, names);
                    }
#else
                    if (!ignoreUnknown) {
                        err << "Warning, ignored search annotation: ";
                        flatAnn[i]->print(err);
                        err << std::endl;
                    }
#endif
                } else {
                    if (!ignore_unknown) {
                        err << "Warning, ignored search annotation: ";
                        flat_ann[i]->print(err);
                        err << std::endl;
                    }
                }
            }
        }
        int introduced = 0;
        int funcdep = 0;
        int searched = 0;
        for (int i = iv.size(); i--;) {
            if (iv_searched[i] || (_method != SAT && _opt_var_is_int && _opt_var == i)) {
                searched++;
            } else if (iv_introduced[2 * i]) {
                if (iv_introduced[2 * i + 1]) {
                    funcdep++;
                } else {
                    introduced++;
                }
            }
        }
        std::vector<std::string> iv_sol_names(iv.size() - (introduced + funcdep + searched));
        IntVarArgs iv_sol(iv.size() - (introduced + funcdep + searched));
        std::vector<std::string> iv_tmp_names(introduced);
        IntVarArgs iv_tmp(introduced);
        for (int i = iv.size(), j = 0, k = 0; i--;) {
            if (iv_searched[i] || (_method != SAT && _opt_var_is_int && _opt_var == i))
                continue;
            if (iv_introduced[2 * i]) {
                if (!iv_introduced[2 * i + 1]) {
                    iv_tmp_names[j] = p.int_var_name(i);
                    iv_tmp[j++] = iv[i];
                }
            } else {
                iv_sol_names[k] = p.int_var_name(i);
                iv_sol[k++] = iv[i];
            }
        }

        introduced = 0;
        funcdep = 0;
        searched = 0;
        for (int i = bv.size(); i--;) {
            if (bv_searched[i]) {
                searched++;
            } else if (bv_introduced[2 * i]) {
                if (bv_introduced[2 * i + 1]) {
                    funcdep++;
                } else {
                    introduced++;
                }
            }
        }
        std::vector<std::string> bv_sol_names(bv.size() - (introduced + funcdep + searched));
        BoolVarArgs bv_sol(bv.size() - (introduced + funcdep + searched));
        BoolVarArgs bv_tmp(introduced);
        std::vector<std::string> bv_tmp_names(introduced);
        for (int i = bv.size(), j = 0, k = 0; i--;) {
            if (bv_searched[i])
                continue;
            if (bv_introduced[2 * i]) {
                if (!bv_introduced[2 * i + 1]) {
                    bv_tmp_names[j] = p.bool_var_name(i);
                    bv_tmp[j++] = bv[i];
                }
            } else {
                bv_sol_names[k] = p.bool_var_name(i);
                bv_sol[k++] = bv[i];
            }
        }

        if (iv_sol.size() > 0 && bv_sol.size() > 0) {
            branch(*this, iv_sol, bv_sol, def_intbool_varsel, def_int_valsel);
        } else if (iv_sol.size() > 0) {
            BrancherGroup bg;
            branch(bg(*this), iv_sol, def_int_varsel, def_int_valsel, nullptr, &var_val_print<IntVar>);
            branch_info.add(bg, def_int_rel_left, def_int_rel_right, iv_sol_names);
        } else if (bv_sol.size() > 0) {
            BrancherGroup bg;
            branch(bg(*this), bv_sol, def_bool_varsel, def_bool_valsel, nullptr, &var_val_print<BoolVar>);
            branch_info.add(bg, def_bool_rel_left, def_bool_rel_right, bv_sol_names);
        }
#ifdef GECODE_HAS_FLOAT_VARS
        introduced = 0;
        funcdep = 0;
        searched = 0;
        for (int i = fv.size(); i--;) {
            if (fv_searched[i] || (_method != SAT && !_opt_var_is_int && _opt_var == i)) {
                searched++;
            } else if (fv_introduced[2 * i]) {
                if (fv_introduced[2 * i + 1]) {
                    funcdep++;
                } else {
                    introduced++;
                }
            }
        }
        std::vector<std::string> fv_sol_names(fv.size() - (introduced + funcdep + searched));
        FloatVarArgs fv_sol(fv.size() - (introduced + funcdep + searched));
        FloatVarArgs fv_tmp(introduced);
        std::vector<std::string> fv_tmp_names(introduced);
        for (int i = fv.size(), j = 0, k = 0; i--;) {
            if (fv_searched[i] || (_method != SAT && !_opt_var_is_int && _opt_var == i))
                continue;
            if (fv_introduced[2 * i]) {
                if (!fv_introduced[2 * i + 1]) {
                    fv_tmp_names[j] = p.float_var_name(i);
                    fv_tmp[j++] = fv[i];
                }
            } else {
                fv_sol_names[k] = p.float_var_name(i);
                fv_sol[k++] = fv[i];
            }
        }

        if (fv_sol.size() > 0) {
            BrancherGroup bg;
            branch(bg(*this), fv_sol, def_float_varsel, def_float_valsel, nullptr, &var_val_print_f);
            branch_info.add(bg, def_float_rel_left, def_float_rel_right, fv_sol_names);
        }
#endif
#ifdef GECODE_HAS_SET_VARS
        introduced = 0;
        funcdep = 0;
        searched = 0;
        for (int i = sv.size(); i--;) {
            if (sv_searched[i]) {
                searched++;
            } else if (sv_introduced[2 * i]) {
                if (sv_introduced[2 * i + 1]) {
                    funcdep++;
                } else {
                    introduced++;
                }
            }
        }
        std::vector<std::string> sv_sol_names(sv.size() - (introduced + funcdep + searched));
        SetVarArgs sv_sol(sv.size() - (introduced + funcdep + searched));
        SetVarArgs sv_tmp(introduced);
        std::vector<std::string> sv_tmp_names(introduced);
        for (int i = sv.size(), j = 0, k = 0; i--;) {
            if (sv_searched[i])
                continue;
            if (sv_introduced[2 * i]) {
                if (!sv_introduced[2 * i + 1]) {
                    sv_tmp_names[j] = p.set_var_name(i);
                    sv_tmp[j++] = sv[i];
                }
            } else {
                sv_sol_names[k] = p.set_var_name(i);
                sv_sol[k++] = sv[i];
            }
        }

        if (sv_sol.size() > 0) {
            BrancherGroup bg;
            branch(bg(*this), sv_sol, def_set_varsel, def_set_valsel, nullptr, &var_val_print<SetVar>);
            branch_info.add(bg, def_set_rel_left, def_set_rel_right, sv_sol_names);
        }
#endif
        iv_aux = IntVarArray(*this, iv_tmp);
        bv_aux = BoolVarArray(*this, bv_tmp);
        int n_aux = iv_aux.size() + bv_aux.size();
#ifdef GECODE_HAS_SET_VARS
        sv_aux = SetVarArray(*this, sv_tmp);
        n_aux += sv_aux.size();
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        fv_aux = FloatVarArray(*this, fv_tmp);
        n_aux += fv_aux.size();
#endif

        if (n_aux > 0) {
            if (_method == SAT) {
                AuxVarBrancher::post(*this, def_int_varsel, def_int_valsel, def_bool_varsel, def_bool_valsel
#ifdef GECODE_HAS_SET_VARS
                                     ,
                                     def_set_varsel, def_set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                                     ,
                                     def_float_varsel, def_float_valsel
#endif
                );
            } else {
                {
                    BrancherGroup bg;
                    branch(bg(*this), iv_aux, def_int_varsel, def_int_valsel, nullptr, &var_val_print<IntVar>);
                    branch_info.add(bg, def_int_rel_left, def_int_rel_right, iv_tmp_names);
                }
                {
                    BrancherGroup bg;
                    branch(bg(*this), bv_aux, def_bool_varsel, def_bool_valsel, nullptr, &var_val_print<BoolVar>);
                    branch_info.add(bg, def_bool_rel_left, def_bool_rel_right, bv_tmp_names);
                }
#ifdef GECODE_HAS_SET_VARS
                {
                    BrancherGroup bg;
                    branch(bg(*this), sv_aux, def_set_varsel, def_set_valsel, nullptr, &var_val_print<SetVar>);
                    branch_info.add(bg, def_set_rel_left, def_set_rel_right, sv_tmp_names);
                }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                {
                    BrancherGroup bg;
                    branch(bg(*this), fv_aux, def_float_varsel, def_float_valsel, nullptr, &var_val_print_f);
                    branch_info.add(bg, def_float_rel_left, def_float_rel_right, fv_tmp_names);
                }
#endif
            }
        }

        if (_method == MIN) {
            if (_opt_var_is_int) {
                std::vector<std::string> names(1);
                names[0] = p.int_var_name(_opt_var);
                BrancherGroup bg;
                branch(bg(*this), iv[_opt_var], INT_VAL_MIN(), &var_val_print<IntVar>);
                branch_info.add(bg, "=", "!=", names);
            } else {
#ifdef GECODE_HAS_FLOAT_VARS
                std::vector<std::string> names(1);
                names[0] = p.float_var_name(_opt_var);
                BrancherGroup bg;
                branch(bg(*this), fv[_opt_var], FLOAT_VAL_SPLIT_MIN(), &var_val_print_f);
                branch_info.add(bg, "<=", ">", names);
#endif
            }
        } else if (_method == MAX) {
            if (_opt_var_is_int) {
                std::vector<std::string> names(1);
                names[0] = p.int_var_name(_opt_var);
                BrancherGroup bg;
                branch(bg(*this), iv[_opt_var], INT_VAL_MAX(), &var_val_print<IntVar>);
                branch_info.add(bg, "=", "!=", names);
            } else {
#ifdef GECODE_HAS_FLOAT_VARS
                std::vector<std::string> names(1);
                names[0] = p.float_var_name(_opt_var);
                BrancherGroup bg;
                branch(bg(*this), fv[_opt_var], FLOAT_VAL_SPLIT_MAX(), &var_val_print_f);
                branch_info.add(bg, "<=", ">", names);
#endif
            }
        }
    }

    AST::Array*
    FlatZincSpace::solve_annotations(void) const {
        return _solve_annotations;
    }

    void
    FlatZincSpace::solve(AST::Array* ann) {
        _method = SAT;
        _solve_annotations = ann;
    }

    void
    FlatZincSpace::minimize(int var, bool is_int, AST::Array* ann) {
        _method = MIN;
        _opt_var = var;
        _opt_var_is_int = is_int;
        _solve_annotations = ann;
    }

    void
    FlatZincSpace::maximize(int var, bool is_int, AST::Array* ann) {
        _method = MAX;
        _opt_var = var;
        _opt_var_is_int = is_int;
        _solve_annotations = ann;
    }

    FlatZincSpace::~FlatZincSpace(void) {
        delete _init_data;
        delete _solve_annotations;
    }

#ifdef GECODE_HAS_GIST

    /**
     * \brief Traits class for search engines
     */
    template <class Engine>
    class GistEngine {};

    /// Specialization for DFS
    template <typename S>
    class GistEngine<DFS<S>> {
    public:
        static void
        explore(S* root, const FlatZincOptions& opt, Gist::Inspector* i, Gist::Comparator* c) {
            Gecode::Gist::Options o;
            o.c_d = opt.c_d();
            o.a_d = opt.a_d();
            o.inspect.click(i);
            o.inspect.compare(c);
            (void)Gecode::Gist::dfs(root, o);
        }
    };

    /// Specialization for BAB
    template <typename S>
    class GistEngine<BAB<S>> {
    public:
        static void
        explore(S* root, const FlatZincOptions& opt, Gist::Inspector* i, Gist::Comparator* c) {
            Gecode::Gist::Options o;
            o.c_d = opt.c_d();
            o.a_d = opt.a_d();
            o.inspect.click(i);
            o.inspect.compare(c);
            (void)Gecode::Gist::bab(root, o);
        }
    };

    /// \brief An inspector for printing simple text output
    template <class S>
    class FZPrintingInspector : public Gecode::Gist::TextOutput, public Gecode::Gist::Inspector {
    private:
        const Printer& p;

    public:
        /// Constructor
        FZPrintingInspector(const Printer& p0);
        /// Use the print method of the template class S to print a space
        void inspect(const Space& node) override;
        /// Finalize when Gist exits
        void finalize(void) override;
    };

    template <class S>
    FZPrintingInspector<S>::FZPrintingInspector(const Printer& p0) : TextOutput("Gecode/FlatZinc"), p(p0) {}

    template <class S>
    void
    FZPrintingInspector<S>::inspect(const Space& node) {
        init();
        dynamic_cast<const S&>(node).print(get_stream(), p);
        get_stream() << std::endl;
    }

    template <class S>
    void
    FZPrintingInspector<S>::finalize(void) {
        Gecode::Gist::TextOutput::finalize();
    }

    template <class S>
    class FZPrintingComparator : public Gecode::Gist::VarComparator<S> {
    private:
        const Printer& p;

    public:
        /// Constructor
        FZPrintingComparator(const Printer& p0);

        /// Use the compare method of the template class S to compare two spaces
        void compare(const Space& s0, const Space& s1) override;
    };

    template <class S>
    FZPrintingComparator<S>::FZPrintingComparator(const Printer& p0)
        : Gecode::Gist::VarComparator<S>("Gecode/FlatZinc"), p(p0) {}

    template <class S>
    void
    FZPrintingComparator<S>::compare(const Space& s0, const Space& s1) {
        this->init();
        try {
            dynamic_cast<const S&>(s0).compare(dynamic_cast<const S&>(s1), this->get_stream(), p);
        } catch (Exception& e) {
            this->get_stream() << "Exception: " << e.what();
        }
        this->get_stream() << std::endl;
    }

#endif

    class FlatZincStop : public Search::Stop {
    protected:
        std::unique_ptr<Search::Stop> stop_object;
        BlackBoxContextHandle black_box_context;

    public:
        FlatZincStop(Search::Stop* stop_object0, const BlackBoxContextHandle& black_box_context0)
            : stop_object(stop_object0), black_box_context(black_box_context0) {}
        bool
        stop(const Search::Statistics& s, const Search::Options& o) override {
            return black_box_context.failed() || ((stop_object.get() != nullptr) && stop_object->stop(s, o));
        }
    };

    class InterruptHandlerGuard {
    protected:
        bool installed;

    public:
        explicit InterruptHandlerGuard(bool install) : installed(install) {
            if (installed)
                Driver::CombinedStop::install_ctrl_handler(true);
        }
        ~InterruptHandlerGuard(void) {
            if (installed)
                Driver::CombinedStop::install_ctrl_handler(false);
        }
    };

    template <template <class> class Engine>
    void
    FlatZincSpace::run_engine(std::ostream& out, const Printer& p, const FlatZincOptions& opt,
                              Support::Timer& t_total) {
        if (opt.restart() == RM_NONE) {
            run_meta<Engine, Driver::EngineToMeta>(out, p, opt, t_total);
        } else {
            run_meta<Engine, RBS>(out, p, opt, t_total);
        }
    }

#ifdef GECODE_HAS_CPPROFILER

    class FlatZincGetInfo : public CPProfilerSearchTracer::GetInfo {
    public:
        const Printer& p;
        FlatZincGetInfo(const Printer& printer) : p(printer) {}
        std::string
        get_info(const Space& space) const override {
            std::stringstream ss;
            if (const FlatZincSpace* fz_space = dynamic_cast<const FlatZincSpace*>(&space)) {
                ss << "{\n\t\"domains\": \"";
                ss << fz_space->get_domains(p);
                ss << "\"\n}";
            }
            return ss.str();
        }
        ~FlatZincGetInfo(void) override {};
    };

    void
    print_int_var(std::ostream& os, const std::string name, const Int::IntView& x) {
        os << "var ";
        if (x.assigned()) {
            os << "int: " << name << " = " << x.val() << ";";
        } else if (x.range()) {
            os << x.min() << ".." << x.max() << ": " << name << ";";
        } else {
            os << "array_union([";
            Gecode::Int::ViewRanges<Int::IntView> r(x);
            while (true) {
                os << r.min() << ".." << r.max();
                ++r;
                if (!r())
                    break;
                os << ',';
            }
            os << "]): " << name << ";";
        }
        os << "\n";
    }
    void
    print_bool_var(std::ostream& os, const std::string name, const BoolVar& b) {
        os << "var bool: " << name;
        if (b.assigned())
            os << " = " << (b.val() ? "true" : "false");
        os << ";\n";
    }
#ifdef GECODE_HAS_FLOAT_VARS
    void
    print_float_var(std::ostream& os, const std::string name, const Float::FloatView& f) {
        os << "var ";
        if (f.assigned())
            os << "float: " << name << " = " << f.med() << ";";
        else
            os << f.min() << ".." << f.max() << ": " << name << ";";
        os << "\n";
    }
#endif
    std::string
    FlatZincSpace::get_domains(const Printer& p) const {
        std::ostringstream oss;

        for (int i = 0; i < iv.size(); i++)
            print_int_var(oss, p.int_var_name(i), iv[i]);

        for (int i = 0; i < bv.size(); i++)
            print_bool_var(oss, p.bool_var_name(i), bv[i]);

#ifdef GECODE_HAS_FLOAT_VARS
        for (int i = 0; i < fv.size(); i++)
            print_float_var(oss, p.float_var_name(i), fv[i]);
#endif
#ifdef GECODE_HAS_SET_VARS
        for (int i = 0; i < sv.size(); i++)
            oss << "var " << sv[i] << ": " << p.set_var_name(i) << ";" << std::endl;
#endif

        return oss.str();
    }

#endif

    template <template <class> class Engine, template <class, template <class> class> class Meta>
    void
    FlatZincSpace::run_meta(std::ostream& out, const Printer& p, const FlatZincOptions& opt, Support::Timer& t_total) {
#ifdef GECODE_HAS_GIST
        if (opt.mode() == SM_GIST) {
            BlackBoxContextHandle& black_box_context = BlackBoxAccess::context(*this);
            (void)status();
            black_box_context.rethrow();
            FZPrintingInspector<FlatZincSpace> pi(p);
            FZPrintingComparator<FlatZincSpace> pc(p);
            (void)GistEngine<Engine<FlatZincSpace>>::explore(this, opt, &pi, &pc);
            black_box_context.rethrow();
            return;
        }
#endif
        StatusStatistics sstat;
        unsigned int n_p = 0;
        Support::Timer t_solve;
        t_solve.start();
        if (status(sstat) != SS_FAILED) {
            n_p = PropagatorGroup::all.size(*this);
        }
        BlackBoxContextHandle& black_box_context = BlackBoxAccess::context(*this);
        black_box_context.rethrow();
        Search::Options o;
        std::unique_ptr<Search::Stop> stop(
            Driver::CombinedStop::create(opt.node(), opt.fail(), opt.time(), opt.restart_limit(), true));
        if (black_box_context) {
            stop.reset(new FlatZincStop(stop.release(), black_box_context));
        }
        o.stop = stop.get();
        std::unique_ptr<SearchTracer> tracer;
        o.c_d = opt.c_d();
        o.a_d = opt.a_d();

#ifdef GECODE_HAS_CPPROFILER
        if (opt.profiler_port()) {
            FlatZincGetInfo* get_info = nullptr;
            if (opt.profiler_info())
                get_info = new FlatZincGetInfo(p);
            tracer.reset(new CPProfilerSearchTracer(opt.profiler_id(), opt.name(), opt.profiler_port(), get_info));
            o.tracer = tracer.get();
        }

#endif

#ifdef GECODE_HAS_FLOAT_VARS
        step = opt.step();
#endif
        o.threads = opt.threads();
        o.nogoods_limit = opt.nogoods() ? opt.nogoods_limit() : 0;
        if (opt.restart() != RM_NONE)
            o.cutoff = new Search::CutoffAppend(new Search::CutoffConstant(0), 1, Driver::create_cutoff(opt));
        int no_of_solutions = opt.solutions();
        if (no_of_solutions == -1) {
            no_of_solutions = (_method == SAT) ? 1 : 0;
        }
        bool print_all = _method == SAT || opt.all_solutions() || no_of_solutions != 0;
        int find_sol = no_of_solutions;
        bool solution_limit_reached = false;
        bool engine_stopped = false;
        Gecode::Search::Statistics stat;
        std::unique_ptr<FlatZincSpace> sol;
        {
            InterruptHandlerGuard interrupt_handler(opt.interrupt());
            Meta<FlatZincSpace, Engine> se(this, o);
            while (FlatZincSpace* next = se.next()) {
                std::unique_ptr<FlatZincSpace> next_sol(next);
                if (black_box_context.failed()) {
                    break;
                }
                sol = std::move(next_sol);
                if (print_all) {
                    sol->print(out, p);
                    out << "----------" << std::endl;
                }
                if (--find_sol == 0) {
                    solution_limit_reached = true;
                    break;
                }
            }
            engine_stopped = se.stopped();
            if (opt.mode() == SM_STAT) {
                stat = se.statistics();
            }
        }
        if (black_box_context.failed()) {
            sol.reset();
            black_box_context.rethrow();
        }
        if (sol && !print_all) {
            sol->print(out, p);
            out << "----------" << std::endl;
        }
        if (!solution_limit_reached) {
            if (!engine_stopped) {
                if (sol) {
                    out << "==========" << std::endl;
                } else {
                    out << "=====UNSATISFIABLE=====" << std::endl;
                }
            } else if (!sol) {
                out << "=====UNKNOWN=====" << std::endl;
            }
        }
        if (opt.mode() == SM_STAT) {
            double total_time = (t_total.stop() / 1000.0);
            double solve_time = (t_solve.stop() / 1000.0);
            double init_time = total_time - solve_time;
            out << std::endl << "%%%mzn-stat: initTime=" << init_time << std::endl;
            out << "%%%mzn-stat: solveTime=" << solve_time << std::endl;
            out << "%%%mzn-stat: solutions=" << std::abs(no_of_solutions - find_sol) << std::endl
                << "%%%mzn-stat: variables=" << (int_var_count + bool_var_count + set_var_count) << std::endl
                << "%%%mzn-stat: propagators=" << n_p << std::endl
                << "%%%mzn-stat: propagations=" << sstat.propagate + stat.propagate << std::endl
                << "%%%mzn-stat: nodes=" << stat.node << std::endl
                << "%%%mzn-stat: failures=" << stat.fail << std::endl
                << "%%%mzn-stat: restarts=" << stat.restart << std::endl
                << "%%%mzn-stat: peakDepth=" << stat.depth << std::endl
                << "%%%mzn-stat-end" << std::endl
                << std::endl;
        }
    }

#ifdef GECODE_HAS_QT
    void
    FlatZincSpace::branch_with_plugin(AST::Node* ann) {
        if (AST::Call* c = dynamic_cast<AST::Call*>(ann)) {
            QString plugin_name(c->id.c_str());
            if (QLibrary::isLibrary(plugin_name + ".dll")) {
                plugin_name += ".dll";
            } else if (QLibrary::isLibrary(plugin_name + ".dylib")) {
                plugin_name = "lib" + plugin_name + ".dylib";
            } else if (QLibrary::isLibrary(plugin_name + ".so")) {
                // Must check .so after .dylib so that Mac OS uses .dylib
                plugin_name = "lib" + plugin_name + ".so";
            }
            QPluginLoader pl(plugin_name);
            QObject* plugin_o = pl.instance();
            if (!plugin_o) {
                throw FlatZinc::Error("FlatZinc", "Error loading plugin " + plugin_name.toStdString() + ": " +
                                                      pl.errorString().toStdString());
            }
            BranchPlugin* pb = qobject_cast<BranchPlugin*>(plugin_o);
            if (!pb) {
                throw FlatZinc::Error("FlatZinc", "Error loading plugin " + plugin_name.toStdString() +
                                                      ": does not contain valid PluginBrancher");
            }
            pb->branch(*this, c);
        }
    }
#else
    void
    FlatZincSpace::branch_with_plugin(AST::Node*) {
        throw FlatZinc::Error("FlatZinc", "Branching with plugins not supported (requires Qt support)");
    }
#endif

    void
    FlatZincSpace::run(std::ostream& out, const Printer& p, const FlatZincOptions& opt, Support::Timer& t_total) {
#ifndef GECODE_HAS_GIST
        if (opt.mode() == SM_GIST)
            throw FlatZinc::Error("FlatZinc", "Gist mode is unavailable in this build");
#endif
        switch (_method) {
        case MIN:
        case MAX:
            run_engine<BAB>(out, p, opt, t_total);
            break;
        case SAT:
            run_engine<DFS>(out, p, opt, t_total);
            break;
        }
    }

    void
    FlatZincSpace::constrain(const Space& s) {
        if (_opt_var_is_int) {
            if (_method == MIN)
                rel(*this, iv[_opt_var], IRT_LE, static_cast<const FlatZincSpace*>(&s)->iv[_opt_var].val());
            else if (_method == MAX)
                rel(*this, iv[_opt_var], IRT_GR, static_cast<const FlatZincSpace*>(&s)->iv[_opt_var].val());
        } else {
#ifdef GECODE_HAS_FLOAT_VARS
            if (_method == MIN)
                rel(*this, fv[_opt_var], FRT_LE, static_cast<const FlatZincSpace*>(&s)->fv[_opt_var].val() - step);
            else if (_method == MAX)
                rel(*this, fv[_opt_var], FRT_GR, static_cast<const FlatZincSpace*>(&s)->fv[_opt_var].val() + step);
#endif
        }
    }

    bool
    FlatZincSpace::slave(const MetaInfo& mi) {
        if (mi.type() == MetaInfo::RESTART) {
            if (restart_data.initialized() && restart_data().mark_complete) {
                // Fail the space
                this->fail();
                // Return true to signal we are in the global search space
                return true;
            }

            bool ret = false;
            if (on_restart_iv.size() > 0) {
                int base = 0;

                // Assign the sol_int values (use lb if no solution is known)
                const FlatZincSpace* last = static_cast<const FlatZincSpace*>(mi.last());
                assert(last == nullptr || last->on_restart_iv.size() == restart_data().on_restart_iv_sol);
                for (int i = 0; i < restart_data().on_restart_iv_sol; ++i) {
                    IntVar& out_var = on_restart_iv[base + restart_data().on_restart_iv_sol + i];
                    int sol_val = (last != nullptr) ? last->on_restart_iv[i].val() : on_restart_iv[base + i].min();
                    rel(*this, out_var, IRT_EQ, sol_val);
                }
                base += restart_data().on_restart_iv_sol * 2;

                // Assign last_val_int values
                for (size_t i = 0; i < restart_data().last_val_int.size(); ++i) {
                    rel(*this, on_restart_iv[base + i], IRT_EQ, restart_data().last_val_int[i]);
                }
                base += restart_data().last_val_int.size();

                // Assign uniform_int random values
                for (size_t i = 0; i < restart_data().uniform_range_int.size(); ++i) {
                    const auto& range = restart_data().uniform_range_int[i];
                    const unsigned long long int width =
                        static_cast<unsigned long long int>(static_cast<long long int>(range.second) -
                                                            static_cast<long long int>(range.first)) +
                        1ULL;
                    const int rnd_val = static_cast<int>(static_cast<long long int>(range.first) +
                                                         static_cast<long long int>(_random(width)));
                    rel(*this, on_restart_iv[base + i], IRT_EQ, rnd_val);
                }
                base += restart_data().uniform_range_int.size();

                // Set the status value
                if (restart_data().on_restart_status) {
                    assert(base == on_restart_iv.size() - 1);
                    IntVar& restart_status = on_restart_iv[base];
                    switch (mi.reason()) {
                    case MetaInfo::RR_INIT:
                        assert(!mi.last());
                        rel(*this, restart_status, IRT_EQ, 1); // 1: START
                        break;
                    case MetaInfo::RR_SOL:
                        assert(mi.solution() > 0);
                        rel(*this, restart_status, IRT_EQ, 4); // 4: SAT
                        break;
                    case MetaInfo::RR_CMPL:
                        rel(*this, restart_status, IRT_EQ, 3); // 3: UNSAT
                        break;
                    default:
                        assert(mi.reason() == MetaInfo::RR_LIM);
                        rel(*this, restart_status, IRT_EQ, 2); // 2: UNKNOWN
                        break;
                    }
                    base += 1;
                }
                assert(base == on_restart_iv.size());

                // Reduce on_restart_iv to only the variables required in a solution
                IntVarArray tmp(*this, restart_data().on_restart_iv_sol);
                for (int i = 0; i < restart_data().on_restart_iv_sol; ++i) {
                    tmp[i] = on_restart_iv[i];
                }
                on_restart_iv = tmp;

                ret = true;
            }
            if (on_restart_bv.size() > 0) {
                int base = 0;

                // Assign the sol_bool values (use lb if no solution is known)
                const FlatZincSpace* last = static_cast<const FlatZincSpace*>(mi.last());
                assert(last == nullptr || last->on_restart_bv.size() == restart_data().on_restart_bv_sol);
                for (int i = 0; i < restart_data().on_restart_bv_sol; ++i) {
                    BoolVar& out_var = on_restart_bv[base + restart_data().on_restart_bv_sol + i];
                    int sol_val = (last != nullptr) ? last->on_restart_bv[i].val() : on_restart_bv[base + i].min();
                    rel(*this, out_var, IRT_EQ, sol_val);
                }
                base += restart_data().on_restart_bv_sol * 2;

                // Assign last_val_bool values
                for (size_t i = 0; i < restart_data().last_val_bool.size(); ++i) {
                    rel(*this, on_restart_bv[base + i], IRT_EQ, restart_data().last_val_bool[i]);
                }
                base += restart_data().last_val_bool.size();
                assert(base == on_restart_bv.size());

                // Reduce on_restart_bv to only the variables required in a solution
                BoolVarArray tmp(*this, restart_data().on_restart_bv_sol);
                for (int i = 0; i < restart_data().on_restart_bv_sol; ++i) {
                    tmp[i] = on_restart_bv[i];
                }
                on_restart_bv = tmp;

                ret = true;
            }
#ifdef GECODE_HAS_SET_VARS
            if (on_restart_sv.size() > 0) {
                int base = 0;

                // Assign the sol_set values (use lb if no solution is known)
                const FlatZincSpace* last = static_cast<const FlatZincSpace*>(mi.last());
                assert(last == nullptr || last->on_restart_sv.size() == restart_data().on_restart_sv_sol);
                for (int i = 0; i < restart_data().on_restart_sv_sol; ++i) {
                    SetVar& out_var = on_restart_sv[base + restart_data().on_restart_sv_sol + i];
                    Set::GlbRanges<Set::SetView> lb(last != nullptr ? last->on_restart_sv[i] : on_restart_sv[base + i]);
                    IntSet val(lb);
                    dom(*this, out_var, SRT_EQ, val);
                }
                base += restart_data().on_restart_sv_sol * 2;

                // Assign last_val_set values
                for (size_t i = 0; i < restart_data().last_val_set.size(); ++i) {
                    dom(*this, on_restart_sv[base + i], SRT_EQ, restart_data().last_val_set[i]);
                }
                base += restart_data().last_val_set.size();
                assert(base == on_restart_sv.size());

                // Reduce on_restart_sv to only the variables required in a solution
                SetVarArray tmp(*this, restart_data().on_restart_sv_sol);
                for (int i = 0; i < restart_data().on_restart_sv_sol; ++i) {
                    tmp[i] = on_restart_sv[i];
                }
                on_restart_sv = tmp;

                ret = true;
            }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
            if (on_restart_fv.size() > 0) {
                int base = 0;

                // Assign the sol_float values (use lb if no solution is known)
                const FlatZincSpace* last = static_cast<const FlatZincSpace*>(mi.last());
                assert(last == nullptr || last->on_restart_fv.size() == restart_data().on_restart_fv_sol);
                for (int i = 0; i < restart_data().on_restart_fv_sol; ++i) {
                    FloatVar& out_var = on_restart_fv[base + restart_data().on_restart_fv_sol + i];
                    FloatVal sol_val = (last != nullptr) ? last->on_restart_fv[i].val() : on_restart_fv[base + i].min();
                    rel(*this, out_var, FRT_EQ, sol_val);
                }
                base += restart_data().on_restart_fv_sol * 2;

                // Assign last_val_float values
                for (size_t i = 0; i < restart_data().last_val_float.size(); ++i) {
                    rel(*this, on_restart_fv[base + i], FRT_EQ, restart_data().last_val_float[i]);
                }
                base += restart_data().last_val_float.size();

                // Assign uniform_float random values
                for (size_t i = 0; i < restart_data().uniform_range_float.size(); ++i) {
                    const auto& range = restart_data().uniform_range_float[i];
                    /* rndVal will be an element of [range.first, range.second] */
                    const FloatVal rnd_val =
                        (static_cast<FloatVal>(_random(INT_MAX)) / static_cast<FloatVal>(INT_MAX - 1)) *
                            (range.second - range.first) +
                        range.first;
                    rel(*this, on_restart_fv[base + i], FRT_EQ, rnd_val);
                }
                base += restart_data().uniform_range_float.size();
                assert(base == on_restart_fv.size());

                // Reduce on_restart_fv to only the variables required in a solution
                FloatVarArray tmp(*this, restart_data().on_restart_fv_sol);
                for (int i = 0; i < restart_data().on_restart_fv_sol; ++i) {
                    tmp[i] = on_restart_fv[i];
                }
                on_restart_fv = tmp;

                ret = true;
            }
#endif
            if (ret) {
                return false;
            }
        }

        if ((mi.type() == MetaInfo::RESTART) && (mi.restart() != 0) && (_lns > 0) && (mi.last() == nullptr) &&
            (_lns_initial_solution.size() > 0)) {
            for (unsigned int i = iv_lns.size(); i--;) {
                if (_random(99U) <= _lns) {
                    rel(*this, iv_lns[i], IRT_EQ, _lns_initial_solution[i]);
                }
            }
            return false;
        } else if ((mi.type() == MetaInfo::RESTART) && (mi.restart() != 0) && (_lns > 0) && mi.last()) {
            const FlatZincSpace& last = static_cast<const FlatZincSpace&>(*mi.last());
            for (unsigned int i = iv_lns.size(); i--;) {
                if (_random(99U) <= _lns) {
                    rel(*this, iv_lns[i], IRT_EQ, last.iv_lns[i]);
                }
            }
            return false;
        }
        return true;
    }

    Space*
    FlatZincSpace::copy(void) {
        return new FlatZincSpace(*this);
    }

    FlatZincSpace::Meth
    FlatZincSpace::method(void) const {
        return _method;
    }

    int
    FlatZincSpace::opt_var(void) const {
        return _opt_var;
    }

    bool
    FlatZincSpace::opt_var_is_int(void) const {
        return _opt_var_is_int;
    }

    void
    FlatZincSpace::print(std::ostream& out, const Printer& p) const {
        p.print(out, iv, bv
#ifdef GECODE_HAS_SET_VARS
                ,
                sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                ,
                fv
#endif
        );
    }

    void
    FlatZincSpace::compare(const Space& s, std::ostream& out) const {
        (void)s;
        (void)out;
#ifdef GECODE_HAS_GIST
        const FlatZincSpace& fs = dynamic_cast<const FlatZincSpace&>(s);
        for (int i = 0; i < iv.size(); ++i) {
            std::stringstream ss;
            ss << "iv[" << i << "]";
            std::string result(Gecode::Gist::Comparator::compare(ss.str(), iv[i], fs.iv[i]));
            if (result.length() > 0)
                out << result << std::endl;
        }
        for (int i = 0; i < bv.size(); ++i) {
            std::stringstream ss;
            ss << "bv[" << i << "]";
            std::string result(Gecode::Gist::Comparator::compare(ss.str(), bv[i], fs.bv[i]));
            if (result.length() > 0)
                out << result << std::endl;
        }
#ifdef GECODE_HAS_SET_VARS
        for (int i = 0; i < sv.size(); ++i) {
            std::stringstream ss;
            ss << "sv[" << i << "]";
            std::string result(Gecode::Gist::Comparator::compare(ss.str(), sv[i], fs.sv[i]));
            if (result.length() > 0)
                out << result << std::endl;
        }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        for (int i = 0; i < fv.size(); ++i) {
            std::stringstream ss;
            ss << "fv[" << i << "]";
            std::string result(Gecode::Gist::Comparator::compare(ss.str(), fv[i], fs.fv[i]));
            if (result.length() > 0)
                out << result << std::endl;
        }
#endif
#endif
    }

    void
    FlatZincSpace::compare(const FlatZincSpace& s, std::ostream& out, const Printer& p) const {
        p.print_diff(out, iv, s.iv, bv, s.bv
#ifdef GECODE_HAS_SET_VARS
                     ,
                     sv, s.sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                     ,
                     fv, s.fv
#endif
        );
    }

    void
    FlatZincSpace::shrink_arrays(Printer& p) {
        p.shrink_arrays(*this, _opt_var, _opt_var_is_int, iv, bv
#ifdef GECODE_HAS_SET_VARS
                        ,
                        sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                        ,
                        fv
#endif
        );
    }

    IntArgs
    FlatZincSpace::arg_to_int_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        IntArgs ia(a->a.size() + offset);
        for (int i = offset; i--;)
            ia[i] = 0;
        for (int i = a->a.size(); i--;)
            ia[i + offset] = a->a[i]->get_int();
        return ia;
    }
    TupleSet
    FlatZincSpace::arg_to_tuple_set(const IntArgs& a, int no_of_vars) {
        int no_of_tuples = a.size() == 0 ? 0 : (a.size() / no_of_vars);

        // Build TupleSet
        TupleSet ts(no_of_vars);
        for (int i = 0; i < no_of_tuples; i++) {
            IntArgs t(no_of_vars);
            for (int j = 0; j < no_of_vars; j++) {
                t[j] = a[i * no_of_vars + j];
            }
            ts.add(t);
        }
        ts.finalize(EPK_AUTO);

        if (_init_data) {
            FlatZincSpaceInitData::TupleSetSet::iterator it = _init_data->tuple_set_set.find(ts);
            if (it != _init_data->tuple_set_set.end()) {
                return *it;
            }
            _init_data->tuple_set_set.insert(ts);
        }

        return ts;
    }
    IntSharedArray
    FlatZincSpace::arg_to_int_shared_array(AST::Node* arg, int offset) {
        IntArgs ia(arg_to_int_args(arg, offset));
        SharedArray<int> sia(ia);
        if (_init_data) {
            FlatZincSpaceInitData::IntSharedArraySet::iterator it = _init_data->int_shared_array_set.find(sia);
            if (it != _init_data->int_shared_array_set.end()) {
                return *it;
            }
            _init_data->int_shared_array_set.insert(sia);
        }

        return sia;
    }
    IntArgs
    FlatZincSpace::arg_to_bool_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        IntArgs ia(a->a.size() + offset);
        for (int i = offset; i--;)
            ia[i] = 0;
        for (int i = a->a.size(); i--;)
            ia[i + offset] = a->a[i]->get_bool();
        return ia;
    }
    IntSharedArray
    FlatZincSpace::arg_to_bool_shared_array(AST::Node* arg, int offset) {
        IntArgs ia(arg_to_bool_args(arg, offset));
        SharedArray<int> sia(ia);
        if (_init_data) {
            FlatZincSpaceInitData::IntSharedArraySet::iterator it = _init_data->int_shared_array_set.find(sia);
            if (it != _init_data->int_shared_array_set.end()) {
                return *it;
            }
            _init_data->int_shared_array_set.insert(sia);
        }

        return sia;
    }
    IntSet
    FlatZincSpace::arg_to_int_set(AST::Node* n) {
        AST::SetLit* sl = n->get_set();
        IntSet d;
        if (sl->interval) {
            d = IntSet(sl->min, sl->max);
        } else {
            Region re;
            int* is = re.alloc<int>(static_cast<unsigned long int>(sl->s.size()));
            for (int i = sl->s.size(); i--;)
                is[i] = sl->s[i];
            d = IntSet(is, sl->s.size());
        }
        return d;
    }
    IntSetArgs
    FlatZincSpace::arg_to_int_set_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        if (a->a.size() == 0) {
            IntSetArgs empty_ia(0);
            return empty_ia;
        }
        IntSetArgs ia(a->a.size() + offset);
        for (int i = offset; i--;)
            ia[i] = IntSet::empty;
        for (int i = a->a.size(); i--;) {
            ia[i + offset] = arg_to_int_set(a->a[i]);
        }
        return ia;
    }
    IntVarArgs
    FlatZincSpace::arg_to_int_var_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        if (a->a.size() == 0) {
            IntVarArgs empty_ia(0);
            return empty_ia;
        }
        IntVarArgs ia(a->a.size() + offset);
        for (int i = offset; i--;)
            ia[i] = IntVar(*this, 0, 0);
        for (int i = a->a.size(); i--;) {
            if (a->a[i]->is_int_var()) {
                ia[i + offset] = iv[a->a[i]->get_int_var()];
            } else {
                int value = a->a[i]->get_int();
                IntVar iv(*this, value, value);
                ia[i + offset] = iv;
            }
        }
        return ia;
    }
    BoolVarArgs
    FlatZincSpace::arg_to_bool_var_args(AST::Node* arg, int offset, int siv) {
        AST::Array* a = arg->get_array();
        if (a->a.size() == 0) {
            BoolVarArgs empty_ia(0);
            return empty_ia;
        }
        BoolVarArgs ia(a->a.size() + offset - (siv == -1 ? 0 : 1));
        for (int i = offset; i--;)
            ia[i] = BoolVar(*this, 0, 0);
        for (int i = 0; i < static_cast<int>(a->a.size()); i++) {
            if (i == siv)
                continue;
            if (a->a[i]->is_bool()) {
                bool value = a->a[i]->get_bool();
                BoolVar iv(*this, value, value);
                ia[offset++] = iv;
            } else if (a->a[i]->is_int_var() && alias_bool2_int(a->a[i]->get_int_var()) != -1) {
                ia[offset++] = bv[alias_bool2_int(a->a[i]->get_int_var())];
            } else {
                ia[offset++] = bv[a->a[i]->get_bool_var()];
            }
        }
        return ia;
    }
    BoolVar
    FlatZincSpace::arg_to_bool_var(AST::Node* n) {
        BoolVar x0;
        if (n->is_bool()) {
            x0 = BoolVar(*this, n->get_bool(), n->get_bool());
        } else {
            x0 = bv[n->get_bool_var()];
        }
        return x0;
    }
    IntVar
    FlatZincSpace::arg_to_int_var(AST::Node* n) {
        IntVar x0;
        if (n->is_int_var()) {
            x0 = iv[n->get_int_var()];
        } else {
            x0 = IntVar(*this, n->get_int(), n->get_int());
        }
        return x0;
    }
    bool
    FlatZincSpace::is_bool_array(AST::Node* b, int& single_int) {
        AST::Array* a = b->get_array();
        single_int = -1;
        if (a->a.size() == 0)
            return true;
        for (int i = a->a.size(); i--;) {
            if (a->a[i]->is_bool_var() || a->a[i]->is_bool()) {
            } else if (a->a[i]->is_int_var()) {
                if (alias_bool2_int(a->a[i]->get_int_var()) == -1) {
                    if (single_int != -1) {
                        return false;
                    }
                    single_int = i;
                }
            } else {
                return false;
            }
        }
        return single_int == -1 || a->a.size() > 1;
    }
#ifdef GECODE_HAS_SET_VARS
    SetVar
    FlatZincSpace::arg_to_set_var(AST::Node* n) {
        SetVar x0;
        if (!n->is_set_var()) {
            IntSet d = arg_to_int_set(n);
            x0 = SetVar(*this, d, d);
        } else {
            x0 = sv[n->get_set_var()];
        }
        return x0;
    }
    SetVarArgs
    FlatZincSpace::arg_to_set_var_args(AST::Node* arg, int offset, int doffset, const IntSet& od) {
        AST::Array* a = arg->get_array();
        SetVarArgs ia(a->a.size() + offset);
        for (int i = offset; i--;) {
            IntSet d = i < doffset ? od : IntSet::empty;
            ia[i] = SetVar(*this, d, d);
        }
        for (int i = a->a.size(); i--;) {
            ia[i + offset] = arg_to_set_var(a->a[i]);
        }
        return ia;
    }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    FloatValArgs
    FlatZincSpace::arg_to_float_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        FloatValArgs fa(a->a.size() + offset);
        for (int i = offset; i--;)
            fa[i] = 0.0;
        for (int i = a->a.size(); i--;)
            fa[i + offset] = a->a[i]->get_float();
        return fa;
    }
    FloatVarArgs
    FlatZincSpace::arg_to_float_var_args(AST::Node* arg, int offset) {
        AST::Array* a = arg->get_array();
        if (a->a.size() == 0) {
            FloatVarArgs empty_fa(0);
            return empty_fa;
        }
        FloatVarArgs fa(a->a.size() + offset);
        for (int i = offset; i--;)
            fa[i] = FloatVar(*this, 0.0, 0.0);
        for (int i = a->a.size(); i--;) {
            if (a->a[i]->is_float_var()) {
                fa[i + offset] = fv[a->a[i]->get_float_var()];
            } else {
                double value = a->a[i]->get_float();
                FloatVar fv(*this, value, value);
                fa[i + offset] = fv;
            }
        }
        return fa;
    }
    FloatVar
    FlatZincSpace::arg_to_float_var(AST::Node* n) {
        FloatVar x0;
        if (n->is_float_var()) {
            x0 = fv[n->get_float_var()];
        } else {
            x0 = FloatVar(*this, n->get_float(), n->get_float());
        }
        return x0;
    }
#endif
    IntPropLevel
    FlatZincSpace::annotation_to_int_prop_level(AST::Node* ann) {
        int ipl = IPL_DEF;
        if (ann) {
            if (ann->has_atom("val") || ann->has_atom("value_propagation"))
                ipl = IPL_VAL;
            else if (ann->has_atom("domain") || ann->has_atom("domain_propagation"))
                ipl = IPL_DOM;
            else if (ann->has_atom("bounds") || ann->has_atom("bounds_propagation") || ann->has_atom("boundsR") ||
                     ann->has_atom("boundsD") || ann->has_atom("boundsZ"))
                ipl = IPL_BND;
            if (ann->has_atom("gecode_full_propagation"))
                ipl |= IPL_FULL;
            else {
                if (ann->has_atom("gecode_basic_propagation"))
                    ipl |= IPL_BASIC;
                if (ann->has_atom("gecode_advanced_propagation"))
                    ipl |= IPL_ADVANCED;
            }
        }
        return static_cast<IntPropLevel>(ipl);
    }

    DFA
    FlatZincSpace::get_shared_dfa(DFA& a) {
        if (_init_data) {
            FlatZincSpaceInitData::DFASet::iterator it = _init_data->dfa_set.find(a);
            if (it != _init_data->dfa_set.end()) {
                return *it;
            }
            _init_data->dfa_set.insert(a);
        }
        return a;
    }

    void
    Printer::init(AST::Array* output) {
        _output = output;
    }

    void
    Printer::print_elem(std::ostream& out, AST::Node* ai, const Gecode::IntVarArray& iv, const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                        ,
                        const Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                        ,
                        const Gecode::FloatVarArray& fv
#endif
    ) const {
        int k;
        if (ai->is_int(k)) {
            out << k;
        } else if (ai->is_int_var()) {
            out << iv[ai->get_int_var()];
        } else if (ai->is_bool_var()) {
            if (bv[ai->get_bool_var()].min() == 1) {
                out << "true";
            } else if (bv[ai->get_bool_var()].max() == 0) {
                out << "false";
            } else {
                out << "false..true";
            }
#ifdef GECODE_HAS_SET_VARS
        } else if (ai->is_set_var()) {
            if (!sv[ai->get_set_var()].assigned()) {
                out << sv[ai->get_set_var()];
                return;
            }
            SetVarGlbRanges svr(sv[ai->get_set_var()]);
            if (!svr()) {
                out << "{}";
                return;
            }
            int min = svr.min();
            int max = svr.max();
            ++svr;
            if (svr()) {
                SetVarGlbValues svv(sv[ai->get_set_var()]);
                int i = svv.val();
                out << "{" << i;
                ++svv;
                for (; svv(); ++svv)
                    out << ", " << svv.val();
                out << "}";
            } else {
                out << min << ".." << max;
            }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        } else if (ai->is_float_var()) {
            if (fv[ai->get_float_var()].assigned()) {
                FloatVal vv = fv[ai->get_float_var()].val();
                FloatNum v;
                if (vv.singleton())
                    v = vv.min();
                else if (vv < 0.0)
                    v = vv.max();
                else
                    v = vv.min();
                std::ostringstream oss;
                // oss << std::scientific;
                oss << std::setprecision(std::numeric_limits<double>::digits10);
                oss << v;
                if (oss.str().find(".") == std::string::npos)
                    oss << ".0";
                out << oss.str();
            } else {
                out << fv[ai->get_float_var()];
            }
#endif
        } else if (ai->is_bool()) {
            out << (ai->get_bool() ? "true" : "false");
        } else if (ai->is_set()) {
            AST::SetLit* s = ai->get_set();
            if (s->interval) {
                out << s->min << ".." << s->max;
            } else {
                out << "{";
                for (unsigned int i = 0; i < s->s.size(); i++) {
                    out << s->s[i] << (i < s->s.size() - 1 ? ", " : "}");
                }
            }
        } else if (ai->is_string()) {
            std::string s = ai->get_string();
            for (unsigned int i = 0; i < s.size(); i++) {
                if (s[i] == '\\' && i < s.size() - 1) {
                    switch (s[i + 1]) {
                    case 'n':
                        out << "\n";
                        break;
                    case '\\':
                        out << "\\";
                        break;
                    case 't':
                        out << "\t";
                        break;
                    default:
                        out << "\\" << s[i + 1];
                    }
                    i++;
                } else {
                    out << s[i];
                }
            }
        }
    }

    void
    Printer::print_elem_diff(std::ostream& out, AST::Node* ai, const Gecode::IntVarArray& iv1,
                             const Gecode::IntVarArray& iv2, const Gecode::BoolVarArray& bv1,
                             const Gecode::BoolVarArray& bv2
#ifdef GECODE_HAS_SET_VARS
                             ,
                             const Gecode::SetVarArray& sv1, const Gecode::SetVarArray& sv2
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                             ,
                             const Gecode::FloatVarArray& fv1, const Gecode::FloatVarArray& fv2
#endif
    ) const {
#ifdef GECODE_HAS_GIST
        using namespace Gecode::Gist;
        int k;
        if (ai->is_int(k)) {
            out << k;
        } else if (ai->is_int_var()) {
            std::string res(Comparator::compare("", iv1[ai->get_int_var()], iv2[ai->get_int_var()]));
            if (res.length() > 0) {
                res.erase(0, 1); // Remove '='
                out << res;
            } else {
                out << iv1[ai->get_int_var()];
            }
        } else if (ai->is_bool_var()) {
            std::string res(Comparator::compare("", bv1[ai->get_bool_var()], bv2[ai->get_bool_var()]));
            if (res.length() > 0) {
                res.erase(0, 1); // Remove '='
                out << res;
            } else {
                out << bv1[ai->get_bool_var()];
            }
#ifdef GECODE_HAS_SET_VARS
        } else if (ai->is_set_var()) {
            std::string res(Comparator::compare("", sv1[ai->get_set_var()], sv2[ai->get_set_var()]));
            if (res.length() > 0) {
                res.erase(0, 1); // Remove '='
                out << res;
            } else {
                out << sv1[ai->get_set_var()];
            }
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        } else if (ai->is_float_var()) {
            std::string res(Comparator::compare("", fv1[ai->get_float_var()], fv2[ai->get_float_var()]));
            if (res.length() > 0) {
                res.erase(0, 1); // Remove '='
                out << res;
            } else {
                out << fv1[ai->get_float_var()];
            }
#endif
        } else if (ai->is_bool()) {
            out << (ai->get_bool() ? "true" : "false");
        } else if (ai->is_set()) {
            AST::SetLit* s = ai->get_set();
            if (s->interval) {
                out << s->min << ".." << s->max;
            } else {
                out << "{";
                for (unsigned int i = 0; i < s->s.size(); i++) {
                    out << s->s[i] << (i < s->s.size() - 1 ? ", " : "}");
                }
            }
        } else if (ai->is_string()) {
            std::string s = ai->get_string();
            for (unsigned int i = 0; i < s.size(); i++) {
                if (s[i] == '\\' && i < s.size() - 1) {
                    switch (s[i + 1]) {
                    case 'n':
                        out << "\n";
                        break;
                    case '\\':
                        out << "\\";
                        break;
                    case 't':
                        out << "\t";
                        break;
                    default:
                        out << "\\" << s[i + 1];
                    }
                    i++;
                } else {
                    out << s[i];
                }
            }
        }
#else
        (void)out;
        (void)ai;
        (void)iv1;
        (void)iv2;
        (void)bv1;
        (void)bv2;
#ifdef GECODE_HAS_SET_VARS
        (void)sv1;
        (void)sv2;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        (void)fv1;
        (void)fv2;
#endif

#endif
    }

    void
    Printer::print(std::ostream& out, const Gecode::IntVarArray& iv, const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                   ,
                   const Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                   ,
                   const Gecode::FloatVarArray& fv
#endif
    ) const {
        if (_output == nullptr)
            return;
        for (unsigned int i = 0; i < _output->a.size(); i++) {
            AST::Node* ai = _output->a[i];
            if (ai->is_array()) {
                AST::Array* aia = ai->get_array();
                int size = aia->a.size();
                out << "[";
                for (int j = 0; j < size; j++) {
                    print_elem(out, aia->a[j], iv, bv
#ifdef GECODE_HAS_SET_VARS
                               ,
                               sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                               ,
                               fv
#endif
                    );
                    if (j < size - 1)
                        out << ", ";
                }
                out << "]";
            } else {
                print_elem(out, ai, iv, bv
#ifdef GECODE_HAS_SET_VARS
                           ,
                           sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                           ,
                           fv
#endif
                );
            }
        }
    }

    void
    Printer::print_diff(std::ostream& out, const Gecode::IntVarArray& iv1, const Gecode::IntVarArray& iv2,
                        const Gecode::BoolVarArray& bv1, const Gecode::BoolVarArray& bv2
#ifdef GECODE_HAS_SET_VARS
                        ,
                        const Gecode::SetVarArray& sv1, const Gecode::SetVarArray& sv2
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                        ,
                        const Gecode::FloatVarArray& fv1, const Gecode::FloatVarArray& fv2
#endif
    ) const {
        if (_output == nullptr)
            return;
        for (unsigned int i = 0; i < _output->a.size(); i++) {
            AST::Node* ai = _output->a[i];
            if (ai->is_array()) {
                AST::Array* aia = ai->get_array();
                int size = aia->a.size();
                out << "[";
                for (int j = 0; j < size; j++) {
                    print_elem_diff(out, aia->a[j], iv1, iv2, bv1, bv2
#ifdef GECODE_HAS_SET_VARS
                                    ,
                                    sv1, sv2
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                                    ,
                                    fv1, fv2
#endif
                    );
                    if (j < size - 1)
                        out << ", ";
                }
                out << "]";
            } else {
                print_elem_diff(out, ai, iv1, iv2, bv1, bv2
#ifdef GECODE_HAS_SET_VARS
                                ,
                                sv1, sv2
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                                ,
                                fv1, fv2
#endif
                );
            }
        }
    }

    void
    Printer::add_int_var_name(const std::string& n) {
        iv_names.push_back(n);
    }
    void
    Printer::add_bool_var_name(const std::string& n) {
        bv_names.push_back(n);
    }
#ifdef GECODE_HAS_FLOAT_VARS
    void
    Printer::add_float_var_name(const std::string& n) {
        fv_names.push_back(n);
    }
#endif
#ifdef GECODE_HAS_SET_VARS
    void
    Printer::add_set_var_name(const std::string& n) {
        sv_names.push_back(n);
    }
#endif

    void
    Printer::shrink_element(AST::Node* node, std::map<int, int>& iv, std::map<int, int>& bv, std::map<int, int>& sv,
                            std::map<int, int>& fv) {
        if (node->is_int_var()) {
            AST::IntVar* x = static_cast<AST::IntVar*>(node);
            if (iv.find(x->i) == iv.end()) {
                int newi = iv.size();
                iv[x->i] = newi;
            }
            x->i = iv[x->i];
        } else if (node->is_bool_var()) {
            AST::BoolVar* x = static_cast<AST::BoolVar*>(node);
            if (bv.find(x->i) == bv.end()) {
                int newi = bv.size();
                bv[x->i] = newi;
            }
            x->i = bv[x->i];
        } else if (node->is_set_var()) {
            AST::SetVar* x = static_cast<AST::SetVar*>(node);
            if (sv.find(x->i) == sv.end()) {
                int newi = sv.size();
                sv[x->i] = newi;
            }
            x->i = sv[x->i];
        } else if (node->is_float_var()) {
            AST::FloatVar* x = static_cast<AST::FloatVar*>(node);
            if (fv.find(x->i) == fv.end()) {
                int newi = fv.size();
                fv[x->i] = newi;
            }
            x->i = fv[x->i];
        }
    }

    void
    Printer::shrink_arrays(Space& home, int& opt_var, bool opt_var_is_int, Gecode::IntVarArray& iv,
                           Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                           ,
                           Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                           ,
                           Gecode::FloatVarArray& fv
#endif
    ) {
        if (_output == nullptr) {
            if (opt_var_is_int && opt_var != -1) {
                IntVar ov = iv[opt_var];
                iv = IntVarArray(home, 1);
                iv[0] = ov;
                opt_var = 0;
            } else {
                iv = IntVarArray(home, 0);
            }
            bv = BoolVarArray(home, 0);
#ifdef GECODE_HAS_SET_VARS
            sv = SetVarArray(home, 0);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
            if (!opt_var_is_int && opt_var != -1) {
                FloatVar ov = fv[opt_var];
                fv = FloatVarArray(home, 1);
                fv[0] = ov;
                opt_var = 0;
            } else {
                fv = FloatVarArray(home, 0);
            }
#endif
            return;
        }
        std::map<int, int> iv_new;
        std::map<int, int> bv_new;
        std::map<int, int> sv_new;
        std::map<int, int> fv_new;

        if (opt_var != -1) {
            if (opt_var_is_int)
                iv_new[opt_var] = 0;
            else
                fv_new[opt_var] = 0;
            opt_var = 0;
        }

        for (unsigned int i = 0; i < _output->a.size(); i++) {
            AST::Node* ai = _output->a[i];
            if (ai->is_array()) {
                AST::Array* aia = ai->get_array();
                for (unsigned int j = 0; j < aia->a.size(); j++) {
                    shrink_element(aia->a[j], iv_new, bv_new, sv_new, fv_new);
                }
            } else {
                shrink_element(ai, iv_new, bv_new, sv_new, fv_new);
            }
        }

        IntVarArgs iva(iv_new.size());
        std::vector<std::string> iv_names_new(iv_new.size());
        for (std::map<int, int>::iterator i = iv_new.begin(); i != iv_new.end(); ++i) {
            iva[(*i).second] = iv[(*i).first];
            iv_names_new[(*i).second] = iv_names[(*i).first];
        }
        iv = IntVarArray(home, iva);
        iv_names = iv_names_new;

        BoolVarArgs bva(bv_new.size());
        std::vector<std::string> bv_names_new(bv_new.size());
        for (std::map<int, int>::iterator i = bv_new.begin(); i != bv_new.end(); ++i) {
            bva[(*i).second] = bv[(*i).first];
            bv_names_new[(*i).second] = bv_names[(*i).first];
        }
        bv = BoolVarArray(home, bva);
        bv_names = bv_names_new;

#ifdef GECODE_HAS_SET_VARS
        SetVarArgs sva(sv_new.size());
        std::vector<std::string> sv_names_new(sv_new.size());
        for (std::map<int, int>::iterator i = sv_new.begin(); i != sv_new.end(); ++i) {
            sva[(*i).second] = sv[(*i).first];
            sv_names_new[(*i).second] = sv_names[(*i).first];
        }
        sv = SetVarArray(home, sva);
        sv_names = sv_names_new;
#endif

#ifdef GECODE_HAS_FLOAT_VARS
        FloatVarArgs fva(fv_new.size());
        std::vector<std::string> fv_names_new(fv_new.size());
        for (std::map<int, int>::iterator i = fv_new.begin(); i != fv_new.end(); ++i) {
            fva[(*i).second] = fv[(*i).first];
            fv_names_new[(*i).second] = fv_names[(*i).first];
        }
        fv = FloatVarArray(home, fva);
        fv_names = fv_names_new;
#endif
    }

    Printer::~Printer(void) {
        delete _output;
    }

}} // namespace Gecode::FlatZinc

// STATISTICS: flatzinc-any
