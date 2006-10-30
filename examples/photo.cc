/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "examples/support.hh"
#include "gecode/minimodel.hh"

/// Specifications for the Photo example
class PhotoSpec {
public:
  const int  n_names; ///< Number of people on picture
  const int  n_prefs; ///< Number of preferences
  const int* prefs;   ///< Array of preferences
  PhotoSpec(const int n_n, const int n_p, const int* p)
    : n_names(n_n), n_prefs(n_p), prefs(p) {}
};

/// Preferences for small example
static const int s_prefs[] = {
  0,2, 1,4, 2,3, 2,4, 3,0, 4,3, 4,0, 4,1
};
/// Small Photo example
static const PhotoSpec p_small(5, 8, s_prefs);

/// Preferences for large example
static const int l_prefs[] = {
  0,2, 0,4, 0,7, 1,4, 1,8, 2,3, 2,4, 3,0, 3,4,
  4,5, 4,0, 5,0, 5,8, 6,2, 6,7, 7,8, 7,6
};
/// Large Photo example
static const PhotoSpec p_large(9,17, l_prefs);

/**
 * \brief %Example: Placing people on a photo
 *
 * A group of people wants to take a group photo. Each person can give
 * preferences next to whom he or she wants to be placed on the
 * photo. The problem to be solved is to find a placement that
 * satisfies as many preferences as possible.
 *
 * \ingroup Example
 *
 */
class Photo : public Example {
protected:
  /// Photo specification
  const PhotoSpec& spec;
  /// Person's position on photo
  IntVarArray      pos;
  /// Number of satisfied preferences
  IntVar           sat;

public:
  /// Actual model
  Photo(const Options& opt) :
    spec(opt.size == 0 ? p_small : p_large),
    pos(this,spec.n_names, 0, spec.n_names-1),
    sat(this,0,spec.n_prefs)
  {
    BoolVarArgs ful(spec.n_prefs);
    // Map preferences to fulfilment
    for (int i = spec.n_prefs; i--; ) {
      int pa = spec.prefs[2*i+0];
      int pb = spec.prefs[2*i+1];
      ful[i] = post(this,
                    ~(pos[pb]-pos[pa] == 1) ^
                    ~(pos[pa]-pos[pb] == 1));
    }
    // Sum of fulfilment
    linear(this, ful, IRT_EQ, sat);

    distinct(this, pos, opt.icl);

    // Break some symmetries
    rel(this, pos[0], IRT_LE, pos[1]);

    if (opt.naive) {
      branch(this, pos, BVAR_NONE, BVAL_MIN);
    } else {
      branch(this, pos, BVAR_DEGREE_MAX, BVAL_MIN);
    }
  }

  /// Constructor for cloning \a s
  Photo(bool share, Photo& s) :
    Example(share,s), spec(s.spec) {
    pos.update(this, share, s.pos);
    sat.update(this, share, s.sat);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Photo(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tpos[] = {";
    for (int i = 0; i < spec.n_names; i++)
      std::cout << pos[i] << ((i<spec.n_names-1)?",":"};\n");
    std::cout << "\tsat: " << sat << std::endl;
  }

  /// Add constraint for next better solution
  void
  constrain(Space* s) {
    rel(this, sat, IRT_GR, static_cast<Photo*>(s)->sat.val());
  }

};

/** \brief Main-function
 *  \relates Photo
 */
int
main(int argc, char** argv) {
  Options opt("Photo");
  opt.solutions  = 0;
  opt.size       = 1;
  opt.iterations = 10;
  opt.icl        = ICL_BND;
  opt.parse(argc,argv);
  Example::run<Photo,BAB>(opt);
  return 0;
}


// STATISTICS: example-any

