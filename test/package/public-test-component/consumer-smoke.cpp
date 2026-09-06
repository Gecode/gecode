#include <test/int.hh>

#include <gecode/int.hh>

namespace {

class ConsumerSmoke final : public ::Test::Int::Test {
public:
  ConsumerSmoke()
    : ::Test::Int::Test("Package::ConsumerSmoke", 1, 0, 1) {}

  bool solution(const ::Test::Int::Assignment& assignment) const override {
    return assignment[0] >= 0;
  }

  void post(Gecode::Space& home, Gecode::IntVarArray& x) override {
    Gecode::rel(home, x[0], Gecode::IRT_GQ, 0);
  }
} consumer_smoke;

} // namespace

int main(int argc, char* argv[]) {
  return Test::run_registered_tests(argc, argv);
}
