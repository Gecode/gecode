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
# Get statistics for all files
#

$i = 0;

while ($file = @ARGV[$i++]) {
  open(FILE,"<$file");
  my $module = "UNKNOWN";
  my $cat    = "UNKNOWN";
  my $n_blank;
  my $n_comment;
  my $n_code;
  my $n_class;
  while ($l = <FILE>) {
    if ($l =~ /\/\/ STATISTICS: ([A-Za-z]+)-([A-Za-z]+)/) {
      $module = $1;
      $cat    = $2;
    }
    if ($l =~ /^[ \t]*$/) {
      $n_blank += 1;
    } elsif ($l =~ /^[ \t]*\/\//) {
      $n_comment += 1;
    } elsif ($l =~ /\/\*/) {
      $n_comment += 1;
      while (!($l =~ /\*\//) && ($l = <FILE>)) {
	$n_comment += 1;
      }
    } else {
      $n_code += 1;
    }
    if ($l =~ /class [A-Za-z_][A-Za-z0-9_]*.*\{/) {
      $n_class += 1;
    }
  }
  if ($module eq "UNKNOWN") {
    print "UNKNOWN: $file\n";
    exit 1;
  }
  $m{$module} = 1;
  $c{$cat} = 1;
  $class{"$module-$cat"} += $n_class;
  $comment{"$module-$cat"} += $n_comment;
  $blank{"$module-$cat"} += $n_blank;
  $code{"$module-$cat"} += $n_code;
  close FILE;
}

@modorder = ("kernel","search","int","set","minimodel","driver",
	     "iter","support","example","test","flatzinc");

@catorder = ("core","var","prop","branch","post",
	     "int","set","minimodel","stress","any",
	     "sequential","parallel",
	     "other");
$i = 0;

$modclear{"kernel"} = "Kernel";
$modclear{"search"} = "Search engines";
$modclear{"int"} = "Finite domain integers";
$modclear{"set"} = "Finite integer sets";
$modclear{"minimodel"} = "Minimal modeling support";
$modclear{"driver"} = "Script commandline driver";
$modclear{"iter"} = "Range and value iterators";
$modclear{"support"} = "Support algorithms and datastructures";
$modclear{"example"} = "\%Example scripts";
$modclear{"test"} = "Systematic tests";
$modclear{"flatzinc"} = "FlatZinc interpreter";


$catclear{"core"} = "Core functionality";
$catclear{"var"} = "Variables, views, and variable implementations";
$catclear{"prop"} = "Propagators";
$catclear{"branch"} = "Branchers";
$catclear{"sequential"} = "Sequential search engines";
$catclear{"parallel"} = "Parallel search engines";
$catclear{"post"} = "Posting propagators for constraints";
$catclear{"any"} = "All";
$catclear{"other"} = "Miscellaneous";
$catclear{"int"} = "Finite domain integers";
$catclear{"set"} = "Finite integer sets";
$catclear{"minimodel"} = "Minimal modeling support";
$catclear{"stress"} = "System stress";

print <<EOF
/**
  \\page PageCodeStat Gecode code statistics

The following approximate breakdown into the different parts of %Gecode
gives some statistics about the amount of code and documentation 
contained in the code (as comments) where blank lines are excluded. 

The abbreviation "loc" means "lines of code" and "lod" means "lines of 
documentation".

EOF
;


$an_code    = 0;
$an_comment = 0;
$an_class   = 0;

foreach $module (@modorder) {
  print " - " . $modclear{$module} . ": ";
  $mn_code    = 0;
  $mn_comment = 0;
  $mn_class   = 0;
  $n_cat = 0;
  $doc = "";
  foreach $cat (@catorder) {
    $k = "$module-$cat";
    if ($code{$k}) {
      $n_cat += 1;
      $doc = $doc . "   - " . $catclear{$cat} . ": ";
      $mn_code    += $code{$k};
      $mn_comment += $comment{$k};
      $mn_class   += $class{$k};
      $doc = $doc .  $class{$k} . " classes, ";
      $doc = $doc .  $code{$k} . " loc, ";
      $doc = $doc .  $comment{$k} . " lod\n";
    }
  }
  print $mn_class . " classes, " . $mn_code . " loc, " . $mn_comment . " lod\n";
  if ($n_cat > 1) {
    print $doc;
  }
  $an_code    += $mn_code;
  $an_comment += $mn_comment;
  $an_class   += $mn_class;
}


print <<EOF

The grand total: $an_class classes, $an_code loc, $an_comment lod

*/
EOF
;
			 
