#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([config-qgh], [1.0.0], [qigaohua@xxx.con])
# add
AM_INIT_AUTOMAKE(config-qgh, 1.0.0)
AM_INIT_AUTOMAKE(subdir-objects)

AC_CONFIG_SRCDIR([test/test_json.c])
AC_CONFIG_HEADERS([config.h])


AC_DEFINE(ENABLE_INI, 1, "use ini")
AC_DEFINE(ENABLE_LUA, 1, "use lua")

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# add 表示构建当前目录时需要生成静态库文件
AC_PROG_RANLIB

# 用于生成动态库
AC_PROG_LIBTOOL
LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h locale.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([localeconv memmove memset modf strcasecmp strchr strdup strerror strrchr strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/json/Makefile
                 src/ini/Makefile
                 src/lua/Makefile
                 test/Makefile])
AC_OUTPUT
