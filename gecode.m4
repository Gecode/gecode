dnl
dnl Main authors:
dnl   Guido Tack <tack@gecode.org>
dnl
dnl Contributing authors:
dnl   Samuel Gagnon <samuel.gagnon92@gmail.com>
dnl
dnl Copyright:
dnl   Guido Tack, 2004, 2005
dnl   Samuel Gagnon, 2018
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

AC_DEFUN([AC_GECODE_GET_OS],
    [AC_ARG_WITH([host-os],
           AC_HELP_STRING([--with-host-os],
       [Override operating system test. Valid values are Linux, Darwin, FreeBSD, NetBSD, and Windows.]))
     AC_MSG_CHECKING([for the host operating system])
         if test "${with_host_os:-no}" = "no"; then
       guess_host_os=$(uname -s 2>/dev/null)
     else
       guess_host_os=${with_host_os}
        fi
     dnl Try to find out operating system
     case ${guess_host_os} in
       GNU/kFreeBSD|*inux*|FreeBSD|NetBSD)
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
dnl
dnl Description:
dnl   Add the flags to the corresponding variables
dnl
dnl Author:
dnl   Marco Kuhlmann <kuhlmann@ps.uni-sb.de>
dnl
AC_DEFUN([AC_GECODE_ADD_TO_CXXFLAGS],
   [CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}"])
AC_DEFUN([AC_GECODE_ADD_TO_COMPILERFLAGS],
   [CFLAGS="$1${CFLAGS:+ }${CFLAGS}"
    CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}"])
AC_DEFUN([AC_GECODE_ADD_TO_CFLAGS],
   [CFLAGS="$1${CFLAGS:+ }${CFLAGS}"])
AC_DEFUN([AC_GECODE_ADD_TO_DLLFLAGS],
  [DLLFLAGS="$1${DLLFLAGS:+ }${DLLFLAGS}"])


dnl Macro:
dnl   AC_GECODE_CHECK_MS_CL_FLAG (FLAG, ACTION-IF-TRUE,
dnl                                ACTION-IF-FALSE)
dnl
dnl Description:
dnl   Check whether FLAG is supported by the Microsoft C++ compiler.  Run
dnl   the shell commands ACTION-IF-TRUE if it is, ACTION-IF-FALSE
dnl   otherwise.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
dnl
AC_DEFUN([AC_GECODE_CHECK_MS_CL_FLAG],
  [AC_MSG_CHECKING(whether ${CXX} accepts [$1])
   if ${CXX} $1 2>&1 | grep "ignoring unknown option" -q; then
     AC_MSG_RESULT([no])
     ifelse([$3], , :, [$3])
   else
     AC_MSG_RESULT([yes])
     ifelse([$2], , :, [$2])
   fi])

dnl Macro:
dnl   AC_GECODE_CHECK_GXX_FLAG (FLAG, ACTION-IF-TRUE,
dnl                                ACTION-IF-FALSE)
dnl
dnl Description:
dnl   Check whether FLAG is supported by the Gnu C++ compiler.  Run
dnl   the shell commands ACTION-IF-TRUE if it is, ACTION-IF-FALSE
dnl   otherwise.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
dnl
AC_DEFUN([AC_GECODE_CHECK_GXX_FLAG],
   [AC_MSG_CHECKING(whether ${CXX} accepts [$1])
   ac_gecode_save_CXXFLAGS="${CXXFLAGS}"
   CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS} -Werror"
   AC_LANG_PUSH(C++)
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
     [if grep -q "unrecognized\|argument unused" conftest.err; then
        CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
        AC_MSG_RESULT([no])
        ifelse([$3], , :, [$3])
      else
        CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
        AC_MSG_RESULT([yes])
        ifelse([$2], , :, [$2])
      fi
     ],
     [CXXFLAGS="${ac_gecode_save_CXXFLAGS}"
      AC_MSG_RESULT([no])
      ifelse([$3], , :, [$3])])
   AC_LANG_POP])

dnl Macro:
dnl   AC_GECODE_CHECK_GCC_FLAG (FLAG, ACTION-IF-TRUE,
dnl                                ACTION-IF-FALSE)
dnl
dnl Description:
dnl   Check whether FLAG is supported by the Gnu C compiler.  Run
dnl   the shell commands ACTION-IF-TRUE if it is, ACTION-IF-FALSE
dnl   otherwise.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
dnl
AC_DEFUN([AC_GECODE_CHECK_GCC_FLAG],[
   AC_MSG_CHECKING(whether ${CC} accepts [$1])
   ac_gecode_save_CFLAGS="${CFLAGS}"
   CFLAGS="$1${CFLAGS:+ }${CFLAGS} -Werror"
   AC_LANG_PUSH(C)
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
     [if grep -q "unrecognized\|argument unused" conftest.err; then
        CFLAGS="${ac_gecode_save_CFLAGS}"
        AC_MSG_RESULT([no])
        ifelse([$3], , :, [$3])
      else
        CFLAGS="${ac_gecode_save_CFLAGS}"
        AC_MSG_RESULT([yes])
        ifelse([$2], , :, [$2])
      fi
     ],
     [CFLAGS="${ac_gecode_save_CFLAGS}"
      AC_MSG_RESULT([no])
      ifelse([$3], , :, [$3])])
   AC_LANG_POP])

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
  [if test "${ac_cv_cxx_compiler_vendor}" = "microsoft"; then
     AC_GECODE_CHECK_MS_CL_FLAG($1,[
       ifelse([$2], , CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}", [$2])],
       [ifelse([$3], , :, [$3])])
   else
     AC_GECODE_CHECK_GXX_FLAG($1,[
       ifelse([$2], , CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}", [$2])],
       [ifelse([$3], , :, [$3])])
   fi
  ])dnl

AC_DEFUN([AC_GECODE_CHECK_COMPILERFLAG],
  [
   if test "${ac_cv_cxx_compiler_vendor}" = "microsoft"; then
     AC_GECODE_CHECK_MS_CL_FLAG($1,[
       CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}"
       CFLAGS="$1${CFLAGS:+ }${CFLAGS}"],[])
   else
     AC_GECODE_CHECK_GXX_FLAG($1,[
        CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS}"],[])
      AC_GECODE_CHECK_GCC_FLAG($1,[
        CFLAGS="$1${CFLAGS:+ }${CFLAGS}"],[])
   fi])dnl

dnl @synopsis _AC_C_IFDEF(MACRO-NAME, ACTION-IF-DEF, ACTION-IF-NOT-DEF)
dnl
dnl Check for the definition of macro MACRO-NAME using the current
dnl language's compiler.
dnl
dnl @category Misc
dnl @author Ludovic Courtï¿½s <ludo@chbouib.org>
dnl @version 2004-09-07
dnl @license AllPermissive
AC_DEFUN([_AC_C_IFDEF],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifndef $1
                      # error "Macro $1 is undefined!"
              /* For some compilers (eg. SGI's CC), #error is not
                 enough...  */
              please, do fail
              #endif]],[[]])],
             [$2], [$3])])

dnl @synopsis AC_CXX_COMPILER_VENDOR(VENDOR-NAME)
dnl
dnl Set VENDOR-NAME to the lower-case name of the compiler vendor or `unknown'
dnl if the compiler's vendor is unknown.
dnl
dnl @version 20040907
dnl @author  Ludovic Courtï¿½s <ludo@chbouib.org>
dnl Modified by G. Tack to recognize only those compilers we need.
AC_DEFUN([AC_ADD_DLL_ARCH],
  [ac_gecode_library_architecture="-$1${ac_gecode_library_architecture}"])

AC_DEFUN([AC_CXX_COMPILER_VENDOR],
  [AC_ARG_WITH([compiler-vendor],
     AC_HELP_STRING([--with-compiler-vendor],
       [Override compiler test. Valid values are gnu, intel, and microsoft.]))
   AC_REQUIRE([AC_PROG_CXX])
   AC_REQUIRE([AC_PROG_CXXCPP])
   if test "${with_compiler_vendor:-no}" = "no"; then
     AC_LANG_PUSH([C++])
     dnl GNU C++
     _AC_C_IFDEF([__GNUG__],
       [ac_cv_cxx_compiler_vendor=gnu],
       [_AC_C_IFDEF([__INTEL_COMPILER],
                    [ac_cv_cxx_compiler_vendor=intel
                     _AC_C_IFDEF([_M_IX86],
                       [AC_ADD_DLL_ARCH([x86])],[])
                     _AC_C_IFDEF([_M_IA64],
                       [AC_ADD_DLL_ARCH([ia64])],[])
                     _AC_C_IFDEF([_M_X64],
                       [AC_ADD_DLL_ARCH([x64])],[])],
       [dnl Note:  We are using the C compiler because VC++ doesn't
        dnl recognize `.cc'(which is used by `configure') as a C++ file
        dnl extension and requires `/TP' to be passed.
        AC_LANG_PUSH([C])
        _AC_C_IFDEF([_MSC_VER],
                    [ac_cv_cxx_compiler_vendor=microsoft
                     _AC_C_IFDEF([_M_IX86],
                       [AC_ADD_DLL_ARCH([x86])],[])
                     _AC_C_IFDEF([_M_IA64],
                       [AC_ADD_DLL_ARCH([ia64])],[])
                     _AC_C_IFDEF([_M_X64],
                       [AC_ADD_DLL_ARCH([x64])],[])],
                    [ac_cv_cxx_compiler_vendor=unknown])
        AC_LANG_POP()])])

     AC_LANG_POP()
     else
       ac_cv_cxx_compiler_vendor=${with_compiler_vendor}
     fi
     $1="$ac_cv_cxx_compiler_vendor"])dnl


AC_DEFUN([AC_CHECK_GCC_VERSION],
    [
    AC_LANG_PUSH(C)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
     #if __GNUC__ < $1 || (__GNUC__ == $1 && __GNUC_MINOR__ < $2)
     #error Version too old
     really, the version is too old
     #endif]],[[]])],[],
    [AC_MSG_ERROR([Your version of gcc is too old. You need at least version $1.$2.])]
    )
    AC_LANG_POP()
    AC_LANG_PUSH(C++)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
     #if __GNUC__ < $1 || (__GNUC__ == $1 && __GNUC_MINOR__ < $2)
     #error Version too old
     really, the version is too old
     #endif]],[[]])],[],
    [AC_MSG_ERROR([Your version of g++ is too old. You need at least version $1.$2.])]
    )
    AC_LANG_POP()
    ])

AC_DEFUN([AC_CHECK_MSVC_VERSION],
    [
    AC_LANG_PUSH(C)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
     #if _MSC_VER < $1
     #error Version too old
     really, the version is too old
     #endif]],[[]])],[],
    [AC_MSG_ERROR([Your version of cl is too old. You need at least Microsoft Visual C++ $2.])]
    AC_LANG_POP()
    )
    ])

dnl Macro:
dnl   AC_GECODE_VTI (vartype, help-string, default, vtidef,
dnl             [ACTION-IF-ENABLED], [ACTION-IF-DISABLED])
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
   [AC_DEFINE(GECODE_HAS_$1_VARS, [], [Whether to build $1 variables])])
AC_DEFUN([AC_GECODE_VIS],[
  AC_ARG_WITH([vis],
    AC_HELP_STRING([--with-vis],
    [additional variable implementation specifications]))
    if test "${with_vis:-no}" != "no"; then
      with_vis=${with_vis//,/ }
      ac_gecode_vis="${with_vis}${ac_gecode_vis:+ }${ac_gecode_vis}"
    fi])
AC_DEFUN([AC_GECODE_VTI],
   [
   AC_ARG_ENABLE([$1-vars],
     AC_HELP_STRING([--enable-$1-vars],[build $2 @<:@default=$3@:>@]))
   ac_gecode_vis="$4${ac_gecode_vis:+ }${ac_gecode_vis}"
   AC_MSG_CHECKING(whether to build the $1 variables library)
   if test "${enable_$1_vars:-$3}" = "yes"; then
     AC_MSG_RESULT(yes)
     enable_$1_vars="yes";
     $5
     AC_GECODE_ADD_VTI(translit($1,`a-z', `A-Z'))
   else
     enable_$1_vars="no";
     $6
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
dnl   Grégoire Dooms <dooms@info.ucl.ac.be>
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
          AC_DEFINE([GECODE_STATIC_LIBS],[],
                    [Whether we are compiling static libraries])
          AC_SUBST(BUILDSTATIC, "yes")
          AC_MSG_RESULT(yes)
          enable_shared=no
        else
          AC_SUBST(BUILDSTATIC, "no")
          enable_shared=yes
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

AC_DEFUN([AC_GECODE_CODESIZE],
    [AC_ARG_ENABLE([small-codesize],
       AC_HELP_STRING([--enable-small-codesize],
         [optimize for code size @<:@default=no@:>@]))
     AC_MSG_CHECKING(whether to optimize for code size)
     if test "${enable_debug:-no}" = "yes"; then
        AC_MSG_RESULT(not in debug builds)
     elif test "${enable_small_codesize:-no}" = "yes"; then
       AC_MSG_RESULT(yes)
       case $host_os in
         darwin*)
           ac_gecode_gcc_optimize_flag=-Oz
         ;;
         *)
           ac_gecode_gcc_optimize_flag=-Os
           ac_gecode_cl_optimize_flag=-O1
         ;;
       esac
     else
        AC_MSG_RESULT(no)
        ac_gecode_gcc_optimize_flag=-O3
        ac_gecode_cl_optimize_flag=-O2
     fi])


AC_DEFUN([AC_GECODE_DEBUG],
    [AC_ARG_ENABLE([debug],
       AC_HELP_STRING([--enable-debug],
         [build with assertions @<:@default=no@:>@]))
     AC_MSG_CHECKING(whether to build with debug symbols and assertions)
     if test "${enable_debug:-no}" = "yes"; then
            AC_ADD_DLL_ARCH([d])
            AC_SUBST(DEBUG_BUILD, [yes])
        AC_MSG_RESULT(yes)
     else
            AC_ADD_DLL_ARCH([r])
            AC_SUBST(DEBUG_BUILD, [no])
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
              [AC_DEFINE([GECODE_HAS_MTRACE],[],
                       [Whether we have mtrace for memory leak debugging])],
              [AC_MSG_ERROR(mtrace not available.)],
              [[#include <mcheck.h>]])        
     else
        AC_MSG_RESULT(no)
     fi])

AC_DEFUN([AC_GECODE_PEAKHEAP],
    [AC_ARG_ENABLE([peakheap],
       AC_HELP_STRING([--enable-peakheap],
         [build with peak heap size tracking @<:@default=no@:>@]))
     if test "${enable_peakheap:-no}" = "yes"; then
        AC_CHECK_FUNC(_msize,
        [
          AC_DEFINE([GECODE_PEAKHEAP],[],[Whether to track peak heap size])
          AC_DEFINE([GECODE_PEAKHEAP_MALLOC_H],[],[Whether we need malloc.h])
          AC_DEFINE([GECODE_MSIZE],[_msize],[How to check allocation size])
          AC_MSG_CHECKING(whether to build with peak heap size tracking)
          AC_MSG_RESULT(yes)
        ],
        [
          AC_CHECK_HEADERS([malloc/malloc.h], 
          [
            AC_CHECK_FUNC(malloc_size,
            [
              AC_DEFINE([GECODE_PEAKHEAP],[],[])
              AC_DEFINE([GECODE_PEAKHEAP_MALLOC_MALLOC_H],[],[Whether we need malloc/malloc.h])
              AC_DEFINE([GECODE_MSIZE],[malloc_size],[])
              AC_MSG_CHECKING(whether to build with peak heap size tracking)
              AC_MSG_RESULT(yes)
            ],
            [
              AC_MSG_CHECKING(whether to build with peak heap size tracking)
              AC_MSG_RESULT(no)
            ])
          ],
          [
            AC_CHECK_HEADERS([malloc.h], 
            [
              AC_CHECK_FUNC(malloc_usable_size,
              [
                AC_DEFINE([GECODE_PEAKHEAP],[],[])
                AC_DEFINE([GECODE_PEAKHEAP_MALLOC_H],[],[Whether we need malloc.h])
                AC_DEFINE([GECODE_MSIZE],[malloc_usable_size],[])
                AC_MSG_CHECKING(whether to build with peak heap size tracking)
                AC_MSG_RESULT(yes)
              ],
              [
                AC_MSG_CHECKING(whether to build with peak heap size tracking)
                AC_MSG_RESULT(no)
              ])
            ],
            [
              AC_MSG_CHECKING(whether to build with peak heap size tracking)
              AC_MSG_RESULT(no)
            ])
          ]
          )
        ])
     else
        AC_MSG_CHECKING(whether to build with peak heap size tracking)
        AC_MSG_RESULT(no)
     fi])

AC_DEFUN([AC_GECODE_ALLOCATOR],
    [AC_ARG_ENABLE([allocator],
       AC_HELP_STRING([--enable-allocator],
         [build with default memory allocator @<:@default=yes@:>@]))
     AC_MSG_CHECKING(whether to build with default memory allocator)
     if test "${enable_allocator:-yes}" = "yes"; then
        AC_DEFINE([GECODE_ALLOCATOR],[],
                  [Whether to build with default memory allocator])
        AC_MSG_RESULT(yes)
     else
        AC_MSG_RESULT(no)
     fi])


AC_DEFUN([AC_GECODE_AUDIT],
    [AC_ARG_ENABLE([audit],
       AC_HELP_STRING([--enable-audit],
         [build with auditing code @<:@default=no@:>@]))
     AC_MSG_CHECKING(whether to build with auditing code)
     if test "${enable_audit:-no}" = "yes"; then
        AC_DEFINE([GECODE_AUDIT],[],
                  [Whether to include audit code])
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
 [AC_GECODE_CHECK_COMPILERFLAG([-fPIC])
  AC_GECODE_CHECK_COMPILERFLAG([-Wextra])
  AC_GECODE_CHECK_COMPILERFLAG([-Wall])
  AC_GECODE_CHECK_COMPILERFLAG([-pipe])

  AC_ARG_ENABLE([cpp11],
       AC_HELP_STRING([--enable-cpp11],
         [compile for C++11 standard @<:@default=yes@:>@]))
  if test "${enable_cpp11:-yes}" = "yes"; then
    AC_GECODE_CHECK_COMPILERFLAG([-std=c++11])
  else
    AC_MSG_CHECKING(whether to compile for C++11 standard)
    AC_MSG_RESULT(no)
  fi

  AC_GECODE_CHECK_CXXFLAG(-ggdb,
     AC_GECODE_ADD_TO_COMPILERFLAGS(-ggdb),
     AC_GECODE_CHECK_COMPILERFLAG(-g))
  AC_CHECK_FUNC([__builtin_ffsll],
    [AC_DEFINE([GECODE_HAS_BUILTIN_FFSLL],[],
      [whether __builtin_ffsll is available])])
  AC_CHECK_FUNC([__builtin_popcountll],
    [AC_DEFINE([GECODE_HAS_BUILTIN_POPCOUNTLL],[],
      [whether __builtin_popcountll is available])])

  AC_SUBST(docdir, "${datadir}/doc/gecode")

  dnl do not use manifest tool with gcc
  AC_SUBST(MANIFEST, "@true")

  dnl file extensions
  AC_SUBST(SBJEXT, "s")
  AC_SUBST(LIBEXT, "${DLLEXT}")
  AC_SUBST(LIBPREFIX, "lib${ac_gecode_userprefix}gecode")
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

  AC_SUBST(LINKPREFIX, "-l${ac_gecode_userprefix}gecode")
  AC_SUBST(LINKSUFFIX, "")

  dnl how to tell the compiler to output an object file
  AC_SUBST(LINKOUTPUT, "-o ")
  dnl how to tell the compiler to output an executable
  AC_SUBST(EXEOUTPUT, "-o ")

  dnl the names of the generated dlls
  AC_SUBST(SUPPORT,    "support")
  AC_SUBST(KERNEL,     "kernel")
  AC_SUBST(SEARCH,     "search")
  AC_SUBST(INT,        "int")
  AC_SUBST(FLOAT,      "float")
  AC_SUBST(SET,        "set")
  AC_SUBST(MM,         "minimodel")
  AC_SUBST(GIST,       "gist")
  AC_SUBST(FLATZINC,   "flatzinc")
  AC_SUBST(DRIVER,     "driver")])


AC_DEFUN([AC_GECODE_GCC_OPTIMIZED_SWITCHES],
 [AC_MSG_CHECKING([if compiler supports forced inlining])
  AC_LANG_PUSH([C++])
  ac_gecode_save_CXXFLAGS="${CXXFLAGS}"
  CXXFLAGS="$1${CXXFLAGS:+ }${CXXFLAGS} -Werror"
  _AC_C_IFDEF([__clang__],
    [AC_MSG_RESULT(no)
     AC_DEFINE(forceinline, [inline],
       [How to tell the compiler to really, really inline])],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([
         [inline __attribute__ ((__always_inline__)) void foo(void) {}]],
         [])],
       [AC_MSG_RESULT(yes)
        AC_DEFINE(forceinline, [inline __attribute__ ((__always_inline__))],
          [How to tell the compiler to really, really inline])],
       [AC_MSG_RESULT(no)
        AC_DEFINE(forceinline, [inline],
          [How to tell the compiler to really, really inline])])])
  CXXFLAGS=${ac_gecode_save_CXXFLAGS}
  AC_LANG_POP([C++])
    AC_GECODE_CHECK_COMPILERFLAG([${ac_gecode_gcc_optimize_flag}])
  AC_GECODE_CHECK_COMPILERFLAG([-fno-strict-aliasing])])

AC_DEFUN([AC_GECODE_GCC_VISIBILITY],
      [AC_ARG_ENABLE([gcc-visibility],
       AC_HELP_STRING([--enable-gcc-visibility],
         [use gcc visibility attributes @<:@default=yes@:>@]))
     AC_MSG_CHECKING(whether to use gcc visibility attributes)
     if test "${enable_gcc_visibility:-yes}" = "yes"; then
        AC_MSG_RESULT(yes)
        AC_GECODE_CHECK_CXXFLAG([-fvisibility=hidden],
          [AC_DEFINE([GECODE_GCC_HAS_CLASS_VISIBILITY],[],
            [Whether gcc understands visibility attributes])
           AC_GECODE_ADD_TO_COMPILERFLAGS([-fvisibility=hidden])],
           [])
     else
        AC_MSG_RESULT(no)
     fi])

AC_DEFUN([AC_GECODE_GCC_DEBUG_SWITCHES],
  [AC_DEFINE(forceinline, [inline],
       [How to tell the compiler to really, really inline])
   AC_GECODE_CHECK_COMPILERFLAG([-fno-inline-functions])
   AC_GECODE_CHECK_COMPILERFLAG([-fimplement-inlines])])

AC_DEFUN([AC_GECODE_NO_BUILDFLAGS],
  [AC_SUBST(GECODE_BUILD_SUPPORT_FLAG, "")
   AC_SUBST(GECODE_BUILD_KERNEL_FLAG, "")
   AC_SUBST(GECODE_BUILD_SEARCH_FLAG, "")
   AC_SUBST(GECODE_BUILD_INT_FLAG, "")
   AC_SUBST(GECODE_BUILD_FLOAT_FLAG, "")
   AC_SUBST(GECODE_BUILD_SET_FLAG, "")
   AC_SUBST(GECODE_BUILD_MINIMODEL_FLAG, "")
   AC_SUBST(GECODE_BUILD_GIST_FLAG, "")
   AC_SUBST(GECODE_BUILD_FLATZINC_FLAG, "")
   AC_SUBST(GECODE_BUILD_DRIVER_FLAG, "")])

AC_DEFUN([AC_GECODE_BUILDFLAGS],
  [AC_SUBST(GECODE_BUILD_SUPPORT_FLAG, "-DGECODE_BUILD_SUPPORT")
   AC_SUBST(GECODE_BUILD_KERNEL_FLAG, "-DGECODE_BUILD_KERNEL")
   AC_SUBST(GECODE_BUILD_SEARCH_FLAG, "-DGECODE_BUILD_SEARCH")
   AC_SUBST(GECODE_BUILD_INT_FLAG, "-DGECODE_BUILD_INT")
   AC_SUBST(GECODE_BUILD_FLOAT_FLAG, "-DGECODE_BUILD_FLOAT")
   AC_SUBST(GECODE_BUILD_SET_FLAG, "-DGECODE_BUILD_SET")
   AC_SUBST(GECODE_BUILD_MINIMODEL_FLAG, "-DGECODE_BUILD_MINIMODEL")
   AC_SUBST(GECODE_BUILD_GIST_FLAG, "-DGECODE_BUILD_GIST")
   AC_SUBST(GECODE_BUILD_FLATZINC_FLAG, "-DGECODE_BUILD_FLATZINC")
   AC_SUBST(GECODE_BUILD_DRIVER_FLAG, "-DGECODE_BUILD_DRIVER")])


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
  AC_GECODE_ADD_TO_COMPILERFLAGS([-nologo -bigobj])
  AC_GECODE_ADD_TO_CFLAGS([-D_CRT_SECURE_NO_DEPRECATE])
  AC_GECODE_ADD_TO_CXXFLAGS([-EHsc])
  AC_DEFINE([GECODE_MEMORY_ALIGNMENT], [sizeof(void*)],
            [Heap memory alignment])
  if test "${enable_debug:-no}" = "no"; then
    dnl compiler flags for an optimized build
    AC_GECODE_ADD_TO_COMPILERFLAGS([${ac_gecode_cl_optimize_flag}])
    AC_GECODE_ADD_TO_COMPILERFLAGS([-MD -GS- -wd4355])
    AC_GECODE_CHECK_COMPILERFLAG([-arch:SSE2])

    dnl flags for creating optimized dlls
    AC_GECODE_ADD_TO_DLLFLAGS([${CXXFLAGS} -LD])
    dnl linker flags
    GLDFLAGS="-link -DEBUG -OPT:REF -OPT:ICF -MANIFEST -INCREMENTAL:NO"
  else
    dnl compiler flags for a debug build
    AC_GECODE_ADD_TO_COMPILERFLAGS([-MDd -Zi -wd4355])  

    dnl flags for creating debug dlls
    AC_GECODE_ADD_TO_DLLFLAGS([${CXXFLAGS} -LDd])
    GLDFLAGS="-link -MANIFEST -INCREMENTAL:NO"
  fi

  AC_SUBST(sharedlibdir, "${bindir}")
  AC_SUBST(docdir, "${prefix}")
  if test "${enable_static:-no}" = "yes"; then
    AC_MSG_ERROR([Static linking not supported for Windows/cl.])
  fi

  AC_CHECK_PROG(MANIFEST, mt.exe, mt.exe)

  AC_SUBST(DLLPATH, "")

  dnl file extensions
  AC_SUBST(SBJEXT, "sbj")
  AC_SUBST(DLLEXT, "dll")
  AC_SUBST(SOSUFFIX, "")
  AC_SUBST(SOLINKSUFFIX, "")
  AC_SUBST(WLSONAME, "")
  AC_SUBST(LIBEXT, "lib")
  AC_SUBST(LIBPREFIX, "${ac_gecode_userprefix}Gecode")
  AC_SUBST(LINKPREFIX, "${ac_gecode_userprefix}Gecode")
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

  dnl Install stub .lib files (required for msvc)
  AC_SUBST(INSTALLLIBS, "yes")

  dnl how to tell the compiler to output an object file
  AC_SUBST(LINKOUTPUT, "-Fe")
  dnl how to tell the compiler to output an executable
  AC_SUBST(EXEOUTPUT, "-Fe")

  dnl the names of the generated dlls
  AC_SUBST(SUPPORT,    "Support")
  AC_SUBST(KERNEL,     "Kernel")
  AC_SUBST(SEARCH,     "Search")
  AC_SUBST(INT,        "Int")
  AC_SUBST(FLOAT,      "Float")
  AC_SUBST(SET,        "Set")
  AC_SUBST(MM,         "Minimodel")
  AC_SUBST(GIST,       "Gist")
  AC_SUBST(FLATZINC,   "FlatZinc")
  AC_SUBST(DRIVER,     "Driver")])

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
    [generate doxygen tagfile @<:@default=yes@:>@]))
   AC_MSG_CHECKING(whether to generate a doxygen tagfile)
   if test "${enable_doc_tagfile:-yes}" = "yes"; then
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
   esac
   AC_ARG_ENABLE([doc-docset],
     AC_HELP_STRING([--enable-doc-docset],
    [build docset documentation for XCode @<:@default=no@:>@]))
   AC_MSG_CHECKING(whether to build docset documentation for XCode)
   case $host_os in
    darwin*)
           if test "${enable_doc_docset:-no}" = "yes"; then
          AC_MSG_RESULT(yes)
               AC_SUBST(ENABLEDOCDOCSET, "yes")
                  AC_SUBST(ENABLEDOCSEARCH, "no")
           else
               AC_MSG_RESULT(no)
               AC_SUBST(ENABLEDOCDOCSET, "no")
           fi
    ;;
    *)
           if test "${enable_doc_docset:-no}" = "yes"; then
          AC_MSG_ERROR([building docsets is only supported on Mac OS X.])
           else
               AC_MSG_RESULT(no)
               AC_SUBST(ENABLEDOCDOCSET, "no")
           fi
    ;;
   esac
   ])

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
    AC_HELP_STRING([--with-architectures],
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
      enable_static="yes";
      enable_shared="no";
    else
      AC_MSG_RESULT(no)
      AC_SUBST(BUILD_MACOS_FRAMEWORK, "no")
    fi
  fi
])

dnl Macro:
dnl   AC_GECODE_MPFR_INCLUDE
dnl
dnl Description:
dnl   Produces the configure switch --with-mpfr-include
dnl   for supplying the path to the mpfr header.
dnl
AC_DEFUN([AC_GECODE_MPFR_INCLUDE],
  [dnl build with support for the mpfr header
  AC_ARG_WITH([mpfr-include],
    AC_HELP_STRING([--with-mpfr-include],
    [path to the mpfr header file]))
  if test "${with_mpfr_include:-no}" != "no"; then
      AC_SUBST(MPFR_CPPFLAGS,[-I${with_mpfr_include}])
  fi
])

dnl Macro:
dnl   AC_GECODE_MPFR_LIB
dnl
dnl Description:
dnl   Produces the configure switch --with-mpfr-lib
dnl   for supplying the path to the mpfr library.
dnl
AC_DEFUN([AC_GECODE_MPFR_LIB],
  [dnl build with support for the mpfr library
  AC_ARG_WITH([mpfr-lib],
    AC_HELP_STRING([--with-mpfr-lib],
    [path to the mpfr library]))
  if test "${with_mpfr_lib:-no}" != "no"; then
      case $ac_gecode_compiler_vendor in
        gnu)
          AC_SUBST(MPFR_LIB_PATH,["-L${with_mpfr_lib}"])
        ;;
        microsoft)
          AC_SUBST(MPFR_LIB_PATH,["/LIBPATH:${with_mpfr_lib}"])
        ;;
       esac
  else
       AC_SUBST(MPFR_LIB_PATH,[""])
  fi
  case $ac_gecode_compiler_vendor in
    gnu)
     AC_SUBST(MPFR_LINK,["-lmpfr"])
    ;;
    microsoft)
      AC_SUBST(MPFR_LINK,["mpfr.lib"])
    ;;
  esac
])

dnl Macro:
dnl   AC_GECODE_GMP_INCLUDE
dnl
dnl Description:
dnl   Produces the configure switch --with-gmp-include
dnl   for supplying the path to the gmp or mpir headers.
dnl
AC_DEFUN([AC_GECODE_GMP_INCLUDE],
  [dnl build with support for the gmp headers
  AC_ARG_WITH([gmp-include],
    AC_HELP_STRING([--with-gmp-include],
    [path to the gmp or mpir header files]))
  if test "${with_gmp_include:-no}" != "no"; then
      AC_SUBST(GMP_CPPFLAGS,[-I${with_gmp_include}])
  fi
])

dnl Macro:
dnl   AC_GECODE_GMP_LIB
dnl
dnl Description:
dnl   Produces the configure switch --with-gmp-lib
dnl   for supplying the path to the GMP library.
dnl
AC_DEFUN([AC_GECODE_GMP_LIB],
  [dnl build with support for the GMP library
  AC_ARG_WITH([gmp-lib],
    AC_HELP_STRING([--with-gmp-lib],
    [path to the gmp or mpir library]))

  ac_gecode_tmp_gmp_lib=""
  if test "${with_gmp_lib:-no}" != "no"; then
      case $ac_gecode_compiler_vendor in
        gnu)
          AC_SUBST(GMP_LIB_PATH,["-L${with_gmp_lib}"])
        ;;
        microsoft)
          AC_SUBST(GMP_LIB_PATH,["/LIBPATH:${with_gmp_lib}"])
        ;;
       esac
  else
       AC_SUBST(GMP_LIB_PATH,[""])
  fi

  ac_gecode_save_CPPFLAGS="${CPPFLAGS}"
  ac_gecode_save_LIBS="${LIBS}"
  case $ac_gecode_compiler_vendor in
    gnu)
      CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ } ${GMP_CPPFLAGS}"
      LIBS="${LIBS}${LIBS:+ } ${GMP_LIB_PATH}"
      AC_CHECK_LIB(gmp, __gmpz_init,[
        AC_SUBST(GMP_LINK,"${ac_gecode_tmp_gmp_lib} -lgmp")
      ],[
        AC_CHECK_LIB(mpir, __gmpz_init,[
          AC_SUBST(GMP_LINK,"${ac_gecode_tmp_gmp_lib} -lmpir")
          ],[
          enable_mpfr=no;
          ])
      ])
    ;;
    microsoft)
      CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ } ${GMP_CPPFLAGS}"
      LIBS="${LIBS}${LIBS:+ } /link ${GMP_LIB_PATH} gmp.lib"
      AC_CHECK_LIB(gmp, __gmpz_init,[
        AC_SUBST(GMP_LINK,"${ac_gecode_tmp_gmp_lib} gmp.lib")
      ],[
        LIBS="${ac_gecode_save_LIBS}"
        LIBS="${LIBS}${LIBS:+ } /link ${GMP_LIB_PATH} mpir.lib"
        AC_CHECK_LIB(mpir, __gmpz_init,[
          AC_SUBST(GMP_LINK,"${ac_gecode_tmp_gmp_lib} mpir.lib")
          ],[
          enable_mpfr=no;
          ])
      ])
    ;;
  esac
  CPPFLAGS="${ac_gecode_save_CPPFLAGS}"
  LIBS="${ac_gecode_save_LIBS}"
  ])


dnl Macro:
dnl   AC_GECODE_MPFR
dnl
dnl Description:
dnl   Produces the configure switch --enable-mpfr
dnl   for compiling parts of Gecode that need the MPFR library.
dnl
dnl Authors:
AC_DEFUN([AC_GECODE_MPFR],
  [
  AC_ARG_ENABLE([mpfr],
    AC_HELP_STRING([--enable-mpfr],
      [build with MPFR support @<:@default=yes@:>@]))
  if test "${enable_float_vars:-yes}" = "yes"; then
    AC_MSG_CHECKING(whether to build with MPFR support)
    if test "${enable_mpfr:-yes}" = "yes"; then
      AC_MSG_RESULT(yes)
      AC_GECODE_GMP_INCLUDE
      AC_GECODE_GMP_LIB
      AC_GECODE_MPFR_INCLUDE
      AC_GECODE_MPFR_LIB
      ac_gecode_save_CPPFLAGS="${CPPFLAGS}"
      ac_gecode_save_LIBS="${LIBS}"
      case $ac_gecode_compiler_vendor in
        gnu)
          CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ } ${MPFR_CPPFLAGS} ${GMP_CPPFLAGS}"
          LIBS="${LIBS}${LIBS:+ } ${MPFR_LIB_PATH} ${GMP_LIB_PATH} ${MPFR_LINK} ${GMP_LINK}"
          AC_CHECK_HEADERS([gmp.h], 
            AC_CHECK_HEADERS([mpfr.h], 
                             AC_CHECK_LIB(mpfr, mpfr_add,
                                          AC_DEFINE([GECODE_HAS_MPFR],[],[Whether MPFR is available])
                                          enable_mpfr=yes;,
                                          enable_mpfr=no;),
                             enable_mpfr=no; ),
                           enable_mpfr=no; )
        ;;
        microsoft)
          CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ } ${MPFR_CPPFLAGS} ${GMP_CPPFLAGS}"
          LIBS="${LIBS}${LIBS:+ } /link ${MPFR_LIB_PATH} ${GMP_LIB_PATH} ${MPFR_LINK} ${GMP_LINK}"
          AC_CHECK_HEADERS([gmp.h], 
            AC_CHECK_HEADERS([mpfr.h], 
                             AC_CHECK_FUNC(mpfr_add,
                                          AC_DEFINE([GECODE_HAS_MPFR],[],[Whether MPFR is available])
                                          enable_mpfr=yes;,
                                          enable_mpfr=no;),
                             enable_mpfr=no; ),
                           enable_mpfr=no; )
        ;;
      esac
      CPPFLAGS="${ac_gecode_save_CPPFLAGS}"
      LIBS="${ac_gecode_save_LIBS}"
    else
      AC_MSG_RESULT(no)
      enable_mpfr=no;
    fi
  else
    enable_mpfr=no;
  fi
  AC_SUBST(enable_mpfr, ${enable_mpfr})
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
      [build with Qt support, requires at least Qt 4.3 @<:@default=yes@:>@]))
  AC_CHECK_PROGS(QMAKE, [qmake-qt4 qmake])
  AC_CHECK_PROGS(MOC, [moc-qt4 moc])
  AC_MSG_CHECKING(whether to build with Qt support)
  if test "${enable_qt:-yes}" = "yes"; then
    if test "${QMAKE}x" = "x" -o "${MOC}x" = "x"; then
      AC_MSG_RESULT(no)
    else
      ac_gecode_qt_version=`${QMAKE} -query QT_VERSION`
      ac_gecode_qt_major=`echo ${ac_gecode_qt_version} | grep -o '^[[0-9]]*'`
      ac_gecode_qt_minor=`echo ${ac_gecode_qt_version} | sed -e 's/^[[0-9]]*\\.//g' -e 's/\\.[[0-9]]*$//g'`
      ac_gecode_qt_ok="yes"
      if test ${ac_gecode_qt_major} -lt 4; then ac_gecode_qt_ok="no";
      else if test ${ac_gecode_qt_major} -eq 4 \
                -a ${ac_gecode_qt_minor} -lt 3; then ac_gecode_qt_ok="no";
           fi
      fi
      if test "${ac_gecode_qt_ok}" != "yes"; then
        AC_MSG_RESULT(no)
      else
        AC_MSG_RESULT(yes)
        AC_DEFINE([GECODE_HAS_QT],[],[Whether Qt is available])
        dnl use qmake to find the Qt installation
        ac_gecode_qt_tmpdir=`mktemp -d gistqt.XXXXXX` || exit 1
        cd ${ac_gecode_qt_tmpdir}
        echo "CONFIG += release" > a.pro
        if test ${ac_gecode_qt_major} -eq 5; then
          echo "QT += widgets printsupport" >> a.pro
        fi
        ${QMAKE}
        if test -d a.xcodeproj; then
          ac_gecode_qt_makefile=a.xcodeproj/qt_preprocess.mak
        elif test -d a.pbproj; then
          ac_gecode_qt_makefile=a.pbproj/qt_preprocess.mak
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
        if test -d a.xcodeproj -o -d a.pbproj; then
          ac_gecode_qt_libs="-framework QtGui -framework QtCore"
        fi
        cd ..
        rm -r ${ac_gecode_qt_tmpdir}
        AC_SUBST(QTINCLUDES, ${ac_gecode_qt_inc})
        AC_SUBST(QTDEFINES, ${ac_gecode_qt_defines})
        AC_SUBST(QTLIBS, ${ac_gecode_qt_libs})
        AC_SUBST(enable_qt,yes)
        enable_qt=yes;
      fi
    fi
  else
    AC_MSG_RESULT(no)
    enable_qt=no;
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
      [build Gecode Interactive Search Tool @<:@default=yes@:>@]))
  AC_MSG_CHECKING(whether to build Gist)
  if test "${enable_gist:-yes}" = "yes" -a "${enable_qt}" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_SUBST(enable_gist, yes)
    AC_DEFINE([GECODE_HAS_GIST],[],[Whether Gist is available])
  else
    AC_MSG_RESULT(no)
  fi
])

dnl
dnl Macro:
dnl   AC_GECODE_CBS
dnl
dnl Description:
dnl   Produces the configure switch --enable-cbs
dnl   for compiling with support for counting-based search.
dnl
dnl Authors:
dnl   Samuel Gagnon <samuel.gagnon92@gmail.com>
AC_DEFUN([AC_GECODE_CBS],
  [
  AC_ARG_ENABLE([cbs],
    AC_HELP_STRING([--enable-cbs],
      [build with support for counting-based search @<:@default=no@:>@]))
  AC_MSG_CHECKING(whether to build with support for cbs)
  if test "${enable_cbs:-no}" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_SUBST(enable_cbs, yes)
    AC_DEFINE([GECODE_HAS_CBS],[],[Whether counting-based search support available])
  else
    AC_MSG_RESULT(no)
  fi
])

dnl Macro:
dnl   AC_GECODE_CPPROFILER
dnl
dnl Description:
dnl   Produces the configure switch --enable-cpprofiler
dnl   for compiling with support for the CPProfiler.
dnl
dnl Authors:
dnl   Guido Tack <tack@gecode.org>
dnl   Christian Schulte <schulte@gecode.org>
AC_DEFUN([AC_GECODE_CPPROFILER],
  [
  AC_ARG_ENABLE([cpprofiler],
    AC_HELP_STRING([--enable-cpprofiler],
      [build with support for CPProfiler @<:@default=yes@:>@]))
  AC_MSG_CHECKING(whether to build with support for CPProfiler)
  if test "${enable_cpprofiler:-no}" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_SUBST(enable_cpprofiler, yes)
    AC_DEFINE([GECODE_HAS_CPPROFILER],[],[Whether CPProfiler support available])
  else
    AC_MSG_RESULT(no)
  fi
])

AC_DEFUN([AC_GECODE_USER_SUFFIX],
  [
  AC_ARG_WITH([lib-prefix],
    AC_HELP_STRING([--with-lib-prefix],
      [add user-defined prefix to library names]))
  AC_MSG_CHECKING(for user-defined library name prefix)
  if test "x${with_lib_prefix}" != "x"; then
    ac_gecode_userprefix=${with_lib_prefix}
        AC_MSG_RESULT(${with_lib_prefix})
  else
    ac_gecode_userprefix=
        AC_MSG_RESULT(no)
  fi
  AC_DEFINE_UNQUOTED(GECODE_DLL_USERPREFIX,"${ac_gecode_userprefix}",
    [User-defined prefix of dll names])
  AC_ARG_WITH([lib-suffix],
    AC_HELP_STRING([--with-lib-suffix],
      [add user-defined suffix to library names]))
  AC_MSG_CHECKING(for user-defined library name suffix)
  if test "x${with_lib_suffix}" != "x"; then
    ac_gecode_usersuffix=${with_lib_suffix}
    AC_SUBST(USERSUFFIX,${with_lib_suffix})
        AC_MSG_RESULT(${with_lib_suffix})
  else
    ac_gecode_usersuffix=
    AC_SUBST(USERSUFFIX,[""])
    AC_MSG_RESULT(no)
  fi
  AC_DEFINE_UNQUOTED(GECODE_DLL_USERSUFFIX,"${ac_gecode_usersuffix}",
    [User-defined suffix of dll names])
])

AC_DEFUN([AC_GECODE_THREADS],[
  AC_ARG_ENABLE([thread],
    AC_HELP_STRING([--enable-thread],
      [build with multi-threading support @<:@default=yes@:>@]))
  AC_MSG_CHECKING(whether to build with multi-threading support)
  if test "${enable_thread:-yes}" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_CHECK_HEADER(unistd.h,
    [AC_DEFINE(GECODE_HAS_UNISTD_H,1,[Whether unistd.h is available])]
    )
    AC_CHECK_HEADER(pthread.h,
    [AC_DEFINE(GECODE_THREADS_PTHREADS,1,[Whether we have posix threads])
     AC_GECODE_ADD_TO_COMPILERFLAGS([-pthread])
     AC_GECODE_ADD_TO_DLLFLAGS([-pthread])
     AC_CHECK_HEADER([os/lock.h],
       [AC_DEFINE(GECODE_THREADS_OSX_UNFAIR,1,[Whether we have Mac OS threads (new version)])],
        AC_CHECK_HEADER([libkern/OSAtomic.h],
        [AC_DEFINE(GECODE_THREADS_OSX,1,[Whether we have Mac OS threads])],
         AC_MSG_CHECKING([for spin locks])
          AC_TRY_COMPILE([#include <pthread.h>],
            [pthread_spinlock_t t;],
            [AC_MSG_RESULT(yes)
             AC_DEFINE(GECODE_THREADS_PTHREADS_SPINLOCK,1,Whether we have posix spinlocks)],
            [AC_MSG_RESULT(no)]
          )
        )
     )],
    [AC_CHECK_HEADER(windows.h,
      [AC_DEFINE(GECODE_THREADS_WINDOWS,1,[Whether we have windows threads])])]
    )
  else
    AC_MSG_RESULT(no)
  fi
])

AC_DEFUN([AC_GECODE_TIMER],[
  AC_CHECK_HEADER(sys/time.h,
  [AC_DEFINE(GECODE_USE_GETTIMEOFDAY,1,[Use gettimeofday for time-measurement])],
  [AC_DEFINE(GECODE_USE_CLOCK,1,[Use clock() for time-measurement])])
])

dnl check whether we have suifficiently recent versions of flex/bison
AC_DEFUN([AC_GECODE_FLEXBISON],
  [AC_CHECK_TOOL(HAVEFLEX, flex)
   AC_MSG_CHECKING(whether we have at least flex 2.5.33)
   if test "${HAVEFLEX}x" = "x"; then
      AC_MSG_RESULT(no)
      AC_SUBST(HAVE_FLEXBISON, "no")
   else
      if flex --version | grep ' 2\.5\.3[[3-9]].*$' >/dev/null 2>&1 ||
         flex --version | grep ' 2\.5\.4[[0-9]].*$' >/dev/null 2>&1 ||
         flex --version | grep ' 2\.[[6-9]]\.[[0-9]]*$' >/dev/null 2>&1
      then
        AC_MSG_RESULT(yes)
        AC_CHECK_TOOL(HAVEBISON, bison)
        AC_MSG_CHECKING(whether we have at least bison 2.3)
        if test "${HAVEBISON}x" = "x"; then
          AC_MSG_RESULT(no)
          AC_SUBST(HAVE_FLEXBISON, "no")
        else
          if bison --version | grep -e ' 2\.[[3-9]][[0-9]]*' >/dev/null 2>&1 ||
             bison --version | grep -e ' 3\.*' >/dev/null 2>&1
          then
            AC_MSG_RESULT(yes)
            AC_SUBST(HAVE_FLEXBISON, "yes")
          else
            AC_MSG_RESULT(no)
            AC_SUBST(HAVE_FLEXBISON, "no")
          fi
        fi
      else
        AC_MSG_RESULT(no)
        AC_SUBST(HAVE_FLEXBISON, "no")
      fi
   fi])

AC_DEFUN([AC_GECODE_RESOURCE],[
  AC_ARG_ENABLE([resource],
    AC_HELP_STRING([--enable-resource],
      [build Gecode with resource files for Microsoft Visual Studio @<:@default=no@:>@]))
  AC_CHECK_PROG(HAVE_RC, rc.exe, [found])
  AC_MSG_CHECKING(whether to build with Visual Studio resource files)
  if test "${enable_resource:-no}" = "yes" -a \
          "${ac_gecode_compiler_vendor}" = "microsoft"; then
    if test "${HAVE_RC}x" = "x"; then
      AC_MSG_RESULT(no)
      AC_SUBST(RESCOMP, [@true])
      AC_SUBST(enable_resource, no)
    else
      AC_MSG_RESULT(yes)
      AC_SUBST(RESCOMP, ["rc.exe"])
      AC_SUBST(enable_resource, yes)
    fi
  else
    AC_MSG_RESULT(no)
    AC_SUBST(enable_resource, no)
    AC_SUBST(RESCOMP, [@true])
  fi
])
