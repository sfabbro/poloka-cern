## Initialization

AC_INIT([poloka-cern], 0.1.0, [poloka team])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([tools/pka-list2hbook.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(autoconf)
AM_INIT_AUTOMAKE([foreign])

## Checks for programs
AC_PROG_CXX

AX_CERNLIB([packlib],
	   [HROPEN],
	   [packlib.h],
           [],
           [AC_MSG_ERROR([CERN library packlib or packlib.h header not found])])

PKG_CHECK_MODULES([POLOKA_CORE],
		  [poloka-core],,
		  [AC_MSG_ERROR([Cound not find the poloka-core library])])

AC_CONFIG_FILES([Makefile
		 tools/Makefile])
AC_OUTPUT
