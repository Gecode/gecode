/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Contributing authors:
 *     Filip Konvicka <filip.konvicka@logis.cz>
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2003
 *     Mikael Lagerkvist, 2006
 *     LOGIS, s.r.o., 2009
 *     Samuel Gagnon, 2018
 *
 *  Bugfixes provided by:
 *     Alexander Samoilov <alexander_samoilov@yahoo.com>
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <iostream>

namespace Gecode {

  class Space;

  /**
   * \defgroup TaskVarMEPC Generic modification events and propagation conditions
   *
   * Predefined modification events must be taken into account
   * by variable types.
   * \ingroup TaskVar
   */
  //@{
  /// Type for modification events
  typedef int ModEvent;

  /// Generic modification event: failed variable
  const ModEvent ME_GEN_FAILED   = -1;
  /// Generic modification event: no modification
  const ModEvent ME_GEN_NONE     =  0;
  /// Generic modification event: variable is assigned a value
  const ModEvent ME_GEN_ASSIGNED =  1;

  /// Type for propagation conditions
  typedef int PropCond;
  /// Propagation condition to be ignored (convenience)
  const PropCond PC_GEN_NONE     = -1;
  /// Propagation condition for an assigned variable
  const PropCond PC_GEN_ASSIGNED = 0;
  //@}

  /**
   * \brief Modification event deltas
   *
   * Modification event deltas are used by propagators. A
   * propagator stores a modification event for each variable type.
   * They can be accessed through a variable or a view from a given
   * propagator. They can be constructed from a given modevent by
   * a variable or view.
   * \ingroup TaskActor
   */
  typedef int ModEventDelta;

}

#include <gecode/kernel/var-type.hpp>

namespace Gecode {

  /// Configuration class for variable implementations without index structure
  class NoIdxVarImpConf {
  public:
    /// Index for update
    static const int idx_c = -1;
    /// Index for disposal
    static const int idx_d = -1;
    /// Maximal propagation condition
    static const PropCond pc_max = PC_GEN_ASSIGNED;
    /// Freely available bits
    static const int free_bits = 0;
    /// Start of bits for modification event delta
    static const int med_fst = 0;
    /// End of bits for modification event delta
    static const int med_lst = 0;
    /// Bitmask for modification event delta
    static const int med_mask = 0;
    /// Combine modification events \a me1 and \a me2
    static Gecode::ModEvent me_combine(ModEvent me1, ModEvent me2);
    /// Update modification even delta \a med by \a me, return true on change
    static bool med_update(ModEventDelta& med, ModEvent me);
  };

  forceinline ModEvent
  NoIdxVarImpConf::me_combine(ModEvent, ModEvent) {
    GECODE_NEVER; return 0;
  }
  forceinline bool
  NoIdxVarImpConf::med_update(ModEventDelta&, ModEvent) {
    GECODE_NEVER; return false;
  }


  /*
   * These are the classes of interest
   *
   */
  class ActorLink;
  class Actor;
  class Propagator;
  class SubscribedPropagators;
  class LocalObject;
  class Advisor;
  class AFC;
  class Choice;
  class Brancher;
  class Group;
  class PropagatorGroup;
  class BrancherGroup;
  class PostInfo;
  class ViewTraceInfo;
  class PropagateTraceInfo;
  class CommitTraceInfo;
  class PostTraceInfo;
  class TraceRecorder;
  class TraceFilter;
  class Tracer;

  template<class A> class Council;
  template<class A> class Advisors;
  template<class VIC> class VarImp;


  /*
   * Variable implementations
   *
   */

  /**
   * \brief Base-class for variable implementations
   *
   * Serves as base-class that can be used without having to know any
   * template arguments.
   * \ingroup TaskVar
   */
  class VarImpBase {};

  /**
   * \brief Base class for %Variable type disposer
   *
   * Controls disposal of variable implementations.
   * \ingroup TaskVar
   */
  class GECODE_VTABLE_EXPORT VarImpDisposerBase {
  public:
    /// Dispose list of variable implementations starting at \a x
    GECODE_KERNEL_EXPORT virtual void dispose(Space& home, VarImpBase* x);
    /// Destructor (not used)
    GECODE_KERNEL_EXPORT virtual ~VarImpDisposerBase(void);
  };

  /**
   * \brief %Variable implementation disposer
   *
   * Controls disposal of variable implementations.
   * \ingroup TaskVar
   */
  template<class VarImp>
  class VarImpDisposer : public VarImpDisposerBase {
  public:
    /// Constructor (registers disposer with kernel)
    VarImpDisposer(void);
    /// Dispose list of variable implementations starting at \a x
    virtual void dispose(Space& home, VarImpBase* x);
  };

  /// Generic domain change information to be supplied to advisors
  class Delta {
    template<class VIC> friend class VarImp;
  private:
    /// Modification event
    ModEvent me;
  };

  /**
   * \brief Base-class for variable implementations
   *
   * Implements variable implementation for variable implementation
   * configuration of type \a VIC.
   * \ingroup TaskVar
   */
  template<class VIC>
  class VarImp : public VarImpBase {
    friend class Space;
    friend class Propagator;
    template<class VarImp> friend class VarImpDisposer;
    friend class SubscribedPropagators;
  private:
    union {
      /**
       * \brief Subscribed actors
       *
       * The base pointer of the array of subscribed actors.
       *
       * This pointer must be first to avoid padding on 64 bit machines.
       */
      ActorLink** base;
      /**
       * \brief Forwarding pointer
       *
       * During cloning, this is used as the forwarding pointer for the
       * variable. The original value is saved in the copy and restored after
       * cloning.
       *
       */
      VarImp<VIC>* fwd;
    } b;

    /// Index for update
    static const int idx_c = VIC::idx_c;
    /// Index for disposal
    static const int idx_d = VIC::idx_d;
    /// Number of freely available bits
    static const int free_bits = VIC::free_bits;
    /// Number of used subscription entries
    unsigned int entries;
    /// Number of free subscription entries
    unsigned int free_and_bits;
    /// Maximal propagation condition
    static const Gecode::PropCond pc_max = VIC::pc_max;
#ifdef GECODE_HAS_CBS
    /// Unique id for variable
    const unsigned var_id;
#endif

    union {
      /**
       * \brief Indices of subscribed actors
       *
       * The entries from base[0] to base[idx[pc_max]] are propagators,
       * where the entries between base[idx[pc-1]] and base[idx[pc]] are
       * the propagators that have subscribed with propagation condition pc.
       *
       * The entries between base[idx[pc_max]] and base[idx[pc_max+1]] are the
       * advisors subscribed to the variable implementation.
       */
      unsigned int idx[pc_max+1];
      /// During cloning, points to the next copied variable
      VarImp<VIC>* next;
    } u;

    /// Return subscribed actor at index \a pc
    ActorLink** actor(PropCond pc);
    /// Return subscribed actor at index \a pc, where \a pc is non-zero
    ActorLink** actorNonZero(PropCond pc);
    /// Return reference to index \a pc, where \a pc is non-zero
    unsigned int& idx(PropCond pc);
    /// Return index \a pc, where \a pc is non-zero
    unsigned int idx(PropCond pc) const;

    /**
     * \brief Update copied variable \a x
     *
     * The argument \a sub gives the memory area where subscriptions are
     * to be stored.
     */
    void update(VarImp* x, ActorLink**& sub);
    /**
     * \brief Update all copied variables of this type
     *
     * The argument \a sub gives the memory area where subscriptions are
     * to be stored.
     */
    static void update(Space& home, ActorLink**& sub);

    /// Enter propagator to subscription array
    void enter(Space& home, Propagator* p, PropCond pc);
    /// Enter advisor to subscription array
    void enter(Space& home, Advisor* a);
    /// Resize subscription array
    void resize(Space& home);
    /// Remove propagator from subscription array
    void remove(Space& home, Propagator* p, PropCond pc);
    /// Remove advisor from subscription array
    void remove(Space& home, Advisor* a);


  protected:
    /// Cancel all subscriptions when variable implementation is assigned
    void cancel(Space& home);
    /**
     * \brief Run advisors when variable implementation has been modified with modification event \a me and domain change \a d
     *
     * Returns false if an advisor has failed.
     */
    bool advise(Space& home, ModEvent me, Delta& d);
  private:
    /// Run advisors to be run on failure
    void _fail(Space& home);
  protected:
    /// Run advisors to be run on failure and returns ME_GEN_FAILED
    ModEvent fail(Space& home);
#ifdef GECODE_HAS_VAR_DISPOSE
    /// Return reference to variables (dispose)
    static VarImp<VIC>* vars_d(Space& home);
    /// %Set reference to variables (dispose)
    static void vars_d(Space& home, VarImp<VIC>* x);
#endif

  public:
    /// Creation
    VarImp(Space& home);
    /// Creation of static instances
    VarImp(void);

#ifdef GECODE_HAS_CBS
    /// Return variable id
    unsigned int id(void) const;
#endif

    /// \name Dependencies
    //@{
    /** \brief Subscribe propagator \a p with propagation condition \a pc
     *
     * In case \a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     *
     * In case the variable is assigned (that is, \a assigned is
     * true), the subscribing propagator is scheduled for execution.
     * Otherwise, the propagator subscribes and is scheduled for execution
     * with modification event \a me provided that \a pc is different
     * from \a PC_GEN_ASSIGNED.
     */
    void subscribe(Space& home, Propagator& p, PropCond pc,
                   bool assigned, ModEvent me, bool schedule);
    /// Cancel subscription of propagator \a p with propagation condition \a pc
    void cancel(Space& home, Propagator& p, PropCond pc);
    /** \brief Subscribe advisor \a a to variable
     *
     * The advisor \a a is only subscribed if \a assigned is false.
     *
     * If \a fail is true, the advisor \a a is also run when a variable
     * operation triggers failure. This feature is undocumented.
     *
     */
    void subscribe(Space& home, Advisor& a, bool assigned, bool fail);
    /// Cancel subscription of advisor \a a
    void cancel(Space& home, Advisor& a, bool fail);

    /**
     * \brief Return degree (number of subscribed propagators and advisors)
     *
     * Note that the degree of a variable implementation is not available
     * during cloning.
     */
    unsigned int degree(void) const;
    /**
     * \brief Return accumulated failure count (plus degree)
     *
     * Note that the accumulated failure count of a variable implementation
     * is not available during cloning.
     */
    double afc(void) const;
    //@}

    /// \name Cloning variables
    //@{
    /// Constructor for cloning
    VarImp(Space& home, VarImp& x);
    /// Is variable already copied
    bool copied(void) const;
    /// Use forward pointer if variable already copied
    VarImp* forward(void) const;
    /// Return next copied variable
    VarImp* next(void) const;
    //@}

    /// \name Variable implementation-dependent propagator support
    //@{
    /**
     * \brief Schedule propagator \a p with modification event \a me
     *
     * If \a force is true, the propagator is re-scheduled (including
     * cost computation) even though its modification event delta has
     * not changed.
     */
    static void schedule(Space& home, Propagator& p, ModEvent me,
                         bool force = false);
    /**
     * \brief Schedule propagator \a p
     *
     * Schedules a propagator for propagation condition \a pc and
     * modification event \a me. If the variable is assigned,
     * the appropriate modification event is used for scheduling.
     */
    static void reschedule(Space& home, Propagator& p, PropCond pc,
                           bool assigned, ModEvent me);
    /// Project modification event for this variable type from \a med
    static ModEvent me(const ModEventDelta& med);
    /// Translate modification event \a me into modification event delta
    static ModEventDelta med(ModEvent me);
    /// Combine modifications events \a me1 and \a me2
    static ModEvent me_combine(ModEvent me1, ModEvent me2);
    //@}

    /// \name Delta information for advisors
    //@{
    /// Return modification event
    static ModEvent modevent(const Delta& d);
    //@}

    /// \name Bit management
    //@{
    /// Provide access to free bits
    unsigned int bits(void) const;
    /// Provide access to free bits
    unsigned int& bits(void);
    //@}

  protected:
    /// Schedule subscribed propagators
    void schedule(Space& home, PropCond pc1, PropCond pc2, ModEvent me);

  public:
    /// \name Memory management
    //@{
    /// Allocate memory from space
    static void* operator new(size_t,Space&);
    /// Return memory to space
    static void  operator delete(void*,Space&);
    /// Needed for exceptions
    static void  operator delete(void*);
    //@}
  };


  /**
   * \defgroup TaskActorStatus Status of constraint propagation and branching commit
   * Note that the enum values starting with a double underscore should not
   * be used directly. Instead, use the provided functions with the same
   * name without leading underscores.
   *
   * \ingroup TaskActor
   */
  enum ExecStatus {
    __ES_SUBSUMED       = -2, ///< Internal: propagator is subsumed, do not use
    ES_FAILED           = -1, ///< Execution has resulted in failure
    ES_NOFIX            =  0, ///< Propagation has not computed fixpoint
    ES_OK               =  0, ///< Execution is okay
    ES_FIX              =  1, ///< Propagation has computed fixpoint
    ES_NOFIX_FORCE      =  2, ///< Advisor forces rescheduling of propagator
    __ES_PARTIAL        =  2  ///< Internal: propagator has computed partial fixpoint, do not use
  };

  /**
   * \brief Propagation cost
   * \ingroup TaskActor
   */
  class PropCost {
    friend class Space;
  public:
    /// The actual cost values that are used
    enum ActualCost {
      AC_RECORD       = 0, ///< Reserved for recording information
      AC_CRAZY_LO     = 1, ///< Exponential complexity, cheap
      AC_CRAZY_HI     = 1, ///< Exponential complexity, expensive
      AC_CUBIC_LO     = 1, ///< Cubic complexity, cheap
      AC_CUBIC_HI     = 1, ///< Cubic complexity, expensive
      AC_QUADRATIC_LO = 2, ///< Quadratic complexity, cheap
      AC_QUADRATIC_HI = 2, ///< Quadratic complexity, expensive
      AC_LINEAR_HI    = 3, ///< Linear complexity, expensive
      AC_LINEAR_LO    = 4, ///< Linear complexity, cheap
      AC_TERNARY_HI   = 4, ///< Three variables, expensive
      AC_BINARY_HI    = 5, ///< Two variables, expensive
      AC_TERNARY_LO   = 5, ///< Three variables, cheap
      AC_BINARY_LO    = 6, ///< Two variables, cheap
      AC_UNARY_LO     = 6, ///< Only single variable, cheap
      AC_UNARY_HI     = 6, ///< Only single variable, expensive
      AC_MAX          = 6  ///< Maximal cost value
    };
    /// Actual cost
    ActualCost ac;
  public:
    /// Propagation cost modifier
    enum Mod {
      LO, ///< Cheap
      HI  ///< Expensive
    };
  private:
    /// Compute dynamic cost for cost \a lo, expensive cost \a hi, and size measure \a n
    static PropCost cost(Mod m, ActualCost lo, ActualCost hi, unsigned int n);
    /// Constructor for automatic coercion of \a ac
    PropCost(ActualCost ac);
  public:
    /// For recording information (no propagation allowed)
    static PropCost record(void);
    /// Exponential complexity for modifier \a m and size measure \a n
    static PropCost crazy(PropCost::Mod m, unsigned int n);
    /// Exponential complexity for modifier \a m and size measure \a n
    static PropCost crazy(PropCost::Mod m, int n);
    /// Cubic complexity for modifier \a m and size measure \a n
    static PropCost cubic(PropCost::Mod m, unsigned int n);
    /// Cubic complexity for modifier \a m and size measure \a n
    static PropCost cubic(PropCost::Mod m, int n);
    /// Quadratic complexity for modifier \a m and size measure \a n
    static PropCost quadratic(PropCost::Mod m, unsigned int n);
    /// Quadratic complexity for modifier \a m and size measure \a n
    static PropCost quadratic(PropCost::Mod m, int n);
    /// Linear complexity for modifier \a pcm and size measure \a n
    static PropCost linear(PropCost::Mod m, unsigned int n);
    /// Linear complexity for modifier \a pcm and size measure \a n
    static PropCost linear(PropCost::Mod m, int n);
    /// Three variables for modifier \a pcm
    static PropCost ternary(PropCost::Mod m);
    /// Two variables for modifier \a pcm
    static PropCost binary(PropCost::Mod m);
    /// Single variable for modifier \a pcm
    static PropCost unary(PropCost::Mod m);
  };


  /**
   * \brief Actor properties
   * \ingroup TaskActor
   */
  enum ActorProperty {
    /**
     * \brief Actor must always be disposed
     *
     * Normally, a propagator will not be disposed if its home space
     * is deleted. However, if an actor uses external resources,
     * this property can be used to make sure that the actor
     * will always be disposed.
     */
    AP_DISPOSE = (1 << 0),
    /**
     * Propagator is only weakly monotonic, that is, the propagator
     * is only monotonic on assignments.
     *
     */
    AP_WEAKLY = (1 << 1),
    /**
     * A propagator is in fact implementing a view trace recorder.
     *
     */
    AP_VIEW_TRACE = (1 << 2),
    /**
     * A propagator is in fact implementing a trace recorder.
     *
     */
    AP_TRACE = (1 << 3)
  };


  /**
   * \brief Double-linked list for actors
   *
   * Used to maintain which actors belong to a space and also
   * (for propagators) to organize actors in the queue of
   * waiting propagators.
   */
  class ActorLink {
    friend class Actor;
    friend class Propagator;
    friend class Advisor;
    friend class Brancher;
    friend class LocalObject;
    friend class Space;
    template<class VIC> friend class VarImp;
  private:
    ActorLink* _next; ActorLink* _prev;
  public:
    //@{
    /// Routines for double-linked list
    ActorLink* prev(void) const; void prev(ActorLink*);
    ActorLink* next(void) const; void next(ActorLink*);
    ActorLink** next_ref(void);
    //@}

    /// Initialize links (self-linked)
    void init(void);
    /// Remove from predecessor and successor
    void unlink(void);
    /// Insert \a al directly after this
    void head(ActorLink* al);
    /// Insert \a al directly before this
    void tail(ActorLink* al);
    /// Test whether actor link is empty (points to itself)
    bool empty(void) const;
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    template<class T> static ActorLink* cast(T* a);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    template<class T> static const ActorLink* cast(const T* a);
  };


  /**
   * \brief Base-class for both propagators and branchers
   * \ingroup TaskActor
   */
  class GECODE_VTABLE_EXPORT Actor : private ActorLink {
    friend class ActorLink;
    friend class Space;
    friend class Propagator;
    friend class Advisor;
    friend class Brancher;
    friend class LocalObject;
    template<class VIC> friend class VarImp;
    template<class A> friend class Council;
  private:
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static Actor* cast(ActorLink* al);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static const Actor* cast(const ActorLink* al);
    /// Static member to test against during space cloning
    GECODE_KERNEL_EXPORT static Actor* sentinel;
  public:
    /// Create copy
    virtual Actor* copy(Space& home) = 0;

    /// \name Memory management
    //@{
    /// Delete actor and return its size
    GECODE_KERNEL_EXPORT
    virtual size_t dispose(Space& home);
    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// No-op for exceptions
    static void  operator delete(void* p, Space& home);
    //@}
  public:
    /// To avoid warnings
    GECODE_KERNEL_EXPORT virtual ~Actor(void);
    /// Not used
    static void* operator new(size_t s);
    /// Not used
    static void  operator delete(void* p);
  };

  class Home;

  /**
   * \brief Group baseclass for controlling actors
   * \ingroup TaskGroup
   */
  class Group {
    friend class Home;
    friend class Propagator;
    friend class Brancher;
    friend class ViewTraceInfo;
    friend class PropagateTraceInfo;
    friend class CommitTraceInfo;
    friend class PostTraceInfo;
  protected:
    /// Fake id for group of all actors
    static const unsigned int GROUPID_ALL = 0U;
    /// Pre-defined default group id
    static const unsigned int GROUPID_DEF = 1U;
    /// The maximal group number
    static const unsigned int GROUPID_MAX = UINT_MAX >> 2;
    /// The group id
    unsigned int gid;
    /// Next group id
    GECODE_KERNEL_EXPORT
    static unsigned int next;
    /// Mutex for protection
    GECODE_KERNEL_EXPORT
    static Support::Mutex m;
    /// Construct with predefined group id \a gid0
    Group(unsigned int gid0);
  public:
    /// \name Construction and access
    //@{
    /// Constructor
    GECODE_KERNEL_EXPORT
    Group(void);
    /// Copy constructor
    Group(const Group& g);
    /// Assignment operator
    Group& operator =(const Group& g);
    /// Return a unique id for the group
    unsigned int id(void) const;
    /// Check whether actor group \a a is included in this group
    bool in(Group a) const;
    /// Check whether this is a real group (and not just default)
    bool in(void) const;
    //@}
    /// Group of all actors
    GECODE_KERNEL_EXPORT
    static Group all;
    /// Group of actors not in any user-defined group
    GECODE_KERNEL_EXPORT
    static Group def;
  };

  /**
   * \brief Group of propagators
   * \ingroup TaskGroup
   */
  class PropagatorGroup : public Group {
    friend class Propagator;
    friend class ViewTraceInfo;
    friend class PropagateTraceInfo;
    friend class PostTraceInfo;
  protected:
    /// Initialize with group id \a gid
    PropagatorGroup(unsigned int gid);
  public:
    /// \name Construction
    //@{
    /// Constructor
    PropagatorGroup(void);
    /// Copy constructor
    PropagatorGroup(const PropagatorGroup& g);
    /// Assignment operator
    PropagatorGroup& operator =(const PropagatorGroup& g);
    /// To augment a space argument
    Home operator ()(Space& home);
    //@}
    /// \name Move propagators between groups
    //@{
    /// Move propagators from group \a g to this group
    GECODE_KERNEL_EXPORT
    PropagatorGroup& move(Space& home, PropagatorGroup g);
    /// Move propagator \a p to this group
    PropagatorGroup& move(Space& home, Propagator& p);
    /**
     * \brief Move propagator with id \a id to this group
     *
     * Throws an exception of type UnknownPropagator, if no propagator
     * with id \a id exists.
     */
    GECODE_KERNEL_EXPORT
    PropagatorGroup& move(Space& home, unsigned int id);
    //@}
    /// \name Operations on groups
    //@{
    /// Test whether this group is equal to group \a g
    bool operator ==(PropagatorGroup g) const;
    /// Test whether this group is different from group \a g
    bool operator !=(PropagatorGroup g) const;
    /// Return number of propagators in a group
    GECODE_KERNEL_EXPORT
    unsigned int size(Space& home) const;
    /// Kill all propagators in a group
    GECODE_KERNEL_EXPORT
    void kill(Space& home);
    /// Disable all propagators in a group
    GECODE_KERNEL_EXPORT
    void disable(Space& home);
    /**
     * \brief Enable all propagators in a group
     *
     * If \a s is true, the propagators will be scheduled for propagation
     * if needed.
     */
    GECODE_KERNEL_EXPORT
    void enable(Space& home, bool s=true);
    //@}
    /// Group of all propagators
    GECODE_KERNEL_EXPORT
    static PropagatorGroup all;
    /// Group of propagators not in any user-defined group
    GECODE_KERNEL_EXPORT
    static PropagatorGroup def;
  };

  /**
   * \brief Group of branchers
   * \ingroup TaskGroup
   */
  class BrancherGroup : public Group {
    friend class Brancher;
  protected:
    /// Initialize with group id \a gid
    BrancherGroup(unsigned int gid);
  public:
    /// \name Construction
    //@{
    /// Constructor
    BrancherGroup(void);
    /// Copy constructor
    BrancherGroup(const BrancherGroup& g);
    /// Assignment operator
    BrancherGroup& operator =(const BrancherGroup& g);
    /// To augment a space argument
    Home operator ()(Space& home);
    //@}
    /// \name Move branchers between groups
    //@{
    /// Move branchers from group \a g to this group
    GECODE_KERNEL_EXPORT
    BrancherGroup& move(Space& home, BrancherGroup g);
    /// Move brancher \a b to this group
    BrancherGroup& move(Space& home, Brancher& b);
    /**
     * \brief Move brancher with id \a id to this group
     *
     * Throws an exception of type UnknownBrancher, if no brancher
     * with id \a id exists.
     */
    GECODE_KERNEL_EXPORT
    BrancherGroup& move(Space& home, unsigned int id);
    //@}
    /// \name Operations on groups
    //@{
    /// Test whether this group is equal to group \a g
    bool operator ==(BrancherGroup g) const;
    /// Test whether this group is different from group \a g
    bool operator !=(BrancherGroup g) const;
    /// Return number of branchers in a group
    GECODE_KERNEL_EXPORT
    unsigned int size(Space& home) const;
    /// Kill all branchers in a group
    GECODE_KERNEL_EXPORT
    void kill(Space& home);
    //@}
    /// Group of all branchers
    GECODE_KERNEL_EXPORT
    static BrancherGroup all;
    /// Group of branchers not in any user-defined group
    GECODE_KERNEL_EXPORT
    static BrancherGroup def;
  };

  /**
   * \brief %Home class for posting propagators
   */
  class Home {
    friend class PostInfo;
  protected:
    /// The space where the propagator is to be posted
    Space& s;
    /// A propagator (possibly) that is currently being rewritten
    Propagator* p;
    /// A propagator group
    PropagatorGroup pg;
    /// A brancher group
    BrancherGroup bg;
  public:
    /// \name Conversion
    //@{
    /// Initialize the home with space \a s and propagator \a p and group \a g
    Home(Space& s, Propagator* p=NULL,
         PropagatorGroup pg=PropagatorGroup::def,
         BrancherGroup bg=BrancherGroup::def);
    /// Assignment operator
    Home& operator =(const Home& h);
    /// Retrieve the space of the home
    operator Space&(void);
    //@}
    /// \name Extended information
    //@{
    /// Return a home extended by propagator to be rewritten
    Home operator ()(Propagator& p);
    /// Return a home extended by a propagator group
    Home operator ()(PropagatorGroup pg);
    /// Return a home extended by a brancher group
    Home operator ()(BrancherGroup bg);
    /// Return propagator (or NULL) for currently rewritten propagator
    Propagator* propagator(void) const;
    /// Return propagator group
    PropagatorGroup propagatorgroup(void) const;
    /// Return brancher group
    BrancherGroup branchergroup(void) const;
    //@}
    /// \name Forwarding of common space operations
    //@{
    /// Check whether corresponding space is failed
    bool failed(void) const;
    /// Mark space as failed
    void fail(void);
    ///  Notice actor property
    void notice(Actor& a, ActorProperty p, bool duplicate=false);
    //@}
  };

  /**
   * \brief View trace information
   */
  class ViewTraceInfo {
    friend class Space;
    friend class PostInfo;
  public:
    /// What is currently executing
    enum What {
      /// A propagator is currently executing
      PROPAGATOR = 0,
      /// A brancher is executing
      BRANCHER   = 1,
      /// A post function is executing
      POST       = 2,
      /// Unknown
      OTHER      = 3
    };
  protected:
    /// Encoding a tagged pointer or a tagged group id
    ptrdiff_t who;
    /// Record that propagator \a p is executing
    void propagator(Propagator& p);
    /// Record that brancher \a b is executing
    void brancher(Brancher& b);
    /// Record that a post function with propagator group \a g is executing
    void post(PropagatorGroup g);
    /// Record that nothing is known at this point
    void other(void);
  public:
    /// Return what is currently executing
    What what(void) const;
    /// Return currently executing propagator
    const Propagator& propagator(void) const;
    /// Return currently executing brancher
    const Brancher& brancher(void) const;
    /// Return propagator group of currently executing post function
    PropagatorGroup post(void) const;
  };

  /**
   * \brief Class to set group information when a post function is executed
   */
  class PostInfo {
    friend class Space;
  protected:
    /// The home space
    Space& h;
    /// The propagator group
    PropagatorGroup pg;
    /// Next free propagator id
    unsigned int pid;
    /// Whether it is used nested
    bool nested;
  public:
    /// Set information
    PostInfo(Home home);
    /// Reset information
    ~PostInfo(void);
  };

  /**
   * \brief Propagate trace information
   */
  class PropagateTraceInfo {
    friend class Space;
  public:
    /// Propagator status
    enum Status {
      FIX,     ///< Propagator computed fixpoint
      NOFIX,   ///< Propagator did not compute fixpoint
      FAILED,  ///< Propagator failed
      SUBSUMED ///< Propagator is subsumed
    };
  protected:
    /// Propagator id
    unsigned int i;
    /// Propagator group
    PropagatorGroup g;
    /// Propagator
    const Propagator* p;
    /// Status
    Status s;
    /// Initialize
    PropagateTraceInfo(unsigned int i, PropagatorGroup g,
                       const Propagator* p, Status s);
  public:
    /// Return propagator identifier
    unsigned int id(void) const;
    /// Return propagator group
    PropagatorGroup group(void) const;
    /// Return pointer to non-subsumed propagator
    const Propagator* propagator(void) const;
    /// Return propagator status
    Status status(void) const;
  };

  /**
   * \brief Commit trace information
   */
  class CommitTraceInfo {
    friend class Space;
  protected:
    /// Brancher
    const Brancher& b;
    /// Choice
    const Choice& c;
    /// Alternative
    unsigned int a;
    /// Initialize
    CommitTraceInfo(const Brancher& b, const Choice& c, unsigned int a);
  public:
    /// Return brancher identifier
    unsigned int id(void) const;
    /// Return brancher group
    BrancherGroup group(void) const;
    /// Return brancher
    const Brancher& brancher(void) const;
    /// Return choice
    const Choice& choice(void) const;
    /// Return alternative
    unsigned int alternative(void) const;
  };
 
  /**
   * \brief Post trace information
   */
  class PostTraceInfo {
    friend class Space;
    friend class PostInfo;
  public:
    /// Post status
    enum Status {
      POSTED,  ///< Propagator was posted
      FAILED,  ///< Posting failed
      SUBSUMED ///< Propagator not posted as already subsumed
    };
  protected:
    /// Propagator group
    PropagatorGroup g;
    /// Status
    Status s;
    /// Number of posted propagators
    unsigned int n;
    /// Initialize
    PostTraceInfo(PropagatorGroup g, Status s, unsigned int n);
  public:
    /// Return post status
    Status status(void) const;
    /// Return propagator group
    PropagatorGroup group(void) const;
    /// Return number of posted propagators
    unsigned int propagators(void) const;
  };

 /**
   * \brief Base-class for propagators
   * \ingroup TaskActor
   */
  class GECODE_VTABLE_EXPORT Propagator : public Actor {
    friend class ActorLink;
    friend class Space;
    template<class VIC> friend class VarImp;
    friend class Advisor;
    template<class A> friend class Council;
    friend class SubscribedPropagators;
    friend class PropagatorGroup;
  private:
    union {
      /// A set of modification events (used during propagation)
      ModEventDelta med;
      /// The size of the propagator (used during subsumption)
      size_t size;
      /// A list of advisors (used during cloning)
      Gecode::ActorLink* advisors;
    } u;
    /// A tagged pointer combining a pointer to global propagator information and whether the propagator is disabled
    void* gpi_disabled;
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static Propagator* cast(ActorLink* al);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static const Propagator* cast(const ActorLink* al);
    /// Disable propagator
    void disable(Space& home);
    /// Enable propagator
    void enable(Space& home);
  protected:
    /// Constructor for posting
    Propagator(Home home);
    /// Constructor for cloning \a p
    Propagator(Space& home, Propagator& p);
    /// Return forwarding pointer during copying
    Propagator* fwd(void) const;
    /// Provide access to global propagator information
    Kernel::GPI::Info& gpi(void);

  public:
    /// \name Propagation
    //@{
    /**
     * \brief Schedule function
     *
     * The function is executed when a propagator is enabled again.
     * Note that a propagator should be scheduled with the right
     * modification event delta and should only be scheduled if
     * it is legal to execute the propagator.
     */
    virtual void reschedule(Space& home) = 0;
    /**
     * \brief Propagation function
     *
     * The propagation function must return an execution status as
     * follows:
     *  - ES_FAILED: the propagator has detected failure
     *  - ES_NOFIX: the propagator has done propagation
     *  - ES_FIX: the propagator has done propagation and has computed
     *    a fixpoint. That is, running the propagator immediately
     *    again will do nothing.
     *
     * Apart from the above values, a propagator can return
     * the result from calling one of the functions defined by a space:
     *  - ES_SUBSUMED: the propagator is subsumed and has been already
     *    deleted.
     *  - ES_NOFIX_PARTIAL: the propagator has consumed some of its
     *    propagation events.
     *  - ES_FIX_PARTIAL: the propagator has consumed some of its
     *    propagation events and with respect to these events is
     *    at fixpoint
     * For more details, see the individual functions.
     *
     */
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) = 0;
    /// Cost function
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const = 0;
    /**
     * \brief Return the modification event delta
     *
     * This function returns the modification event delta of the currently
     * executing propagator and hence can only be called within the
     * propagate function of a propagator.
     */
    ModEventDelta modeventdelta(void) const;
    /**
     * \brief Advise function
     *
     * The advisor is passed as argument \a a.
     *
     * A propagator must specialize this advise function, if it
     * uses advisors. The advise function must return an execution
     * status as follows:
     *  - ES_FAILED: the advisor has detected failure.
     *  - ES_FIX: the advisor's propagator (that is, this propagator)
     *    does not need to be run.
     *  - ES_NOFIX: the advisor's propagator (that is, this propagator)
     *    must be run.
     *  - ES_NOFIX_FORCE: the advisor's propagator (that is, this propagator)
     *    must be run and it must forcefully be rescheduled (including
     *    recomputation of cost).
     *
     * Apart from the above values, an advisor can return
     * the result from calling the function defined by a space:
     *  - ES_FIX_DISPOSE: the advisor's propagator does not need to be run
     *    and the advisor will be disposed.
     *  - ES_NOFIX_DISPOSE: the advisor's propagator must be run and the
     *    advisor will be disposed.
     *  - ES_NOFIX_FORCE_DISPOSE: the advisor's propagator must be run
     *    , it must forcefully be rescheduled (including recomputation
     *    of cost), and the adviser will be disposed.
     * For more details, see the function documentation.
     *
     * The delta \a d describes how the variable has been changed
     * by an operation on the advisor's variable. Typically,
     * the delta information can only be utilized by either
     * static or member functions of views as the actual delta
     * information is both domain and view dependent.
     *
     */
    GECODE_KERNEL_EXPORT
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Run advisor \a a to be run on failure in failed space
    GECODE_KERNEL_EXPORT
    virtual void advise(Space& home, Advisor& a);
    //@}
    /// \name Information
    //@{
    /// Return the accumlated failure count
    double afc(void) const;
    //@}
#ifdef GECODE_HAS_CBS
    /// \name Marginal distribution
    //@{
    /**
     * \brief Solution distribution
     *
     * Computes the marginal distribution for every variable and value in the
     * propagator. A callback is used to transmit each result.
     */
    /// Signature for function transmitting marginal distributions
    typedef std::function<void(unsigned int prop_id, unsigned int var_id,
                               int val, double dens)> SendMarginal;
    virtual void solndistrib(Space& home, SendMarginal send) const;
    /**
     * \brief Sum of variable cardinalities
     *
     * \param size   Sum of variable cardinalities
     * \param size_b Sum of variable cardinalities for subset involved
     *               in branching decisions
     */
    /// Signature for function testing if variables are candidates to branching decisions
    typedef std::function<bool(unsigned int var_id)> InDecision;
    virtual void domainsizesum(InDecision in, unsigned int& size,
                               unsigned int& size_b) const;
    //@}
#endif
    /// \name Id and group support
    //@{
    /// Return propagator id
    unsigned int id(void) const;
    /// Return group propagator belongs to
    PropagatorGroup group(void) const;
    /// Add propagator to group \a g
    void group(PropagatorGroup g);
    /// Whether propagator is currently disabled
    bool disabled(void) const;
    //@}
  };


  /**
   * \brief %Council of advisors
   *
   * If a propagator uses advisors, it must maintain its advisors
   * through a council.
   * \ingroup TaskActor
   */
  template<class A>
  class Council {
    friend class Advisor;
    friend class Advisors<A>;
  private:
    /// Starting point for a linked list of advisors
    mutable ActorLink* advisors;
  public:
    /// Default constructor
    Council(void);
    /// Construct advisor council
    Council(Space& home);
    /// Test whether council has advisor left
    bool empty(void) const;
    /// Update during cloning (copies all advisors)
    void update(Space& home, Council<A>& c);
    /// Dispose council
    void dispose(Space& home);
  };


  /**
   * \brief Class to iterate over advisors of a council
   * \ingroup TaskActor
   */
  template<class A>
  class Advisors {
  private:
    /// The current advisor
    ActorLink* a;
  public:
    /// Initialize
    Advisors(const Council<A>& c);
    /// Test whether there advisors left
    bool operator ()(void) const;
    /// Move iterator to next advisor
    void operator ++(void);
    /// Return advisor
    A& advisor(void) const;
  };


  /**
   * \brief Base-class for advisors
   *
   * Advisors are typically subclassed for each propagator that
   * wants to use advisors. The actual member function that
   * is executed when a variable is changed, must be implemented
   * by the advisor's propagator.
   *
   * \ingroup TaskActor
   */
  class Advisor : private ActorLink {
    template<class VIC> friend class VarImp;
    template<class A> friend class Council;
    template<class A> friend class Advisors;
    friend class SubscribedPropagators;
  private:
    /// Is the advisor disposed?
    bool disposed(void) const;
    /// Static cast
    static Advisor* cast(ActorLink* al);
    /// Static cast
    static const Advisor* cast(const ActorLink* al);
  protected:
    /// Return the advisor's propagator
    Propagator& propagator(void) const;
  public:
    /// Constructor for creation
    template<class A>
    Advisor(Space& home, Propagator& p, Council<A>& c);
    /// Copying constructor
    Advisor(Space& home, Advisor& a);
    /// Provide access to view trace information
    const ViewTraceInfo& operator ()(const Space& home) const;

    /// \name Memory management
    //@{
    /// Dispose the advisor
    template<class A>
    void dispose(Space& home, Council<A>& c);
    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// No-op for exceptions
    static void  operator delete(void* p, Space& home);
    //@}
  private:
#ifndef __GNUC__
    /// Not used (uses dispose instead)
    static void  operator delete(void* p);
#endif
    /// Not used
    static void* operator new(size_t s);
  };


  /**
   * \brief No-good literal recorded during search
   *
   */
  class GECODE_VTABLE_EXPORT NGL {
  private:
    /// Combines pointer to next literal and mark whether it is a leaf
    void* nl;
  public:
    /// The status of a no-good literal
    enum Status {
      FAILED,   ///< The literal is failed
      SUBSUMED, ///< The literal is subsumed
      NONE      ///< The literal is neither failed nor subsumed
    };
    /// Constructor for creation
    NGL(void);
    /// Constructor for creation
    NGL(Space& home);
    /// Constructor for cloning \a ngl
    NGL(Space& home, NGL& ngl);
    /// Subscribe propagator \a p to all views of the no-good literal
    virtual void subscribe(Space& home, Propagator& p) = 0;
    /// Cancel propagator \a p from all views of the no-good literal
    virtual void cancel(Space& home, Propagator& p) = 0;
    /// Schedule propagator \a p for all views of the no-good literal
    virtual void reschedule(Space& home, Propagator& p) = 0;
    /// Test the status of the no-good literal
    virtual NGL::Status status(const Space& home) const = 0;
    /// Propagate the negation of the no-good literal
    virtual ExecStatus prune(Space& home) = 0;
    /// Create copy
    virtual NGL* copy(Space& home) = 0;
    /// Whether dispose must always be called (returns false)
    GECODE_KERNEL_EXPORT
    virtual bool notice(void) const;
    /// Dispose
    virtual size_t dispose(Space& home);
    /// \name Internal management routines
    //@{
    /// Test whether literal is a leaf
    bool leaf(void) const;
    /// Return pointer to next literal
    NGL* next(void) const;
    /// Mark literal as leaf or not
    void leaf(bool l);
    /// %Set pointer to next literal
    void next(NGL* n);
    /// Add node \a n and mark it as leaf \a l and return \a n
    NGL* add(NGL* n, bool l);
    //@}
    /// \name Memory management
    //@{
    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// Return memory to space
    static void  operator delete(void* s, Space& home);
    /// Needed for exceptions
    static void  operator delete(void* p);
    //@}
  public:
    /// To avoid warnings
    GECODE_KERNEL_EXPORT virtual ~NGL(void);
    /// Not used
    static void* operator new(size_t s);
  };

  /**
   * \brief %Choice for performing commit
   *
   * Must be refined by inheritance such that the information stored
   * inside a choice is sufficient to redo a commit performed by a
   * particular brancher.
   *
   * \ingroup TaskActor
   */
  class GECODE_VTABLE_EXPORT Choice : public HeapAllocated {
    friend class Space;
  private:
    unsigned int bid; ///< Identity to match creating brancher
    unsigned int alt; ///< Number of alternatives

    /// Return id of the creating brancher
    unsigned int id(void) const;
  protected:
    /// Initialize for particular brancher \a b and alternatives \a a
    Choice(const Brancher& b, const unsigned int a);
  public:
    /// Return number of alternatives
    unsigned int alternatives(void) const;
    /// Destructor
    GECODE_KERNEL_EXPORT virtual ~Choice(void);
    /// Archive into \a e
    GECODE_KERNEL_EXPORT
    virtual void archive(Archive& e) const;
  };

  /**
   * \brief Base-class for branchers
   *
   * Note that branchers cannot be created inside a propagator
   * (no idea why one would like to do that anyway). If you do that
   * the system will explode in a truly interesting way.
   *
   * \ingroup TaskActor
   */
  class GECODE_VTABLE_EXPORT Brancher : public Actor {
    friend class ActorLink;
    friend class Space;
    friend class Choice;
  private:
    /// Unique brancher identity
    unsigned int bid;
    /// Group id
    unsigned int gid;
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static Brancher* cast(ActorLink* al);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static const Brancher* cast(const ActorLink* al);
  protected:
    /// Constructor for creation
    Brancher(Home home);
    /// Constructor for cloning \a b
    Brancher(Space& home, Brancher& b);
  public:
    /// \name Brancher
    //@{
    /**
     * \brief Check status of brancher, return true if alternatives left
     *
     * This method is called when Space::status is called, it determines
     * whether to continue branching with this brancher or move on to
     * the (possibly) next brancher.
     *
     */
    virtual bool status(const Space& home) const = 0;
    /**
     * \brief Return choice
     *
     * Note that this method relies on the fact that it is called
     * immediately after a previous call to status. Moreover, the
     * member function can only be called once.
     */
    virtual const Choice* choice(Space& home) = 0;
    /// Return choice from \a e
    virtual const Choice* choice(const Space& home, Archive& e) = 0;
    /**
     * \brief Commit for choice \a c and alternative \a a
     *
     * The current brancher in the space \a home performs a commit from
     * the information provided by the choice \a c and the alternative \a a.
     */
    virtual ExecStatus commit(Space& home, const Choice& c,
                              unsigned int a) = 0;
    /**
     * \brief Create no-good literal for choice \a c and alternative \a a
     *
     * The current brancher in the space \a home creates a no-good literal
     * from the information provided by the choice \a c and the alternative
     * \a a. The brancher has the following options:
     *  - it returns NULL for all alternatives \a a. This means that the
     *    brancher does not support no-good literals (default).
     *  - it returns NULL for the last alternative \a a. This means that
     *    this alternative is equivalent to the negation of the disjunction
     *    of all other alternatives.
     *
     */
    GECODE_KERNEL_EXPORT
    virtual NGL* ngl(Space& home, const Choice& c, unsigned int a) const;
    /**
     * \brief Print branch for choice \a c and alternative \a a
     *
     * Prints an explanation of the alternative \a a of choice \a c
     * on the stream \a o.
     *
     */
    GECODE_KERNEL_EXPORT
    virtual void print(const Space& home, const Choice& c, unsigned int a,
                       std::ostream& o) const;
    //@}
    /// \name Id and group support
    //@{
    /// Return brancher id
    unsigned int id(void) const;
    /// Return group brancher belongs to
    BrancherGroup group(void) const;
    /// Add brancher to group \a g
    void group(BrancherGroup g);
    //@}
  };

  /**
   * \brief Local (space-shared) object
   *
   * Local objects must inherit from this base class.
   *
   */
  class LocalObject : public Actor {
    friend class ActorLink;
    friend class Space;
    friend class LocalHandle;
  protected:
    /// Constructor for creation
    LocalObject(Home home);
    /// Copy constructor
    LocalObject(Space& home, LocalObject& l);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static LocalObject* cast(ActorLink* al);
    /// Static cast for a non-null pointer (to give a hint to optimizer)
    static const LocalObject* cast(const ActorLink* al);
  private:
    /// Make copy and set forwarding pointer
    GECODE_KERNEL_EXPORT void fwdcopy(Space& home);
  public:
    /// Return forwarding pointer
    LocalObject* fwd(Space& home);
  };

  /**
   * \brief Handles for local (space-shared) objects
   *
   */
  class LocalHandle {
  private:
    /// The local object
    LocalObject* o;
  protected:
    /// Create local handle pointing to NULL object
    LocalHandle(void);
    /// Create local handle that points to local object \a lo
    LocalHandle(LocalObject* lo);
    /// Copy constructor
    LocalHandle(const LocalHandle& lh);
  public:
    /// Assignment operator
    LocalHandle& operator =(const LocalHandle& lh);
    /// Updating during cloning
    void update(Space& home, LocalHandle& lh);
    /// Destructor
    ~LocalHandle(void);
  protected:
    /// Access to the local object
    LocalObject* object(void) const;
    /// Modify local object
    void object(LocalObject* n);
  };


  /**
   * \brief No-goods recorded from restarts
   *
   */
  class GECODE_VTABLE_EXPORT NoGoods {
  protected:
    /// Number of no-goods
    unsigned long int n;
  public:
    /// Initialize
    NoGoods(void);
    /// Post no-goods
    GECODE_KERNEL_EXPORT
    virtual void post(Space& home) const;
    /// Return number of no-goods posted
    unsigned long int ng(void) const;
    /// %Set number of no-goods posted to \a n
    void ng(unsigned long int n);
    /// Destructor
    virtual ~NoGoods(void);
    /// Empty no-goods
    GECODE_KERNEL_EXPORT
    static NoGoods eng;
  };

  /**
   * \brief Information passed by meta search engines
   *
   */
  class GECODE_VTABLE_EXPORT MetaInfo {
  public:
    /// Which type of information is provided
    enum Type {
      /// Information is provided by a restart-based engine
      RESTART,
      /// Information is provided by a portfolio-based engine
      PORTFOLIO
    };
  protected:
    /// Type of information
    const Type t;
    /// \name Restart-based information
    //@{
    /// Number of restarts
    const unsigned long int r;
    /// Number of solutions since last restart
    const unsigned long int s;
    /// Number of failures since last restart
    const unsigned long int f;
    /// Last solution found
    const Space* l;
    /// No-goods from restart
    const NoGoods& ng;
    //@}
    /// \name Portfolio-based information
    //@{
    /// Number of asset in portfolio
    const unsigned int a;
    //@}
  public:
    /// \name Constructors depending on type of engine
    //@{
    /// Constructor for restart-based engine
    MetaInfo(unsigned long int r,
             unsigned long int s,
             unsigned long int f,
             const Space* l,
             NoGoods& ng);
    /// Constructor for portfolio-based engine
    MetaInfo(unsigned int a);
    //@}
    /// Return type of information
    Type type(void) const;
    /// \name Restart-based information
    //@{
    /// Return number of restarts
    unsigned long int restart(void) const;
    /// Return number of solutions since last restart
    unsigned long int solution(void) const;
    /// Return number of failures since last restart
    unsigned long int fail(void) const;
    /// Return last solution found (possibly NULL)
    const Space* last(void) const;
    /// Return no-goods recorded from restart
    const NoGoods& nogoods(void) const;
    //@}
    /// \name Portfolio-based information
    //@{
    /// Return number of asset in portfolio
    unsigned int asset(void) const;
    //@}
  };

  /**
   * \brief %Space status
   * \ingroup TaskSearch
   */
  enum SpaceStatus {
    SS_FAILED, ///< %Space is failed
    SS_SOLVED, ///< %Space is solved (no brancher left)
    SS_BRANCH  ///< %Space must be branched (at least one brancher left)
  };

  /**
   * \brief %Statistics for execution of status
   *
   */
  class StatusStatistics {
  public:
    /// Number of propagator executions
    unsigned long int propagate;
    /// Initialize
    StatusStatistics(void);
    /// Reset information
    void reset(void);
    /// Return sum with \a s
    StatusStatistics operator +(const StatusStatistics& s);
    /// Increment by statistics \a s
    StatusStatistics& operator +=(const StatusStatistics& s);
  };

  /**
   * \brief %Statistics for execution of clone
   *
   */
  class CloneStatistics {
  public:
    /// Initialize
    CloneStatistics(void);
    /// Reset information
    void reset(void);
    /// Return sum with \a s
    CloneStatistics operator +(const CloneStatistics& s);
    /// Increment by statistics \a s
    CloneStatistics& operator +=(const CloneStatistics& s);
  };

  /**
   * \brief %Statistics for execution of commit
   *
   */
  class CommitStatistics {
  public:
    /// Initialize
    CommitStatistics(void);
    /// Reset information
    void reset(void);
    /// Return sum with \a s
    CommitStatistics operator +(const CommitStatistics& s);
    /// Increment by statistics \a s
    CommitStatistics& operator +=(const CommitStatistics& s);
  };



  /**
   * \brief Computation spaces
   */
  class GECODE_VTABLE_EXPORT Space : public HeapAllocated {
    friend class Actor;
    friend class Propagator;
    friend class PropagatorGroup;
    friend class Propagators;
    friend class Brancher;
    friend class BrancherGroup;
    friend class Branchers;
    friend class Advisor;
    template <class A> friend class Council;
    template<class VIC> friend class VarImp;
    template<class VarImp> friend class VarImpDisposer;
    friend class LocalObject;
    friend class Region;
    friend class AFC;
    friend class PostInfo;
    friend GECODE_KERNEL_EXPORT
    void trace(Home home, TraceFilter tf, int te, Tracer& t);
  private:
    /// Shared data for space
    Kernel::SharedSpaceData ssd;
    /// Performs memory management for space
    Kernel::MemoryManager mm;
#ifdef GECODE_HAS_CBS
    /// Global counter for variable ids
    unsigned int var_id_counter;
#endif
    /// Doubly linked list of all propagators
    ActorLink pl;
    /// Doubly linked list of all branchers
    ActorLink bl;
    /**
     * \brief Points to the first brancher to be used for status
     *
     * If equal to &bl, no brancher does exist.
     */
    Brancher* b_status;
    /**
     * \brief Points to the first brancher to be used for commit
     *
     * Note that \a b_commit can point to an earlier brancher
     * than \a b_status. This reflects the fact that the earlier
     * brancher is already done (that is, status on that brancher
     * returns false) but there might be still choices
     * referring to the earlier brancher.
     *
     * If equal to &bl, no brancher does exist.
     */
    Brancher* b_commit;
    /// Find brancher with identity \a id
    Brancher* brancher(unsigned int id);

    /// Kill brancher \a b
    void kill(Brancher& b);
    /// Kill propagator \a p
    void kill(Propagator& p);

    /// Kill brancher with identity \a id
    GECODE_KERNEL_EXPORT
    void kill_brancher(unsigned int id);

    /// Reserved brancher id (never created)
    static const unsigned reserved_bid = 0U;

    /// Number of bits for status control
    static const unsigned int sc_bits = 2;
    /// No special features activated
    static const unsigned int sc_fast = 0;
    /// Disabled propagators are supported
    static const unsigned int sc_disabled = 1;
    /// Tracing is supported
    static const unsigned int sc_trace = 2;

    union {
      /// Data only available during propagation or branching
      struct {
        /**
         * \brief Cost level with next propagator to be executed
         *
         * This maintains the following invariant (but only if the
         * space does not perform propagation):
         *  - If active points to a queue, this queue might contain
         *    a propagator. However, there will be at least one queue
         *    containing a propagator.
         *  - Otherwise, active is smaller than the first queue
         *    or larger than the last queue. Then, the space is stable.
         *  - If active is larger than the last queue, the space is failed.
         */
        ActorLink* active;
        /// Scheduled propagators according to cost
        ActorLink queue[PropCost::AC_MAX+1];
        /**
         * \brief Id of next brancher to be created plus status control
         *
         * The last two bits are reserved for status control.
         *
         */
        unsigned int bid_sc;
        /// Number of subscriptions
        unsigned int n_sub;
        /// View trace information
        ViewTraceInfo vti;
      } p;
      /// Data available only during copying
      struct {
        /// Entries for updating variables
        VarImpBase* vars_u[AllVarConf::idx_c];
        /// Keep variables during copying without index structure
        VarImpBase* vars_noidx;
        /// Linked list of local objects
        LocalObject* local;
      } c;
    } pc;
    /// Put propagator \a p into right queue
    void enqueue(Propagator* p);
    /**
     * \name update, and dispose variables
     */
    //@{
#ifdef GECODE_HAS_VAR_DISPOSE
    /// Registered variable type disposers
    GECODE_KERNEL_EXPORT static VarImpDisposerBase* vd[AllVarConf::idx_d];
    /// Entries for disposing variables
    VarImpBase* _vars_d[AllVarConf::idx_d];
    /// Return reference to variables (dispose)
    template<class VIC> VarImpBase* vars_d(void) const;
    /// %Set reference to variables (dispose)
    template<class VIC> void vars_d(VarImpBase* x);
#endif
    /// Update all cloned variables
    void update(ActorLink** sub);
    //@}

    /// First actor for forced disposal
    Actor** d_fst;
    /// Current actor for forced disposal
    Actor** d_cur;
    /// Last actor for forced disposal
    Actor** d_lst;

    /// Used for default argument
    GECODE_KERNEL_EXPORT static StatusStatistics unused_status;
    /// Used for default argument
    GECODE_KERNEL_EXPORT static CloneStatistics unused_clone;
    /// Used for default argument
    GECODE_KERNEL_EXPORT static CommitStatistics unused_commit;

    /**
     * \brief Clone space
     *
     * Assumes that the space is stable and not failed. If the space is
     * failed, an exception of type SpaceFailed is thrown. If the space
     * is not stable, an exception of SpaceNotStable is thrown.
     *
     * Otherwise, a clone of the space is returned.
     *
     * Throws an exception of type SpaceNotCloned when the copy constructor
     * of the Space class is not invoked during cloning.
     *
     */
    GECODE_KERNEL_EXPORT Space* _clone(void);

    /**
     * \brief Commit choice \a c for alternative \a a
     *
     * The current brancher in the space performs a commit from
     * the information provided by the choice \a c
     * and the alternative \a a. The statistics information \a stat is
     * updated.
     *
     * Note that no propagation is perfomed (to support path
     * recomputation), in order to perform propagation the member
     * function status must be used.
     *
     * Committing with choices must be carried
     * out in the same order as the choices have been
     * obtained by the member function Space::choice().
     *
     * It is perfectly okay to add constraints interleaved with
     * choices (provided they are in the right order).
     * However, if propagation is performed by calling the member
     * function status and then new choices are
     * computed, these choices are different.
     *
     * Only choices can be used that are up-to-date in the following
     * sense: if a new choice is created (via the choice member
     * function), no older choices can be used.
     *
     * Committing throws the following exceptions:
     *  - SpaceNoBrancher, if the space has no current brancher (it is
     *    already solved).
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     */
    GECODE_KERNEL_EXPORT
    void _commit(const Choice& c, unsigned int a);

    /**
     * \brief Commit choice \a c for alternative \a a if possible
     *
     * The current brancher in the space performs a commit from
     * the information provided by the choice \a c
     * and the alternative \a a. The statistics information \a stat is
     * updated.
     *
     * Note that no propagation is perfomed (to support path
     * recomputation), in order to perform propagation the member
     * function status must be used.
     *
     * Committing with choices must be carried
     * out in the same order as the choices have been
     * obtained by the member function Space::choice().
     *
     * It is perfectly okay to add constraints interleaved with
     * choices (provided they are in the right order).
     * However, if propagation is performed by calling the member
     * function status and then new choices are
     * computed, these choices are different.
     *
     * Only choices can be used that are up-to-date in the following
     * sense: if a new choice is created (via the choice member
     * function), no older choices can be used.
     *
     * Committing throws the following exceptions:
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     */
    GECODE_KERNEL_EXPORT
    void _trycommit(const Choice& c, unsigned int a);

    /// Find trace recorder if exists
    GECODE_KERNEL_EXPORT
    TraceRecorder* findtracerecorder(void);
    /// Trace posting event
    GECODE_KERNEL_EXPORT
    void post(const PostInfo& pi);

    /**
     * \brief Notice that an actor must be disposed
     *
     * Note that \a a might be a marked pointer where the mark
     * indicates that \a a is a trace recorder.
     */
    GECODE_KERNEL_EXPORT
    void ap_notice_dispose(Actor* a, bool d);
    /**
     * \brief Ignore that an actor must be disposed
     *
     * Note that \a a might be a marked pointer where the mark
     * indicates that \a a is a trace recorder.
     */
    GECODE_KERNEL_EXPORT
    void ap_ignore_dispose(Actor* a, bool d);
  public:
    /**
     * \brief Default constructor
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT
    Space(void);
    /**
     * \brief Constructor for cloning
     *
     * Must copy and update all data structures (such as variables
     * and variable arrays) required by the subclass of Space.
     *
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT
    Space(Space& s);
    /**
     * \brief Destructor
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT
    virtual ~Space(void);
    /**
     * \brief Copying member function
     *
     * Must create a new object using the constructor for cloning.
     * \ingroup TaskModelScript
     */
    virtual Space* copy(void) = 0;
    /**
     * \brief Constrain function for best solution search
     *
     * Must constrain this space to be better than the so far best
     * solution \a best.
     *
     * The default function does nothing.
     *
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT virtual void constrain(const Space& best);
    /**
     * \brief Master configuration function for meta search engines
     *
     * This configuration function is used by both restart and
     * portfolio meta search engines.
     *
     * \li A restart meta search engine calls this function on its
     *     master space whenever it finds a solution or exploration
     *     restarts. \a mi contains information about the current restart.
     *
     *     If a solution has been found, then search will continue with
     *     a restart id the function returns true, otherwise search
     *     will continue.
     *
     *     The default function posts no-goods obtained from \a mi.
     *
     * \li A portfolio meta engine calls this function once on
     *     the master space. The return value is ignored.
     *
     *     The default function does nothing.
     *
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT
    virtual bool master(const MetaInfo& mi);
    /**
     * \brief Slave configuration function for meta search engines
     *
     * This configuration function is used by both restart and
     * portfolio meta search engines.
     *
     * \li A restart meta search engine calls this function on its
     *     slave space whenever it finds a solution or exploration
     *     restarts.  \a mi contains information about the current restart.
     *
     *     If the function returns true, the search on the slave space is
     *     considered complete, i.e., if it fails or exhaustively explores the
     *     entire search space, the meta search engine finishes. If the
     *     function returns false, the search on the slave space is considered
     *     incomplete, and the meta engine will restart the search regardless
     *     of whether the search on the slave space finishes or times out.
     *
     * \li A portfolio meta engine calls this function once on each asset
     *     (that is, on each slave) and passes the number of the asset,
     *     starting from zero.
     *
     * The default function does nothing and returns true.
     *
     * \ingroup TaskModelScript
     */
    GECODE_KERNEL_EXPORT
    virtual bool slave(const MetaInfo& mi);

    /*
     * Member functions for search engines
     *
     */

    /**
     * \brief Query space status
     *
     * Propagates the space until fixpoint or failure;
     * updates the statistics information \a stat; and:
     *  - if the space is failed, SpaceStatus::SS_FAILED is returned.
     *  - if the space is not failed but the space has no brancher left,
     *    SpaceStatus::SS_SOLVED is returned.
     *  - otherwise, SpaceStatus::SS_BRANCH is returned.
     * \ingroup TaskSearch
     */
    GECODE_KERNEL_EXPORT
    SpaceStatus status(StatusStatistics& stat=unused_status);

    /**
     * \brief Create new choice for current brancher
     *
     * This member function can only be called after the member function
     * Space::status on the same space has been called and in between
     * no non-const member function has been called on this space.
     *
     * Moreover, the member function can only be called at most once
     * (otherwise, it might generate conflicting choices).
     *
     * Note that the above invariant only pertains to calls of member
     * functions of the same space. If the invariant is violated, the
     * system is likely to crash (hopefully it does). In particular, if
     * applied to a space with no current brancher, the system will
     * crash.
     *
     * After a new choice has been created, no older choices
     * can be used on the space.
     *
     * If the status() member function has returned that the space has
     * no more branchers (that is, the result was either SS_FAILED or
     * SS_SOLVED), a call to choice() will return NULL and purge
     * all remaining branchers inside the space. This is interesting
     * for the case SS_SOLVED, where the call to choice can serve as
     * garbage collection.
     *
     * Throws an exception of type SpaceNotStable when applied to a not
     * yet stable space.
     *
     * \ingroup TaskSearch
     */
    GECODE_KERNEL_EXPORT
    const Choice* choice(void);

    /**
     * \brief Create new choice from \a e
     *
     * The archived representation \a e must have been created from
     * a Choice that is compatible with this space (i.e., created from
     * the same model).
     *
     * \ingroup TaskSearch
     */
    GECODE_KERNEL_EXPORT
    const Choice* choice(Archive& e) const;

    /**
     * \brief Clone space
     *
     * Assumes that the space is stable and not failed. If the space is
     * failed, an exception of type SpaceFailed is thrown. If the space
     * is not stable, an exception of SpaceNotStable is thrown.
     *
     * Otherwise, a clone of the space is returned and the statistics
     * information \a stat is updated.
     *
     * Throws an exception of type SpaceNotCloned when the copy constructor
     * of the Space class is not invoked during cloning.
     *
     * \ingroup TaskSearch
     */
    Space* clone(CloneStatistics& stat=unused_clone) const;

    /**
     * \brief Commit choice \a c for alternative \a a
     *
     * The current brancher in the space performs a commit from
     * the information provided by the choice \a c
     * and the alternative \a a. The statistics information \a stat is
     * updated.
     *
     * Note that no propagation is perfomed (to support path
     * recomputation), in order to perform propagation the member
     * function status must be used.
     *
     * Committing with choices must be carried
     * out in the same order as the choices have been
     * obtained by the member function Space::choice().
     *
     * It is perfectly okay to add constraints interleaved with
     * choices (provided they are in the right order).
     * However, if propagation is performed by calling the member
     * function status and then new choices are
     * computed, these choices are different.
     *
     * Only choices can be used that are up-to-date in the following
     * sense: if a new choice is created (via the choice member
     * function), no older choices can be used.
     *
     * Committing throws the following exceptions:
     *  - SpaceNoBrancher, if the space has no current brancher (it is
     *    already solved).
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     *
     * \ingroup TaskSearch
     */
    void commit(const Choice& c, unsigned int a,
                CommitStatistics& stat=unused_commit);
    /**
     * \brief If possible, commit choice \a c for alternative \a a
     *
     * The current brancher in the space performs a commit from
     * the information provided by the choice \a c
     * and the alternative \a a. The statistics information \a stat is
     * updated.
     *
     * Note that no propagation is perfomed (to support path
     * recomputation), in order to perform propagation the member
     * function status must be used.
     *
     * Committing with choices must be carried
     * out in the same order as the choices have been
     * obtained by the member function Space::choice().
     *
     * It is perfectly okay to add constraints interleaved with
     * choices (provided they are in the right order).
     * However, if propagation is performed by calling the member
     * function status and then new choices are
     * computed, these choices are different.
     *
     * Only choices can be used that are up-to-date in the following
     * sense: if a new choice is created (via the choice member
     * function), no older choices can be used.
     *
     * Committing throws the following exceptions:
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     *
     * \ingroup TaskSearch
     */
    void trycommit(const Choice& c, unsigned int a,
                   CommitStatistics& stat=unused_commit);
    /**
     * \brief Create no-good literal for choice \a c and alternative \a a
     *
     * The current brancher in the space \a home creates a no-good literal
     * from the information provided by the choice \a c and the alternative
     * \a a. The brancher has the following options:
     *  - it returns NULL for all alternatives \a a. This means that the
     *    brancher does not support no-good literals.
     *  - it returns NULL for the last alternative \a a. This means that
     *    this alternative is equivalent to the negation of the disjunction
     *    of all other alternatives.
     *
     * It throws the following exceptions:
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     *
     * \ingroup TaskSearch
     */
    GECODE_KERNEL_EXPORT
    NGL* ngl(const Choice& c, unsigned int a);

    /**
     * \brief Print branch for choice \a c and alternative \a a
     *
     * Prints an explanation of the alternative \a a of choice \a c
     * on the stream \a o.
     *
     * Print throws the following exceptions:
     *  - SpaceNoBrancher, if the space has no current brancher (it is
     *    already solved).
     *  - SpaceIllegalAlternative, if \a a is not smaller than the number
     *    of alternatives supported by the choice \a c.
     *
     * \ingroup TaskSearch
     */
    GECODE_KERNEL_EXPORT
    void print(const Choice& c, unsigned int a, std::ostream& o) const;

    /**
     * \brief Notice actor property
     *
     * Make the space notice that the actor \a a has the property \a p.
     * Note that the same property can only be noticed once for an actor
     * unless \a duplicate is true.
     * \ingroup TaskActor
     */
    GECODE_KERNEL_EXPORT
    void notice(Actor& a, ActorProperty p, bool duplicate=false);
    /**
     * \brief Ignore actor property
     *
     * Make the space ignore that the actor \a a has the property \a p.
     * Note that a property must be ignored before an actor is disposed.
     * \ingroup TaskActor
     */
    GECODE_KERNEL_EXPORT
    void ignore(Actor& a, ActorProperty p, bool duplicate=false);


    /**
     * \Brief %Propagator \a p is subsumed
     *
     * First disposes the propagator and then returns subsumption.
     *
     * \warning Has a side-effect on the propagator. Overwrites
     *          the modification event delta of a propagator.
     *          Use only directly with returning from propagation.
     * \ingroup TaskActorStatus
     */
    ExecStatus ES_SUBSUMED(Propagator& p);
    /**
     * \brief %Propagator \a p is subsumed
     *
     * The size of the propagator is \a s.
     *
     * Note that the propagator must be subsumed and also disposed. So
     * in general, there should be code such as
     * \code return ES_SUBSUMED_DISPOSE(home,*this,dispose(home)) \endcode.
     *
     * \warning Has a side-effect on the propagator. Overwrites
     *          the modification event delta of a propagator.
     *          Use only directly with returning from propagation.
     * \ingroup TaskActorStatus
     */
    ExecStatus ES_SUBSUMED_DISPOSED(Propagator& p, size_t s);
    /**
     * \brief %Propagator \a p has computed partial fixpoint
     *
     * %Set modification event delta to \a med and schedule propagator
     * accordingly.
     *
     * \warning Has a side-effect on the propagator.
     *          Use only directly with returning from propagation.
     * \ingroup TaskActorStatus
     */
    ExecStatus ES_FIX_PARTIAL(Propagator& p, const ModEventDelta& med);
    /**
     * \brief %Propagator \a p has not computed partial fixpoint
     *
     * Combine current modification event delta with \a and schedule
     * propagator accordingly.
     *
     * \warning Has a side-effect on the propagator.
     *          Use only directly with returning from propagation.
     * \ingroup TaskActorStatus
     */
    ExecStatus ES_NOFIX_PARTIAL(Propagator& p, const ModEventDelta& med);

    /**
     * \brief %Advisor \a a must be disposed
     *
     * Disposes the advisor and returns that the propagator of \a a
     * need not be run.
     *
     * \warning Has a side-effect on the advisor. Use only directly when
     *          returning from advise.
     * \ingroup TaskActorStatus
     */
    template<class A>
    ExecStatus ES_FIX_DISPOSE(Council<A>& c, A& a);
    /**
     * \brief %Advisor \a a must be disposed and its propagator must be run
     *
     * Disposes the advisor and returns that the propagator of \a a
     * must be run.
     *
     * \warning Has a side-effect on the advisor. Use only directly when
     *          returning from advise.
     * \ingroup TaskActorStatus
     */
    template<class A>
    ExecStatus ES_NOFIX_DISPOSE(Council<A>& c, A& a);
    /**
     * \brief %Advisor \a a must be disposed and its propagator must be forcefully rescheduled
     *
     * Disposes the advisor and returns that the propagator of \a a
     * must be run and must be forcefully rescheduled (including
     * recomputation of cost).
     *
     * \warning Has a side-effect on the advisor. Use only directly when
     *          returning from advise.
     * \ingroup TaskActorStatus
     */
    template<class A>
    ExecStatus ES_NOFIX_DISPOSE_FORCE(Council<A>& c, A& a);

    /**
     * \brief Fail space
     *
     * This is useful for failing outside of actors. Never use inside
     * a propagate or commit member function. The system will crash!
     * \ingroup TaskActor
     */
    void fail(void);
    /**
     * \brief Check whether space is failed
     *
     * Note that this does not perform propagation. This is useful
     * for posting actors: only if a space is not yet failed, new
     * actors are allowed to be created.
     * \ingroup TaskActor
     */
    bool failed(void) const;
    /**
     * \brief Return if space is stable (at fixpoint or failed)
     * \ingroup TaskActor
     */
    bool stable(void) const;

    /// \name Conversion from Space to Home
    //@{
    /// Return a home for this space with the information that \a p is being rewritten
    Home operator ()(Propagator& p);
    /// Return a home for this space with propagator group information \a pg
    Home operator ()(PropagatorGroup pg);
    /// Return a home for this space with brancher group information \a bg
    Home operator ()(BrancherGroup bg);
    //@}

    /**
     * \defgroup FuncMemSpace Space-memory management
     * \ingroup FuncMem
     */
    //@{
    /**
     * \brief Allocate block of \a n objects of type \a T from space heap
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(long unsigned int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from space heap
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(long int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from space heap
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(unsigned int n);
    /**
     * \brief Allocate block of \a n objects of type \a T from space heap
     *
     * Note that this function implements C++ semantics: the default
     * constructor of \a T is run for all \a n objects.
     */
    template<class T>
    T* alloc(int n);
    /**
     * \brief Delete \a n objects allocated from space heap starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, it is just scheduled for later
     * reusal.
     */
    template<class T>
    void free(T* b, long unsigned int n);
    /**
     * \brief Delete \a n objects allocated from space heap starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, it is just scheduled for later
     * reusal.
     */
    template<class T>
    void free(T* b, long int n);
    /**
     * \brief Delete \a n objects allocated from space heap starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, it is just scheduled for later
     * reusal.
     */
    template<class T>
    void free(T* b, unsigned int n);
    /**
     * \brief Delete \a n objects allocated from space heap starting at \a b
     *
     * Note that this function implements C++ semantics: the destructor
     * of \a T is run for all \a n objects.
     *
     * Note that the memory is not freed, it is just scheduled for later
     * reusal.
     */
    template<class T>
    void free(T* b, int n);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the space heap
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, long unsigned int n, long unsigned int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the space heap
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, long int n, long int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the space heap
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, unsigned int n, unsigned int m);
    /**
     * \brief Reallocate block of \a n objects starting at \a b to \a m objects of type \a T from the space heap
     *
     * Note that this function implements C++ semantics: the copy constructor
     * of \a T is run for all \f$\min(n,m)\f$ objects, the default
     * constructor of \a T is run for all remaining
     * \f$\max(n,m)-\min(n,m)\f$ objects, and the destrucor of \a T is
     * run for all \a n objects in \a b.
     *
     * Returns the address of the new block.
     */
    template<class T>
    T* realloc(T* b, int n, int m);
    /**
     * \brief Reallocate block of \a n pointers starting at \a b to \a m objects of type \a T* from the space heap
     *
     * Returns the address of the new block.
     *
     * This is a specialization for performance.
     */
    template<class T>
    T** realloc(T** b, long unsigned int n, long unsigned int m);
    /**
     * \brief Reallocate block of \a n pointers starting at \a b to \a m objects of type \a T* from the space heap
     *
     * Returns the address of the new block.
     *
     * This is a specialization for performance.
     */
    template<class T>
    T** realloc(T** b, long int n, long int m);
    /**
     * \brief Reallocate block of \a n pointers starting at \a b to \a m objects of type \a T* from the space heap
     *
     * Returns the address of the new block.
     *
     * This is a specialization for performance.
     */
    template<class T>
    T** realloc(T** b, unsigned int n, unsigned int m);
    /**
     * \brief Reallocate block of \a n pointers starting at \a b to \a m objects of type \a T* from the space heap
     *
     * Returns the address of the new block.
     *
     * This is a specialization for performance.
     */
    template<class T>
    T** realloc(T** b, int n, int m);
    /// Allocate memory on space heap
    void* ralloc(size_t s);
    /// Free memory previously allocated with alloc (might be reused later)
    void rfree(void* p, size_t s);
    /// Reallocate memory block starting at \a b from size \a n to size \a s
    void* rrealloc(void* b, size_t n, size_t m);
    /// Allocate from freelist-managed memory
    template<size_t> void* fl_alloc(void);
    /**
     * \brief Return freelist-managed memory to freelist
     *
     * The first list element to be retuned is \a f, the last is \a l.
     */
    template<size_t> void  fl_dispose(FreeList* f, FreeList* l);
    //@}
    /// Construction routines
    //@{
    /**
     * \brief Constructs a single object of type \a T from space heap using the default constructor.
     */
    template<class T>
    T& construct(void);
    /**
     * \brief Constructs a single object of type \a T from space heap using a unary constructor.
     *
     * The parameter is passed as a const reference.
     */
    template<class T, typename A1>
    T& construct(A1 const& a1);
    /**
     * \brief Constructs a single object of type \a T from space heap using a binary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2>
    T& construct(A1 const& a1, A2 const& a2);
    /**
     * \brief Constructs a single object of type \a T from space heap using a ternary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3);
    /**
     * \brief Constructs a single object of type \a T from space heap using a quaternary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3, typename A4>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4);
    /**
     * \brief Constructs a single object of type \a T from space heap using a quinary constructor.
     *
     * The parameters are passed as const references.
     */
    template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
    T& construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5);
    //@}

    /// \name Low-level support for AFC
    //@{
    /// %Set AFC decay factor to \a d
    void afc_decay(double d);
    /// Return AFC decay factor
    double afc_decay(void) const;
    /// Unshare AFC information for all propagators
    GECODE_KERNEL_EXPORT void afc_unshare(void);
    //@}

  protected:
    /**
     * \brief Class to iterate over propagators of a space
     *
     * Note that the iterator cannot be used during cloning.
     */
    class Propagators {
    private:
      /// Current space
      Space& home;
      /// Current queue
      ActorLink* q;
      /// Current propagator
      ActorLink* c;
      /// End mark
      ActorLink* e;
    public:
      /// Initialize
      Propagators(Space& home);
      /// Test whether there are propagators left
      bool operator ()(void) const;
      /// Move iterator to next propagator
      void operator ++(void);
      /// Return propagator
      Propagator& propagator(void) const;
    };
    /**
     * \brief Class to iterate over scheduled propagators of a space
     *
     * Note that the iterator cannot be used during cloning.
     */
    class ScheduledPropagators {
    private:
      /// current space
      Space& home;
      /// current queue
      ActorLink* q;
      /// current propagator
      ActorLink* c;
      /// end mark
      ActorLink* e;
    public:
      /// Initialize
      ScheduledPropagators(Space& home);
      /// Test whether there are propagators left
      bool operator ()(void) const;
      /// Move iterator to next propagator
      void operator ++(void);
      /// Return propagator
      Propagator& propagator(void) const;
    };
    /**
     * \brief Class to iterate over idle propagators of a space
     *
     * Note that the iterator cannot be used during cloning.
     */
    class IdlePropagators {
    private:
      /// Current propagator
      ActorLink* c;
      /// End mark
      ActorLink* e;
    public:
      /// Initialize
      IdlePropagators(Space& home);
      /// Test whether there are propagators left
      bool operator ()(void) const;
      /// Move iterator to next propagator
      void operator ++(void);
      /// Return propagator
      Propagator& propagator(void) const;
    };
    /**
     * \brief Class to iterate over branchers of a space
     *
     * Note that the iterator cannot be used during cloning.
     */
    class Branchers {
    private:
      /// current brancher
      ActorLink* c;
      /// end mark
      ActorLink* e;
    public:
      /// Initialize
      Branchers(Space& home);
      /// Test whether there are branchers left
      bool operator ()(void) const;
      /// Move iterator to next brancher
      void operator ++(void);
      /// Return propagator
      Brancher& brancher(void) const;
    };
  };

  /// Class to iterate over propagators in a group
  class Propagators {
  private:
    /// Propagators
    Space::Propagators ps;
    /// Current group
    PropagatorGroup g;
  public:
    /// Initialize
    Propagators(const Space& home, PropagatorGroup g);
    /// Test whether there are propagators left
    bool operator ()(void) const;
    /// Move iterator to next propagator
    void operator ++(void);
    /// Return propagator
    const Propagator& propagator(void) const;
  };

  /// Class to iterate over branchers in a group
  class Branchers {
  private:
    /// Branchers
    Space::Branchers bs;
    /// Current group
    BrancherGroup g;
  public:
    /// Initialize
    Branchers(const Space& home, BrancherGroup g);
    /// Test whether there are branchers left
    bool operator ()(void) const;
    /// Move iterator to next brancher
    void operator ++(void);
    /// Return propagator
    const Brancher& brancher(void) const;
  };




  /*
   * Memory management
   *
   */

  // Space allocation: general space heaps and free lists
  forceinline void*
  Space::ralloc(size_t s) {
    return mm.alloc(ssd.data().sm,s);
  }
  forceinline void
  Space::rfree(void* p, size_t s) {
    return mm.reuse(p,s);
  }
  forceinline void*
  Space::rrealloc(void* _b, size_t n, size_t m) {
    char* b = static_cast<char*>(_b);
    if (n < m) {
      char* p = static_cast<char*>(ralloc(m));
      memcpy(p,b,n);
      rfree(b,n);
      return p;
    } else {
      rfree(b+m,m-n);
      return b;
    }
  }

  template<size_t s>
  forceinline void*
  Space::fl_alloc(void) {
    return mm.template fl_alloc<s>(ssd.data().sm);
  }
  template<size_t s>
  forceinline void
  Space::fl_dispose(FreeList* f, FreeList* l) {
    mm.template fl_dispose<s>(f,l);
  }

  /*
   * Typed allocation routines
   *
   */
  template<class T>
  forceinline T*
  Space::alloc(long unsigned int n) {
    T* p = static_cast<T*>(ralloc(sizeof(T)*n));
    for (long unsigned int i=0; i<n; i++)
      (void) new (p+i) T();
    return p;
  }
  template<class T>
  forceinline T*
  Space::alloc(long int n) {
    assert(n >= 0);
    return alloc<T>(static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline T*
  Space::alloc(unsigned int n) {
    return alloc<T>(static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline T*
  Space::alloc(int n) {
    assert(n >= 0);
    return alloc<T>(static_cast<long unsigned int>(n));
  }

  template<class T>
  forceinline void
  Space::free(T* b, long unsigned int n) {
    for (long unsigned int i=0; i<n; i++)
      b[i].~T();
    rfree(b,n*sizeof(T));
  }
  template<class T>
  forceinline void
  Space::free(T* b, long int n) {
    assert(n >= 0);
    free<T>(b,static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline void
  Space::free(T* b, unsigned int n) {
    free<T>(b,static_cast<long unsigned int>(n));
  }
  template<class T>
  forceinline void
  Space::free(T* b, int n) {
    assert(n >= 0);
    free<T>(b,static_cast<long unsigned int>(n));
  }

  template<class T>
  forceinline T*
  Space::realloc(T* b, long unsigned int n, long unsigned int m) {
    if (n < m) {
      T* p = static_cast<T*>(ralloc(sizeof(T)*m));
      for (long unsigned int i=0; i<n; i++)
        (void) new (p+i) T(b[i]);
      for (long unsigned int i=n; i<m; i++)
        (void) new (p+i) T();
      free<T>(b,n);
      return p;
    } else {
      free<T>(b+m,m-n);
      return b;
    }
  }
  template<class T>
  forceinline T*
  Space::realloc(T* b, long int n, long int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T*
  Space::realloc(T* b, unsigned int n, unsigned int m) {
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T*
  Space::realloc(T* b, int n, int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T>(b,static_cast<long unsigned int>(n),
                      static_cast<long unsigned int>(m));
  }

#define GECODE_KERNEL_REALLOC(T)                                        \
  template<>                                                            \
  forceinline T*                                                        \
  Space::realloc<T>(T* b, long unsigned int n, long unsigned int m) {   \
    return static_cast<T*>(rrealloc(b,n*sizeof(T),m*sizeof(T)));        \
  }                                                                     \
  template<>                                                            \
  forceinline T*                                                        \
  Space::realloc<T>(T* b, long int n, long int m) {                     \
    assert((n >= 0) && (m >= 0));                                       \
    return realloc<T>(b,static_cast<long unsigned int>(n),              \
                      static_cast<long unsigned int>(m));               \
  }                                                                     \
  template<>                                                            \
  forceinline T*                                                        \
  Space::realloc<T>(T* b, unsigned int n, unsigned int m) {             \
    return realloc<T>(b,static_cast<long unsigned int>(n),              \
                      static_cast<long unsigned int>(m));               \
  }                                                                     \
  template<>                                                            \
  forceinline T*                                                        \
  Space::realloc<T>(T* b, int n, int m) {                               \
    assert((n >= 0) && (m >= 0));                                       \
    return realloc<T>(b,static_cast<long unsigned int>(n),              \
                      static_cast<long unsigned int>(m));               \
  }

  GECODE_KERNEL_REALLOC(bool)
  GECODE_KERNEL_REALLOC(signed char)
  GECODE_KERNEL_REALLOC(unsigned char)
  GECODE_KERNEL_REALLOC(signed short int)
  GECODE_KERNEL_REALLOC(unsigned short int)
  GECODE_KERNEL_REALLOC(signed int)
  GECODE_KERNEL_REALLOC(unsigned int)
  GECODE_KERNEL_REALLOC(signed long int)
  GECODE_KERNEL_REALLOC(unsigned long int)
  GECODE_KERNEL_REALLOC(float)
  GECODE_KERNEL_REALLOC(double)

#undef GECODE_KERNEL_REALLOC

  template<class T>
  forceinline T**
  Space::realloc(T** b, long unsigned int n, long unsigned int m) {
    return static_cast<T**>(rrealloc(b,n*sizeof(T),m*sizeof(T*)));
  }
  template<class T>
  forceinline T**
  Space::realloc(T** b, long int n, long int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T*>(b,static_cast<long unsigned int>(n),
                       static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T**
  Space::realloc(T** b, unsigned int n, unsigned int m) {
    return realloc<T*>(b,static_cast<long unsigned int>(n),
                       static_cast<long unsigned int>(m));
  }
  template<class T>
  forceinline T**
  Space::realloc(T** b, int n, int m) {
    assert((n >= 0) && (m >= 0));
    return realloc<T*>(b,static_cast<long unsigned int>(n),
                       static_cast<long unsigned int>(m));
  }


#ifdef GECODE_HAS_VAR_DISPOSE
  template<class VIC>
  forceinline VarImpBase*
  Space::vars_d(void) const {
    return _vars_d[VIC::idx_d];
  }
  template<class VIC>
  forceinline void
  Space::vars_d(VarImpBase* x) {
    _vars_d[VIC::idx_d] = x;
  }
#endif

  // Space allocated entities: Actors, variable implementations, and advisors
  forceinline void
  Actor::operator delete(void*) {}
  forceinline void
  Actor::operator delete(void*, Space&) {}
  forceinline void*
  Actor::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::operator delete(void*) {}
  template<class VIC>
  forceinline void
  VarImp<VIC>::operator delete(void*, Space&) {}
  template<class VIC>
  forceinline void*
  VarImp<VIC>::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }

#ifndef __GNUC__
  forceinline void
  Advisor::operator delete(void*) {}
#endif
  forceinline void
  Advisor::operator delete(void*, Space&) {}
  forceinline void*
  Advisor::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }

  forceinline void
  NGL::operator delete(void*) {}
  forceinline void
  NGL::operator delete(void*, Space&) {}
  forceinline void*
  NGL::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }


  /*
   * No-goods
   *
   */
  forceinline
  NoGoods::NoGoods(void)
    : n(0) {}
  forceinline unsigned long int
  NoGoods::ng(void) const {
    return n;
  }
  forceinline void
  NoGoods::ng(unsigned long int n0) {
    n=n0;
  }
  forceinline
  NoGoods::~NoGoods(void) {}


  /*
   * Information from meta search engines
   */
  forceinline
  MetaInfo::MetaInfo(unsigned long int r0,
                     unsigned long int s0,
                     unsigned long int f0,
                     const Space* l0,
                     NoGoods& ng0)
    : t(RESTART), r(r0), s(s0), f(f0), l(l0), ng(ng0), a(0) {}

  forceinline
  MetaInfo::MetaInfo(unsigned int a0)
    : t(PORTFOLIO), r(0), s(0), f(0), l(NULL), ng(NoGoods::eng), a(a0) {}

  forceinline MetaInfo::Type
  MetaInfo::type(void) const {
    return t;
  }
  forceinline unsigned long int
  MetaInfo::restart(void) const {
    assert(type() == RESTART);
    return r;
  }
  forceinline unsigned long int
  MetaInfo::solution(void) const {
    assert(type() == RESTART);
    return s;
  }
  forceinline unsigned long int
  MetaInfo::fail(void) const {
    assert(type() == RESTART);
    return f;
  }
  forceinline const Space*
  MetaInfo::last(void) const {
    assert(type() == RESTART);
    return l;
  }
  forceinline const NoGoods&
  MetaInfo::nogoods(void) const {
    assert(type() == RESTART);
    return ng;
  }
  forceinline unsigned int
  MetaInfo::asset(void) const {
    assert(type() == PORTFOLIO);
    return a;
  }



  /*
   * ActorLink
   *
   */
  forceinline ActorLink*
  ActorLink::prev(void) const {
    return _prev;
  }

  forceinline ActorLink*
  ActorLink::next(void) const {
    return _next;
  }

  forceinline ActorLink**
  ActorLink::next_ref(void) {
    return &_next;
  }

  forceinline void
  ActorLink::prev(ActorLink* al) {
    _prev = al;
  }

  forceinline void
  ActorLink::next(ActorLink* al) {
    _next = al;
  }

  forceinline void
  ActorLink::unlink(void) {
    ActorLink* p = _prev; ActorLink* n = _next;
    p->_next = n; n->_prev = p;
  }

  forceinline void
  ActorLink::init(void) {
    _next = this; _prev =this;
  }

  forceinline void
  ActorLink::head(ActorLink* a) {
    // Inserts al at head of link-chain (that is, after this)
    ActorLink* n = _next;
    this->_next = a; a->_prev = this;
    a->_next = n; n->_prev = a;
  }

  forceinline void
  ActorLink::tail(ActorLink* a) {
    // Inserts al at tail of link-chain (that is, before this)
    ActorLink* p = _prev;
    a->_next = this; this->_prev = a;
    p->_next = a; a->_prev = p;
  }

  forceinline bool
  ActorLink::empty(void) const {
    return _next == this;
  }

  template<class T>
  forceinline ActorLink*
  ActorLink::cast(T* a) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(a);
    ActorLink& t = *a;
    return static_cast<ActorLink*>(&t);
  }

  template<class T>
  forceinline const ActorLink*
  ActorLink::cast(const T* a) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(a);
    const ActorLink& t = *a;
    return static_cast<const ActorLink*>(&t);
  }


  /*
   * Actor
   *
   */
  forceinline Actor*
  Actor::cast(ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    ActorLink& t = *al;
    return static_cast<Actor*>(&t);
  }

  forceinline const Actor*
  Actor::cast(const ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    const ActorLink& t = *al;
    return static_cast<const Actor*>(&t);
  }

  forceinline void
  Home::notice(Actor& a, ActorProperty p, bool duplicate) {
    s.notice(a,p,duplicate);
  }

  forceinline Space*
  Space::clone(CloneStatistics&) const {
    // Clone is only const for search engines. During cloning, several data
    // structures are updated (e.g. forwarding pointers), so we have to
    // cast away the constness.
    return const_cast<Space*>(this)->_clone();
  }

  forceinline void
  Space::commit(const Choice& c, unsigned int a, CommitStatistics&) {
    _commit(c,a);
  }

  forceinline void
  Space::trycommit(const Choice& c, unsigned int a, CommitStatistics&) {
    _trycommit(c,a);
  }

  forceinline double
  Space::afc_decay(void) const {
    return ssd.data().gpi.decay();
  }

  forceinline void
  Space::afc_decay(double d) {
    ssd.data().gpi.decay(d);
  }

  forceinline size_t
  Actor::dispose(Space&) {
    return sizeof(*this);
  }


  /*
   * Home for posting actors
   *
   */
  forceinline
  Home::Home(Space& s0, Propagator* p0,
             PropagatorGroup pg0, BrancherGroup bg0)
    : s(s0), p(p0), pg(pg0), bg(bg0) {}
  forceinline Home&
  Home::operator =(const Home& h) {
    s=h.s; p=h.p; pg=h.pg; bg=h.bg;
    return *this;
  }
  forceinline
  Home::operator Space&(void) {
    return s;
  }
  forceinline Home
  Home::operator ()(Propagator& p) {
    return Home(s,&p);
  }
  forceinline Home
  Home::operator ()(PropagatorGroup pg) {
    return Home(s,NULL,pg,BrancherGroup::def);
  }
  forceinline Home
  Home::operator ()(BrancherGroup bg) {
    return Home(s,NULL,PropagatorGroup::def,bg);
  }
  forceinline Home
  Space::operator ()(Propagator& p) {
    return Home(*this,&p);
  }
  forceinline Home
  Space::operator ()(PropagatorGroup pg) {
    return Home(*this,NULL,pg,BrancherGroup::def);
  }
  forceinline Home
  Space::operator ()(BrancherGroup bg) {
    return Home(*this,NULL,PropagatorGroup::def,bg);
  }
  forceinline Propagator*
  Home::propagator(void) const {
    return p;
  }
  forceinline PropagatorGroup
  Home::propagatorgroup(void) const {
    return pg;
  }
  forceinline BrancherGroup
  Home::branchergroup(void) const {
    return bg;
  }

  /*
   * View trace information
   *
   */
  forceinline void
  ViewTraceInfo::propagator(Propagator& p) {
    who = reinterpret_cast<ptrdiff_t>(&p) | PROPAGATOR;
  }
  forceinline void
  ViewTraceInfo::brancher(Brancher& b) {
    who = reinterpret_cast<ptrdiff_t>(&b) | BRANCHER;
  }
  forceinline void
  ViewTraceInfo::post(PropagatorGroup g) {
    who = (g.id() << 2) | POST;
  }
  forceinline void
  ViewTraceInfo::other(void) {
    who = OTHER;
  }
  forceinline ViewTraceInfo::What
  ViewTraceInfo::what(void) const {
    return static_cast<What>(who & 3);
  }
  forceinline const Propagator&
  ViewTraceInfo::propagator(void) const {
    assert(what() == PROPAGATOR);
    // Because PROPAGATOR == 0
    return *reinterpret_cast<Propagator*>(who);
  }
  forceinline const Brancher&
  ViewTraceInfo::brancher(void) const {
    assert(what() == BRANCHER);
    return *reinterpret_cast<Brancher*>(who & ~3);
  }
  forceinline PropagatorGroup
  ViewTraceInfo::post(void) const {
    assert(what() == POST);
    return PropagatorGroup(static_cast<unsigned int>(who >> 2));
  }

  /*
   * Post information
   */
  forceinline
  PostInfo::PostInfo(Home home)
    : h(home), pg(home.propagatorgroup()),
      pid(h.ssd.data().gpi.pid()),
      nested(h.pc.p.vti.what() != ViewTraceInfo::OTHER) {
    h.pc.p.vti.post(pg);
  }

  forceinline
  PostInfo::~PostInfo(void) {
    if (!nested) {
      if (h.pc.p.bid_sc & Space::sc_trace)
        h.post(*this);
      h.pc.p.vti.other();
    }
  }


  /*
   * Propagate trace information
   *
   */
  forceinline
  PropagateTraceInfo::PropagateTraceInfo(unsigned int i0, PropagatorGroup g0,
                                         const Propagator* p0, Status s0)
    : i(i0), g(g0), p(p0), s(s0) {}
  forceinline unsigned int
  PropagateTraceInfo::id(void) const {
    return i;
  }
  forceinline PropagatorGroup
  PropagateTraceInfo::group(void) const {
    return g;
  }
  forceinline const Propagator*
  PropagateTraceInfo::propagator(void) const {
    return p;
  }
  forceinline PropagateTraceInfo::Status
  PropagateTraceInfo::status(void) const {
    return s;
  }


  /*
   * Commit trace information
   *
   */
  forceinline
  CommitTraceInfo::CommitTraceInfo(const Brancher& b0, const Choice& c0,
                                   unsigned int a0)
    : b(b0), c(c0), a(a0) {}
  forceinline unsigned int
  CommitTraceInfo::id(void) const {
    return b.id();
  }
  forceinline BrancherGroup
  CommitTraceInfo::group(void) const {
    return b.group();
  }
  forceinline const Brancher&
  CommitTraceInfo::brancher(void) const {
    return b;
  }
  forceinline const Choice&
  CommitTraceInfo::choice(void) const {
    return c;
  }
  forceinline unsigned int
  CommitTraceInfo::alternative(void) const {
    return a;
  }


  /*
   * Post trace information
   *
   */
  forceinline
  PostTraceInfo::PostTraceInfo(PropagatorGroup g0, Status s0, unsigned int n0)
    : g(g0), s(s0), n(n0) {}
  forceinline PropagatorGroup
  PostTraceInfo::group(void) const {
    return g;
  }
  forceinline PostTraceInfo::Status
  PostTraceInfo::status(void) const {
    return s;
  }
  forceinline unsigned int
  PostTraceInfo::propagators(void) const {
    return n;
  }


  /*
   * Propagator
   *
   */
  forceinline Propagator*
  Propagator::cast(ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    ActorLink& t = *al;
    return static_cast<Propagator*>(&t);
  }

  forceinline const Propagator*
  Propagator::cast(const ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    const ActorLink& t = *al;
    return static_cast<const Propagator*>(&t);
  }

  forceinline Propagator*
  Propagator::fwd(void) const {
    return static_cast<Propagator*>(prev());
  }

  forceinline bool
  Propagator::disabled(void) const {
    return Support::marked(gpi_disabled);
  }

  forceinline void
  Propagator::disable(Space& home) {
    home.pc.p.bid_sc |= Space::sc_disabled;
    gpi_disabled = Support::fmark(gpi_disabled);
  }

  forceinline void
  Propagator::enable(Space& home) {
    (void) home;
    gpi_disabled = Support::funmark(gpi_disabled);
  }

  forceinline Kernel::GPI::Info&
  Propagator::gpi(void) {
    return *static_cast<Kernel::GPI::Info*>(Support::funmark(gpi_disabled));
  }

  forceinline
  Propagator::Propagator(Home home)
    : gpi_disabled((home.propagator() != NULL) ?
                   // Inherit propagator information
                   home.propagator()->gpi_disabled :
                   // New propagator information
                   static_cast<Space&>(home).ssd.data().gpi.allocate
                   (home.propagatorgroup().gid)) {
    u.advisors = NULL;
    assert((u.med == 0) && (u.size == 0));
    static_cast<Space&>(home).pl.head(this);
  }

  forceinline
  Propagator::Propagator(Space&, Propagator& p)
    : gpi_disabled(p.gpi_disabled) {
    u.advisors = NULL;
    assert((u.med == 0) && (u.size == 0));
    // Set forwarding pointer
    p.prev(this);
  }

  forceinline ModEventDelta
  Propagator::modeventdelta(void) const {
    return u.med;
  }

  forceinline double
  Propagator::afc(void) const {
    return const_cast<Propagator&>(*this).gpi().afc;
  }

#ifdef GECODE_HAS_CBS
  forceinline void
  Propagator::solndistrib(Space&, SendMarginal) const {}

  forceinline void
  Propagator::domainsizesum(InDecision, unsigned int& size,
                                  unsigned int& size_b) const {
    size = 0;
    size_b = 0;
  }
#endif

  forceinline unsigned int
  Propagator::id(void) const {
    return const_cast<Propagator&>(*this).gpi().pid;
  }

  forceinline PropagatorGroup
  Propagator::group(void) const {
    return PropagatorGroup(const_cast<Propagator&>(*this).gpi().gid);
  }

  forceinline void
  Propagator::group(PropagatorGroup g) {
    gpi().gid = g.id();
  }

  forceinline ExecStatus
  Space::ES_SUBSUMED_DISPOSED(Propagator& p, size_t s) {
    p.u.size = s;
    return __ES_SUBSUMED;
  }

  forceinline ExecStatus
  Space::ES_SUBSUMED(Propagator& p) {
    p.u.size = p.dispose(*this);
    return __ES_SUBSUMED;
  }

  forceinline ExecStatus
  Space::ES_FIX_PARTIAL(Propagator& p, const ModEventDelta& med) {
    p.u.med = med;
    assert(p.u.med != 0);
    return __ES_PARTIAL;
  }

  forceinline ExecStatus
  Space::ES_NOFIX_PARTIAL(Propagator& p, const ModEventDelta& med) {
    p.u.med = AllVarConf::med_combine(p.u.med,med);
    assert(p.u.med != 0);
    return __ES_PARTIAL;
  }



  /*
   * Brancher
   *
   */
  forceinline Brancher*
  Brancher::cast(ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    ActorLink& t = *al;
    return static_cast<Brancher*>(&t);
  }

  forceinline const Brancher*
  Brancher::cast(const ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    const ActorLink& t = *al;
    return static_cast<const Brancher*>(&t);
  }

  forceinline
  Brancher::Brancher(Home _home) :
    gid(_home.branchergroup().gid) {
    Space& home = static_cast<Space&>(_home);
    bid = home.pc.p.bid_sc >> Space::sc_bits;
    home.pc.p.bid_sc += (1 << Space::sc_bits);
    if ((home.pc.p.bid_sc >> Space::sc_bits) == 0U)
      throw TooManyBranchers("Brancher::Brancher");
    // If no brancher available, make it the first one
    if (home.b_status == &static_cast<Space&>(home).bl) {
      home.b_status = this;
      if (home.b_commit == &static_cast<Space&>(home).bl)
        home.b_commit = this;
    }
    home.bl.tail(this);
  }

  forceinline
  Brancher::Brancher(Space&, Brancher& b)
    : bid(b.bid), gid(b.gid) {
    // Set forwarding pointer
    b.prev(this);
  }

  forceinline unsigned int
  Brancher::id(void) const {
    return bid;
  }

  forceinline BrancherGroup
  Brancher::group(void) const {
    return BrancherGroup(gid);
  }

  forceinline void
  Brancher::group(BrancherGroup g) {
    gid = g.id();
  }


  forceinline void
  Space::kill(Brancher& b) {
    assert(!failed());
    // Make sure that neither b_status nor b_commit does not point to b!
    if (b_commit == &b)
      b_commit = Brancher::cast(b.next());
    if (b_status == &b)
      b_status = Brancher::cast(b.next());
    b.unlink();
    rfree(&b,b.dispose(*this));
  }

  forceinline void
  Space::kill(Propagator& p) {
    assert(!failed());
    p.unlink();
    rfree(&p,p.dispose(*this));
  }

  forceinline Brancher*
  Space::brancher(unsigned int id) {
    /*
     * Due to weakly monotonic propagators the following scenario might
     * occur: a brancher has been committed with all its available
     * choices. Then, propagation determines less information
     * than before and the brancher now will create new choices.
     * Later, during recomputation, all of these choices
     * can be used together, possibly interleaved with
     * choices for other branchers. That means all branchers
     * must be scanned to find the matching brancher for the choice.
     *
     * b_commit tries to optimize scanning as it is most likely that
     * recomputation does not generate new choices during recomputation
     * and hence b_commit is moved from newer to older branchers.
     */
    Brancher* b_old = b_commit;
    // Try whether we are lucky
    while (b_commit != Brancher::cast(&bl))
      if (id != b_commit->id())
        b_commit = Brancher::cast(b_commit->next());
      else
        return b_commit;
    if (b_commit == Brancher::cast(&bl)) {
      // We did not find the brancher, start at the beginning
      b_commit = Brancher::cast(bl.next());
      while (b_commit != b_old)
        if (id != b_commit->id())
          b_commit = Brancher::cast(b_commit->next());
        else
          return b_commit;
    }
    return NULL;
  }


  /*
   * Local objects
   *
   */

  forceinline LocalObject*
  LocalObject::cast(ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    ActorLink& t = *al;
    return static_cast<LocalObject*>(&t);
  }

  forceinline const LocalObject*
  LocalObject::cast(const ActorLink* al) {
    // Turning al into a reference is for gcc, assume is for MSVC
    GECODE_NOT_NULL(al);
    const ActorLink& t = *al;
    return static_cast<const LocalObject*>(&t);
  }

  forceinline
  LocalObject::LocalObject(Home home) {
    (void) home;
    ActorLink::cast(this)->prev(NULL);
  }

  forceinline
  LocalObject::LocalObject(Space&, LocalObject&) {
    ActorLink::cast(this)->prev(NULL);
  }

  forceinline LocalObject*
  LocalObject::fwd(Space& home) {
    if (prev() == NULL)
      fwdcopy(home);
    return LocalObject::cast(prev());
  }

  forceinline
  LocalHandle::LocalHandle(void) : o(NULL) {}
  forceinline
  LocalHandle::LocalHandle(LocalObject* lo) : o(lo) {}
  forceinline
  LocalHandle::LocalHandle(const LocalHandle& lh) : o(lh.o) {}
  forceinline LocalHandle&
  LocalHandle::operator =(const LocalHandle& lh) {
    o = lh.o;
    return *this;
  }
  forceinline
  LocalHandle::~LocalHandle(void) {}
  forceinline LocalObject*
  LocalHandle::object(void) const { return o; }
  forceinline void
  LocalHandle::object(LocalObject* n) { o = n; }
  forceinline void
  LocalHandle::update(Space& home, LocalHandle& lh) {
    object(lh.object()->fwd(home));
  }


  /*
   * Choices
   *
   */
  forceinline
  Choice::Choice(const Brancher& b, const unsigned int a)
    : bid(b.id()), alt(a) {}

  forceinline unsigned int
  Choice::alternatives(void) const {
    return alt;
  }

  forceinline unsigned int
  Choice::id(void) const {
    return bid;
  }

  forceinline
  Choice::~Choice(void) {}



  /*
   * No-good literal
   *
   */
  forceinline bool
  NGL::leaf(void) const {
    return Support::marked(nl);
  }
  forceinline NGL*
  NGL::next(void) const {
    return static_cast<NGL*>(Support::funmark(nl));
  }
  forceinline void
  NGL::leaf(bool l) {
    nl = l ? Support::fmark(nl) : Support::funmark(nl);
  }
  forceinline void
  NGL::next(NGL* n) {
    nl = Support::marked(nl) ? Support::mark(n) : n;
  }
  forceinline NGL*
  NGL::add(NGL* n, bool l) {
    nl = Support::marked(nl) ? Support::mark(n) : n;
    n->leaf(l);
    return n;
  }

  forceinline
  NGL::NGL(void)
    : nl(NULL) {}
  forceinline
  NGL::NGL(Space&)
    : nl(NULL) {}
  forceinline
  NGL::NGL(Space&, NGL&)
    : nl(NULL) {}
  forceinline size_t
  NGL::dispose(Space&) {
    return sizeof(*this);
  }

  /*
   * Advisor
   *
   */
  template<class A>
  forceinline
  Advisor::Advisor(Space&, Propagator& p, Council<A>& c) {
    // Store propagator and forwarding in prev()
    ActorLink::prev(&p);
    // Link to next advisor in next()
    ActorLink::next(c.advisors); c.advisors = static_cast<A*>(this);
  }

  forceinline
  Advisor::Advisor(Space&, Advisor&) {}

  forceinline bool
  Advisor::disposed(void) const {
    return prev() == NULL;
  }

  forceinline Advisor*
  Advisor::cast(ActorLink* al) {
    return static_cast<Advisor*>(al);
  }

  forceinline const Advisor*
  Advisor::cast(const ActorLink* al) {
    return static_cast<const Advisor*>(al);
  }

  forceinline Propagator&
  Advisor::propagator(void) const {
    assert(!disposed());
    return *Propagator::cast(ActorLink::prev());
  }

  template<class A>
  forceinline void
  Advisor::dispose(Space&,Council<A>&) {
    assert(!disposed());
    ActorLink::prev(NULL);
    // Shorten chains of disposed advisors by one, if possible
    Advisor* n = Advisor::cast(next());
    if ((n != NULL) && n->disposed())
      next(n->next());
  }

  forceinline const ViewTraceInfo&
  Advisor::operator ()(const Space& home) const {
    return home.pc.p.vti;
  }

  template<class A>
  forceinline ExecStatus
  Space::ES_FIX_DISPOSE(Council<A>& c, A& a) {
    a.dispose(*this,c);
    return ES_FIX;
  }

  template<class A>
  forceinline ExecStatus
  Space::ES_NOFIX_DISPOSE(Council<A>& c, A& a) {
    a.dispose(*this,c);
    return ES_NOFIX;
  }

  template<class A>
  forceinline ExecStatus
  Space::ES_NOFIX_DISPOSE_FORCE(Council<A>& c, A& a) {
    a.dispose(*this,c);
    return ES_NOFIX_FORCE;
  }



  /*
   * Advisor council
   *
   */
  template<class A>
  forceinline
  Council<A>::Council(void) {}

  template<class A>
  forceinline
  Council<A>::Council(Space&)
    : advisors(NULL) {}

  template<class A>
  forceinline bool
  Council<A>::empty(void) const {
    ActorLink* a = advisors;
    while ((a != NULL) && static_cast<A*>(a)->disposed())
      a = a->next();
    advisors = a;
    return a == NULL;
  }

  template<class A>
  forceinline void
  Council<A>::update(Space& home, Council<A>& c) {
    // Skip all disposed advisors
    {
      ActorLink* a = c.advisors;
      while ((a != NULL) && static_cast<A*>(a)->disposed())
        a = a->next();
      c.advisors = a;
    }
    // Are there any advisors to be cloned?
    if (c.advisors != NULL) {
      // The propagator in from-space
      Propagator* p_f = &static_cast<A*>(c.advisors)->propagator();
      // The propagator in to-space
      Propagator* p_t = Propagator::cast(p_f->prev());
      // Advisors in from-space
      ActorLink** a_f = &c.advisors;
      // Advisors in to-space
      A* a_t = NULL;
      while (*a_f != NULL) {
        if (static_cast<A*>(*a_f)->disposed()) {
          *a_f = (*a_f)->next();
        } else {
          // Run specific copying part
          A* a = new (home) A(home,*static_cast<A*>(*a_f));
          // Set propagator pointer
          a->prev(p_t);
          // Set forwarding pointer
          (*a_f)->prev(a);
          // Link
          a->next(a_t);
          a_t = a;
          a_f = (*a_f)->next_ref();
        }
      }
      advisors = a_t;
      // Enter advisor link for reset
      assert(p_f->u.advisors == NULL);
      p_f->u.advisors = c.advisors;
    } else {
      advisors = NULL;
    }
  }

  template<class A>
  forceinline void
  Council<A>::dispose(Space& home) {
    ActorLink* a = advisors;
    while (a != NULL) {
      if (!static_cast<A*>(a)->disposed())
        static_cast<A*>(a)->dispose(home,*this);
      a = a->next();
    }
  }



  /*
   * Advisor iterator
   *
   */
  template<class A>
  forceinline
  Advisors<A>::Advisors(const Council<A>& c)
    : a(c.advisors) {
    while ((a != NULL) && static_cast<A*>(a)->disposed())
      a = a->next();
  }

  template<class A>
  forceinline bool
  Advisors<A>::operator ()(void) const {
    return a != NULL;
  }

  template<class A>
  forceinline void
  Advisors<A>::operator ++(void) {
    do {
      a = a->next();
    } while ((a != NULL) && static_cast<A*>(a)->disposed());
  }

  template<class A>
  forceinline A&
  Advisors<A>::advisor(void) const {
    return *static_cast<A*>(a);
  }



  /*
   * Space
   *
   */
  forceinline void
  Space::enqueue(Propagator* p) {
    ActorLink::cast(p)->unlink();
    ActorLink* c = &pc.p.queue[p->cost(*this,p->u.med).ac];
    c->tail(ActorLink::cast(p));
    if (c > pc.p.active)
      pc.p.active = c;
  }

  forceinline void
  Space::fail(void) {
    pc.p.active = &pc.p.queue[PropCost::AC_MAX+1]+1;
    /*
     * Now active points beyond the last queue. This is essential as
     * enqueuing a propagator in a failed space keeps the space
     * failed.
     */
  }
  forceinline void
  Home::fail(void) {
    s.fail();
  }

  forceinline bool
  Space::failed(void) const {
    return pc.p.active > &pc.p.queue[PropCost::AC_MAX+1];
  }
  forceinline bool
  Home::failed(void) const {
    return s.failed();
  }

  forceinline bool
  Space::stable(void) const {
    return ((pc.p.active < &pc.p.queue[0]) ||
            (pc.p.active > &pc.p.queue[PropCost::AC_MAX+1]));
  }

  forceinline void
  Space::notice(Actor& a, ActorProperty p, bool d) {
    if (p & AP_DISPOSE) {
      ap_notice_dispose(&a,d);
    }
    if (p & AP_VIEW_TRACE) {
      pc.p.bid_sc |= sc_trace;
    }
    if (p & AP_TRACE) {
      pc.p.bid_sc |= sc_trace;
    }
    // Currently unused
    if (p & AP_WEAKLY) {}
  }

  forceinline void
  Space::ignore(Actor& a, ActorProperty p, bool d) {
    // Check wether array has already been discarded as space
    // deletion is already in progress
    if ((p & AP_DISPOSE) && (d_fst != NULL))
      ap_ignore_dispose(&a,d);
    if (p & AP_VIEW_TRACE) {}
    if (p & AP_TRACE) {}
    // Currently unused
    if (p & AP_WEAKLY) {}
  }



  /*
   * Variable implementation
   *
   */
  template<class VIC>
  forceinline ActorLink**
  VarImp<VIC>::actor(PropCond pc) {
    assert((pc >= 0)  && (pc < pc_max+2));
    return (pc == 0) ? b.base : b.base+u.idx[pc-1];
  }

  template<class VIC>
  forceinline ActorLink**
  VarImp<VIC>::actorNonZero(PropCond pc) {
    assert((pc > 0)  && (pc < pc_max+2));
    return b.base+u.idx[pc-1];
  }

  template<class VIC>
  forceinline unsigned int&
  VarImp<VIC>::idx(PropCond pc) {
    assert((pc > 0)  && (pc < pc_max+2));
    return u.idx[pc-1];
  }

  template<class VIC>
  forceinline unsigned int
  VarImp<VIC>::idx(PropCond pc) const {
    assert((pc > 0)  && (pc < pc_max+2));
    return u.idx[pc-1];
  }

  template<class VIC>
  forceinline
  VarImp<VIC>::VarImp(Space& home)
#ifdef GECODE_HAS_CBS
  : var_id(++home.var_id_counter)
#endif
  {
#ifndef GECODE_HAS_CBS
    (void) home;
#endif
    b.base = NULL; entries = 0;
    for (PropCond pc=1; pc<pc_max+2; pc++)
      idx(pc) = 0;
    free_and_bits = 0;
  }

  template<class VIC>
  forceinline
  VarImp<VIC>::VarImp(void)
#ifdef GECODE_HAS_CBS
  : var_id(0)
#endif
  {
    b.base = NULL; entries = 0;
    for (PropCond pc=1; pc<pc_max+2; pc++)
      idx(pc) = 0;
    free_and_bits = 0;
  }

#ifdef GECODE_HAS_CBS
  template<class VIC>
  forceinline unsigned int
  VarImp<VIC>::id(void) const {
    return var_id;
  }
#endif

  template<class VIC>
  forceinline unsigned int
  VarImp<VIC>::degree(void) const {
    assert(!copied());
    return entries;
  }

  template<class VIC>
  forceinline double
  VarImp<VIC>::afc(void) const {
    double d = 0.0;
    // Count the afc of each propagator
    {
      ActorLink** a = const_cast<VarImp<VIC>*>(this)->actor(0);
      ActorLink** e = const_cast<VarImp<VIC>*>(this)->actorNonZero(pc_max+1);
      while (a < e) {
        d += Propagator::cast(*a)->afc(); a++;
      }
    }
    // Count the afc of each advisor's propagator
    {
      ActorLink** a = const_cast<VarImp<VIC>*>(this)->actorNonZero(pc_max+1);
      ActorLink** e = const_cast<VarImp<VIC>*>(this)->b.base+entries;
      while (a < e) {
        d += Advisor::cast(static_cast<ActorLink*>(Support::funmark(*a)))
          ->propagator().afc();
        a++;
      }
    }
    return d;
  }

  template<class VIC>
  forceinline ModEvent
  VarImp<VIC>::modevent(const Delta& d) {
    return d.me;
  }

  template<class VIC>
  forceinline unsigned int
  VarImp<VIC>::bits(void) const {
    return free_and_bits;
  }

  template<class VIC>
  forceinline unsigned int&
  VarImp<VIC>::bits(void) {
    return free_and_bits;
  }

#ifdef GECODE_HAS_VAR_DISPOSE
  template<class VIC>
  forceinline VarImp<VIC>*
  VarImp<VIC>::vars_d(Space& home) {
    return static_cast<VarImp<VIC>*>(home.vars_d<VIC>());
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::vars_d(Space& home, VarImp<VIC>* x) {
    home.vars_d<VIC>(x);
  }
#endif

  template<class VIC>
  forceinline bool
  VarImp<VIC>::copied(void) const {
    return Support::marked(b.fwd);
  }

  template<class VIC>
  forceinline VarImp<VIC>*
  VarImp<VIC>::forward(void) const {
    assert(copied());
    return static_cast<VarImp<VIC>*>(Support::unmark(b.fwd));
  }

  template<class VIC>
  forceinline VarImp<VIC>*
  VarImp<VIC>::next(void) const {
    assert(copied());
    return u.next;
  }

  template<class VIC>
  forceinline
  VarImp<VIC>::VarImp(Space& home, VarImp<VIC>& x)
#ifdef GECODE_HAS_CBS
  : var_id(x.var_id)
#endif
  {
    VarImpBase** reg;
    free_and_bits = x.free_and_bits & ((1 << free_bits) - 1);
    if (x.b.base == NULL) {
      // Variable implementation needs no index structure
      reg = &home.pc.c.vars_noidx;
      assert(x.degree() == 0);
    } else {
      reg = &home.pc.c.vars_u[idx_c];
    }
    // Save subscriptions in copy
    b.base = x.b.base;
    entries = x.entries;
    for (PropCond pc=1; pc<pc_max+2; pc++)
      idx(pc) = x.idx(pc);

    // Set forwarding pointer
    x.b.fwd = static_cast<VarImp<VIC>*>(Support::mark(this));
    // Register original
    x.u.next = static_cast<VarImp<VIC>*>(*reg); *reg = &x;
  }

  template<class VIC>
  forceinline ModEvent
  VarImp<VIC>::me(const ModEventDelta& med) {
    return static_cast<ModEvent>((med & VIC::med_mask) >> VIC::med_fst);
  }

  template<class VIC>
  forceinline ModEventDelta
  VarImp<VIC>::med(ModEvent me) {
    return static_cast<ModEventDelta>(me << VIC::med_fst);
  }

  template<class VIC>
  forceinline ModEvent
  VarImp<VIC>::me_combine(ModEvent me1, ModEvent me2) {
    return VIC::me_combine(me1,me2);
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::schedule(Space& home, Propagator& p, ModEvent me,
                        bool force) {
    if (VIC::med_update(p.u.med,me) || force)
      home.enqueue(&p);
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::schedule(Space& home, PropCond pc1, PropCond pc2, ModEvent me) {
    ActorLink** b = actor(pc1);
    ActorLink** p = actorNonZero(pc2+1);
    while (p-- > b)
      schedule(home,*Propagator::cast(*p),me);
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::resize(Space& home) {
    if (b.base == NULL) {
      assert((free_and_bits >> free_bits) == 0);
      // Create fresh dependency array with four entries
      free_and_bits += 4 << free_bits;
      b.base = home.alloc<ActorLink*>(4);
      for (int i=0; i<pc_max+1; i++)
        u.idx[i] = 0;
    } else {
      // Resize dependency array
      unsigned int n = degree();
      // Find out whether the area is most likely in the special area
      // reserved for subscriptions. If yes, just resize mildly otherwise
      // more agressively
      ActorLink** s = static_cast<ActorLink**>(home.mm.subscriptions());
      unsigned int m =
        ((s <= b.base) && (b.base < s+home.pc.p.n_sub)) ?
        (n+4) : ((n+1)*3>>1);
      ActorLink** prop = home.alloc<ActorLink*>(m);
      free_and_bits += (m-n) << free_bits;
      // Copy entries
      Heap::copy<ActorLink*>(prop, b.base, n);
      home.free<ActorLink*>(b.base,n);
      b.base = prop;
    }
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::enter(Space& home, Propagator* p, PropCond pc) {
    assert(pc <= pc_max);
    // Count one new subscription
    home.pc.p.n_sub += 1;
    if ((free_and_bits >> free_bits) == 0)
      resize(home);
    free_and_bits -= 1 << free_bits;

    // Enter subscription
    b.base[entries] = *actorNonZero(pc_max+1);
    entries++;
    for (PropCond j = pc_max; j > pc; j--) {
      *actorNonZero(j+1) = *actorNonZero(j);
      idx(j+1)++;
    }
    *actorNonZero(pc+1) = *actor(pc);
    idx(pc+1)++;
    *actor(pc) = ActorLink::cast(p);

#ifdef GECODE_AUDIT
    ActorLink** f = actor(pc);
    while (f < (pc == pc_max+1 ? b.base+entries : actorNonZero(pc+1)))
      if (*f == p)
        goto found;
      else
        f++;
    GECODE_NEVER;
    found: ;
#endif
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::enter(Space& home, Advisor* a) {
    // Note that a might be a marked pointer
    // Count one new subscription
    home.pc.p.n_sub += 1;
    if ((free_and_bits >> free_bits) == 0)
      resize(home);
    free_and_bits -= 1 << free_bits;

    // Enter subscription
    b.base[entries++] = *actorNonZero(pc_max+1);
    *actorNonZero(pc_max+1) = a;
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::subscribe(Space& home, Propagator& p, PropCond pc,
                         bool assigned, ModEvent me, bool schedule) {
    if (assigned) {
      // Do not subscribe, just schedule the propagator
      if (schedule)
        VarImp<VIC>::schedule(home,p,ME_GEN_ASSIGNED);
    } else {
      enter(home,&p,pc);
      // Schedule propagator
      if (schedule && (pc != PC_GEN_ASSIGNED))
        VarImp<VIC>::schedule(home,p,me);
    }
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::subscribe(Space& home, Advisor& a, bool assigned, bool fail) {
    if (!assigned) {
      Advisor* ma = static_cast<Advisor*>(Support::ptrjoin(&a,fail ? 1 : 0));
      enter(home,ma);
    }
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::reschedule(Space& home, Propagator& p, PropCond pc,
                          bool assigned, ModEvent me) {
    if (assigned)
      VarImp<VIC>::schedule(home,p,ME_GEN_ASSIGNED);
    else if (pc != PC_GEN_ASSIGNED)
      VarImp<VIC>::schedule(home,p,me);
  }

  template<class VIC>
  void
  VarImp<VIC>::remove(Space& home, Propagator* p, PropCond pc) {
    assert(pc <= pc_max);
    ActorLink* a = ActorLink::cast(p);
    // Find actor in dependency array
    ActorLink** f = actor(pc);
#ifdef GECODE_AUDIT
    while (f < actorNonZero(pc+1))
      if (*f == a)
        goto found;
      else
        f++;
    GECODE_NEVER;
  found: ;
#else
    while (*f != a) f++;
#endif
    // Remove actor
    *f = *(actorNonZero(pc+1)-1);
    for (PropCond j = pc+1; j< pc_max+1; j++) {
      *(actorNonZero(j)-1) = *(actorNonZero(j+1)-1);
      idx(j)--;
    }
    *(actorNonZero(pc_max+1)-1) = b.base[entries-1];
    idx(pc_max+1)--;
    entries--;
    free_and_bits += 1 << free_bits;
    home.pc.p.n_sub -= 1;
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::cancel(Space& home, Propagator& p, PropCond pc) {
    if (b.base != NULL)
      remove(home,&p,pc);
  }

  template<class VIC>
  void
  VarImp<VIC>::remove(Space& home, Advisor* a) {
    // Note that a might be a marked pointer
    // Find actor in dependency array
    ActorLink** f = actorNonZero(pc_max+1);
#ifdef GECODE_AUDIT
    while (f < b.base+entries)
      if (*f == a)
        goto found;
      else
        f++;
    GECODE_NEVER;
  found: ;
#else
    while (*f != a) f++;
#endif
    // Remove actor
    *f = b.base[--entries];
    free_and_bits += 1 << free_bits;
    home.pc.p.n_sub -= 1;
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::cancel(Space& home, Advisor& a, bool fail) {
    if (b.base != NULL) {
      Advisor* ma = static_cast<Advisor*>(Support::ptrjoin(&a,fail ? 1 : 0));
      remove(home,ma);
    }
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::cancel(Space& home) {
    unsigned int n_sub = degree();
    home.pc.p.n_sub -= n_sub;
    unsigned int n = (free_and_bits >> free_bits) + n_sub;
    home.free<ActorLink*>(b.base,n);
    // Must be NULL such that cloning works
    b.base = NULL;
    // Must be 0 such that degree works
    entries = 0;
    // Must be NULL such that afc works
    for (PropCond pc=1; pc<pc_max+2; pc++)
      idx(pc) = 0;
    free_and_bits &= (1 << free_bits) - 1;
  }

  template<class VIC>
  forceinline bool
  VarImp<VIC>::advise(Space& home, ModEvent me, Delta& d) {
    /*
     * An advisor that is executed might remove itself due to subsumption.
     * As entries are removed from front to back, the advisors must
     * be iterated in forward direction.
     */
    ActorLink** la = actorNonZero(pc_max+1);
    ActorLink** le = b.base+entries;
    if (la == le)
      return true;
    d.me = me;
    // An advisor that is run, might be removed during execution.
    // As removal is done from the back the advisors have to be executed
    // in inverse order.
    do {
      Advisor* a = Advisor::cast
        (static_cast<ActorLink*>(Support::funmark(*la)));
      assert(!a->disposed());
      Propagator& p = a->propagator();
      switch (p.advise(home,*a,d)) {
      case ES_FIX:
        break;
      case ES_FAILED:
        return false;
      case ES_NOFIX:
        schedule(home,p,me);
        break;
      case ES_NOFIX_FORCE:
        schedule(home,p,me,true);
        break;
      case __ES_SUBSUMED:
      default:
        GECODE_NEVER;
      }
    } while (++la < le);
    return true;
  }

  template<class VIC>
  void
  VarImp<VIC>::_fail(Space& home) {
    /*
     * An advisor that is executed might remove itself due to subsumption.
     * As entries are removed from front to back, the advisors must
     * be iterated in forward direction.
     */
    ActorLink** la = actorNonZero(pc_max+1);
    ActorLink** le = b.base+entries;
    if (la == le)
      return;
    // An advisor that is run, might be removed during execution.
    // As removal is done from the back the advisors have to be executed
    // in inverse order.
    do {
      if (Support::marked(*la)) {
        Advisor* a = Advisor::cast(static_cast<ActorLink*>
                                   (Support::unmark(*la)));
        assert(!a->disposed());
        Propagator& p = a->propagator();
        p.advise(home,*a);
      }
    } while (++la < le);
  }

  template<class VIC>
  ModEvent
  VarImp<VIC>::fail(Space& home) {
    _fail(home); 
    return ME_GEN_FAILED;
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::update(VarImp<VIC>* x, ActorLink**& sub) {
    // this refers to the variable to be updated (clone)
    // x refers to the original
    // Recover from copy
    x->b.base = b.base;
    x->u.idx[0] = u.idx[0];
    if (pc_max > 0 && sizeof(ActorLink**) > sizeof(unsigned int))
      x->u.idx[1] = u.idx[1];

    unsigned int np =
      static_cast<unsigned int>(x->actorNonZero(pc_max+1) - x->actor(0));
    unsigned int na = 
      static_cast<unsigned int >(x->b.base + x->entries - 
                                 x->actorNonZero(pc_max+1));
    unsigned int n  = na + np;
    assert(n == x->degree());

    ActorLink** f = x->b.base;
    ActorLink** t = sub;

    sub += n;
    b.base = t;
    // Process propagator subscriptions
    while (np >= 4) {
      ActorLink* p3 = f[3]->prev();
      ActorLink* p0 = f[0]->prev();
      ActorLink* p1 = f[1]->prev();
      ActorLink* p2 = f[2]->prev(); 
      t[0] = p0; t[1] = p1; t[2] = p2; t[3] = p3;
      np -= 4; t += 4; f += 4;
    }
    if (np >= 2) {
      ActorLink* p0 = f[0]->prev();
      ActorLink* p1 = f[1]->prev();
      t[0] = p0; t[1] = p1;
      np -= 2; t += 2; f += 2;
    }
    if (np > 0) {
      ActorLink* p0 = f[0]->prev();
      t[0] = p0;
      t += 1; f += 1;
    }
    // Process advisor subscriptions
    while (na >= 4) {
      ptrdiff_t m0, m1, m2, m3;
      ActorLink* p3 =
        static_cast<ActorLink*>(Support::ptrsplit(f[3],m3))->prev();
      ActorLink* p0 = 
        static_cast<ActorLink*>(Support::ptrsplit(f[0],m0))->prev();
      ActorLink* p1 =
        static_cast<ActorLink*>(Support::ptrsplit(f[1],m1))->prev();
      ActorLink* p2 =
        static_cast<ActorLink*>(Support::ptrsplit(f[2],m2))->prev();
      t[0] = static_cast<ActorLink*>(Support::ptrjoin(p0,m0));
      t[1] = static_cast<ActorLink*>(Support::ptrjoin(p1,m1));
      t[2] = static_cast<ActorLink*>(Support::ptrjoin(p2,m2));
      t[3] = static_cast<ActorLink*>(Support::ptrjoin(p3,m3));
      na -= 4; t += 4; f += 4;
    }
    if (na >= 2) {
      ptrdiff_t m0, m1;
      ActorLink* p0 = 
        static_cast<ActorLink*>(Support::ptrsplit(f[0],m0))->prev();
      ActorLink* p1 =
        static_cast<ActorLink*>(Support::ptrsplit(f[1],m1))->prev();
      t[0] = static_cast<ActorLink*>(Support::ptrjoin(p0,m0));
      t[1] = static_cast<ActorLink*>(Support::ptrjoin(p1,m1));
      na -= 2; t += 2; f += 2;
    }
    if (na > 0) {
      ptrdiff_t m0;
      ActorLink* p0 = 
        static_cast<ActorLink*>(Support::ptrsplit(f[0],m0))->prev();
      t[0] = static_cast<ActorLink*>(Support::ptrjoin(p0,m0));
    }
  }

  template<class VIC>
  forceinline void
  VarImp<VIC>::update(Space& home, ActorLink**& sub) {
    VarImp<VIC>* x = static_cast<VarImp<VIC>*>(home.pc.c.vars_u[idx_c]);
    while (x != NULL) {
      VarImp<VIC>* n = x->next(); x->forward()->update(x,sub); x = n;
    }
  }



  /*
   * Variable disposer
   *
   */
  template<class VarImp>
  VarImpDisposer<VarImp>::VarImpDisposer(void) {
#ifdef GECODE_HAS_VAR_DISPOSE
    Space::vd[VarImp::idx_d] = this;
#endif
  }

  template<class VarImp>
  void
  VarImpDisposer<VarImp>::dispose(Space& home, VarImpBase* _x) {
    VarImp* x = static_cast<VarImp*>(_x);
    do {
      x->dispose(home); x = static_cast<VarImp*>(x->next_d());
    } while (x != NULL);
  }

  /*
   * Statistics
   */

  forceinline void
  StatusStatistics::reset(void) {
    propagate = 0;
  }
  forceinline
  StatusStatistics::StatusStatistics(void) {
    reset();
  }
  forceinline StatusStatistics&
  StatusStatistics::operator +=(const StatusStatistics& s) {
    propagate += s.propagate;
    return *this;
  }
  forceinline StatusStatistics
  StatusStatistics::operator +(const StatusStatistics& s) {
    StatusStatistics t(s);
    return t += *this;
  }

  forceinline void
  CloneStatistics::reset(void) {}

  forceinline
  CloneStatistics::CloneStatistics(void) {
    reset();
  }
  forceinline CloneStatistics
  CloneStatistics::operator +(const CloneStatistics&) {
    CloneStatistics s;
    return s;
  }
  forceinline CloneStatistics&
  CloneStatistics::operator +=(const CloneStatistics&) {
    return *this;
  }

  forceinline void
  CommitStatistics::reset(void) {}

  forceinline
  CommitStatistics::CommitStatistics(void) {
    reset();
  }
  forceinline CommitStatistics
  CommitStatistics::operator +(const CommitStatistics&) {
    CommitStatistics s;
    return s;
  }
  forceinline CommitStatistics&
  CommitStatistics::operator +=(const CommitStatistics&) {
    return *this;
  }

  /*
   * Cost computation
   *
   */

  forceinline
  PropCost::PropCost(PropCost::ActualCost ac0) : ac(ac0) {}

  forceinline PropCost
  PropCost::cost(PropCost::Mod m,
                 PropCost::ActualCost lo, PropCost::ActualCost hi,
                 unsigned int n) {
    if (n < 2)
      return (m == LO) ? AC_UNARY_LO : AC_UNARY_HI;
    else if (n == 2)
      return (m == LO) ? AC_BINARY_LO : AC_BINARY_HI;
    else if (n == 3)
      return (m == LO) ? AC_TERNARY_LO : AC_TERNARY_HI;
    else
      return (m == LO) ? lo : hi;
  }

  forceinline PropCost
  PropCost::record(void) {
    return AC_RECORD;
  }
  forceinline PropCost
  PropCost::crazy(PropCost::Mod m, unsigned int n) {
    return cost(m,AC_CRAZY_LO,AC_CRAZY_HI,n);
  }
  forceinline PropCost
  PropCost::crazy(PropCost::Mod m, int n) {
    assert(n >= 0);
    return crazy(m,static_cast<unsigned int>(n));
  }
  forceinline PropCost
  PropCost::cubic(PropCost::Mod m, unsigned int n) {
    return cost(m,AC_CUBIC_LO,AC_CUBIC_HI,n);
  }
  forceinline PropCost
  PropCost::cubic(PropCost::Mod m, int n) {
    assert(n >= 0);
    return cubic(m,static_cast<unsigned int>(n));
  }
  forceinline PropCost
  PropCost::quadratic(PropCost::Mod m, unsigned int n) {
    return cost(m,AC_QUADRATIC_LO,AC_QUADRATIC_HI,n);
  }
  forceinline PropCost
  PropCost::quadratic(PropCost::Mod m, int n) {
    assert(n >= 0);
    return quadratic(m,static_cast<unsigned int>(n));
  }
  forceinline PropCost
  PropCost::linear(PropCost::Mod m, unsigned int n) {
    return cost(m,AC_LINEAR_LO,AC_LINEAR_HI,n);
  }
  forceinline PropCost
  PropCost::linear(PropCost::Mod m, int n) {
    assert(n >= 0);
    return linear(m,static_cast<unsigned int>(n));
  }
  forceinline PropCost
  PropCost::ternary(PropCost::Mod m) {
    return (m == LO) ? AC_TERNARY_LO : AC_TERNARY_HI;
  }
  forceinline PropCost
  PropCost::binary(PropCost::Mod m) {
    return (m == LO) ? AC_BINARY_LO : AC_BINARY_HI;
  }
  forceinline PropCost
  PropCost::unary(PropCost::Mod m) {
    return (m == LO) ? AC_UNARY_LO : AC_UNARY_HI;
  }

  /*
   * Iterators for propagators and branchers of a space
   *
   */
  forceinline
  Space::Propagators::Propagators(Space& home0)
    : home(home0), q(home.pc.p.active) {
    while (q >= &home.pc.p.queue[0]) {
      if (q->next() != q) {
        c = q->next(); e = q; q--;
        return;
      }
      q--;
    }
    q = NULL;
    if (!home.pl.empty()) {
      c = Propagator::cast(home.pl.next());
      e = Propagator::cast(&home.pl);
    } else {
      c = e = NULL;
    }
  }
  forceinline bool
  Space::Propagators::operator ()(void) const {
    return c != NULL;
  }
  forceinline void
  Space::Propagators::operator ++(void) {
    c = c->next();
    if (c == e) {
      if (q == NULL) {
        c = NULL;
      } else {
        while (q >= &home.pc.p.queue[0]) {
          if (q->next() != q) {
            c = q->next(); e = q; q--;
            return;
          }
          q--;
        }
        q = NULL;
        if (!home.pl.empty()) {
          c = Propagator::cast(home.pl.next());
          e = Propagator::cast(&home.pl);
        } else {
          c = NULL;
        }
      }
    }
  }
  forceinline Propagator&
  Space::Propagators::propagator(void) const {
    return *Propagator::cast(c);
  }


  forceinline
  Space::ScheduledPropagators::ScheduledPropagators(Space& home0)
    : home(home0), q(home.pc.p.active) {
    while (q >= &home.pc.p.queue[0]) {
      if (q->next() != q) {
        c = q->next(); e = q; q--;
        return;
      }
      q--;
    }
    q = c = e = NULL;
  }
  forceinline bool
  Space::ScheduledPropagators::operator ()(void) const {
    return c != NULL;
  }
  forceinline void
  Space::ScheduledPropagators::operator ++(void) {
    c = c->next();
    if (c == e) {
      if (q == NULL) {
        c = NULL;
      } else {
        while (q >= &home.pc.p.queue[0]) {
          if (q->next() != q) {
            c = q->next(); e = q; q--;
            return;
          }
          q--;
        }
        q = c = e = NULL;
      }
    }
  }
  forceinline Propagator&
  Space::ScheduledPropagators::propagator(void) const {
    return *Propagator::cast(c);
  }


  forceinline
  Space::IdlePropagators::IdlePropagators(Space& home) {
    c = Propagator::cast(home.pl.next());
    e = Propagator::cast(&home.pl);
  }
  forceinline bool
  Space::IdlePropagators::operator ()(void) const {
    return c != e;
  }
  forceinline void
  Space::IdlePropagators::operator ++(void) {
    c = c->next();
  }
  forceinline Propagator&
  Space::IdlePropagators::propagator(void) const {
    return *Propagator::cast(c);
  }


  forceinline
  Space::Branchers::Branchers(Space& home)
    : c(Brancher::cast(home.bl.next())), e(&home.bl) {}
  forceinline bool
  Space::Branchers::operator ()(void) const {
    return c != e;
  }
  forceinline void
  Space::Branchers::operator ++(void) {
    c = c->next();
  }
  forceinline Brancher&
  Space::Branchers::brancher(void) const {
    return *Brancher::cast(c);
  }


  /*
   * Groups of actors
   */
  forceinline
  Group::Group(unsigned int gid0) : gid(gid0) {}

  forceinline bool
  Group::in(Group actor) const {
    return (gid == GROUPID_ALL) || (gid == actor.gid);
  }

  forceinline bool
  Group::in(void) const {
    return (gid != GROUPID_ALL) && (gid != GROUPID_DEF);
  }

  forceinline
  Group::Group(const Group& g) : gid(g.gid) {}

  forceinline Group&
  Group::operator =(const Group& g) {
    gid=g.gid; return *this;
  }

  forceinline unsigned int
  Group::id(void) const {
    return gid;
  }


  forceinline
  PropagatorGroup::PropagatorGroup(void) {}

  forceinline
  PropagatorGroup::PropagatorGroup(unsigned int gid)
    : Group(gid) {}

  forceinline
  PropagatorGroup::PropagatorGroup(const PropagatorGroup& g)
    : Group(g) {}

  forceinline PropagatorGroup&
  PropagatorGroup::operator =(const PropagatorGroup& g) {
    return static_cast<PropagatorGroup&>(Group::operator =(g));
  }

  forceinline Home
  PropagatorGroup::operator ()(Space& home) {
    return Home(home,NULL,*this,BrancherGroup::def);
  }

  forceinline bool
  PropagatorGroup::operator ==(PropagatorGroup g) const {
    return id() == g.id();
  }
  forceinline bool
  PropagatorGroup::operator !=(PropagatorGroup g) const {
    return id() != g.id();
  }

  forceinline PropagatorGroup&
  PropagatorGroup::move(Space& , Propagator& p) {
    if (id() != GROUPID_ALL)
      p.group(*this);
    return *this;
  }


  forceinline
  BrancherGroup::BrancherGroup(void) {}

  forceinline
  BrancherGroup::BrancherGroup(unsigned int gid)
    : Group(gid) {}

  forceinline
  BrancherGroup::BrancherGroup(const BrancherGroup& g)
    : Group(g) {}

  forceinline BrancherGroup&
  BrancherGroup::operator =(const BrancherGroup& g) {
    return static_cast<BrancherGroup&>(Group::operator =(g));
  }

  forceinline Home
  BrancherGroup::operator ()(Space& home) {
    return Home(home,NULL,PropagatorGroup::def,*this);
  }

  forceinline bool
  BrancherGroup::operator ==(BrancherGroup g) const {
    return id() == g.id();
  }
  forceinline bool
  BrancherGroup::operator !=(BrancherGroup g) const {
    return id() != g.id();
  }

  forceinline BrancherGroup&
  BrancherGroup::move(Space& , Brancher& p) {
    if (id() != GROUPID_ALL)
      p.group(*this);
    return *this;
  }


  /*
   * Iterators for propagators and branchers in a group
   *
   */
  forceinline
  Propagators::Propagators(const Space& home, PropagatorGroup g0)
    : ps(const_cast<Space&>(home)), g(g0) {
    while (ps() && !g.in(ps.propagator().group()))
      ++ps;
  }
  forceinline bool
  Propagators::operator ()(void) const {
    return ps();
  }
  forceinline void
  Propagators::operator ++(void) {
    do
      ++ps;
    while (ps() && !g.in(ps.propagator().group()));
  }
  forceinline const Propagator&
  Propagators::propagator(void) const {
    return ps.propagator();
  }

  forceinline
  Branchers::Branchers(const Space& home, BrancherGroup g0)
    : bs(const_cast<Space&>(home)), g(g0) {
    while (bs() && !g.in(bs.brancher().group()))
      ++bs;
  }
  forceinline bool
  Branchers::operator ()(void) const {
    return bs();
  }
  forceinline void
  Branchers::operator ++(void) {
    do
      ++bs;
    while (bs() && !g.in(bs.brancher().group()));
  }
  forceinline const Brancher&
  Branchers::brancher(void) const {
    return bs.brancher();
  }


  /*
   * Space construction support
   *
   */
  template<class T>
  forceinline T&
  Space::construct(void) {
    return alloc<T>(1);
  }
  template<class T, typename A1>
  forceinline T&
  Space::construct(A1 const& a1) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1);
    return t;
  }
  template<class T, typename A1, typename A2>
  forceinline T&
  Space::construct(A1 const& a1, A2 const& a2) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3>
  forceinline T&
  Space::construct(A1 const& a1, A2 const& a2, A3 const& a3) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3, typename A4>
  forceinline T&
  Space::construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3,a4);
    return t;
  }
  template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
  forceinline T&
  Space::construct(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5) {
    T& t = *static_cast<T*>(ralloc(sizeof(T)));
    new (&t) T(a1,a2,a3,a4,a5);
    return t;
  }

}

// STATISTICS: kernel-core
