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

print <<EOF
/**

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
$modclear{"bdd"} = "BDD-based finite integer sets";
$modclear{"minimodel"} = "Minimal modelling support";
$modclear{"iter"} = "Range and value iterators";
$modclear{"support"} = "Support algorithms and datastructures";
$modclear{"example"} = "\%Example scripts";
$modclear{"test"} = "Systematic tests";
$modclear{"other"} = "General";

$whatclear{"bug"} = "Bug fixes";
$whatclear{"performance"} = "Performance improvements";
$whatclear{"documentation"} = "Documentation fixes";
$whatclear{"new"} = "Additions";
$whatclear{"removed"} = "Removals";
$whatclear{"change"} = "Other changes";

$rankclear{"minor"} = "minor";
$rankclear{"major"} = "major";

@modorder = ("kernel","search","int","set","bdd","minimodel",
	     "iter","support","example","test","other");

@whatorder = ("new","removed","change","bug","performance","documentation");

foreach $mod (@modorder) {
  $hastext{$mod} = 0;
  foreach $what (@whatorder) {
    $text{"$mod-$what"} = "";
  }
}

$versions = "";

while ($l = <>) {
 LINE:
  next if ($l =~ /^\#/);
  if ($l =~ /^\[RELEASE\]/) {
    # Print previous
    if (!$first) {
      $sid = "PageChanges_$version";
      $sid =~ s|\.|_|g;
      $versions = $versions . "\n - \\ref $sid \"Gecode $version ($date)\"";
      print "\\page $sid Changes in Version $version ($date)\n\n";

      print "$info\n\n";

      foreach $mod (@modorder) {
	if ($hastext{$mod}) {
	  print " - " . $modclear{$mod} . "\n";
	  $hastext{$mod} = 0;
	  foreach $what (@whatorder) {
	    if (!($text{"$mod-$what"} eq "")) {
	      print "   - " . $whatclear{$what} . "\n";
	      print $text{"$mod-$what"};
	      $text{"$mod-$what"} = "";
	      print "      .\n";
	    }
	  }
	  print "   .\n";
	}
      }
      print "\n\n";
    }
    $first   = 0;
    $version = "";
    $info    = "";
    $date    = "";
    while (($l = <>) && !($l =~ /\[DESCRIPTION\]/)) {
      if ($l =~ /Version:[\t ]*(.*)$/) {
	$version = $1;
      } elsif ($l =~ /Date:[\t ]*(.*)$/) {
	$date    = $1;
      }
    }
    while (($l = <>) && !($l =~ /\[ENTRY\]/)) {
      chop $l;
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
      chop $l;
      $desc = $desc . " " . $l;
    }
    $hastext{$mod} = 1;
    $rb = $rankclear{$rank};
    if (!($bug eq "")) {
      $rb = $rb . ", <a href=\"http://www.gecode.org/bugzilla/show_bug.cgi?id="
	. $bug . "\">bugzilla entry</a>";
    }
    if (!($thanks eq "")) {
      $rb = $rb . ", thanks to $thanks";
    }
    $text{"$mod-$what"} = 
      ($text{"$mod-$what"} . "      - $desc (" . $rb . ")\n");
    goto LINE;
  }
}

print <<EOF

\\page PageChanges_1_0_0 Changes in Version 1.0.0 (2005-12-06, initial release)

No changes, of course.

\\page PageChange Changelog

\\section SectionChangeList Changes in Gecode Versions

$versions
 - \\ref PageChanges_1_0_0 "Gecode 1.0.0 (2005-12-06)"

\\section SectionChangeWhat Gecode Version Numbers

Gecode version numbers <em>x</em>.<em>y</em>.<em>z</em> change
according to the following rules (of thumb):

 - when \\e z changes, the programming interfaces for 
   \\ref TaskInt, \\ref TaskMiniModel, and \\ref TaskSearch remain
   stable and only minor additions or improvements are included.
 - when \\e y changes, the above mentioned interfaces might have changed
   and medium to major additions or improvements are included.
 - the change of \\e x is reserved for radical changes to %Gecode.

EOF
;

print "\n\n*/\n";
