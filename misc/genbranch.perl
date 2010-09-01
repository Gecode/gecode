#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2008
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

$file = $ARGV[0];

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file
 *      $file
 *    instead.
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
      if ($l =~ /^BranchName:\s*(\w+)/io) {
        $hasbranchname = 1;
        $branchname = $1;
      } elsif ($l =~ /^View:\s*(\w+)/io) {
        $view = $1;
      } elsif ($l =~ /^VarArgs:\s*(\w+)/io) {
        $varargs = $1;
      } elsif ($l =~ /^VarBranch:\s*(\w+)/io) {
        $varbranch = $1;
      } elsif ($l =~ /^ValBranch:\s*(\w+)/io) {
        $valbranch = $1;
      } elsif ($l =~ /^GNS:\s*(.+)/io) {
        $gns = $1;
      } elsif ($l =~ /^LNS:\s*(.+)/io) {
        $lns = $1;
      } elsif ($l =~ /^UNS:\s*(.+)/io) {
        $hasuns = 1;
        $uns = $1;
      } elsif ($l =~ /^Exception:\s*(.+)/io) {
        $exception = $1;
      } elsif ($l =~ /^Include:\s*(.+)/io) {
        $hasinclude = 1;
        $include = $1;
      }
    }
    goto LINE;
  } elsif ($l =~ /^\[VarBranch\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Value:\s*(\w+)\s*=\s*NONE/io) {
        # Found a special variable branching
        $lhs = $1;
        $vb[$n] = $lhs;
        $number{$lhs} = $n;
        $none = $n;
      } elsif ($l =~ /^Value:\s*(\w+)\s*=\s*COMPLETE/io) {
        # Found a special variable branching
        $lhs = $1;
        $vb[$n] = $lhs;
        $number{$lhs} = $n;
        $complete[$n] = 1;
      } elsif ($l =~ /^Value:\s*(\w+)/io) {
        # Found a normal variable branching
        $vb[$n] = $1;
        $number{$1} = $n;
      } elsif ($l =~ /^Type:\s*(.+)/io) {
        # Found a normal variable branching
        $type[$n] = $1;
      }
    }
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

if (!$hasbranchname) {
  $branchname = "branch";
}

if ($hasinclude) {
print "#include $include\n";
}

if ($hasuns) {
  print "\n";
  $nss = "";
  foreach $ns (split('::',$uns)) {
    $nss .= "::".$ns;
    print "using namespace $nss;\n";
  }
  $pns = $lns."::";
}

print $hdr;
$cns = "";
foreach $ns (split('::',$lns)) {
  $cns = $cns."namespace $ns { ";
}
$cns =~ s| $||g;
print "$cns\n\n";
print "  /// Create virtual view selector for tie-breaking\n";
print "  void\n";
print "  virtualize(Gecode::Home home, $varbranch vars,\n";
print "             const Gecode::VarBranchOptions& o_vars,\n";
print "             Gecode::ViewSelVirtualBase<$view>*& v) {\n";
print "    switch (vars) {\n";
for ($i=0; $i<$n; $i++) {
  next unless ($i != $none);
  print "    case $vb[$i]:\n";
  $l =  "      v = new (home) ViewSelVirtual<$type[$i]>(home,o_vars);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
  print "      break;\n";
}
print "    default:\n";
print "      throw $exception;\n";
print "    }\n";
print "  }\n\n";
foreach $ns (split('::',$lns)) {
  print "}";
}
print "\n\n";
$cns = "";
foreach $ns (split('::',$gns)) {
  $cns = $cns."namespace $ns { ";
}
$cns =~ s| $||g;
print "$cns\n\n";
print "  void\n";
print "  $branchname(Gecode::Home home, const $varargs\& x,\n";
print "         $varbranch vars, $valbranch vals,\n";
print "         const Gecode::VarBranchOptions& o_vars,\n";
print "         const Gecode::ValBranchOptions& o_vals) {\n";
$ans = "";
foreach $ns (split('::',$lns)) {
  if ($ans eq "") {
    $ans = $ns;
  } else {
    $ans = "${ans}\:\:$ns";
  }
  print "    using namespace $ans;\n";
}
print "\n\n";
print "    if (home.failed()) return;\n";
print "    ViewArray<$view> xv(home,x);\n";
print "    switch (vars) {\n";
for ($i=0; $i<$n; $i++) {
  print "    case $vb[$i]:\n";
  print "      {\n";
  $l =  "        $type[$i] v(home,o_vars);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
  print "        ".$pns."post(home,xv,v,vals,o_vals,o_vars.bf);\n";
  print "      }\n";
  print "      break;\n";
}
print "    default:\n";
print "      throw $exception;\n";
print "    }\n";
print "  }\n\n";
print "  void\n";
print "  $branchname(Gecode::Home home, const $varargs\& x,\n";
print "         const Gecode::TieBreakVarBranch<$varbranch>\& vars,\n";
print "         $valbranch vals,\n";
print "         const Gecode::TieBreakVarBranchOptions& o_vars,\n";
print "         const Gecode::ValBranchOptions& o_vals) {\n";
$ans = "";
foreach $ns (split('::',$lns)) {
  if ($ans eq "") {
    $ans = $ns;
  } else {
    $ans = "${ans}\:\:$ns";
  }
  print "    using namespace $ans;\n";
}
print "\n\n";
print "    if (home.failed()) return;\n";
$c = "";
for ($i=0; $i<$n; $i++) {
  if ($complete[$i]) {
    $c = "$c || (vars.a == $vb[$i])";
  }
}
print "    if ((vars.a == $vb[$none])$c ||\n";
print "        ((vars.b == $vb[$none]) && (vars.c == $vb[$none]) && (vars.d == $vb[$none]))) {\n";
print "      $branchname(home,x,vars.a,vals,o_vars.a,o_vals);\n";
print "      return;\n";
print "    }\n";
print "    ViewArray<$view> xv(home,x);\n";
print "    Gecode::ViewSelVirtualBase<$view>* tb[3];\n";
print "    int n=0;\n";
print "    if (vars.b != $vb[$none])\n";
print "      virtualize(home,vars.b,o_vars.b,tb[n++]);\n";
print "    if (vars.c != $vb[$none])\n";
print "      virtualize(home,vars.c,o_vars.c,tb[n++]);\n";
print "    if (vars.d != $vb[$none])\n";
print "      virtualize(home,vars.d,o_vars.d,tb[n++]);\n";
print "    assert(n > 0);\n";
print "    ViewSelTieBreakDynamic<$view> vbcd(home,tb,n);\n";
print "    switch (vars.a) {\n";
for ($i=0; $i<$n; $i++) {
  next unless ($i != $none) && !$complete[$i];
  print "    case $vb[$i]:\n";
  print "      {\n";
  print "        $type[$i] va(home,o_vars.a);\n";
  print "        ViewSelTieBreakStatic<$type[$i],\n";
  print "          ViewSelTieBreakDynamic<$view> > v(home,va,vbcd);\n";
  print "        ".$pns."post(home,xv,v,vals,o_vals,o_vars.a.bf);\n";
  print "      }\n";
  print "      break;\n";
}
print "    default:\n";
print "      throw $exception;\n";
print "    }\n";
print "  }\n\n";

foreach $ns (split('::',$gns)) {
  print "}";
}
print "\n\n";
print $ftr;
