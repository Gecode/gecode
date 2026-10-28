/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Gabriel Hjort Blindell <gabriel.hjort.blindell@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007-2012
 *     Gabriel Hjort Blindell, 2012
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

#ifndef GECODE_FLATZINC_HH
#define GECODE_FLATZINC_HH

#include <iostream>

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif
#include <map>

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_FLATZINC
#define GECODE_FLATZINC_EXPORT __declspec(dllexport)
#else
#define GECODE_FLATZINC_EXPORT __declspec(dllimport)
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define GECODE_FLATZINC_EXPORT __attribute__((visibility("default")))

#else

#define GECODE_FLATZINC_EXPORT

#endif
#endif

// Configure auto-linking
#ifndef GECODE_BUILD_FLATZINC
#define GECODE_LIBRARY_NAME "FlatZinc"
#include <gecode/support/auto-link.hpp>
#endif

#include <gecode/driver.hh>

#include <gecode/flatzinc/conexpr.hh>
#include <gecode/flatzinc/ast.hh>
#include <gecode/flatzinc/varspec.hh>

/**
 * \namespace Gecode::FlatZinc
 * \brief Interpreter for the %FlatZinc language
 *
 * The Gecode::FlatZinc namespace contains all functionality required
 * to parse and solve constraint models written in the %FlatZinc language.
 *
 */

namespace Gecode { namespace FlatZinc {

    /**
     * \brief Output support class for %FlatZinc interpreter
     *
     */
    class GECODE_FLATZINC_EXPORT Printer {
    private:
        /// Names of integer variables
        std::vector<std::string> iv_names;
        /// Names of Boolean variables
        std::vector<std::string> bv_names;
#ifdef GECODE_HAS_FLOAT_VARS
        /// Names of float variables
        std::vector<std::string> fv_names;
#endif
#ifdef GECODE_HAS_SET_VARS
        /// Names of set variables
        std::vector<std::string> sv_names;
#endif
        AST::Array* _output;
        void print_elem(std::ostream& out, AST::Node* ai, const Gecode::IntVarArray& iv, const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                        ,
                        const Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                        ,
                        const Gecode::FloatVarArray& fv
#endif
        ) const;
        void print_elem_diff(std::ostream& out, AST::Node* ai, const Gecode::IntVarArray& iv1,
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
        ) const;
    public:
        Printer(void) : _output(nullptr) {}
        void init(AST::Array* output);

        void print(std::ostream& out, const Gecode::IntVarArray& iv, const Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                   ,
                   const Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                   ,
                   const Gecode::FloatVarArray& fv
#endif
        ) const;

        void print_diff(std::ostream& out, const Gecode::IntVarArray& iv1, const Gecode::IntVarArray& iv2,
                        const Gecode::BoolVarArray& bv1, const Gecode::BoolVarArray& bv2
#ifdef GECODE_HAS_SET_VARS
                        ,
                        const Gecode::SetVarArray& sv1, const Gecode::SetVarArray& sv2
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                        ,
                        const Gecode::FloatVarArray& fv1, const Gecode::FloatVarArray& fv2
#endif
        ) const;

        ~Printer(void);

        void add_int_var_name(const std::string& n);
        const std::string&
        int_var_name(int i) const {
            return iv_names[i];
        }
        void add_bool_var_name(const std::string& n);
        const std::string&
        bool_var_name(int i) const {
            return bv_names[i];
        }
#ifdef GECODE_HAS_FLOAT_VARS
        void add_float_var_name(const std::string& n);
        const std::string&
        float_var_name(int i) const {
            return fv_names[i];
        }
#endif
#ifdef GECODE_HAS_SET_VARS
        void add_set_var_name(const std::string& n);
        const std::string&
        set_var_name(int i) const {
            return sv_names[i];
        }
#endif

        void shrink_element(AST::Node* node, std::map<int, int>& iv, std::map<int, int>& bv, std::map<int, int>& sv,
                            std::map<int, int>& fv);

        void shrink_arrays(Space& home, int& opt_var, bool opt_var_is_int, Gecode::IntVarArray& iv,
                           Gecode::BoolVarArray& bv
#ifdef GECODE_HAS_SET_VARS
                           ,
                           Gecode::SetVarArray& sv
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                           ,
                           Gecode::FloatVarArray& fv
#endif
        );

    private:
        Printer(const Printer&);
        Printer& operator=(const Printer&);
    };

    /**
     * \brief %Options for running %FlatZinc models
     *
     */
    class FlatZincOptions : public Gecode::BaseOptions {
    protected:
        /// \name Search options
        //@{
        Gecode::Driver::IntOption _solutions;            ///< How many solutions
        Gecode::Driver::BoolOption _all_solutions;       ///< Return all solutions
        Gecode::Driver::DoubleOption _threads;           ///< How many threads to use
        Gecode::Driver::BoolOption _free;                ///< Use free search
        Gecode::Driver::DoubleOption _decay;             ///< Decay option
        Gecode::Driver::UnsignedIntOption _c_d;          ///< Copy recomputation distance
        Gecode::Driver::UnsignedIntOption _a_d;          ///< Adaptive recomputation distance
        Gecode::Driver::UnsignedLongLongIntOption _node; ///< Cutoff for number of nodes
        Gecode::Driver::UnsignedLongLongIntOption _fail; ///< Cutoff for number of failures
        Gecode::Driver::DoubleOption _time;              ///< Cutoff for time
        Gecode::Driver::DoubleOption _time_limit;   ///< Cutoff for time (for compatibility with flatzinc command line)
        Gecode::Driver::IntOption _seed;            ///< Random seed
        Gecode::Driver::StringOption _restart;      ///< Restart method option
        Gecode::Driver::DoubleOption _r_base;       ///< Restart base
        Gecode::Driver::UnsignedIntOption _r_scale; ///< Restart scale factor
        Gecode::Driver::UnsignedLongLongIntOption _r_limit; ///< Cutoff for number of restarts
        Gecode::Driver::BoolOption _nogoods;                ///< Whether to use no-goods
        Gecode::Driver::UnsignedIntOption _nogoods_limit;   ///< Depth limit for extracting no-goods
        Gecode::Driver::BoolOption _interrupt;              ///< Whether to catch SIGINT
        Gecode::Driver::DoubleOption _step;                 ///< Step option
        //@}

        /// \name Execution options
        //@{
        Gecode::Driver::StringOption _mode;        ///< Script mode to run
        Gecode::Driver::BoolOption _stat;          ///< Emit statistics
        Gecode::Driver::StringValueOption _output; ///< Output file

#ifdef GECODE_HAS_CPPROFILER
        Gecode::Driver::ProfilerOption _profiler; ///< Use this execution id for the CP-profiler
#endif

        //@}
    public:
        /// Constructor
        FlatZincOptions(const char* s)
            : Gecode::BaseOptions(s), _solutions("n", "number of solutions (0 = all, -1 = one/best)", -1),
              _all_solutions("a", "return all solutions (equal to -n 0)"),
              _threads("p", "number of threads (0 = #processing units)", Gecode::Search::Config::threads),
              _free("f", "free search, no need to follow search-specification"), _decay("decay", "decay factor", 0.99),
              _c_d("c-d", "recomputation commit distance", Gecode::Search::Config::c_d),
              _a_d("a-d", "recomputation adaption distance", Gecode::Search::Config::a_d),
              _node("node", "node cutoff (0 = none, solution mode)"),
              _fail("fail", "failure cutoff (0 = none, solution mode)"),
              _time("time", "time (in ms) cutoff (0 = none, solution mode)"),
              _time_limit("t", "time (in ms) cutoff (0 = none, solution mode)"), _seed("r", "random seed", 0),
              _restart("restart", "restart sequence type", RM_NONE),
              _r_base("restart-base", "base for geometric restart sequence", 1.5),
              _r_scale("restart-scale", "scale factor for restart sequence", 250),
              _r_limit("restart-limit", "restart cutoff (0 = none, solution mode)"),
              _nogoods("nogoods", "whether to use no-goods from restarts", false),
              _nogoods_limit("nogoods-limit", "depth limit for no-good extraction", Search::Config::nogoods_limit),
              _interrupt("interrupt", "whether to catch Ctrl-C (true) or not (false)", true),
              _step("step", "step distance for float optimization", 0.0),
              _mode("mode", "how to execute script", Gecode::SM_SOLUTION), _stat("s", "emit statistics"),
              _output("o", "file to send output to")

#ifdef GECODE_HAS_CPPROFILER
              ,
              _profiler("cp-profiler", "use this execution id and port (comma separated) with CP-profiler")
#endif
        {
            _mode.add(Gecode::SM_SOLUTION, "solution");
            _mode.add(Gecode::SM_STAT, "stat");
            _mode.add(Gecode::SM_GIST, "gist");
            _restart.add(RM_NONE, "none");
            _restart.add(RM_CONSTANT, "constant");
            _restart.add(RM_LINEAR, "linear");
            _restart.add(RM_LUBY, "luby");
            _restart.add(RM_GEOMETRIC, "geometric");

            add(_solutions);
            add(_threads);
            add(_c_d);
            add(_a_d);
            add(_all_solutions);
            add(_free);
            add(_decay);
            add(_node);
            add(_fail);
            add(_time);
            add(_time_limit);
            add(_interrupt);
            add(_seed);
            add(_step);
            add(_restart);
            add(_r_base);
            add(_r_scale);
            add(_r_limit);
            add(_nogoods);
            add(_nogoods_limit);
            add(_mode);
            add(_stat);
            add(_output);
#ifdef GECODE_HAS_CPPROFILER
            add(_profiler);
#endif
        }

        void
        parse(int& argc, char* argv[]) {
            Gecode::BaseOptions::parse(argc, argv);
            if (_all_solutions.value() && _solutions.value() == -1) {
                _solutions.value(0);
            }
            if (_time_limit.value()) {
                _time.value(_time_limit.value());
            }
            if (_stat.value() && (_mode.value() == Gecode::SM_SOLUTION))
                _mode.value(Gecode::SM_STAT);
        }

        void
        help(void) override {
            std::cerr << "Gecode FlatZinc interpreter" << std::endl
                      << " - FlatZinc text interface" << std::endl
                      << std::endl;
            Gecode::BaseOptions::help();
        }

        int
        solutions(void) const {
            return _solutions.value();
        }
        bool
        all_solutions(void) const {
            return _all_solutions.value();
        }
        double
        threads(void) const {
            return _threads.value();
        }
        bool
        free(void) const {
            return _free.value();
        }
        unsigned int
        c_d(void) const {
            return _c_d.value();
        }
        unsigned int
        a_d(void) const {
            return _a_d.value();
        }
        unsigned long long int
        node(void) const {
            return _node.value();
        }
        unsigned long long int
        fail(void) const {
            return _fail.value();
        }
        double
        time(void) const {
            return _time.value();
        }
        int
        seed(void) const {
            return _seed.value();
        }
        double
        step(void) const {
            return _step.value();
        }
        const char*
        output(void) const {
            return _output.value();
        }

        Gecode::ScriptMode
        mode(void) const {
            return static_cast<Gecode::ScriptMode>(_mode.value());
        }

        double
        decay(void) const {
            return _decay.value();
        }
        RestartMode
        restart(void) const {
            return static_cast<RestartMode>(_restart.value());
        }
        void
        restart(RestartMode rm) {
            _restart.value(rm);
        }
        double
        restart_base(void) const {
            return _r_base.value();
        }
        void
        restart_base(double d) {
            _r_base.value(d);
        }
        unsigned int
        restart_scale(void) const {
            return _r_scale.value();
        }
        void
        restart_scale(int i) {
            _r_scale.value(i);
        }
        unsigned long long int
        restart_limit(void) const {
            return _r_limit.value();
        }
        bool
        nogoods(void) const {
            return _nogoods.value();
        }
        unsigned int
        nogoods_limit(void) const {
            return _nogoods_limit.value();
        }
        bool
        interrupt(void) const {
            return _interrupt.value();
        }

#ifdef GECODE_HAS_CPPROFILER

        int
        profiler_id(void) const {
            return _profiler.execution_id();
        }
        unsigned int
        profiler_port(void) const {
            return _profiler.port();
        }
        bool
        profiler_info(void) const {
            return true;
        }

#endif

        void
        all_solutions(bool b) {
            _all_solutions.value(b);
        }
    };

    class BranchInformation : public SharedHandle {
    public:
        /// Constructor
        BranchInformation(void);
        /// Copy constructor
        BranchInformation(const BranchInformation& bi);
        /// Initialise for use
        void init(void);
        /// Add new brancher information
        void add(BrancherGroup bg, const std::string& rel0, const std::string& rel1, const std::vector<std::string>& n);
        /// Output branch information
        void print(const Brancher& b, unsigned int a, int i, int n, std::ostream& o) const;
#ifdef GECODE_HAS_FLOAT_VARS
        /// Output branch information
        void print(const Brancher& b, unsigned int a, int i, const FloatNumBranch& nl, std::ostream& o) const;
#endif
        /// Assignment operator
        BranchInformation& operator=(const BranchInformation&) = default;
    };

    /// Uninitialized default random number generator
    GECODE_FLATZINC_EXPORT
    extern Rnd defrnd;

    class FlatZincSpaceInitData;
    class BlackBoxAccess;

    /**
     * \brief A space that can be initialized with a %FlatZinc model
     *
     */
    class GECODE_FLATZINC_EXPORT FlatZincSpace : public Space {
    public:
        enum Meth {
            SAT, //< Solve as satisfaction problem
            MIN, //< Solve as minimization problem
            MAX  //< Solve as maximization problem
        };

    protected:
        /// Initialisation data (only used for posting constraints)
        FlatZincSpaceInitData* _init_data;
        /// Number of integer variables
        int int_var_count;
        /// Number of Boolean variables
        int bool_var_count;
        /// Number of float variables
        int float_var_count;
        /// Number of set variables
        int set_var_count;

        /// Index of the variable to optimize
        int _opt_var;
        /// Whether variable to optimize is integer (or float)
        bool _opt_var_is_int;

        /// Whether to solve as satisfaction or optimization problem
        Meth _method;

        /// Percentage of variables to keep in LNS (or 0 for no LNS)
        unsigned int _lns;

        /// Initial solution to start the LNS (or nullptr for no LNS)
        IntSharedArray _lns_initial_solution;

        /// Random number generator
        Rnd _random;

        /// Annotations on the solve item
        AST::Array* _solve_annotations;

        /// Copy constructor
        FlatZincSpace(FlatZincSpace&);

    private:
        friend class BlackBoxAccess;
        /// Run the search engine
        template <template <class> class Engine>
        void run_engine(std::ostream& out, const Printer& p, const FlatZincOptions& opt,
                        Gecode::Support::Timer& t_total);
        /// Run the meta search engine
        template <template <class> class Engine, template <class, template <class> class> class Meta>
        void run_meta(std::ostream& out, const Printer& p, const FlatZincOptions& opt, Gecode::Support::Timer& t_total);
        void branch_with_plugin(AST::Node* ann);

    public:
        /// The integer variables
        Gecode::IntVarArray iv;
        /// The introduced integer variables
        Gecode::IntVarArray iv_aux;

        /// The integer variables used in LNS
        Gecode::IntVarArray iv_lns;

        /* === Experimental `on_restart` support === */
        class OnRestartHandle : public SharedHandle {
        protected:
            class OnRestartData : public SharedHandle::Object {
            public:
                /// Marker set to `true` if solve process can be marked as complete
                bool mark_complete = false;
                /// Inclusive ranges to assign random values
                std::vector<std::pair<int, int>> uniform_range_int;
#ifdef GECODE_HAS_FLOAT_VARS
                std::vector<std::pair<FloatVal, FloatVal>> uniform_range_float;
#endif
                /// Last assigned values for different types of decisions
                std::vector<bool> last_val_bool;
                std::vector<int> last_val_int;
#ifdef GECODE_HAS_SET_VARS
                std::vector<IntSet> last_val_set;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                std::vector<FloatVal> last_val_float;
#endif
                /// Number of `sol` calls for which variables are stored in
                /// on_restart_iv. In the array you first find this number of variable
                /// for which the solution is read, then you find the same number of
                /// variables which value is set.
                int on_restart_iv_sol = 0;
                int on_restart_bv_sol = 0;
                int on_restart_sv_sol = 0;
                int on_restart_fv_sol = 0;
                /// Whether the last on_restart_iv should be set to the restart status
                bool on_restart_status = false;
            };

        public:
            OnRestartHandle() : SharedHandle() {}
            OnRestartHandle(const OnRestartHandle& handle) : SharedHandle(handle) {}
            OnRestartHandle&
            operator=(const OnRestartHandle& handle) {
                return static_cast<OnRestartHandle&>(SharedHandle::operator=(handle));
            }
            virtual ~OnRestartHandle() {};

            void
            init() {
                if (object() == nullptr) {
                    object(new OnRestartData());
                }
            }
            bool
            initialized() const {
                return object() != nullptr;
            }
            OnRestartData&
            operator()() {
                return *static_cast<OnRestartData*>(object());
            };
        };
        OnRestartHandle restart_data;
        /// On Restart Tracked Integer Variables
        Gecode::IntVarArray on_restart_iv;
        /// On Restart Tracked Boolean Variables
        Gecode::BoolVarArray on_restart_bv;
#ifdef GECODE_HAS_SET_VARS
        /// On Restart Tracked Set Variables
        Gecode::SetVarArray on_restart_sv;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        /// On Restart Tracked Floating Point Variables
        Gecode::FloatVarArray on_restart_fv;
#endif
        /* === End `on_restart` === */

        /// Indicates whether an integer variable is introduced by mzn2fzn
        std::vector<bool> iv_introduced;
        /// Indicates whether an integer variable aliases a Boolean variable
        int* iv_boolalias;
        /// The Boolean variables
        Gecode::BoolVarArray bv;
        /// The introduced Boolean variables
        Gecode::BoolVarArray bv_aux;
        /// Indicates whether a Boolean variable is introduced by mzn2fzn
        std::vector<bool> bv_introduced;
#ifdef GECODE_HAS_SET_VARS
        /// The set variables
        Gecode::SetVarArray sv;
        /// The introduced set variables
        Gecode::SetVarArray sv_aux;
        /// Indicates whether a set variable is introduced by mzn2fzn
        std::vector<bool> sv_introduced;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        /// The float variables
        Gecode::FloatVarArray fv;
        /// The introduced float variables
        Gecode::FloatVarArray fv_aux;
        /// Indicates whether a float variable is introduced by mzn2fzn
        std::vector<bool> fv_introduced;
        /// Step by which a next solution has to have lower cost
        Gecode::FloatNum step;
#endif
        /// Whether the introduced variables still need to be copied
        bool need_aux_vars;
        /// Construct empty space
        FlatZincSpace(Rnd& random = defrnd);

        /// Destructor
        ~FlatZincSpace(void) override;

        /// Initialize space with given number of variables
        void init(int int_vars, int bool_vars, int set_vars, int float_vars);

        /// Create new integer variable from specification
        void new_int_var(IntVarSpec* vs);
        /// Link integer variable \a iv to Boolean variable \a bv
        void alias_bool2_int(int iv, int bv);
        /// Return linked Boolean variable for integer variable \a iv
        int alias_bool2_int(int iv);
        /// Create new Boolean variable from specification
        void new_bool_var(BoolVarSpec* vs);
        /// Create new set variable from specification
        void new_set_var(SetVarSpec* vs);
        /// Create new float variable from specification
        void new_float_var(FloatVarSpec* vs);

        /// Post a constraint specified by \a ce
        void post_constraints(std::vector<ConExpr*>& ces);

        /// Post the solve item
        void solve(AST::Array* annotation);
        /// Post that integer variable \a var should be minimized
        void minimize(int var, bool is_int, AST::Array* annotation);
        /// Post that integer variable \a var should be maximized
        void maximize(int var, bool is_int, AST::Array* annotation);

        /// Run the search
        void run(std::ostream& out, const Printer& p, const FlatZincOptions& opt, Gecode::Support::Timer& t_total);

        /// Produce output on \a out using \a p
        void print(std::ostream& out, const Printer& p) const;
#ifdef GECODE_HAS_CPPROFILER
        /// Get string representing the domains of variables (for cpprofiler)
        std::string get_domains(const Printer& p) const;
#endif
        /// Compare this space with space \a s and print the differences on
        /// \a out
        void compare(const Space& s, std::ostream& out) const;
        /// Compare this space with space \a s and print the differences on
        /// \a out using \a p
        void compare(const FlatZincSpace& s, std::ostream& out, const Printer& p) const;

        /**
         * \brief Remove all variables not needed for output
         *
         * After calling this function, no new constraints can be posted through
         * FlatZinc variable references, and the create_branchers method must
         * not be called again.
         *
         */
        void shrink_arrays(Printer& p);

        /// Return whether to solve a satisfaction or optimization problem
        Meth method(void) const;

        /// Return index of variable used for optimization
        int opt_var(void) const;
        /// Return whether variable used for optimization is integer (or float)
        bool opt_var_is_int(void) const;

        /**
         * \brief Create branchers corresponding to the solve item annotations
         *
         * If \a ignoreUnknown is true, unknown solve item annotations will be
         * ignored, otherwise a warning is written to \a err.
         *
         * The seed for random branchers is given by the \a seed parameter.
         *
         */
        void create_branchers(Printer& p, AST::Node* ann, FlatZincOptions& opt, bool ignore_unknown,
                              std::ostream& err = std::cerr);

        /// Return the solve item annotations
        AST::Array* solve_annotations(void) const;

        /// Information for printing branches
        BranchInformation branch_info;

        /// Implement optimization
        void constrain(const Space& s) override;
        /// Copy function
        Gecode::Space* copy(void) override;
        /// Slave function for restarts
        bool slave(const MetaInfo& mi) override;

        /// \name AST to variable and value conversion
        //@{
        /// Convert \a arg (array of integers) to IntArgs
        IntArgs arg_to_int_args(AST::Node* arg, int offset = 0);
        /// Convert \a arg (array of integers) to IntSharedArray
        IntSharedArray arg_to_int_shared_array(AST::Node* arg, int offset = 0);
        /// Convert \a arg (array of Booleans) to IntArgs
        IntArgs arg_to_bool_args(AST::Node* arg, int offset = 0);
        /// Convert \a arg (array of integers) to IntSharedArray
        IntSharedArray arg_to_bool_shared_array(AST::Node* arg, int offset = 0);
        /// Convert \a n to IntSet
        IntSet arg_to_int_set(AST::Node* n);
        /// Convert \a arg to IntSetArgs
        IntSetArgs arg_to_int_set_args(AST::Node* arg, int offset = 0);
        /// Convert \a arg to IntVarArgs
        IntVarArgs arg_to_int_var_args(AST::Node* arg, int offset = 0);
        /// Convert \a arg to BoolVarArgs
        BoolVarArgs arg_to_bool_var_args(AST::Node* arg, int offset = 0, int siv = -1);
        /// Convert \a n to BoolVar
        BoolVar arg_to_bool_var(AST::Node* n);
        /// Convert \a n to IntVar
        IntVar arg_to_int_var(AST::Node* n);
        /// Convert \a a to TupleSet
        TupleSet arg_to_tuple_set(const IntArgs& a, int no_of_vars);
        /// Check if \a b is array of Booleans (or has a single integer)
        bool is_bool_array(AST::Node* b, int& single_int);
#ifdef GECODE_HAS_SET_VARS
        /// Convert \a n to SetVar
        SetVar arg_to_set_var(AST::Node* n);
        /// Convert \a n to SetVarArgs
        SetVarArgs arg_to_set_var_args(AST::Node* arg, int offset = 0, int doffset = 0,
                                       const IntSet& od = IntSet::empty);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        /// Convert \a n to FloatValArgs
        FloatValArgs arg_to_float_args(AST::Node* arg, int offset = 0);
        /// Convert \a n to FloatVar
        FloatVar arg_to_float_var(AST::Node* n);
        /// Convert \a n to FloatVarArgs
        FloatVarArgs arg_to_float_var_args(AST::Node* arg, int offset = 0);
#endif
        /// Convert \a ann to integer propagation level
        IntPropLevel annotation_to_int_prop_level(AST::Node* ann);
        /// Share DFA \a a if possible
        DFA get_shared_dfa(DFA& a);
        //@}
    };

    /// %Exception class for %FlatZinc errors
    class GECODE_VTABLE_EXPORT Error {
    private:
        const std::string msg;

    public:
        Error(const std::string& where, const std::string& what) : msg(where + ": " + what) {}
        Error(const std::string& where, const std::string& what, AST::Array* const ann)
            : msg(make_message(where, what, ann)) {}
        const std::string&
        to_string(void) const {
            return msg;
        }

    private:
        static std::string
        make_message(const std::string& where, const std::string& what, AST::Array* const ann) {
            std::ostringstream result;
            result << where << ": " << what;

            std::vector<std::string> names = get_constraint_names(ann);
            if (names.size() > 1) {
                result << " in constraints ";
                for (unsigned int i = 0; i < names.size(); ++i) {
                    result << '\"' << names[i] << '\"';
                    if (i < names.size() - 1) {
                        result << ",";
                    }
                    result << " ";
                }
            } else if (names.size() == 1) {
                result << " in constraint " << '\"' << names[0] << '\"';
            }

            return result.str();
        }
        static std::vector<std::string>
        get_constraint_names(AST::Array* const ann) {
            std::vector<std::string> result;
            if (ann) {
                for (const auto& i : ann->a) {
                    if (i->is_array()) {
                        auto nested_result = get_constraint_names(i->get_array());
                        result.insert(result.end(), nested_result.begin(), nested_result.end());
                    } else if (i->is_call("mzn_constraint_name")) {
                        result.emplace_back(i->get_call()->args->get_string());
                    }
                }
            }
            return result;
        }
    };

    /**
     * \brief Parse FlatZinc file \a fileName into \a fzs and return it.
     *
     * Creates a new empty FlatZincSpace if \a fzs is nullptr.
     * The caller owns the returned space. On failure or exception, only a space
     * created by the parser is deleted; a supplied space remains caller-owned.
     * A supplied space may then be partially initialized and should be
     * deleted rather than reused.
     */
    GECODE_FLATZINC_EXPORT
    FlatZincSpace* parse(const std::string& file_name, Printer& p, std::ostream& err = std::cerr,
                         FlatZincSpace* fzs = nullptr, Rnd& rnd = defrnd);

    /**
     * \brief Parse FlatZinc from \a is into \a fzs and return it.
     *
     * Creates a new empty FlatZincSpace if \a fzs is nullptr.
     * The caller owns the returned space. On failure or exception, only a space
     * created by the parser is deleted; a supplied space remains caller-owned.
     * A supplied space may then be partially initialized and should be
     * deleted rather than reused.
     */
    GECODE_FLATZINC_EXPORT
    FlatZincSpace* parse(std::istream& is, Printer& p, std::ostream& err = std::cerr, FlatZincSpace* fzs = nullptr,
                         Rnd& rnd = defrnd);

}} // namespace Gecode::FlatZinc

#endif

// STATISTICS: flatzinc-any
