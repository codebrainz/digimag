AC_PREREQ([2.67])
AC_INIT([digimag], [1.0], [mbrush@codebrainz.ca])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign silent-rules])
AM_SILENT_RULES([yes])

AC_PROG_CC

AC_CHECK_HEADERS(ctype.h errno.h libgen.h stdio.h string.h sys/stat.h)

PKG_CHECK_MODULES([OPENCV], [opencv >= 2.3])

AC_CONFIG_FILES([
	Makefile
  src/Makefile
])
AC_OUTPUT
