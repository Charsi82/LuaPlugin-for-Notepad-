#pragma once
#include "PluginOptions.hpp"
#include "PluginSettings.hpp"

// addtional menu item for test
#ifdef _DEBUG
#define TEST_ITEM
#endif // _DEBUG

// print content Lua stack
//#define DUMPSTACK

// quick and dirty using the varargs
using voidfunc_t = void* (*)();
using varfuncchar_t = char* (*)(void* L, ...);
using varfuncint_t = int (*)(void* L, ...);
using varfuncvoid_t = void (*)(void* L, ...);

enum enumNFuncItems
{
	CheckSyntax = 0,
	RunScript,
	CheckFiles,
	AutoFormat,
	RunLove2D,
	Separator1,
	//LUA51,
	//LUA52,
	//LUA53,
	//LUA54,
	//LUAPLUTO,
	//LUAJIT,
	//Separator2,
	ShowHideConsole,
	//Separator3,
	Options,
	About,
#ifdef TEST_ITEM
	TestItem,
#endif
	nbFunc
};

enum eLuaVersion
{
	LUA51 = nbFunc + 1,
	LUA52,
	LUA53,
	LUA54,
	LUA55,
	LUAPLUTO,
	LUAJIT,
};

class CLuaManager
{
private:
	HMODULE hinstLib{ NULL };
	void* L{ nullptr }; // Lua_State
	//eLuaVersion m_lua_ver{ eLuaVersion::LUA51 };
	void dump_stack(void* State);
	void reset_state();
	void destroy_state();

	bool isLua51() const noexcept;
	bool isLua55() const noexcept;
	bool isFunction(void* State, int idx, const char* msg) const;
	void do_load_libs(void* State);
	void do_lua_call(void* State, int narg, int nret);
	int do_lua_pcall(void* State, int narg, int nret);
	void do_lua_getglobal(void* State, const char* name);
	void do_lua_setglobal(void* State, const char* name);
	int do_lua_tointeger(void* State, int idx);

	voidfunc_t luaL_newstate{}; /* lua_State *luaL_newstate (void) */
	varfuncvoid_t luaL_openlibs{}; /* void (luaL_openlibs) (lua_State *L); */
	varfuncvoid_t luaL_openselectedlibs{}; /* void (luaL_openselectedlibs) (lua_State *L, int load, int preload); */
	varfuncvoid_t lua_close{}; /* void lua_close (lua_State *L) */
	varfuncchar_t lua_pushstring{}; /* const char *(lua_pushstring) (lua_State *L, const char *s); */
	varfuncvoid_t lua_pushinteger{}; /*void  (lua_pushinteger)(lua_State * L, lua_Integer n);*/
	varfuncvoid_t lua_pushvalue{}; /* void  (lua_pushvalue) (lua_State *L, int idx);*/
	varfuncvoid_t lua_pushcclosure{}; /* void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n); */
	varfuncvoid_t lua_setfield{}; /* void  (lua_setfield) (lua_State *L, int idx, const char *k); */
	varfuncvoid_t lua_getfield{}; /* void  (lua_getfield) (lua_State *L, int idx, const char *k); */
	varfuncchar_t lua_tolstring{}; /* const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len); */
	varfuncint_t lua_gettop{}; /* int   (lua_gettop) (lua_State *L); */
	varfuncvoid_t lua_settop{}; /* void  (lua_settop) (lua_State *L, int idx);*/
	varfuncchar_t luaL_tolstring{}; /*const char* luaL_tolstring(lua_State* L, int idx, size_t* len)*/
	varfuncint_t lua_type{}; /* int (lua_type) (lua_State *L, int idx);*/
	varfuncchar_t lua_typename{}; /* const char     *(lua_typename) (lua_State *L, int tp); */
	varfuncvoid_t lua_createtable{}; /*void  (lua_createtable) (lua_State *L, int narr, int nrec); */
	varfuncvoid_t lua_rawseti{}; /*void  (lua_rawseti) (lua_State *L, int idx, int n);*/
	varfuncint_t lua_tointeger{}; //Lua5.1 /* lua_Number      (lua_tonumber) (lua_State *L, int idx); */
	varfuncvoid_t lua_call{}; // Lua5.1 /* void  (lua_call) (lua_State *L, int nargs, int nresults);*/
	varfuncint_t lua_pcall{}; // Lua5.1 /*int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc)*/
	varfuncvoid_t lua_setglobal{}; //Lua5.2+ /* void  (lua_setglobal) (lua_State *L, const char *name); */
	varfuncvoid_t lua_getglobal{}; //Lua5.2+ /* void  (lua_getglobal) (lua_State *L, const char *var); */
	varfuncvoid_t lua_callk{}; // Lua5.2+ /* void lua_callk (lua_State *L, int nargs, int nresults, lua_KContext ctx, lua_KFunction k); */
	varfuncint_t lua_tointegerx{}; //Lua5.2+ /* lua_Number      (lua_tonumber) (lua_State *L, int idx); */
	varfuncint_t lua_pcallk{}; // Lua5.2+ /* int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc, lua_KContext ctx, lua_KFunction k); */

public:
	~CLuaManager();
	const wchar_t* GeLuaDllName() const noexcept;
	void init_state();
	void print(void* State);
	int string_to_utf8(void* State);
	int string_from_utf8(void* State);
	void set_textcolor(void* State);
	void list_files(void* State);
	void msgbox(void* State);
	std::tuple<int, std::string> run_file(const char* fpath);
	std::tuple<int, std::string> validate(const char* fpath);
};

void print_from_lua(const char* txt, bool enc = true);
CLuaManager& GetLM();