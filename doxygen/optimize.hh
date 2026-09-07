/* Documentation for the optional optimization component. */

/**
 * \defgroup TaskOptimize Modeling and solving with Optimize
 * \ingroup TaskModel
 *
 * The optional C++17 namespace Gecode::Optimize offers an owning sparse model,
 * numerical LP/MILP solving and an explicit bounded integer bridge to native
 * %Gecode. Start with \ref PageOptimize. Existing Space-based models, propagators,
 * search engines and the original FlatZinc driver retain their own interfaces.
 */

/**
 * \namespace Gecode::Optimize
 * \brief Optional sparse optimization models, backends and owning results.
 * \ingroup TaskOptimize
 *
 * See \ref PageOptimize for backend scope and \ref PageOptimizeModel for
 * model ownership, result guarantees and validation.
 */

/**
 * \page PageOptimize Optimize: sparse optimization and native integer search
 *
 * The optimization API is included with
 * \code{.cpp}
 * #include <gecode/optimize.hh>
 * \endcode
 * It is enabled separately from the existing %Gecode libraries. A model owns
 * variables, sparse rows, objective data and supported logical/global metadata;
 * a solve returns a historical result rather than a mutable search Space.
 *
 * - \subpage PageOptimizeModel
 * - \subpage PageOptimizeNative
 * - \subpage PageOptimizeLP
 * - \subpage PageOptimizeWorkflows
 * - \subpage PageOptimizeBuild
 *
 * \section OptimizeRoutes Choose the interface and backend
 *
 * | Interface | Supported models | Restrictions |
 * | --- | --- | --- |
 * | Existing Space, IntVar, BoolVar, SetVar, FloatVar and native search APIs | The original CP modeling, global constraints, propagators, search customization and configured parallel search | Optimize restrictions below do not remove these existing capabilities. See \ref TaskModel and \ref TaskModelSearch. |
 * | Optimize with Backend::Highs | Numerical continuous LP and mixed-integer linear models, including supported binary and semi domains | One worker per solve; numerical tolerances and adapter scaling limits apply. Exact and Certified requests are rejected. |
 * | Optimize with Backend::Native | Finite Integer, Binary and SemiInteger models with exact integral linear data, retained indicators and six typed global families | Conservative native coefficient/activity limits; one deterministic worker; no arbitrary continuous or fractional model conversion. |
 * | Explicit native LP/frontier/neighborhood APIs | Checked integer LP deductions, optional original-row root covers, frontier bounds, binary reliability probes and one bounded incumbent neighborhood | Explicit entry points allow direct control; the common Native policy selects suitable LP, covers and reliability. An optional sequential race compares automatic and ordinary search. |
 * | QuadraticModel and solve_quadratic | Bounded continuous convex minimization or concave maximization expressed as weighted squares plus linear terms | Numerical checking; separate model type. No integer QP, quadratic constraints or general nonconvex optimization. |
 *
 * Gecode::Optimize::solve selects native %Gecode for active typed globals under
 * Backend::Auto, and HiGHS otherwise. Auto does not select a backend from the
 * requested guarantee: an ordinary linear model requesting Exact must select
 * Native explicitly. Missing or unsupported backends return Unsupported;
 * explicit requests are not silently substituted. Within Native, the common
 * dispatcher uses solve_native_auto to preserve eligible knapsack DP and select
 * optional checked LP/branching from bounded model structure. Direct
 * solve_native retains ordinary BAB. The automatic route additionally applies
 * bounded exact presolve, independent-component solving and ordering of exactly
 * interchangeable columns when compatible. The opt-in solve_native_race API
 * compares automatic and ordinary routes with sequential probes, then restarts
 * the selected route under the same time/node budget. Exploration can increase
 * CPU work and solve time, but can reveal a better strategy for a longer solve.
 * Exploration time is configurable. The experimental MiniZinc registration
 * exposes automatic, racing and explicitly configured native strategies.
 * No conflict learning is added. Inspect
 * Gecode::Optimize::capabilities, Gecode::Optimize::native_capabilities,
 * Gecode::Optimize::native_lp_capabilities and
 * Gecode::Optimize::quadratic_capabilities for the current build.
 *
 * \section OptimizeExample A small exact integer model
 *
 * \code{.cpp}
 * #include <gecode/optimize.hh>
 * #include <limits>
 * namespace O = Gecode::Optimize;
 *
 * O::Model model;
 * const auto x = model.add_integer(0, 4, "x");
 * const auto y = model.add_integer(0, 4, "y");
 * model.add_row({{x, 1}, {y, 1}}, 3,
 *               std::numeric_limits<double>::infinity(), "demand");
 * model.minimize({{x, 2}, {y, 1}}, 7);
 * O::SolveOptions options;
 * options.backend = O::Backend::Native;
 * options.guarantee = O::Guarantee::Exact;
 * options.time_limit_seconds = 10;
 * const auto result = O::solve(model, options);
 * if (result.has_solution()) {
 *   const double chosen_y = result.value(y);
 *   (void)chosen_y;
 * }
 * // A completed optimum is x=0, y=3, objective=10.
 * // Check result.termination before claiming optimality.
 * \endcode
 *
 * For a numerical LP, use Continuous variables and Backend::Highs with
 * Guarantee::Numerical. Consult \ref PageOptimizeBuild for the required build.
 */

/**
 * \page PageOptimizeModel Model ownership, logical constraints and results
 *
 * \section OptimizeOwnership Models and edits
 *
 * Gecode::Optimize::Model stores sparse ranged rows and min/max objectives with
 * an offset. VariableType distinguishes Continuous, Integer, Binary,
 * SemiContinuous and SemiInteger domains. Duplicate terms are coalesced
 * deterministically; invalid/nonfinite coefficients are rejected. The
 * VariableSpec, RowSpec and SparseRowBatch overloads provide atomic bulk
 * additions, including compressed sparse row input.
 *
 * Variable and constraint handles carry an owner identity and never-reused
 * slot. Removal leaves tombstones; foreign and deleted handles are rejected.
 * Successful edits advance the revision. A Model is movable, not copyable;
 * ModelSnapshot owns a historical copy. Public snapshot fields remain
 * untrusted: structural and original-semantic checks run at API boundaries.
 * Historical SolveResult, observations and analysis artifacts survive edits
 * and destruction of their originating model/session.
 *
 * \section OptimizeLogic Logical and global constraints
 *
 * Gecode::Optimize::add_indicator retains the original implication and derives
 * finite conservative M values from declared domains for numerical solving.
 * It rejects insufficient bounds or overflow instead of guessing M. Generated
 * rows/gates and their domain guards remain associated with the original
 * metadata. Mutations that invalidate the lowering are rejected; use
 * Gecode::Optimize::remove_indicator for its supported removal lifecycle.
 * Native search uses the original reified implication and preserves exposed
 * gate semantics. The independent checker tests the original implication too.
 *
 * Gecode::Optimize::add_boolean_and and Gecode::Optimize::add_boolean_or
 * require Binary inputs/results. Empty AND is true; empty OR is false.
 *
 * | Typed helper | Original meaning |
 * | --- | --- |
 * | Gecode::Optimize::add_all_different | Pairwise different integer values. |
 * | Gecode::Optimize::add_element | Variable index/result with explicit index base and retained array aliases. |
 * | Gecode::Optimize::add_table | Membership in a positive table of integer tuples. |
 * | Gecode::Optimize::add_cumulative | Mandatory fixed-duration, fixed-height tasks with half-open intervals; zero duration/height consumes no resource. |
 * | Gecode::Optimize::add_circuit | One cycle through a nonempty successor array with explicit index base. |
 * | Gecode::Optimize::add_regular | Deterministic finite automaton, sparse state IDs, unique state/symbol transitions, no epsilon transitions. A missing transition rejects; the empty word accepts exactly when the initial state is final. |
 *
 * These six families are the Optimize registry, not the full original %Gecode
 * global catalog. Active globals require native compilation, which performs
 * additional domain, indexing, storage and arithmetic admission. HiGHS and
 * model exporters reject unsupported original metadata instead of discarding it.
 *
 * \section OptimizeResult Result and guarantee contracts
 *
 * Gecode::Optimize::SolveResult separates termination from availability of a
 * validated incumbent. Test has_solution() before value(handle), and inspect
 * termination before claiming completion. Values use original slots, including
 * a separate active mask for tombstones. Missing bounds/gaps are absent, not
 * zero. The common relative gap is
 * abs(primal-dual)/max(1,abs(primal),abs(dual)); native_backend_gap, when present,
 * retains a vendor's different convention.
 *
 * Numerical results are tolerance-qualified. Native Exact uses admitted integer
 * arithmetic, original-domain/constraint checking and finite search; it does
 * not export a complete independently checkable proof. Certified is unsupported
 * by these solve routes. Checked LP deductions do not upgrade a complete solve
 * into a certified proof artifact. Infeasibility, local exhaustion and
 * unavailable numerical data are distinct outcomes.
 *
 * A SolveBudget shares a monotonic deadline, cancellation and node accounting.
 * Copying, conversion and checking are included in the documented whole-call
 * allowance. Calls into propagation, factorization or a backend are cooperative
 * and can overrun a wall limit. Publication rules differ by operation: inspect
 * the result status and availability flags even when historical diagnostics or
 * a previously accepted incumbent remain present.
 */

/**
 * \page PageOptimizeNative Native search, starts and optional enhancements
 *
 * \section OptimizeNativeAdmission Exact subset and complete starts
 *
 * The native bridge admits finite Integer/Binary/SemiInteger domains and integral
 * coefficients/finite row sides within native integer limits. SemiInteger keeps
 * its zero alternative. Conservative sums of absolute products bound row,
 * indicator and objective activities; cancellation is not used to evade those
 * guards. Objective offsets and all attainable objective values must remain
 * exactly representable in result doubles. Unsupported fractions, ranges or
 * native-global storage limits fail before search; they never imply infeasibility.
 *
 * Complete primal_start entries use original handles and exact integral values.
 * Every active ordinary slot must be supplied. Only live indicator inactivity
 * gates may be completed from their exact activators; explicitly supplied gates
 * must agree. Removed indicators do not imply a retained gate's value. Other
 * private/fixed slots are not inferred. Unresolved partial starts are Unsupported,
 * and invalid/infeasible complete assignments are InvalidModel. No near-integral
 * rounding or permanent fixing is performed. A timely checked start seeds the
 * incumbent and an unfixed original root with a strict improvement cutoff.
 *
 * Native routes require threads=1 and random_seed=0. Gap tolerances do not
 * enable early gap stopping. Existing native CP search options remain available
 * through their original APIs independently of these facade limitations.
 *
 * Ordinary native search recognizes a bounded exact binary capacity case: all
 * active variables are Binary with domains [0,1], and one same-sign integral
 * row covers them all, with one capacity side and no indicators or globals.
 * Positive upper rows and their negative lower-row equivalents are supported.
 * A capacity-indexed dynamic program uses two rolling value rows and packed
 * decisions, with capacity 65536, 32 million transitions, 8 MiB accounted payload
 * and a cooperative 250 ms local preprocessing cap. Its completed witness supplies a branch
 * preference and its exact optimum supplies a one-sided objective bound.
 * Existing search still checks and publishes the solution and termination.
 * Other model shapes or size-cap misses use the existing brancher. Time and
 * cancellation checks remain active during preprocessing. Explicit checked-LP
 * and local-neighborhood construction do not enable this optimization.
 * The DP preference follows the engine's actual descent order. BestBound favors
 * the region containing that witness only when objective bounds tie. Generic
 * smallest-domain/minimum-first branching and non-DP frontier order are unchanged.
 *
 * \section OptimizeNativeRoutes Explicit native entry points
 *
 * | API | Behavior and evidence |
 * | --- | --- |
 * | Gecode::Optimize::solve_native | Native propagation and BAB. An interrupted incumbent may be returned, but no unfinished-frontier global bound is exposed. |
 * | Gecode::Optimize::solve_native_lp | Native constraints plus a sparse ordinary-row relaxation. LP bounds/domain deductions require checked integer certificates; numerical LP infeasibility alone never prunes. Native, HiGHS and checked-wide-integer support are required. |
 * | Gecode::Optimize::solve_native_search | DepthFirst or BestBound frontier with explicit storage cap. Every queued/active region remains represented during partial expansion; interrupted bounds aggregate all unresolved regions and the incumbent when initial compilation established a bound. |
 * | Gecode::Optimize::solve_native_neighborhoods | The same frontier plus at most one bounded BinaryHamming incumbent improvement attempt. See below. |
 *
 * NativeLpSettings can schedule root or after-bound-change relaxations and
 * explicitly enable root_cover_cuts. Covers are independently verified against
 * immutable original ordinary rows and global domains. An augmentation and its
 * exact evidence retain their owning source. LP suggestions select candidates;
 * they are not original feasible witnesses or proof. Local-scope cuts are not
 * promoted into the global root pool.
 *
 * NativeSearchOptions::branching enables BinaryReliability. Only completed
 * finite paired propagation probes update solve-local history. These gains are
 * not LP pseudocosts. Probes choose a split; they publish neither incumbents nor
 * pruning evidence. General-integer reliability and LP-informed branching are
 * unsupported.
 *
 * \section OptimizeNeighborhood One bounded incumbent neighborhood
 *
 * NativeNeighborhoodOptions wraps the ordinary search options without enabling
 * any existing default route. BinaryHamming waits for a checked incumbent and
 * a surviving stable parent. It posts a fresh original native model, a strict
 * original objective cutoff and a Hamming radius. Distance counts all eligible
 * original nonfixed Binary slots without indicator_origin. It counts slots,
 * not independent mathematical decisions; equality-linked slots count separately.
 * Other original variables and constraints remain, including globals and semis.
 *
 * The neighborhood may find an improvement outside the active parent. Only a
 * timely exact original-model-validated assignment is published after local
 * cleanup. Local bounds, infeasibility and exhaustion never become global proof.
 * The main frontier remains represented throughout the attempt. This operation
 * cannot create the first incumbent of a cold solve and is not RINS, RENS,
 * partial-start repair or an automatic heuristic portfolio.
 *
 * Settings cap status attempts, source entries, coordinator work, retained
 * Spaces, distance-variable count and local elapsed time. These are not byte
 * or CPU-instruction guarantees. Local caps stop optional work; outer limits
 * stop the whole solve. With reliability and neighborhoods enabled, the shared
 * node count equals frontier admissions + probe status attempts + neighborhood
 * status attempts. The statistics' budget_nodes fields repeat that total; do
 * not add them again. Optional local admissions reserve two ordinary child slots.
 */

/**
 * \page PageOptimizeLP Numerical LP observations, basis, evidence and sensitivity
 *
 * These explicit workflows use ordinary Continuous linear models and numerical
 * HiGHS semantics. They do not automatically relax a MIP or replace unsupported
 * indicators/globals. Historical artifacts own their source model and original
 * slot mapping. Available zero values, unavailable data and unrequested data
 * have different states; read those states before optional payloads.
 *
 * | Operation | Returned data | Restrictions |
 * | --- | --- | --- |
 * | Gecode::Optimize::solve_lp_observed | Original row activity/slack, checked duals/reduced costs and basis statuses | Accepted duals require timely optimal primal/dual data and independent KKT checks. Basis data is not a proof. |
 * | Gecode::Optimize::make_lp_basis and Gecode::Optimize::solve_lp_with_basis | Immutable original basis input and explicit accepted/repaired/rejected submission | Exact owner/revision/content checks; no simultaneous primal start, silent cold fallback or faster-solve guarantee. |
 * | Gecode::Optimize::analyze_lp_evidence | Independently checked numerical primal rays or Farkas multipliers using explicit private auxiliary solves | Original and auxiliary coordinates/statuses remain distinct; no exact infeasibility certificate claim. |
 * | Gecode::Optimize::analyze_lp_sensitivity | One-parameter objective-coefficient or common equality-RHS interval for a selected optimal basis | Additional private factorization/system solves, zero optimization runs; numerical intervals only. |
 *
 * \section OptimizeSensitivity Selected-basis sensitivity
 *
 * LpSensitivityOptions requests unique LpObjectiveParameter or
 * LpEqualityRhsParameter entries from an owning LpObservedResult. The analyzer
 * checks original feasibility, dual/KKT evidence, complete basis statuses and
 * reconstructed reference point before interval publication. It neither repairs
 * nor chooses another basis. A degenerate optimum can have different intervals
 * for different selected bases/statuses.
 *
 * \code{.cpp}
 * namespace O = Gecode::Optimize;
 * O::Model model;
 * const auto x = model.add_continuous();
 * const auto y = model.add_continuous();
 * const auto balance = model.add_row({{x, 1}, {y, 1}}, 3, 3);
 * model.minimize({{x, 2}, {y, 1}}, 7);
 * const auto observed = O::solve_lp_observed(model);
 * O::LpSensitivityOptions options;
 * options.parameters = {O::LpObjectiveParameter{x},
 *                       O::LpEqualityRhsParameter{balance}};
 * const auto analysis = O::analyze_lp_sensitivity(observed, options);
 * if (analysis.sensitivity) {
 *   const auto* entry = analysis.sensitivity->objective(x);
 *   if (entry && entry->group.state == O::LpSensitivityState::Available) {
 *     // Read entry->interval: tagged endpoints, slope and checks.
 *   }
 * }
 * \endcode
 *
 * Each interval varies one original parameter alone with the selected basis
 * and nonbasic statuses fixed. Equality RHS varies both equal sides together.
 * Endpoints are absolute parameter values; infinity is tagged, not a finite
 * double. A singleton is an available interval. The optional objective_slope
 * describes slope*(parameter-anchor), without adding the original offset again.
 * Variable-bound, inequality-side, matrix-entry and simultaneous perturbation
 * ranges are not supported. There is no CPLEX/Gurobi ranging equivalence claim.
 *
 * Complete means every requested interval passed; Partial allows per-request
 * rejection. Whole-call stop, resource, allocation or cleanup failure clears
 * every interval's availability, including earlier results. Source history and
 * completed diagnostics can remain. Lookups perform no solver work and survive
 * model/session destruction. Size/work/factor-solve caps and one cooperative
 * deadline cover admission through backend cleanup.
 */

/**
 * \page PageOptimizeWorkflows Repeated solves, diagnostics and model exchange
 *
 * | API | Meaning and current scope |
 * | --- | --- |
 * | Gecode::Optimize::SolveSession | Persistent numerical backend state, compatible LP basis reuse and revalidated previous MIP hints; supports observed/basis solves. Reuse is reported, not assumed to improve runtime. |
 * | Gecode::Optimize::solve_lexicographic | Highest-priority-first linear objectives, checked retention rows and explicit degradation. Unfinished stages are not a lexicographic optimum. |
 * | Gecode::Optimize::analyze_conflict | Numerical deletion-filter conflict groups over original rows/bounds/domains; irreducibility is relative to the documented groups, not a minimum-cardinality conflict or exact proof. |
 * | Gecode::Optimize::relax_feasibility | Selected row/bound sides with positive L1 penalties in a private model; repair residuals do not make the unchanged original model feasible. |
 * | Gecode::Optimize::solve_pool | One representative per finite discrete projection; established ranked prefix is separate from a final unranked candidate and from projection exhaustion. |
 * | Gecode::Optimize::presolve_integer | Explicit checked bounded-integer reductions with owning reconstruction. A presolve fixpoint is not original optimality; postsolve returns a checked original witness without transferring a reduced-model global bound. |
 * | Gecode::Optimize::solve_scenarios | Serial sparse objective/bound overrides on ordinary Continuous/Integer/Binary linear models under one batch allowance; no shared search tree. Explicit Native uses its one-shot route. |
 * | Gecode::Optimize::solve_quadratic | Separate weighted-square continuous convex/concave model and original-coordinate checks; see QuadraticModel and QuadraticOptions. |
 *
 * Workflow support is narrower than the base Model vocabulary. Consult each
 * entry point before combining semis, globals, indicators, guarantees, starts or
 * multistage node limits. Unsupported combinations fail explicitly; a sequence
 * of solves does not imply a global consumed-node or proof contract it does not
 * implement. Private model results retain explicit mappings to original slots.
 *
 * \section OptimizeExchange Files and FlatZinc
 *
 * Gecode::Optimize::read_model and Gecode::Optimize::write_model support a strict
 * numerical LP/free-MPS subset. Unsupported dialect/metadata is rejected.
 * Export performs a checked semantic round trip before atomic replacement;
 * it does not silently omit indicators or globals. This is not full support for
 * every vendor extension.
 *
 * The opt-in fzn-gecode-optimize driver uses immutable raw FlatZinc capture,
 * Gecode::Optimize::compile_flatzinc and an independent original-source checker
 * before publishing output. Its admitted integer linear, Boolean, reified and
 * typed-global subset includes all-different, element, table/holey domains,
 * explicit-offset circuit, fixed four-argument cumulative and literal-parameter
 * six-argument Regular. Predicate signatures, annotation forms, alias/domain
 * semantics and finite resource admission are checked explicitly. Other
 * predicates or parameter forms may be rejected even when the original
 * fzn-gecode driver supports them.
 *
 * The separate experimental MiniZinc registration invokes this driver in
 * --minizinc mode. Its compiler library lowers supported forms and preserves
 * explicit rejections elsewhere; it does not change the default registration.
 * The standard -t milliseconds convention belongs to that mode: -t 0 means no
 * limit, whereas the direct --time-limit 0 option requests an immediate limit.
 * Ordinary incomplete MiniZinc output uses protocol status and exit zero;
 * malformed/unsupported inputs remain errors.
 * Namespaced --native-* extra flags expose automatic feature switches,
 * sequential racing, checked LP/cover cuts, frontier order, reliability
 * branching and bounded Hamming neighborhoods. --native-diagnostics on reports
 * requested settings, actual route and available work counters as comments.
 * NativeAutoOptions and solve_native_auto_configured expose the same automatic
 * switches to C++; NativeRaceOptions::automatic controls its automatic candidate.
 * Integer 0..1 source domains retain integer output while becoming internal
 * binary decisions. Bounded automatic presolve can eliminate a singly defined
 * affine objective auxiliary, preserving its bounds and restoring/checking
 * its original value before publication. Numerical MiniZinc model support and
 * general solve annotations remain outside this registration's scope.
 *
 * \section OptimizeBindings C and Python
 *
 * The installed gecode/optimize/c_api.h and shared Gecode::optimize_c target
 * provide a versioned C surface. The Python 3.9+ ctypes package is in python/;
 * it loads a built library and does not download a solver at runtime. The
 * bindings cover model/bulk edits, logic/globals including Regular, sessions,
 * complete native starts, solution pools, feasibility repair, scenarios and
 * LP observations/basis/evidence/sensitivity. C++ API availability alone does
 * not imply a matching binding: standalone lexicographic objectives, conflict
 * analysis, integer presolve and the explicit native neighborhood API remain
 * available only in C++. C handles and owning result objects
 * have explicit close/destroy lifetimes; follow the header's counted-buffer and
 * version fields.
 */

/**
 * \page PageOptimizeBuild Building and linking the optional component
 *
 * Optimize requires C++17 and CMake 3.21+. In a full %Gecode source build,
 * GECODE_ENABLE_OPTIMIZE defaults to OFF. Enable it explicitly; the native
 * bridge also requires integer and search components. GECODE_OPTIMIZE_WITH_NATIVE
 * defaults to ON when those native targets exist. For a native-only component:
 *
 * \code{.sh}
 * cmake -S . -B build/native-optimize \
 *   -DGECODE_ENABLE_OPTIMIZE=ON -DGECODE_OPTIMIZE_WITH_HIGHS=OFF \
 *   -DGECODE_ENABLE_INT_VARS=ON -DGECODE_ENABLE_SEARCH=ON \
 *   -DCMAKE_BUILD_TYPE=Release
 * cmake --build build/native-optimize --config Release --parallel 2
 * ctest --test-dir build/native-optimize -C Release --output-on-failure
 * \endcode
 *
 * A standalone numerical build uses -S gecode/optimize and an installed HiGHS
 * package, or GECODE_OPTIMIZE_HIGHS_SOURCE pointing to a separately obtained
 * source checkout. For reproducible builds, use HiGHS 1.15.1 at commit
 * 04024d701f79feb8e2f18bc3df0dffc04ef05088. CMake does not fetch it. A standalone
 * build cannot enable the native bridge. With both backends disabled, model,
 * checking, exchange and coordinator APIs remain available; solving returns
 * Unsupported rather than a substitute implementation.
 *
 * After installation, use one matching package discovery route:
 * \code{.cmake}
 * find_package(Gecode CONFIG REQUIRED COMPONENTS optimize) # combined package
 * # Or: find_package(GecodeOptimize CONFIG REQUIRED)         # standalone
 * target_link_libraries(my_target PRIVATE Gecode::optimize)
 * \endcode
 * Both expose Gecode::optimize and Gecode::gecodeoptimize aliases. For the C
 * library use Gecode::optimize_c. A native-only component request does not
 * require HiGHS. Compiled native libraries and generated configuration headers
 * must match; do not mix different ABI/configuration or sanitizer cohorts.
 *
 * The existing native documentation target uses Doxygen 1.17.0 or newer and uv
 * for generated helper pages. These Optimize pages are part of that target even
 * when the optional solver component is disabled. Documentation does not enable
 * a backend. See docs/optimize.md for build and test commands.
 */
