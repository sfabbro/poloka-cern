dnl                                                      -*- Autoconf -*- 
dnl  autoconf macros for the cernlib libraries
dnl  Copyright (C) 2004, 2005 Patrice Dumas
dnl  Copyright (C) 2012 Sebastien Fabbro (almost complete rewrite)
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

dnl A basic axample of the macro usage:
dnl 
dnl AX_CERNLIB([kernlib], [CLTOU],,
dnl [
dnl    AX_CERNLIB([mathlib], [GAUSS],[LIBS="$CERNLIB_LIBS $LIBS"])
dnl ])

dnl
dnl --enable-cernlib-static: forces the static or dynamic linking
dnl --with-cernlib-libs: specify the linking flags (default is -l<CERN_LIB>)
dnl --with-cernlib-cflags: specify pre-processing flags (default is blank)
dnl
dnl If the type of linking isn't specified it is assumed to be static.
dnl
dnl For static linking:
dnl - if the binary program 'cernlib-static' or 'cernlib' is in the path it is 
dnl   assumed that a static linking using the information reported by that 
dnl   script is wanted.
dnl - otherwise if the environment variable CERNLIB exists it is assumed to be the
dnl   location of the static library.
dnl - otherwise if the environment variable CERN_ROOT exists it is assumed 
dnl   that CERN_ROOT/lib is the location of the static library.
dnl - otherwise a simple linking is performed.
dnl 
dnl If a dynamic linking is selected:
dnl - if the binary program 'cernlib' is in the path it is assumed that it is the 
dnl   debian script and it is called with -dy.
dnl - otherwise a simple linking is performed.
dnl
dnl AX_CERNLIB ([LIBRARY = kernlib], [FUNCTION = CLTOU], [HEADER = ],
dnl             [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND]) 
dnl should be called afterwards for each of the cernlib library needed. 
dnl Based on the information collected by AC_CERNLIB the needed flags are 
dnl added to the linking flags or the files and flags are added to the 
dnl $CERNLIB_LIBS shell variable and a test of linking is performed.
dnl
dnl The resulting linking flags and files are in $CERNLIB_LIBS.

AC_DEFUN([AX_DEFINE_CERNLIB], [

CERNLIB_STATIC="no"
CERNLIB_BIN=""
CERNLIB_DIR=""

AC_ARG_ENABLE([cernlib-static],
	    AS_HELP_STRING([--enable-cernlib-static],
			   [Link statically with cernlib libraries]),
	    [CERNLIB_STATIC=$enableval],
	    [CERNLIB_STATIC=no])

AC_ARG_WITH([cernlib-libs],
	    AS_HELP_STRING([--with-cernlib-libs=LIBS],
			   [Linking flags for CERNLIB libraries]),
	    [CERNLIB_LIBS="$withval"])

AC_ARG_WITH([cernlib-cflags],
	    AS_HELP_STRING([--with-cernlib-cflags=CFLAGS],
		           [Preprocessing flags for CERNLIB headers]),
	    [CERNLIB_CFLAGS="$withval"])

AC_PATH_PROG([CERNLIB_BIN], [cernlib])
if test x"$CERNLIB_BIN" = x -a x"$CERNLIB_STATIC" != x"no"; then
   AC_PATH_PROG([CERNLIB_BIN], [cernlib-static])
fi

if test x"$CERNLIB_BIN" = x; then
   if test x"$CERNLIB" != x -a -d "$CERNLIB"; then
      CERNLIB_DIR="$CERNLIB"
      AC_MSG_NOTICE([Using the CERNLIB environment variable to determine directory install])
   elif test x"$CERN_ROOT" != x -a -d "$CERN_ROOT/lib"; then
      CERNLIB_DIR="$CERN_ROOT/lib"
      AC_MSG_NOTICE([Using the CERN_ROOT environment variable to determine directory install])
  fi
fi

AC_PROG_F77

])

# AX_CERNLIB ([LIBRARY = kernlib], [FUNCTION = CLTOU], [HEADER], [ACTION-IF-FOUND],
#          [ACTION-IF-NOT-FOUND]) 
# check for a function in a library of the cernlib
AC_DEFUN([AX_CERNLIB], [

AC_REQUIRE([AX_DEFINE_CERNLIB])

ac_cernlib_func="CLTOU"
ac_cernlib_library="kernlib"
ac_cernlib_header=""
ac_cernlib_ok="no"

if test x"$1" != x; then
    ac_cernlib_library=$1
fi

if test x"$2" != x; then
    ac_cernlib_func=$2
fi

if test x"$3" != x; then
    ac_cernlib_header=$3
fi


save_CERNLIB_LIBS=$CERNLIB_LIBS
save_LIBS=$LIBS

dnl define CERNLIB_LIBS variable
if test x"$CERNLIB_STATIC" != x"no"; then
    ac_cernlib_lib_static_found=no
    AC_MSG_NOTICE([cernlib: linking with a static $ac_cernlib_library])
    if test x"$CERNLIB_BIN" != x; then
        ac_cernlib_bin_out=`$CERNLIB_BIN $ac_cernlib_library`
        CERNLIB_LIBS="$ac_cernlib_bin_out $CERNLIB_LIBS"
        ac_cernlib_lib_static_found=yes
    elif test x"$CERNLIB_DIR" != x; then
        if test -f "$CERNLIB_DIR/lib${ac_cernlib_library}.a"; then 
            CERNLIB_LIBS="$CERNLIB_DIR/lib${ac_cernlib_library}.a $CERNLIB_LIBS"
            ac_cernlib_lib_static_found=yes
        fi
    fi
    if test x"$ac_cernlib_lib_static_found" = x"no"; then
        AC_MSG_WARN([cannot determine the cernlib location for static linking])
    fi
else
    if test x"$CERNLIB_BIN" != x; then
        # try link with debian cernlib script with -dy
        ac_cernlib_bin_out=`$CERNLIB_BIN -dy $ac_cernlib_library`
        CERNLIB_LIBS="$ac_cernlib_bin_out"
    elif test x"$CERNLIB_DIR" != x; then
        CERNLIB_LIBS="-L$CERNLIB_DIR -l${ac_cernlib_library} $CERNLIB_LIBS"
    else
        CERNLIB_LIBS="-l${ac_cernlib_library} $CERNLIB_LIBS"
    fi
fi

dnl now try the link
AC_MSG_CHECKING([for $ac_cernlib_func is defined in $ac_cernlib_library])
LIBS="$CERNLIB_LIBS $LIBS"
AC_LANG_PUSH(Fortran 77)
AC_LINK_IFELSE([      program main
      call $ac_cernlib_func
      end
], 
[ 
    ac_cernlib_ok="yes"
],
[
    CERNLIB_LIBS=$save_CERNLIB_LIBS
])

LIBS=$save_LIBS
AC_MSG_RESULT([$ac_cernlib_ok])

AC_LANG_POP([Fortran 77])

dnl check header (will need cfortran.h)
if test x"$ac_cernlib_header" != x; then
   CPPFLAGS="$CERNLIB_CFLAGS $CPPFLAGS"
   ## need cfortran
   case x$F77 in
	x*gfortran) CERNLIB_CFLAGS="-DgFortran $CERNLIB_CFLAGS" ;;
   	x*g77) CERNLIB_CFLAGS="-Dg77Fortran $CERNLIB_CFLAGS" ;;
	x*ifort) CERNLIB_CFLAGS="-DINTEL_COMPILER $CERNLIB_CFLAGS" ;;
	x*pgf77) CERNLIB_CFLAGS="-DpgiFortran $CERNLIB_CFLAGS" ;;
    	x*) CERNLIB_CFLAGS="-Df2cFortran $CERNLIB_CFLAGS" ;;
   esac
   
   AC_CHECK_HEADER([cfortran.h],,
		   AC_MSG_WARN([Could not find cfortran.h]))

   AC_CHECK_HEADER([$ac_cernlib_header], [ac_cernlib_header="yes"],
		   AC_MSG_WARN([Could not find $ac_cernlib_header]))
   if test x"$ac_cernlib_header" != x"yes"; then
      ac_cernlib_ok="no"
   fi
   CPPFLAGS=$save_CPPFLAGS
fi

dnl done
AC_SUBST([CERNLIB_LIBS])
AC_SUBST([CERNLIB_CFLAGS])

AS_IF([test x"$ac_cernlib_ok" = x"yes"],
      [m4_default([$4], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_CERNLIB_${ac_cernlib_library}_${ac_cernlib_func}))
      ])],
      [
       AC_MSG_WARN([Cannot link or compile simple cernlib programs ])
       $5
      ])
])
