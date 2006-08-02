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
$forceinline = "inline";

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
      } elsif ($l =~ /^Forceinline:\s*(\w+)/io) {
	$forceinline = $1;
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
        if ($rhs eq "NONE") {
           $me_none = "ME_${VTI}_$lhs";
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
	  $mepc{$1}{$n} = 1;
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

$maxpc = "PC_${VTI}_$pcn[$pc_n-1]";
$class = "${name}VarImpBase";
$base  = "Gecode::Variable<VTI_$VTI,$maxpc>";

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
      $pc_assigned = "PC_${VTI}_${n}";
      print "Gecode::PC_GEN_" . $pcspecial{$n};
    } else {
      print "Gecode::PC_GEN_ASSIGNED + " . $o;
      $o++;
    }
    print ";\n\n";
  }

  print "$pcftr";

  print <<EOF

  class $class : public $base {
  protected:
    /// Variable procesor for variables of this type
    class Processor : public VarTypeProcessor<VTI_${VTI},$pcmax> {
    public:
      /// Initialize and register variables with kernel
      Processor(void);
    };
    /// The processor used
    $export static Processor p;

    /// Modification event combiner for variables of this type
    class Combiner {
    private:
    public:
      /// Combine modification events \\a me1 and \\a me2
      ModEvent operator()(ModEvent me1, ModEvent me2) const;
    };
    /// The combiner used
    Combiner c;

    /// Constructor for cloning \\a x
    $class(Space* home, bool share, $class\& x);
  public:
    /// Constructor for creating variable
    $class(Space* home);
    /// \\name Dependencies
    //\@{
    /** \\brief Subscribe propagator \\a p with propagation condition \\a pc to variable
     *
     * In case the variable is assigned (that is, \\a assigned is true),
     * the subscribing propagator is processed for execution.
     * Otherwise, the propagator subscribes and is processed for execution
     * with modification event \\a me provided that \\a pc is different
     * from \\a $pc_assigned.
     */
    void subscribe(Space* home, Propagator* p, PropCond pc,
		   bool assigned, ModEvent me);
    /// Cancel subscription of propagator \\a p with propagation condition \\a pc
    void cancel(Space* home, Propagator* p, PropCond pc);
    //\@}
  };


  $forceinline
  ${class}::${class}(Space* home)
    : $base(home) {}

  $forceinline
  ${class}::${class}(Space* home, bool share, $class\<& x)
    : $base(home,share,x) {}

  $forceinline void
  ${class}::subscribe(Space* home, Propagator* p, PropCond pc,
		      bool assigned, ModEvent me) {
    ${base}::subscribe(home,p,pc,assigned,me);
  }

  $forceinline void
  ${class}::cancel(Space* home, Propagator* p, PropCond pc) {
    ${base}::cancel(home,p,pc);
  }

EOF
;
  print "$ftr";

} else {

  print "$hdr";

  print <<EOF

  /*
   * The variable processor for $class
   *
   */

  inline
  ${class}::Processor::Processor(void) {
    // Combination of modification events
EOF
;
  for ($i=0; $i<$me_n; $i++) {
    $n = $men[$i];
    if (!($mespecial{$n} eq "NONE") && !($mespecial{$n} eq "FAILED")) {
      $n = "ME_${VTI}_$n";
      print "    mec($me_none,$n,$n);\n"; 
      print "    mec($n,$me_none,$n);\n";
    }
  }
  for ($i=0; $i<$me_n; $i++) {
    $n1 = $men[$i];
    if (!($mespecial{$n1} eq "NONE") && !($mespecial{$n1} eq "FAILED")) {
      $n1 = "ME_${VTI}_$n1";
      for ($j=0; $j<$me_n; $j++) {
        $n2 = $men[$j];
        if (!($mespecial{$n2} eq "NONE") && !($mespecial{$n2} eq "FAILED")) {
          $n2 = "ME_${VTI}_$n2";
          $n3 = "ME_${VTI}_" . $mec{$men[$i]}{$men[$j]};
          print "    mec($n1,$n2,$n3);\n";
        }
      }
    }
  }

  print <<EOF
    // Mapping between modification events and propagation conditions
EOF
;

  for ($i=0; $i<$me_n; $i++) {
    $n1 = $men[$i];
    if (!($mespecial{$n1} eq "NONE") && !($mespecial{$n1} eq "FAILED")) {
      $n1 = "ME_${VTI}_$n1";
      for ($j=0; $j<$pc_n; $j++) {
        $n2 = "PC_${VTI}_" . $pcn[$j];
        if ($mepc{$men[$i]}{$pcn[$j]}) {
          print "    mepc($n1,$n2);\n";
        }
      }
    }
  }


  print <<EOF
    // Transfer to kernel
    enter();
  }
  
  ${class}::Processor ${class}::p;

EOF
;

  print "$ftr";

}
