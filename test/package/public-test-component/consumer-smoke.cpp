#include <test/int.hh>

#include <gecode/int.hh>

namespace {

void post_equal(Gecode::Space& home, Gecode::IntVar x, Gecode::IntVar y) {
  Gecode::rel(home, x, Gecode::IRT_EQ, y);
}

class EqualityTest final : public ::Test::Int::Test {
public:
  EqualityTest()
    : ::Test::Int::Test("Package::Equality", 2, 0, 1) {}

  bool solution(const ::Test::Int::Assignment& assignment) const override {
    return assignment[0] == assignment[1];
  }

  void post(Gecode::Space& home, Gecode::IntVarArray& x) override {
    post_equal(home, x[0], x[1]);
  }
} equality_test;

} // namespace

int main(int argc, char* argv[]) {
  return Test::run_registered_tests(argc, argv);
}
