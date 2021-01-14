NULL =

AUTOMAKE_OPTIONS = foreign
#bin_PROGRAMS = test_json test_ini test_lua

if TEST_LUA
bin_PROGRAMS = test_json test_ini test_lua
else
bin_PROGRAMS = test_json test_ini
endif

if TEST_LUA
test_json_LDADD =  \
        -L../src/ -lconfig \
        -L../src/json/ -lcjson \
        -L../src/ini/ -lini \
        -L../src/lua/ -lluac
        $(NULL)

test_ini_LDADD =  \
        -L../src/ -lconfig \
        -L../src/json/ -lcjson \
        -L../src/ini/ -lini \
        -L../src/lua/ -lluac
        $(NULL)

test_lua_LDADD =  \
        -L../src/ -lconfig \
        -L../src/json/ -lcjson \
        -L../src/ini/ -lini \
        -L../src/lua/ -lluac
        $(NULL)
else

test_json_LDADD =  \
        -L../src/ -lconfig \
        -L../src/json/ -lcjson \
        -L../src/ini/ -lini \
        $(NULL)

test_ini_LDADD =  \
        -L../src/ -lconfig \
        -L../src/json/ -lcjson \
        -L../src/ini/ -lini \
        $(NULL)

endif

test_json_SOURCES = test_json.c
test_ini_SOURCES = test_ini.c

if TEST_LUA
test_lua_SOURCES = test_lua.c
endif

