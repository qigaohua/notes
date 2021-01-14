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
#AC_DEFINE(ENABLE_LUA, 1, "use lua") # 放到后面了

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# add 表示构建当前目录时需要生成静态库文件
AC_PROG_RANLIB

# 用于生成动态库
AC_PROG_LIBTOOL
# LT_INIT # 以前libtool版本没有

#AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug], [compile for debugging])],
#        [AM_CFLAGS="-g -O0"],
#        [AM_CFLAGS="-O2 -minline-all-stringops -fomit-frame-pointer"])

# 添加--enable-lua，当使能的时候编译 '获取lua配置文件数据' 的模块
AC_ARG_ENABLE([lua], [AC_HELP_STRING([--enable-lua], [compile for get lua config file data])],
        [LUA_DIR="lua";LUA_LIBLDD="-L./lua/ -lluac"],
        [LUA_DIR=""; LUA_LIBLDD=""]
        )
# 使所有Makefile.am 可以使用LUA_DIR、LUA_LIBLDD
AC_SUBST(LUA_DIR)
AC_SUBST(LUA_LIBLDD)

# Checks for libraries.
# 如果使用的--enable-lua, 检测系统中是否由安装了lua5.2,没有就退出
if test x${LUA_DIR} == x"lua"; then
    AC_CHECK_LIB(lua5.2, luaL_newstate, [AC_DEFINE(ENABLE_LUA, 1, "use lua")], [
            echo "Error! You need to have liblua5.2 installed, use --disable-lua !!!"
            exit -1
    ])
    LUA_MAKEFILE="src/lua/Makefile"
else
    LUA_MAKEFILE=""
fi

# 编译测试模块test_lua使用
AM_CONDITIONAL(TEST_LUA, test ${LUA_MAKEFILE} != "")


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
                 ${LUA_MAKEFILE}
                 test/Makefile])
AC_OUTPUT
