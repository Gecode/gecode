/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#ifndef GECODE_EXAMPLES_TSPTW_DATA_HH
#define GECODE_EXAMPLES_TSPTW_DATA_HH

#include <gecode/int.hh>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/// Travel times, costs, and visit windows for the TSPTW examples.
/// All values use the same integer scale; node 0 is the depot.
namespace TSPTW {
  struct Instance {
    int n;
    int scale;
    // Upper bounds for a complete tour cost and for time including waiting.
    int cost_limit;
    int time_limit;
    std::vector<int> cost;
    std::vector<int> ready;
    std::vector<int> due;
    int arc(int from, int to) const { return cost[from*n+to]; }
  };

  /// Lower bounds on travel time between every pair of nodes.
  /// Paths may use already visited nodes, so they also bound remaining tours
  /// when indirect travel is cheaper than a direct arc.
  inline std::vector<std::int64_t> shortest_paths(const Instance& data) {
    const int n=data.n;
    std::vector<std::int64_t> distance(data.cost.begin(),data.cost.end());
    for (int i=0; i<n; i++)
      distance[i*n+i]=0;
    for (int k=0; k<n; k++)
      for (int i=0; i<n; i++)
        for (int j=0; j<n; j++)
          distance[i*n+j]=std::min(distance[i*n+j],
            distance[i*n+k]+distance[k*n+j]);
    return distance;
  }

  namespace Detail {
    struct Decimal {
      long long value;
      int places;
    };

    inline Decimal decimal(std::string token) {
      const std::string original = token;
      const std::size_t dot = token.find('.');
      if (token.find_first_not_of("0123456789.") != std::string::npos ||
          (dot != std::string::npos && token.find('.',dot+1) != std::string::npos))
        throw std::runtime_error("invalid nonnegative decimal: " + original);
      if (token.size() > 1 && token[0] == '.')
        token.insert(0,"0");
      if (dot != std::string::npos) {
        while (!token.empty() && token.back() == '0')
          token.pop_back();
        if (!token.empty() && token.back() == '.')
          token.pop_back();
      }
      Decimal result = {0, 0};
      bool fraction = false;
      bool digit = false;
      for (std::size_t i=0; i<token.size(); i++) {
        char c = token[i];
        if (c == '.') {
          fraction = true;
          continue;
        }
        digit = true;
        if (result.value > (Gecode::Int::Limits::max-(c-'0'))/10)
          throw std::runtime_error("decimal exceeds Gecode integer limits: " + original);
        result.value = result.value*10 + c-'0';
        if (fraction)
          result.places++;
      }
      if (!digit)
        throw std::runtime_error("invalid decimal: " + original);
      return result;
    }
  }

  /// Read a node count, row-major arc matrix, and one ready/due pair per node.
  /// Decimal values are scaled exactly to integers; comments begin with #.
  /// Arcs supply both travel time and cost, including any service time.
  inline Instance read(std::istream& input) {
    std::vector<std::string> tokens;
    std::string line;
    std::string token;
    while (std::getline(input,line)) {
      std::istringstream row(line.substr(0,line.find('#')));
      while (row >> token)
        tokens.push_back(token);
    }
    if (input.bad() || tokens.empty())
      throw std::runtime_error("cannot read TSPTW instance");
    if (tokens[0].find_first_not_of("0123456789") != std::string::npos)
      throw std::runtime_error("node count must be an integer");
    const long long n = Detail::decimal(tokens[0]).value;
    if (n < 2 || n*n > Gecode::Int::Limits::max ||
        static_cast<unsigned long long>(1+n*n+2*n) != tokens.size())
      throw std::runtime_error("invalid TSPTW dimensions");
    std::vector<Detail::Decimal> values;
    int places = 0;
    for (std::size_t i=1; i<tokens.size(); i++) {
      values.push_back(Detail::decimal(tokens[i]));
      places = std::max(places,values.back().places);
    }
    Instance data;
    data.n = static_cast<int>(n);
    data.scale = 1;
    for (int i=0; i<places; i++) {
      if (data.scale > Gecode::Int::Limits::max/10)
        throw std::runtime_error("decimal scale exceeds Gecode integer limits");
      data.scale *= 10;
    }
    std::vector<int> scaled;
    for (std::size_t i=0; i<values.size(); i++) {
      long long value = values[i].value;
      for (int p=values[i].places; p<places; p++) {
        if (value > Gecode::Int::Limits::max/10)
          throw std::runtime_error("scaled value exceeds Gecode integer limits");
        value *= 10;
      }
      scaled.push_back(static_cast<int>(value));
    }
    data.cost.assign(scaled.begin(),scaled.begin()+n*n);
    for (int i=0; i<n; i++) {
      data.ready.push_back(scaled[n*n+2*i]);
      data.due.push_back(scaled[n*n+2*i+1]);
      if (data.ready[i] > data.due[i])
        throw std::runtime_error("window ready time exceeds due time");
    }
    if (data.ready[0] != 0)
      throw std::runtime_error("nonzero start-depot ready time is unsupported");
    // A tour has n arcs. Waiting can raise the clock to at most the largest
    // due time; all subsequent travel adds at most n times the largest arc.
    // These bounds keep model arithmetic within Gecode integer limits, even
    // for prefixes that will later fail a time-window constraint.
    const long long cost_limit = n * *std::max_element(data.cost.begin(),data.cost.end());
    const long long time_limit = cost_limit + *std::max_element(data.due.begin(),data.due.end());
    if (time_limit > Gecode::Int::Limits::max)
      throw std::runtime_error("tour arithmetic exceeds Gecode integer limits");
    data.cost_limit = static_cast<int>(cost_limit);
    data.time_limit = static_cast<int>(time_limit);
    return data;
  }

  inline Instance read(const std::string& path) {
    std::ifstream input(path.c_str());
    if (!input)
      throw std::runtime_error("cannot open TSPTW instance: " + path);
    return read(input);
  }
}
#endif
