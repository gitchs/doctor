#include "lhashlib.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "xxh3.h"
#include "xxhash.h"

const static char* const XXH64_TYPE = "XXH64*";

#define GET_XXH64_STATE(L, state)                     \
  do {                                                \
    (state) = (XXH64_state_t*)lua_touserdata((L), 1); \
    assert((state) != NULL);                          \
    if ((state) == NULL) {                            \
      lua_error((L));                                 \
      return 0;                                       \
    }                                                 \
  } while (0)

static int lhashlib_xxh64_init(lua_State* L) {
  unsigned long long seed = 0;
  if (lua_isinteger(L, 1)) {
    seed = (unsigned long long)lua_tointeger(L, 1);
  }
  XXH64_state_t* state = lua_newuserdata(L, sizeof(XXH64_state_t));
  XXH64_reset(state, seed);
  luaL_setmetatable(L, XXH64_TYPE);
  return 1;
}

static int lhashlib_xxh64_update(lua_State* L) {
  XXH64_state_t* state = NULL;
  GET_XXH64_STATE(L, state);
  size_t blen = 0;
  unsigned char* const buffer = (unsigned char*)lua_tolstring(L, 2, &blen);
  if (blen > 0) {
    XXH64_update(state, buffer, blen);
  }
  lua_pushboolean(L, 1);
  return 1;
}

static int lhashlib_xxh64_digest(lua_State* L) {
  XXH64_state_t* state = NULL;
  GET_XXH64_STATE(L, state);
  unsigned long long digest = (unsigned long long)XXH64_digest(state);
  lua_pushinteger(L, digest);
  return 1;
}

static int lhashlib_xxh64_hexdigest(lua_State* L) {
  char hexdigest[17] = {0};
  XXH64_state_t* state = NULL;
  GET_XXH64_STATE(L, state);
  uint64_t digest = (uint64_t)XXH64_digest(state);
  sprintf(hexdigest, "%02x%02x%02x%02x%02x%02x%02x%02x",
          (uint8_t)(digest >> 56), (uint8_t)(digest >> 48),
          (uint8_t)(digest >> 40), (uint8_t)(digest >> 32),
          (uint8_t)(digest >> 24), (uint8_t)(digest >> 16),
          (uint8_t)(digest >> 8), (uint8_t)(digest));
  lua_pushstring(L, hexdigest);
  return 1;
}

static int lhashlib_xxh64_reset(lua_State* L) {
  XXH64_state_t* state = NULL;
  GET_XXH64_STATE(L, state);
  unsigned long long seed = 0;
  if (lua_isinteger(L, 2)) {
    seed = (unsigned long long)lua_tointeger(L, 2);
  }
  XXH64_reset(state, seed);
  lua_pushboolean(L, 1);
  return 1;
}

static luaL_Reg xxh64_libs[] = {{"reset", lhashlib_xxh64_reset},
                                {"update", lhashlib_xxh64_update},
                                {"digest", lhashlib_xxh64_digest},
                                {"hexdigest", lhashlib_xxh64_hexdigest},
                                {NULL, NULL}};

static luaL_Reg libs[] = {{"xxh64", lhashlib_xxh64_init}, {NULL, NULL}};

LUAMOD_API int luaopen_hashlib(lua_State* L) {
  luaL_newlib(L, libs);
  if (luaL_newmetatable(L, XXH64_TYPE)) {
    luaL_newlib(L, xxh64_libs);
    lua_setfield(L, -2, "__index");
  }
  lua_pop(L, 1);
  return 1;
}
