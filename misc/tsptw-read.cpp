// Small CLI for exercising the native example input reader.
#include "examples/tsptw-data.hh"
#include <iostream>
int main(int argc, char* argv[]) {
  if (argc != 2) { std::cerr << "usage: tsptw-read INSTANCE\n"; return 2; }
  try {
    TSPTW::Instance d = TSPTW::read(argv[1]);
    std::cout << d.n << ' ' << d.scale << ' ' << d.cost_limit << ' '
              << d.time_limit << '\n';
    for (std::size_t i=0; i<d.cost.size(); i++) std::cout << d.cost[i] << ' ';
    for (int i=0; i<d.n; i++) std::cout << d.ready[i] << ' ' << d.due[i] << ' ';
    std::cout << '\n';
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
