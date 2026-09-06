/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2026
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

#include "test/test.hh"
#include "test/gecode-tags.hh"

#include <string>

namespace Test {

  /// Patterns that reproduce the historic make check selection
  static const char* const check_patterns[] = {
    "Branch::Int::Dense::3",
    "FlatZinc::Options",
    "FlatZinc::magic_square",
    "FlatZinc::blackbox",
    "Float::Arithmetic::PositiveNRootBounds",
    "Float::Arithmetic::PowConsistency",
    "Float::Arithmetic::MultZeroEndpoint",
    "Float::Arithmetic::Pow::N::2::XY::Sol::C",
    "Float::Arithmetic::NRoot::N::2::XY::Sol::C",
    "Float::Arithmetic::Mult::XYZ::Sol::C",
    "Int::Arithmetic::Abs",
    "Int::Arithmetic::ArgMax",
    "Int::Arithmetic::Max::Nary",
    "Int::Cumulative::Man::Fix::0::4",
    "Int::Distinct::Random",
    "Int::Extensional::TupleSet::Sparse::IncrementalDelta",
    "Int::Extensional::TupleSet::Auto::DefaultDispatch",
    "Int::Linear::Bool::Int::Lq",
    "Int::MiniModel::LinExpr::Bool::352",
    "NoGoods::Queens",
    "Search::DFS::Sol::Binary::Nary::Binary::1::1::1",
    "Set::Dom::Dom::Gr",
    "Set::RelOp::ConstSSI::Union",
    "Set::Sequence::SeqU1",
    "Set::Wait",
    nullptr
  };

  /// Representative cases retained from otherwise exhaustive sweep families
  static const char* const normal_patterns[] = {
    "Float::Arithmetic::Abs::XX::A",
    "Float::Arithmetic::Div::A",
    "Float::Arithmetic::Max::Bin::XXX::A",
    "Float::Arithmetic::Min::Bin::XXX::A",
    "Float::Arithmetic::Sqr::XX::A",
    "Float::Arithmetic::Sqrt::XX::A",
    "Float::Linear::Float::Eq::11::0::1",
    "Float::Linear::Var::Eq::11::1",
    "Float::MiniModel::LinExpr::000",
    "Float::Transcendental::Exp::XX::A",
    "Float::Transcendental::Log::XX::A",
    "Float::Transcendental::Pow::N::1.5::XX::A",
    "Float::Trigonometric::ACos::XX::A",
    "Float::Trigonometric::ASin::XX::A",
    "Float::Trigonometric::ATan::XX::A",
    "Float::Trigonometric::Cos::XX::A",
    "Float::Trigonometric::Sin::XX::A",
    "Float::Trigonometric::Tan::XX::A",
    "Int::Arithmetic::Nroot::XX::1::Bnd::A",
    "Int::Arithmetic::Pow::XX::0::Bnd::A",
    "Int::Channel::Bool::Multi::A",
    "Int::Circuit::Cost::Dom::4::0",
    "Int::Count::Distinct::Bnd::Dense",
    "Int::Cumulative::Opt::Fix::-2147483646::-1",
    "Int::Cumulative::Opt::Flex::-2147483646::4::0::2",
    "Int::Distinct::Bnd::Dense",
    "Int::Distinct::Dom::Dense",
    "Int::Distinct::Offset::Dense::Bnd",
    "Int::GCC::Int::All::Max::Bnd",
    "Int::Linear::Int::Int::Eq::Bnd::11::0::1",
    "Int::MiniModel::SetExpr::Const::000::0::0",
    "Int::MiniModel::SetExpr::Expr::000::000::0",
    "Int::NValues::Int::Int::Eq::1::0",
    "Int::NoOverlap::Int::2::2::[1,1,1,1]::[1,1,1,1]",
    "Int::Path::Cost::Dom::3::0",
    "Int::Rel::Int::Array::Eq::0::4",
    "Int::Unary::Man::Fix::-2147483646::[2,2,0,2,2]::Def+A",
    "Int::Unary::Man::Flex::-2147483646::4::0::2::Def+A",
    "Int::Unary::Opt::Fix::-2147483646::[2,2,0,2,2]::Def+A",
    "Int::Unary::Opt::Flex::-2147483646::4::0::2::Def+A",
    "Search::BAB::Sol::BalGr::Binary::Binary::Binary::1::1::1",
    "Set::Branch::Dense::3",
    "Set::Channel::Bool::1",
    "Set::Element::Disjoint",
    "Set::Precede::Multi::[1,2,3]",
    "Set::Rel::Bin::Cmpl::S0",
    "Set::RelOp::ConstISI::DUnion::Cmpl::0::0",
    nullptr
  };

  /// Patterns for tests that are too heavy for the normal suite
  static const char* const sweep_patterns[] = {
    "FlatZinc::oss",
    "FlatZinc::packing",
    "FlatZinc::radiation",
    "FlatZinc::steiner_triples",
    "FlatZinc::template_design",
    "FlatZinc::tenpenki",
    "FlatZinc::timetabling",
    "FlatZinc::trucking",
    "Float::Arithmetic",
    "Float::Linear::Float",
    "Float::Linear::Var",
    "Float::MiniModel::LinExpr",
    "Float::Transcendental",
    "Float::Trigonometric",
    "Int::Arithmetic::Nroot",
    "Int::Arithmetic::Pow",
    "Int::Channel",
    "Int::Circuit",
    "Int::Count::Distinct",
    "Int::Cumulative::Man",
    "Int::Cumulative::Opt",
    "Int::Distinct::Bnd",
    "Int::Distinct::Dom",
    "Int::Distinct::Offset",
    "Int::Distinct::Pathological",
    "Int::Extensional::TupleSet",
    "Int::GCC",
    "Int::Linear::Bool",
    "Int::Linear::Int",
    "Int::MiniModel::LinExpr",
    "Int::MiniModel::SetExpr",
    "Int::NValues::Int",
    "Int::NoOverlap",
    "Int::Path",
    "Int::Rel::Int",
    "Int::Unary",
    "Search::BAB::Sol",
    "Search::DFS::Sol",
    "Set::Branch",
    "Set::Channel",
    "Set::Dom",
    "Set::Element",
    "Set::Precede",
    "Set::Rel",
    "Set::RelOp",
    nullptr
  };

  static bool
  matches_any_pattern(const std::string& name,
                      const char* const patterns[]) {
    for (int i=0; patterns[i] != nullptr; i++)
      if (name.find(patterns[i]) != std::string::npos)
        return true;
    return false;
  }

  void
  apply_gecode_test_tags(void) {
    for (Base* test = Base::tests(); test != nullptr;
         test = test->next()) {
      const std::string& name = test->name();
      const bool check = matches_any_pattern(name, check_patterns);
      const bool representative = matches_any_pattern(name, normal_patterns);
      if (!check && !representative &&
          matches_any_pattern(name, sweep_patterns)) {
        test->remove_tags(TestTag::normal);
        test->add_tags(TestTag::sweep);
      }
      if (check)
        test->add_tags(TestTag::check);
    }
  }

}

// STATISTICS: test-core
