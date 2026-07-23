/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
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

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

  bool
  text_file(const std::filesystem::path& path) {
    static const std::set<std::string> extensions = {
      ".c", ".cc", ".cpp", ".h", ".hh", ".hpp",
      ".md", ".txt", ".in", ".cmake", ".sh"
    };
    return extensions.find(path.extension().string()) != extensions.end();
  }

  bool
  word_character(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return std::isalnum(uc) || (c == '_');
  }

  bool
  legacy_role(const std::string& line) {
    const std::string first = std::string("mas") + "ter";
    const std::string second = std::string("sla") + "ve";
    const std::string roles[] = {
      first, second, first + "s", second + "s"
    };
    std::string lower(line);
    std::transform(lower.begin(),lower.end(),lower.begin(),
                   [](unsigned char c) {
                     return static_cast<char>(std::tolower(c));
                   });
    for (const std::string& role : roles) {
      std::string::size_type at = lower.find(role);
      while (at != std::string::npos) {
        const std::string::size_type after = at + role.size();
        if (((at == 0) || !word_character(lower[at-1])) &&
            ((after == lower.size()) || !word_character(lower[after])))
          return true;
        at = lower.find(role,at+1);
      }
    }
    return false;
  }

  void
  scan(const std::filesystem::path& root,
       const std::filesystem::path& relative,
       std::vector<std::string>& result) {
    std::ifstream input(root / relative);
    if (!input) {
      std::cerr << "Cannot read " << (root / relative) << std::endl;
      std::exit(2);
    }
    std::string line;
    unsigned int line_number = 0;
    while (std::getline(input,line)) {
      line_number++;
      if (legacy_role(line))
        result.push_back(relative.generic_string() + ":" +
                         std::to_string(line_number));
    }
  }

}

int
main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " ROOT EXPECTED" << std::endl;
    return 2;
  }

  const std::filesystem::path root =
    std::filesystem::absolute(argv[1]).lexically_normal();
  std::vector<std::filesystem::path> files;
  for (const char* directory : {"gecode", "test", "examples", "docs"}) {
    for (const auto& entry :
           std::filesystem::recursive_directory_iterator(root / directory))
      if (entry.is_regular_file() && text_file(entry.path()))
        files.push_back(std::filesystem::relative(entry.path(),root));
  }
  std::sort(files.begin(),files.end());
  files.push_back("changelog.in");

  std::vector<std::string> actual;
  for (const std::filesystem::path& file : files)
    scan(root,file,actual);

  std::ifstream expected_input(argv[2]);
  if (!expected_input) {
    std::cerr << "Cannot read " << argv[2] << std::endl;
    return 2;
  }
  std::vector<std::string> expected;
  std::string line;
  while (std::getline(expected_input,line))
    expected.push_back(line);

  if (actual == expected)
    return 0;

  std::cerr << "Origin/Variant terminology inventory is stale."
            << std::endl << "Expected:" << std::endl;
  for (const std::string& entry : expected)
    std::cerr << entry << std::endl;
  std::cerr << "Actual:" << std::endl;
  for (const std::string& entry : actual)
    std::cerr << entry << std::endl;
  return 1;
}
