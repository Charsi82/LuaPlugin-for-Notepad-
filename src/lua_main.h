#pragma once
#include "PluginOptions.h"

// addtional menu item for test
//#define TEST_ITEM

typedef void* voidfunc();
//typedef int *varfunc(void *L, ...); // quick and dirty using the varargs
typedef char* varfuncchar(void* L, ...);
typedef int varfuncint(void* L, ...);
typedef void varfuncvoid(void* L, ...);
//void print_from_lua(const char*);

class CLuaManager
{
private:
	HMODULE hinstLib;
	BYTE intrp_type;
	void m_lua_call(void* L, int narg, int nret);
	int m_lua_pcall(void* L, int narg, int nret);
	void m_lua_getglobal(void* L, const char* name);
	void m_lua_setglobal(void* L, const char* name);
	int m_lua_tointeger(void* L, int idx);
	//		void dump_stack();
	void* L; // Lua_State

	voidfunc* luaL_newstate{}; /* lua_State *luaL_newstate (void) */
	varfuncvoid* luaL_openlibs{}; /* void (luaL_openlibs) (lua_State *L); */
	varfuncvoid* lua_close{}; /* void lua_close (lua_State *L) */
	varfuncchar* lua_pushstring{}; /* const char *(lua_pushstring) (lua_State *L, const char *s); */
	varfuncvoid* lua_pushinteger{}; /*void  (lua_pushinteger)(lua_State * L, lua_Integer n);*/
	varfuncvoid* lua_pushvalue{}; /* void  (lua_pushvalue) (lua_State *L, int idx);*/
	varfuncvoid* lua_pushcclosure{}; /* void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n); */
	varfuncvoid* lua_setfield{}; /* void  (lua_setfield) (lua_State *L, int idx, const char *k); */
	varfuncvoid* lua_getfield{}; /* void  (lua_getfield) (lua_State *L, int idx, const char *k); */
	varfuncchar* lua_tolstring{}; /* const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len); */
	varfuncint* lua_gettop{}; /* int   (lua_gettop) (lua_State *L); */
	varfuncvoid* lua_settop{}; /* void  (lua_settop) (lua_State *L, int idx);*/
	varfuncchar* luaL_tolstring{}; /*const char* luaL_tolstring(lua_State* L, int idx, size_t* len)*/
	varfuncint* lua_type{}; /* int (lua_type) (lua_State *L, int idx);*/
	varfuncchar* lua_typename{}; /* const char     *(lua_typename) (lua_State *L, int tp); */
	varfuncvoid* lua_createtable{}; /*void  (lua_createtable) (lua_State *L, int narr, int nrec); */
	varfuncvoid* lua_rawseti{}; /*void  (lua_rawseti) (lua_State *L, int idx, int n);*/
	varfuncint* lua_tointeger{}; //Lua5.1 /* lua_Number      (lua_tonumber) (lua_State *L, int idx); */
	varfuncvoid* lua_call{}; // Lua5.1 /* void  (lua_call) (lua_State *L, int nargs, int nresults);*/
	varfuncint* lua_pcall{}; // Lua5.1 /*int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc)*/
	varfuncvoid* lua_setglobal{}; //Lua5.2+ /* void  (lua_setglobal) (lua_State *L, const char *name); */
	varfuncvoid* lua_getglobal{}; //Lua5.2+ /* void  (lua_getglobal) (lua_State *L, const char *var); */
	varfuncvoid* lua_callk{}; // Lua5.2+ /* void lua_callk (lua_State *L, int nargs, int nresults, lua_KContext ctx, lua_KFunction k); */
	varfuncint* lua_tointegerx{}; //Lua5.2+ /* lua_Number      (lua_tonumber) (lua_State *L, int idx); */
	varfuncint* lua_pcallk{}; // Lua5.2+ /* int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc, lua_KContext ctx, lua_KFunction k); */

public:
	CLuaManager() :hinstLib(NULL), L(NULL), intrp_type(0) {};
	~CLuaManager();
	void reset_lib(const TCHAR*, BYTE);
	void print();
	void set_textcolor();
	void list_files();
	void msgbox();
	int process(const char*, bool = false);
};

enum enumNFuncItems
{
	CheckSyntax = 0,
	RunScript,
	CheckFiles,
	RunLove2D,
	Separator1,
	LUA51,
	LUA52,
	LUA53,
	LUA54,
	LUAJIT,
	Separator2,
	ShowHideConsole,
//	ClearConsole,
//	AutoClear,
//	PrintTime,
	Separator3,
//	SwitchLang,
	Options,
	About,
#ifdef TEST_ITEM
	TestItem,
#endif
	nbFunc
};

static TCHAR* loc_ru[nbFunc] =
{
	L"Проверка синтаксиса",
	L"Запуск скрипта",
	L"Проверить все файлы в папке",
	L"Запуск Love2D",
	L"-",
	L"Lua51",
	L"Lua52",
	L"Lua53",
	L"Lua54",
	L"LuaJIT",
	L"-",
	L"Показать\\cкрыть консоль",
//	L"Очистить консоль",
//	L"Автоочистка консоли",
//	L"Время выполнения",
	L"-",
//	L"Сменить язык(RU\\ENG)",
	L"Опции",
	L"О плагине",
#ifdef TEST_ITEM
	L"тест",
#endif
};

static TCHAR* loc_en[nbFunc] =
{
	L"Verify syntax",
	L"Run script",
	L"Check all files in folder",
	L"Run Love2D",
	L"-",
	L"Lua51",
	L"Lua52",
	L"Lua53",
	L"Lua54",
	L"LuaJIT",
	L"-",
	L"Show\\hide Console",
//	L"Clear Console",
//	L"Autoclean Console",
//	L"Print run time",
	L"-",
//	L"Change language(ENG\\RU)",
	L"Options",
	L"About",
#ifdef TEST_ITEM
	L"test",
#endif
};
