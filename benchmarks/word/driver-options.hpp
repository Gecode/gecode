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

#ifndef GECODE_BENCHMARKS_WORD_DRIVER_OPTIONS_HPP
#define GECODE_BENCHMARKS_WORD_DRIVER_OPTIONS_HPP

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace WordDriver {

  /// Read the next argument, rejecting options without a value.
  inline const char*
  consume_value(int& i, int argc, char* argv[]) {
    if (i+1 >= argc)
      throw std::invalid_argument(std::string("missing value for ")+argv[i]);
    return argv[++i];
  }

  /// Parse a complete decimal unsigned value, without a sign or whitespace.
  inline unsigned long long
  parse_unsigned(const char* option, const char* value) {
    for (const char* p=value; *p != '\0'; p++) {
      if ((*p < '0') || (*p > '9'))
        throw std::invalid_argument(std::string("invalid value for ")+option);
    }
    errno=0;
    char* end=nullptr;
    const unsigned long long result=std::strtoull(value,&end,10);
    const bool is_valid=(errno == 0) && (end != value) && (*end == '\0');
    if (!is_valid)
      throw std::invalid_argument(std::string("invalid value for ")+option);
    return result;
  }

  /// Parse a complete decimal signed value, allowing only a leading minus.
  inline long long
  parse_signed(const char* option, const char* value) {
    const char* digits=(*value == '-') ? value+1 : value;
    for (const char* p=digits; *p != '\0'; p++) {
      if ((*p < '0') || (*p > '9'))
        throw std::invalid_argument(std::string("invalid value for ")+option);
    }
    errno=0;
    char* end=nullptr;
    const long long result=std::strtoll(value,&end,10);
    const bool is_valid=(errno == 0) && (end != value) && (*end == '\0');
    if (!is_valid)
      throw std::invalid_argument(std::string("invalid value for ")+option);
    return result;
  }
}
#endif
