/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2013
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

namespace Gecode {

  /**
   * \brief Class for AFC (accumulated failure count) management
   *
   */
  class AFC {
  protected:
    /// Number of views
    int n;
  public:
    /// \name Constructors and initialization
    //@{
    /**
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized AFC storage is init and the assignment operator.
     *
     */
    AFC(void);
    /// Copy constructor
    AFC(const AFC& a);
    /// Assignment operator
    AFC& operator =(const AFC& a);
    /** \brief Initialize for variables \a x and decay factor \a d
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    template<class Var>
    AFC(Home home, const VarArgArray<Var>& x, double d, bool share=true);
    /** \brief Initialize for variables \a x and decay factor \a d
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    template<class Var>
    void init(Home home, const VarArgArray<Var>& x, double d, bool share=true);
    /// Test whether already initialized
    operator bool(void) const;
    /// Default (empty) AFC information
    GECODE_KERNEL_EXPORT static const AFC def;
    //@}

    /// Destructor
    ~AFC(void);

    /// \name Information access
    //@{
    /// Return number of AFC values
    int size(void) const;
    //@}

    /// \name Decay factor for aging
    //@{
    /// Set decay factor to \a d
    void decay(Space& home, double d);
    /// Return decay factor
    double decay(const Space& home) const;
    //@}
  };

  /**
   * \brief Print AFC information (prints nothing)
   * \relates AFC
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const AFC& a);

  /*
   * AFC
   *
   */
  forceinline int
  AFC::size(void) const {
    assert(n >= 0);
    return n;
  }

  forceinline
  AFC::AFC(void) : n(-1) {}

  forceinline
  AFC::operator bool(void) const {
    return n >= 0;
  }

  template<class Var>
  forceinline
  AFC::AFC(Home home, const VarArgArray<Var>& x, double d, bool share)
    : n(x.size()) {
    if ((d < 0.0) || (d > 1.0))
      throw IllegalDecay("AFC");
    static_cast<Space&>(home).afc_decay(d);
    if (!share)
      static_cast<Space&>(home).afc_unshare();
  }
  template<class Var>
  forceinline void
  AFC::init(Home home, const VarArgArray<Var>& x, double d, bool share) {
    n = x.size();
    if ((d < 0.0) || (d > 1.0))
      throw IllegalDecay("AFC");
    static_cast<Space&>(home).afc_decay(d);
    if (!share)
      static_cast<Space&>(home).afc_unshare();
  }


  forceinline
  AFC::AFC(const AFC& a)
    : n(a.n) {}
  forceinline AFC&
  AFC::operator =(const AFC& a) {
    n=a.n;
    return *this;
  }
  forceinline
  AFC::~AFC(void) {}

  forceinline void
  AFC::decay(Space& home, double d) {
    if ((d < 0.0) || (d > 1.0))
      throw IllegalDecay("AFC");
    home.afc_decay(d);
  }

  forceinline double
  AFC::decay(const Space& home) const {
    return home.afc_decay();
  }


  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const AFC& a) {
    (void)a;
    return os << "AFC(no information available)";
  }

}

// STATISTICS: kernel-branch
