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

using namespace Gecode;

/// Specifications for the photo example
class PhotoSpec {
public:
  const int  n_names; ///< Number of people on picture
  const int  n_prefs; ///< Number of preferences
  const int* prefs;   ///< Array of preferences
  PhotoSpec(const int n_n, const int n_p, const int* p)
    : n_names(n_n), n_prefs(n_p), prefs(p) {}
};

/// Preferences for small example
const int s_prefs[] = {
  0,2, 1,4, 2,3, 2,4, 3,0, 4,3, 4,0, 4,1
};
/// Small Photo example
const PhotoSpec p_small(5, 8, s_prefs);

/// Preferences for large example
const int l_prefs[] = {
  0,2, 0,4, 0,7, 1,4, 1,8, 2,3, 2,4, 3,0, 3,4,
  4,5, 4,0, 5,0, 5,8, 6,2, 6,7, 7,8, 7,6
};
/// Large Photo example
const PhotoSpec p_large(9,17, l_prefs);

/**
 * \brief %Example: Placing people on a photo
 *
 * A group of people wants to take a group photo. Each person can give
 * preferences next to whom he or she wants to be placed on the
 * photo. The problem to be solved is to find a placement that
 * violates as few preferences as possible.
 *
 * \ingroup Example
 *
 */
class Photo : public MinimizeScript {
protected:
  /// Photo specification
  const PhotoSpec& spec;
  /// Person's position on photo
  IntVarArray      pos;
  /// Number of violated preferences
  IntVar           violations;

public:
  /// Branching to use for model
  enum {
    BRANCH_NONE,  ///< Choose variables from left to right
    BRANCH_DEGREE ///< Choose variable with largest degree
  };
  /// Actual model
  Photo(const SizeOptions& opt) :
    spec(opt.size() == 0 ? p_small : p_large),
    pos(*this,spec.n_names, 0, spec.n_names-1),
    violations(*this,0,spec.n_prefs)
  {
    // Map preferences to violation
    BoolVarArgs viol(spec.n_prefs);
    for (int i=0; i<spec.n_prefs; i++) {
      int pa = spec.prefs[2*i+0];
      int pb = spec.prefs[2*i+1];
      viol[i] = expr(*this, abs(pos[pa]-pos[pb]) > 1);
    }
    rel(*this, violations == sum(viol));

    distinct(*this, pos, opt.icl());

    // Break some symmetries
    rel(*this, pos[0] < pos[1]);

    if (opt.branching() == BRANCH_NONE) {
      branch(*this, pos, INT_VAR_NONE, INT_VAL_MIN);
    } else {
      branch(*this, pos, tiebreak(INT_VAR_DEGREE_MAX,INT_VAR_SIZE_MIN),
             INT_VAL_MIN);
    }
  }

  /// Constructor for cloning \a s
  Photo(bool share, Photo& s) :
    MinimizeScript(share,s), spec(s.spec) {
    pos.update(*this, share, s.pos);
    violations.update(*this, share, s.violations);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Photo(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tpos[] = " << pos << std::endl
       << "\tviolations: " << violations << std::endl;
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return violations;
  }
};

/** \brief Main-function
 *  \relates Photo
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Photo");
  opt.solutions(0);
  opt.size(1);
  opt.iterations(10);
  opt.icl(ICL_BND);
  opt.branching(Photo::BRANCH_DEGREE);
  opt.branching(Photo::BRANCH_NONE,   "none");
  opt.branching(Photo::BRANCH_DEGREE, "degree");
  opt.parse(argc,argv);
  MaximizeScript::run<Photo,BAB,SizeOptions>(opt);
  return 0;
}


// STATISTICS: example-any

