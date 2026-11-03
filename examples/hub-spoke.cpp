/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#include <gecode/driver.hh>
#include <gecode/int.hh>

using namespace Gecode;

/**
 * \brief %Example: meta-search hooks after the Gecode 7 rename
 *
 * Gecode 7 renames the two meta-search configuration hooks of a space:
 * \code
 * // Gecode 6                               // Gecode 7
 * bool master(const MetaInfo&) override;    bool hub(const MetaInfo&) override;
 * bool slave(const MetaInfo&) override;     bool spoke(const MetaInfo&) override;
 * \endcode
 * The removed declarations have no compatibility aliases. With `override`,
 * the compiler reports every model that still uses them. Run with
 * `-restart` to have restart-based search call both hooks.
 *
 * \ingroup Example
 */
class HubSpoke : public Script {
protected:
  /// Variable with two solutions
  IntVar x;

public:
  /// Post the model
  HubSpoke(const Options& opt) : Script(opt), x(*this, 0, 1) {
    branch(*this, x, INT_VAL_MIN());
  }
  /// Constructor for cloning \a s
  HubSpoke(HubSpoke& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy during cloning
  Space*
  copy(void) override {
    return new HubSpoke(*this);
  }
  /// Exclude solution \a s, as the default hub() does after a solution
  void
  constrain(const Space& s) override {
    rel(*this, x, IRT_NQ, static_cast<const HubSpoke&>(s).x.val());
  }
  /// Configure the hub space; formerly master()
  bool
  hub(const MetaInfo& mi) override {
    return Space::hub(mi);
  }
  /// Configure a derived spoke space; formerly slave()
  bool
  spoke(const MetaInfo& mi) override {
    return Space::spoke(mi);
  }
  /// Print solution
  void
  print(std::ostream& os) const override {
    os << "x = " << x << '\n';
  }
};

/** \brief Main-function
 *  \relates HubSpoke
 */
int
main(int argc, char* argv[]) {
  Options opt("HubSpoke");
  opt.parse(argc, argv);
  Script::run<HubSpoke, DFS, Options>(opt);
  return 0;
}

// STATISTICS: example-any
