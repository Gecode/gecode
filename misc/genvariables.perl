#!/usr/bin/perl
#
#  Main authors:
#     Christian Schulte <schulte@gecode.org>
#
#  Copyright:
#     Christian Schulte, 2007
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
  } elsif ($arg eq "-typeicc") {
    $gen_typeicc   = 1;
  } elsif ($arg eq "-typecc") {
    $gen_typecc   = 1;
  }
}


$varfile = $ARGV[$i];

open VARFILE, $varfile;

$n_files = 0;
while ($l = <VARFILE>) {
  if ($l =~ /^File:\s*([^ \t\r\n]+)/io) {
    $files[$n_files] = $1;
    $n_files++;
  }
}
close VARFILE;

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file "variable.vsl" instead.
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte\@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

for ($f=0; $f<$n_files; $f++) {
  open FILE, $files[$f];

  ## General values
  $name[$f]    = "";
  $vti[$f]     = "";
  $ifdef[$f]   = "";
  $endif[$f]   = "";
  $dispose[$f] = 0;

  ##
  ## Headers and footers
  ##
  $mehdr[$f]  = ""; # Header text for modification events
  $meftr[$f]  = ""; # Footer text for modification events

  $pchdr[$f]  = ""; # Header text for propagation conditions
  $pcftr[$f]  = ""; # Footer text for propagation conditions

  ##
  ## Real stuff
  ##

  # $mec[$f] : combination table
  # $men[$f] : name table
  # $meh[$f] : header table
  $me_n[$f] = 0; # running number of modification events
  $me_subscribe[$f] = "";

  # $pcn[$f] : name table
  # $pch[$f] : header table
  $pc_n[$f] = 0; # running number of propagation conditions


  while ($l = <FILE>) {
  LINE:
    next if ($l =~ /^\#/);
    last if ($l =~ /^\[End\]/io);

    if ($l =~ /^\[General\]/io) {
      while (($l = <FILE>) && !($l =~ /^\[/)) {
	next if ($l =~ /^\#/);
	if ($l =~ /^Name:\s*(\w+)/io) {
	  $name[$f] = $1;
	  $vti[$f]  = uc($name[$f]);
	} elsif ($l =~ /^Ifdef:\s*(\w+)/io) {
	  $ifdef[$f] = "#ifdef $1\n";
	  $endif[$f] = "#endif\n";
	} elsif ($l =~ /^Dispose:\s*true/io) {
	  $dispose[$f] = 1;
	} elsif ($l =~ /^Namespace:\s*([^ \t\r\n]+)/io) {
	  $namespace[$f] = $1;
	}
      }
      goto LINE;
    } elsif ($l =~ /^\[ModEventHeader\]/io) {
      while (($l = <FILE>) && !($l =~ /^\[/)) {
	next if ($l =~ /^\#/);
	$mehdr[$f] = $mehdr[$f] . $l;
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
	    $me_subscribe[$f] = "ME_$vti[$f]_$lhs";
	  } else {
	    $mespecial{$f}{$lhs} = $rhs;
	    if ($rhs eq "ASSIGNED") {
	      $me_assigned[$f] = "ME_$vti[$f]_$lhs";
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
	    $mec{$f}{$n}{$1} = $2;
	    $mec{$f}{$1}{$n} = $2;
	  }
	} else {
	  $h = $h . $l;
	}
      }
      $men[$f][$me_n[$f]] = $n;
      $meh[$f][$me_n[$f]] = $h;
      $me_n[$f]++;
      goto LINE;
    } elsif ($l =~ /^\[ModEventFooter\]/io) {
      while (($l = <FILE>) && !($l =~ /^\[/)) {
	next if ($l =~ /^\#/);
	$meftr[$f] = $meftr[$f] . $l;
      }
      goto LINE;
    } elsif ($l =~ /^\[PropCondHeader\]/io) {
      while (($l = <FILE>) && !($l =~ /^\[/)) {
	next if ($l =~ /^\#/);
	$pchdr[$f] = $pchdr[$f] . $l;
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
	  $pcspecial{$f}{$lhs} = $rhs;
	  $n = $lhs;
	} elsif ($l =~ /^Name:\s*(\w+)/io) {
	  # Found a normal modification event
	  $n = $1;
	} elsif ($l =~ /^ModEvents:\s*(.+)/io) {
	  # Found relation to modification events
	  $events = $1;
	  while ($events =~ /(\w+)/g) {
	    $mepc{$f}{$1}{$n} = 1;
	  }
	} else {
	  $h = $h . $l;
	}
      }
      $pcn[$f][$pc_n[$f]] = $n;
      $pch[$f][$pc_n[$f]] = $h;
      $pc_n[$f]++;
      goto LINE;
    } elsif ($l =~ /^\[PropCondFooter\]/io) {
      while (($l = <FILE>) && !($l =~ /^\[/)) {
	next if ($l =~ /^\#/);
	$pcftr[$f] = $pcftr[$f] . $l;
      }
      goto LINE;
    }

  }
  close FILE;


  $maxpc[$f] = "PC_$vti[$f]_" . $pcn[$f][$pc_n[$f]-1];
  $class[$f] = "$name[$f]VarImpBase";
  $conf[$f]  = "$name[$f]VarImpConf";
  $base[$f]  = "Gecode::VarImp<$namespace[$f]::$conf[$f]>";
  # Generate namespace header and footer
  foreach $ns (split('::',$namespace[$f])) {
    $hdr[$f] = "$hdr[$f]namespace $ns { ";
    $ftr[$f] = "$ftr[$f]}";
  }
  $hdr[$f] = "$hdr[$f]\n";
  $ftr[$f] = "$ftr[$f]\n";

  ## Check whether there is only one real event
  if ($me_n[$f] == 3) {
    $me_subscribe[$f] = $me_assigned[$f];
  }

  $o = 2;
  for ($i=0; $i<$me_n[$f]; $i++) {
    $n = $men[$f][$i];
    if ($mespecial{$f}{$n} eq "NONE") {
      $val2me[$f][0] = $n;
    } elsif ($mespecial{$f}{$n} eq "ASSIGNED") {
      $val2me[$f][1] = $n;
    } elsif (!$mespecial{$f}{$n}) {
      $val2me[$f][$o] = $n; $o++;
    }
  }
  $me_max[$f]   = "ME_$vti[$f]_" . $val2me[$f][$o-1] . "+1";
  $me_max_n[$f] = $o;

  for ($b=1; (1 << $b) < $me_max_n[$f]; $b++) {}
  $bits[$f] = $b;

}

if ($gen_typeicc) {
  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];
    print $mehdr[$f];
    $o = 1;
    for ($i=0; $i<$me_n[$f]; $i++) {
      $n = $men[$f][$i];
      print $meh[$f][$i];
      print "  const Gecode::ModEvent ME_" . $vti[$f] . "_${n} = ";
      if ($mespecial{$f}{$n}) {
	print "Gecode::ME_GEN_" . $mespecial{$f}{$n};
      } else {
	print "Gecode::ME_GEN_ASSIGNED + " . $o;
	$o++;
      }
      print ";\n";
    }

    print $meftr[$f];
    print $pchdr[$f];

    print "  /// Propagation condition to be ignored (convenience)\n";
    print "  const Gecode::PropCond PC_$vti[$f]_NONE = Gecode::PC_GEN_NONE;\n";
    $o = 1;
    for ($i=0; $i<$pc_n[$f]; $i++) {
      $n = $pcn[$f][$i];
      print $pch[$f][$i];
      print "  const Gecode::PropCond PC_$vti[$f]_${n} = ";
      if ($pcspecial{$f}{$n}) {
	$pc_assigned[$f] = "PC_$vti[$f]_${n}";
	print "Gecode::PC_GEN_" . $pcspecial{$f}{$n};
      } else {
	print "Gecode::PC_GEN_ASSIGNED + " . $o;
	$o++;
      }
      print ";\n";
    }
    print $pcftr[$f];
    print $ftr[$f];
    print $endif[$f];
  }


  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    if ($dispose[$f]) {
      print "\n#ifndef GECODE_HAVE_VAR_DISPOSE\n";
      print "#define GECODE_HAVE_VAR_DISPOSE 1\n";
      print "#endif\n\n";
    }
    print $hdr[$f];
    print "  /// Configuration class for $name[$f]-variable implementations\n";
    print "  class $conf[$f] {\n";
    print "  public:\n";
    print "    /// Index for processing and update\n";
    print "    static const int idx_pu = ";
    if ($f == 0) {
      print "0;\n";
    } else {
      print "$namespace[$f-1]::$conf[$f-1]::idx_pu+1;\n";
    }
    print "    /// Index for disposal\n";
    print "    static const int idx_d = ";
    if ($dispose[$f]) {
      if ($f == 0) {
	print "0;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_d+1;\n";
      }
    } else {
      if ($f == 0) {
	print "-1;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_d;\n";
      }
    }
    print "    /// Maximal propagation condition\n";
    print "    static const Gecode::PropCond pc_max = $maxpc[$f];\n";
    print "    /// Start of bits for propagator modification event\n";
    print "    static const int pme_bits_fst = ";
    if ($f == 0) {
      print "0;\n";
    } else {
      print "$namespace[$f-1]::$conf[$f-1]::pme_bits_lst;\n";
    }
    print "    /// Number of bits for propagator modification event\n";
    print "    static const int me_bits_num = $bits[$f];\n";
    print "    /// Bitmask for propagator modification event\n";
    print "    static const int me_bits_mask = (1 << $bits[$f]) - 1;\n";
    print "    /// End of bits for propagator modification event\n";
    print "    static const int pme_bits_lst = pme_bits_fst + me_bits_num;\n";
    print "    /// Bit pattern for assigned propagator modification event\n";
    print "    static const Gecode::PropModEvent pme_assigned = (1 << pme_bits_fst);\n";
    print "    /// Return difference when changing modification event \\a me2 to \\a me1\n";
    print "    static ModEvent mec(ModEvent me1, ModEvent me2);\n";
    print "    /// Variable type identifier for reflection\n";
    print "    static GECODE_KERNEL_EXPORT const Support::Symbol vti;\n";
    print "  };\n";
    print $ftr[$f];
    if (!($ifdef[$f] eq "")) {
      print "#else\n";
      print $hdr[$f];
      print "  /// Configuration class for $name[$f]-variable implementations\n";
      print "  class $conf[$f] {\n";
      print "  public:\n";
      print "    /// Index for processing and update\n";
      print "    static const int idx_pu = ";
      if ($f == 0) {
	print "-1;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_pu;\n";
      }
      print "    /// Index for disposal\n";
      print "    static const int idx_d = ";
      if ($f == 0) {
	print "-1;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_d;\n";
      }
      print "    /// End of bits for propagator modification event\n";
      print "    static const int pme_bits_lst = ";
      if ($f == 0) {
	print "0;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::pme_bits_lst;\n";
      }
      print "    /// Bit pattern for assigned propagator modification event\n";
      print "    static const Gecode::PropModEvent pme_assigned = 0;\n";
      print "  };\n";
      print $ftr[$f];
    }
    print $endif[$f];
  }
  print "\n";
  print "namespace Gecode {\n\n";
  print "  /// Configuration class for all variable implementations\n";
  print "  class AllVarConf {\n";
  print "  public:\n";
  print "    /// Index for processing and update\n";
  print "    static const int idx_pu = $namespace[$n_files-1]::$conf[$n_files-1]::idx_pu+1;\n";
  print "    /// Index for dispose\n";
  print "    static const int idx_d = $namespace[$n_files-1]::$conf[$n_files-1]::idx_d+1;\n";
  print "    /// Bit pattern for assigned propagator modification event\n";
  print "    static const Gecode::PropModEvent pme_assigned =\n";
  for ($f=0; $f<$n_files; $f++) {
    print "        $namespace[$f]::$conf[$f]::pme_assigned";
    if ($f+1 == $n_files) {
      print ";\n";
    } else {
      print " |\n";
    }
  }
  print "  };\n\n}\n\n";
}

if ($gen_typecc) {
  print "#include \"gecode/kernel.hh\"\n";
  print "\n";
  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];
    print "  const Support::Symbol $conf[$f]::vti = \"$name[$f]\";\n";
    print $ftr[$f];
    print $endif[$f];
  }
  print "\n";
}

if ($gen_header) {

  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];
    print "  forceinline ModEvent\n";
    print "  $conf[$f]::mec(ModEvent me1, ModEvent me2) {\n";

    if ($me_max_n[$f] == 2) {
      print "    return me2^me1;\n";
    } elsif ($me_max_n[$f] <= 4) {
      print "    const int med = (\n";

      for ($i=0; $i<$me_max_n[$f];$i++) {
	$n1 = $val2me[$f][$i];
	print "      (\n";
	for ($j=0; $j<$me_max_n[$f];$j++) {
	  $n2 = $val2me[$f][$j];
	  if ($n1 eq "NONE") {
	    $n3 = $n2;
	  } elsif ($n2 eq "NONE") {
	    $n3 = $n1;
	  } else {
	    $n3 = $mec{$f}{$n1}{$n2};
	  }
	  print "        ((ME_$vti[$f]_$n2 ^ ME_$vti[$f]_$n3) << ";
	  print (($i << 3) + ($j << 1));
	  if ($j+1 == $me_max_n[$f]) {
	    print ")   ";
	  } else {
	    print ") | ";
	  }
	  print " // [ME_$vti[$f]_$n1][ME_$vti[$f]_$n2]\n";
	}
	if ($i+1 == $me_max_n[$f]) {
	  print "      )\n";
	} else {
	  print "      ) |\n";
	}
      }
      print "    );\n";
      print "    return (((med >> (me1 << 3)) >> (me2 << 1)) & 3);\n";
    } else {
      print "    static const unsigned char med[$me_max[$f]][$me_max[$f]] = {\n";
      for ($i=0; $i<$me_max_n[$f];$i++) {
	$n1 = $val2me[$f][$i];
	print "      {\n";
	for ($j=0; $j<$me_max_n[$f];$j++) {
	  $n2 = $val2me[$f][$j];
	  if ($n1 eq "NONE") {
	    $n3 = $n2;
	  } elsif ($n2 eq "NONE") {
	    $n3 = $n1;
	  } else {
	    $n3 = $mec{$f}{$n1}{$n2};
	  }
	  print "        ME_$vti[$f]_$n2 ^ ME_$vti[$f]_$n3";
	  if ($j+1 == $me_max_n[$f]) {
	    print " ";
	  } else {
	    print ",";
	  }
	  print " // [ME_$vti[$f]_$n1][ME_$vti[$f]_$n2]\n";
	}
	print "      }";
	if ($i+1 == $me_max_n[$f]) {
	  print "\n";
	} else {
	  print ",\n";
	}
      }
      print "    };\n";
      print "    return med[me1][me2];\n";
    }
    print "  }\n\n";
    print "  /// Base-class for $name[$f]-variable implementations\n";
    print "  class $class[$f] : public $base[$f] {\n";
    if ($dispose[$f]) {
    print <<EOF
  private:
    /// Link to next variable, used for disposal
    $class[$f]* _next_d;
EOF
;
  }

  print <<EOF
  protected:
    /// Constructor for cloning \\a x
    $class[$f](Space* home, bool share, $class[$f]\& x);
  public:
    /// Constructor for creating static instance of variable
    $class[$f](void);
    /// Constructor for creating variable
    $class[$f](Space* home);
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

  if ($dispose[$f]) {
  print <<EOF
    /// Return link to next variable to be disposed
    $class[$f]* next_d(void) const;
EOF
;
  }

  print <<EOF

  };

EOF
;

if ($dispose[$f]) {
  print <<EOF

  forceinline
  $class[$f]::$class[$f](void) {}

  forceinline
  $class[$f]::$class[$f](Space* home)
    : $base[$f](home) {
     _next_d = static_cast<$class[$f]*>(vars_d(home)); vars_d(home,this);
  }

  forceinline
  $class[$f]::$class[$f](Space* home, bool share, $class[$f]\& x)
    : $base[$f](home,share,x) {
     _next_d = static_cast<$class[$f]*>(vars_d(home)); vars_d(home,this);
  }

  forceinline $class[$f]*
  $class[$f]::next_d(void) const {
    return _next_d;
  }

EOF
;
} else {
  print <<EOF

  forceinline
  $class[$f]::$class[$f](void) {}

  forceinline
  $class[$f]::$class[$f](Space* home)
    : $base[$f](home) {}

  forceinline
  $class[$f]::$class[$f](Space* home, bool share, $class[$f]\& x)
    : $base[$f](home,share,x) {}
EOF
;
}
  print <<EOF

  forceinline void
  $class[$f]::subscribe(Space* home, Propagator* p, PropCond pc, bool assigned, bool process) {
    $base[$f]::subscribe(home,p,pc,assigned,$me_subscribe[$f],process);
  }
  forceinline void
  $class[$f]::subscribe(Space* home, Advisor* a, bool assigned) {
    $base[$f]::subscribe(home,a,assigned);
  }

EOF
;

if ($me_max_n[$f] == 2) {
  print <<EOF
  forceinline bool
  $class[$f]::notify(Space* home, ModEvent, Delta* d) {
    return $base[$f]::notify(home,d);
  }

EOF
;
} else {
  print <<EOF
  forceinline bool
  $class[$f]::notify(Space* home, ModEvent me, Delta* d) {
    return $base[$f]::notify(home,me,d);
  }

EOF
;
}
print $ftr[$f];

print $endif[$f];

}
    print <<EOF

namespace Gecode {

  forceinline void
  Space::process(void) {
EOF
;

  for ($f = $n_files-1; $f>=0; $f--) {
    print $ifdef[$f];
    print "    {\n";
    if (!($namespace[$f] eq "")) {
      print "      using namespace $namespace[$f];\n";
    }
    print <<EOF
      $base[$f]* x = $base[$f]::vars_pu(this);
      if (x != NULL) {
        $base[$f]::vars_pu(this,NULL);
EOF
;

    if ($me_max_n[$f] == 2) {
      print <<EOF
        do {
          x->process(this); x = x->next();
        } while (x != NULL);
EOF
;

    } else {
      print <<EOF
        do {
          switch (x->modevent()) {
EOF
;

  for ($i=0; $i<$pc_n[$f]; $i++) {
     if ($pcspecial{$f}{$pcn[$f][$i]} eq "ASSIGNED") {
       $val2pc[$f][0] = $pcn[$f][$i];
     }
  }
  $o = 1;
  for ($i=0; $i<$pc_n[$f]; $i++) {
     if (!($pcspecial{$f}{$pcn[$f][$i]} eq "ASSIGNED")) {
       $val2pc[$f][$o] = $pcn[$f][$i]; $o++;
     }
  }

  for ($i=0; $i<$me_n[$f]; $i++) {
    $n = $men[$f][$i];
    if ($mespecial{$f}{$n} eq "ASSIGNED") {
      print "          case ME_$vti[$f]_$n:\n";
      print "            x->process(this);\n";
      print "            break;\n";
    } elsif (!($mespecial{$f}{$n} eq "NONE") && !($mespecial{$f}{$n} eq "FAILED")) {
      print "          case ME_$vti[$f]_$n:\n";
      print "            // Conditions: ";
      for ($j=0; $j<$pc_n[$f]; $j++) {
        if ($mepc{$f}{$men[$f][$i]}{$pcn[$f][$j]}) {
          print $pcn[$f][$j] . " ";
        }
      }
      print "\n";
      for ($j=0; $j<$pc_n[$f]; $j++) {
	if ($mepc{$f}{$men[$f][$i]}{$val2pc[$f][$j]}) {
	  # Found initial entry (plus one for stopping)
	  print "            x->process(this,PC_$vti[$f]_" . $val2pc[$f][$j] . ",";
	  # Look for all connected entries
	  while ($mepc{$f}{$men[$f][$i]}{$val2pc[$f][$j+1]}) {
	    $j++;
          }
	  # Found last entry
	  print "PC_$vti[$f]_" . $val2pc[$f][$j] . ",ME_$vti[$f]_$n);\n";
	}
      }
      print "            break;\n";
    }
  }


  print <<EOF
          default: GECODE_NEVER;
          }
          x = x->next();
        } while (x != NULL);
EOF
;

}

  print <<EOF
      }
    }
EOF
;
  print $endif[$f];

}
  print <<EOF
  }
}
EOF
;


    print <<EOF
namespace Gecode {

  forceinline void
  Space::update(ActorLink** s) {
EOF
;

  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print "    {\n";
    if (!($namespace[$f] eq "")) {
      print "      using namespace $namespace[$f];\n";
    }
    print <<EOF
      $base[$f]* x = $base[$f]::vars_pu(this);
      if (x != NULL) {
        $base[$f]::vars_pu(this,NULL);
        do {
          x->forward()->update(x,s); x = x->next();
        } while (x != NULL);
      }
    }
EOF
;
  print $endif[$f];

}
  print <<EOF
  }
}
EOF
;



}

print "// STATISTICS: kernel-var\n";
