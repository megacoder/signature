dnl $Id: configure.in,v 1.1 2009/10/24 17:48:09 reynolds Exp $

dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([signature], [0.14], [oldest.software.guy@gmail.com])
AC_CONFIG_SRCDIR([signature.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PATH_PROG(fortune, fortune, none, /usr/games:/usr/local/games:/usr/local/bin:/opt/bin:/opt/games:$PATH)
if test "$fortune" = "NONE"; then
    AC_MSG_ERROR(Oh dear[,] I couldn't find fortune on this system)
else
    AC_ARG_ENABLE(offensive, [  --enable-offensive      build with offensive fortunes], [if test $enableval = yes; then
	     dnl --enable-offensive-fortunes
	     $fortune -a
	     if test $? = 0; then
		 AC_DEFINE_UNQUOTED(OFFENSIVE_FORTUNES)
	     else
		 AC_MSG_WARN(Your fortune program doesn't produce offensive cookies!)
	     fi
	 else
	     dnl --disable-offensive-fortunes
	     :
	 fi],
	[dnl No option given
	 $fortune -a
	 if test $? = 0; then
	     AC_DEFINE_UNQUOTED(OFFENSIVE_FORTUNES)
	 else
	     AC_MSG_WARN(Your fortune program doesn't produce offensive cookies!)
	 fi])
AC_DEFINE_UNQUOTED(FORTUNE, "$fortune")
fi

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(mkfifo uname)

AC_OUTPUT([
   Makefile
   signature.spec
])
AC_OUTPUT_COMMANDS([echo OK, now type \'make\' to build signature])
