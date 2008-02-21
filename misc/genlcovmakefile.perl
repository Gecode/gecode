#!/usr/bin/perl -w
use File::Basename;

open (TESTS, "./test/test -list |");

my %tests;

my $blockSize = 40;

while ($l = <TESTS>) {
  chomp($l);
  $tests{$l} = "";
}
close (TESTS);

$size = 0;
foreach my $k (keys %tests) {
  $size++;
}

my $targets = $size / $blockSize;

open (EXAMPLES, "find ./examples -maxdepth 1 -type f ! -name '*.*' |");

my @examples;
while (my $x = <EXAMPLES>) {
  chomp($x);
  my ($filename, $dummydir, $suffix) = fileparse($x);

  open (HELP, "$x -help 2>&1 |");
  my @prop;
  my @model;
  while (my $l = <HELP>) {
    if ($l =~ /-propagation \((.*)\)/) {
      $l1 = $1;
      $l1 =~ s/ //g;
      @prop = split(/,/, $l1);
    } elsif ($l =~ /-model \((.*)\)/) {
      $l1 = $1;
      $l1 =~ s/ //g;
      @model = split(/,/, $l1);
    }
  }
  if (scalar(@prop) == 0) {
    if (scalar(@model) == 0) {
      push(@examples, $filename);
    } else {
      foreach $m (@model) {
        push(@examples, "$filename -model $m");
      }
    }
  } else {
    foreach $p (@prop) {
      if (scalar(@model) == 0) {
        push(@examples, "$filename -propagation $p");
      } else {
        foreach $m (@model) {
          push(@examples, "$filename -propagation $p -model $m");
        }
      }
    }
  }
  close (HELP);
}
close (EXAMPLES);

print "LCOVOUTDIR = /srv/gecode/httpd/html/Internal/gcov-trunk\n\n";
print "all: tests examples\n";
print "\tlcov --directory \$(PWD) --base-directory \$(PWD) --capture \\\n";
print "\t     --output-file testsandexamples.info\n";
print "\tgenhtml testsandexamples.info -o \$(LCOVOUTDIR)\n\n";

print "tests: \\\n";
for (my $tcount = 0; $tcount <= $targets; $tcount++) {
  print "\ttest$tcount \\\n";
}
print "\tdone\n\n";
print "examples: \\\n";
foreach $e (@examples) {
  $etarget = $e;
  $etarget =~ s/[ -]/_/g;
  print "\t$etarget \\\n";
}
print "\tdone\n\n";

$tcount = 1;
$count = 0;
print "test0:\n";
print "\t./test/test \\\n";
foreach my $k (keys %tests) {
  if ($count == $blockSize) {
    print "\t\t\$(TESTOPTIONS)\n\n";
    print "test$tcount:\n";
    print "\t./test/test \\\n";
    $count = 0;
    $tcount++;
  }
  print "\t\t-test ".$k." \\\n";
  $count++;
}
if ($count < $blockSize) {
  print "\t\t\$(TESTOPTIONS)\n\n";
}

foreach $e (@examples) {
  $etarget = $e;
  $etarget =~ s/[ -]/_/g;
  print "$etarget:\n";
  print "\t./examples/$e\n\n";
}

print "done:\n";
