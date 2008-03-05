dnl
dnl Main authors:
dnl   Guido Tack <tack@gecode.org>
dnl
dnl Copyright:
dnl   Guido Tack, 2004, 2005
dnl
dnl Last modified:
dnl   $Date: 2005-10-27 21:01:30 +0200 (Thu, 27 Oct 2005) $
dnl   by $Author: schulte $
dnl   $Revision: 2420 $
dnl
dnl This file is part of Gecode, the generic constraint
dnl development environment:
dnl   http://www.gecode.org
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining
dnl a copy of this software and associated documentation files (the
dnl "Software"), to deal in the Software without restriction, including
dnl without limitation the rights to use, copy, modify, merge, publish,
dnl distribute, sublicense, and/or sell copies of the Software, and to
dnl permit persons to whom the Software is furnished to do so, subject to
dnl the following conditions:
dnl
dnl The above copyright notice and this permission notice shall be
dnl included in all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
dnl EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
dnl NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
dnl LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
dnl OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
dnl WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
dnl
dnl

dnl check whether we have pkgconfig and should generate config files for it
AC_DEFUN([AC_GECODE_PKGCONFIG],
	[AC_CHECK_PROG(PKGCONFIG, pkg-config, [found])
	 AC_MSG_CHECKING(whether to generate pkg-config files)
	 if test "${PKGCONFIG}x" = "x"; then
	    AC_MSG_RESULT(no)
	    AC_SUBST(HAS_PKGCONFIG, "no")
	 else
	    AC_MSG_RESULT(yes)
	    AC_SUBST(HAS_PKGCONFIG, "yes")
	 fi])

AC_DEFUN([AC_GECODE_GET_OS],
	[AC_ARG_WITH([host-os],
           AC_HELP_STRING([--with-host-os],
	   [Override operating system test. Valid values are Linux, Darwin, and Windows.]))
	 AC_MSG_CHECKING([for the host operating system])
         if test "${with_host_os:-no}" = "no"; then
	   guess_host_os=$(uname -s 2>/dev/null)
	 else
	   guess_host_os=${with_host_os}
   	 fi
	 dnl Try to find out operating system
	 case ${guess_host_os} in
	   GNU/kFreeBSD|*inux*)
	   host_os=linux
	   AC_MSG_RESULT([Linux])
	   ;;
	   *arwin*)
	   host_os=darwin
	   AC_MSG_RESULT([Darwin])
	   ;;
	   CYGWIN*|*indows*)
	   host_os=windows
	   AC_MSG_RESULT([Windows])
	   ;;
	   *)
	   AC_MSG_ERROR([Host OS not supported.])
	   ;;
	 esac])

dnl Macros:
dnl   AC_GECODE_ADD_TO_CXXFLAGS ([FLAG...])
dnl   AC_GECODE_ADD_TO_GLDFLAGS ([FLAG...])
dnl
dnl Description:
dnl   Add the flags to the corresponding variables
dnl
dnl Author:
dnl   Marco Kuhlmann <kuhlmann@ps.uni-sb.de>
dnl
AC_DEFUN([AC_GECODE_ADD_TO_CXXFLAGS],
   [CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1"])
AC_DEFUN([AC_GECODE_ADD_TO_COMPILERFLAGS],
   [CFLAGS="${CFLAGS}${CFLAGS:+ }$1"
    CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1"])
AC_DEFUN([AC_GECODE_ADD_TO_CFLAGS],
   [CFLAGS="${CFLAGS}${CFLAGS:+ }$1"])
AC_DEFUN([AC_GECODE_ADD_TO_GLDFLAGS],
   [GLDFLAGS="${GLDFLAGS}${GLDFLAGS:+ }$1"])
AC_DEFUN([AC_GECODE_ADD_TO_PKG_CXXFLAGS],
  [ac_gecode_pkg_cxxflags="${ac_gecode_pkg_cxxflags}${ac_gecode_pkg_cxxflags:+ }$1"])
AC_DEFUN([AC_GECODE_ADD_TO_DLLFLAGS],
  [DLLFLAGS="${DLLFLAGS}${DLLFLAGS:+ }$1"])


dnl Macro:
dnl   AC_GECODE_CHECK_CXXFLAG (FLAG, [ACTION-IF-TRUE,
dnl                                [ACTION-IF-FALSE]])
dnl
dnl Description:
dnl   Check whether FLAG is supported by the C++ compiler.  Run
dnl   the shell commands ACTION-IF-TRUE if it is, ACTION-IF-FALSE
dnl   otherwise.  If ACTION-IF-TRUE is not given, append FLAG to
dnl   the contents of $CXXFLAGS.
dnl
dnl Authors:
dnl   Leif Kornstaedt <kornstae@ps.uni-sb.de>
dnl   Marco Kuhlmann <kuhlmann@ps.uni-sb.de>
dnl
AC_DEFUN([AC_GECODE_CHECK_CXXFLAG],
  [AC_MSG_CHECKING(whether ${CXX} accepts [$1])
   ac_gecode_save_CXXFLAGS="${CXXFLAGS}"
   CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1 -Werror"
   AC_LANG_PUSH(C++)
   AC_COMPILE_IFELSE(AC_LANG_PROGRAM(),
     [AC_MSG_RESULT(yes)
      CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
      ifelse([$2], , [CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1"], [$2])],
     [AC_MSG_RESULT(no)
      CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
      ifelse([$3], , :, [$3])])
   AC_LANG_POP])dnl

AC_DEFUN([AC_GECODE_CHECK_COMPILERFLAG],
  [AC_MSG_CHECKING(whether ${CXX} accepts [$1])
   ac_gecode_save_CXXFLAGS="${CXXFLAGS}"
   CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1 -Werror"
   AC_LANG_PUSH(C++)
   AC_COMPILE_IFELSE(AC_LANG_PROGRAM(),
     [AC_MSG_RESULT(yes)
      CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
      CXXFLAGS="${CXXFLAGS}${CXXFLAGS:+ }$1"],
     [AC_MSG_RESULT(no)
      CXXFLAGS="${ac_gecode_save_CXXFLAGS}"])
   AC_LANG_POP
   
   AC_MSG_CHECKING(whether ${CC} accepts [$1])
   ac_gecode_save_CFLAGS="${CFLAGS}"
   CFLAGS="${CFLAGS}${CFLAGS:+ }$1 -Werror"
   AC_LANG_PUSH(C)
   AC_COMPILE_IFELSE(AC_LANG_PROGRAM(),
     [AC_MSG_RESULT(yes)
      CFLAGS="${ac_gecode_save_CFLAGS}"
      CFLAGS="${CFLAGS}${CFLAGS:+ }$1"],
     [AC_MSG_RESULT(no)
      CFLAGS="${ac_gecode_save_CFLAGS}"])
   AC_LANG_POP])dnl

dnl Macro:
dnl   AC_GECODE_CHECK_GLDFLAG (FLAG, [ACTION-IF-TRUE,
dnl                                  [ACTION-IF-FALSE]])
dnl
dnl Description:
dnl   Check whether FLAG is supported by the linker.  Run the
dnl   shell commands ACTION-IF-TRUE if it is, ACTION-IF-FALSE
dnl   otherwise.  If ACTION-IF-TRUE is not given, append FLAG to
dnl   the contents of $GLDFLAGS.
dnl
dnl Authors:
dnl   Leif Kornstaedt <kornstae@ps.uni-sb.de>
dnl   Marco Kuhlmann <kuhlmann@ps.uni-sb.de>
dnl
AC_DEFUN([AC_GECODE_CHECK_GLDFLAG],
  [AC_REQUIRE([AC_PROG_CXX])
   AC_MSG_CHECKING(whether the linker accepts [$1])
   ac_gecode_save_GLDFLAGS="${GLDFLAGS}"
   GLDFLAGS="${GLDFLAGS}${GLDFLAGS:+ }$1"
   AC_LINK_IFELSE(AC_LANG_PROGRAM(),
     [AC_MSG_RESULT(yes)
      GLDFLAGS="$ac_gecode_save_GLDFLAGS"
      ifelse([$2], , [GLDFLAGS="${GLDFLAGS}${GLDFLAGS:+ }$1"], [$2])],
     [AC_MSG_RESULT(no)
      GLDFLAGS="$ac_gecode_save_GLDFLAGS"
      ifelse([$3], , :, [$3])])])dnl

dnl @synopsis _AC_C_IFDEF(MACRO-NAME, ACTION-IF-DEF, ACTION-IF-NOT-DEF)
dnl
dnl Check for the definition of macro MACRO-NAME using the current
dnl language's compiler.
dnl
dnl @category Misc
dnl @author Ludovic Court�s <ludo@chbouib.org>
dnl @version 2004-09-07
dnl @license AllPermissive
AC_DEFUN([_AC_C_IFDEF],
  [AC_COMPILE_IFELSE([#ifndef $1
                      # error "Macro $1 is undefined!"
		      /* For some compilers (eg. SGI's CC), #error is not
		         enough...  */
		      please, do fail
		      #endif],
		     [$2], [$3])])

dnl @synopsis AC_CXX_COMPILER_VENDOR(VENDOR-NAME)
dnl
dnl Set VENDOR-NAME to the lower-case name of the compiler vendor or `unknown'
dnl if the compiler's vendor is unknown.
dnl
dnl @version 20040907
dnl @author  Ludovic Court�s <ludo@chbouib.org>
dnl Modified by G. Tack to recognize only those compilers we need.
AC_DEFUN([AC_CXX_COMPILER_VENDOR],
  [AC_ARG_WITH([compiler-vendor],
     AC_HELP_STRING([--with-compiler-vendor],
       [Override compiler test. Valid values are gnu, intel, and microsoft.]))
   AC_REQUIRE([AC_PROG_CXX])
   AC_REQUIRE([AC_PROG_CXXCPP])
   AC_CACHE_CHECK([the C++ compiler vendor],
    [ac_cv_cxx_compiler_vendor],

    [if test "${with_compiler_vendor:-no}" = "no"; then

     AC_LANG_PUSH([C++])

     dnl GNU C++
     _AC_C_IFDEF([__GNUG__],
       [ac_cv_cxx_compiler_vendor=gnu],
       [_AC_C_IFDEF([__INTEL_COMPILER],
                    [ac_cv_cxx_compiler_vendor=intel],
       [dnl Note:  We are using the C compiler because VC++ doesn't
        dnl recognize `.cc'(which is used by `configure') as a C++ file
        dnl extension and requires `/TP' to be passed.
        AC_LANG_PUSH([C])
        _AC_C_IFDEF([_MSC_VER],
                    [ac_cv_cxx_compiler_vendor=microsoft],
                    [ac_cv_cxx_compiler_vendor=unknown])
        AC_LANG_POP()])])

     AC_LANG_POP()
     else
       ac_cv_cxx_compiler_vendor=${with_compiler_vendor}
     fi
     $1="$ac_cv_cxx_compiler_vendor"])])dnl


dnl Macro:
dnl   AC_GECODE_VTI (vartype, help-string, default,
dnl		     [ACTION-IF-ENABLED])
dnl
dnl Description:
dnl   Introduce --enable-vartype-vars configure switch. The help-string
dnl   is used for the configure --help output.
dnl   default is either yes or no and says whether this option should be
dnl   enabled by default. ACTION-IF-ENABLED can be used to perform additional
dnl   actions in case this variable type is enabled.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_ADD_VTI],
   [AC_DEFINE(GECODE_HAS_$1_VARS)])
AC_DEFUN([AC_GECODE_VTI],
   [
   AC_ARG_ENABLE([$1-vars],
     AC_HELP_STRING([--enable-$1-vars],[build $2 @<:@default=$3@:>@]))
   AC_MSG_CHECKING(whether to build the $1 variables library)
   if test "${enable_$1_vars:-$3}" = "yes"; then
     AC_MSG_RESULT(yes)
     enable_$1_vars="yes";
     $4
     AC_GECODE_ADD_VTI(translit($1,`a-z', `A-Z'))
   else
     enable_$1_vars="no";
     AC_MSG_RESULT(no)
   fi
   AC_SUBST(enable_$1_vars, ${enable_$1_vars})
   ])

dnl Macro:
dnl   AC_GECODE_ENABLE_MODULE (module, default, help-string,
dnl                            [ACTION-IF-ENABLED])
dnl
dnl Description:
dnl   Produces a configure switch --enable-<module>.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_ENABLE_MODULE],
	 [
     AC_ARG_ENABLE([$1],
	   AC_HELP_STRING([--enable-$1],
	     [$3 @<:@default=$2@:>@]))
	 AC_MSG_CHECKING(whether to build $1)
	 if test "${enable_$1:-$2}" = "yes"; then
	    enable_$1="yes";
	    AC_MSG_RESULT(yes)
	    $4
	 else
	    enable_$1="no";
	    AC_MSG_RESULT(no)
	 fi
   	 AC_SUBST(enable_$1, ${enable_$1})])	

dnl Description:
dnl   Makes an enable check for a contrib
dnl   The third argument can be used for dependency checking 
dnl
dnl Authors:
dnl   Gr�goire Dooms <dooms@info.ucl.ac.be>
AC_DEFUN([AC_GECODE_ENABLE_CONTRIB],
[
   AC_ARG_ENABLE([$1],
     AC_HELP_STRING([--enable-$1],[build $2 @<:@default=yes@:>@]))
   AC_MSG_CHECKING(whether to build the $1 contrib library)
   if test "${enable_$1:-yes}" = "yes"; then
     AC_MSG_RESULT(yes)
     enable_$1="yes";
     $3
   else
     enable_$1="no";
     AC_MSG_RESULT(no)
   fi
   AC_SUBST(enable_$1, ${enable_$1})])


AC_DEFUN([AC_GECODE_STATICLIBS],
	 [if test "${host_os}" = "windows" -a \
                  "${ac_gecode_compiler_vendor}" = "gnu"; then
            enable_static="yes"
	    enable_shared="no"
          fi
          AC_ARG_ENABLE([static],
	   AC_HELP_STRING([--enable-static],
	     [build static libraries @<:@default=no@:>@]))
	  AC_MSG_CHECKING(whether to build static libraries)
 	  if test "${enable_static:-no}" = "yes"; then
 	     AC_DEFINE(GECODE_STATIC_LIBS)
 	     AC_SUBST(BUILDSTATIC, "yes")
 	     AC_MSG_RESULT(yes)
 	  else
 	     AC_SUBST(BUILDSTATIC, "no")
 	     AC_MSG_RESULT(no)
 	  fi
	  AC_ARG_ENABLE([shared],
	   AC_HELP_STRING([--enable-shared],
	     [build shared libraries @<:@default=yes@:>@]))
	  AC_MSG_CHECKING(whether to build shared libraries)
 	  if test "${enable_shared:-yes}" = "yes"; then
 	     AC_SUBST(BUILDDLL, "yes")
 	     AC_MSG_RESULT(yes)
 	  else
 	     AC_SUBST(BUILDDLL, "no")
 	     if test "${enable_static:-no}" = "no"; then
 	       AC_MSG_ERROR([One of --enable-static or --enable-shared must be given])
 	     fi
 	     AC_MSG_RESULT(no)
 	  fi
	  ])


AC_DEFUN([AC_GECODE_DEBUG],
	[AC_ARG_ENABLE([debug],
	   AC_HELP_STRING([--enable-debug],
	     [build with assertions @<:@default=no@:>@]))
	 AC_MSG_CHECKING(whether to build with debug symbols and assertions)
	 if test "${enable_debug:-no}" = "yes"; then
	    AC_MSG_RESULT(yes)
	 else
	    AC_MSG_RESULT(no)
       	    AC_GECODE_ADD_TO_COMPILERFLAGS(-DNDEBUG)
	 fi])

AC_DEFUN([AC_GECODE_LEAK_DEBUG],
	[AC_ARG_ENABLE([leak-debug],
	   AC_HELP_STRING([--enable-leak-debug],
	     [build with support for finding memory leaks @<:@default=no@:>@]))
	 AC_MSG_CHECKING(whether to build with support for finding memory leaks)
	 if test "${enable_leak_debug:-no}" = "yes"; then
	    AC_MSG_RESULT(yes)
	    AC_CHECK_DECL(mtrace,
			  [AC_DEFINE(GECODE_HAS_MTRACE)],
			  [AC_MSG_ERROR(mtrace not available.)],
			  [[#include <mcheck.h>]])	    
	 else
	    AC_MSG_RESULT(no)
	 fi])

AC_DEFUN([AC_GECODE_AUDIT],
	[AC_ARG_ENABLE([audit],
	   AC_HELP_STRING([--enable-audit],
	     [build with auditing code @<:@default=no@:>@]))
	 AC_MSG_CHECKING(whether to build with auditing code)
	 if test "${enable_audit:-no}" = "yes"; then
	    AC_DEFINE(GECODE_AUDIT)
	    AC_MSG_RESULT(yes)
	 else
	    AC_MSG_RESULT(no)
	 fi])


AC_DEFUN([AC_GECODE_PROFILE],
	 [AC_ARG_ENABLE([profile],
	   AC_HELP_STRING([--enable-profile],
	     [build with profiling information @<:@default=no@:>@]))
	 AC_MSG_CHECKING(whether to build with profiling information)
	 if test "${enable_profile:-no}" = "yes"; then
	    AC_MSG_RESULT(yes)
	    AC_GECODE_CHECK_CXXFLAG(-pg,
	       AC_GECODE_ADD_TO_COMPILERFLAGS(-pg),
	       AC_GECODE_CHECK_COMPILERFLAG(-p))
	 else
	    AC_MSG_RESULT(no)
	 fi])

AC_DEFUN([AC_GECODE_GCOV],
	 [AC_ARG_ENABLE([gcov],
	   AC_HELP_STRING([--enable-gcov],
	     [build with gcov support @<:@default=no@:>@]))
	 AC_MSG_CHECKING(whether to build with gcov support)
	 if test "${enable_gcov:-no}" = "yes"; then
	    AC_MSG_RESULT(yes)
	    AC_GECODE_CHECK_COMPILERFLAG(-fprofile-arcs)
	    AC_GECODE_CHECK_COMPILERFLAG(-ftest-coverage)
	    AC_GECODE_ADD_TO_DLLFLAGS("-fprofile-arcs")
	    AC_GECODE_ADD_TO_DLLFLAGS("-ftest-coverage")
	 else
	    AC_MSG_RESULT(no)
	 fi])


# Test for platform specific behaviour of arithmetic

AC_DEFUN([AC_GECODE_CHECK_ARITH],
	[AC_LANG_PUSH([C])
	 AC_CHECK_SIZEOF(int)
	 AC_MSG_CHECKING([if int has at least 32 bit])
	 AC_TRY_COMPILE([],
	   [
	   #if SIZEOF_INT>=4
	   #else
	   blablub
	   #endif
	   ],
	   [AC_MSG_RESULT(yes)],
	   [AC_MSG_ERROR([Gecode needs at least 32 bit integers.])])

	 AC_MSG_CHECKING([if doubles have a big enough mantissa])
	 AC_TRY_COMPILE([#include <float.h>],
	   [
	   #if DBL_MANT_DIG>=53
	   #else
	   blablub
	   #endif
	   ],
	   [AC_MSG_RESULT(yes)],
	   [AC_MSG_ERROR([Gecode needs a double mantissa of at least 53 bits.])])

	 AC_LANG_POP([C])])

AC_DEFUN([AC_GECODE_GCC_GENERAL_SWITCHES],
 [AC_DEFINE(forceinline, [inline])
  AC_GECODE_CHECK_COMPILERFLAG([-fPIC])
  AC_GECODE_CHECK_COMPILERFLAG([-Wextra])
  AC_GECODE_CHECK_COMPILERFLAG([-Wall])
  AC_GECODE_CHECK_COMPILERFLAG([-pipe])
  AC_GECODE_CHECK_CXXFLAG(-ggdb,
     AC_GECODE_ADD_TO_COMPILERFLAGS(-ggdb),
     AC_GECODE_CHECK_COMPILERFLAG(-g))

  AC_SUBST(docdir, "${datadir}/doc/gecode")

  dnl file extensions
  AC_SUBST(SBJEXT, "s")
  AC_SUBST(LIBEXT, "${DLLEXT}")
  AC_SUBST(LIBPREFIX, "libgecode")
  AC_SUBST(STATICLIBEXT, "a")
  AC_SUBST(MINUSLDIR, "-L${libdir}")
  AC_SUBST(LINKLIBDIR, "")

  cygpathprefix=$prefix
  test "x$cygpathprefix" = xNONE && cygpathprefix=$ac_default_prefix
  AC_SUBST(cygpathprefix, "${cygpathprefix}")

  AC_SUBST(COMPILEOBJ, "-c -o ")
  AC_SUBST(COMPILESBJ, "-S -o ")
  AC_SUBST(CXXIN, " ")
  AC_SUBST(CCIN, " ")
  AC_SUBST(EXAMPLES_EXTRA_CXXFLAGS, "")

  dnl Do not install stub .lib files (required for msvc)
  AC_SUBST(INSTALLLIBS, "no")

  AC_SUBST(LINKPREFIX, "-lgecode")
  AC_SUBST(LINKSUFFIX, "")

  dnl how to tell the compiler to output an object file
  AC_SUBST(LINKOUTPUT, "-o ")
  dnl how to tell the compiler to output an executable
  AC_SUBST(EXEOUTPUT, "-o ")

  dnl the names of the generated dlls
  AC_SUBST(SUPPORT, "support")
  AC_SUBST(KERNEL,  "kernel")
  AC_SUBST(SEARCH,  "search")
  AC_SUBST(INT,     "int")
  AC_SUBST(SET,     "set")
  AC_SUBST(CPLTSET, "cpltset")
  AC_SUBST(MM,      "minimodel")
  AC_SUBST(SER,     "serialization")
  AC_SUBST(GIST,    "gist")])


AC_DEFUN([AC_GECODE_GCC_OPTIMIZED_SWITCHES],
 [AC_GECODE_CHECK_COMPILERFLAG([-O3])
  AC_GECODE_CHECK_COMPILERFLAG([-fno-strict-aliasing])])

AC_DEFUN([AC_GECODE_GCC_VISIBILITY],
  	[AC_ARG_ENABLE([gcc-visibility],
	   AC_HELP_STRING([--enable-gcc-visibility],
	     [use gcc visibility attributes @<:@default=yes@:>@]))
	 AC_MSG_CHECKING(whether to use gcc visibility attributes)
	 if test "${enable_gcc_visibility:-yes}" = "yes"; then
	    AC_MSG_RESULT(yes)
	    AC_GECODE_CHECK_CXXFLAG([-fvisibility=hidden],
	      [AC_DEFINE(GECODE_GCC_HAS_CLASS_VISIBILITY)
	       AC_GECODE_ADD_TO_COMPILERFLAGS([-fvisibility=hidden])],
	       [])
	 else
	    AC_MSG_RESULT(no)
	 fi])

AC_DEFUN([AC_GECODE_GCC_DEBUG_SWITCHES],
  [AC_GECODE_CHECK_COMPILERFLAG([-fno-inline-functions])
   AC_GECODE_CHECK_COMPILERFLAG([-fimplement-inlines])])

AC_DEFUN([AC_GECODE_NO_BUILDFLAGS],
  [AC_SUBST(GECODE_BUILD_SUPPORT_FLAG, "")
   AC_SUBST(GECODE_BUILD_KERNEL_FLAG, "")
   AC_SUBST(GECODE_BUILD_SEARCH_FLAG, "")
   AC_SUBST(GECODE_BUILD_INT_FLAG, "")
   AC_SUBST(GECODE_BUILD_SET_FLAG, "")
   AC_SUBST(GECODE_BUILD_CPLTSET_FLAG, "")
   AC_SUBST(GECODE_BUILD_MINIMODEL_FLAG, "")
   AC_SUBST(GECODE_BUILD_SERIALIZATION_FLAG, "")
   AC_SUBST(GECODE_BUILD_GIST_FLAG, "")])

AC_DEFUN([AC_GECODE_BUILDFLAGS],
  [AC_SUBST(GECODE_BUILD_SUPPORT_FLAG, "-DGECODE_BUILD_SUPPORT")
   AC_SUBST(GECODE_BUILD_KERNEL_FLAG, "-DGECODE_BUILD_KERNEL")
   AC_SUBST(GECODE_BUILD_SEARCH_FLAG, "-DGECODE_BUILD_SEARCH")
   AC_SUBST(GECODE_BUILD_INT_FLAG, "-DGECODE_BUILD_INT")
   AC_SUBST(GECODE_BUILD_SET_FLAG, "-DGECODE_BUILD_SET")
   AC_SUBST(GECODE_BUILD_CPLTSET_FLAG, "-DGECODE_BUILD_CPLTSET")
   AC_SUBST(GECODE_BUILD_MINIMODEL_FLAG, "-DGECODE_BUILD_MINIMODEL")
   AC_SUBST(GECODE_BUILD_SERIALIZATION_FLAG, "-DGECODE_BUILD_SERIALIZATION")
   AC_SUBST(GECODE_BUILD_GIST_FLAG, "-DGECODE_BUILD_GIST")])


AC_DEFUN([AC_GECODE_UNIX_PATHS],
 [AC_SUBST(DLLPATH, [-L.])
  dnl flags for creating dlls
  case $host_os in
     darwin*)
       AC_SUBST(need_soname, "yes")
       AC_GECODE_ADD_TO_DLLFLAGS([-Wl,-single_module])
       AC_GECODE_ADD_TO_DLLFLAGS("-dynamiclib")
       AC_SUBST(DLLEXT, "${ac_gecode_soversion}.0.dylib")
       AC_SUBST(SOSUFFIX, ".${ac_gecode_soversion}.dylib")
       AC_SUBST(SOLINKSUFFIX, ".dylib")
       AC_SUBST(sharedlibdir, "${libdir}")
       AC_SUBST(WLSONAME, "-compatibility_version ${ac_gecode_soversion}.0 -current_version ${ac_gecode_soversion}.0 -install_name ${libdir}/")
       AC_GECODE_NO_BUILDFLAGS
       ;;
     windows*)
       AC_SUBST(need_soname, "no")
       if test "${enable_static:-no}"  = "yes" -a \
	       "${enable_shared:-yes}" = "yes"; then
         AC_MSG_ERROR([Only either static or shared libraries can be built.])
       fi
       AC_GECODE_ADD_TO_DLLFLAGS("-shared")
       AC_SUBST(DLLEXT, "dll")
       AC_SUBST(SOSUFFIX, "")
       AC_SUBST(SOLINKSUFFIX, "")
       AC_SUBST(WLSONAME, "")
       if test "${enable_static:-no}" = "no"; then
	  AC_SUBST(sharedlibdir, "${bindir}")
       else
          AC_SUBST(sharedlibdir, "${libdir}")
       fi
       AC_GECODE_BUILDFLAGS
       ;;
     *)
       AC_SUBST(need_soname, "yes")
       AC_GECODE_ADD_TO_DLLFLAGS("-shared")
       AC_SUBST(DLLEXT, "so.${ac_gecode_soversion}.0")
       AC_SUBST(SOSUFFIX, ".so.${ac_gecode_soversion}")
       AC_SUBST(SOLINKSUFFIX, ".so")
       AC_SUBST(WLSONAME, ["-Wl,-soname="])
       AC_SUBST(sharedlibdir, "${libdir}")
       AC_GECODE_NO_BUILDFLAGS
       ;;
  esac])

AC_DEFUN([AC_GECODE_MSVC_SWITCHES],
 [dnl general compiler flags
  AC_DEFINE(forceinline,[__forceinline])
  AC_GECODE_ADD_TO_COMPILERFLAGS([-nologo])
  AC_GECODE_ADD_TO_CFLAGS([-D_CRT_SECURE_NO_DEPRECATE])
  AC_GECODE_ADD_TO_CXXFLAGS([-EHsc])
  AC_DEFINE(GECODE_MEMORY_ALIGNMENT, 4)

  if test "${enable_debug:-no}" = "no"; then
    dnl compiler flags for an optimized build
    AC_GECODE_ADD_TO_COMPILERFLAGS([-Ox -fp:fast -GS- -arch:SSE -wd4355])
    dnl AC_GECODE_ADD_TO_COMPILERFLAGS([-Ox -fp:fast -GS- -wd4355 -w34640 -w34365 -w34800 -w34265])

    dnl flags for creating optimized dlls
    AC_GECODE_ADD_TO_DLLFLAGS([${CXXFLAGS} -LD -MD])
    dnl linker flags
    GLDFLAGS="-link -DEBUG -OPT:REF -OPT:ICF"
  else
    dnl compiler flags for a debug build
    AC_GECODE_ADD_TO_COMPILERFLAGS([-Zi -wd4355])  

    dnl flags for creating debug dlls
    AC_GECODE_ADD_TO_DLLFLAGS([${CXXFLAGS} -LDd -MDd])
  fi

  AC_SUBST(sharedlibdir, "${bindir}")
  AC_SUBST(docdir, "${prefix}")
  if test "${enable_static:-no}" = "yes"; then
    AC_MSG_ERROR([Static linking not supported for Windows/cl.])
  fi

  AC_SUBST(DLLPATH, "")

  dnl file extensions
  AC_SUBST(SBJEXT, "sbj")
  AC_SUBST(DLLEXT, "dll")
  AC_SUBST(SOSUFFIX, "")
  AC_SUBST(SOLINKSUFFIX, "")
  AC_SUBST(WLSONAME, "")
  AC_SUBST(LIBEXT, "lib")
  AC_SUBST(LIBPREFIX, "Gecode")
  AC_SUBST(LINKPREFIX, "Gecode")
  AC_SUBST(LINKSUFFIX, ".lib")
  AC_SUBST(MINUSLDIR, "")
  AC_SUBST(LINKLIBDIR, "${libdir}/")

  cygpathprefix=$prefix
  test "x$cygpathprefix" = xNONE && cygpathprefix=$ac_default_prefix
  cygpathprefix=`cygpath -m ${cygpathprefix}`

  AC_SUBST(cygpathprefix, "${cygpathprefix}")
  AC_GECODE_BUILDFLAGS

  dnl compiler options
  AC_SUBST(COMPILEOBJ, "-c -Fo")
  AC_SUBST(COMPILESBJ, "-c -Fa")
  AC_SUBST(CXXIN, "-Tp")
  AC_SUBST(CCIN, "-Tc")
  AC_SUBST(EXAMPLES_EXTRA_CXXFLAGS, "")

  dnl Install stub .lib files (required for msvc)
  AC_SUBST(INSTALLLIBS, "yes")

  dnl how to tell the compiler to output an object file
  AC_SUBST(LINKOUTPUT, "-Fe")
  dnl how to tell the compiler to output an executable
  AC_SUBST(EXEOUTPUT, "-Fe")

  dnl the names of the generated dlls
  AC_SUBST(SUPPORT, "Support")
  AC_SUBST(KERNEL,  "Kernel")
  AC_SUBST(SEARCH,  "Search")
  AC_SUBST(INT,     "Int")
  AC_SUBST(SET,     "Set")
  AC_SUBST(CPLTSET, "CpltSet")
  AC_SUBST(MM,      "Minimodel")
  AC_SUBST(SER,     "Serialization")
  AC_SUBST(GIST,    "Gist")])

dnl Macro:
dnl   AC_GECODE_DOC_SWITCHES
dnl
dnl Description:
dnl   Produces the configure switches --enable-doc-search,
dnl   --enable-doc-chm, and --enable-doc-tagfile.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_DOC_SWITCHES],
  [dnl check if we can use dot for generating graphs in the documentation
   AC_ARG_ENABLE([doc-dot],
     AC_HELP_STRING([--enable-doc-dot],
	[enable graphs in documentation @<:@default=yes@:>@]))
   AC_CHECK_PROG(DOT, dot, dot)
   AC_MSG_CHECKING(whether to enable graphs in the documentation)
   if test "${enable_doc_dot:-yes}" = "yes"; then
     if test x$DOT = x; then
        if test x"${enable_doc_dot}" = x; then
	  AC_MSG_RESULT(no)
     	  AC_SUBST(GECODE_DOXYGEN_DOT, NO)
	else
          AC_MSG_ERROR(you need the dot tool from graphviz to generate graphs in the documentation)
	fi
     else
        AC_MSG_RESULT(yes)
	AC_SUBST(GECODE_DOXYGEN_DOT, YES)
     fi
   else
     AC_MSG_RESULT(no)
     AC_SUBST(GECODE_DOXYGEN_DOT, NO)
   fi

   AC_ARG_ENABLE([doc-search],
     AC_HELP_STRING([--enable-doc-search],
	[enable documentation search engine @<:@default=no@:>@]))
   AC_MSG_CHECKING(whether to enable the documentation search engine)
   if test "${enable_doc_search:-no}" = "yes"; then
     AC_MSG_RESULT(yes)
     AC_SUBST(ENABLEDOCSEARCH, "yes")
   else
     AC_MSG_RESULT(no)
     AC_SUBST(ENABLEDOCSEARCH, "no")
   fi
   AC_ARG_ENABLE([doc-tagfile],
     AC_HELP_STRING([--enable-doc-tagfile],
	[generate doxygen tagfile @<:@default=no@:>@]))
   AC_MSG_CHECKING(whether to generate a doxygen tagfile)
   if test "${enable_doc_tagfile:-no}" = "yes"; then
     AC_MSG_RESULT(yes)
     AC_SUBST(GECODE_DOXYGEN_TAGFILE, "doc/gecode-doc.tag")
   else
     AC_MSG_RESULT(no)
     AC_SUBST(GECODE_DOXYGEN_TAGFILE, [])
   fi
   AC_ARG_ENABLE([doc-chm],
     AC_HELP_STRING([--enable-doc-chm],
	[build compressed html documentation @<:@default=yes on Windows@:>@]))
   AC_MSG_CHECKING(whether to build compressed html documentation)
   case $host_os in
	windows*)
   		if test "${enable_doc_chm:-yes}" = "yes"; then
		  AC_MSG_RESULT(yes)
     		  AC_SUBST(ENABLEDOCCHM, "yes")
                  AC_SUBST(ENABLEDOCSEARCH, "no")
   		else
     		  AC_MSG_RESULT(no)
     		  AC_SUBST(ENABLEDOCCHM, "no")
   		fi
	;;
	*)
   		if test "${enable_doc_chm:-no}" = "yes"; then
		  AC_MSG_ERROR([building chms is only supported on Windows.])
   		else
     		  AC_MSG_RESULT(no)
     		  AC_SUBST(ENABLEDOCCHM, "no")
   		fi
	;;
   esac])

dnl Macro:
dnl   AC_GECODE_UNIVERSAL
dnl
dnl Description:
dnl   Produces the configure switches --enable-universal and --with-sdk
dnl   for compiling universal binaries on Mac OS X.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_UNIVERSAL],
  [dnl build universal binaries on Mac OS X
  AC_ARG_WITH([sdk],
    AC_HELP_STRING([--with-sdk],
	[SDK to use on Mac OS X]))
  if test "${host_os}" = "darwin"; then
    if test "${with_sdk:-no}" != "no"; then
      AC_GECODE_CHECK_COMPILERFLAG([-isysroot ${with_sdk}])
      AC_GECODE_ADD_TO_DLLFLAGS([-Wl,-syslibroot,${with_sdk}])
    fi
  fi
  AC_ARG_WITH([macosx-version-min],
    AC_HELP_STRING([--with-macosx-version-min],
	[minimum version of Mac OS X to support]))
  if test "${host_os}" = "darwin"; then
    if test "${with_macosx_version_min:-no}" != "no"; then
      AC_GECODE_CHECK_COMPILERFLAG(
        [-mmacosx-version-min=${with_macosx_version_min}])
      AC_GECODE_ADD_TO_DLLFLAGS(
        [-mmacosx-version-min=${with_macosx_version_min}])
    fi
  fi
  AC_ARG_WITH([architectures],
    AC_HELP_STRING([--with-archiectures],
	[architectures to compile for on Mac OS X]))
  AC_MSG_CHECKING(Whether to build for different architectures)
  if test "${host_os}" = "darwin"; then
    if test "${with_architectures:-no}" != "no"; then
      archflags="";
      for a in `echo ${with_architectures} | sed -e "s/,/ /g"`; do
	archflags="${archflags} -arch $a";
      done
      AC_GECODE_CHECK_COMPILERFLAG([${archflags}])
      AC_GECODE_ADD_TO_DLLFLAGS([${archflags}])
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  else
    AC_MSG_RESULT(no)
  fi
])

dnl Macro:
dnl   AC_GECODE_FRAMEWORK
dnl
dnl Description:
dnl   Produces the configure switch --enable-framework
dnl   for creating a Mac OS X framework instead of individual libraries.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_FRAMEWORK],
  [dnl build framework bundle on Mac OS X
  AC_ARG_ENABLE([framework],
    AC_HELP_STRING([--enable-framework],
	[build framework bundle on Mac OS X @<:@default=no@:>@]))
  if test "${host_os}" = "darwin"; then
    AC_MSG_CHECKING(whether to build framework bundle on Mac OS X)
    if test "${enable_framework:-no}" = "yes"; then
      AC_MSG_RESULT(yes)
      AC_SUBST(BUILD_MACOS_FRAMEWORK, "yes")
      AC_DEFINE(GECODE_STATIC_LIBS)
      AC_SUBST(BUILDSTATIC, "yes")
    else
      AC_MSG_RESULT(no)
      AC_SUBST(BUILD_MACOS_FRAMEWORK, "no")
    fi
  fi
])

dnl Macro:
dnl   AC_GECODE_BOOST_SERIALIZATION
dnl
dnl Description:
dnl   Produces the configure switch --with-boost
dnl   for supplying the path to the boost serialization library.
dnl   If present, support for boost serialization will be built
dnl   into Gecode.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_BOOST_SERIALIZATION],
  [dnl build with support for the boost serialization library
  AC_ARG_WITH([boost-include],
    AC_HELP_STRING([--with-boost-include],
	[path to the boost header files (used for serialization)]))
  if test "${with_boost_include:-no}" != "no"; then
      AC_SUBST(BOOST_CPPFLAGS,[-I${with_boost_include}])
      AC_SUBST(BOOST_LINK,["-L${with_boost_include}/../../lib -lboost_serialization"])
      AC_DEFINE(GECODE_HAS_BOOST_SERIALIZATION)
  fi
])

dnl Macro:
dnl   AC_GECODE_QT
dnl
dnl Description:
dnl   Produces the configure switch --enable-qt
dnl   for compiling parts of Gecode that need the Qt library.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_QT],
  [
  AC_ARG_ENABLE([qt],
    AC_HELP_STRING([--enable-qt],
      [build with Qt support @<:@default=no@:>@]))
  AC_MSG_CHECKING(whether to build with Qt support)
  if test "${enable_qt:-no}" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(GECODE_HAS_QT)

    AC_CHECK_PROG(QMAKE, qmake, [found])
    if test "${QMAKE}x" = "x"; then
      AC_MSG_ERROR([The qmake tool was not found.])
    else
      ac_gecode_qt_version=`qmake -query QT_VERSION`
      ac_gecode_qt_major=`echo ${ac_gecode_qt_version} | grep -o '^[[0-9]]*'`
      ac_gecode_qt_minor=`echo ${ac_gecode_qt_version} | sed -e 's/^[[0-9]]*\\.//g' -e 's/\\.[[0-9]]*$//g'`
      ac_gecode_qt_ok="yes"
      if test ${ac_gecode_qt_major} -lt 4; then ac_gecode_qt_ok="no";
      else if test ${ac_gecode_qt_major} -eq 4 \
                -a ${ac_gecode_qt_minor} -lt 2; then ac_gecode_qt_ok="no";
           fi
      fi
      if test "${ac_gecode_qt_ok}" != "yes"; then
        AC_MSG_ERROR([Your version of Qt is too old. Minimum required: 4.2. Your version: ${ac_gecode_qt_version}.])
      fi
    fi

    dnl use qmake to find the Qt installation
    ac_gecode_qt_tmpdir=`mktemp -d gistqt.XXXXXX` || exit 1
    cd ${ac_gecode_qt_tmpdir}
    echo "QT += script" > a.pro
    qmake
    if test -d a.xcodeproj; then
      ac_gecode_qt_makefile=a.xcodeproj/qt_preprocess.mak
    elif test -f Makefile.Debug; then
        if test "${enable_debug:-no}" = "no"; then
          ac_gecode_qt_makefile=Makefile.Release
        else
          ac_gecode_qt_makefile=Makefile.Debug
        fi
    else
      ac_gecode_qt_makefile=Makefile
    fi
    ac_gecode_qt_defines=`grep ${ac_gecode_qt_makefile} -e 'DEFINES.*=' | sed -e 's/.*=//' -e 's|\\\\|/|g' -e 's|-I\\("*\\)\\.\\./\\.\\.|-I\\1..|g'`
    ac_gecode_qt_inc=`grep ${ac_gecode_qt_makefile} -e 'INCPATH.*=' | sed -e 's/.*=//' -e 's|\\\\|/|g' -e 's|-I\\("*\\)\\.\\./\\.\\.|-I\\1..|g'`
    ac_gecode_qt_libs=`grep ${ac_gecode_qt_makefile} -e 'LIBS.*=' | sed -e 's/.*=//' -e 's|\\\\|/|g' -e 's|-I\\("*\\)\\.\\./\\.\\.|-I\\1..|g'`
    if test -d a.xcodeproj; then
      ac_gecode_qt_libs="-framework QtScript -framework QtGui -framework QtCore"
    fi
    cd ..
    rm -r ${ac_gecode_qt_tmpdir}
    AC_SUBST(QTINCLUDES, ${ac_gecode_qt_inc})
    AC_SUBST(QTDEFINES, ${ac_gecode_qt_defines})
    AC_SUBST(QTLIBS, ${ac_gecode_qt_libs})
  else
    AC_MSG_RESULT(no)
  fi
  AC_SUBST(enable_qt, ${enable_qt})
])

dnl Macro:
dnl   AC_GECODE_GIST
dnl
dnl Description:
dnl   Produces the configure switch --enable-gist
dnl   for compiling the Gecode Interactive Search Tool.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
AC_DEFUN([AC_GECODE_GIST],
  [
  AC_ARG_ENABLE([gist],
    AC_HELP_STRING([--enable-gist],
      [build Gecode Interactive Search Tool @<:@default=no@:>@]))
  AC_MSG_CHECKING(whether to build Gist)
  if test "${enable_gist:-no}" = "yes"; then
    enable_qt="yes";
    AC_MSG_RESULT(yes)
    AC_DEFINE(GECODE_HAS_GIST)
  else
    AC_MSG_RESULT(no)
  fi
  AC_SUBST(enable_gist, ${enable_gist})
])
