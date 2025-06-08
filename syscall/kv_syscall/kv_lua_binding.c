// kv_lua_binding.c
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <sys/syscall.h>
#include <unistd.h>

// 假设你的系统调用号
#define __NR_write_kv 449
#define __NR_read_kv 450

static int lua_write_kv(lua_State *L) {
    int key = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    long result = syscall(__NR_write_kv, key, value);
    printf("Writing key: %d, value: %d Ret: %d\n", key, value, (int)result);

    lua_pushinteger(L, result);
    return 1;
}

static int lua_read_kv(lua_State *L) {
    int key = luaL_checkinteger(L, 1);
    
    long result = syscall(__NR_read_kv, key);
    printf("Reading key: %d Ret: %d\n", key, (int)result);
    lua_pushinteger(L, result);
    return 1;
}

static const struct luaL_Reg kv_functions[] = {
    {"write_kv", lua_write_kv},
    {"read_kv", lua_read_kv},
    {NULL, NULL}
};

int luaopen_kv(lua_State *L) {
    luaL_newlib(L, kv_functions);
    return 1;
}