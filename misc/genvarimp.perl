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
  } elsif ($arg eq "-typehpp") {
    $gen_typehpp   = 1;
  }
}

$n_files = 0;
while ($arg = $ARGV[$i]) {
  $files[$n_files] = $arg;
  $n_files++; $i++;
}

print <<EOF
/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the following files instead:
EOF
;

for ($f=0; $f<$n_files; $f++) {
  print " *     - $files[$f]\n";
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
  open (FILE, $files[$f]) || die "Could not open ".$files[$f];

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


  $free_bits[$f] = 0;

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
	} elsif ($l =~ /^Bits:\s*([^ \t\r\n]+)/io) {
	  $free_bits[$f] = $1+0;
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
	    } elsif ($rhs eq "FAILED") {
	      $me_failed[$f] = "ME_$vti[$f]_$lhs";
	    } els
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
      $mec{$f}{"NONE"}{$n} = $n;
      $mec{$f}{$n}{"NONE"} = $n;
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
	  if (!($rhs eq "ASSIGNED") && !($rhs eq "NONE")) {
	    die "Unknown special propagation condition: $rhs\n";
	  }
	  $pcspecial{$f}{$lhs} = $rhs;
	  if ($rhs eq "ASSIGNED") {
	    $pc_assigned[$f] = "PC_$vti[$f]_$lhs";
	  }
	  if ($rhs eq "NONE") {
	    $pc_none[$f] = "PC_$vti[$f]_$lhs";
	  }
	  $n = $lhs;
	} elsif ($l =~ /^Name:\s*(\w+)/io) {
	  # Found a normal modification event
	  $n = $1;
	} elsif ($l =~ /^ScheduledBy:\s*(.+)/io) {
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

  $len = 0;
  for ($i=0; $i<$me_max_n[$f];$i++) {
    $n = $val2me[$f][$i];
    $me_a{$f}{$n} = "ME_$vti[$f]_$n";
    if (length($me_a{$f}{$n}) > $len) {
      $len = length($me_a{$f}{$n});
    }
  }
  for ($i=0; $i<$me_max_n[$f]; $i++) {
    $n = $val2me[$f][$i];
    while ($len > length($me_a{$f}{$n})) {
      $me_a{$f}{$n} = "$me_a{$f}{$n} ";
    }
  }

  for ($b=1; (1 << $b) < $me_max_n[$f]; $b++) {}
  $bits[$f] = $b;
}

if ($gen_typehpp) {
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

    $o = 1;
    for ($i=0; $i<$pc_n[$f]; $i++) {
      $n = $pcn[$f][$i];
      print $pch[$f][$i];
      print "  const Gecode::PropCond PC_$vti[$f]_${n} = ";
      if ($pcspecial{$f}{$n}) {
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
      print "\n#ifndef GECODE_HAS_VAR_DISPOSE\n";
      print "#define GECODE_HAS_VAR_DISPOSE 1\n";
      print "#endif\n\n";
    }
    print $hdr[$f];
    print "  /// Configuration for $name[$f]-variable implementations\n";
    print "  class $conf[$f] {\n";
    print "  public:\n";
    print "    /// Index for cloning\n";
    print "    static const int idx_c = ";
    if ($f == 0) {
      print "0;\n";
    } else {
      print "$namespace[$f-1]::$conf[$f-1]::idx_c+1;\n";
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
    print "    /// Freely available bits\n";
    print "    static const int free_bits = $free_bits[$f];\n";
    print "    /// Start of bits for modification event delta\n";
    print "    static const int med_fst = ";
    if ($f == 0) {
      print "0;\n";
    } else {
      print "$namespace[$f-1]::$conf[$f-1]::med_lst;\n";
    }
    print "    /// End of bits for modification event delta\n";
    print "    static const int med_lst = med_fst + $bits[$f];\n";
    print "    /// Bitmask for modification event delta\n";
    print "    static const int med_mask = ((1 << $bits[$f]) - 1) << med_fst;\n";
    print "    /// Combine modification events \\a me1 and \\a me2\n";
    print "    static Gecode::ModEvent me_combine(Gecode::ModEvent me1, Gecode::ModEvent me2);\n";
    print "    /// Update modification even delta \\a med by \\a me, return true on change\n";
    print "    static bool med_update(Gecode::ModEventDelta& med, Gecode::ModEvent me);\n";
    print "  };\n";
    print $ftr[$f];
    if (!($ifdef[$f] eq "")) {
      print "#else\n";
      print $hdr[$f];
      print "  /// Dummy configuration for $name[$f]-variable implementations\n";
      print "  class $conf[$f] {\n";
      print "  public:\n";
      print "    /// Index for cloning\n";
      print "    static const int idx_c = ";
      if ($f == 0) {
	print "-1;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_c;\n";
      }
      print "    /// Index for disposal\n";
      print "    static const int idx_d = ";
      if ($f == 0) {
	print "-1;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::idx_d;\n";
      }
      print "    /// End of bits for modification event delta\n";
      print "    static const int med_lst = ";
      if ($f == 0) {
	print "0;\n";
      } else {
	print "$namespace[$f-1]::$conf[$f-1]::med_lst;\n";
      }
      print "  };\n";
      print $ftr[$f];
    }
    print $endif[$f];
  }
  print "\n";
  print "namespace Gecode {\n\n";
  print "  /// Configuration for all variable implementations\n";
  print "  class AllVarConf {\n";
  print "  public:\n";
  print "    /// Index for cloning\n";
  print "    static const int idx_c = $namespace[$n_files-1]::$conf[$n_files-1]::idx_c+1;\n";
  print "    /// Index for dispose\n";
  print "    static const int idx_d = $namespace[$n_files-1]::$conf[$n_files-1]::idx_d+1;\n";
  print "    /// Combine modification event delta \\a med1 with \\a med2\n";
  print "    static ModEventDelta med_combine(ModEventDelta med1, ModEventDelta med2);\n";
  print "  };\n\n}\n\n";
  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];
    print "  forceinline Gecode::ModEvent\n";
    print "  $conf[$f]::me_combine(Gecode::ModEvent me1, Gecode::ModEvent me2) {\n";

    if ($me_max_n[$f] == 2) {
      print "    return me1 | me2;\n";
    } elsif ($me_max_n[$f] <= 4) {
      print "    static const Gecode::ModEvent me_c = (\n";

      for ($i=0; $i<$me_max_n[$f];$i++) {
	$n1 = $val2me[$f][$i];
	print "      (\n";
	for ($j=0; $j<$me_max_n[$f];$j++) {
	  $n2 = $val2me[$f][$j];
	  $n3 = $mec{$f}{$n1}{$n2};
	  $shift = (($i << 3) + ($j << 1));
	  if ($shift < 10) {
	    $shift = " $shift";
	  }
	  print "        ($me_a{$f}{$n3} << $shift)";
	  if ($j+1 == $me_max_n[$f]) {
	    print "   ";
	  } else {
	    print " | ";
	  }
	  print " // [$me_a{$f}{$n1}][$me_a{$f}{$n2}]\n";
	}
	if ($i+1 == $me_max_n[$f]) {
	  print "      )\n";
	} else {
	  print "      ) |\n";
	}
      }
      print "    );\n";
      print "    return ((me_c >> (me2 << 3)) >> (me1 << 1)) & 3;\n";
    } else {
      print "    static const Gecode::ModEvent me_c[$me_max[$f]][$me_max[$f]] = {\n";
      for ($i=0; $i<$me_max_n[$f];$i++) {
	$n1 = $val2me[$f][$i];
	print "      {\n";
	for ($j=0; $j<$me_max_n[$f];$j++) {
	  $n2 = $val2me[$f][$j];
	  $n3 = $mec{$f}{$n1}{$n2};
	  print "        $me_a{$f}{$n3}";
	  if ($j+1 == $me_max_n[$f]) {
	    print " ";
	  } else {
	    print ",";
	  }
	  print " // [$me_a{$f}{$n1}][$me_a{$f}{$n2}]\n";
	}
	print "      }";
	if ($i+1 == $me_max_n[$f]) {
	  print "\n";
	} else {
	  print ",\n";
	}
      }
      print "    };\n";
      print "    return me_c[me1][me2];\n";
    }
    print "  }\n";
    print "  forceinline bool\n";
    print "  $conf[$f]::med_update(Gecode::ModEventDelta& med, Gecode::ModEvent me) {\n";

    print "    switch (me) {\n";
    if ($me_max_n[$f] == 2) {
      $ME_NONE = "ME_$vti[$f]_NONE";
      if ($val2me[$f][0] eq "NONE") {
        $ME_ASSIGNED = "ME_$vti[$f]_$val2me[$f][1]";
      } else {
        $ME_ASSIGNED = "ME_$vti[$f]_$val2me[$f][0]";
      }
      print "    case $ME_NONE:\n";
      print "      return false;\n";
      print "    case $ME_ASSIGNED:\n";
      print "      if ((med & ($ME_ASSIGNED << med_fst)) != 0)\n";
      print "        return false;\n";
      print "      med |= $ME_ASSIGNED << med_fst;\n";
      print "      break;\n";
    } else {
      $lvti = lc($vti[$f]);
      for ($i=0; $i<$me_max_n[$f];$i++) {
	$n1 = $val2me[$f][$i];
	$weak = 1;
	for ($j=0; $j<$me_max_n[$f];$j++) {
	  $n2 = $val2me[$f][$j];
	  if (!($n2 eq "NONE") && !($n2 eq $n1) &&
	      !($mec{$f}{$n1}{$n2} eq $n2)) {
	    $weak = 0;
	  }
	}
        print "    case ME_$vti[$f]_$n1:\n";
        if ($n1 eq "NONE") {
          print "      return false;\n";
        } elsif ($mespecial{$f}{$n1} eq "ASSIGNED") {
          print "      {\n";
          print "        Gecode::ModEventDelta med_$lvti = med & med_mask;\n";
          print "        if (med_$lvti == (ME_$vti[$f]_$n1 << med_fst))\n";
          print "          return false;\n";
          print "        med ^= med_$lvti;\n";
          print "        med ^= ME_$vti[$f]_$n1 << med_fst;\n";
          print "        break;\n";
          print "      }\n";
	} elsif ($weak) {
          print "      {\n";
          print "        Gecode::ModEventDelta med_$lvti = med & med_mask;\n";
          print "        if (med_$lvti != 0)\n";
          print "          return false;\n";
          print "        med |= ME_$vti[$f]_$n1 << med_fst;\n";
          print "        break;\n";
          print "      }\n";
	} else {
          print "      {\n";
          if ($me_max_n[$f] <= 8) {
            print "        static const Gecode::ModEvent me_c = (\n";
	    for ($j=0; $j<$me_max_n[$f];$j++) {
	      $n2 = $val2me[$f][$j];
	      $n3 = $mec{$f}{$n1}{$n2};
              $shift = $j << 2;
	      if ($shift < 10) {
                $shift = " $shift";
              }
	      print "          (($me_a{$f}{$n2} ^ $me_a{$f}{$n3}) << $shift)";
	      if ($j+1 != $me_max_n[$f]) {
	        print " |\n";
	      }
	    }
            print "\n        );\n";
            print "        Gecode::ModEvent me_o = (med & med_mask) >> med_fst;\n";
            print "        Gecode::ModEvent me_n = (me_c >> (me_o << 2)) & (med_mask >> med_fst);\n";
            print "        if (me_n == 0)\n";
            print "          return false;\n";
            print "        med ^= me_n << med_fst;\n";
            print "        break;\n";
          } else {
            print "        static const Gecode::ModEventDelta me_c[$me_max[$f]] = {\n";
	    for ($j=0; $j<$me_max_n[$f];$j++) {
	      $n2 = $val2me[$f][$j];
	      $n3 = $mec{$f}{$n1}{$n2};
	      print "          ($me_a{$f}{$n2} ^ $me_a{$f}{$n3}) << med_fst";
	      if ($j+1 != $me_max_n[$f]) {
	        print ",\n";
	      }
	    }
            print "\n        };\n";
            print "        Gecode::ModEvent me_o = (med & med_mask) >> med_fst;\n";
            print "        Gecode::ModEventDelta med_n = me_c[me_o];\n";
            print "        if (med_n == 0)\n";
            print "          return false;\n";
            print "        med ^= med_n;\n";
            print "        break;\n";
          }
          print "      }\n";
        }
      }
    }
    print "    default: GECODE_NEVER;\n";
    print "    }\n";
    print "    return true;\n";
    print "  }\n\n";
    print $ftr[$f];
    print $endif[$f];
  }
  print "namespace Gecode {\n";
  print "  forceinline ModEventDelta\n";
  print "  AllVarConf::med_combine(ModEventDelta med1, ModEventDelta med2) {\n";
  for ($f = 0; $f<$n_files; $f++) {
    $vic = "$namespace[$f]::$conf[$f]";
    print $ifdef[$f];
    print "    (void) ${vic}::med_update(med1,(med2 & ${vic}::med_mask) >> ${vic}::med_fst);\n";
    print $endif[$f];
  }
  print "    return med1;\n";
  print "  }\n}\n\n";
}

if ($gen_header) {
  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];
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
    $class[$f](Gecode::Space& home, bool share, $class[$f]\& x);
  public:
    /// Constructor for creating static instance of variable
    $class[$f](void);
    /// Constructor for creating variable
    $class[$f](Gecode::Space& home);
    /// \\name Dependencies
    //\@{
    /** \\brief Subscribe propagator \\a p with propagation condition \\a pc
     *
     * In case \\a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     *
     * In case the variable is assigned (that is, \\a assigned is
     * true), the subscribing propagator is scheduled for execution.
     * Otherwise, the propagator subscribes and is scheduled for execution
     * with modification event \\a me provided that \\a pc is different
     * from \\a $pc_assigned[$f].
     */
    void subscribe(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned, bool schedule);
    /// Subscribe advisor \\a a if \\a assigned is false.
    void subscribe(Gecode::Space& home, Gecode::Advisor& a, bool assigned);
    /// Notify that variable implementation has been modified with modification event \\a me and delta information \\a d
    Gecode::ModEvent notify(Gecode::Space& home, Gecode::ModEvent me, Gecode::Delta& d);
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

  print $ftr[$f];
  print $endif[$f];


}

  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print $hdr[$f];

if ($dispose[$f]) {
  print <<EOF

  forceinline
  $class[$f]::$class[$f](void) {}

  forceinline
  $class[$f]::$class[$f](Gecode::Space& home)
    : $base[$f](home) {
     _next_d = static_cast<$class[$f]*>(vars_d(home)); vars_d(home,this);
  }

  forceinline
  $class[$f]::$class[$f](Gecode::Space& home, bool share, $class[$f]\& x)
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
  $class[$f]::$class[$f](Gecode::Space& home)
    : $base[$f](home) {}

  forceinline
  $class[$f]::$class[$f](Gecode::Space& home, bool share, $class[$f]\& x)
    : $base[$f](home,share,x) {}
EOF
;
}
  print <<EOF

  forceinline void
  $class[$f]::subscribe(Gecode::Space& home, Gecode::Propagator& p, Gecode::PropCond pc, bool assigned, bool schedule) {
    $base[$f]::subscribe(home,p,pc,assigned,$me_subscribe[$f],schedule);
  }
  forceinline void
  $class[$f]::subscribe(Gecode::Space& home, Gecode::Advisor& a, bool assigned) {
    $base[$f]::subscribe(home,a,assigned);
  }

EOF
;

if ($me_max_n[$f] == 2) {
  print <<EOF
  forceinline Gecode::ModEvent
  $class[$f]::notify(Gecode::Space& home, Gecode::ModEvent, Gecode::Delta& d) {
    schedule(home,$pc_assigned[$f],$pc_assigned[$f],$me_assigned[$f]);
    if (!$base[$f]::advise(home,$me_assigned[$f],d))
      return $me_failed[$f];
    cancel(home);
    return $me_assigned[$f];
  }

EOF
;
} else {
  print <<EOF
  forceinline Gecode::ModEvent
  $class[$f]::notify(Gecode::Space& home, Gecode::ModEvent me, Gecode::Delta& d) {
    switch (me) {
EOF
;

  for ($i=0; $i<$pc_n[$f]; $i++) {
     if ($pcspecial{$f}{$pcn[$f][$i]} eq "ASSIGNED") {
       $val2pc[$f][0] = $pcn[$f][$i];
     }
  }
  $o = 1;
  for ($i=0; $i<$pc_n[$f]; $i++) {
     if (!($pcspecial{$f}{$pcn[$f][$i]} eq "ASSIGNED") &&
	 !($pcspecial{$f}{$pcn[$f][$i]} eq "NONE")) {
       $val2pc[$f][$o] = $pcn[$f][$i]; $o++;
     }
  }

  for ($i=0; $i<$me_n[$f]; $i++) {
    $n = $men[$f][$i];
    if (!($mespecial{$f}{$n} eq "NONE") && !($mespecial{$f}{$n} eq "FAILED")) {
      print "    case ME_$vti[$f]_$n:\n";
      print "      // Conditions: ";
      $fst = 1;
      for ($j=0; $j<$pc_n[$f]; $j++) {
        if ($mepc{$f}{$men[$f][$i]}{$pcn[$f][$j]}) {
          if ($fst) {
            $fst = 0;
          } else {
            print ", ";
          }
          print $pcn[$f][$j];
        }
      }
      print "\n";
      for ($j=0; $j<$pc_n[$f]; $j++) {
	if ($mepc{$f}{$men[$f][$i]}{$val2pc[$f][$j]}) {
	  # Found initial entry (plus one for stopping)
	  print "      schedule(home,PC_$vti[$f]_" . $val2pc[$f][$j] . ",";
	  # Look for all connected entries
	  while ($mepc{$f}{$men[$f][$i]}{$val2pc[$f][$j+1]}) {
	    $j++;
          }
	  # Found last entry
	  print "PC_$vti[$f]_" . $val2pc[$f][$j] . ",ME_$vti[$f]_$n);\n";
	}
      }
      print "      if (!$base[$f]::advise(home,ME_$vti[$f]_$n,d))\n";
      print "        return $me_failed[$f];\n";
      if ($mespecial{$f}{$n} eq "ASSIGNED") {
        print "      cancel(home);\n";
      }
      print "      break;\n";
    }
  }


  print <<EOF
    default: GECODE_NEVER;
    }
    return me;
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
  Space::update(ActorLink** sub) {
EOF
;

  for ($f = 0; $f<$n_files; $f++) {
    print $ifdef[$f];
    print "    $base[$f]::update(*this,sub);\n";
    print $endif[$f];
  }

  print <<EOF
  }
}
EOF
;



}

print "// STATISTICS: kernel-var\n";
