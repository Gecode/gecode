#!/bin/sh
#
#  Main authors:
#     Guido Tack <tack@gecode.org>
#
#  Copyright:
#     Guido Tack, 2006
#
#  Last modified:
#     $Date$ by $Author$
#     $Revision$
#
#  This file is part of Gecode, the generic constraint
#  development environment:
#     http://www.gecode.org
#
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
#  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#

set -e

# List of file extensions for which properties should be set
KEYWORDEXTS="cpp hpp hh sh perl ac in vsl vis mzn m4"

for ext in ${KEYWORDEXTS}; do
    find . -name "*.$ext" ! -path './contribs/*' -prune \
    -exec svn propset svn:keywords 'Author Date Id Revision' "{}" ";"
done

# List of normal directories to set ignore properties on
GECODEDIRS="int iter kernel minimodel search set support gist driver graph scheduling flatzinc"
TOOLSDIRS="flatzinc"

for dir in ${GECODEDIRS}; do
    find . -type d -path "./gecode/$dir*" \
    ! -path '*.svn*' \
    -exec svn propset svn:ignore -F misc/svn-ignore.txt '{}' \;
done
for dir in ${TOOLSDIRS}; do
    find . -type d -path "./tools/$dir*" \
    ! -path '*.svn*' \
    -exec svn propset svn:ignore -F misc/svn-ignore.txt '{}' \;
done

svn propset svn:ignore -F misc/svn-ignore-root.txt '.'

IGNORESUPPORT=`mktemp ignoresupportXXXX` || exit 1
(echo config.hpp; svn propget svn:ignore './gecode/support') \
  > ${IGNORESUPPORT}
svn propset svn:ignore -F ${IGNORESUPPORT} './gecode/support'
rm -f ${IGNORESUPPORT}

# Create list of executable examples, append it to svn-ignore.txt
IGNOREEXAMPLES=`mktemp ignoreexamplesXXXX` || exit 1
(cat misc/svn-ignore.txt; \
  find "./examples" -name "*.cpp" | sed -e 's/\.cpp//g' | xargs -n1 basename) \
  > ${IGNOREEXAMPLES}
svn propset svn:ignore -F ${IGNOREEXAMPLES} './examples'
rm -f ${IGNOREEXAMPLES}

# Create list of executable tools, append it to svn-ignore.txt
TOOLSDIRS=`find tools -type d -and -not -path '*.svn*' -and -not -path 'tools'`
for t in $TOOLSDIRS; do
IGNORETOOLS=`mktemp ignoretoolsXXXX` || exit 1
(cat misc/svn-ignore.txt; \
  find "$t" -name "*.cpp" | sed -e 's/\.cpp//g' | xargs -n1 basename) \
  > ${IGNORETOOLS}
svn propset svn:ignore -F ${IGNORETOOLS} $t
rm -f ${IGNORETOOLS}
done

# Append the test executable to svn-ignore.txt
IGNORETEST=`mktemp ignoretestXXXX` || exit 1
(cat misc/svn-ignore.txt; echo "test") > ${IGNORETEST}
svn propset svn:ignore -F ${IGNORETEST} './test'
rm -f ${IGNORETEST}

find . -type d -path "./test/*" \
    ! -path '*.svn*' \
    -exec svn propset svn:ignore -F misc/svn-ignore.txt '{}' \;
