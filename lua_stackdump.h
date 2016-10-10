/**
 * @file
 * @brief Small helper function for LUA to print stack traces from C
 * @copyright Martin Gerhardy (http://github.com/mgerhardy)
 *
 * @note
 * to create the implementation,
 *     #define LUASD_IMPLEMENTATION
 * in *one* C/CPP file that includes this file.
 *
 * @par License
 * Public domain
 *
 * @par Changelog:
 * 0.1 Initial release with some missing features and bugs, see the TODO markers in the code
 */

#ifndef LUA_STACKDUMP_H
#define LUA_STACKDUMP_H

#ifndef luaSD_MAXDEPTH
#define luaSD_MAXDEPTH 2
#endif

#ifndef luaSD_PRINT
#include <stdio.h>
#define luaSD_PRINT printf
#endif

#ifndef LUASD_API
#define LUASD_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef int (*luaSD_printf) (const char *__restrict __format, ...);

extern void luaSD_stackdump (lua_State* state, luaSD_printf luasdprintf);
extern void luaSD_stackdump_default (lua_State* state);

#ifdef __cplusplus
}
#endif

#ifdef LUASD_IMPLEMENTATION
static void luaSD_stackdumpvalue (lua_State* state, luaSD_printf luasdorintf, int stackIndex, int depth, int newline, int indentLevel, int performIndent) {
	int t;
	/* ensure that the value is on the top of the stack */
	lua_pushvalue(state, stackIndex);
	t = lua_type(state, -1);
	switch (t) {
	case LUA_TNUMBER:
		lua_pushfstring(state, "%f (%s)", lua_tonumber(state, -1), luaL_typename(state, -1));
		break;
	case LUA_TSTRING:
		lua_pushfstring(state, "%s (%s)", lua_tostring(state, -1), luaL_typename(state, -1));
		break;
	case LUA_TLIGHTUSERDATA:
	case LUA_TUSERDATA: {
		if (lua_getmetatable(state, -1) == 0) {
			lua_pushfstring(state, "%s [%p] (no metatable)", luaL_typename(state, -1), lua_topointer(state, -1));
		} else {
			/* TODO: this else branch was never reached in my tests */
			lua_getfield(state, -1, "__tostring");
			if (lua_isfunction(state, -1)) {
				if (lua_pcall(state, 0, 1, 0)) {
					lua_pushfstring(state, "%s [%p]", luaL_typename(state, -1), lua_topointer(state, -1));
				}
				lua_pop(state, 1);
			} else {
				lua_pop(state, 1);
				luaSD_stackdumpvalue(state, luasdorintf, -1, depth + 1, 1, indentLevel + 4, 1);
				lua_pop(state, 1);
				lua_pushfstring(state, "%s [%p]", luaL_typename(state, -1), lua_topointer(state, -1));
			}
			lua_pop(state, 1);
		}
		break;
	}
	case LUA_TBOOLEAN: {
		const int value = lua_toboolean(state, -1);
		if (value) {
			lua_pushliteral(state, "true");
		} else {
			lua_pushliteral(state, "false");
		}
		break;
	}
	case LUA_TNIL:
		lua_pushliteral(state, "nil");
		break;
	case LUA_TFUNCTION: {
#if 0
		/* TODO: somehow get function details. */
		lua_Debug ar;
		lua_getstack(state, -1, &ar);
		if (lua_getinfo(state, "nSlLtuf", &ar) != 0) {
			lua_pushfstring(state, "%s %s %d @%d %s", ar.namewhat, ar.name, ar.nups, ar.linedefined, ar.short_src);
		} else
#endif
		{
			lua_pushfstring(state, "%s [%p]", luaL_typename(state, -1), lua_topointer(state, -1));
		}
		break;
	}
	case LUA_TTABLE: {
		int len = 0;
		if (performIndent) {
			if (depth == 0) {
				luasdorintf("%-5i | ", stackIndex);
			} else {
				luasdorintf("       ", stackIndex);
			}
			for (int i = 0; i < indentLevel - 4; ++i) {
				luasdorintf(" ");
			}
			if (indentLevel >= 4) {
				luasdorintf("\\-- ");
			}
		}
		lua_pushnil(state);
		while (lua_next(state, -2)) {
			lua_pop(state, 1);
			++len;
		}

		lua_pushfstring(state, "%s: %p (size: %d)",
				luaL_typename(state, -1),
				lua_topointer(state, -1),
				len);
		luasdorintf("%s", lua_tostring(state, -1));
		lua_pop(state, 1);

		if (depth < luaSD_MAXDEPTH) {
			lua_pushnil(state);
			while (lua_next(state, -2)) {
				luasdorintf("\n");
				luaSD_stackdumpvalue(state, luasdorintf, -2, depth + 1, 0, indentLevel + 4, 1);
				luasdorintf(" = ");
				luaSD_stackdumpvalue(state, luasdorintf, -1, depth + 1, 0, indentLevel + 4, 0);
				if (lua_type(state, -1) == LUA_TFUNCTION && lua_type(state, -2) == LUA_TSTRING) {
					if (!strcmp("__tostring", lua_tostring(state, -2))) {
						if (lua_pcall(state, 0, 1, 0)) {
							/* TODO: push object to the stack to call __tostring from */
							if (lua_isstring(state, -1)) {
								luasdorintf(" (%s)", lua_tostring(state, -1));
							} else {
								luasdorintf(" (failed to call __tostring)");
							}
						} else {
							luasdorintf("%s", lua_tostring(state, -1));
							lua_pop(state, 1);
						}
					}
				}
				lua_pop(state, 1);
			}
		}
		/* pop the reference copy from the stack to restore the original state */
		lua_pop(state, 1);
		if (newline) {
			luasdorintf("\n");
		}
		return;
	}
	default:
		lua_pushfstring(state, "%s [%p]", luaL_typename(state, -1), lua_topointer(state, -1));
		break;
	}

	{
		int width = 20;
		if (performIndent) {
			if (depth == 0) {
				luasdorintf("%-5i | ", stackIndex);
			} else {
				luasdorintf("       ", stackIndex);
			}
			for (int i = 0; i < indentLevel - 4; ++i) {
				luasdorintf(" ");
			}
			if (indentLevel >= 4) {
				luasdorintf("\\-- ");
			}
			width -= indentLevel;
		}
		luasdorintf("%-*s", width, lua_tostring(state, -1));
	}
	if (newline) {
		luasdorintf("\n");
	}
	/* pop the string and the reference copy from the stack to restore the original state */
	lua_pop(state, 2);
}

LUASD_API void luaSD_stackdump (lua_State* state, luaSD_printf luasdprintf) {
	const int top = lua_gettop(state);
	luasdprintf("\n--------------------start-of-stacktrace----------------\n");
	luasdprintf("index | details (%i entries)\n", top);
	int i;
	for (i = -1; i >= -top; --i) {
		luaSD_stackdumpvalue(state, luasdprintf, i, 0, 1, 0, 1);
	}
	luasdprintf("----------------------end-of-stacktrace----------------\n\n");
}

LUASD_API void luaSD_stackdump_default (lua_State* state) {
	luaSD_stackdump(state, luaSD_PRINT);
}
#endif

#endif

