/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

/*
 * gecode-instance: read instance files with reader specs
 *
 */

#include <gecode/instance.hh>

#include <cstring>
#include <iostream>

using namespace Gecode;

namespace {

  void
  usage(std::ostream& os) {
    os << "usage: gecode-instance -spec SPEC [options] FILE...\n"
       << "\n"
       << "Reads instance files with the reader spec in file SPEC and\n"
       << "prints the data they contain.\n"
       << "\n"
       << "options:\n"
       << "  -spec SPEC      file with the reader spec\n"
       << "  -summary        print names, types, and shapes (default)\n"
       << "  -dzn            print the data as MiniZinc data\n"
       << "  -list           list the instances in each file, and the\n"
       << "                  first failure of each invalid one\n"
       << "  -param N=V      give value V to spec parameter N; repeat,\n"
       << "                  or separate by commas, for several; use\n"
       << "                  instance=K or instance=NAME to select an\n"
       << "                  instance in a file with several\n"
       << "  -check SPEC     check that spec file SPEC parses\n"
       << "  -help           print this help\n";
  }

  /// Report the usage error \a m, and return 2: usage errors exit with 2,
  /// errors in specs and instances with 1
  int
  usage_error(const std::string& m) {
    std::cerr << "gecode-instance: " << m << "\n";
    usage(std::cerr);
    return 2;
  }

  /// Print the type and shape of entry \a n of \a d
  void
  print_shape(std::ostream& os, const Instance::Data& d, const std::string& n) {
    Instance::ValueType t = d.type(n);
    const char* tn = (t == Instance::ValueType::INT) ? "int" : ((t == Instance::ValueType::FLOAT) ? "float" : "string");
    switch (d.depth(n)) {
    case 0:
      os << tn << " ";
      if (t == Instance::ValueType::INT) os << d.integer(n);
      else if (t == Instance::ValueType::FLOAT) os << d.number(n);
      else os << '"' << d.text(n) << '"';
      return;
    case 1:
      os << tn << " array " << d.size(n);
      return;
    default: {
      // Report the range of row lengths for the second level
      int rows = d.size(n), lo = 0, hi = 0;
      for (int i=0; i<rows; i++) {
        int l = d.size(n, {i});
        if ((i == 0) || (l < lo)) lo = l;
        if ((i == 0) || (l > hi)) hi = l;
      }
      os << tn << " array " << rows << " x " << lo;
      if (hi != lo) os << ".." << hi;
      for (int k=2; k<d.depth(n); k++)
        os << " x ...";
      return;
    }
    }
  }

  /// Return option \a a with a leading -- shortened to -
  std::string
  option(const char* a) {
    std::string o(a);
    if ((o.size() > 2) && (o[0] == '-') && (o[1] == '-'))
      o = o.substr(1);
    return o;
  }

  /// Test whether option \a o takes a value
  bool
  valued(const std::string& o) {
    return (o == "-spec") || (o == "-param") || (o == "-check");
  }

  /// Test whether argument \a o is a file rather than an option
  bool
  is_file(const std::string& o) {
    return (o.size() <= 1) || (o[0] != '-');
  }

  /// Read or list file \a f with \a spec in \a mode
  void
  process(const std::string& f, const Instance::Spec& spec, const Instance::Params& params, const std::string& mode) {
    if (mode == "list") {
      Instance::Data c = spec.catalog(f, params);
      int n = static_cast<int>(c.integer("instances"));
      std::cout << f << ": " << n << " instance" << (n == 1 ? "" : "s") << "\n";
      // Every instance if they have names, otherwise those that failed
      bool names = c.has("name"), failed = c.has("failed");
      for (int i=0; (names || failed) && (i<n); i++) {
        std::string m = failed ? c.text("failed", {i}) : std::string();
        if (!names && m.empty())
          continue;
        std::cout << "  " << (i+1);
        if (names)
          std::cout << " " << c.text("name", {i});
        if (!m.empty())
          std::cout << " (failed: " << m << ")";
        std::cout << "\n";
      }
      return;
    }
    Instance::Data d = spec.read(f, params);
    if (mode == "dzn") {
      d.dzn(std::cout);
      return;
    }
    std::cout << f;
    if (!spec.name().empty())
      std::cout << " [" << spec.name() << "]";
    std::cout << "\n";
    for (int n=0; n<d.entries(); n++) {
      std::cout << "  " << d.name(n) << ": ";
      print_shape(std::cout, d, d.name(n));
      std::cout << "\n";
    }
  }

}

int
main(int argc, char* argv[]) {
  std::string mode = "summary", spec_file;
  Instance::Params params;
  int files = 0;
  // All options apply to all files, wherever they are given
  for (int i=1; i<argc; i++) {
    std::string o = option(argv[i]);
    if (valued(o) && (i+1 >= argc))
      return usage_error("option " + o + " needs a value");
    if ((o == "-summary") || (o == "-dzn") || (o == "-list")) {
      mode = o.substr(1);
    } else if (o == "-spec") {
      spec_file = argv[++i];
    } else if (o == "-param") {
      try {
        params.set(argv[++i]);
      } catch (const Instance::Error& e) {
        return usage_error(std::string("-param: ") + e.what());
      }
    } else if (o == "-check") {
      files++; i++;
    } else if (o == "-help") {
      usage(std::cout);
      return 0;
    } else if (!is_file(o)) {
      return usage_error(std::string("unknown option ") + argv[i]);
    } else {
      files++;
    }
  }
  if (files == 0)
    return usage_error("no files given");
  try {
    Instance::Spec spec;
    bool loaded = false;
    for (int i=1; i<argc; i++) {
      std::string o = option(argv[i]);
      if (o == "-check") {
        std::string f = argv[++i];
        Instance::Spec c = Instance::Spec::load(f);
        std::cout << f << ": ok";
        if (!c.name().empty())
          std::cout << " (format " << c.name() << ")";
        std::cout << "\n";
      } else if (valued(o)) {
        i++;
      } else if (is_file(o)) {
        if (spec_file.empty())
          return usage_error("no reader spec given (use -spec SPEC)");
        if (!loaded) {
          spec = Instance::Spec::load(spec_file);
          loaded = true;
        }
        process(argv[i], spec, params, mode);
      }
    }
  } catch (const Instance::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    // For example, running out of memory
    std::cerr << "gecode-instance: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

// STATISTICS: instance-any
