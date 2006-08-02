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

while (($arg = $ARGV[$i]) && ($arg =~ /^-/)) {
  $i++;
  if ($arg eq "-header") {
    $gen_header = 1;
  } elsif ($arg eq "-body") {
    $gen_header = 0;
  }
}

$file = $ARGV[$i];

open FILE, $file;

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

$chdr   = ""; # Class header

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


while ($l = <FILE>) {
 LINE:
  next if ($l =~ /^\#/);
  last if ($l =~ /^\[End\]/io);
  
  if ($l =~ /^\[General\]/io) {
    $h = "";
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      if ($l =~ /^Name:\s*(\w+)/io) {
	$name = $1;
      } elsif ($l =~ /^VTI:\s*(\w+)/io) {
	$VTI = $1;
      } elsif ($l =~ /^Export:\s*(\w+)/io) {
	$export = $1;
      } elsif ($l =~ /^Forceinline:\s*(\w+)/io) {
	$forceinline = $1;
      } else {
        $h = $h . $l;
      }
    }
    $chdr = $h;
    goto LINE;
  } elsif ($l =~ /^\[ModEventHeader\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $mehdr = $mehdr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[ModEvent\]/io) {
    $n = "";
    $h = "";

    while (($l = <FILE>) && !($l =~ /^\[/)) {
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
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $meftr = $meftr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[PropCondHeader\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $pchdr = $pchdr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[PropCond\]/io) {
    $n = "";
    $h = "";

    while (($l = <FILE>) && !($l =~ /^\[/)) {
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
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $pcftr = $pcftr . $l;
    }
    goto LINE;
  } elsif ($l =~ /^\[Header\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $hdr = $hdr . "$l";
    }
    goto LINE;
  } elsif ($l =~ /^\[Footer\]/io) {
    while (($l = <FILE>) && !($l =~ /^\[/)) {
      next if ($l =~ /^\#/);
      $ftr = $ftr . "$l";
    }
    goto LINE;
  }

}

close FILE;

##
## Generate the output
##

$maxpc = "PC_${VTI}_$pcn[$pc_n-1]";
$class = "${name}VarImpBase";
$diffc = "${name}MeDiff";
$base  = "Gecode::Variable<VTI_$VTI,$maxpc,$diffc>";

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated.
 *    Do not edit, edit the file "$file" instead.
 *
 *  This file contains generated code fragments which are 
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte\@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

EOF
;
print "$hdr";

$o = 2;
for ($i=0; $i<$me_n; $i++) {
  $n = $men[$i];
  if ($mespecial{$n} eq "NONE") {
    $me2val{$n} = 0;  $val2me[0] = $n;
  } elsif ($mespecial{$n} eq "ASSIGNED") {
    $me2val{$n} = 1;  $val2me[1] = $n;
  } elsif (!$mespecial{$n}) {
    $me2val{$n} = $o; $val2me[$o] = $n; $o++;
  }
}
$me_max   = "ME_${VTI}_" . $val2me[$o-1] . "+1";
$me_max_n = $o;

if ($gen_header) {

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

  /// Modification event difference for $name-variable implementations
  class $diffc {
EOF
;

if ($me_max_n > 4) {
  print <<EOF
  private:
    $export static const Gecode::ModEvent med[$me_max][$me_max];
EOF
;
}
  print <<EOF
  public:
    /// Return difference when changing modification event \\a me2 to \\a me1
    ModEvent operator()(ModEvent me1, ModEvent me2) const;
  };

$chdr
  class $class : public $base {
  protected:
    /// Variable procesor for variables of this type
    class Processor : public Gecode::VarTypeProcessor<VTI_${VTI},$maxpc,$diffc> {
    public:
      /// Process modified variables linked from \\a x
      $export virtual void process(Space* home, VarBase* x);
    };
    /// The processor used
    $export static Processor p;

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


  $forceinline ModEvent
  ${diffc}::operator()(ModEvent me1, ModEvent me2) const {
EOF
;

if ($me_max_n <= 4) {
  print "    const int med = (\n";

  for ($i=0; $i<$me_max_n;$i++) {
    $n1 = $val2me[$i];
    print "      (\n";
    for ($j=0; $j<$me_max_n;$j++) {
      $n2 = $val2me[$j];
      if ($n1 eq "NONE") {
        $n3 = $n2;
      } elsif ($n2 eq "NONE") {
        $n3 = $n1;
      } else {
        $n3 = $mec{$n1}{$n2};
      }
      print "        ((ME_${VTI}_$n2 ^ ME_${VTI}_$n3) << ";
      print (($i << 3) + ($j << 1));
      if ($j+1 == $me_max_n) {
        print ")   ";
      } else {
        print ") | ";
      }
      print " // [ME_${VTI}_$n1][ME_${VTI}_$n2]\n";
    }
    if ($i+1 == $me_max_n) {
      print "      )\n";
    } else {
      print "      ) |\n";
    }
  }


  print "    );\n";
  print "    return (((med >> (me1 << 3)) >> (me2 << 1)) & 3);\n";
} else {
  print "    return med[me1][me2];\n";
}

print <<EOF
  }

  $forceinline
  ${class}::${class}(Space* home)
    : $base(home) {}

  $forceinline
  ${class}::${class}(Space* home, bool share, $class\& x)
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

} else {

  if ($me_max_n > 4) {
  print <<EOF

  /*
   * Modification event difference for $name-variable implementations
   *
   */
  const Gecode::ModEvent ${diffc}::med[$me_max][$me_max] = {
EOF
;

  for ($i=0; $i<$me_max_n;$i++) {
    $n1 = $val2me[$i];
    print "    {\n";
    for ($j=0; $j<$me_max_n;$j++) {
      $n2 = $val2me[$j];
      if ($n1 eq "NONE") {
        $n3 = $n2;
      } elsif ($n2 eq "NONE") {
        $n3 = $n1;
      } else {
        $n3 = $mec{$n1}{$n2};
      }
      print "      ME_${VTI}_$n2 ^ ME_${VTI}_$n3";
      if ($j+1 == $me_max_n) {
        print " ";
      } else {
        print ",";
      }
      print " // [ME_${VTI}_$n1][ME_${VTI}_$n2]\n";
    }
    if ($i+1 == $me_max_n) {
      print "    }\n";
    } else {
      print "    },\n";
    }
  }
  print "  };\n";
}

  print <<EOF


  /*
   * The variable processor for $class
   *
   */

  void
  ${class}::Processor::process(Space* home, VarBase* _x) {
    // Process modified variables
    ${base}* x = 
      static_cast<${base}*>(_x);
    do {
      switch (x->modevent()) {
EOF
;

  for ($i=0; $i<$pc_n; $i++) {
     if ($pcspecial{$pcn[$i]} eq "ASSIGNED") {
       $val2pc[0] = $pcn[$i];
     }
  }
  $o = 1;
  for ($i=0; $i<$pc_n; $i++) {
     if (!($pcspecial{$pcn[$i]} eq "ASSIGNED")) {
       $val2pc[$o] = $pcn[$i]; $o++;
     }
  }

#  for ($i=0; $i<$pc_n; $i++) {
#     print "val2pc[$i] = " . $val2pc[$i] . "\n";
# }

  for ($i=0; $i<$me_n; $i++) {
    $n = $men[$i];
    if ($mespecial{$n} eq "ASSIGNED") {
      print "      case ME_${VTI}_$n:\n";
      print "        x->process(home);\n";
      print "        break;\n";
    } elsif (!($mespecial{$n} eq "NONE") && !($mespecial{$n} eq "FAILED")) {
      print "      case ME_${VTI}_$n:\n";
      print "        // Conditions: ";
      for ($j=0; $j<$pc_n; $j++) {
        if ($mepc{$men[$i]}{$pcn[$j]}) {
          print $pcn[$j] . " ";
        }
      }
      print "\n";
      for ($j=0; $j<$pc_n; $j++) {
	if ($mepc{$men[$i]}{$val2pc[$j]}) {
	  # Found initial entry (plus one for stopping)
	  print "        x->process(home,PC_${VTI}_" . $val2pc[$j] . ",";
	  # Look for all connected entries
	  while ($mepc{$men[$i]}{$val2pc[$j+1]}) {
	    $j++;
          }
	  # Found last entry
	  print "PC_${VTI}_" . $val2pc[$j] . ",ME_${VTI}_$n);\n";
	}
      }
      print "        break;\n";
    }
  }


  print <<EOF
      default: GECODE_NEVER;
      }
      x = x->next();
    } while (x != NULL);
  }
  
  ${class}::Processor ${class}::p;

EOF
;

}

print "$ftr";

