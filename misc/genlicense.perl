#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2005
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

# 
# Get authors of all files
#

%main = ();
%fix  = ();

while ($l = <>) {
  if (($l =~ /Main authors:/) || ($l =~ /Contributing authors:/)) {
    while (($l = <>) && ($l =~ /([A-Z].*) <(.*)>/)) {
      $main{$1} = $2;
    }
  } elsif ($l =~ /provided by:/) {
    while (($l = <>) && ($l =~ /([A-Z][^<]*)/)) {
      $fix{$1} = "";
    }
  }
}

print <<EOF
/**
  \\page PageLic %Gecode license and authors

\\section SectLicLic GECODE LICENSE AGREEMENT
			 
This software and its documentation are copyrighted by the
individual authors as listed in each file. All files, unless explicitly disclaimed in individual files, may be used according to the terms of the
<a href="http://www.opensource.org/licenses/mit-license.php">
MIT license</a>, which reads as follows:

<TT>
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</TT>

\\section SecLicAuthors %Gecode authors

The following authors have contributed to the %Gecode library
and its documentation (in alphabetical order of firstname):
EOF
;


foreach $n (sort(keys(%main))) {
  $e = $main{$n};
  print " - $n <$e>\n";
}

print <<EOF

The above authors are grateful to the following people who have provided
bugfixes:
EOF
;

foreach $n (sort(keys(%fix))) {
  print " - $n\n";
}

print <<EOF

We are also grateful for our users reporting bugs and suggesting 
improvements.

*/
EOF
;

