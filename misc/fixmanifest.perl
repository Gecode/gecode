#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2010
#
#  Last modified:
#     $Date: 2010-06-29 10:42:27 +0200 (Tue, 29 Jun 2010) $ by $Author: schulte $
#     $Revision: 11119 $
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

if ($file =~ /(.*)Gecode([A-Za-z]+)-([0-9]+)-([0-9]+)-([0-9]+)-[rd]-(x[0-9]+)\.dll\.manifest/) {
  $type = "Modules";
  $name = $2;
  $x = $3; $y = $4; $z = $5;
  $arch = $6;
} elsif ($file =~ /(.*)\/examples\/(.+)\.exe/) {
  $type = "Examples";
  $name = $2;
  $dllsuffix = $ARGV[1];
  if ($dllsuffix =~ /-([0-9]+)-([0-9]+)-([0-9]+)-[rd]-(x[0-9]+)\.dll/) {
    $x = $1; $y = $2; $z = $3;
    $arch = $4;
  }
} elsif ($file =~ /(.*)fz\.exe/) {
  $type = "Tools";
  $name = "FlatZinc";
  $dllsuffix = $ARGV[1];
  if ($dllsuffix =~ /-([0-9]+)-([0-9]+)-([0-9]+)-[rd]-(x[0-9]+)\.dll/) {
    $x = $1; $y = $2; $z = $3;
    $arch = $4;
  }
} elsif ($file =~ /(.*)\/test\.exe/) {
  $type = "Tools";
  $name = "Test";
  $dllsuffix = $ARGV[1];
  if ($dllsuffix =~ /-([0-9]+)-([0-9]+)-([0-9]+)-[rd]-(x[0-9]+)\.dll/) {
    $x = $1; $y = $2; $z = $3;
    $arch = $4;
  }
}

if ($arch =~ /x64/) {
  $arch = "amd64";
}

open (INMANIFEST, "<", $file) || die "Could not open " . $file;
open (OUTMANIFEST, ">", "$file.tmp") || die "Could not open " . "$file.tmp";

while ($l = <INMANIFEST>) {
  print OUTMANIFEST $l;
  if ($l =~ /\<assembly /) {
    print OUTMANIFEST "  <assemblyIdentity type=\"win32\"\n"; 
    print OUTMANIFEST "                    name=\"Gecode.$type.$name\"\n";
    print OUTMANIFEST "                    version=\"3.4.2.0\"\n"; 
    print OUTMANIFEST "                    processorArchitecture=\"$arch\"\n";
    print OUTMANIFEST "                    publicKeyToken=\"0000000000000000\"\n";
    print OUTMANIFEST "  />\n";
  }
}

close INMANIFEST;
close OUTMANIFEST;
system("rm -f \"$file\"");
system("mv \"$file.tmp\" \"$file\"");
