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
#     $Date: 2006-02-23 14:51:37 +0100 (Thu, 23 Feb 2006) $ by $Author: schulte $
#     $Revision: 3018 $
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
Changelog for Gecode
================================================================================

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
$modclear{"minimodel"} = "Minimal modelling support";
$modclear{"iter"} = "Range and value iterators";
$modclear{"support"} = "Support algorithms and datastructures";
$modclear{"example"} = "Example scripts";
$modclear{"test"} = "Systematic tests";
$modclear{"other"} = "General";

$whatclear{"bug"} = "Bug fixes";
$whatclear{"performance"} = "Performance improvements";
$whatclear{"documentation"} = "Documentation fixes";
$whatclear{"new"} = "Additions";
$whatclear{"change"} = "Other changes";

$rankclear{"minor"} = "minor";
$rankclear{"major"} = "major";

@modorder = ("kernel","search","int","set","minimodel",
	     "iter","support","example","test","other");

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
      print "\n--------------------------------------------------------------------------------\n\n";
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
#      chop $l;
	if ($desc eq "") {
	    $desc = $l;
	} else {
	    $desc = $desc . "      " . $l;
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
