#include "lua_main.hpp"
#include <sstream>

#define LUA_GLOBALSINDEX	(-10002)	//Lua5.1
#define LUAREG(L, name, func) (lua_pushcclosure(L, (func), 0), do_lua_setglobal((name)))
#define LOAD_FUNCTION(tp, fn) fn = reinterpret_cast<tp>(GetProcAddress(hinstLib, #fn)); if (!fn) AddStr(L"'"#fn"' not loaded\r\n");
#define lua_pop(L,n) lua_settop(L, -(n)-1)

extern CLuaManager* LM;
extern CPluginOptions g_opt;
extern ExecData execData;

void SetConsoleColor(COLORREF color);
COLORREF GetERRColor();
COLORREF GetOKColor();
COLORREF GetNormalColor();

void print_from_lua(const char* txt, bool enc)
{
	const size_t nSize = strlen(txt);
	std::wstring wtmp(nSize, 0);
	SysUniConv::MultiByteToUnicode(wtmp.data(), static_cast<int>(nSize), txt, static_cast<int>(nSize), enc ? (g_opt.m_bConEncoding ? CP_UTF8 : CP_ACP) : CP_ACP);
	AddStr(wtmp.c_str());
}

namespace
{
	int luafunc_msgbox(void*)
	{
		LM->msgbox();
		return 1;
	}

	int luafunc_luahelp(void*)
	{
		print_from_lua(
			R"(lua_help() - print this help
list_files('.\\\\*.*') - return table with list of file names by mask
msgbox('sMessage' , 'sTitle', 'sIconType') - function show messagebox
sMessage: text message
sTitle : caption messagebox
sIconType : any text from { 'ok', 'error' , 'warning', 'question' ,'info', 'yesnocancel', 'yesno' }
return : code of pressed button
)");
		return 0;
	}

	int luafunc_list_files(void*)
	{
		LM->list_files();
		return 1;
	}

	int luafunc_settextcolor(void*)
	{
		LM->set_textcolor();
		return 0;
	}

	int luafunc_set_err_color(void*)
	{
		SetConsoleColor(GetERRColor());
		return 0;
	}

	int luafunc_set_ok_color(void*)
	{
		SetConsoleColor(GetOKColor());
		return 0;
	}

	int luafunc_set_normal_color(void*)
	{
		SetConsoleColor(GetNormalColor());
		return 0;
	}

	int luafunc_print(void*)
	{
		LM->print();
		return 0;
	}
}

void CLuaManager::msgbox()
{
	int res = 0;
	switch (lua_gettop(L))
	{
	case 1:
		res = MessageBoxA(NULL, lua_tolstring(L, 1, nullptr), "", MB_OK);
		break;

	case 2:
		res = MessageBoxA(NULL, lua_tolstring(L, 1, nullptr), lua_tolstring(L, 2, nullptr), MB_OK);
		break;

	case 3:
	{
		const char* icon = lua_tolstring(L, 3, nullptr);
		UINT icon_type = MB_OK;
		if (strstr(icon, "error"))		icon_type |= MB_ICONERROR;
		if (strstr(icon, "warning"))	icon_type |= MB_ICONWARNING;
		if (strstr(icon, "info"))		icon_type |= MB_ICONINFORMATION;
		if (strstr(icon, "question"))	icon_type |= MB_ICONQUESTION;
		if (strstr(icon, "yesnocancel"))icon_type |= MB_YESNOCANCEL;
		else if (strstr(icon, "yesno"))	icon_type |= MB_YESNO;
		res = MessageBoxA(NULL, lua_tolstring(L, 1, nullptr), lua_tolstring(L, 2, nullptr), icon_type);
		break;
	}
	default:
		break;
	}
	lua_pushinteger(L, res);
}

void CLuaManager::list_files()
{
	WIN32_FIND_DATAA f;
	const char* mask = lua_tolstring(L, -1, nullptr);
	lua_createtable(L, 0, 0);
	if (!mask) return;
	HANDLE h = FindFirstFileA(mask, &f);
	if (h != INVALID_HANDLE_VALUE)
	{
		int i = 1;
		do
		{
			lua_pushstring(L, f.cFileName);
			lua_rawseti(L, -2, i++);
		} while (FindNextFileA(h, &f));
	}
}

void CLuaManager::print()
{
	int sz = lua_gettop(L);
	if (!sz) return;
	std::stringstream ss;
	do_lua_getglobal("tostring");
	for (int i = 1; i <= sz; i++)
	{
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		do_lua_pcall(1, 1);
		const char* str = lua_tolstring(L, -1, nullptr);
		if (i > 1) ss << " ";
		if (str)
			ss << str;
		else
			ss << lua_typename(L, lua_type(L, i));
		lua_pop(L, 1);
	}
	ss << '\n';
	print_from_lua(ss.str().c_str());
}

void CLuaManager::set_textcolor()
{
	unsigned int r = 0, g = 0, b = 0;
	const char* clrs = lua_tolstring(L, 1, nullptr);
	if (clrs) sscanf_s(clrs, "#%02x%02x%02x", &r, &g, &b);
	SetConsoleColor(RGB(r, g, b));
}

void CLuaManager::reset_lib(const wchar_t* dll_name, eLuaVersion lib_type)
{
	m_lua_ver = lib_type;
	destroy();

	wchar_t lua_dll[MAX_PATH]{};
	UINT nLen = GetModuleFileName((HMODULE)execData.hNPP, lua_dll, MAX_PATH);
	while (nLen > 0 && lua_dll[nLen] != L'\\') lua_dll[nLen--] = 0;
	lstrcat(lua_dll, dll_name);

	hinstLib = LoadLibrary(lua_dll);
	if (hinstLib)
	{
		//SetCharFormat();
		OnClearConsole();

		LOAD_FUNCTION(voidfunc*, luaL_newstate);
		LOAD_FUNCTION(varfuncvoid*, luaL_openlibs);
		LOAD_FUNCTION(varfuncvoid*, lua_close);
		LOAD_FUNCTION(varfuncchar*, lua_pushstring);
		LOAD_FUNCTION(varfuncvoid*, lua_pushvalue);
		LOAD_FUNCTION(varfuncvoid*, lua_pushinteger);
		LOAD_FUNCTION(varfuncvoid*, lua_pushcclosure);
		LOAD_FUNCTION(varfuncchar*, lua_tolstring);
		LOAD_FUNCTION(varfuncvoid*, lua_setfield);
		LOAD_FUNCTION(varfuncvoid*, lua_getfield);
		LOAD_FUNCTION(varfuncvoid*, lua_settop);
		LOAD_FUNCTION(varfuncint*, lua_gettop);
		LOAD_FUNCTION(varfuncint*, lua_type);
		LOAD_FUNCTION(varfuncchar*, lua_typename);
		LOAD_FUNCTION(varfuncvoid*, lua_createtable);
		LOAD_FUNCTION(varfuncvoid*, lua_rawseti);

		if (m_lua_ver == eLuaVersion::LUA_51)
		{
			LOAD_FUNCTION(varfuncvoid*, lua_call);
			LOAD_FUNCTION(varfuncint*, lua_pcall);
			LOAD_FUNCTION(varfuncint*, lua_tointeger);
		}
		else
		{
			LOAD_FUNCTION(varfuncvoid*, lua_callk);
			LOAD_FUNCTION(varfuncint*, lua_pcallk);
			LOAD_FUNCTION(varfuncint*, lua_tointegerx);
			LOAD_FUNCTION(varfuncvoid*, lua_setglobal);
			LOAD_FUNCTION(varfuncvoid*, lua_getglobal);
		}

		L = luaL_newstate();
		luaL_openlibs(L);

		//init_lua_functions
		LUAREG(L, "msgbox", luafunc_msgbox);
		LUAREG(L, "print", luafunc_print);
		LUAREG(L, "list_files", luafunc_list_files);
		LUAREG(L, "lua_help", luafunc_luahelp);
		LUAREG(L, "set_text_color", luafunc_settextcolor);
		LUAREG(L, "set_err_color", luafunc_set_err_color);
		LUAREG(L, "set_ok_color", luafunc_set_ok_color);
		LUAREG(L, "set_normal_color", luafunc_set_normal_color);
	}
	else
	{
		wchar_t buf[MAX_PATH]{};
		wsprintf(buf, L"Can't load '%s'", lua_dll);
		MessageBox(NULL, buf, L"Error", MB_OK | MB_ICONERROR);
	}
}

CLuaManager::~CLuaManager()
{
	destroy();
}

void CLuaManager::destroy()
{
	if (L)
	{
		lua_close(L);
		L = nullptr;
	}
	if (hinstLib)
	{
		FreeLibrary(hinstLib);
		hinstLib = NULL;
	}
}

void CLuaManager::do_lua_call(int narg, int nret)
{
	if (lua_type(L, -narg - 1) != 6 /*LUA_TFUNCTION*/)
	{
		MessageBox(NULL, L"try to call non function object", L"Error", MB_OK);
		return;
	}
	if (m_lua_ver == eLuaVersion::LUA_51)
		lua_call(L, narg, nret);
	else
		lua_callk(L, narg, nret, 0, NULL);
}

int CLuaManager::do_lua_pcall(int narg, int nret)
{
	if (lua_type(L, -narg - 1) != 6 /*LUA_TFUNCTION*/)
	{
		MessageBox(NULL, L"try to call non function object", L"Error", MB_OK);
		return 0;
	}
	if (m_lua_ver == eLuaVersion::LUA_51)
		return lua_pcall(L, narg, nret, 0);
	else
		return lua_pcallk(L, narg, nret, 0, 0, NULL);
}

void CLuaManager::do_lua_getglobal(const char* name)
{
	if (m_lua_ver == eLuaVersion::LUA_51)
		lua_getfield(L, LUA_GLOBALSINDEX, name);
	else
		lua_getglobal(L, name);
}

void CLuaManager::do_lua_setglobal(const char* name)
{
	if (m_lua_ver == eLuaVersion::LUA_51)
		lua_setfield(L, LUA_GLOBALSINDEX, name);
	else
		lua_setglobal(L, name);
}

int CLuaManager::do_lua_tointeger(int idx)
{
	if (m_lua_ver == eLuaVersion::LUA_51)
		return lua_tointeger(L, idx);
	else
		return lua_tointegerx(L, idx, 0);
}

std::tuple<int, std::string> CLuaManager::run_file(const char* fpath)
{
	if (!L)
	{
		return { -1, "Error:Lua not initialized!" };
	}
	const char lua_code[] = "\
return function(quote, fpath) \
if quote>0 then \
	local tm = os.clock() \
	local function hook() \
		if os.clock() - tm > quote then \
			debug.sethook() \
			error('quote in '..quote..' seconds exceeded', 2) \
		end \
	end \
	debug.sethook(hook, '', 1000000) \
end \
local res, err = pcall(dofile,fpath) \
if quote>0 then debug.sethook() end \
if res then return -1, '' end \
return (tonumber(err:match(':(%d+):')) or 1) - 1, err \
end";
	lua_settop(L, 0);
	do_lua_getglobal((m_lua_ver == eLuaVersion::LUA_51) ? "loadstring" : "load");
	lua_pushstring(L, lua_code);
	do_lua_call(1, 1); // OK -> function + nil, not OK -> nil + errstring
	do_lua_call(0, 1); //
	lua_pushinteger(L, g_opt.timequote);
	lua_pushstring(L, fpath);
	do_lua_pcall(2, 2); // call created function (2 arguments, 2 results)
	int res = do_lua_tointeger(1);
	std::string err = lua_tolstring(L, 2, nullptr);
	lua_settop(L, 0);
	return { res, err };
}


std::tuple<int, std::string> CLuaManager::validate(const char* fpath)
{
	if (!L) {
		return { 0 , "Error:Lua not initialized!" };
	}

	const char lua_code[] = "\
return function(fpath) \
local res, err = loadfile(fpath) \
if res then \
	return -1, fpath .. ' - syntax OK\\n' \
end \
return (tonumber(err:match(':(%d+):')) or 1) - 1, err..'\\n' \
end";

	lua_settop(L, 0);
	do_lua_getglobal((m_lua_ver == eLuaVersion::LUA_51) ? "loadstring" : "load");
	lua_pushstring(L, lua_code);
	//dump_stack();
	do_lua_call(1, 1); // OK -> function + nil, not OK -> nil + errstring
	//dump_stack();
	if (lua_type(L, 1) != 6) /* is not a function */
	{
		auto err_info = std::format("internal error (not a function): {}", lua_tolstring(L, -1, nullptr));
		lua_settop(L, 0);
		return { 0, err_info };
	}
	do_lua_call(0, 1); // create lua function
	lua_pushstring(L, fpath);
	do_lua_pcall(1, 2); // call created function
	int line = do_lua_tointeger(1);
	std::string err_info = lua_tolstring(L, 2, nullptr);
	return { line, err_info };
}

void CLuaManager::dump_stack()
{
	//#define DUMPSTACK
#ifdef DUMPSTACK
	if (!L) return;
	int sz = lua_gettop(L);
	if (!sz) return;
	print_from_lua("\r\n==== dump start ====");
	do_lua_getglobal(L, "tostring");
	for (int i = 1; i <= sz; i++)
	{
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		do_lua_pcall(L, 1, 1);
		const char* str = lua_tolstring(L, -1, nullptr);
		print_from_lua("\r\n");
		if (str)
			print_from_lua(str);
		else
		{
			print_from_lua(lua_typename(L, lua_type(L, i)));
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	print_from_lua("\r\n===== dump end ====\r\n");
#endif
}
