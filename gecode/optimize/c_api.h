/* Version 1 C ABI. C99; no C++ headers or borrowed solver storage. */
#ifndef GECODE_OPTIMIZE_C_API_H
#define GECODE_OPTIMIZE_C_API_H
#include <stdint.h>
#if defined(_WIN32)
# if defined(GECODE_OPT_C_API_EXPORTS)
#  define GECODE_OPT_API __declspec(dllexport)
# else
#  define GECODE_OPT_API __declspec(dllimport)
# endif
#else
# define GECODE_OPT_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
# define GECODE_OPT_NOEXCEPT noexcept
extern "C" {
#else
# define GECODE_OPT_NOEXCEPT
#endif

typedef uint64_t gecode_opt_handle;
enum { GECODE_OPT_VARIABLE_ID=1, GECODE_OPT_ROW_ID=2,
       GECODE_OPT_GLOBAL_ID=3, GECODE_OPT_INDICATOR_ID=4 };
typedef struct { uint64_t model_id; uint64_t slot; uint32_t kind, reserved; } gecode_opt_id;
typedef struct { gecode_opt_id variable; double coefficient; } gecode_opt_term;
typedef struct { gecode_opt_id variable; double value; } gecode_opt_start;

enum { GECODE_OPT_OK=0, GECODE_OPT_INVALID_ARGUMENT=1, GECODE_OPT_INVALID_HANDLE=2,
       GECODE_OPT_MODEL_ERROR=3, GECODE_OPT_OUT_OF_MEMORY=4, GECODE_OPT_INTERNAL_ERROR=5,
       GECODE_OPT_BUFFER_TOO_SMALL=6, GECODE_OPT_NO_SOLUTION=7, GECODE_OPT_NO_OBSERVATIONS=8, GECODE_OPT_NO_BASIS=9 };
enum { GECODE_OPT_CONTINUOUS=0, GECODE_OPT_INTEGER=1, GECODE_OPT_BINARY=2,
       GECODE_OPT_SEMI_CONTINUOUS=3, GECODE_OPT_SEMI_INTEGER=4 };
enum { GECODE_OPT_MINIMIZE=0, GECODE_OPT_MAXIMIZE=1 };
enum { GECODE_OPT_AUTO=0, GECODE_OPT_HIGHS=1, GECODE_OPT_NATIVE=2 };
enum { GECODE_OPT_NUMERICAL=0, GECODE_OPT_EXACT=1, GECODE_OPT_CERTIFIED=2 };
enum { GECODE_OPT_UNKNOWN=0, GECODE_OPT_OPTIMAL=1, GECODE_OPT_INFEASIBLE=2,
       GECODE_OPT_UNBOUNDED=3, GECODE_OPT_INFEASIBLE_OR_UNBOUNDED=4, GECODE_OPT_TIME_LIMIT=5,
       GECODE_OPT_NODE_LIMIT=6, GECODE_OPT_MEMORY_LIMIT=7, GECODE_OPT_ITERATION_LIMIT=8,
       GECODE_OPT_SOLUTION_LIMIT=9, GECODE_OPT_OBJECTIVE_LIMIT=10, GECODE_OPT_CANCELLED=11,
       GECODE_OPT_NUMERICAL_FAILURE=12, GECODE_OPT_UNSUPPORTED=13, GECODE_OPT_INVALID_MODEL=14,
       GECODE_OPT_BACKEND_ERROR=15 };
enum { GECODE_OPT_OBJECTIVE=0, GECODE_OPT_BEST_BOUND=1, GECODE_OPT_ABSOLUTE_GAP=2,
       GECODE_OPT_RELATIVE_GAP=3, GECODE_OPT_NATIVE_GAP=4 };
enum { GECODE_OPT_BACKEND_NAME=0, GECODE_OPT_BACKEND_VERSION=1, GECODE_OPT_MESSAGE=2 };

typedef struct {
  uint64_t struct_size;
  int32_t backend, guarantee, threads, random_seed;
  double time_limit_seconds, relative_gap, absolute_gap;
  double feasibility_tolerance, integrality_tolerance;
  uint64_t node_limit;
  int32_t has_node_limit, reserved;
  gecode_opt_handle cancellation; /* zero means no cancellation token */
  const gecode_opt_start* primal_start;
  uint64_t primal_start_count;
} gecode_opt_options_v1;

typedef struct {
  uint64_t model_id, revision, variable_slots;
  int32_t termination, guarantee, has_solution, solution_validated, start_submitted, reserved;
  double elapsed_seconds;
} gecode_opt_result_info_v1;
typedef struct {
  uint64_t solve_calls, model_loads, incremental_updates, unchanged_models;
  uint64_t basis_warm_starts, incumbent_starts;
} gecode_opt_session_statistics_v1;

/* Atomic bulk input records; exact struct_size and zero reserved required.
 * Names are NUL-terminated UTF-8 (NULL means empty), copied during the call.
 * CSR uses independent counts; names_count is zero or the row count. */
typedef struct {
  uint64_t struct_size;
  int32_t type, reserved;
  double lower, upper;
  const char* name;
} gecode_opt_variable_spec_v1;
typedef struct {
  uint64_t struct_size, reserved;
  const gecode_opt_term* terms;
  uint64_t term_count;
  double lower, upper;
  const char* name;
} gecode_opt_row_spec_v1;
typedef struct {
  uint64_t struct_size, reserved;
  const gecode_opt_id* columns; uint64_t columns_count;
  const uint64_t* row_start; uint64_t row_start_count;
  const uint64_t* column; uint64_t column_count;
  const double* coefficient; uint64_t coefficient_count;
  const double* lower; uint64_t lower_count;
  const double* upper; uint64_t upper_count;
  const char* const* names; uint64_t names_count;
} gecode_opt_sparse_row_batch_v1;

/* Additive workflow records. No existing ABI-1 record changes layout. */
typedef struct { int32_t present, reserved; double value; } gecode_opt_optional_number_v1;
enum { GECODE_OPT_POOL_INCOMPLETE=0, GECODE_OPT_POOL_REQUESTED_LIMIT=1, GECODE_OPT_POOL_EXHAUSTED=2 };
enum { GECODE_OPT_RELAX_LOWER=0, GECODE_OPT_RELAX_UPPER=1 };
enum { GECODE_OPT_REPAIR_MINIMUM_VIOLATION=0, GECODE_OPT_REPAIR_VIOLATION=1,
       GECODE_OPT_REPAIR_ORIGINAL_OBJECTIVE=2 };
enum { GECODE_OPT_REPAIR_MESSAGE=0, GECODE_OPT_REPAIR_WORKFLOW_MESSAGE=1,
       GECODE_OPT_REPAIR_VALIDATION_MESSAGE=2, GECODE_OPT_REPAIR_ITEM_NAME=3,
       GECODE_OPT_REPAIR_STAGE_NAME=4 };
typedef struct {
  uint64_t struct_size;
  gecode_opt_options_v1 solve;
  uint64_t max_solutions;
  int32_t has_projection, reserved;
  const gecode_opt_id* projection;
  uint64_t projection_count;
} gecode_opt_pool_options_v1;
typedef struct {
  uint64_t model_id, revision, projection_count, entry_count, attempt_count, ranked_prefix;
  int32_t termination, completion, guarantee, reserved;
  double elapsed_seconds;
} gecode_opt_pool_info_v1;
typedef struct { uint64_t projection_count; int32_t rank_established, reserved; } gecode_opt_pool_entry_info_v1;
typedef struct {
  int32_t termination, guarantee, candidate_accepted, rank_established;
  gecode_opt_optional_number_v1 objective, remaining_bound;
} gecode_opt_pool_attempt_info_v1;
typedef struct {
  gecode_opt_id source; /* active Variable or Row identity */
  int32_t side, reserved;
  double penalty;
} gecode_opt_relaxation_selection_v1;
typedef struct {
  uint64_t struct_size;
  gecode_opt_options_v1 solve;
  const gecode_opt_relaxation_selection_v1* selections;
  uint64_t selection_count;
  int32_t optimize_original_objective, reserved;
} gecode_opt_repair_options_v1;
typedef struct {
  uint64_t source_model_id, source_revision, private_model_id, private_revision;
  uint64_t variable_slots, item_count, stage_count, completed_stages;
  int32_t termination, guarantee, has_private_model, has_repair;
  int32_t minimum_violation_established, original_objective_optimized;
  int32_t workflow_termination, workflow_guarantee;
  double elapsed_seconds, workflow_elapsed_seconds;
} gecode_opt_repair_info_v1;
typedef struct {
  gecode_opt_id source, slack, penalty_row;
  int32_t side, reserved;
  double original_bound, penalty;
  gecode_opt_optional_number_v1 activity, violation, weighted_violation, slack_value;
} gecode_opt_repair_item_info_v1;
typedef struct {
  uint64_t index;
  int32_t completed, reserved;
  gecode_opt_optional_number_v1 retention_bound;
} gecode_opt_repair_stage_info_v1;
typedef struct {
  int32_t valid, model_valid;
  uint64_t violated_globals;
  double max_bound_violation, max_row_violation, max_integrality_violation, max_indicator_violation;
  gecode_opt_optional_number_v1 objective;
} gecode_opt_validation_info_v1;

/* Distinct finite-box continuous QP input. Exact sizes and zero reserved fields
 * are required. All square/term/name data is copied before an atomic objective
 * replacement. Minimize adds positive weighted squares; maximize subtracts them. */
typedef struct {
  uint64_t struct_size, reserved;
  const gecode_opt_term* terms;
  uint64_t term_count;
  double offset, weight;
  const char* name;
} gecode_opt_weighted_square_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_options_v1 solve;
  uint64_t iteration_limit, max_auxiliary_variables, max_lifted_nonzeros;
  double stationarity_tolerance, complementarity_tolerance, optimality_tolerance;
} gecode_opt_quadratic_options_v1;
typedef struct {
  gecode_opt_result_info_v1 result;
  uint64_t qp_iterations;
  double regularization;
} gecode_opt_quadratic_info_v1;
typedef struct {
  int32_t primal_valid, objective_valid, kkt_available, kkt_valid, bound_valid, reserved;
  gecode_opt_optional_number_v1 max_stationarity, max_complementarity;
  gecode_opt_optional_number_v1 original_objective, normalized_lower_bound, gap_upper_bound;
  uint64_t square_count, gradient_slots; /* complete arrays only if objective_valid */
} gecode_opt_quadratic_checks_v1;
enum { GECODE_OPT_QP_VENDOR_OBJECTIVE=5, GECODE_OPT_QP_VENDOR_DUAL_ESTIMATE=6 };
enum { GECODE_OPT_QP_CHECK_MESSAGE=3 };
enum { GECODE_OPT_QP_SQUARE_RESIDUALS=0, GECODE_OPT_QP_ORIGINAL_GRADIENT=1 };

/* Tokens are process-local, typed, never reused. Destroy invalidates a token;
 * already-running operations retain shared ownership and may complete.
 * Every nonnull pointer must identify valid accessible C storage. Arrays are
 * copied before solving. NULL arrays are accepted only with zero count.
 * Outputs are caller-owned. API errors differ from solver termination codes.
 * last_error is thread-local, valid until the next API call on that thread.
 */
GECODE_OPT_API uint32_t gecode_opt_v1_abi_version(void) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API const char* gecode_opt_v1_last_error(void) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_options_default(gecode_opt_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_capabilities(int32_t backend, int32_t* available, int32_t* lp, int32_t* mip) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_create(gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_identity(gecode_opt_handle, uint64_t* model_id, uint64_t* revision) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_read(const char* filename, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_write(gecode_opt_handle, const char* filename) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_variable(gecode_opt_handle, int32_t type, double lower, double upper, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_row(gecode_opt_handle, const gecode_opt_term*, uint64_t count, double lower, double upper, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
/* These mutations have NO size-query mode. Output capacity must cover the
 * input count (CSR lower_count) before posting. NULL output is valid only for
 * zero capacity/count. Errors leave model/revision/output elements unchanged.
 * Nonempty success advances revision once; valid empty batches do not.
 * Output storage must not overlap input arrays/records/names. */
GECODE_OPT_API int32_t gecode_opt_v1_model_add_variables(gecode_opt_handle, const gecode_opt_variable_spec_v1*, uint64_t count, gecode_opt_id* output, uint64_t capacity) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_rows(gecode_opt_handle, const gecode_opt_row_spec_v1*, uint64_t count, gecode_opt_id* output, uint64_t capacity) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_rows_sparse(gecode_opt_handle, const gecode_opt_sparse_row_batch_v1*, gecode_opt_id* output, uint64_t capacity) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_objective(gecode_opt_handle, const gecode_opt_term*, uint64_t count, int32_t sense, double offset) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_variable_bounds(gecode_opt_handle, gecode_opt_id, double lower, double upper) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_row_bounds(gecode_opt_handle, gecode_opt_id, double lower, double upper) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_coefficient(gecode_opt_handle, gecode_opt_id row, gecode_opt_id variable, double value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_objective_coefficient(gecode_opt_handle, gecode_opt_id variable, double value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_objective_offset(gecode_opt_handle, double offset) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_variable_name(gecode_opt_handle, gecode_opt_id, const char*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_row_name(gecode_opt_handle, gecode_opt_id, const char*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_remove_variable(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_remove_row(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
/* Typed global records preserve aliases and explicit integer index bases.
 * Table values are row-major; value_count must equal arity * tuple_count,
 * including arity zero. Cumulative arrays have independently checked counts. */
GECODE_OPT_API int32_t gecode_opt_v1_model_add_all_different(gecode_opt_handle, const gecode_opt_id* variables, uint64_t count, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_element(gecode_opt_handle, gecode_opt_id index, const gecode_opt_id* elements, uint64_t count, gecode_opt_id result, int64_t index_base, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_table(gecode_opt_handle, const gecode_opt_id* variables, uint64_t arity, const int64_t* values, uint64_t value_count, uint64_t tuple_count, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_cumulative(gecode_opt_handle, const gecode_opt_id* starts, uint64_t count, const int64_t* durations, uint64_t duration_count, const int64_t* heights, uint64_t height_count, int64_t capacity, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_circuit(gecode_opt_handle, const gecode_opt_id* successors, uint64_t count, int64_t index_base, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
/* Sparse deterministic automaton. States are zero based, symbols are signed
 * exact integers within +/-2^53. Missing edges reject; duplicate (from,symbol)
 * keys are invalid, even identical edges. Final-state duplicates are harmless.
 * Empty words accept iff initial_state is final; repeated variables retain
 * equality. All input arrays are copied. element_size and each struct_size
 * must equal sizeof(gecode_opt_regular_transition_v1); reserved must be zero.
 * A failed addition leaves the model unchanged and clears the output ID. */
typedef struct {
  uint64_t struct_size, reserved, from;
  int64_t symbol;
  uint64_t to;
} gecode_opt_regular_transition_v1;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_regular(gecode_opt_handle, const gecode_opt_id* variables, uint64_t variable_count,
  uint64_t state_count, uint64_t initial_state, const gecode_opt_regular_transition_v1* transitions, uint64_t transition_count,
  uint64_t transition_element_size, const uint64_t* final_states, uint64_t final_count, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_remove_global(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_set_global_name(gecode_opt_handle, gecode_opt_id, const char*) GECODE_OPT_NOEXCEPT;
/* Indicator posting is one atomic model edit. has_gate is 0/1 and gate is
 * zeroed when absent. Generated rows remain private and protected. Removing
 * the indicator removes its rows; a generated gate remains until removed. */
GECODE_OPT_API int32_t gecode_opt_v1_model_add_indicator(gecode_opt_handle, gecode_opt_id activator, int32_t active_value, const gecode_opt_term*, uint64_t count, double lower, double upper, const char* name, gecode_opt_id* indicator, int32_t* has_gate, gecode_opt_id* gate) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_remove_indicator(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
/* Atomic ordinary-row posting; no helper handle/group removal is exposed.
 * AND(empty)=true and OR(empty)=false; inputs/result must have Binary type. */
GECODE_OPT_API int32_t gecode_opt_v1_model_add_boolean_and(gecode_opt_handle, gecode_opt_id result, const gecode_opt_id* inputs, uint64_t count, const char* name) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_model_add_boolean_or(gecode_opt_handle, gecode_opt_id result, const gecode_opt_id* inputs, uint64_t count, const char* name) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_cancellation_create(gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_cancellation_cancel(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_cancellation_is_cancelled(gecode_opt_handle, int32_t*) GECODE_OPT_NOEXCEPT;
/* Independent registry owner sharing the same thread-safe cancellation state. */
GECODE_OPT_API int32_t gecode_opt_v1_cancellation_copy(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_cancellation_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_create(gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_reset(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_statistics(gecode_opt_handle, gecode_opt_session_statistics_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_solve(gecode_opt_handle model, const gecode_opt_options_v1*, gecode_opt_handle* result) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_solve(gecode_opt_handle session, gecode_opt_handle model, const gecode_opt_options_v1*, gecode_opt_handle* result) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_result_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_result_info(gecode_opt_handle, gecode_opt_result_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
/* present=0 and value=0 represent absence, never an ambiguous NaN sentinel. */
GECODE_OPT_API int32_t gecode_opt_v1_result_number(gecode_opt_handle, int32_t field, int32_t* present, double* value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_result_value(gecode_opt_handle, gecode_opt_id variable, double* value) GECODE_OPT_NOEXCEPT;
/* Query with NULL buffer(s), capacity0. required includes the string NUL.
 * Insufficient capacity writes no array/string elements. Deleted/missing
 * values are zero with distinct active/present masks. */
GECODE_OPT_API int32_t gecode_opt_v1_result_values(gecode_opt_handle, double* values, uint8_t* active, uint8_t* present, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_result_text(gecode_opt_handle, int32_t field, char* buffer, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* Original-coordinate continuous-LP observations. These additive symbols keep
 * ABI version 1. No observed-result token implicitly converts to Result; use
 * copy_result for a new independent owner. Group availability is independent
 * of primal termination. Basis/duals are numerical observations, not proofs.
 * NULL options use defaults; outer/nested sizes must match exactly and reserved
 * fields must be zero. Output size arguments are exact sizeof(record); returned
 * struct_size is populated and all reserved fields are zero. Bulk arrays include
 * inactive historical slots; individual row/column lookups reject tombstones.
 * Absent observation owners return NO_OBSERVATIONS for observation accessors.
 * Buffer queries use NULL/capacity zero; short buffers receive no elements. */
enum { GECODE_OPT_LP_NOT_REQUESTED=0, GECODE_OPT_LP_AVAILABLE=1,
       GECODE_OPT_LP_UNAVAILABLE=2, GECODE_OPT_LP_REJECTED=3 };
enum { GECODE_OPT_LP_REASON_NONE=0, GECODE_OPT_LP_REASON_NOT_REQUESTED=1,
       GECODE_OPT_LP_REASON_UNSUPPORTED=2, GECODE_OPT_LP_REASON_NO_BACKEND_SOLVE=3,
       GECODE_OPT_LP_REASON_NO_PRIMAL_POINT=4, GECODE_OPT_LP_REASON_NOT_OPTIMAL=5,
       GECODE_OPT_LP_REASON_NO_DUAL_POINT=6, GECODE_OPT_LP_REASON_NO_BASIS=7,
       GECODE_OPT_LP_REASON_ELIDED_CONSTANT_ROWS=8, GECODE_OPT_LP_REASON_INTERRUPTED=9,
       GECODE_OPT_LP_REASON_INVALID_BACKEND_DATA=10, GECODE_OPT_LP_REASON_FAILED_CHECKS=11,
       GECODE_OPT_LP_REASON_ALLOCATION_FAILURE=12, GECODE_OPT_LP_REASON_INVALID_MODEL=13 };
enum { GECODE_OPT_LP_BASIS_LOWER=0, GECODE_OPT_LP_BASIS_BASIC=1,
       GECODE_OPT_LP_BASIS_UPPER=2, GECODE_OPT_LP_BASIS_ZERO=3,
       GECODE_OPT_LP_BASIS_NONBASIC_UNSPECIFIED=4 };
enum { GECODE_OPT_LP_DUAL_NONE=0, GECODE_OPT_LP_DUAL_BACKEND=1,
       GECODE_OPT_LP_DUAL_DERIVED_CONSTANT_ROW=2 };
enum { GECODE_OPT_LP_PRIMAL_ROWS=0, GECODE_OPT_LP_DUAL_POINT=1, GECODE_OPT_LP_BASIS=2 };
enum { GECODE_OPT_LP_BACKEND_NAME=0, GECODE_OPT_LP_BACKEND_VERSION=1,
       GECODE_OPT_LP_PRIMAL_MESSAGE=2, GECODE_OPT_LP_DUAL_MESSAGE=3,
       GECODE_OPT_LP_BASIS_MESSAGE=4, GECODE_OPT_LP_CHECK_MESSAGE=5 };
enum { GECODE_OPT_LP_CAP_BACKEND=0, GECODE_OPT_LP_CAP_VERSION=1, GECODE_OPT_LP_CAP_LIMITATION=2 };
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_options_v1 solve;
  int32_t duals, basis;
  double dual_feasibility, stationarity, complementarity, objective_gap;
} gecode_opt_lp_options_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t available, duals, basis_export, reserved_flags;
  uint64_t limitation_count;
} gecode_opt_lp_capabilities_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_result_info_v1 result;
  int32_t has_observations, reserved_flags;
  uint64_t model_id, revision, row_slots, column_slots;
} gecode_opt_lp_info_v1;
typedef struct {
  uint64_t struct_size, reserved;
  double dual_feasibility, stationarity, complementarity, objective_gap, primal_check_tolerance;
  gecode_opt_optional_number_v1 backend_primal_tolerance, backend_dual_tolerance;
} gecode_opt_lp_metadata_v1;
typedef struct { uint64_t struct_size, reserved; int32_t state, reason; } gecode_opt_lp_group_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t active, dual_source, has_basis, basis;
  gecode_opt_optional_number_v1 activity, lower_slack, upper_slack, dual;
} gecode_opt_lp_row_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t active, has_basis, basis, reserved_flags;
  gecode_opt_optional_number_v1 reduced_cost;
} gecode_opt_lp_column_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t primal_valid, dual_signs_valid, stationarity_valid, complementarity_valid,
          gap_valid, accepted;
  gecode_opt_optional_number_v1 max_dual_sign_violation, max_stationarity,
          max_complementarity, dual_objective_estimate, normalized_gap;
} gecode_opt_lp_checks_v1;
GECODE_OPT_API int32_t gecode_opt_v1_lp_capabilities(gecode_opt_lp_capabilities_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_capability_text(int32_t field, uint64_t index, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_options_default(gecode_opt_lp_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_solve_lp_observed(gecode_opt_handle model, const gecode_opt_lp_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_solve_lp_observed(gecode_opt_handle session, gecode_opt_handle model, const gecode_opt_lp_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_copy_result(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_info(gecode_opt_handle, gecode_opt_lp_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_metadata(gecode_opt_handle, gecode_opt_lp_metadata_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_group(gecode_opt_handle, int32_t group, gecode_opt_lp_group_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_checks(gecode_opt_handle, gecode_opt_lp_checks_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_row(gecode_opt_handle, gecode_opt_id, gecode_opt_lp_row_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_column(gecode_opt_handle, gecode_opt_id, gecode_opt_lp_column_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_rows(gecode_opt_handle, gecode_opt_lp_row_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_columns(gecode_opt_handle, gecode_opt_lp_column_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_observed_result_text(gecode_opt_handle, int32_t field, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* Owning LP basis submission. Factories copy source snapshots and require
 * original-slot counts, with -1 only for inactive slots. Status buffers use
 * VARIABLE_ID or ROW_ID as entity_kind. Empty arrays are valid for empty models.
 * Factories do not solve; singularity may be repaired/rejected upon submission.
 * Submission requires identical source identity/revision and active content.
 * All options use the existing LP options layout. No primal_start is allowed.
 * Solver termination and submission state are independent. Copied child tokens
 * own their data and outlive every parent/model/session token. Absent requested
 * basis returns NO_BASIS; a missing observation artifact is not an invalid token.
 * Info outputs require exact size; reserved output fields are always zero. */
enum { GECODE_OPT_BASIS_CALLER=0, GECODE_OPT_BASIS_OBSERVATIONS=1 };
enum { GECODE_OPT_BASIS_NOT_ATTEMPTED=0, GECODE_OPT_BASIS_ACCEPTED=1,
       GECODE_OPT_BASIS_REPAIRED=2, GECODE_OPT_BASIS_REJECTED=3,
       GECODE_OPT_BASIS_INTERRUPTED=4 };
typedef struct {
  uint64_t struct_size, reserved, model_id, revision, row_slots, column_slots;
  int32_t origin, reserved_flags;
} gecode_opt_basis_info_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_result_info_v1 result;
  uint64_t requested_model_id, requested_revision;
  int32_t has_requested_basis, state, backend_attempted,
          has_statuses_changed, statuses_changed, reserved_flags;
} gecode_opt_basis_result_info_v1;
GECODE_OPT_API int32_t gecode_opt_v1_basis_from_observed(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_from_model(gecode_opt_handle, const int32_t* columns, uint64_t column_count, const int32_t* rows, uint64_t row_count, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_info(gecode_opt_handle, gecode_opt_basis_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_statuses(gecode_opt_handle, int32_t entity_kind, int32_t*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_row(gecode_opt_handle, gecode_opt_id, int32_t*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_column(gecode_opt_handle, gecode_opt_id, int32_t*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_solve_lp_with_basis(gecode_opt_handle model, gecode_opt_handle basis, const gecode_opt_lp_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_session_solve_lp_with_basis(gecode_opt_handle session, gecode_opt_handle model, gecode_opt_handle basis, const gecode_opt_lp_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_result_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_result_info(gecode_opt_handle, gecode_opt_basis_result_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_result_message(gecode_opt_handle, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_result_copy_observed(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_basis_result_copy_basis(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;

/* Quadratic tokens never convert to ordinary Model or Result tokens. Every
 * ordinary solve/session/workflow rejects a QP model token. NULL options use
 * defaults. Unsupported solver policies remain explicit termination values.
 * Info/check outputs require exact sizeof(record), like the other v1 outputs.
 * Array queries follow result_values/text conventions above; absent complete
 * objective evaluation yields zero array length, distinguished by objective_valid.
 * QP result_number accepts ordinary scalar fields plus the two vendor fields.
 * KKT residuals are absent unless kkt_available; bound/gap presence is explicit.
 * gap_upper_bound cancels the objective offset before rounding and need not
 * equal the scalar absolute_gap. Neither implies an Exact guarantee. */
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_capabilities(int32_t* available) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_options_default(gecode_opt_quadratic_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_create(gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_identity(gecode_opt_handle, uint64_t* model_id, uint64_t* revision) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_add_continuous(gecode_opt_handle, double lower, double upper, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_add_row(gecode_opt_handle, const gecode_opt_term*, uint64_t count, double lower, double upper, const char* name, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_set_objective(gecode_opt_handle, const gecode_opt_weighted_square_v1*, uint64_t square_count, const gecode_opt_term* linear, uint64_t linear_count, int32_t sense, double offset) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_set_variable_bounds(gecode_opt_handle, gecode_opt_id, double lower, double upper) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_set_row_bounds(gecode_opt_handle, gecode_opt_id, double lower, double upper) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_set_coefficient(gecode_opt_handle, gecode_opt_id row, gecode_opt_id variable, double value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_remove_variable(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_model_remove_row(gecode_opt_handle, gecode_opt_id) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_solve(gecode_opt_handle model, const gecode_opt_quadratic_options_v1*, gecode_opt_handle* result) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_info(gecode_opt_handle, gecode_opt_quadratic_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_checks(gecode_opt_handle, gecode_opt_quadratic_checks_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_number(gecode_opt_handle, int32_t field, int32_t* present, double* value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_value(gecode_opt_handle, gecode_opt_id variable, double* value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_values(gecode_opt_handle, double* values, uint8_t* active, uint8_t* present, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_array(gecode_opt_handle, int32_t field, double* values, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_quadratic_result_text(gecode_opt_handle, int32_t field, char* buffer, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* Pools retain original slot identities. Entries return NEW owning Result
 * tokens with Unknown termination and no original-model optimality bound.
 * Attempt bounds refer to the remaining, restricted model, never the original.
 * max_solutions stops enumeration without proving exhaustion. NULL options use
 * defaults. has_projection=0 requires NULL projection and count zero. */
GECODE_OPT_API int32_t gecode_opt_v1_pool_options_default(gecode_opt_pool_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_solve(gecode_opt_handle model, const gecode_opt_pool_options_v1*, gecode_opt_handle* pool) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_info(gecode_opt_handle, gecode_opt_pool_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_projection(gecode_opt_handle, gecode_opt_id*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_entry_info(gecode_opt_handle, uint64_t index, gecode_opt_pool_entry_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_entry_result(gecode_opt_handle, uint64_t index, gecode_opt_handle* result) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_entry_projection(gecode_opt_handle, uint64_t index, int64_t*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_attempt_info(gecode_opt_handle, uint64_t index, gecode_opt_pool_attempt_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_pool_message(gecode_opt_handle, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
/* Repairs never manufacture a feasible/optimal original-model Result. The
 * final/stage Result tokens belong to the PRIVATE model and own independent
 * copies, surviving repair/model destruction. Source values and validation
 * are separate. Source-variable masks include historical deleted slots.
 * Mapping supplies zero private IDs when no private model exists. */
GECODE_OPT_API int32_t gecode_opt_v1_repair_options_default(gecode_opt_repair_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_solve(gecode_opt_handle model, const gecode_opt_repair_options_v1*, gecode_opt_handle* repair) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_info(gecode_opt_handle, gecode_opt_repair_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_number(gecode_opt_handle, int32_t field, int32_t* present, double* value) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_original_value(gecode_opt_handle, gecode_opt_id variable, double*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_original_values(gecode_opt_handle, double*, uint8_t* active, uint8_t* present, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_variable_map(gecode_opt_handle, gecode_opt_id* source, gecode_opt_id* private_ids, uint8_t* active, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_validation(gecode_opt_handle, gecode_opt_validation_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_item_info(gecode_opt_handle, uint64_t index, gecode_opt_repair_item_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_stage_info(gecode_opt_handle, uint64_t index, gecode_opt_repair_stage_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_stage_result(gecode_opt_handle, uint64_t index, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_final_result(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_violation_lock(gecode_opt_handle, int32_t* present, gecode_opt_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_repair_objective_values(gecode_opt_handle, double*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
/* index must be zero for message/workflow/validation fields; item/stage names
 * require an in-range index. Strings/arrays use the same stable size protocol. */
GECODE_OPT_API int32_t gecode_opt_v1_repair_text(gecode_opt_handle, int32_t field, uint64_t index, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* Owning serial scenario batches (ordinary linear models only). ABI 1 remains
 * additive. No caller Session is mutated; materialize() is C++-only. */
typedef struct { uint64_t batch_id, index; } gecode_opt_scenario_id;
enum { GECODE_OPT_SCENARIO_AUTOMATIC=0, GECODE_OPT_SCENARIO_COLD=1 };
enum { GECODE_OPT_SCENARIO_REJECTED=0, GECODE_OPT_SCENARIO_INTERRUPTED=1, GECODE_OPT_SCENARIO_COMPLETE=2 };
enum { GECODE_OPT_SCENARIO_NOT_STARTED=0, GECODE_OPT_SCENARIO_ATTEMPTED=1 };
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_id entity;
  int32_t has_lower, has_upper;
  double lower, upper;
} gecode_opt_scenario_bounds_v1;
typedef struct {
  uint64_t struct_size, reserved;
  const char* name;
  const gecode_opt_term* objective_coefficients;
  uint64_t objective_count;
  gecode_opt_optional_number_v1 objective_offset;
  const gecode_opt_scenario_bounds_v1* variable_bounds;
  uint64_t variable_count;
  const gecode_opt_scenario_bounds_v1* row_bounds;
  uint64_t row_count;
} gecode_opt_scenario_definition_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_options_v1 solve;
  int32_t reuse, reserved_flags;
  uint64_t max_scenarios, max_patch_entries, max_saved_value_slots, max_work;
} gecode_opt_scenario_options_v1;
typedef struct {
  uint64_t struct_size, reserved, model_id, revision, batch_id, scenario_count, outcome_count;
  int32_t has_batch, completion, has_stop_reason, stop_reason;
  int32_t has_offending_scenario, all_resolved;
  uint64_t offending_scenario, attempted, resolved, work;
  double elapsed_seconds;
  gecode_opt_session_statistics_v1 reuse_statistics;
} gecode_opt_scenario_info_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_scenario_id scenario;
  int32_t state, has_result, has_check, reserved_flags;
  gecode_opt_result_info_v1 result;
  gecode_opt_session_statistics_v1 reuse_delta;
  double elapsed_seconds;
} gecode_opt_scenario_outcome_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t has_check, identity_valid, candidate_examined, objective_matches, exact_witness_validated, reserved_flags;
  /* Meaningful only when candidate_examined; otherwise all zero/absent. */
  gecode_opt_validation_info_v1 validation;
} gecode_opt_scenario_check_v1;
typedef struct {
  uint64_t struct_size, reserved, objective_count, variable_count, row_count;
  gecode_opt_optional_number_v1 objective_offset;
} gecode_opt_scenario_definition_info_v1;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_options_default(gecode_opt_scenario_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
/* All definitions/arrays/names are copied before any solve. Input sizes must
 * equal v1, reserved fields zero, presence flags exactly 0/1. */
GECODE_OPT_API int32_t gecode_opt_v1_solve_scenarios(gecode_opt_handle model, const gecode_opt_scenario_definition_v1*, uint64_t count, uint64_t element_size, const gecode_opt_scenario_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_info(gecode_opt_handle, gecode_opt_scenario_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_id(gecode_opt_handle, uint64_t index, gecode_opt_scenario_id*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_outcome(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_scenario_outcome_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_check(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_scenario_check_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
/* Copy is an independent ordinary Result with PRIVATE owner/revision. Save a
 * map() output to use its values after closing the batch; original IDs differ. */
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_copy_result(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_map(gecode_opt_handle, gecode_opt_id original, gecode_opt_id* private_id) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_value(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_id original, double*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_message(gecode_opt_handle, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
enum { GECODE_OPT_SCENARIO_NAME=0, GECODE_OPT_SCENARIO_VALIDATION_MESSAGE=1 };
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_text(gecode_opt_handle, gecode_opt_scenario_id, int32_t field, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_definition(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_scenario_definition_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_objective(gecode_opt_handle, gecode_opt_scenario_id, gecode_opt_term*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
/* entity_kind selects VARIABLE_ID or ROW_ID. Query size with NULL/capacity 0;
 * short buffers remain unchanged. Returned bounds have original source IDs. */
GECODE_OPT_API int32_t gecode_opt_v1_scenario_batch_bounds(gecode_opt_handle, gecode_opt_scenario_id, int32_t entity_kind, gecode_opt_scenario_bounds_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* O3: additional-work numerical evidence. No source-model Result conversion. */
enum { GECODE_OPT_NO_EVIDENCE=10 };
enum { GECODE_OPT_EVIDENCE_AUTOMATIC=0, GECODE_OPT_EVIDENCE_PRIMAL_RAY=1, GECODE_OPT_EVIDENCE_FARKAS=2, GECODE_OPT_EVIDENCE_BOTH=3 };
enum { GECODE_OPT_EVIDENCE_NOT_REQUESTED=0, GECODE_OPT_EVIDENCE_AVAILABLE=1, GECODE_OPT_EVIDENCE_UNAVAILABLE=2, GECODE_OPT_EVIDENCE_REJECTED=3 };
enum { GECODE_OPT_EVIDENCE_REASON_NONE=0, GECODE_OPT_EVIDENCE_REASON_NOT_REQUESTED=1,
  GECODE_OPT_EVIDENCE_REASON_UNSUPPORTED=2, GECODE_OPT_EVIDENCE_REASON_NO_FEASIBLE_BASE=3,
  GECODE_OPT_EVIDENCE_REASON_NO_IMPROVEMENT=4, GECODE_OPT_EVIDENCE_REASON_NO_CONTRADICTION=5,
  GECODE_OPT_EVIDENCE_REASON_STOPPED=6, GECODE_OPT_EVIDENCE_REASON_INVALID_BACKEND=7,
  GECODE_OPT_EVIDENCE_REASON_FAILED_CHECKS=8, GECODE_OPT_EVIDENCE_REASON_INCONSISTENT=9,
  GECODE_OPT_EVIDENCE_REASON_INVALID_MODEL=10, GECODE_OPT_EVIDENCE_REASON_RESOURCE_LIMIT=11,
  GECODE_OPT_EVIDENCE_REASON_ALLOCATION=12 };
enum { GECODE_OPT_EVIDENCE_COMPLETE=0, GECODE_OPT_EVIDENCE_INTERRUPTED=1, GECODE_OPT_EVIDENCE_ANALYSIS_REJECTED=2 };
enum { GECODE_OPT_EVIDENCE_FEASIBLE_BASE=0, GECODE_OPT_EVIDENCE_RECESSION=1, GECODE_OPT_EVIDENCE_FARKAS_PHASE=2 };
enum { GECODE_OPT_EVIDENCE_LOWER=0, GECODE_OPT_EVIDENCE_UPPER=1 };
enum { GECODE_OPT_EVIDENCE_SOURCE_VARIABLE=0, GECODE_OPT_EVIDENCE_ROW_SIDE=1, GECODE_OPT_EVIDENCE_VARIABLE_SIDE=2 };
enum { GECODE_OPT_EVIDENCE_PRIMAL_GROUP=0, GECODE_OPT_EVIDENCE_FARKAS_GROUP=1 };
enum { GECODE_OPT_EVIDENCE_BASE_VALUE=0, GECODE_OPT_EVIDENCE_DIRECTION_VALUE=1 };
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_options_v1 solve;
  int32_t request, reserved_flags;
  double recession, stationarity, minimum_improvement, minimum_contradiction;
  uint64_t max_auxiliary_variables, max_auxiliary_rows, max_auxiliary_nonzeros;
  uint64_t max_retained_slots, max_work, max_auxiliary_solves;
} gecode_opt_evidence_options_v1;
typedef struct {
  uint64_t struct_size, reserved, model_id, revision;
  int32_t has_evidence, completion, has_stop_reason, stop_reason;
  uint64_t row_slots, column_slots, stage_count, attempted_calls, work;
  double elapsed_seconds;
} gecode_opt_evidence_info_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t state, reason;
} gecode_opt_evidence_group_v1;
typedef struct {
  uint64_t struct_size, reserved;
  double recession, stationarity, minimum_improvement, minimum_contradiction, primal_tolerance;
} gecode_opt_evidence_metadata_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t has_base_check, reserved_flags;
  gecode_opt_validation_info_v1 base_check;
  gecode_opt_optional_number_v1 direction_scale, normalized_objective_slope;
  gecode_opt_optional_number_v1 max_variable_recession_violation, max_row_recession_violation;
} gecode_opt_evidence_primal_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_optional_number_v1 multiplier_scale, contradiction_margin, max_stationarity;
} gecode_opt_evidence_farkas_v1;
/* Diagnostic slots may remain populated after rejection. Group Available alone
 * denotes accepted numerical evidence. Inactive/absent data has present=0. */
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_id source;
  int32_t active, has_side, side, reserved_flags;
  gecode_opt_optional_number_v1 base_value, direction, multiplier, contribution, selected_bound;
} gecode_opt_evidence_slot_v1;
/* Explicitly reported raw auxiliary fields: never ordinary has_solution().
 * Integer termination/guarantee codes can be unknown malformed backend values. */
typedef struct {
  uint64_t struct_size, reserved, model_id, revision, value_count, mask_count;
  int32_t termination_code, guarantee_code, reported_solution_validated, reported_start_submitted;
  double elapsed_seconds;
  gecode_opt_optional_number_v1 objective, best_bound, absolute_gap, relative_gap, native_gap;
} gecode_opt_evidence_raw_result_v1;
typedef struct {
  uint64_t struct_size, reserved, index, private_model_id, private_revision, row_count, column_count, nonzeros;
  int32_t phase, attempted, has_raw_result, candidate_examined;
  gecode_opt_validation_info_v1 check;
  gecode_opt_evidence_raw_result_v1 raw_result;
} gecode_opt_evidence_stage_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_id private_variable, source;
  int32_t kind, has_side, side, reserved_flags;
} gecode_opt_evidence_column_v1;
typedef struct {
  uint64_t struct_size, reserved, slot;
  gecode_opt_optional_number_v1 reported_value;
  int32_t has_reported_mask, reported_mask;
} gecode_opt_evidence_raw_value_v1;
GECODE_OPT_API int32_t gecode_opt_v1_evidence_options_default(gecode_opt_evidence_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_analyze_lp_evidence(gecode_opt_handle model, const gecode_opt_evidence_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_info(gecode_opt_handle, gecode_opt_evidence_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_group(gecode_opt_handle, int32_t group, gecode_opt_evidence_group_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_metadata(gecode_opt_handle, gecode_opt_evidence_metadata_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_primal(gecode_opt_handle, gecode_opt_evidence_primal_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_farkas(gecode_opt_handle, gecode_opt_evidence_farkas_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_slot(gecode_opt_handle, gecode_opt_id source, gecode_opt_evidence_slot_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_slots(gecode_opt_handle, int32_t entity_kind, gecode_opt_evidence_slot_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
/* These typed accepted getters additionally require effective Available. */
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_value(gecode_opt_handle, gecode_opt_id source, int32_t field, double*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_multiplier(gecode_opt_handle, gecode_opt_id source, gecode_opt_evidence_slot_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
enum { GECODE_OPT_EVIDENCE_MESSAGE=0, GECODE_OPT_EVIDENCE_PRIMAL_MESSAGE=1,
       GECODE_OPT_EVIDENCE_FARKAS_MESSAGE=2, GECODE_OPT_EVIDENCE_BASE_CHECK_MESSAGE=3 };
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_text(gecode_opt_handle, int32_t field, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
/* Independent owning typed child; no ordinary Result conversion. */
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_copy_stage(gecode_opt_handle, uint64_t index, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_stage_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_stage_info(gecode_opt_handle, gecode_opt_evidence_stage_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_stage_columns(gecode_opt_handle, gecode_opt_evidence_column_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_stage_raw_values(gecode_opt_handle, gecode_opt_evidence_raw_value_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
enum { GECODE_OPT_EVIDENCE_RAW_BACKEND=0, GECODE_OPT_EVIDENCE_RAW_BACKEND_VERSION=1,
       GECODE_OPT_EVIDENCE_RAW_MESSAGE=2, GECODE_OPT_EVIDENCE_STAGE_CHECK_MESSAGE=3 };
GECODE_OPT_API int32_t gecode_opt_v1_lp_evidence_stage_text(gecode_opt_handle, int32_t field, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

/* Owning numerical sensitivity of the exact supplied O1 result and selected
 * basis. No ordinary Result conversion, implicit solve, or replacement basis.
 * All new records require exact sizes and zero reserved input fields. Requests
 * are explicit/nonempty/unique. Infinity is an endpoint tag, not a parameter.
 * Getters copy historical data and perform no factorization or solver work.
 * Whole-operation stops revoke every interval, retaining source diagnostics.
 * Valid unrequested lookup sets requested=0; foreign/deleted IDs are errors. */
enum { GECODE_OPT_NO_SENSITIVITY=11 };
enum { GECODE_OPT_SENSITIVITY_OBJECTIVE=0, GECODE_OPT_SENSITIVITY_EQUALITY_RHS=1 };
enum { GECODE_OPT_SENSITIVITY_NOT_REQUESTED=0, GECODE_OPT_SENSITIVITY_AVAILABLE=1,
       GECODE_OPT_SENSITIVITY_UNAVAILABLE=2, GECODE_OPT_SENSITIVITY_REJECTED=3 };
enum { GECODE_OPT_SENSITIVITY_COMPLETE=0, GECODE_OPT_SENSITIVITY_PARTIAL=1,
       GECODE_OPT_SENSITIVITY_INTERRUPTED=2, GECODE_OPT_SENSITIVITY_ANALYSIS_REJECTED=3 };
enum { GECODE_OPT_SENSITIVITY_REASON_NONE=0, GECODE_OPT_SENSITIVITY_REASON_NOT_REQUESTED=1,
       GECODE_OPT_SENSITIVITY_REASON_UNSUPPORTED=2, GECODE_OPT_SENSITIVITY_REASON_NOT_OPTIMAL=3,
       GECODE_OPT_SENSITIVITY_REASON_NO_BASIS=4, GECODE_OPT_SENSITIVITY_REASON_INVALID_SOURCE=5,
       GECODE_OPT_SENSITIVITY_REASON_INVALID_BASIS=6, GECODE_OPT_SENSITIVITY_REASON_CHANGED_BASIS=7,
       GECODE_OPT_SENSITIVITY_REASON_REFERENCE_CHECKS=8, GECODE_OPT_SENSITIVITY_REASON_SYSTEM_CHECKS=9,
       GECODE_OPT_SENSITIVITY_REASON_INTERVAL_CHECKS=10, GECODE_OPT_SENSITIVITY_REASON_RESOURCE_LIMIT=11,
       GECODE_OPT_SENSITIVITY_REASON_STOPPED=12, GECODE_OPT_SENSITIVITY_REASON_ALLOCATION=13,
       GECODE_OPT_SENSITIVITY_REASON_BACKEND=14 };
enum { GECODE_OPT_RANGE_FINITE=0, GECODE_OPT_RANGE_NEGATIVE_INFINITY=1,
       GECODE_OPT_RANGE_POSITIVE_INFINITY=2 };
enum { GECODE_OPT_SENSITIVITY_LOWER=0, GECODE_OPT_SENSITIVITY_UPPER=1,
       GECODE_OPT_SENSITIVITY_FIXED=2, GECODE_OPT_SENSITIVITY_FREE=3 };
typedef struct {
  uint64_t struct_size, reserved;
  int32_t kind, reserved_flags;
  gecode_opt_id entity;
} gecode_opt_sensitivity_request_v1;
typedef struct {
  uint64_t struct_size, reserved;
  double primal_feasibility, dual_feasibility, stationarity, complementarity,
         objective_gap, system_absolute, system_relative;
} gecode_opt_sensitivity_checks_options_v1;
typedef struct {
  uint64_t struct_size, reserved, max_rows, max_columns, max_nonzeros, max_requests,
           max_basis_solves, max_factor_entries, max_retained_slots, max_work;
} gecode_opt_sensitivity_limits_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t backend, reserved_flags;
  double time_limit_seconds;
  gecode_opt_handle cancellation;
  gecode_opt_sensitivity_checks_options_v1 checks;
  gecode_opt_sensitivity_limits_v1 limits;
  const gecode_opt_sensitivity_request_v1* requests;
  uint64_t request_count;
} gecode_opt_sensitivity_options_v1;
typedef struct {
  uint64_t struct_size, reserved, model_id, revision;
  int32_t completion, reason, has_stop_reason, stop_reason, has_sensitivity,
          has_basis, guarantee, reserved_flags;
  uint64_t entry_count, factor_order_count, row_slots, column_slots;
  double elapsed_seconds;
} gecode_opt_sensitivity_info_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t factor_setup_attempted, reserved_flags;
  uint64_t basis_solves, coordinator_visits, retained_slots, preparation_visits;
} gecode_opt_sensitivity_work_v1;
typedef struct { uint64_t struct_size, reserved; int32_t state, reason; } gecode_opt_sensitivity_group_v1;
typedef struct {
  uint64_t struct_size, reserved;
  int32_t kind, reserved_flags;
  gecode_opt_optional_number_v1 value;
} gecode_opt_sensitivity_end_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_id entity;
  int32_t side, dual_condition;
} gecode_opt_sensitivity_limiter_v1;
typedef struct {
  uint64_t struct_size, reserved, inequalities;
  int32_t accepted, lower_direction_checked, upper_direction_checked, reserved_flags;
  gecode_opt_optional_number_v1 max_endpoint_violation;
} gecode_opt_sensitivity_interval_checks_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_sensitivity_request_v1 request;
  gecode_opt_sensitivity_group_v1 group;
  uint64_t index;
  int32_t requested, has_interval, has_lower_limiter, has_upper_limiter;
  double anchor;
  gecode_opt_sensitivity_end_v1 lower, upper;
  gecode_opt_optional_number_v1 objective_slope;
  gecode_opt_sensitivity_limiter_v1 lower_limiter, upper_limiter;
  gecode_opt_sensitivity_interval_checks_v1 checks;
} gecode_opt_sensitivity_entry_v1;
typedef struct {
  uint64_t struct_size, reserved;
  gecode_opt_validation_info_v1 primal;
  gecode_opt_lp_checks_v1 kkt;
  int32_t basis_point_matches, reserved_flags;
  gecode_opt_optional_number_v1 max_point_difference, max_system_residual,
                                max_scaled_system_residual;
} gecode_opt_sensitivity_reference_checks_v1;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_options_default(gecode_opt_sensitivity_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_analyze_lp_sensitivity(gecode_opt_handle observed, const gecode_opt_sensitivity_options_v1*, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_destroy(gecode_opt_handle) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_info(gecode_opt_handle, gecode_opt_sensitivity_info_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_work(gecode_opt_handle, gecode_opt_sensitivity_work_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_checks_options(gecode_opt_handle, gecode_opt_sensitivity_checks_options_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_reference_checks(gecode_opt_handle, gecode_opt_sensitivity_reference_checks_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_copy_source_observed(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_copy_basis(gecode_opt_handle, gecode_opt_handle*) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_entry(gecode_opt_handle, uint64_t index, gecode_opt_sensitivity_entry_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_entries(gecode_opt_handle, gecode_opt_sensitivity_entry_v1*, uint64_t element_size, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_objective(gecode_opt_handle, gecode_opt_id, gecode_opt_sensitivity_entry_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_equality_rhs(gecode_opt_handle, gecode_opt_id, gecode_opt_sensitivity_entry_v1*, uint64_t size) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_factor_order(gecode_opt_handle, gecode_opt_id*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_active_slots(gecode_opt_handle, int32_t entity_kind, uint8_t*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;
enum { GECODE_OPT_SENSITIVITY_MESSAGE=0, GECODE_OPT_SENSITIVITY_BACKEND_VERSION=1,
       GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE=2, GECODE_OPT_SENSITIVITY_INTERVAL_MESSAGE=3,
       GECODE_OPT_SENSITIVITY_PRIMAL_MESSAGE=4, GECODE_OPT_SENSITIVITY_KKT_MESSAGE=5 };
GECODE_OPT_API int32_t gecode_opt_v1_sensitivity_text(gecode_opt_handle, int32_t field, uint64_t index, char*, uint64_t capacity, uint64_t* required) GECODE_OPT_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#endif
