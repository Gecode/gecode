/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2014
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

#include "test/flatzinc.hh"

#include <memory>
#include <cmath>
#include <iomanip>
#include <limits>

namespace Test { namespace FlatZinc {

  namespace {

    /// Verify that FlatZinc table conversion uses automatic representation.
    class TupleSetAutoRepresentation : public Base {
    public:
      TupleSetAutoRepresentation(void)
        : Base("FlatZinc::TupleSet::AutoRepresentation") {}

      virtual bool run(void) {
        using namespace Gecode;

        const int n = 4096;
        IntArgs tuples(2*n);
        for (int i=0; i<n; i++) {
          tuples[2*i] = i;
          tuples[2*i+1] = n+i;
        }

        Gecode::FlatZinc::FlatZincSpace space;
        TupleSet ts = space.arg2tupleset(tuples,2);
        return ts.finalized() &&
          (ts.representation() == EPK_DENSE_COMPRESSED) &&
          (ts.tuples() == n);
      }
    };

    TupleSetAutoRepresentation tuple_set_auto_representation;

    /// Verify integer objective comparison for FlatZinc spaces.
    class IntegerObjectiveComparison : public Base {
    private:
      static std::unique_ptr<Gecode::FlatZinc::FlatZincSpace>
      model(const char* source) {
        Gecode::FlatZinc::Printer p;
        std::stringstream ss(source);
        return std::unique_ptr<Gecode::FlatZinc::FlatZincSpace>
          (Gecode::FlatZinc::parse(ss,p,olog));
      }
    public:
      IntegerObjectiveComparison(void)
        : Base("FlatZinc::IntegerObjectiveComparison") {}

      virtual bool run(void) {
        using namespace Gecode;
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> one =
          model("var 1..1: x; solve minimize x;\n");
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> two =
          model("var 2..2: x; solve minimize x;\n");
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> high =
          model("var 2..2: x; solve maximize x;\n");
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> low =
          model("var 1..1: x; solve maximize x;\n");
        if (!one || !two || !high || !low ||
            (one->compare(*two) != SC_BETTER) ||
            (two->compare(*one) != SC_WORSE) ||
            (one->compare(*one) != SC_EQUIVALENT) ||
            (high->compare(*low) != SC_BETTER))
          return false;
        try {
          (void) one->compare(*high);
          return false;
        } catch (const DynamicCastFailed&) {}
        return true;
      }
    };

    IntegerObjectiveComparison integer_objective_comparison;

#ifdef GECODE_HAS_FLOAT_VARS
    /// Verify float objective comparison and its interval-valued cuts.
    class FloatObjectiveComparison : public Base {
    private:
      static std::unique_ptr<Gecode::FlatZinc::FlatZincSpace>
      model(Gecode::FloatVal v, bool minimize, Gecode::FloatNum step=0.0) {
        std::stringstream source;
        source << std::showpoint
               << std::setprecision(std::numeric_limits<double>::max_digits10)
               << "var float: x = " << v.min() << "; solve "
               << (minimize ? "minimize" : "maximize")
               << " x;\n";
        Gecode::FlatZinc::Printer p;
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> result
          (Gecode::FlatZinc::parse(source,p,olog));
        if (result) {
          result->fv[result->optVar()] =
            Gecode::FloatVar(*result,v.min(),v.max());
          result->step = step;
        }
        return result;
      }

      static bool admitted(Gecode::FloatVal candidate,
                           const Gecode::FlatZinc::FlatZincSpace& incumbent,
                           bool minimize, Gecode::FloatNum step) {
        std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> c =
          model(candidate,minimize,step);
        c->constrain(incumbent);
        return c->status() != Gecode::SS_FAILED;
      }
    public:
      FloatObjectiveComparison(void)
        : Base("FlatZinc::FloatObjectiveComparison") {}

      virtual bool run(void) {
        using namespace Gecode;
        using Gecode::FlatZinc::FlatZincSpace;
        const FloatNum next = std::nextafter(1.0,2.0);
        std::unique_ptr<FlatZincSpace> m9=model(9.5,true,1.0);
        std::unique_ptr<FlatZincSpace> m10=model(10.0,true,1.0);
        std::unique_ptr<FlatZincSpace> m10b=model(10.0,true,1.0);
        std::unique_ptr<FlatZincSpace> x11=model(10.5,false,1.0);
        std::unique_ptr<FlatZincSpace> x10=model(10.0,false,1.0);
        std::unique_ptr<FlatZincSpace> x10b=model(10.0,false,1.0);
        std::unique_ptr<FlatZincSpace> adjacent=model(FloatVal(1.0,next),true);
        std::unique_ptr<FlatZincSpace> adjacent_key=model(1.0,true);
        std::unique_ptr<FlatZincSpace> adjacent_max=
          model(FloatVal(1.0,next),false);
        std::unique_ptr<FlatZincSpace> adjacent_max_key=model(next,false);
        if (!m9 || !m10 || !m10b || !x11 || !x10 || !x10b || !adjacent ||
            !adjacent_key ||
            !adjacent_max || !adjacent_max_key ||
            (m9->compare(*m10) != SC_BETTER) ||
            (m10->compare(*m9) != SC_WORSE) ||
            (m10->compare(*m10b) != SC_EQUIVALENT) ||
            (x11->compare(*x10) != SC_BETTER) ||
            (x10->compare(*x11) != SC_WORSE) ||
            (x10->compare(*x10b) != SC_EQUIVALENT) ||
            (adjacent->compare(*adjacent_key) != SC_EQUIVALENT) ||
            (adjacent_max->compare(*adjacent_max_key) != SC_EQUIVALENT))
          return false;

        const FloatVal probes[] = {FloatVal(8.4), FloatVal(8.5),
                                   FloatVal(8.9), FloatVal(9.0)};
        for (unsigned int i=0; i<sizeof(probes)/sizeof(probes[0]); i++) {
          if (admitted(probes[i],*m10,true,1.0) !=
              admitted(probes[i],*m10b,true,1.0))
            return false;
          if (admitted(probes[i],*m9,true,1.0) &&
              !admitted(probes[i],*m10,true,1.0))
            return false;
        }
        const FloatVal max_probes[] = {FloatVal(11.0), FloatVal(11.1),
                                       FloatVal(11.5), FloatVal(11.6)};
        for (unsigned int i=0;
             i<sizeof(max_probes)/sizeof(max_probes[0]); i++) {
          if (admitted(max_probes[i],*x10,false,1.0) !=
              admitted(max_probes[i],*x10b,false,1.0))
            return false;
          if (admitted(max_probes[i],*x11,false,1.0) &&
              !admitted(max_probes[i],*x10,false,1.0))
            return false;
        }
        std::unique_ptr<FlatZincSpace> zero=model(10.0,true);
        if (admitted(FloatVal(10.0),*zero,true,0.0) ||
            !admitted(FloatVal(9.0),*zero,true,0.0) ||
            admitted(FloatVal(11.0),*x10,false,1.0) ||
            !admitted(FloatVal(11.1),*x10,false,1.0))
          return false;

        FloatVal before=m9->fv[m9->optVar()].val();
        FloatVal before_other=m10->fv[m10->optVar()].val();
        (void) m9->compare(*m10);
        if ((m9->fv[m9->optVar()].val().min() != before.min()) ||
            (m9->fv[m9->optVar()].val().max() != before.max()) ||
            (m10->fv[m10->optVar()].val().min() != before_other.min()) ||
            (m10->fv[m10->optVar()].val().max() != before_other.max()))
          return false;
        std::unique_ptr<FlatZincSpace> different_step=model(10.0,true,0.0);
        try { (void) m10->compare(*different_step); return false; }
        catch (const DynamicCastFailed&) {}
        try { (void) m10->compare(*x10); return false; }
        catch (const DynamicCastFailed&) {}
        return true;
      }
    };

    FloatObjectiveComparison float_objective_comparison;
#endif

    /// Verify that statistics do not override an explicit Gist mode.
    class GistStatisticsMode : public Base {
    private:
      static Gecode::ScriptMode mode(std::vector<std::string> args) {
        Gecode::FlatZinc::FlatZincOptions opt("Gecode/FlatZinc");
        std::string cmd("fzn-gecode");
        int argc = static_cast<int>(args.size()) + 1;
        std::vector<char*> argv(argc);
        argv[0] = const_cast<char*>(cmd.data());
        for (int i=1; i<argc; i++)
          argv[i] = const_cast<char*>(args[i-1].data());
        opt.parse(argc,argv.data());
        return opt.mode();
      }
    public:
      GistStatisticsMode(void)
        : Base("FlatZinc::Options::GistStatisticsMode") {}

      virtual bool run(void) {
        return
          (mode({"-s"}) == Gecode::SM_STAT) &&
          (mode({"-mode", "gist", "-s"}) == Gecode::SM_GIST) &&
          (mode({"-s", "-mode", "gist"}) == Gecode::SM_GIST);
      }
    };

    GistStatisticsMode gist_statistics_mode;

#ifndef GECODE_HAS_GIST
    /// Verify that unavailable Gist mode is rejected instead of running search.
    FlatZincErrorTest gist_unavailable(
      "Options::GistUnavailable",
      "var 1..1: x :: output_var;\nsolve satisfy;\n",
      {"-mode", "gist", "-s"},
      "Gist mode is unavailable in this build");
#endif

  }

  FlatZincTest::FlatZincTest(const std::string& name, const std::string& source,
                             const std::string& expected, bool allSolutions,
                             std::vector<std::string> cmdlineOpt,
                             OutputCheck check, BeforeRun before)
    : Base("FlatZinc::"+name), _name(name), _source(source), _expected(expected),
      _allSolutions(allSolutions), _cmdlineOpt(cmdlineOpt),
      _check(check), _before(before) {}

  FlatZincErrorTest::FlatZincErrorTest(const std::string& name,
                                       const std::string& source,
                                       std::vector<std::string> cmdlineOpt,
                                       std::string expectedMessage)
    : FlatZincTest(name, source, "", false, cmdlineOpt),
      _expectedMessage(expectedMessage) {}

  bool
  FlatZincTest::run(void) {
    using namespace Gecode;
    Support::Timer t_total;
    t_total.start();
    Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
    if (!_cmdlineOpt.empty()) {
      std::string cmd("fzn-gecode");
      int argc = static_cast<int>(_cmdlineOpt.size()) + 1;
      std::vector<char*> argv(argc);
      argv[0] = const_cast<char*>(cmd.data());
      for (int i = 1; i < argc; ++i) {
        argv[i] = const_cast<char*>(_cmdlineOpt[i-1].data());
      }
      fznopt.parse(argc, argv.data());
    }
    fznopt.allSolutions(_allSolutions);
    Gecode::FlatZinc::Printer p;
    try {
      if (_before) {
        _before();
      }
      std::stringstream ss(_source);
      std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> fg(
        Gecode::FlatZinc::parse(ss, p, olog));

      if (fg) {
        fg->createBranchers(p, fg->solveAnnotations(), fznopt,
                            false, olog);
        fg->shrinkArrays(p);
        std::ostringstream os;
        fg->run(os, p, fznopt, t_total);

        const std::string output = os.str();
        fg.reset();
        if (_check ? _check(output) : (_expected == output)) {
          return true;
        }
        if (opt.log)
          olog << "FlatZinc produced the following output:\n" << output << "\n";
        return false;
      } else {
        if (opt.log)
          olog << "Could not parse input\n";
        return false;
      }
    } catch (Gecode::FlatZinc::Error& e) {
      if (opt.log)
        olog << ind(2) << "FlatZinc error : " << e.toString() << std::endl;
      return false;
    }
    return true;
  }

  bool
  FlatZincErrorTest::run(void) {
    using namespace Gecode;
    Support::Timer t_total;
    t_total.start();
    Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
    if (!_cmdlineOpt.empty()) {
      std::string cmd("fzn-gecode");
      int argc = static_cast<int>(_cmdlineOpt.size()) + 1;
      std::vector<char*> argv(argc);
      argv[0] = const_cast<char*>(cmd.data());
      for (int i = 1; i < argc; ++i) {
        argv[i] = const_cast<char*>(_cmdlineOpt[i-1].data());
      }
      fznopt.parse(argc, argv.data());
    }
    Gecode::FlatZinc::Printer p;
    std::ostringstream os;
    try {
      std::stringstream ss(_source);
      std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> fg(
        Gecode::FlatZinc::parse(ss, p, olog));
      if (fg) {
        fg->createBranchers(p, fg->solveAnnotations(), fznopt,
                            false, olog);
        fg->shrinkArrays(p);
        fg->run(os, p, fznopt, t_total);
      }
      return false;
    } catch (Gecode::FlatZinc::Error& e) {
      const std::string message = e.toString();
      if (opt.log)
        olog << ind(2) << "Expected FlatZinc error : "
             << message << std::endl;
      return (os.str().find("----------") == std::string::npos) &&
        (_expectedMessage.empty() ||
         (message.find(_expectedMessage) != std::string::npos));
    } catch (Gecode::Exception& e) {
      const std::string message = e.what();
      if (opt.log)
        olog << ind(2) << "Expected Gecode exception : "
             << message << std::endl;
      return (os.str().find("----------") == std::string::npos) &&
        (_expectedMessage.empty() ||
         (message.find(_expectedMessage) != std::string::npos));
    }
    return false;
  }

}}

// STATISTICS: test-flatzinc
