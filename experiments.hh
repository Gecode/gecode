/*
 * Switches for our experiments
 */

/*
 * BASE EXPERIMENTS
 *
 */

// Base system: no advisors: OK
#ifdef SYSTEM_BASE
#undef GECODE_USE_ADVISORS
#endif

// Base system with some events switched off: OK
#ifdef SYSTEM_BASE_NO_EVENTS
#undef GECODE_USE_ADVISORS
#define NQ_NO_EVENTS
#define BINLIN_NQ_NO_EVENTS
#define DISTINCT_NAIVE_NO_EVENTS
// Switches off dynamic events
#define BOOL_LINEAR_INT_NO_EVENTS
#endif

// System with advisor on, but nothing else changed: OK
#ifdef SYSTEM_ADVISOR_BASE
#define GECODE_USE_ADVISORS
#endif

// System with advisors on and used naively (just for scheduling)
#ifdef SYSTEM_ADVISOR_BASE_SCHEDULE
#define GECODE_USE_ADVISORS
// OK
#define LE_ADVISOR_BASE
// OK
#define NQ_ADVISOR_BASE
// CRASHES
#define BINLIN_NQ_ADVISOR_BASE
// OK
#define DISTINCT_NAIVE_ADVISOR_BASE
#endif

// System with advisors on and try to avoid execution
#ifdef SYSTEM_ADVISOR_BASE_AVOID
#define GECODE_USE_ADVISORS
// Avoids by checking bounds: OK
#define LE_ADVISOR_AVOID
// Avoids by checking modification events: OK
#define NQ_ADVISOR_AVOID
// Avoids by checking modification events: CRASHES
#define BINLIN_NQ_ADVISOR_AVOID
// Avoids by checking modification events: OK
#define DISTINCT_NAIVE_ADVISOR_AVOID
#endif


/*
 * REAL EXPERIMENTS
 *
 */

// Try to avoid execution
#ifdef SYSTEM_ADVISOR_AVOID
#define GECODE_USE_ADVISORS
// Boolean linear to integer
#define BOOL_LINEAR_INT_ADVISOR
#endif

// Try to improve execution, cheap approach
#ifdef SYSTEM_ADVISOR_IMPROVE_CHEAP
#define GECODE_USE_ADVISORS
// Boolean linear to integer
#define BOOL_LINEAR_INT_ADVISOR
#endif

// Try to improve execution, expensive approach
#ifdef SYSTEM_ADVISOR_IMPROVE_EXPENSIVE
#define GECODE_USE_ADVISORS
// Boolean linear to integer
#define BOOL_LINEAR_INT_ADVISOR
#endif

