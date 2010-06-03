#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#     Guido Tack <tack@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2005
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

print <<EOF
Changelog for Gecode
==============================================================================

EOF
;


# 
# Compile changelog
#

$first   = 1;
$version = "";
$info    = "";

$modclear{"kernel"} = "Kernel";
$modclear{"search"} = "Search engines";
$modclear{"int"} = "Finite domain integers";
$modclear{"set"} = "Finite integer sets";
$modclear{"cpltset"} = "Finite integer sets with complete representation";
$modclear{"minimodel"} = "Minimal modelling support";
$modclear{"graph"} = "Graph constraints";
$modclear{"scheduling"} = "Scheduling constraints";
$modclear{"driver"} = "Script commandline driver";
$modclear{"iter"} = "Range and value iterators";
$modclear{"support"} = "Support algorithms and datastructures";
$modclear{"example"} = "Example scripts";
$modclear{"test"} = "Systematic tests";
$modclear{"gist"} = "Gist";
$modclear{"other"} = "General";

$whatclear{"bug"} = "Bug fixes";
$whatclear{"performance"} = "Performance improvements";
$whatclear{"documentation"} = "Documentation fixes";
$whatclear{"new"} = "Additions";
$whatclear{"removed"} = "Removals";
$whatclear{"change"} = "Other changes";

$rankclear{"minor"} = "minor";
$rankclear{"major"} = "major";

@modorder = ("kernel","search","int","set","cpltset","scheduling","minimodel",
	     "iter","support","example","test","gist","other");

@whatorder = ("new","change","bug","performance","documentation");

foreach $mod (@modorder) {
  $hastext{$mod} = 0;
  foreach $what (@whatorder) {
    $text{"$mod-$what"} = "";
  }
}

while ($l = <>) {
 LINE:
  next if ($l =~ /^\#/);
  $l =~ s/%Gecode/Gecode/g;
  if ($l =~ /^\[RELEASE\]/) {
    # Print previous
    if (!$first) {
      print "Changes in Version $version ($date)\n\n";

      print "Scope:$info";

      foreach $mod (@modorder) {
	if ($hastext{$mod}) {
	  print "- " . $modclear{$mod} . "\n";
	  $hastext{$mod} = 0;
	  foreach $what (@whatorder) {
	    if (!($text{"$mod-$what"} eq "")) {
	      print "  - " . $whatclear{$what} . "\n";
	      print $text{"$mod-$what"};
	      $text{"$mod-$what"} = "";
	    }
	  }

	}
      }
      print "\n------------------------------------------------------------------------------\n\n";
    }
    $first   = 0;
    $version = "";
    $info    = "";
    $date    = "";
    while (($l = <>) && !($l =~ /\[DESCRIPTION\]/)) {
      $l =~ s/%Gecode/Gecode/g;
      if ($l =~ /Version:[\t ]*(.*)$/) {
	$version = $1;
      } elsif ($l =~ /Date:[\t ]*(.*)$/) {
	$date    = $1;
      }
    }
    while (($l = <>) && !($l =~ /\[ENTRY\]/)) {
      $l =~ s/%Gecode/Gecode/g;
#      chop $l;
      $info = $info . " " . $l;
    }
    goto LINE;
  }
  if ($l =~ /^\[ENTRY\]/) {
    my $mod, $what, $rank, $bug, $desc, $thanks;
    $desc = "";
    $bug = "";
    $rank = "";
    $what = "";
    $mod = "";
    $thanks = "";
    while (($l = <>) && !($l =~ /\[DESCRIPTION\]/)) {
      $l =~ s/%Gecode/Gecode/g;
      if ($l =~ /Module:[\t ]*(.*)$/) {
	$mod  = $1;
      } elsif ($l =~ /What:[\t ]*(.*)$/) {
	$what = $1;
      } elsif ($l =~ /Rank:[\t ]*(.*)$/) {
	$rank = $1;
      } elsif ($l =~ /Bug:[\t ]*(.*)$/) {
	$bug  = $1;
      } elsif ($l =~ /Thanks:[\t ]*(.*)$/) {
	$thanks  = $1;
      }
    }
    
    while (($l = <>) && 
	   !(($l =~ /\[ENTRY\]/) || ($l =~ /\[RELEASE\]/))) {
       $l =~ s/%Gecode/Gecode/g;
#      chop $l;
       if (!($l =~ /\[MORE\]/)) {
	   if ($desc eq "") {
	       $desc = $l;
	   } else {
	       $desc = $desc . "      " . $l;
	   }
       }
    }
    chop $desc;
    $hastext{$mod} = 1;
    $rb = $rankclear{$rank};
    if (!($bug eq "")) {
	$rb = $rb . ", bugzilla entry $bug";
#      $rb = $rb . ", <a href=\"http://www.gecode.org/bugzilla/show_bug.cgi?id="
#	. $bug . "\">bugzilla entry</a>";
    }
    if (!($thanks eq "")) {
      $rb = $rb . ", thanks to $thanks";
    }
    $text{"$mod-$what"} = 
      ($text{"$mod-$what"} . "    - $desc(" . $rb . ")\n");
    goto LINE;
  }
}

print <<EOF
Initial release (2005-12-06)

 No changes, of course.

EOF
;
