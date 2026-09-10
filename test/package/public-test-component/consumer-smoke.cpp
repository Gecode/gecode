#include <test/int.hh>
#ifdef GECODE_PACKAGE_HAS_SET
#include <test/set.hh>
#endif
#ifdef GECODE_PACKAGE_HAS_FLOAT
#include <test/float.hh>
#endif

#include <gecode/int.hh>
#ifdef GECODE_PACKAGE_HAS_SET
#include <gecode/set.hh>
#endif
#ifdef GECODE_PACKAGE_HAS_FLOAT
#include <gecode/float.hh>
#endif

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

#ifdef GECODE_PACKAGE_HAS_SET
class SingletonSetTest final : public ::Test::Set::SetTest {
public:
  SingletonSetTest()
    : ::Test::Set::SetTest("Package::Singleton", 1, Gecode::IntSet(0, 1)) {}

  bool solution(const ::Test::Set::SetAssignment& assignment) const override {
    return assignment[0] == 1;
  }

  void post(Gecode::Space& home, Gecode::SetVarArray& x,
            Gecode::IntVarArray&) override {
    Gecode::dom(home, x[0], Gecode::SRT_EQ, Gecode::IntSet(0, 0));
  }
} singleton_set_test;
#endif

#ifdef GECODE_PACKAGE_HAS_FLOAT
class FloatEqualityTest final : public ::Test::Float::Test {
public:
  FloatEqualityTest()
    : ::Test::Float::Test("Package::Equality", 2, 0.0, 1.0, 1.0,
                          ::Test::Float::CPLT_ASSIGNMENT, false) {}

  ::Test::Float::MaybeType
  solution(const ::Test::Float::Assignment& assignment) const override {
    return cmp(assignment[0], Gecode::FRT_EQ, assignment[1]);
  }

  void post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
    Gecode::rel(home, x[0], Gecode::FRT_EQ, x[1]);
  }
} float_equality_test;
#endif

} // namespace

int main(int argc, char* argv[]) {
  return Test::run_registered_tests(argc, argv);
}
