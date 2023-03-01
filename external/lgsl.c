#include <assert.h>
#include <string.h>
#include "gsl/gsl_sort_double.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "stdlib.h"

#include "gsl/gsl_sort.h"
#include "gsl/gsl_statistics.h"

static size_t num2vector(lua_State* L, int index, double** vp) {
  size_t size = lua_rawlen(L, index);
  *vp = (double*)malloc(sizeof(double) * size);
  assert(*vp != NULL);
  for (size_t i = 0; i < size; i++) {
    lua_pushinteger(L, i + 1);
    lua_gettable(L, index);
    (*vp)[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }
  return size;
}

static int lgsl_mean(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  double* v = NULL;
  size_t size = num2vector(L, 1, &v);
  double mean = gsl_stats_mean(v, 1, size);
  lua_pushnumber(L, mean);
  free(v);
  return 1;
}

static int lgsl_std(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  double* v = NULL;
  size_t size = num2vector(L, 1, &v);
  double mean = gsl_stats_sd(v, 1, size);
  lua_pushnumber(L, mean);
  free(v);
  return 1;
}

static int lgsl_status(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
    return 6;
  }
  double* v = NULL;
  size_t size = num2vector(L, 1, &v);
  gsl_sort(v, 1, size);
  double mean = gsl_stats_mean(v, 1, size);
  double std = gsl_stats_sd_m(v, 1, size, mean);
  double skew = gsl_stats_skew_m_sd(v, 1, size, mean, std);
  double kurt = gsl_stats_kurtosis_m_sd(v, 1, size, mean, std);
  lua_pushnumber(L, v[0]);  // min, max
  lua_pushnumber(L, v[size - 1]);
  lua_pushnumber(L, mean);
  lua_pushnumber(L, std);
  lua_pushnumber(L, skew);
  lua_pushnumber(L, kurt);
  free(v);
  return 6;
}

static int lgsl_skew(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    lua_pushnil(L);
    return 2;
  }
  double* v = NULL;
  size_t size = num2vector(L, 1, &v);
  double skew = gsl_stats_skew(v, 1, size);
  lua_pushnumber(L, skew);
  free(v);
  return 1;
}

static int lgsl_kurt(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    lua_pushnil(L);
    return 2;
  }
  double* v = NULL;
  size_t size = num2vector(L, 1, &v);
  double kurt = gsl_stats_kurtosis(v, 1, size);
  lua_pushnumber(L, kurt);
  free(v);
  return 1;
}

static int lgsl_percentiles(lua_State* L) {
  if (!lua_istable(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  double* v = NULL;
  double* percentiles = NULL;
  size_t vsize = num2vector(L, 1, &v);
  size_t psize = num2vector(L, 2, &percentiles);
  if (psize == 0 || vsize == 0) {
    lua_pushnil(L);
    return 1;
  }
  gsl_sort(v, 1, vsize);
  lua_newtable(L);
  double median = gsl_stats_median_from_sorted_data(v, 1, vsize);
  lua_pushnumber(L, median);
  lua_setfield(L, -2, "median");
  for (size_t i = 0; i < psize; i++) {
    char pname[64] = {0};
    sprintf(pname, "%.2f", percentiles[i] * 100);
    size_t pnlen = strlen(pname);
    if (pname[pnlen - 1] == '0' && pname[pnlen - 2] == '0' &&
        pname[pnlen - 3] == '.') {
      pname[pnlen - 3] = '%';
      pname[pnlen - 2] = '\0';
    } else if (pname[pnlen - 1] == '0') {
      pname[pnlen - 1] = '%';
    } else {
      pname[pnlen] = '%';
      pname[pnlen + 1] = '\0';
    }
    double p = gsl_stats_quantile_from_sorted_data(v, 1, vsize, percentiles[i]);
    lua_pushnumber(L, p);
    lua_setfield(L, -2, pname);
  }
  free(v);
  free(percentiles);
  return 1;
}

const static luaL_Reg libs[] = {
    {"mean", lgsl_mean},
    {"std", lgsl_std},
    {"skew", lgsl_skew},
    {"kurt", lgsl_kurt},
    {"percentiles", lgsl_percentiles},
    {"status", lgsl_status},  // return mean, std, skew, kurt
    {NULL, NULL}};

LUAMOD_API int luaopen_gsl(lua_State* L) {
  luaL_newlib(L, libs);
  return 1;
}
