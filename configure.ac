#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#
# This file is part of Antz.
# Copyright (C) 2017  Albert Farres
#
# Antz is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Antz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Antz.  If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.69])
AC_INIT([Antz], [0.0.1], [albertfc@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])

# Libtool
#AM_PROG_LIBTOOL

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([src/config.h])

# Throw away default compiler flags. Must be placed after AC_INIT but before AC_PROG_CXX
: ${CFLAGS=""}
: ${CXXFLAGS=""}

# Set proper flags
CXXFLAGS+=" -g -Os -Wall"
CFLAGS+=" -g -Os -Wall"

# Check host
AC_CANONICAL_HOST
AS_CASE([$host],
		[avr-*-*], [],
		[AC_MSG_ERROR([Unsupported host. Use --host=avr option])]
	)

# Force to fail if no crosscompiler is found
AS_IF([test "$host" != "$build" && test "$cross_compiling" = no],
		[AS_CASE([$host],
			[avr-*-*], [guess_cc="avr-gcc"],
			[guess_cc="unknown"]
		)]
		[AC_MSG_ERROR([no cross compiler found. Install $guess_cc.])]
	)

# Checks for programs.
AC_PROG_RANLIB
AC_PROG_CC 
AC_PROG_CXX

AC_LANG([C++])

AX_CXX_COMPILE_STDCXX_11( , [mandatory])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
# c++ stdlib
AC_CHECK_HEADER( [cstdlib], [AC_DEFINE([HAVE_CXXSTDLIB], 1, [c++ stdlib available])] )

# Checks for tools
AC_CHECK_TOOL([OBJCOPY], [objcopy], [:])
AC_CHECK_TOOL([OBJDUMP], [objdump], [:])
AC_CHECK_TOOL([OBJSIZE], [size], [:])

# Check for MCU
AC_ARG_VAR([MCU_TARGET], [select target MCU. Default = atmega328p])
AC_MSG_CHECKING([for selected MCU])
AS_IF([test "x$MCU_TARGET" = x],
		[MCU_TARGET="atmega328p"]
		[AC_MSG_RESULT([$MCU_TARGET (override with env. variable MCU_TARGET)])]
		,
		[AC_MSG_RESULT([$MCU_TARGET])]
	)

# Checks for programs
AC_ARG_VAR([AVRDUDE_MCU], [select target MCU for avrdude. Default = m328p])
AC_ARG_VAR([AVRDUDE_PROGRAMMER], [select programmer options for avrdude. Default = -V -c usbasp -P usb])
AC_CHECK_PROG([AVRDUDE_CHECK], [avrdude], [yes])
AS_IF([test "x$AVRDUDE_CHECK" = xyes],
		[ AC_SUBST([AVRDUDE], [avrdude]) ]
		[AC_MSG_CHECKING([for avrdude selected MCU])]
		[AS_IF([test "x$AVRDUDE_MCU" = x],
			[AVRDUDE_MCU="m328p"]
			[AC_MSG_RESULT([$AVRDUDE_MCU (override with env. variable AVRDUDE_MCU)])]
			,
			[AC_MSG_RESULT([$AVRDUDE_MCU])],
		)]
		[AC_MSG_CHECKING([for avrdude programmer options])]
		[AS_IF([test "x$AVRDUDE_PROGRAMMER" = x],
			[AVRDUDE_PROGRAMMER="-V -c usbasp -P usb"]
			[AC_MSG_RESULT([$AVRDUDE_PROGRAMMER (override with env. variable AVRDUDE_PROGRAMMER)])]
			,
			[AC_MSG_RESULT([$AVRDUDE_PROGRAMMER])],
		)]
		,
		[ AC_MSG_WARN([avrdude not found. You will not be able to upload this program to any chip.])]
	)
AM_CONDITIONAL([WITH_AVRDUDE], [test "x$AVRDUDE_CHECK" = xyes])

AC_CHECK_PROG([CSCOPE_CHECK], [cscope], [yes])
AM_CONDITIONAL([WITH_CSCOPE], [test "x$CSCOPE_CHECK" = xyes])

AC_MSG_NOTICE([])
AC_MSG_NOTICE([type 'make' to compile everything])
AS_IF([test "x$AVRDUDE_CHECK" = xyes],
		[AC_MSG_NOTICE([type 'make upload' to upload this program to an AVR MCU.])]
	)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
#AC_CHECK_LIB([m], [fabs], [], AC_MSG_ERROR([fabs() not found in libm.]))

AC_OUTPUT(
	[Makefile]
	[src/Makefile]
)
