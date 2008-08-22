#!/usr/bin/perl

use File::Basename;

print <<EOF
// !\$*UTF8*\$!
{
  archiveVersion = 1;
  classes = {};
  objectVersion = 44;
  objects = {

/* Begin PBXFileReference section */
EOF
;

%files = ();
%sourceTree = ();
%headerTree = ();

findFiles("h","sourcecode.c.h");
findFiles("c","sourcecode.c.c");
findFiles("hpp","sourcecode.cpp.h");
findFiles("hh","sourcecode.cpp.h");
findFiles("cpp","sourcecode.cpp.cpp");

foreach my $k (keys %files) {
  my $relpath = $k;
  $relpath =~ s/\.\/(.*)/\1/;
  print "    ".$files{$k}[0]." /* ".$files{$k}[1]." */ = ";
  print "{isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = ";
  print $files{$k}[2]."; path =\"".$files{$k}[1];
  print "\"; sourceTree = \"<group>\"; };\n"
}

print "/* End PBXFileReference section */\n\n";
print "/* Begin PBXGroup section */\n";

my $sourcesUID = uid();
printGroup("Sources",$sourcesUID,\%sourceTree,"./");

my $headersUID = uid();
printGroup("Headers",$headersUID,\%headerTree,"./");

my $mainUID = uid();

print <<EOF
    $mainUID = {
      isa = PBXGroup;
      children = (
        $sourcesUID /* Sources */,
        $headersUID /* Headers */,
      );
      sourceTree = "<group>";
    };
EOF
;

my $rootUID = uid();

print "/* End PBXGroup section */\n";
print "/* Begin PBXProject section */\n";
print "    ".$rootUID." /* Project object */ = {\n";
print "      isa = PBXProject;\n";
print "      compatibilityVersion = \"XCode 2.4\";\n";
print "      mainGroup = ".$mainUID." /* Gecode */;\n";
print "      projectDirPath = \"\";\n";
print "      projectRoot = \"\";\n";
print "    };\n";
print "/* End PBXProject section */\n";
print "  };\n";
print "  rootObject = ".$rootUID." /* Project object */;\n";
print "}\n";

sub findFiles {
  my $extension = $_[0];
  my $type = $_[1];
  my $find = "find . -name '*.".$extension."' -type f |";
  open (F, $find);
  while (my $f = <F>) {
    chomp($f);
    my ($filename, $dummydir, $suffix) = fileparse($f);

    my @dirs = split(/\//,$f);
    shift(@dirs);
    pop(@dirs);
    
    my $treeP;
    if ($type =~ m/sourcecode\..*\.h/) {
      $treeP = \%headerTree;
    } else {
      $treeP = \%sourceTree;      
    }
    foreach my $d (@dirs) {
      if (not exists $treeP->{$d}) {
        my %subTree = ();
        my @child = (uid(), \%subTree);
        $treeP->{$d} = [@child];
      }
      $treeP = $treeP->{$d}[1];
    }
    $treeP->{$filename} = "";
    my @r = (uid(), $filename, $type);
    $files{$f} = [@r];
  }
  close (F);
}

sub uid {
  my $uid = `uuidgen`;
  $uid =~ /[A-Z0-9]+-([A-Z0-9]+)-([A-Z0-9]+)-([A-Z0-9]+)-([A-Z0-9]+)/;
  return $1.$2.$3.$4;
}

sub printGroup {
  my $g = $_[0];
  my $id = $_[1];
  my $treeP = $_[2];
  my $path = $_[3];
  print "    ".$id. " /* ".$g." */ = {\n";
  print "      isa = PBXGroup;\n";
  print "      children = (\n";
  foreach my $k (sort keys %{$treeP}) {
    if (exists $treeP->{$k}[0]) {
      print "        ".$treeP->{$k}[0]." /* ".$k." */,\n";
    } else {
      print "        ".$files{$path.$k}[0]." /* ".$k." */,\n";
    }
  }
  print "      );\n";
  if ($path eq "./") {
    print "      path = \".\";\n";
    print "      name = \"$g\";\n";
  } else {
    print "      path = \"$g\";\n";
  }
  print "      sourceTree = \"<group>\";\n    };\n";
  foreach my $k (keys %{$treeP}) {
    if (exists $treeP->{$k}[0]) {
      printGroup($k, $treeP->{$k}[0], $treeP->{$k}[1], 
                 $path.$k."/");
    }
  }
}
