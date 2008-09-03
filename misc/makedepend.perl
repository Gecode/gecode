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
# Ultra-simplistic makedepend: Just find existing files
# Relies on:
#  - all filenames are relative to root directory, which is the first argument
#  - #ifdef can be safely ignored
#

$i=0;

$root = $ARGV[$i++];

while ($target = $ARGV[$i++]) {
  my @todo = ();
  my %done = ();
  push @todo, $target;
  while ($f = pop @todo) {
    open FILE, "$root/$f" or
      open FILE, "$f" or die "File missing: $root/$f\n";
    
    while ($l = <FILE>) {
      if ($l =~ /^\#include <(gecode\/.*)>/ || $l =~ /^\#include "(.*)"/) {
	$g = $1;
	$g =~ s|^\./||og;
	if (!$done{$g}) {
	  push @todo, $g;
	  if (-e "$root/$g") {
	    $done{$g} = "$root/";
	  } else {
	    $done{$g} = "";
	  }
	}
      }
    }
    close FILE;
  }
  $target =~ s|\.cpp?||og;
  print "$target\$(OBJSUFFIX) $target\$(SBJSUFFIX): ";
  $l = 3;
  foreach $g (sort(keys(%done))) {
    if ($l == 3) {
      print "\\\n\t";
      $l = 0;
    }
    print "$done{$g}$g ";
    $l++;
  }
  print "\n";
}

