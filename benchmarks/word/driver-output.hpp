/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
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

#ifndef GECODE_BENCHMARKS_WORD_DRIVER_OUTPUT_HPP
#define GECODE_BENCHMARKS_WORD_DRIVER_OUTPUT_HPP

#include <ostream>
#include <string>
#include <vector>

namespace WordDriver {

  /// Write an arbitrary identifier as a JSON string, including control bytes.
  inline void write_string(std::ostream& out, const std::string& value) {
    const char digits[] = "0123456789abcdef";
    out << '"';
    for (unsigned char c : value) {
      if (c == '"' || c == '\\') {
        out << '\\' << c;
      } else if (c < 0x20) {
        out << "\\u00" << digits[c >> 4] << digits[c & 15];
      } else {
        out << c;
      }
    }
    out << '"';
  }

  template<class Integer>
  void write_rows(std::ostream& out,
                  const std::vector<std::vector<Integer> >& rows) {
    out << '[';
    for (std::size_t i=0; i<rows.size(); i++) {
      if (i != 0) out << ',';
      out << '[';
      for (std::size_t j=0; j<rows[i].size(); j++) {
        if (j != 0) out << ',';
        out << rows[i][j];
      }
      out << ']';
    }
    out << ']';
  }

  template<class Integer>
  void write_scalar_rows(std::ostream& out, const std::vector<Integer>& values) {
    out << '[';
    for (std::size_t i=0; i<values.size(); i++) {
      if (i != 0) out << ',';
      out << '[' << values[i] << ']';
    }
    out << ']';
  }
}
#endif
