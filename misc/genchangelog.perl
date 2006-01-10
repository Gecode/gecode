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
  \\page PageChange Changelog

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
$modclear{"example"} = "\%Example scripts";
$modclear{"test"} = "Systematic tests";
$modclear{"other"} = "Other";

$whatclear{"bug"} = "Bug fixes";
$whatclear{"performance"} = "Performance improvements";
$whatclear{"documentation"} = "Documentation fixes";
$whatclear{"new"} = "Additions";
$whatclear{"change"} = "Other changes";

$rankclear{"minor"} = "minor";
$rankclear{"major"} = "major";

@modorder = ("kernel","search","int","set","minimodel",
	     "iter","support","example","test");

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
      if (!($info eq "")) {
	$info = " ($info)";
      }
      $sid = "Change$version";
      $sid =~ s|\.|_|g;
      print "\\section $sid Changes in Version $version$info\n\n";
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
    while (($l = <>) && 
	   !(($l =~ /\[ENTRY\]/) || ($l =~ /\[RELEASE\]/))) {
      if ($l =~ /Version:[\t ]*(.*)$/) {
	$version = $1;
      } elsif ($l =~ /Info:[\t ]*(.*)$/) {
	$info    = $1;
      }
    }
    goto LINE;
  }
  if ($l =~ /^\[ENTRY\]/) {
    my $mod, $what, $rank, $bug, $desc;
    while (($l = <>) && !($l =~ /\[DESCRIPTION\]/)) {
      if ($l =~ /Module:[\t ]*(.*)$/) {
	$mod  = $1;
      } elsif ($l =~ /What:[\t ]*(.*)$/) {
	$what = $1;
      } elsif ($l =~ /Rank:[\t ]*(.*)$/) {
	$rank = $1;
      } elsif ($l =~ /Bug:[\t ]*(.*)$/) {
	$bug  = $1;
      }
    }
    
    while (($l = <>) && 
	   !(($l =~ /\[ENTRY\]/) || ($l =~ /\[RELEASE\]/))) {
      chop $l;
      $desc = $desc . " " . $l;
    }
    $hastext{$mod} = 1;
    $rb = $rankclear{$rank};
    if ($bug) {
      $rb = $rb . ", <a href=\"http://www.gecode.org/bugzilla/show_bug.cgi?id="
	. $bug . "\">bugzilla entry</a>";
    }
    $text{"$mod-$what"} = 
      ($text{"$mod-$what"} . "      - $desc (" . $rb . ")\n");
    $desc = "";
    $bug = "";
    $rank = "";
    $what = "";
    $mod = "";
    goto LINE;
  }
}

print "\n\n*/\n";
