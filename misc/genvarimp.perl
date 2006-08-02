#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2006
#
#  Last modified:
#     $Date: 2006-07-18 17:36:47 +0200 (Tue, 18 Jul 2006) $ by $Author: schulte $
#     $Revision: 3393 $
#
#  This file is part of Gecode, the generic constraint
#  development environment:
#     http://www.gecode.org
#
#  See the file "LICENSE" for information on usage and
#  redistribution of this file, and for a
#     DISCLAIMER OF ALL WARRANTIES.
#
#

if (@ARGV[0] eq "-header") {
  $gen_header = 1;
} else {
  $gen_header = 0;
}
pop;

## General values
$name   = "";
$VTI    = "";
$export = "";

##
## Headers and footers
##
$hdr    = ""; # Header text
$ftr    = ""; # Footer text

$mehdr  = ""; # Header text for modification events
$meftr  = ""; # Footer text for modification events

$pchdr  = ""; # Header text for propagation conditions
$pcftr  = ""; # Footer text for propagation conditions

##
## Real stuff
##

# $mec : combination table
# $men : name table
# $meh : header table
$me_n = 0; # running number of modification events

# $pcn : name table
# $pch : header table
$pc_n = 0; # running number of propagation conditions


while ($l = <>) {
 LINE:
  next if ($l =~ /^\#/);
  last if ($l =~ /^\[End\]/io);
  
  if ($l =~ /^\[General\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Name:\s*(\w+)/io) {
	$name = $1;
      } elsif ($l =~ /^VTI:\s*(\w+)/io) {
	$VTI = $1;
      } elsif ($l =~ /^Export:\s*(\w+)/io) {
	$export = $1;
      }
    }
    goto LINE;
  } elsif ($l =~ /^\[ModEventHeader\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $mehdr = $mehdr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[ModEvent\]/io) {
    $n = "";
    $h = "";

    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Name:\s*(\w+)\s*=\s*(\w+)/io) {
	# Found a special modification event
	$lhs = $1; $rhs = $2;
	if (!($rhs eq "FAILED") && !($rhs eq "NONE") &&
	    !($rhs eq "ASSIGNED")) {
	  die "Unknown special modification event: $rhs\n";
	}
	$mespecial{$lhs} = $rhs;
	$n = $lhs;
      } elsif ($l =~ /^Name:\s*(\w+)/io) {
	# Found a normal modification event
	$n = $1;
      } elsif ($l =~ /^Combine:\s*(.+)/io) {
	# Found combination information
	$combines = $1;
	while ($combines =~ /(\w+)\s*=(\w+)/g) {
	  $mec{$n}{$1} = $2;
	  $mec{$1}{$n} = $2;
	}
      } else {
	$h = $h . $l;
      }
    }
    $men[$me_n] = $n;
    $meh[$me_n] = $h;
    $me_n++;
    goto LINE;
  } elsif ($l =~ /^\[ModEventFooter\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $meftr = $meftr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[PropCondHeader\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $pchdr = $pchdr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[PropCond\]/io) {
    $n = "";
    $h = "";

    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Name:\s*(\w+)\s*=\s*(\w+)/io) {
	# Found a special propagation condition
	$lhs = $1; $rhs = $2;
	if (!($rhs eq "ASSIGNED")) {
	  die "Unknown special propagation condition: $rhs\n";
	}
	$pcspecial{$lhs} = $rhs;
	$n = $lhs;
      } elsif ($l =~ /^Name:\s*(\w+)/io) {
	# Found a normal modification event
	$n = $1;
      } elsif ($l =~ /^ModEvents:\s*(.+)/io) {
	# Found relation to modification events
	$events = $1;
	while ($events =~ /(\w+)/g) {
	  $me2pc{$1} = $n;
	}
      } else {
	$h = $h . $l;
      }
    }
    $pcn[$pc_n] = $n;
    $pch[$pc_n] = $h;
    $pc_n++;
    goto LINE;
  } elsif ($l =~ /^\[PropCondFooter\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $pcftr = $pcftr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[Header\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $hdr = $hdr . "$l";
    }
    goto LINE;
  } elsif ($l =~ /^\[Footer\]/io) {
    while (($l = <>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $ftr = $ftr . "$l";
    }
    goto LINE;
  }

}

##
## Generate the output
##

if ($gen_header) {

  print "$hdr";
  print "$mehdr";

  $o = 1;
  for ($i=0; $i<$me_n; $i++) {
    $n = $men[$i];
    print $meh[$i];
    print "  const Gecode::ModEvent ME_${VTI}_${n} = ";
    if ($mespecial{$n}) {
      print "Gecode::ME_GEN_" . $mespecial{$n};
    } else {
      print "Gecode::ME_GEN_ASSIGNED + " . $o;
      $o++;
    }
    print ";\n\n";
  }

  print "$meftr";
  print "$pchdr";

  $o = 1;
  for ($i=0; $i<$pc_n; $i++) {
    $n = $pcn[$i];
    print $pch[$i];
    print "  const Gecode::PropCond PC_${VTI}_${n} = ";
    if ($pcspecial{$n}) {
      print "Gecode::PC_GEN_" . $pcspecial{$n};
    } else {
      print "Gecode::PC_GEN_ASSIGNED + " . $o;
      $o++;
    }
    print ";\n\n";
  }

  print "$pcftr";
  print "$ftr";

} else {

  $maxpc = "PC_${VTI}_$pcn[$pc_n-1]";

  print "$hdr";
  print <<EOF

  class ${name}VarImpBase : public Variable<VTI_$VTI,$maxpc> {
    public:
  };

EOF
;
  print "$ftr";

}
