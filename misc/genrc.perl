#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2010
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


$dlldescription{"Kernel"} = "kernel";
$dlldescription{"Search"} = "search engines";
$dlldescription{"Int"} = "finite domain integers";
$dlldescription{"Set"} = "finite integer sets";
$dlldescription{"Minimodel"} = "minimal modeling support";
$dlldescription{"Graph"} = "graph constraints";
$dlldescription{"Scheduling"} = "scheduling constraints";
$dlldescription{"Driver"} = "script commandline driver";
$dlldescription{"FlatZinc"} = "FlatZinc interpreter library";
$dlldescription{"Gist"} = "Gist";
$dlldescription{"Support"} = "support algorithms and datastructures";

$i = 0;

# Includes Gecode version and more
$dllsuffix = $ARGV[$i++];

if ($dllsuffix =~ /-([0-9]+)-([0-9]+)-([0-9]+)-([rd])-x[0-9]+\.dll/) {
  $revx = $1; $revy = $2; $revz = $3;
  $mode = $4;
}

# Directory where source files reside
$dir = $ARGV[$i++];

# File for which a resource template is to be generated
$file = $ARGV[$i++];

# Source files follows from folloeing argument positions
$n_srcfiles = 0;
while ($arg = $ARGV[$i]) {
  $srcfile[$n_srcfiles] = "$dir/$arg";
  $n_srcfiles++; $i++;
}

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the following files instead:
 *     - $dir/misc/genrc.perl
EOF
;

for ($f=0; $f<$n_srcfiles; $f++) {
  print " *     - $srcfile[$f]\n";
}

print <<EOF
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte\@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

# Find authors for copyright from srcfiles
for ($f=0; $f<$n_srcfiles; $f++) {
  open SRCFILE, $srcfile[$f];
  $incopyright = 0;
  while ($l = <SRCFILE>) {
    if ($incopyright) {
      if ($l =~ /\*.*Last modified:/) {
	$incopyright = 0;
      } elsif ($l =~ /\*\s*(.*), [0-9]+/) {
	$authors{$1} = 1;
      }
    } elsif ($l =~ /\*.*Copyright:/) {
      $incopyright = 1;
    }
  }
  close SRCFILE;
}
$copyright = "";
foreach $a (sort(keys(%authors))) {
  $copyright = $copyright . ", " . $a;
}
$copyright =~ s|^, ||o;

if ($file =~ /Gecode(.*)-([0-9]+-[0-9]+-[0-9]+-[rd]-.+)\.dll$/) {
  $filename    = $file;
  $filetype    = "VFT_DLL";
  $icon        = 0;
  $basename    = "Gecode$1-$2";
  $description = "Gecode " . $dlldescription{$1};
} else {
  $filetype    = "VFT_APP";
  $icon        = 1;
  if ($file =~ /fz\.exe/) {
    $filename    = "fz.exe";
    $basename    = "fz";
    $description = "Gecode FlatZinc interpreter";
  } elsif ($file =~ /test\.exe/) {
    $filename    = "fz.exe";
    $basename    = "fz";
    $description = "Gecode systematic tests";
  } elsif ($file =~ /examples\/(.+)\.exe/) {
    $filename    = "$1.exe";
    $basename    = "$1";
    open SRCFILE, $srcfile[0];
    $example = "";
    while ($l = <SRCFILE>) {
      if ($l =~ /\\brief.*Example: (.*)/) {
	$example = $1;
      }
    }
    $example =~ s|\%||og;
    close SRCFILE;
    $description = "Gecode example: $example";
  } else {
    die "Illegal file";
  }
}

if ($mode =~ /r/) {
  $debug = 0;
} else {
  $debug = "VS_FF_DEBUG";
}


print <<EOF

#include <windows.h>

EOF
;

if ($icon) {
  print "0 ICON \"misc/gecode-logo.ico\"\n";
}

print <<EOF

VS_VERSION_INFO VERSIONINFO
FILEVERSION    	$revx,$revy,$revz,0
PRODUCTVERSION 	$revx,$revy,$revz,0
FILEFLAGSMASK  	VS_FFI_FILEFLAGSMASK
FILEFLAGS      	$debug
FILEOS         	VOS_UNKNOWN
FILETYPE       	$filetype
FILESUBTYPE    	VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",      "Gecode team"
            VALUE "FileDescription",  "$description"
            VALUE "FileVersion",      "$revx.$revy.$revz"
            VALUE "InternalName",     "$basename"
            VALUE "LegalCopyright",   "$copyright (license information available from www.gecode.org)"
            VALUE "OriginalFilename", "$filename"
            VALUE "ProductName",      "Gecode"
            VALUE "ProductVersion",   "$revx.$revy.$revz"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END

EOF
;

