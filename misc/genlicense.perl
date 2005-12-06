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
#  See the file "LICENSE" for information on usage and
#  redistribution of this file, and for a
#     DISCLAIMER OF ALL WARRANTIES.
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
  } elsif ($l =~ /Bugfixes provided by:/) {
    while (($l = <>) && ($l =~ /([A-Z].*) <(.*)>/)) {
      $fix{$1} = $2;
    }
  }
}

print <<EOF
/**
  \\page PageLic Gecode license and authors

\\section SectLicLic GECODE LICENSE AGREEMENT
			 
This software and its documentation are copyrighted by the
individual authors as listed in each file. The following
terms apply to all files associated with the software unless
explicitly disclaimed in individual files.

The authors hereby grant permission to use, copy, modify,
distribute, and license this software and its documentation
for any purpose, provided that existing copyright notices
are retained in all copies and that this notice is included
verbatim in any distributions. No written agreement,
license, or royalty fee is required for any of the
authorized uses. Modifications to this software may be
copyrighted by their authors and need not follow the
licensing terms described here, provided that the new terms
are clearly indicated on the first page of each file where
they apply.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE AND ITS
DOCUMENTATION ARE PROVIDED ON AN "AS IS" BASIS, AND THE
AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.

\\section SecLicAuthors Gecode authors

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

