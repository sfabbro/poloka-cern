## Initialization
AC_PREREQ(2.59)
AC_INIT([poloka-cern], 0.1.0, [poloka team])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([tools/l2tup.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(autoconf)
AM_INIT_AUTOMAKE([foreign])

## Checks for programs to build poloka-core library
AC_PROG_CXX
AC_PROG_F77

AC_LIB_CERNLIB([packlib],
	       [HROPEN],
               [CERNLIB=true],
               [AC_MSG_ERROR([cernlib was not found])])

PKG_CHECK_MODULES([POLOKA_CORE],
		  [poloka-core],,
		  [AC_MSG_ERROR([Cound not find the poloka-core library])])

AC_CONFIG_FILES([Makefile
		 tools/Makefile])
 AC_OUTPUT
