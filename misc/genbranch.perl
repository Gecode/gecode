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
      if ($l =~ /^View:\s*(\w+)/io) {
	$view = $1;
      } elsif ($l =~ /^VarBranch:\s*(\w+)/io) {
	$varbranch = $1;
      } elsif ($l =~ /^ValBranch:\s*(\w+)/io) {
	$valbranch = $1;
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

if ($hasinclude) {
print "#include $include\n";
}
print $hdr;
print "  Gecode::ViewSelVirtualBase<$view>*\n";
print "  post(Gecode::Space* home, $varbranch vars,\n";
print "       const Gecode::VarBranchOptions& o_vars) {\n";
print "    switch (vars) {\n";
for ($i=0; $i<$n; $i++) {
  print "     case $vb[$i]:\n";
  $l =  "       return new (home) ViewSelVirtual<$type[$i]>(home,o_vars);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
}
print "    default:\n";
print "      throw $exception;\n";
print "    }\n";
print "  }\n\n";
print "  template<int n>\n";
print "  void\n";
print "  post(Gecode::Space* home, Gecode::ViewArray<$view>\& x,\n";
print "       $varbranch vars, const Gecode::VarBranchOptions& o_vars,\n";
print "       Gecode::ViewSelVirtualBase<$view>* tb[n],\n";
print "       $valbranch vals,\n";
print "       const Gecode::ValBranchOptions& o_vals) {\n";
print "    switch (vars) {\n";
for ($i=0; $i<$n; $i++) {
  next unless $i != $none;
  print "    case $vb[$i]: {\n";
  print "        $type[$i] vs(home,o_vars);\n";
  print "        ViewSelTieBreak<$type[$i],n> v(home,vs,tb);\n";
  $l =  "        post<ViewSelTieBreak<$type[$i],n>>(home,x,v,vals,o_vals);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
  print "      }\n";
  print "      break;\n";
}
print "    default:\n";
print "      throw $exception;\n";
print "    }\n";
print "  }\n\n";
print "  void\n";
print "  post(Gecode::Space* home, Gecode::ViewArray<$view>\& x,\n";
print "       const Gecode::TieBreakVarBranch<$varbranch>\& vars,\n";
print "       $valbranch vals,\n";
print "       const Gecode::TieBreakVarBranchOptions& o_vars,\n";
print "       const Gecode::ValBranchOptions& o_vals) {\n";
print "    if ((vars.a == $vb[$none]) ||\n";
print "        ((vars.b == $vb[$none]) && \n";
print "         (vars.c == $vb[$none]) && \n";
print "         (vars.d == $vb[$none]))) {\n";
print "      switch (vars.a) {\n";
for ($i=0; $i<$n; $i++) {
  print "    case $vb[$i]: {\n";
  $l =  "        $type[$i] v(home,o_vars.a);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
  $l =  "        post<$type[$i]>(home,x,v,vals,o_vals);\n";
  $l =~ s|>>|> >|og; $l =~ s|>>|> >|og;
  print $l;
  print "      }\n";
  print "      break;\n";
}
print "      default:\n";
print "        throw $exception;\n";
print "      }\n";
print "    } else if ((vars.c == $vb[$none]) && \n";
print "               (vars.d == $vb[$none])) {\n";
print "      Gecode::ViewSelVirtualBase<$view>* tb[1] = {\n";
print "        post(home,vars.b,o_vars.b)\n";
print "      };\n";
print "      post<1>(home,x,vars.a,o_vars.a,tb,vals,o_vals);\n";
print "    } else if (vars.d == $vb[$none]) {\n";
print "      Gecode::ViewSelVirtualBase<$view>* tb[2] = {\n";
print "        post(home,vars.b,o_vars.b),\n";
print "        post(home,vars.c,o_vars.c)\n";
print "      };\n";
print "      post<2>(home,x,vars.a,o_vars.a,tb,vals,o_vals);\n";
print "    } else {\n";
print "      Gecode::ViewSelVirtualBase<$view>* tb[3] = {\n";
print "        post(home,vars.b,o_vars.b),\n";
print "        post(home,vars.c,o_vars.c),\n";
print "        post(home,vars.d,o_vars.d)\n";
print "      };\n";
print "      post<3>(home,x,vars.a,o_vars.a,tb,vals,o_vals);\n";
print "    }\n";
print "  }\n\n";
print $ftr;
