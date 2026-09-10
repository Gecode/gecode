/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
// Copyright (c) 2026 Mikael Zayenz Lagerkvist. MIT license; see LICENSE.
// Deliberately failing fixtures for the test runner's state replay protocol.
#include "test/test.hh"

namespace {
  class Replay : public Test::Base {
    bool exception;
  public:
    explicit Replay(bool e)
      : Base(e ? "Random::Replay::Exception" : "Random::Replay::Failure"),
        exception(e) {}
    bool run() override {
      if (_rand(4) != 0)
        return true;
      auto child = _rand.split(7);
      std::cout << "Replay draw: " << child.next() << '\n';
      if (exception)
        throw Gecode::Exception("Random::Replay", "deliberate exception");
      return false;
    }
  } failure(false), exception(true);
}
