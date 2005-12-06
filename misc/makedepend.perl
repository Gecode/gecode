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
# Ultra-simplistic makedepend: Just find existing files
# Relies on:
#  - all filenames are relative to root directory, which is the first argument
#  - #ifdef can be safely ignored
#

$i=0;

$predef{"vti.icc"} = 1;

$root = $ARGV[$i++];

while ($target = $ARGV[$i++]) {
  my @todo = ();
  my %done = ();
  push @todo, $target;
  while ($f = pop @todo) {
    open FILE, "$root/$f";
    while ($l = <FILE>) {
      if ($l =~ /^\#include "(.*)"/) {
	$g = $1;
	$g =~ s|^\./||og;
	if ($predef{$g} || ((-e "$root/$g") && !$done{$g})) {
	  push @todo, $g;
	  $done{$g} = 1;
	}
      }
    }
    close FILE;
  }
  $target =~ s|\.cc||og;
  print "$target\$(OBJSUFFIX) $target\$(SBJSUFFIX): ";
  $l = 3;
  foreach $g (sort(keys(%done))) {
    if ($l == 3) {
      print "\\\n\t";
      $l = 0;
    }
    print "$root/$g ";
    $l++;
  }
  print "\n";
}

