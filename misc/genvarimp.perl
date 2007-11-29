#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2006
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
$forcedispose = 0;

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
$me_subscribe = "";

# $pcn : name table
# $pch : header table
$pc_n = 0; # running number of propagation conditions


while ($l = <FILE>) {
 LINE:
  next if ($l =~ /^\#/);
  last if ($l =~ /^\[End\]/io);

  if ($l =~ /^\[General\]/io) {
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
      }	elsif ($l =~ /^Forcedispose:\s*true/io) {
	$forcedispose = 1;
      }
    }
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
	    !($rhs eq "ASSIGNED") && !($rhs eq "SUBSCRIBE")) {
	  die "Unknown special modification event: $rhs\n";
	}
        if ($rhs eq "SUBSCRIBE") {
	  $me_subscribe = "ME_${VTI}_$lhs";
        } else {
	  $mespecial{$lhs} = $rhs;
	  if ($rhs eq "ASSIGNED") {
	    $me_assigned = "ME_${VTI}_$lhs";
	  }
        }
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
$file  =~ s|^../||g;

## Check whether there is only one real event
if ($me_n == 3) {
  $me_subscribe = $me_assigned;
}

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
print "$hdr";

$o = 2;
for ($i=0; $i<$me_n; $i++) {
  $n = $men[$i];
  if ($mespecial{$n} eq "NONE") {
    $val2me[0] = $n;
  } elsif ($mespecial{$n} eq "ASSIGNED") {
    $val2me[1] = $n;
  } elsif (!$mespecial{$n}) {
    $val2me[$o] = $n; $o++;
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

  print "  const Gecode::PropCond PC_${VTI}_NONE = Gecode::PC_GEN_NONE;\n";
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

  /// Base-class for $name-variable implementations
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
EOF
;
if ($forcedispose) {
  print <<EOF
    /// Link to next variable, used for disposal
    VarBase* _nextDispose;
EOF
;
}

print <<EOF
    /// Constructor for cloning \\a x
    $class(Space* home, bool share, $class\& x);
  public:
    /// Constructor for creating static instance of variable
    $class(void);
    /// Constructor for creating variable
    $class(Space* home);
    /// \\name Dependencies
    //\@{
    /** \\brief Subscribe propagator \\a p with propagation condition \\a pc to variable
     *
     * In case \\a process is false, the propagator is just subscribed but
     * not processed for execution (this must be used when creating
     * subscriptions during propagation).
     *
     * In case the variable is assigned (that is, \\a assigned is 
     * true), the subscribing propagator is processed for execution.
     * Otherwise, the propagator subscribes and is processed for execution
     * with modification event \\a me provided that \\a pc is different
     * from \\a PC_GEN_ASSIGNED.
     */
    void subscribe(Space* home, Propagator* p, PropCond pc, bool assigned, bool process);
    /// Subscribe advisor \\a a
    void subscribe(Space* home, Advisor* a, bool assigned);
    /// Notify that variable implementation has been modified with modification event \\a me and delta information \\a d
    bool notify(Space* home, ModEvent me, Delta* d);
    //\@}
EOF
;
if ($me_max_n == 2) {
print <<EOF
    /// Return the current modification event
    ModEvent modevent(void) const;
EOF
;
}

if ($forcedispose) {
  print <<EOF
    /// Return link to next variable, used for dispose
    ${class}* nextDispose(void) const;
    /// Set link to next variable, used for dispose
    void nextDispose(${class}* next);
EOF
;
}

print <<EOF

  };


  $forceinline ModEvent
  ${diffc}::operator()(ModEvent me1, ModEvent me2) const {
EOF
;

if ($me_max_n == 2) {
  print "    return me2^me1;\n";
} elsif ($me_max_n <= 4) {
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
print "  }\n";

if ($me_max_n == 2) {
print <<EOF

  $forceinline ModEvent
  ${class}::modevent(void) const {
    return modified() ? ME_GEN_ASSIGNED : ME_GEN_NONE;
  }

EOF
;
}

if ($forcedispose) {
  print <<EOF

  $forceinline
  ${class}::${class}(void) {}

  $forceinline
  ${class}::${class}(Space* home)
    : $base(home), _nextDispose(home->varsDisposeList<VTI_${VTI}>()) {
    home->varsDisposeList<VTI_${VTI}>(this);
  }

  $forceinline
  ${class}::${class}(Space* home, bool share, $class\& x)
    : $base(home,share,x), _nextDispose(home->varsDisposeList<VTI_${VTI}>()) {
    home->varsDisposeList<VTI_${VTI}>(this);
  }

  forceinline ${class}*
  ${class}::nextDispose(void) const {
    return static_cast<${class}*>(_nextDispose);
  }

EOF
;
} else {
  print <<EOF

  $forceinline
  ${class}::${class}(void) {}

  $forceinline
  ${class}::${class}(Space* home)
    : $base(home) {}

  $forceinline
  ${class}::${class}(Space* home, bool share, $class\& x)
    : $base(home,share,x) {}
EOF
;
}
  print <<EOF

  $forceinline void
  ${class}::subscribe(Space* home, Propagator* p, PropCond pc, bool assigned, bool process) {
    ${base}::subscribe(home,p,pc,assigned,$me_subscribe,process);
  }
  $forceinline void
  ${class}::subscribe(Space* home, Advisor* a, bool assigned) {
    ${base}::subscribe(home,a,assigned);
  }

EOF
;

if ($me_max_n == 2) {
  print <<EOF
  $forceinline bool
  ${class}::notify(Space* home, ModEvent, Delta* d) {
    return ${base}::notify(home,d);
  }
EOF
;
} else {
  print <<EOF
  $forceinline bool
  ${class}::notify(Space* home, ModEvent me, Delta* d) {
    return ${base}::notify(home,me,d);
  }
EOF
;
}

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

if ($me_max_n == 2) {

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
      x->process(home); x = x->next();
    } while (x != NULL);
  }

EOF
;


} else {
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

EOF
;

}


print <<EOF

  ${class}::Processor ${class}::p;

EOF
;

}

print "$ftr";

