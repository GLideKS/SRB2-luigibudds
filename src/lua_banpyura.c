// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2026 by GLide KS, romoney5.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_banpyura.c
/// \brief Custom Lua functions for SRB2 Banpyura

#include "p_local.h"
#include "lua_script.h"
#include "lua_libs.h"

angle_t Banpyura_SpriteShadow_Angle = 0;
boolean Banpyura_SpriteShadow_SnapToCamera = true;

///////////////
// FUNCTIONS //
///////////////

// Banpyura.SpriteShadow_SetAngle(angle_t angle?)
static int lib_SpriteShadow_SetAngle(lua_State *L)
{
	if (!lua_isnoneornil(L, 1))
	{
		Banpyura_SpriteShadow_Angle = luaL_checkangle(L, 1);
		Banpyura_SpriteShadow_SnapToCamera = false;
	}
	else
	{
		Banpyura_SpriteShadow_Angle = 0;
		Banpyura_SpriteShadow_SnapToCamera = true;
	}

	return 0;
}

// Banpyura.CV_GetPossibleValue(consvar_t cvar, int num)
// this could be exposed as userdata but i want to keep modding features clean
// this SHOULD return a table
static int lib_CV_GetPossibleValue(lua_State *L)
{
	consvar_t *cvar = *((consvar_t **)luaL_checkudata(L, 1, META_CVAR));
	INT32 num = luaL_checkinteger(L, 2);
	if (!cvar)
		return LUA_ErrInvalid(L, "consvar_t");

	// make sure it's valid
	for (INT32 i = 0; i < num; i++)
		if (cvar->PossibleValue[i].strvalue == NULL)
			return 0;

	CV_PossibleValue_t pv = cvar->PossibleValue[num];

	lua_pushinteger(L, pv.value);
	lua_pushstring(L, pv.strvalue);

	return 2;
}

// Banpyura.CV_ForceAddValue(consvar_t cvar, int value)
// this is needed to set cvars by force
static int lib_CV_ForceAddValue(lua_State *L)
{
	consvar_t *cvar = *(consvar_t **)luaL_checkudata(L, 1, META_CVAR);

	//if (!(cvar->flags & CV_ALLOWLUA))
	//	return luaL_error(L, "Variable %s cannot be set from Lua.", cvar->name);

	CV_AddValue(cvar, (INT32)luaL_checknumber(L, 2));

	return 0;
}

// Banpyura.CV_ForceSet(consvar_t cvar, int value)
// this is needed to set cvars by force
static int lib_CV_ForceSet(lua_State *L)
{
	consvar_t *cvar = *(consvar_t **)luaL_checkudata(L, 1, META_CVAR);

	//if (!(cvar->flags & CV_ALLOWLUA))
	//	return luaL_error(L, "Variable '%s' cannot be set from Lua.", cvar->name);

	switch (lua_type(L, 2))
	{
	case LUA_TSTRING:
		CV_Set(cvar, lua_tostring(L, 2));
		break;
	case LUA_TNUMBER:
		CV_SetValue(cvar, (INT32)lua_tonumber(L, 2));
		break;
	default:
		return luaL_typerror(L, 1, "string or number");
	}

	return 0;
}
// actually, why is it called luaL_typerror and not luaL_typeerror?


static luaL_Reg Banpyura[] = {
	{"SpriteShadow_SetAngle", lib_SpriteShadow_SetAngle},

	{"CV_GetPossibleValue", lib_CV_GetPossibleValue},
	{"CV_ForceAddValue", lib_CV_ForceAddValue},
	{"CV_ForceSet", lib_CV_ForceSet},

	{NULL, NULL}
};

int LUA_BanpyuraLib(lua_State *L)
{
	luaL_register(L, "Banpyura", Banpyura);

	return 0;
}
