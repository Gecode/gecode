#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2008
#
#  Last modified:
#     $Date: 2008-04-28 17:47:23 +0200 (Mon, 28 Apr 2008) $ by $Author: raphael $
#     $Revision: 6797 $
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

use File::Basename;

$file = $ARGV[0];

my ($filename, $directory, $suffix) = fileparse($file);

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file "$file" instead.
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte\@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

EOF
;

$n = 0;

open (FILE, $file) || die "Could not open ".$file;

while ($l = <FILE>) {
 LINE:
  next if ($l =~ /^\#/);
  last if ($l =~ /^\[End\]/io);
  
  if ($l =~ /^\[General\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^View:\s*(\w+)/io) {
	$view = $1;
      } elsif ($l =~ /^VarBranch:\s*(\w+)/io) {
	$varbranch = $1;
      } elsif ($l =~ /^ValBranch:\s*(\w+)/io) {
	$valbranch = $1;
      } elsif ($l =~ /^Inline:\s*true/io) {
	$inline = 1;
      }
    }
    goto LINE;
  } elsif ($l =~ /^\[VarBranch\]/io) {
    
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Value:\s*(\w+)\s*=\s*(\w+)/io) {
	# Found a special variable branching
	$lhs = $1; $rhs = $2;
	if (!$rhs eq "NONE") {
	  die "Unknown special variable branching: $rhs\n";
	}
	$vb[$n] = $lhs;
	$number{$lhs} = $n;
	$none = $n;
      } elsif ($l =~ /^Value:\s*(\w+)/io) {
	# Found a normal variable branching
	$vb[$n] = $1;
	$number{$1} = $n;
      } elsif ($l =~ /^Type:\s*(.+)/io) {
	# Found a normal variable branching
	$type[$n] = $1;
      } elsif ($l =~ /^NoTies:\s*(.+)/io) {
	# Found tie breaking information information
	$nt[$n] = $1;
      }
    }
    $last = $n;
    $n++;
    goto LINE;
  } elsif ($l =~ /^\[Header\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $hdr = $hdr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[Footer\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $ftr = $ftr . $l;
    }
    goto LINE;
  }
  
}
close FILE;

for ($i=0; $i<=$last; $i++) {
  for ($j=0; $j<=$last; $j++) {
    $ties[$i][$j] = 1;
  }
}
for ($i=0; $i<=$last; $i++) {
  $ties[$i][$i] = 0;
}
for ($i=0; $i<=$last; $i++) {
  if ($i == $none) {
    for ($j=0; $j<=$last; $j++) {
      $ties[$i][$j] = 0;
    }
  } else {
    foreach $k (split(/,/,$nt[$i])) {
      $j = $number{$k};
      $ties[$i][$j] = 0;
    }
  }
}

print $hdr;
print "#define GECODE_VBTB(a,b) \\\n";
print "  (((b) * ($vb[$last]+1)) + (a))\n\n";
if ($inline) {
  print "  inline void\n";
} else {
  print "  void\n";
}
print "  post(Space* home, Gecode::ViewArray<$view> x,\n";
print "       const TieBreakVarBranch<$varbranch>\& vars,\n";
print "       $valbranch vals) {\n";
print "     switch (GECODE_VBTB(vars.a,vars.b)) {\n";
for ($i=0; $i<=$last; $i++) {
  print "     case GECODE_VBTB($vb[$i],$vb[$none]):\n";
  $l =  "       post<$type[$i]>(home,x,vals);";
  $l =~ s|>>|> >|og;
  print "$l\n";
  print "       break;\n";
}
for ($i=0; $i<=$last; $i++) {
  next unless ($i != $none);
  for ($j=0; $j<=$last; $j++) {
    next unless ($j != $none) && $ties[$i][$j];
    print "     case GECODE_VBTB($vb[$i],$vb[$j]):\n";
    $l =  "       post<ViewSelTieBreak<$type[$i],$type[$j]>>(home,x,vals);";
    $l =~ s|>>|> >|og;
    $l =~ s|>>|> >|og;
    print "$l\n";
    print "       break;\n";
  }
}
print "     default: GECODE_NEVER;\n";
print "     }\n";
print "  }\n\n";
print "#undef GECODE_VBTB\n\n";
print $ftr;
