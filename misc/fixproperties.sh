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
#  See the file "LICENSE" for information on usage and
#  redistribution of this file, and for a
#     DISCLAIMER OF ALL WARRANTIES.
#
#

set -e

# List of file extensions for which properties should be set
KEYWORDEXTS="cc icc hh sh perl ac in"

for ext in ${KEYWORDEXTS}; do
    find . -name "*.$ext" ! -wholename './contribs/*' -prune \
    -exec svn propset svn:keywords 'Author Date Id Revision' "{}" ";"
done

