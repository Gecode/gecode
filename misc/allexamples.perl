#!/usr/bin/perl -w
use File::Basename;

$directory = $ARGV[0];

open (EXAMPLES, "find $directory/examples -maxdepth 1 -type f ! -name '*.*' |");

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
      runexample($directory,$filename);
    } else {
      foreach $m (@model) {
        runexample($directory,"$filename -model $m");
      }
    }
  } else {
    foreach $p (@prop) {
      if (scalar(@model) == 0) {
        runexample($directory,"$filename -propagation $p");
      } else {
        foreach $m (@model) {
          runexample($directory,"$filename -propagation $p -model $m");
        }
      }
    }
  }
  close (HELP);
}
close (EXAMPLES);

sub runexample {
  $directory = $_[0];
  $filename = $_[1];
  print "------------------------------------------------------------\n";
  print "Running $filename\n";
  open (EX, "$directory/examples/$filename -time 120000 2>&1 |");
  while ($l = <EX>) {
    print $l;
  }
  close (EX);
  print "------------------------------------------------------------\n";
}
