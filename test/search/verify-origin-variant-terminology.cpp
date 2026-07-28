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
#include <sstream>
#include <string>
#include <vector>

namespace {

  struct Occurrence {
    std::string path;
    unsigned int line;
    unsigned int column;
    std::string spelling;
    std::string classification;
    std::string excerpt;
  };

  bool
  word_character(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return std::isalnum(uc) || (c == '_');
  }

  std::string
  escape(const std::string& text) {
    std::string result;
    std::string::size_type end = text.size();
    while ((end > 0) && (text[end-1] == ' '))
      end--;
    for (std::string::size_type i=0; i<end; i++) {
      const char c = text[i];
      switch (c) {
      case '\\': result += "\\\\"; break;
      case '\t': result += "\\t"; break;
      case '\r': result += "\\r"; break;
      default: result += c;
      }
    }
    for (std::string::size_type i=end; i<text.size(); i++)
      result += "\\s";
    return result;
  }

  bool
  excluded_directory(const std::filesystem::path& path) {
    const std::string name = path.filename().string();
    return (name == ".git") || (name == ".zd") ||
      (name == "autom4te.cache") ||
      ((name.size() >= 5) && (name.substr(0,5) == "build"));
  }

  void
  scan(const std::filesystem::path& root,
       const std::filesystem::path& relative,
       std::vector<Occurrence>& result) {
    std::ifstream input(root / relative);
    if (!input) {
      std::cerr << "Cannot read " << (root / relative) << std::endl;
      std::exit(2);
    }
    const std::string first = std::string("mas") + "ter";
    const std::string second = std::string("sla") + "ve";
    const std::string roles[] = {
      first, second, first + "s", second + "s"
    };
    const std::size_t result_start = result.size();
    std::string line;
    unsigned int line_number = 0;
    while (std::getline(input,line)) {
      line_number++;
      if (line.find('\0') != std::string::npos) {
        result.resize(result_start);
        return;
      }
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
              ((after == lower.size()) || !word_character(lower[after]))) {
            Occurrence occurrence = {
              relative.generic_string(), line_number,
              static_cast<unsigned int>(at+1),
              line.substr(at,role.size()), std::string(), escape(line)
            };
            result.push_back(occurrence);
          }
          at = lower.find(role,at+1);
        }
      }
    }
  }

  std::string
  format(const Occurrence& occurrence) {
    std::ostringstream result;
    result << occurrence.path << ":" << occurrence.line << ":"
           << occurrence.column << "\t" << occurrence.spelling << "\t"
           << occurrence.classification << "\t" << occurrence.excerpt;
    return result.str();
  }

  bool
  parse(const std::string& row, Occurrence& occurrence) {
    static const std::set<std::string> classifications = {
      "bibliography", "design-rationale", "historical-changelog",
      "historical-migration", "negative-api-build-check",
      "negative-api-fixture", "third-party", "unrelated-domain",
      "unrelated-scheduler"
    };
    std::string::size_type first_tab = row.find('\t');
    std::string::size_type second_tab =
      (first_tab == std::string::npos) ? first_tab :
      row.find('\t',first_tab+1);
    std::string::size_type third_tab =
      (second_tab == std::string::npos) ? second_tab :
      row.find('\t',second_tab+1);
    if ((first_tab == std::string::npos) ||
        (second_tab == std::string::npos) ||
        (third_tab == std::string::npos))
      return false;
    std::string location = row.substr(0,first_tab);
    std::string::size_type last_colon = location.rfind(':');
    std::string::size_type line_colon =
      (last_colon == std::string::npos) ? last_colon :
      location.rfind(':',last_colon-1);
    if ((last_colon == std::string::npos) ||
        (line_colon == std::string::npos))
      return false;
    occurrence.path = location.substr(0,line_colon);
    occurrence.line = static_cast<unsigned int>(
      std::strtoul(location.substr(line_colon+1,
                                   last_colon-line_colon-1).c_str(),
                   nullptr,10));
    occurrence.column = static_cast<unsigned int>(
      std::strtoul(location.substr(last_colon+1).c_str(),nullptr,10));
    occurrence.spelling =
      row.substr(first_tab+1,second_tab-first_tab-1);
    occurrence.classification =
      row.substr(second_tab+1,third_tab-second_tab-1);
    occurrence.excerpt = row.substr(third_tab+1);
    return (occurrence.line != 0U) && (occurrence.column != 0U) &&
      !occurrence.spelling.empty() && !occurrence.classification.empty() &&
      !occurrence.excerpt.empty() &&
      (classifications.find(occurrence.classification) !=
       classifications.end());
  }

  bool
  same_evidence(const Occurrence& actual, const Occurrence& expected) {
    return (actual.path == expected.path) &&
      (actual.line == expected.line) &&
      (actual.column == expected.column) &&
      (actual.spelling == expected.spelling) &&
      (actual.excerpt == expected.excerpt);
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
  const std::filesystem::path expected_path =
    std::filesystem::absolute(argv[2]).lexically_normal();
  std::vector<std::filesystem::path> files;
  std::filesystem::recursive_directory_iterator iterator(root), end;
  while (iterator != end) {
    if (iterator->is_directory() && excluded_directory(iterator->path())) {
      iterator.disable_recursion_pending();
    } else if (iterator->is_regular_file()) {
      const std::filesystem::path path =
        std::filesystem::absolute(iterator->path()).lexically_normal();
      const std::filesystem::path relative =
        std::filesystem::relative(path,root);
      if ((relative != std::filesystem::path("Makefile")) &&
          (path != expected_path) &&
          (relative !=
           std::filesystem::path(
             "test/search/origin-variant-terminology.expected")))
        files.push_back(relative);
    }
    ++iterator;
  }
  std::sort(files.begin(),files.end());

  std::vector<Occurrence> actual;
  for (const std::filesystem::path& file : files)
    scan(root,file,actual);
  std::sort(actual.begin(),actual.end(),
            [](const Occurrence& a, const Occurrence& b) {
              if (a.path != b.path)
                return a.path < b.path;
              if (a.line != b.line)
                return a.line < b.line;
              return a.column < b.column;
            });

  if (std::string(argv[2]) == "-") {
    for (Occurrence occurrence : actual) {
      occurrence.classification = "UNCLASSIFIED";
      std::cout << format(occurrence) << std::endl;
    }
    return 0;
  }

  std::ifstream expected_input(expected_path);
  if (!expected_input) {
    std::cerr << "Cannot read " << expected_path << std::endl;
    return 2;
  }
  std::vector<Occurrence> expected;
  std::string row;
  unsigned int row_number = 0;
  while (std::getline(expected_input,row)) {
    row_number++;
    Occurrence occurrence;
    if (!parse(row,occurrence)) {
      std::cerr << "Invalid terminology inventory row "
                << row_number << std::endl;
      return 2;
    }
    expected.push_back(occurrence);
  }

  bool matches = actual.size() == expected.size();
  if (matches)
    for (std::size_t i=0; i<actual.size(); i++)
      if (!same_evidence(actual[i],expected[i])) {
        matches = false;
        break;
      }
  if (matches)
    return 0;

  std::cerr << "Origin/Variant terminology inventory is stale."
            << std::endl << "Expected:" << std::endl;
  for (const Occurrence& occurrence : expected)
    std::cerr << format(occurrence) << std::endl;
  std::cerr << "Actual (classification required):" << std::endl;
  for (Occurrence occurrence : actual) {
    occurrence.classification = "UNCLASSIFIED";
    std::cerr << format(occurrence) << std::endl;
  }
  return 1;
}

// STATISTICS: test-search
