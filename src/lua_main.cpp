#include "lua_main.hpp"
#include "DarkModeSupport.hpp"
#include <sstream>
#include <filesystem>

#define LUA_GLOBALSINDEX	(-10002)	//Lua5.1
#define LUAREG(LState, name, func) (lua_pushcclosure(L, (func), 0), do_lua_setglobal(LState, (name)))
#define LOAD_FUNCTION(tp, fn) fn = reinterpret_cast<tp>(GetProcAddress(hinstLib, #fn)); if (!fn) { AddStr(L"'"#fn"' not loaded\r\n"); return; }
#define lua_pop(L, n) lua_settop(L, -(n)-1)

//extern CLuaManager* LM;
extern CPluginOptions g_opt;
extern ExecData execData;

void SetConsoleColor(COLORREF color);

namespace
{
	std::wstring StringConvert(const char* txt, bool enc)
	{
		return SysUniConv::MultibyteToWideChar(txt, (enc && g_opt.m_bConEncoding) ? CP_UTF8 : CP_ACP);
	}

	int luafunc_msgbox(void* State)
	{
		GetLM().msgbox(State);
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

	int luafunc_list_files(void* State)
	{
		GetLM().list_files(State);
		return 1;
	}

	int luafunc_settextcolor(void* State)
	{
		GetLM().set_textcolor(State);
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

	int luafunc_print(void* State)
	{
		GetLM().print(State);
		return 0;
	}

	int luafunc_string_to_utf8(void* State)
	{
		return GetLM().string_to_utf8(State);
	}

	int luafunc_string_from_utf8(void* State)
	{
		return GetLM().string_from_utf8(State);
	}
}

void print_from_lua(const char* txt, bool enc)
{
	AddStr(StringConvert(txt, enc).c_str());
}

void CLuaManager::msgbox(void* State)
{
	int res = 0;
	switch (lua_gettop(State))
	{
	case 1:
	{
		std::wstring text = StringConvert(lua_tolstring(State, 1, nullptr), true);
		res = MessageBox(NULL, text.c_str(), L"", MB_OK);
		break;
	}

	case 2:
	{
		std::wstring text = StringConvert(lua_tolstring(State, 1, nullptr), true);
		std::wstring caption = StringConvert(lua_tolstring(State, 2, nullptr), true);
		res = MessageBox(NULL, text.c_str(), caption.c_str(), MB_OK);
		break;
	}

	case 3:
	{
		const char* icon = lua_tolstring(State, 3, nullptr);
		UINT icon_type = MB_OK;
		if (strstr(icon, "error"))		icon_type |= MB_ICONERROR;
		if (strstr(icon, "warning"))	icon_type |= MB_ICONWARNING;
		if (strstr(icon, "info"))		icon_type |= MB_ICONINFORMATION;
		if (strstr(icon, "question"))	icon_type |= MB_ICONQUESTION;
		if (strstr(icon, "yesnocancel"))icon_type |= MB_YESNOCANCEL;
		else if (strstr(icon, "yesno"))	icon_type |= MB_YESNO;

		std::wstring text = StringConvert(lua_tolstring(State, 1, nullptr), true);
		std::wstring caption = StringConvert(lua_tolstring(State, 2, nullptr), true);
		res = MessageBox(NULL, text.c_str(), caption.c_str(), icon_type);
		break;
	}
	default:
		break;
	}
	lua_pushinteger(State, res);
}

void CLuaManager::list_files(void* State)
{
	const char* mask = lua_tolstring(State, -1, nullptr);
	lua_createtable(State, 0, 0);
	if (!mask) return;
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(mask, &f);
	if (h != INVALID_HANDLE_VALUE)
	{
		int i = 1;
		do
		{
			lua_pushstring(State, f.cFileName);
			lua_rawseti(State, -2, i++);
		} while (FindNextFileA(h, &f));
	}
}

int CLuaManager::string_to_utf8(void* State)
{
	if (const char* text = lua_tolstring(State, 1, nullptr))
	{
		std::string utf8 = SysUniConv::AnsiToUtf8(text);
		lua_pushstring(State, utf8.c_str());
		return 1;
	}
	return 0;
}

int CLuaManager::string_from_utf8(void* State)
{
	if (const char* text = lua_tolstring(State, 1, nullptr))
	{
		std::string ansi = SysUniConv::Utf8ToAnsi(text);
		lua_pushstring(State, ansi.c_str());
		return 1;
	}
	return 0;
}

void CLuaManager::print(void* State)
{
	int sz = lua_gettop(State);
	if (!sz) return;
	std::stringstream ss;
	do_lua_getglobal(State, "tostring");
	for (int i = 1; i <= sz; i++)
	{
		lua_pushvalue(State, -1);
		lua_pushvalue(State, i);
		do_lua_pcall(State, 1, 1);
		const char* str = lua_tolstring(State, -1, nullptr);
		if (i > 1) ss << " ";
		if (str)
			ss << str;
		else
			ss << lua_typename(State, lua_type(State, i));
		lua_pop(State, 1);
	}
	ss << '\n';
	print_from_lua(ss.str().c_str());
}

void CLuaManager::set_textcolor(void* State)
{
	unsigned int r{}, g{}, b{};
	const char* clrs = lua_tolstring(State, 1, nullptr);
	if (clrs && sscanf_s(clrs, "#%02x%02x%02x", &r, &g, &b) > 0)
		SetConsoleColor(RGB(r, g, b));
	else
		SetConsoleColor(GetNormalColor());
}

const wchar_t* CLuaManager::GeLuaDllName() const noexcept
{
	switch (g_opt.GetLuaType())
	{
	case eLuaVersion::LUA54:
		return L"Lua54";

	case eLuaVersion::LUA53:
		return L"Lua53";

	case eLuaVersion::LUA52:
		return L"Lua52";

	case eLuaVersion::LUA55:
		return L"Lua55";

	case eLuaVersion::LUAPLUTO:
		return L"Pluto";

	case eLuaVersion::LUAJIT:
		return L"LuaJIT";

	case eLuaVersion::LUA51:
	default:
		return L"Lua51";
	}
}

//void CLuaManager::SetVersion(eLuaVersion ver)
//{
//	//m_lua_ver = ver;
//	g_opt.SetLuaType((BYTE)ver);
//}

void CLuaManager::init_state()
{
	std::wstring dll_path = std::format(L".\\plugins\\" PROJECT_NAME "\\{}.dll", GeLuaDllName());
	hinstLib = LoadLibrary(dll_path.c_str());
	if (!hinstLib)
	{
		wchar_t buf[MAX_PATH]{};
		wsprintf(buf, L"Can't load '%s'", dll_path.c_str());
		MessageBox(NULL, buf, L"Error", MB_OK | MB_ICONERROR);
	}
}

void CLuaManager::reset_state()
{
	destroy_state();
	init_state();
	if (!hinstLib) return;

	LOAD_FUNCTION(voidfunc_t, luaL_newstate);
	LOAD_FUNCTION(varfuncvoid_t, lua_close);
	LOAD_FUNCTION(varfuncchar_t, lua_pushstring);
	LOAD_FUNCTION(varfuncvoid_t, lua_pushvalue);
	LOAD_FUNCTION(varfuncvoid_t, lua_pushinteger);
	LOAD_FUNCTION(varfuncvoid_t, lua_pushcclosure);
	LOAD_FUNCTION(varfuncchar_t, lua_tolstring);
	LOAD_FUNCTION(varfuncvoid_t, lua_setfield);
	LOAD_FUNCTION(varfuncvoid_t, lua_getfield);
	LOAD_FUNCTION(varfuncvoid_t, lua_settop);
	LOAD_FUNCTION(varfuncint_t, lua_gettop);
	LOAD_FUNCTION(varfuncint_t, lua_type);
	LOAD_FUNCTION(varfuncchar_t, lua_typename);
	LOAD_FUNCTION(varfuncvoid_t, lua_createtable);
	LOAD_FUNCTION(varfuncvoid_t, lua_rawseti);

	if (isLua55())
	{
		LOAD_FUNCTION(varfuncvoid_t, luaL_openselectedlibs);
	}
	else
	{
		LOAD_FUNCTION(varfuncvoid_t, luaL_openlibs);
	}
	if (isLua51())
	{
		LOAD_FUNCTION(varfuncvoid_t, lua_call);
		LOAD_FUNCTION(varfuncint_t, lua_pcall);
		LOAD_FUNCTION(varfuncint_t, lua_tointeger);
	}
	else
	{
		LOAD_FUNCTION(varfuncvoid_t, lua_callk);
		LOAD_FUNCTION(varfuncint_t, lua_pcallk);
		LOAD_FUNCTION(varfuncint_t, lua_tointegerx);
		LOAD_FUNCTION(varfuncvoid_t, lua_setglobal);
		LOAD_FUNCTION(varfuncvoid_t, lua_getglobal);
	}

	L = luaL_newstate();
	do_load_libs(L); //luaL_openlibs(L);

	//init_lua_functions
	LUAREG(L, "msgbox", luafunc_msgbox);
	LUAREG(L, "print", luafunc_print);
	LUAREG(L, "list_files", luafunc_list_files);
	LUAREG(L, "lua_help", luafunc_luahelp);
	LUAREG(L, "set_text_color", luafunc_settextcolor);
	LUAREG(L, "set_err_color", luafunc_set_err_color);
	LUAREG(L, "set_ok_color", luafunc_set_ok_color);
	LUAREG(L, "set_normal_color", luafunc_set_normal_color);
	do_lua_getglobal(L, "string");
	lua_pushcclosure(L, luafunc_string_to_utf8, 0);
	lua_setfield(L, -2, "to_utf8");
	lua_pushcclosure(L, luafunc_string_from_utf8, 0);
	lua_setfield(L, -2, "from_utf8");
	lua_pop(L, 1);
}

bool CLuaManager::isLua51() const noexcept
{
	return g_opt.GetLuaType() == eLuaVersion::LUA51 || g_opt.GetLuaType() == eLuaVersion::LUAJIT;
}

bool CLuaManager::isLua55() const noexcept
{
	// pluto since 0.12.0 work with luaL_openselectedlibs
	return g_opt.GetLuaType() == eLuaVersion::LUA55 || g_opt.GetLuaType() == eLuaVersion::LUAPLUTO;
}

CLuaManager::~CLuaManager()
{
	destroy_state();
}

void CLuaManager::destroy_state()
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

bool CLuaManager::isFunction(void* State, int idx, const char* msg) const
{
	if (lua_type(State, idx) == 6 /*LUA_TFUNCTION*/) return true;
	print_from_lua(msg, false);
	return false;
}

void CLuaManager::do_load_libs(void* State)
{
	if (isLua55())
		luaL_openselectedlibs(State, ~0, 0);
	else
		luaL_openlibs(State);
}

void CLuaManager::do_lua_call(void* State, int narg, int nret)
{
	if (!isFunction(State, -narg - 1, "try to call non function object")) return;
	if (isLua51())
		lua_call(State, narg, nret);
	else
		lua_callk(State, narg, nret, 0, NULL);
}

int CLuaManager::do_lua_pcall(void* State, int narg, int nret)
{
	if (!isFunction(State, -narg - 1, "try to pcall non function object")) return 0;
	if (isLua51())
		return lua_pcall(State, narg, nret, 0);
	else
		return lua_pcallk(State, narg, nret, 0, 0, NULL);
}

void CLuaManager::do_lua_getglobal(void* State, const char* name)
{
	if (isLua51())
		lua_getfield(State, LUA_GLOBALSINDEX, name);
	else
		lua_getglobal(State, name);
}

void CLuaManager::do_lua_setglobal(void* State, const char* name)
{
	if (isLua51())
		lua_setfield(State, LUA_GLOBALSINDEX, name);
	else
		lua_setglobal(State, name);
}

int CLuaManager::do_lua_tointeger(void* State, int idx)
{
	if (isLua51())
		return lua_tointeger(State, idx);
	else
		return lua_tointegerx(State, idx, 0);
}

std::tuple<int, std::string> CLuaManager::run_file(const char* fpath)
{
	reset_state();
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
	do_lua_getglobal(L, isLua51() ? "loadstring" : "load");
	lua_pushstring(L, lua_code);
	do_lua_call(L, 1, 1); // OK -> function + nil, not OK -> nil + errstring
	do_lua_call(L, 0, 1); //
	lua_pushinteger(L, g_opt.m_timequote);
	lua_pushstring(L, fpath);
	do_lua_pcall(L, 2, 2); // call created function (2 arguments, 2 results)
	int res = do_lua_tointeger(L, 1);
	std::string err = lua_tolstring(L, 2, nullptr);
	lua_settop(L, 0);
	return { res, err };
}

std::tuple<int, std::string> CLuaManager::validate(const char* fpath)
{
	reset_state();
	if (!L) {
		return { 0 , "Error:Lua not initialized!" };
	}

	//	const char lua_code[] = "\
	//return function(fpath) \
	//local res, err = loadfile(fpath) \
	//if res then \
	//	return -1, fpath .. ' - syntax OK\\n' \
	//end \
	//return (tonumber(err:match(':(%d+):')) or 1) - 1, err..'\\n' \
	//end";
	//
	//	do_lua_getglobal(L, isLua51() ? "loadstring" : "load");
	//	lua_pushstring(L, lua_code);
	//	//dump_stack(L);
	//	do_lua_call(L, 1, 1); // OK -> function + nil, not OK -> nil + errstring
	//	//dump_stack(L);
	//	if (lua_type(L, 1) != 6) /* is not a function */
	//	{
	//		auto err_info = std::format("internal error (not a function): {}", lua_tolstring(L, -1, nullptr));
	//		lua_settop(L, 0);
	//		return { 0, err_info };
	//	}
	//	do_lua_call(L, 0, 1); // create lua function
	//	lua_pushstring(L, fpath);
		//dump_stack(L);
	do_lua_getglobal(L, isLua51() ? "loadstring" : "load");
	const char lua_code[] = "local args = {...} \
		local fpath = args[1] \
		local res, err = loadfile(fpath)\
		if res then \
			return -1, fpath .. ' - syntax OK\\n' \
		end \
		return (tonumber(err:match(':(%d+):')) or 1) - 1, err..'\\n'";
	lua_pushstring(L, lua_code);
	//dump_stack(L);
	do_lua_call(L, 1, 1);
	//dump_stack(L);
	if (lua_type(L, 1) != 6) /* is not a function */
	{
		auto err_info = std::format("internal error (not a function): {}", lua_tolstring(L, -1, nullptr));
		lua_settop(L, 0);
		return { 0, err_info };
	}
	lua_pushstring(L, fpath);
	//dump_stack(L);
	do_lua_pcall(L, 1, 2); // call created function
	//dump_stack(L);
	int line = do_lua_tointeger(L, 1);
	std::string err_info = lua_tolstring(L, 2, nullptr);
	lua_settop(L, 0);
	return { line, err_info };
}

void CLuaManager::dump_stack([[maybe_unused]] void* State)
{
	//#define DUMPSTACK
#ifdef DUMPSTACK
	if (!State) return;
	int sz = lua_gettop(State);
	if (!sz) return;
	print_from_lua("\r\n==== dump start ====");
	do_lua_getglobal(State, "tostring");
	for (int i = 1; i <= sz; i++)
	{
		lua_pushvalue(State, -1);
		lua_pushvalue(State, i);
		do_lua_pcall(State, 1, 1);
		const char* str = lua_tolstring(State, -1, nullptr);
		print_from_lua("\r\n");
		if (str)
		{
			print_from_lua(str);
		}
		else
		{
			print_from_lua(lua_typename(State, lua_type(State, i)));
		}
		lua_pop(State, 1);
	}
	lua_pop(State, 1);
	print_from_lua("\r\n===== dump end ====\r\n");
#endif
}
