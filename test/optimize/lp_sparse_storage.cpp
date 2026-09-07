// Structural memory regression: dense materialization cannot fit this allocation
// guard. No timing threshold, RSS comparison or performance claim is involved.
#include <gecode/minimodel/lp-relaxation.hpp>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>

static std::atomic<std::size_t> allocation_limit{std::numeric_limits<std::size_t>::max()};
static std::atomic<std::size_t> largest_allocation{0};
void* operator new(std::size_t size) {
  if (size>allocation_limit.load()) throw std::bad_alloc();
  auto previous=largest_allocation.load();
  while (previous<size && !largest_allocation.compare_exchange_weak(previous,size)) {}
  if (void* memory=std::malloc(size ? size : 1)) return memory;
  throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }

namespace LP=Gecode::Experimental::LpRelaxation;
static void check(bool condition,const char* message) {
  if (!condition) throw std::runtime_error(message);
}
// Compile-only compatibility of legacy untyped empty model/null backend forms,
// exact posting function signatures and derived backend shared pointers.
[[maybe_unused]] static void legacy_calls(Gecode::Home home,const Gecode::IntVarArgs& x,
                                          Gecode::IntVar cost) {
  LP::validate_model({});
  LP::post_native(home,x,cost,{});
  LP::binary_linear_minimize(home,x,cost,nullptr);
  using LegacyPost=void(*)(Gecode::Home,const Gecode::IntVarArgs&,Gecode::IntVar,
                           const std::shared_ptr<LP::Backend>&,LP::Frequency);
  LegacyPost post=&LP::binary_linear_minimize;
  (void) post;
  struct Derived : LP::Backend { using LP::Backend::Backend; };
  const std::shared_ptr<Derived> derived;
  LP::binary_linear_minimize(home,x,cost,derived);
}

class Problem : public Gecode::Space {
public:
  Gecode::IntVarArray x;
  Gecode::IntVar cost;
  Problem(const std::shared_ptr<LP::SparseBackend>& backend)
    : x(*this,static_cast<int>(backend->model.c.size()),0,1),cost(*this,0,x.size()) {
    LP::binary_linear_minimize(*this,x,cost,backend);
  }
  Problem(Problem& other) : Gecode::Space(other) {
    x.update(*this,other.x); cost.update(*this,other.cost);
  }
  Gecode::Space* copy() override { return new Problem(*this); }
};
class IntegerProblem : public Gecode::Space {
public:
  Gecode::IntVarArray x;
  Gecode::IntVar cost;
  IntegerProblem(const std::shared_ptr<LP::BoundedIntegerBackend>& backend)
    : x(*this,static_cast<int>(backend->model.linear.c.size()),-1,1),cost(*this,-x.size(),x.size()) {
    LP::integer_linear_minimize(*this,x,cost,backend);
  }
  IntegerProblem(IntegerProblem& other):Gecode::Space(other) {
    x.update(*this,other.x);cost.update(*this,other.cost);
  }
  Gecode::Space* copy() override {return new IntegerProblem(*this);}
};
int main() {
  try {
    const std::size_t n=16384;
    // A dense int64 array is 2 GiB. All stages must work with allocations
    // below 16 MiB, including CSR backend import and native actor posting.
    allocation_limit.store(16*1024*1024);
    LP::SparseLinearModel model;
    model.row_start.reserve(n+1); model.column.reserve(n); model.a.reserve(n);
    model.b.assign(n,1); model.c.assign(n,1);
    for (std::size_t i=0;i<n;++i) {
      model.column.push_back(i); model.a.push_back(1); model.row_start.push_back(i+1);
    }
    auto backend=std::make_shared<LP::SparseBackend>(std::move(model));
    check(backend->model.nonzeros()==n,"CSR payload is not sparse");
    {
      Problem problem(backend);
      check(problem.status()!=Gecode::SS_FAILED,"large sparse posting failed");
      check(problem.cost.assigned() && problem.cost.val()==static_cast<int>(n),
            "native original sparse rows/objective not enforced");
      for (int j=0;j<problem.x.size();++j)
        check(problem.x[j].assigned() && problem.x[j].val()==1,"native sparse row disappeared");
    }
    check(backend.use_count()==1,"sparse actor retained backend after destruction");
    const auto bound=backend->bound(std::vector<std::int64_t>(n,1),
                                    std::vector<std::int64_t>(n,1),true);
    check(bound.valid && bound.lower_bound==static_cast<std::int64_t>(n),
          "large sparse backend certificate failed");
    auto integer_backend=std::make_shared<LP::BoundedIntegerBackend>(LP::BoundedIntegerModel{
      backend->model,std::vector<std::int64_t>(n,-1),std::vector<std::int64_t>(n,1)});
    {
      IntegerProblem problem(integer_backend);
      check(problem.status()!=Gecode::SS_FAILED && problem.cost.assigned() &&
            problem.cost.val()==static_cast<int>(n),"large sparse integer native posting");
      for(int j=0;j<problem.x.size();++j)
        check(problem.x[j].assigned() && problem.x[j].val()==1,"integer sparse original row disappeared");
    }
    check(integer_backend.use_count()==1,"integer sparse actor retained backend after destruction");
    // Fix the columns before certification, as in the binary case: the LP's
    // bounded work budget is not required to optimize a 16K-row relaxation.
    const auto integer_bound=integer_backend->bound(std::vector<std::int64_t>(n,1),
                                                    std::vector<std::int64_t>(n,1),true);
    check(integer_bound.valid && integer_bound.lower_bound==static_cast<std::int64_t>(n),
          "large sparse integer backend certificate failed");
    const auto largest=largest_allocation.load();
    allocation_limit.store(std::numeric_limits<std::size_t>::max());
    std::cout << "PASS binary and integer 16384x16384 CSR backend/import/native-posting/certificate with "
                 "16 MiB single-allocation guard; largest request " << largest << " bytes\n";
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
