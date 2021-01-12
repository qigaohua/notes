NULL =

lib_LTLIBRARIES = libluac.la


# $(includdir)的值由autoconf为我们自动生成，默认是'/urs/include' 或 '/usr/local/include'
# _CPPFLAGS指定预处理阶段头文件的查找目录(gcc -I)
libluac_la_CPPFLAGS = \
    -I$(includedir) \
    $(NULL)


# automake可以从autoconf继承变量
# 即Makefile文件用到下列变量时可以在configure脚本文件中寻找这些变量的定义
# 当然，这些变量最初是由开发者在configure.ac文件中增加的
# _LIBADD指定链接阶段so库的查找路径(gcc -l)
libluac_la_LIBADD = \
    -L/usr/lib/x86_64-linux-gnu/ -llua5.2 \
    $(NULL)


libluac_la_SOURCES = ./lua_c.c ./lua_config.c

# 安装
libluac_includedir=$(includedir)
libluac_include_HEADERS = \
                ./lua_c.h
