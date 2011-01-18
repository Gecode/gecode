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


$dlldescription{"Kernel"} = "kernel";
$dlldescription{"Search"} = "search engines";
$dlldescription{"Int"} = "finite domain integers";
$dlldescription{"Set"} = "finite integer sets";
$dlldescription{"Minimodel"} = "minimal modeling support";
$dlldescription{"Graph"} = "graph constraints";
$dlldescription{"Scheduling"} = "scheduling constraints";
$dlldescription{"Driver"} = "script commandline driver";
$dlldescription{"FlatZinc"} = "FlatZinc interpreter";
$dlldescription{"Gist"} = "Gist";
$dlldescription{"Support"} = "Support algorithms and datastructures";


# $dlldescription{"iter"} = "Range and value iterators";
# $dlldescription{"example"} = "\%Example scripts";
# $dlldescription{"test"} = "Systematic tests";


$i = 0;

# Gecode version
$version = $ARGV[$i++];

# File for which a resource template is to be generated
$file = $ARGV[$i++];

# Debug or release mode
$mode = $ARGV[$i++];

# Source files follows from argument position 3 onwards
$n_srcfiles = 0;
while ($arg = $ARGV[$i]) {
  $srcfile[$n_srcfiles] = $arg;
  $n_srcfiles++; $i++;
}

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the following files instead:
 *      misc/genresourcefile.perl
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

if ($file =~ /Gecode(.*)-([0-9]+-[0-9]+-[0-9]+-[rd]-.+)\.dll$/) {
  $filename    = $file;
  $filetype    = "VFT_DLL";
  $description = "Gecode " . $dlldescription{$1};
  $basename    = "Gecode$1-$2";
  $copyright = "";
} else {
  $filetype    = "VFT_APP";
}

$fileversion = $version;
$fileversion =~ s|\.|,|go;

if ($mode =~ /r/) {
  $debug = 0;
} else {
  $debug = "VS_FF_DEBUG";
}


print <<EOF

#include <windows.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION    	$fileversion,0
PRODUCTVERSION 	$fileversion,0
FILEFLAGSMASK  	VS_FFI_FILEFLAGSMASK
FILEFLAGS      	$debug
FILEOS         	VOS__WINDOWS32
FILETYPE       	$filetype
FILESUBTYPE    	VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",      "Gecode team"
            VALUE "FileDescription",  "$description"
            VALUE "FileVersion",      "$version.0"
            VALUE "InternalName",     "$basename"
            VALUE "LegalCopyright",   "$copyright"
            VALUE "OriginalFilename", "$filename"
            VALUE "ProductName",      "Gecode"
            VALUE "ProductVersion",   "$version.0"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END

EOF
;

