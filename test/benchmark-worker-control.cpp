/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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
 */

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <gecode/search/worker-control.hh>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

  using Clock = std::chrono::steady_clock;
  using Gecode::BoolVarArray;
  using Gecode::IntArgs;
  using Gecode::IntVar;
  using Gecode::IntVarArray;
  using Gecode::MetaInfo;
  using Gecode::Space;
  using Gecode::Search::WorkerControl;
  using Gecode::Search::WorkerControlAccess;

  class EnumerationSpace : public Space {
  public:
    BoolVarArray x;

    explicit EnumerationSpace(unsigned int bits)
      : x(*this,static_cast<int>(bits),0,1) {
      Gecode::branch(*this,x,Gecode::BOOL_VAR_NONE(),
                     Gecode::BOOL_VAL_MIN());
    }

    EnumerationSpace(EnumerationSpace& s)
      : Space(s) {
      x.update(*this,s.x);
    }

    Space* copy(void) override {
      return new EnumerationSpace(*this);
    }
  };

  class OptimizationSpace : public Space {
  public:
    BoolVarArray x;
    IntVar cost;

    explicit OptimizationSpace(unsigned int bits)
      : x(*this,static_cast<int>(bits),0,1),
        cost(*this,0,static_cast<int>(bits)) {
      Gecode::linear(*this,x,Gecode::IRT_EQ,cost);
      Gecode::branch(*this,x,Gecode::BOOL_VAR_NONE(),
                     Gecode::BOOL_VAL_MAX());
    }

    OptimizationSpace(OptimizationSpace& s)
      : Space(s) {
      x.update(*this,s.x);
      cost.update(*this,s.cost);
    }

    Space* copy(void) override {
      return new OptimizationSpace(*this);
    }

    void constrain(const Space& _s) override {
      const OptimizationSpace& s =
        static_cast<const OptimizationSpace&>(_s);
      Gecode::rel(*this,cost,Gecode::IRT_LE,s.cost.val());
    }
  };

  class PropagationSpace : public Space {
  public:
    IntVarArray x;

    explicit PropagationSpace(unsigned int size)
      : x(*this,static_cast<int>(size),0,static_cast<int>(size)-1) {
      Gecode::distinct(*this,x);
      Gecode::branch(*this,x,Gecode::INT_VAR_SIZE_MIN(),
                     Gecode::INT_VAL_MIN());
    }

    PropagationSpace(PropagationSpace& s)
      : Space(s) {
      x.update(*this,s.x);
    }

    Space* copy(void) override {
      return new PropagationSpace(*this);
    }
  };

  class AtomicStop : public Gecode::Search::Stop {
  public:
    std::atomic<bool> requested;

    AtomicStop(void) : requested(false) {}

    bool stop(const Gecode::Search::Statistics&,
              const Gecode::Search::Options&) override {
      return requested.load(std::memory_order_acquire);
    }
  };

  class PortfolioSpace : public Space {
  public:
    IntVar x;

    PortfolioSpace(void)
      : x(*this,0,65535) {
      Gecode::branch(*this,x,Gecode::INT_VAL_MAX());
    }

    PortfolioSpace(PortfolioSpace& s)
      : Space(s) {
      x.update(*this,s.x);
    }

    Space* copy(void) override {
      return new PortfolioSpace(*this);
    }

    bool origin(const MetaInfo&) override {
      return false;
    }

    bool variant(const MetaInfo& mi) override {
      if (mi.type() == MetaInfo::PORTFOLIO) {
        if (mi.asset() == 0U)
          Gecode::rel(*this,x,Gecode::IRT_LQ,32767);
        else
          Gecode::rel(*this,x,Gecode::IRT_GR,32767);
      }
      return true;
    }
  };

  struct Arguments {
    std::string benchmark_case;
    std::string variant;
    unsigned int iterations = 1U;
    unsigned int threads = 4U;
    unsigned int duration_ms = 200U;
  };

  unsigned int
  parse_unsigned(const char* value, const char* option) {
    char* end = nullptr;
    unsigned long parsed = std::strtoul(value,&end,10);
    if ((end == value) || (*end != '\0') || (parsed == 0UL) ||
        (parsed > 100000000UL))
      throw std::runtime_error(std::string("invalid ")+option);
    return static_cast<unsigned int>(parsed);
  }

  Arguments
  parse_arguments(int argc, char* argv[]) {
    Arguments args;
    for (int i=1; i<argc; i++) {
      std::string option(argv[i]);
      if ((option == "--case") && (i+1 < argc))
        args.benchmark_case = argv[++i];
      else if ((option == "--variant") && (i+1 < argc))
        args.variant = argv[++i];
      else if ((option == "--iterations") && (i+1 < argc))
        args.iterations = parse_unsigned(argv[++i],"--iterations");
      else if ((option == "--threads") && (i+1 < argc))
        args.threads = parse_unsigned(argv[++i],"--threads");
      else if ((option == "--duration-ms") && (i+1 < argc))
        args.duration_ms = parse_unsigned(argv[++i],"--duration-ms");
      else
        throw std::runtime_error("unknown or incomplete option: "+option);
    }
    if (args.benchmark_case.empty())
      throw std::runtime_error("--case is required");
    return args;
  }

  bool
  wait_for(const WorkerControl& control, unsigned long long int generation,
           unsigned int leases, std::chrono::seconds timeout) {
    Clock::time_point deadline = Clock::now()+timeout;
    do {
      if ((WorkerControlAccess::observed_generation(control) == generation) &&
          (WorkerControlAccess::leases(control) == leases))
        return true;
      std::this_thread::yield();
    } while (Clock::now() < deadline);
    return false;
  }

  Gecode::Search::Engine*
  make_enumeration_engine(unsigned int bits,
                          Gecode::Search::Options& options) {
    EnumerationSpace* root = new EnumerationSpace(bits);
    Gecode::Search::Engine* engine;
    try {
      engine = Gecode::Search::dfsengine(root,options);
    } catch (...) {
      delete root;
      throw;
    }
    delete root;
    return engine;
  }

  template<class Model>
  Gecode::Search::Engine*
  make_dfs_engine(Model* root, Gecode::Search::Options& options) {
    Gecode::Search::Engine* engine;
    try {
      engine = Gecode::Search::dfsengine(root,options);
    } catch (...) {
      delete root;
      throw;
    }
    delete root;
    return engine;
  }

  std::uint64_t
  enumerate_dfs(unsigned int bits, const Gecode::Search::Options& source) {
    Gecode::Search::Options options(source);
    Gecode::Search::Engine* engine = make_enumeration_engine(bits,options);
    std::uint64_t solutions = 0U;
    while (Space* solution = engine->next()) {
      solutions++;
      delete solution;
    }
    bool ok = !engine->stopped();
    delete engine;
    if (!ok)
      throw std::runtime_error("DFS stopped");
    return solutions;
  }

  int
  optimize_bab(unsigned int bits, const Gecode::Search::Options& source) {
    Gecode::Search::Options options(source);
    OptimizationSpace* root = new OptimizationSpace(bits);
    Gecode::Search::Engine* engine;
    try {
      engine = Gecode::Search::babengine(root,options);
    } catch (...) {
      delete root;
      throw;
    }
    delete root;
    int best = static_cast<int>(bits)+1;
    while (OptimizationSpace* solution =
           static_cast<OptimizationSpace*>(engine->next())) {
      best = solution->cost.val();
      delete solution;
    }
    bool ok = !engine->stopped();
    delete engine;
    if (!ok)
      throw std::runtime_error("BAB stopped");
    return best;
  }

  void
  print_common(const std::string& benchmark_case,
               const std::string& variant, std::uint64_t elapsed_ns,
               bool ok) {
    std::cout << "{\"case\":\"" << benchmark_case
              << "\",\"variant\":\"" << variant
              << "\",\"elapsed_ns\":" << elapsed_ns
              << ",\"ok\":" << (ok ? "true" : "false");
  }

  int
  run_overhead(const Arguments& args, bool bab) {
    const unsigned int bits = bab ? 24U : 17U;
    bool controlled = args.variant == "control";
    if (!controlled && (args.variant != "baseline"))
      throw std::runtime_error("overhead variant must be baseline or control");
    auto options = [&] {
      Gecode::Search::Options result;
      result.threads = static_cast<double>(args.threads);
      if (controlled)
        result.worker_control = WorkerControl(args.threads);
      return result;
    };

    // Warm up allocator, worker creation, and dynamic loader paths.
    {
      Gecode::Search::Options warmup = options();
      if (bab)
        (void) optimize_bab(bits,warmup);
      else
        (void) enumerate_dfs(bits,warmup);
    }

    std::uint64_t solutions = 0U;
    int best = static_cast<int>(bits)+1;
    Clock::time_point start = Clock::now();
    for (unsigned int i=0U; i<args.iterations; i++) {
      Gecode::Search::Options iteration_options = options();
      if (bab)
        best = optimize_bab(bits,iteration_options);
      else
        solutions += enumerate_dfs(bits,iteration_options);
    }
    std::uint64_t elapsed_ns =
      static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
          Clock::now()-start).count());
    std::uint64_t expected =
      static_cast<std::uint64_t>(args.iterations) << bits;
    bool ok = bab ? (best == 0) : (solutions == expected);
    print_common(args.benchmark_case,args.variant,elapsed_ns,ok);
    std::cout << ",\"iterations\":" << args.iterations
              << ",\"threads\":" << args.threads
              << ",\"solutions\":" << solutions
              << ",\"best\":" << best << "}\n";
    return ok ? 0 : 1;
  }

  template<class Model>
  int
  run_latency_model(const Arguments& args, bool grow, Model* root) {
    unsigned int initial = grow ? 1U : args.threads;
    unsigned int requested = grow ? args.threads : 1U;
    WorkerControl control(initial);
    AtomicStop stop;
    Gecode::Search::Options options;
    options.threads = static_cast<double>(args.threads);
    options.worker_control = control;
    options.stop = &stop;
    Gecode::Search::Engine* engine = make_dfs_engine(root,options);
    std::atomic<bool> consumer_done(false);
    std::thread consumer([&] {
      while (Space* solution = engine->next())
        delete solution;
      consumer_done.store(true,std::memory_order_release);
    });

    unsigned long long int initial_generation =
      WorkerControlAccess::generation(control);
    bool initial_ok =
      wait_for(control,initial_generation,initial,std::chrono::seconds(5));
    Clock::time_point start = Clock::now();
    control.request(requested);
    unsigned long long int generation =
      WorkerControlAccess::generation(control);
    bool converged =
      wait_for(control,generation,requested,std::chrono::seconds(5));
    unsigned int converged_leases = WorkerControlAccess::leases(control);
    std::uint64_t latency_ns =
      static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
          Clock::now()-start).count());

    stop.requested.store(true,std::memory_order_release);
    control.request(args.threads);
    consumer.join();
    bool ok = initial_ok && converged &&
      consumer_done.load(std::memory_order_acquire) && engine->stopped();
    unsigned int final_leases = WorkerControlAccess::leases(control);
    delete engine;
    print_common(args.benchmark_case,grow ? "grow" : "shrink",
                 latency_ns,ok);
    std::cout << ",\"latency_ns\":" << latency_ns
              << ",\"initial_limit\":" << initial
              << ",\"requested_limit\":" << requested
              << ",\"generation\":" << generation
              << ",\"converged_leases\":" << converged_leases
              << ",\"shutdown_leases\":" << final_leases << "}\n";
    return ok ? 0 : 1;
  }

  int
  run_latency(const Arguments& args, bool expensive, bool grow) {
    if (expensive)
      return run_latency_model(args,grow,new PropagationSpace(12U));
    return run_latency_model(args,grow,new EnumerationSpace(28U));
  }

  int
  run_parked(const Arguments& args) {
    bool parked_variant = args.variant == "parked";
    if (!parked_variant && (args.variant != "one-worker"))
      throw std::runtime_error(
        "parked variant must be one-worker or parked");
    AtomicStop stop;
    Gecode::Search::Options options;
    options.threads =
      static_cast<double>(parked_variant ? args.threads : 1U);
    WorkerControl control;
    if (parked_variant) {
      control = WorkerControl(1U);
      options.worker_control = control;
    }
    options.stop = &stop;
    Gecode::Search::Engine* engine =
      make_enumeration_engine(28U,options);
    std::thread consumer([&] {
      while (Space* solution = engine->next())
        delete solution;
    });
    bool converged = true;
    if (parked_variant) {
      unsigned long long int generation =
        WorkerControlAccess::generation(control);
      converged =
        wait_for(control,generation,1U,std::chrono::seconds(5));
    }
    std::clock_t cpu_start = std::clock();
    Clock::time_point wall_start = Clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(args.duration_ms));
    std::uint64_t wall_ns =
      static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
          Clock::now()-wall_start).count());
    std::uint64_t cpu_ns =
      static_cast<std::uint64_t>(
        (static_cast<long double>(std::clock()-cpu_start) /
         static_cast<long double>(CLOCKS_PER_SEC)) * 1000000000.0L);
    unsigned int observed_parked =
      parked_variant ? WorkerControlAccess::parked(control) : 0U;
    stop.requested.store(true,std::memory_order_release);
    if (parked_variant)
      control.request(args.threads);
    consumer.join();
    bool ok = converged && engine->stopped();
    delete engine;
    print_common(args.benchmark_case,args.variant,wall_ns,ok);
    std::cout << ",\"cpu_ns\":" << cpu_ns
              << ",\"duration_ms\":" << args.duration_ms
              << ",\"capacity\":" << (parked_variant ? args.threads : 1U)
              << ",\"active_limit\":1"
              << ",\"observed_parked\":" << observed_parked << "}\n";
    return ok ? 0 : 1;
  }

  bool
  settled(const WorkerControl& control, unsigned long long int generation,
          unsigned int leases) {
    Clock::time_point deadline = Clock::now()+std::chrono::seconds(5);
    do {
      if ((WorkerControlAccess::observed_generation(control) == generation) &&
          (WorkerControlAccess::leases(control) == leases) &&
          (WorkerControlAccess::admitted(control) == 0U))
        return true;
      std::this_thread::yield();
    } while (Clock::now() < deadline);
    return false;
  }

  template<class Engine>
  bool
  portfolio_leaf(Engine& engine, WorkerControl& control,
                 unsigned int request, unsigned long long int generation,
                 std::vector<PortfolioSpace*>& results,
                 unsigned int& high_water) {
    WorkerControlAccess::gate_install(
      control,WorkerControlAccess::GATE_ACTION_BEGIN,
      WorkerControlAccess::ALL_WORKERS,request);
    WorkerControlAccess::reset_max_admitted(control,generation);
    std::atomic<bool> ready(false);
    std::atomic<bool> exhausted(false);
    std::thread consumer([&] {
      while (true) {
        PortfolioSpace* result = engine.next();
        if (result == nullptr) {
          exhausted.store(true,std::memory_order_release);
          return;
        }
        results.push_back(result);
        if (ready.load(std::memory_order_acquire))
          return;
      }
    });
    Clock::time_point deadline = Clock::now()+std::chrono::seconds(5);
    unsigned int waiting = 0U;
    do {
      waiting = 0U;
      for (unsigned int worker=0U; worker<control.capacity(); worker++)
        waiting += WorkerControlAccess::gate_waiting(
          control,WorkerControlAccess::GATE_ACTION_BEGIN,worker);
      if (waiting >= request)
        break;
      std::this_thread::yield();
    } while (!exhausted.load(std::memory_order_acquire) &&
             (Clock::now() < deadline));
    high_water = WorkerControlAccess::max_admitted(control);
    ready.store(true,std::memory_order_release);
    WorkerControlAccess::gate_release_all(
      control,WorkerControlAccess::GATE_ACTION_BEGIN);
    consumer.join();
    return !exhausted.load(std::memory_order_acquire) &&
      (waiting >= request) && (high_water <= request) &&
      settled(control,generation,request) &&
      (WorkerControlAccess::completed_generation(control) >= generation);
  }

  template<class Engine>
  bool
  portfolio_phase(Engine& engine, WorkerControl& first,
                  unsigned int first_request, WorkerControl& second,
                  unsigned int second_request,
                  std::vector<PortfolioSpace*>& results,
                  unsigned int& total_high_water) {
    first.request(first_request);
    second.request(second_request);
    unsigned long long int first_generation =
      WorkerControlAccess::generation(first);
    unsigned long long int second_generation =
      WorkerControlAccess::generation(second);
    unsigned int first_high = 0U, second_high = 0U;
    bool first_ok = portfolio_leaf(
      engine,first,first_request,first_generation,results,first_high);
    bool second_ok = portfolio_leaf(
      engine,second,second_request,second_generation,results,second_high);
    total_high_water = first_high+second_high;
    return first_ok && second_ok &&
      (WorkerControlAccess::observed_generation(first) == first_generation) &&
      (WorkerControlAccess::observed_generation(second) ==
       second_generation) &&
      (WorkerControlAccess::leases(first) == first_request) &&
      (WorkerControlAccess::leases(second) == second_request) &&
      (total_high_water <= first_request+second_request);
  }

  int
  run_portfolio(const Arguments& args) {
    if (args.threads != 4U)
      throw std::runtime_error("portfolio benchmark requires four threads");
    WorkerControl first(4U), second(4U);
    Gecode::Search::Options child[2];
    for (unsigned int i=0U; i<2U; i++)
      child[i].threads = 4.0;
    child[0].worker_control = first;
    child[1].worker_control = second;
    Gecode::SEBs builders(2);
    builders[0] = Gecode::dfs<PortfolioSpace>(child[0]);
    builders[1] = Gecode::dfs<PortfolioSpace>(child[1]);
    Gecode::Search::Options outer;
    outer.threads = 2.0;
    PortfolioSpace* root = new PortfolioSpace;
    Gecode::PBS<PortfolioSpace,Gecode::DFS> engine(root,builders,outer);
    delete root;

    static const unsigned int allocation[3][2] = {
      {3U,1U}, {1U,3U}, {2U,2U}
    };
    std::vector<PortfolioSpace*> results;
    unsigned int maximum_total = 0U;
    bool ok = true;
    Clock::time_point start = Clock::now();
    for (unsigned int phase=0U; phase<3U; phase++) {
      unsigned int total = 0U;
      bool phase_ok = portfolio_phase(
        engine,first,allocation[phase][0],second,allocation[phase][1],
        results,total);
      ok = ok && phase_ok && (total <= 4U);
      if (maximum_total < total)
        maximum_total = total;
    }
    std::uint64_t elapsed_ns =
      static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
          Clock::now()-start).count());
    for (PortfolioSpace* result : results)
      delete result;
    print_common(args.benchmark_case,"reallocate",elapsed_ns,ok);
    std::cout << ",\"budget\":4,\"phases\":3"
              << ",\"max_total_admitted\":" << maximum_total
              << ",\"first_capacity\":" << first.capacity()
              << ",\"second_capacity\":" << second.capacity() << "}\n";
    return ok ? 0 : 1;
  }

}

int
main(int argc, char* argv[]) {
  try {
    Arguments args = parse_arguments(argc,argv);
    if (args.benchmark_case == "overhead-dfs")
      return run_overhead(args,false);
    if (args.benchmark_case == "overhead-bab")
      return run_overhead(args,true);
    if (args.benchmark_case == "latency-cheap-shrink")
      return run_latency(args,false,false);
    if (args.benchmark_case == "latency-cheap-grow")
      return run_latency(args,false,true);
    if (args.benchmark_case == "latency-expensive-shrink")
      return run_latency(args,true,false);
    if (args.benchmark_case == "latency-expensive-grow")
      return run_latency(args,true,true);
    if (args.benchmark_case == "parked-cost")
      return run_parked(args);
    if (args.benchmark_case == "portfolio")
      return run_portfolio(args);
    throw std::runtime_error("unknown benchmark case: "+
                             args.benchmark_case);
  } catch (const std::exception& exception) {
    std::cerr << "error: " << exception.what() << '\n';
    return 2;
  }
}
