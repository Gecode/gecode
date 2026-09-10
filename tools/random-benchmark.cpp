// Copyright (c) 2026 Mikael Zayenz Lagerkvist. MIT license; see LICENSE.
// Compile against main without RANDOM_NEW, or against feature/random with it.
#include <gecode/int.hh>
#include <gecode/search.hh>
#include <chrono>
#include <iostream>
#include <memory>

using namespace Gecode;
using Clock = std::chrono::steady_clock;

template<class F>
void measure(const char* name, uint64_t operations, F work) {
  auto start=Clock::now();
  uint64_t checksum=work();
  double ns=std::chrono::duration<double,std::nano>(Clock::now()-start).count();
  std::cout << name << '\t' << ns/operations << "\tns/op\t" << checksum << '\n';
}

class Model : public Space {
public:
  IntVarArray x;
  Model(unsigned int n, unsigned int seed, bool queens, bool random)
    : x(*this,n,0,queens ? n-1 : 1) {
    if (queens) {
      distinct(*this,x,IPL_DOM);
      IntArgs up(n),down(n);
      for (unsigned int i=0; i<n; ++i) { up[i]=i; down[i]=-int(i); }
      distinct(*this,up,x,IPL_DOM);
      distinct(*this,down,x,IPL_DOM);
    }
    branch(*this,x,INT_VAR_SIZE_MIN(),random ? INT_VAL_RND(Rnd(seed)) : INT_VAL_MIN());
  }
  Model(Model& s) : Space(s) { x.update(*this,s.x); }
  Space* copy() override { return new Model(*this); }
};

int main(int argc, char** argv) {
  const uint64_t draws=argc>1 ? std::stoull(argv[1]) : 1000000;
  const unsigned int seed=argc>2 ? std::stoul(argv[2]) : 42;
  if (!draws) return 1;
  std::cout << "size.engine\t" << sizeof(Support::RandomGenerator) << "\tbytes\t0\n"
            << "size.space\t" << sizeof(Space) << "\tbytes\t0\n"
            << "size.choice\t" << sizeof(PosValChoice<int>) << "\tbytes\t0\n";
  Support::RandomGenerator raw(seed);
  measure("raw.default",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) sum ^= raw.next();
    return sum;
  });
  Support::RandomGenerator bounded(seed);
  measure("bounded.default",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) sum += bounded(17U);
    return sum;
  });
  Rnd handle(seed);
  measure("bounded.handle",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) sum += handle(17U);
    return sum;
  });
#ifdef RANDOM_NEW
  Support::Random<Support::Xorshift64Star> xs(seed);
  measure("raw.xorshift64star",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) sum ^= xs.next();
    return sum;
  });
  measure("split.indexed",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) {
      raw=raw.split(uint32_t(i & 1));
      sum ^= raw.next();
    }
    return sum;
  });
  // Warm the shared jump table before measuring xorshift splitting.
  (void) xs.split(0);
  measure("split.xorshift64star",draws,[&] {
    uint64_t sum=0;
    for (uint64_t i=0; i<draws; ++i) {
      xs=xs.split(uint32_t(i & 1));
      sum ^= xs.next();
    }
    return sum;
  });
#endif
  for (bool random : {false,true}) {
    Model root(14,seed,false,random);
    root.status();
    std::unique_ptr<const Choice> choice(root.choice());
    Archive archive;
    choice->archive(archive);
    std::cout << (random ? "size.random_archive" : "size.plain_archive")
              << '\t' << archive.size()*sizeof(unsigned int) << "\tbytes\t0\n";
#ifdef RANDOM_NEW
    std::cout << (random ? "size.random_snapshot" : "size.plain_snapshot")
              << '\t' << (archive[1] ? (archive[1]+1)*sizeof(uint64_t) : 0)
              << "\tbytes\t0\n";
#endif
    measure(random ? "clone.random" : "clone.plain",10000,[&] {
      uint64_t sum=0;
      for (int i=0; i<10000; ++i) {
        std::unique_ptr<Space> copy(root.clone());
        sum += copy->status();
      }
      return sum;
    });
    for (unsigned int distance : {1U,16U}) {
      Search::Options options;
      options.c_d=distance;
      options.a_d=distance;
      const char* name=random ? (distance==1 ? "tree.random.clone" : "tree.random.recompute")
                              : (distance==1 ? "tree.plain.clone" : "tree.plain.recompute");
      // Complete binary tree: exactly 32767 nodes for every engine/seed.
      measure(name,32767,[&] {
        DFS<Model> search(&root,options);
        uint64_t solutions=0;
        while (std::unique_ptr<Model> s{search.next()}) ++solutions;
        if (solutions!=16384 || search.statistics().node!=32767)
          throw std::runtime_error("Controlled tree changed");
        return solutions;
      });
    }
  }
  Model queens(10,seed,true,true);
  Search::Options options;
  uint64_t nodes=0;
  measure("queens.random",1,[&] {
    DFS<Model> search(&queens,options);
    uint64_t solutions=0;
    while (std::unique_ptr<Model> s{search.next()}) ++solutions;
    nodes=search.statistics().node;
    if (solutions!=724) throw std::runtime_error("Queens solutions changed");
    return solutions;
  });
  std::cout << "queens.nodes\t" << nodes << "\tnodes\t0\n";
}
