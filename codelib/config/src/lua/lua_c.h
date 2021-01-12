#ifndef __LUAC_H
#define __LUAC_H



#define LUA_INT_ERROR  (-991287367)
#define LUA_DOUBLE_ERROR  (-98321123.012)

lua_State* lua_c_loadfile(const char *filename);
void lua_c_close(lua_State *L);

#define LUA_CONFIG
#ifdef LUA_CONFIG

int lua_get_integer(lua_State *L, va_list ap);
double lua_get_double(lua_State *L, va_list ap);
int lua_get_bool(lua_State *L, va_list ap);
const char* lua_get_string(lua_State *L, va_list ap);

#else


int lua_get_integer(lua_State *L, ...);
double lua_get_double(lua_State *L, ...);
int lua_get_bool(lua_State *L, ...);
const char* lua_get_string(lua_State *L, ...);


// 用户调用
#define lua_c_get_integer(L, ...) lua_get_integer(L, __VA_ARGS__, NULL)
#define lua_c_get_double(L, ...) lua_get_double(L, __VA_ARGS__, NULL)
#define lua_c_get_bool(L, ...) lua_get_bool(L, __VA_ARGS__, NULL)
#define lua_c_get_string(L, ...) lua_get_string(L, __VA_ARGS__, NULL)
#endif

#endif
