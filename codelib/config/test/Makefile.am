NULL =

AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = test_json test_ini test_lua

test_json_LDADD =  \
        -L../src/ -lconfig \
        $(NULL)

test_ini_LDADD =  \
        -L../src/ -lconfig \
        $(NULL)

test_lua_LDADD =  \
        -L../src/ -lconfig \
        $(NULL)

test_json_SOURCES = test_json.c
test_ini_SOURCES = test_ini.c
test_lua_SOURCES = test_lua.c

