#include <Richedit.h>
//#include <shlwapi.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <CommCtrl.h>
#include "resource.h"
#include "lua_main.hpp"
#include "DarkModeSupport.hpp"
#include "Localize.hpp"

//#pragma comment (lib, "shlwapi") // PathFileExist

#define SendNpp(msg, wParam, lParam) SendMessage(nppData._nppHandle, (msg), (WPARAM)wParam, (LPARAM)lParam)																		
#define AppMenu(ID) AppendMenu(menu, MF_ENABLED, (ID), Localize((ID), lng))
#define LocalizeDlgItem(ID) SetDlgItemText(hw, (ID), Localize((ID), lng))

__declspec(dllimport)
std::tuple<std::string, std::string> autoformat(std::string input, const std::vector<std::string>& v_options);

///////////////////
// Plugin Variables
///////////////////

NppData nppData;
ExecData execData;
CPluginOptions g_opt;
bool bNeedClear = false;
tTbData dockingData;
HMODULE m_hRichEditDll;
CLuaManager* LM;

const wchar_t* path_lua51 = L"lua51.dll";
const wchar_t* path_lua52 = L"lua52.dll";
const wchar_t* path_lua53 = L"lua53.dll";
const wchar_t* path_lua54 = L"lua54.dll";
const wchar_t* path_pluto = L"pluto.dll";
const wchar_t* path_luajit = L"luajit.dll";

static bool is_floating = true;
static bool clr_choose_shown = false;
static COLORREF current_color = 0;
HWND g_hOptionDlg = NULL;

ShortcutKey sk1{ false, true, false, 'S' };// alt + S
ShortcutKey sk2{ false, false, false, VK_F5 };// F5
ShortcutKey sk3{ false, true, false, 'F' };// alt + F

enum class ColorMode
{
	NORMAL,
	OK,
	ERR
};

FuncItem funcItems[nbFunc];

void InitFuncItem(int nItem, const wchar_t* szName, PFUNCPLUGINCMD pFunc, ShortcutKey* pShortcut)
{
	funcItems[nItem]._pFunc = pFunc;
	funcItems[nItem]._init2Check = false; //bCheck;
	funcItems[nItem]._pShKey = pShortcut;
	lstrcpy(funcItems[nItem]._itemName, szName);
}

HWND GetCurrentScintilla()
{
	int currentView = 0;
	SendNpp(NPPM_GETCURRENTSCINTILLA, 0, &currentView);
	return (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

LRESULT SendSci(UINT iMessage, WPARAM wParam, LPARAM lParam) noexcept
{
	return SendMessage(GetCurrentScintilla(), iMessage, wParam, lParam);
}

void SetConsoleColor(COLORREF color)
{
	current_color = color;
}

namespace
{
	bool isFileExists(const wchar_t* path)
	{
		return std::filesystem::exists(path);
	}


	void PrintError(const wchar_t* str)
	{
		OnClearConsole();
		if (!g_opt.m_bConsoleOpen) OpenCloseConsole();
		SetConsoleColor(GetERRColor());
		AddStr(str);
	}

	void OnLove2D()
	{
		// check love2d path
		//if (!PathFileExists(g_opt.LovePath))
		if (!isFileExists(g_opt.LovePath))
		{
			const std::wstring tmp = std::format(L"Can't find file '{}'.", g_opt.LovePath);
			PrintError(tmp.c_str());
			EnableMenuItem(execData.hMenu, funcItems[RunLove2D]._cmdID, MF_BYCOMMAND | MF_DISABLED);
			return;
		}

		wchar_t current_path[MAX_PATH]{};
		SendNpp(NPPM_GETCURRENTDIRECTORY, MAX_PATH, current_path);
		std::wstring scurrent_path{ current_path };

		// check main.lua in current directory
		const std::wstring main_path = scurrent_path + L"\\main.lua";
		//if (!PathFileExists(main_path.c_str()))
		if (!isFileExists(main_path.c_str()))
		{
			PrintError(L"File 'main.lua' not found in current directory!");
			return;
		}

		// run script
		auto shell_execute = [love_path = g_opt.LovePath, main_path = scurrent_path](/*const wchar_t* _command, const wchar_t* _args*/)
			{
				SHELLEXECUTEINFO ShExecInfo{};
				ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				ShExecInfo.hwnd = NULL;
				ShExecInfo.lpVerb = L"open";
				ShExecInfo.lpFile = love_path; // _command;
				ShExecInfo.lpParameters = main_path.c_str(); // _args;
				ShExecInfo.lpDirectory = NULL;
				ShExecInfo.nShow = SW_NORMAL;
				ShExecInfo.hInstApp = NULL;

				ShellExecuteEx(&ShExecInfo);
				if (!ShExecInfo.hProcess)
				{
					// throw exception
					//throw GetLastErrorAsString(GetLastError());
					return; // GetLastError();
				}

				WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
				CloseHandle(ShExecInfo.hProcess);
			};
		std::thread(shell_execute).detach();
	}

	HWND GetConsole() noexcept
	{
		return GetDlgItem(execData.hConsole, IDC_RICHEDIT21);
	}

	void ResetConsoleColors() noexcept
	{
		SendMessage(GetConsole(), EM_SETBKGNDCOLOR, 0, GetBackGroundColor());
		SetConsoleColor(GetNormalColor());
	}

	const wchar_t* GetAddInfo()	noexcept
	{
		switch (g_opt.m_uInterpType)
		{
		case LUA54:
			return L"Lua54";

		case LUA53:
			return L"Lua53";

		case LUA52:
			return L"Lua52";

		case LUAPLUTO:
			return L"Pluto";

		case LUAJIT:
			return L"LuaJIT";

		case LUA51:
		default:
			return L"Lua51";
		}
	}

	void ResetItemCheck()
	{
		for (int i = LUA51; i <= LUAJIT; ++i)
			SendNpp(NPPM_SETMENUITEMCHECK, funcItems[i]._cmdID, g_opt.m_uInterpType == i);
		wcscpy_s(execData.addInfo, GetAddInfo());
		if (g_opt.m_bConsoleOpen)
			SendNpp(NPPM_DMMUPDATEDISPINFO, 0, execData.hConsole);
	}

	void OnLua51()
	{
		g_opt.m_uInterpType = LUA51;
		ResetItemCheck();
		LM->reset_state(path_lua51, eLuaVersion::LUA_51);
	}

	void OnLua52()
	{
		g_opt.m_uInterpType = LUA52;
		ResetItemCheck();
		LM->reset_state(path_lua52, eLuaVersion::LUA_52);
	}

	void OnLua53()
	{
		g_opt.m_uInterpType = LUA53;
		ResetItemCheck();
		LM->reset_state(path_lua53, eLuaVersion::LUA_53);
	}

	void OnLua54()
	{
		g_opt.m_uInterpType = LUA54;
		ResetItemCheck();
		LM->reset_state(path_lua54, eLuaVersion::LUA_54);
	}

	void OnLuaPluto()
	{
		g_opt.m_uInterpType = LUAPLUTO;
		ResetItemCheck();
		LM->reset_state(path_pluto, eLuaVersion::LUA_54);
	}

	void OnLuaJIT()
	{
		g_opt.m_uInterpType = LUAJIT;
		ResetItemCheck();
		LM->reset_state(path_luajit, eLuaVersion::LUA_51); // luajit based on Lua5.1
	}

	void ApplyLang()
	{
		MENUITEMINFO info{};
		info.cbSize = sizeof(MENUITEMINFO);
		info.fType = MFT_STRING;
		info.fMask = MIIM_TYPE;

		wchar_t buf[menuItemSize]{};
		info.dwTypeData = buf;
		GetMenuString(execData.hMenu, funcItems[About]._cmdID, buf, menuItemSize, FALSE);
		if (lstrcmp(buf, Localize(About, g_opt.GetLang())) == 0) { return; }
		wchar_t hot_key[menuItemSize]{};
		for (int i = 0; i < nbFunc; i++)
			if (funcItems[i]._pFunc)
			{
				GetMenuString(execData.hMenu, funcItems[i]._cmdID, buf, menuItemSize, FALSE);
				hot_key[0] = 0;
				wchar_t* hk = wcschr(buf, L'\t');
				if (hk) lstrcpy(hot_key, hk);
				wsprintf(buf, L"%s%s", Localize(i, g_opt.GetLang()), hot_key);
				SetMenuItemInfo(execData.hMenu, funcItems[i]._cmdID, FALSE, &info);
			}
	}

	void AddMarker(int line)
	{
		SendSci(SCI_GOTOLINE, line);
		//SendSci(SCI_SETEMPTYSELECTION);
		const int marker_type = SC_MARK_CIRCLEMINUS; /* SC_MARK_ARROWS */
		SendSci(SCI_MARKERDELETE, line, marker_type); // remove mark if added
		SendSci(SCI_MARKERADD, line, marker_type);
		SetFocus(GetCurrentScintilla());
	}

	bool isValidDocument(const wchar_t* ws_full_path)
	{
		// save file
		SendNpp(NPPM_SAVECURRENTFILE, 0, 0);
		if (ws_full_path[1] != ':' && ws_full_path[2] != '\\') return false;
		// get language type
		LangType docType = LangType::L_EXTERNAL;
		SendNpp(NPPM_GETCURRENTLANGTYPE, 0, &docType);
		return docType == LangType::L_LUA;
	}

	void OnCheckSyntax()
	{
		wchar_t ws_full_path[MAX_PATH]{};
		SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path);
		if (!isValidDocument(ws_full_path)) return;

		OnClearConsole();
		if (!g_opt.m_bConsoleOpen) OpenCloseConsole();

		char full_filepath[MAX_PATH];
		SysUniConv::UnicodeToMultiByte(full_filepath, MAX_PATH, ws_full_path);

		auto [line, res] = LM->validate(full_filepath);
		bool isErr = line > -1;
		if (isErr) AddMarker(line);
		SetConsoleColor((isErr) ? GetERRColor() : GetOKColor());
		print_from_lua(res.c_str(), false);

		bNeedClear = true;
	}

	void OnRunScriptAsync()
	{
		wchar_t ws_full_path[MAX_PATH]{};
		SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path);
		if (!isValidDocument(ws_full_path)) return;

		if (g_opt.m_bConsoleAutoclear || bNeedClear) OnClearConsole();
		if (!g_opt.m_bConsoleOpen) OpenCloseConsole();

		char full_path[MAX_PATH]{};
		SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);

		auto [val_line, val_res] = LM->validate(full_path);
		if (val_line > -1)
		{
			AddMarker(val_line);
			SetConsoleColor(GetERRColor());
			print_from_lua(val_res.c_str(), false);
			return;
		}

		auto start = std::chrono::steady_clock::now();
		auto [run_line, run_res] = LM->run_file(full_path);
		auto end = std::chrono::steady_clock::now();

		bNeedClear = true;
		if (run_line >= 0) // runtime error
		{
			SetConsoleColor(GetERRColor());
			AddMarker(run_line);
			print_from_lua(run_res.c_str(), false);
		}
		else
		{
			SYSTEMTIME sTime;
			GetLocalTime(&sTime);

			SetConsoleColor(GetNormalColor());
			std::wstring wtmp = std::format(L"\nSuccess: {:02}:{:02}:{:02}", sTime.wHour, sTime.wMinute, sTime.wSecond);
			AddStr(wtmp.c_str());

			if (g_opt.m_bShowRunTime)
			{
				std::chrono::duration<double, std::milli> duration{ end - start };
				wtmp = std::format(L"\nDuration {:.3f} ms", duration.count());
				AddStr(wtmp.c_str());
			}
			bNeedClear = false;
		}
	}

	void OnRunScript()
	{
		std::thread(OnRunScriptAsync).detach();
	}

	void OnCheckFilesAsync()
	{
		wchar_t ws_full_path[MAX_PATH]{};
		SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path); // full file path
		if (!isValidDocument(ws_full_path)) return;

		OnClearConsole();
		if (!g_opt.m_bConsoleOpen) OpenCloseConsole();

		wchar_t f_ext[MAX_PATH]{};
		SendNpp(NPPM_GETEXTPART, MAX_PATH, f_ext);
		wchar_t ws_pattern[MAX_PATH]{};
		SendNpp(NPPM_GETCURRENTDIRECTORY, MAX_PATH, ws_full_path); // current dir

		wsprintf(ws_pattern, L"%s\\*%s", ws_full_path, f_ext);
		WIN32_FIND_DATA f;
		HANDLE h = FindFirstFile(ws_pattern, &f);
		if (h != INVALID_HANDLE_VALUE)
		{
			int file_cnt = 0;
			int errf_cnt = 0;
			wchar_t ws_full_filepath[MAX_PATH]{};
			const int verbose = g_opt.get_err_verbose();
			do
			{
				wsprintf(ws_full_filepath, L"%s\\%s", ws_full_path, f.cFileName);
				char full_filepath[MAX_PATH]{};
				SysUniConv::UnicodeToMultiByte(full_filepath, MAX_PATH, ws_full_filepath);
				auto [line, res] = LM->validate(full_filepath);
				bool isOK = line < 0;
				++file_cnt;
				if (!isOK)
				{
					++errf_cnt;
					SendNpp(NPPM_DOOPEN, 0, ws_full_filepath);
					SetConsoleColor(GetERRColor());
					print_from_lua(res.c_str(), false);
				}
				else if (verbose)
				{
					SetConsoleColor(GetOKColor());
					print_from_lua(res.c_str(), false);
				}
			} while (FindNextFile(h, &f));

			SetConsoleColor(errf_cnt ? GetERRColor() : GetOKColor());
			const wchar_t* fmtstr = Localize(STRINGID_FMT_CHECKALL, g_opt.GetLang());
			wsprintf(ws_full_filepath, fmtstr, (verbose ? file_cnt : errf_cnt) ? L"\n" : L"", errf_cnt, file_cnt);
			AddStr(ws_full_filepath);
		}
	}

	void OnCheckFiles()
	{
		std::thread(OnCheckFilesAsync).detach();
	}

#ifdef TEST_ITEM
	void OnTestItem()
	{
		OnClearConsole();
		AddStr(L"on test item clicked\n");
	}
#endif

	void StartColorDialog(COLORREF& clr)
	{
		static COLORREF custom_colours[16]{};
		custom_colours[0] = g_opt.clrOK;
		custom_colours[1] = g_opt.clrOKdm;
		custom_colours[2] = g_opt.clrERR;
		custom_colours[3] = g_opt.clrERRdm;
		CHOOSECOLORW m_choose_color;
		::ZeroMemory(&m_choose_color, sizeof(CHOOSECOLOR));
		m_choose_color.lStructSize = sizeof(CHOOSECOLOR);
		m_choose_color.hwndOwner = nppData._nppHandle;
		//m_choose_color.hInstance = NULL;
		m_choose_color.rgbResult = clr;
		m_choose_color.lpCustColors = custom_colours;
		m_choose_color.Flags = CC_RGBINIT | CC_FULLOPEN;
		m_choose_color.lCustData = 0;
		//m_choose_color.lpfnHook = NULL;
		//m_choose_color.lpTemplateName = NULL;
		if (ChooseColor(&m_choose_color))
		{
			g_opt.clrOK = custom_colours[0];
			g_opt.clrOKdm = custom_colours[1];
			g_opt.clrERR = custom_colours[2];
			g_opt.clrERRdm = custom_colours[3];
			clr = m_choose_color.rgbResult;
		}
	}

	void SetLangOptDialog(HWND hw, bool lng)
	{
		SetWindowText(hw, Localize(Options, lng));
		LocalizeDlgItem(IDC_CHECKLNG);
		LocalizeDlgItem(ID_BTNRESET);
		LocalizeDlgItem(IDC_CHECKAUTOCLEAR);
		LocalizeDlgItem(IDC_CHECKRUNTIME);
		LocalizeDlgItem(IDC_STATIC_RTQ);
		LocalizeDlgItem(IDC_GBOX_AF);
		LocalizeDlgItem(ID_BTN_AF_APPLY);
		LocalizeDlgItem(ID_BTN_AF_RESET);
		LocalizeDlgItem(IDC_TABSEP);
		LocalizeDlgItem(IDC_AFUSETABS);
		LocalizeDlgItem(IDC_KeepSimpleBlockOneLine);
		LocalizeDlgItem(IDC_KeepSimpleFunctionOneLine);
		LocalizeDlgItem(IDC_AlignArgs);
		LocalizeDlgItem(IDC_AlignParams);
		LocalizeDlgItem(IDC_ChopDownParams);
		LocalizeDlgItem(IDC_BreakAfterFCLP);
		LocalizeDlgItem(IDC_BreakBeforeFCRP);
		LocalizeDlgItem(IDC_AlignTableFields);
		LocalizeDlgItem(IDC_BreakAfterTLB);
		LocalizeDlgItem(IDC_BreakBeforeTRB);
		LocalizeDlgItem(IDC_ChopDownTable);
		LocalizeDlgItem(IDC_ChopDownKVTable);
		LocalizeDlgItem(IDC_BreakAfterFDLP);
		LocalizeDlgItem(IDC_BreakBeforeFDRP);
		LocalizeDlgItem(IDC_BreakAfterOperator);
		LocalizeDlgItem(IDC_ExtraSep);
		LocalizeDlgItem(IDC_TransformLiterals);
		LocalizeDlgItem(IDC_RADIO_SINGLEQUOTE);
		LocalizeDlgItem(IDC_RADIO_DOUBLEQOUTE);
		LocalizeDlgItem(IDC_SpaceInFuncDefParens);
		LocalizeDlgItem(IDC_SpaceInFuncCallParens);
		LocalizeDlgItem(IDC_SpaceInsideTableBraces);
		LocalizeDlgItem(IDC_SpaceAroundEquals);
		LocalizeDlgItem(IDC_DUMPCFG);
		LocalizeDlgItem(IDC_EOLTYPE_STATIC);
		LocalizeDlgItem(IDC_MAXLINELEN_STATIC);
		LocalizeDlgItem(IDC_NUMSPACES);
		LocalizeDlgItem(IDC_NUMSPACES_TAB);
		LocalizeDlgItem(IDC_NUMLINEBREAKSFB);
		LocalizeDlgItem(IDC_INDENTWIDTHCONT);
		LocalizeDlgItem(IDC_SPACESBEFOREFC);
		LocalizeDlgItem(IDC_COLMNLIMITTBL);
		LocalizeDlgItem(IDC_COLMNLIMITKVTBL);
		LocalizeDlgItem(IDC_GBOX_COMMON);
		LocalizeDlgItem(IDC_CHECKVERBOSE);

		// DarkMode support
		RedrawWindow(hw, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}

	void InitOptions(HWND hw)
	{
		SendDlgItemMessage(hw, IDC_AFUSETABS, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::UseTabs), 0);
		SendDlgItemMessage(hw, IDC_KeepSimpleBlockOneLine, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::KeepSimpleBlockOneLine), 0);
		SendDlgItemMessage(hw, IDC_KeepSimpleFunctionOneLine, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::KeepSimpleFunctionOneLine), 0);
		SendDlgItemMessage(hw, IDC_AlignArgs, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::AlighArgs), 0);
		SendDlgItemMessage(hw, IDC_BreakAfterFCLP, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakAfterFCLP), 0);
		SendDlgItemMessage(hw, IDC_BreakBeforeFCRP, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakBeforeFCRP), 0);
		SendDlgItemMessage(hw, IDC_AlignParams, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::AlignParams), 0);
		SendDlgItemMessage(hw, IDC_ChopDownParams, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::ChopDownParams), 0);
		SendDlgItemMessage(hw, IDC_BreakAfterFDLP, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakAfterFDLP), 0);
		SendDlgItemMessage(hw, IDC_BreakBeforeFDRP, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakBeforeFDRP), 0);
		SendDlgItemMessage(hw, IDC_AlignTableFields, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::AlignTableFields), 0);
		SendDlgItemMessage(hw, IDC_BreakAfterTLB, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakAfterTLB), 0);
		SendDlgItemMessage(hw, IDC_BreakBeforeTRB, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakBeforeTRB), 0);
		SendDlgItemMessage(hw, IDC_ChopDownTable, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::ChopDownTable), 0);
		SendDlgItemMessage(hw, IDC_ChopDownKVTable, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::ChopDownKVTable), 0);
		SendDlgItemMessage(hw, IDC_ExtraSep, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::ExtraSep), 0);
		SendDlgItemMessage(hw, IDC_BreakAfterOperator, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::BreakAfterOperator), 0);

		bool enable = g_opt.get_autoformat_option(AFOptions::TransformLiterals_Enabled);
		SendDlgItemMessage(hw, IDC_TransformLiterals, BM_SETCHECK, enable, 0);
		SendDlgItemMessage(hw, IDC_RADIO_SINGLEQUOTE, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::TransformLiterals), 0);
		SendDlgItemMessage(hw, IDC_RADIO_DOUBLEQOUTE, BM_SETCHECK, !g_opt.get_autoformat_option(AFOptions::TransformLiterals), 0);
		EnableWindow(GetDlgItem(hw, IDC_RADIO_SINGLEQUOTE), enable);
		EnableWindow(GetDlgItem(hw, IDC_RADIO_DOUBLEQOUTE), enable);

		SendDlgItemMessage(hw, IDC_SpaceInFuncDefParens, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::SpaceInFuncDefParens), 0);
		SendDlgItemMessage(hw, IDC_SpaceInFuncCallParens, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::SpaceInFuncCallParens), 0);
		SendDlgItemMessage(hw, IDC_SpaceInsideTableBraces, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::SpaceInsideTableBraces), 0);
		SendDlgItemMessage(hw, IDC_SpaceAroundEquals, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::SpaceAroundEquals), 0);
		SendDlgItemMessage(hw, IDC_DUMPCFG, BM_SETCHECK, g_opt.get_autoformat_option(AFOptions::DumpConfig), 0);
		SetDlgItemTextA(hw, IDC_LINE_SEP, g_opt.get_line_sep().c_str());
		SetDlgItemInt(hw, IDC_COLUMN_LIMIT, g_opt.get_column_limit(), FALSE);
		SetDlgItemInt(hw, IDC_INDENT_WIDTH, g_opt.get_indent_width(), FALSE);
		SetDlgItemInt(hw, IDC_TAB_WIDTH, g_opt.get_tab_width(), FALSE);
		SetDlgItemInt(hw, IDC_CONT_INDENT_WIDTH, g_opt.get_continuation_indent_width(), FALSE);
		SetDlgItemInt(hw, IDC_SPACES_BEFORE_CALL, g_opt.get_spaces_before_call(), FALSE);
		SetDlgItemInt(hw, IDC_COLUMN_TABLE_LIMIT, g_opt.get_column_table_limit(), FALSE);
		SetDlgItemInt(hw, IDC_COLUMN_TABLE_LIMIT_KV, g_opt.get_column_table_limit_kv(), FALSE);
		SetDlgItemInt(hw, IDC_LINE_BREAKS, g_opt.get_line_breaks(), FALSE);

		SendDlgItemMessage(hw, IDC_RADIO_TABSEP0, BM_SETCHECK, g_opt.tabsep_is_comma(), 0);		// table separator is comma
		SendDlgItemMessage(hw, IDC_RADIO_TABSEP1, BM_SETCHECK, !g_opt.tabsep_is_comma(), 0);	// table separator is semicilon
		SendDlgItemMessage(hw, IDC_CHECKVERBOSE, BM_SETCHECK, g_opt.get_err_verbose(), 0);
	}

	void SaveOptions(HWND hw)
	{
		char str[6]{};
		GetDlgItemTextA(hw, IDC_LINE_SEP, str, 5);
		g_opt.set_line_sep(str);
		g_opt.set_column_limit(GetDlgItemInt(hw, IDC_COLUMN_LIMIT, NULL, FALSE));
		g_opt.set_indent_width(GetDlgItemInt(hw, IDC_INDENT_WIDTH, NULL, FALSE));
		g_opt.set_tab_width(GetDlgItemInt(hw, IDC_TAB_WIDTH, NULL, FALSE));
		g_opt.set_spaces_before_call(GetDlgItemInt(hw, IDC_SPACES_BEFORE_CALL, NULL, FALSE));
		g_opt.set_column_table_limit(GetDlgItemInt(hw, IDC_COLUMN_TABLE_LIMIT, NULL, FALSE));
		g_opt.set_column_table_limit_kv(GetDlgItemInt(hw, IDC_COLUMN_TABLE_LIMIT_KV, NULL, FALSE));
		g_opt.set_continuation_indent_width(GetDlgItemInt(hw, IDC_CONT_INDENT_WIDTH, NULL, FALSE));
		g_opt.set_line_breaks(GetDlgItemInt(hw, IDC_LINE_BREAKS, NULL, FALSE));
	}

	void OnAutoFormat()
	{
		const size_t len = SendSci(SCI_GETTEXT);
		if (!len) return;

		wchar_t ws_full_path[MAX_PATH]{};
		SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path);
		if (!isValidDocument(ws_full_path)) return;

		OnClearConsole();
		if (!g_opt.m_bConsoleOpen) OpenCloseConsole();

		char full_path[MAX_PATH]{};
		SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);
		auto [line, res] = LM->validate(full_path);
		if (line >= 0)
		{
			AddMarker(line);
			SetConsoleColor(GetERRColor());
			print_from_lua(res.c_str(), false);
			return;
		}

		std::string buf(len, 0);
		SendSci(SCI_GETTEXT, len + 1, reinterpret_cast<LPARAM>(buf.data()));

		const std::vector<std::string> v_options = g_opt.get_autoformat_options();
		auto [new_text, error_info] = autoformat(buf, v_options);
		if (new_text.size())
		{
			SendSci(SCI_BEGINUNDOACTION);
			const Sci_PositionU topline = SendSci(SCI_GETFIRSTVISIBLELINE);
			SendSci(SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(new_text.data()));
			SendSci(SCI_ENDUNDOACTION);
			SendNpp(NPPM_SAVECURRENTFILE, 0, 0);
			SendSci(SCI_SETFIRSTVISIBLELINE, topline);
			SetConsoleColor(GetOKColor());
			AddStr(L"Аutoformat done.");
		}
		else
		{
			SetConsoleColor(GetERRColor());
			if (error_info.size())
			{
				AddStr(L"Аutoformat failed: ");
				print_from_lua(error_info.c_str());
			}
			else
			{
				AddStr(L"Аutoformat error.");
			}
		}
	}

	INT_PTR CALLBACK OptionDlgProc(HWND hw, UINT msg, WPARAM wp, LPARAM /*lp*/)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hw, IDC_CHECKAUTOCLEAR, BM_SETCHECK, g_opt.m_bConsoleAutoclear, 0);
			SendDlgItemMessage(hw, IDC_CHECKRUNTIME, BM_SETCHECK, g_opt.m_bShowRunTime, 0);
			SendDlgItemMessage(hw, IDC_CHECKCONENC, BM_SETCHECK, g_opt.m_bConEncoding, 0);
			SetDlgItemText(hw, IDC_CHECKCONENC, g_opt.m_bConEncoding ? L"UTF8" : L"ANSI");
			SetDlgItemText(hw, IDC_EDITLOVEPATH, g_opt.LovePath);
			SendDlgItemMessage(hw, IDC_SPIN1, UDM_SETRANGE32, 0, 15);
			SendDlgItemMessage(hw, IDC_SPIN1, UDM_SETPOS32, 0, g_opt.timequote);

			const bool lng = g_opt.GetLang();
			SendDlgItemMessage(hw, IDC_CHECKLNG, BM_SETCHECK, lng, 0);
			SetLangOptDialog(hw, lng);

			// init autoformat options
			InitOptions(hw);

			// DarkMode support
			g_hOptionDlg = hw;
			SendNpp(NPPM_DARKMODESUBCLASSANDTHEME, NppDarkMode::dmfInit, hw);
			return TRUE;
		}
		case WM_LBUTTONDOWN:
		{
			if (clr_choose_shown) return TRUE;
			clr_choose_shown = true;
			POINT ptCursor;
			GetCursorPos(&ptCursor); // on screen
			ScreenToClient(hw, &ptCursor); // on window
			RECT rt;

			// OK
			HWND hs = GetDlgItem(hw, ID_STATIC);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			if (PtInRect(&rt, ptCursor))
			{
				StartColorDialog(g_opt.clrOK);
				SetFocus(hw);
				InvalidateRect(hw, nullptr, TRUE);
				clr_choose_shown = false;
				return TRUE;
			}

			// OK DM
			hs = GetDlgItem(hw, ID_STATIC3);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			if (PtInRect(&rt, ptCursor))
			{
				StartColorDialog(g_opt.clrOKdm);
				SetFocus(hw);
				InvalidateRect(hw, nullptr, TRUE);
				clr_choose_shown = false;
				return TRUE;
			}

			// ERR
			hs = GetDlgItem(hw, ID_STATIC2);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			if (PtInRect(&rt, ptCursor))
			{
				StartColorDialog(g_opt.clrERR);
				SetFocus(hw);
				InvalidateRect(hw, nullptr, TRUE);
				clr_choose_shown = false;
				return TRUE;
			}

			// ERR DM
			hs = GetDlgItem(hw, ID_STATIC4);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			if (PtInRect(&rt, ptCursor))
			{
				StartColorDialog(g_opt.clrERRdm);
				SetFocus(hw);
				InvalidateRect(hw, nullptr, TRUE);
				clr_choose_shown = false;
				return TRUE;
			}
			break;
		}
		case WM_PAINT:
		{
			RECT rt;
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hw, &ps);

			// OK
			HWND hs = GetDlgItem(hw, ID_STATIC);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			HBRUSH br = CreateSolidBrush(g_opt.clrOK);
			FillRect(hdc, &rt, br);
			DeleteObject(br);

			// OK DM
			hs = GetDlgItem(hw, ID_STATIC3);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			br = CreateSolidBrush(g_opt.clrOKdm);
			FillRect(hdc, &rt, br);
			DeleteObject(br);

			// ERR
			hs = GetDlgItem(hw, ID_STATIC2);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			br = CreateSolidBrush(g_opt.clrERR);
			FillRect(hdc, &rt, br);
			DeleteObject(br);

			// ERR DM
			hs = GetDlgItem(hw, ID_STATIC4);
			GetWindowRect(hs, &rt);
			MapWindowPoints(NULL, hw, (LPPOINT)&rt, 2);
			br = CreateSolidBrush(g_opt.clrERRdm);
			FillRect(hdc, &rt, br);
			DeleteObject(br);

			EndPaint(hw, &ps);
			return FALSE;
		}
		case WM_COMMAND:
			switch (LOWORD(wp))
			{
			case IDC_BUTTONOK:
			{
				SaveOptions(hw);
				EndDialog(hw, IDOK);
				break;
			}
			case IDCANCEL:
			{
				EndDialog(hw, IDCANCEL);
				break;
			}
			case IDC_CHECKAUTOCLEAR:
			{
				g_opt.m_bConsoleAutoclear = SendDlgItemMessage(hw, IDC_CHECKAUTOCLEAR, BM_GETCHECK, 0, 0);
				break;
			}
			case IDC_CHECKCONENC:
			{
				g_opt.m_bConEncoding = SendDlgItemMessage(hw, IDC_CHECKCONENC, BM_GETCHECK, 0, 0);
				SetDlgItemText(hw, IDC_CHECKCONENC, g_opt.m_bConEncoding ? L"UTF8" : L"ANSI");
				break;
			}
			case IDC_CHECKRUNTIME:
			{
				g_opt.m_bShowRunTime = SendDlgItemMessage(hw, IDC_CHECKRUNTIME, BM_GETCHECK, 0, 0);
				break;
			}
			case IDC_OFDLOVEPATH:
			{
				wchar_t Love2D_path[MAX_PATH]{};
				OPENFILENAME ofn{};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFilter = L"love.exe\0love.exe\0\0"; // filter
				ofn.lpstrTitle = L"Path to 'love.exe'"; // title
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrFile = Love2D_path; // buffer for result
				if (!GetOpenFileName(&ofn)) return TRUE;
				const wchar_t* pt = L"\\love.exe";
				if (lstrcmp(Love2D_path + lstrlen(Love2D_path) - lstrlen(pt), pt)) return TRUE;
				lstrcpy(g_opt.LovePath, Love2D_path); // save in options
				SetDlgItemText(hw, IDC_EDITLOVEPATH, Love2D_path);
				EnableMenuItem(execData.hMenu, funcItems[RunLove2D]._cmdID, MF_BYCOMMAND | MF_ENABLED);
				break;
			}
			case IDC_QUOTEEDIT:
			{
				if (IsWindowVisible(hw))
					g_opt.timequote = GetDlgItemInt(hw, IDC_QUOTEEDIT, 0, 0);
				break;
			}
			case IDC_CHECKLNG:
			{
				g_opt.OnSwitchLang();
				SetLangOptDialog(hw, g_opt.GetLang());
				ApplyLang();
				break;
			}
			case IDC_AFUSETABS:
			{
				g_opt.set_autoformat_option(AFOptions::UseTabs, SendDlgItemMessage(hw, IDC_AFUSETABS, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_KeepSimpleBlockOneLine:
			{
				g_opt.set_autoformat_option(AFOptions::KeepSimpleBlockOneLine, SendDlgItemMessage(hw, IDC_KeepSimpleBlockOneLine, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_KeepSimpleFunctionOneLine:
			{
				g_opt.set_autoformat_option(AFOptions::KeepSimpleFunctionOneLine, SendDlgItemMessage(hw, IDC_KeepSimpleFunctionOneLine, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_AlignArgs:
			{
				g_opt.set_autoformat_option(AFOptions::AlighArgs, SendDlgItemMessage(hw, IDC_AlignArgs, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakAfterFCLP:
			{
				g_opt.set_autoformat_option(AFOptions::BreakAfterFCLP, SendDlgItemMessage(hw, IDC_BreakAfterFCLP, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakBeforeFCRP:
			{
				g_opt.set_autoformat_option(AFOptions::BreakBeforeFCRP, SendDlgItemMessage(hw, IDC_BreakBeforeFCRP, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_AlignParams:
			{
				g_opt.set_autoformat_option(AFOptions::AlignParams, SendDlgItemMessage(hw, IDC_AlignParams, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_ChopDownParams:
			{
				g_opt.set_autoformat_option(AFOptions::ChopDownParams, SendDlgItemMessage(hw, IDC_ChopDownParams, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakAfterFDLP:
			{
				g_opt.set_autoformat_option(AFOptions::BreakAfterFDLP, SendDlgItemMessage(hw, IDC_BreakAfterFDLP, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakBeforeFDRP:
			{
				g_opt.set_autoformat_option(AFOptions::BreakBeforeFDRP, SendDlgItemMessage(hw, IDC_BreakBeforeFDRP, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_AlignTableFields:
			{
				g_opt.set_autoformat_option(AFOptions::AlignTableFields, SendDlgItemMessage(hw, IDC_AlignTableFields, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakAfterTLB:
			{
				g_opt.set_autoformat_option(AFOptions::BreakAfterTLB, SendDlgItemMessage(hw, IDC_BreakAfterTLB, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakBeforeTRB:
			{
				g_opt.set_autoformat_option(AFOptions::BreakBeforeTRB, SendDlgItemMessage(hw, IDC_BreakBeforeTRB, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_ChopDownTable:
			{
				g_opt.set_autoformat_option(AFOptions::ChopDownTable, SendDlgItemMessage(hw, IDC_ChopDownTable, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_ChopDownKVTable:
			{
				g_opt.set_autoformat_option(AFOptions::ChopDownKVTable, SendDlgItemMessage(hw, IDC_ChopDownKVTable, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_ExtraSep:
			{
				g_opt.set_autoformat_option(AFOptions::ExtraSep, SendDlgItemMessage(hw, IDC_ExtraSep, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_BreakAfterOperator:
			{
				g_opt.set_autoformat_option(AFOptions::BreakAfterOperator, SendDlgItemMessage(hw, IDC_BreakAfterOperator, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_TransformLiterals:
			{
				const int enable = static_cast<int>(SendDlgItemMessage(hw, IDC_TransformLiterals, BM_GETCHECK, 0, 0));
				EnableWindow(GetDlgItem(hw, IDC_RADIO_SINGLEQUOTE), enable);
				EnableWindow(GetDlgItem(hw, IDC_RADIO_DOUBLEQOUTE), enable);
				g_opt.set_autoformat_option(AFOptions::TransformLiterals_Enabled, enable);
				g_opt.set_autoformat_option(AFOptions::TransformLiterals, SendDlgItemMessage(hw, IDC_RADIO_SINGLEQUOTE, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_SpaceInFuncDefParens:
			{
				g_opt.set_autoformat_option(AFOptions::SpaceInFuncDefParens, SendDlgItemMessage(hw, IDC_SpaceInFuncDefParens, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_SpaceInFuncCallParens:
			{
				g_opt.set_autoformat_option(AFOptions::SpaceInFuncCallParens, SendDlgItemMessage(hw, IDC_SpaceInFuncCallParens, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_SpaceInsideTableBraces:
			{
				g_opt.set_autoformat_option(AFOptions::SpaceInsideTableBraces, SendDlgItemMessage(hw, IDC_SpaceInsideTableBraces, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_SpaceAroundEquals:
			{
				g_opt.set_autoformat_option(AFOptions::SpaceAroundEquals, SendDlgItemMessage(hw, IDC_SpaceAroundEquals, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_DUMPCFG:
			{
				g_opt.set_autoformat_option(AFOptions::DumpConfig, SendDlgItemMessage(hw, IDC_DUMPCFG, BM_GETCHECK, 0, 0));
				break;
			}
			case ID_BTN_AF_APPLY:
			{
				SaveOptions(hw);
				OnAutoFormat();
				break;
			}
			case ID_BTN_AF_RESET:
			{
				g_opt.reset_af_options();
				InitOptions(hw);
				break;
			}
			case IDC_RADIO_TABSEP0:
			{
				g_opt.set_table_sep(true);
				break;
			}
			case IDC_RADIO_TABSEP1:
			{
				g_opt.set_table_sep(false);
				break;
			}
			case IDC_RADIO_SINGLEQUOTE:
			{
				g_opt.set_autoformat_option(AFOptions::TransformLiterals, SendDlgItemMessage(hw, IDC_RADIO_SINGLEQUOTE, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_RADIO_DOUBLEQOUTE:
			{
				g_opt.set_autoformat_option(AFOptions::TransformLiterals, !SendDlgItemMessage(hw, IDC_RADIO_DOUBLEQOUTE, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_CHECKVERBOSE:
			{
				g_opt.set_err_verbose(static_cast<int>(SendDlgItemMessage(hw, IDC_CHECKVERBOSE, BM_GETCHECK, 0, 0)));
				break;
			}
			case ID_BTNRESET:
			{
				g_opt.clrOK = DEFCOLOROK;
				g_opt.clrOKdm = DEFCOLORDMOK;
				g_opt.clrERR = DEFCOLORERR;
				g_opt.clrERRdm = DEFCOLORDMERR;
				SendDlgItemMessage(hw, IDC_SPIN1, UDM_SETPOS32, 0, 3);
				InvalidateRect(hw, nullptr, TRUE); // redraw
				break;
			}
			default:
				break;
			}
		}
		return FALSE;
	}

	void OnOptions()
	{
		DialogBox(execData.hNPP, MAKEINTRESOURCE(IDD_OPTDIALOG), nppData._nppHandle, OptionDlgProc);
	}

	void OnShowAboutDlg()
	{
		const wchar_t* txt =
			L" Lua plugin v2.3.9 "
#ifdef _WIN64
			L"(64-bit)"
#else
			L"(32-bit)"
#endif 
			L"\n\n"\
			L" Author: Charsi <charsi2011@gmail.com>\n\n"\
			L" Syntax checking of Lua scripts.\n"\
			L" Run the script from current file.\n"\
			L" Autoformatting code.\n"\
			L" Run LÖVE2D game from current directory.\n";
		MessageBox(NULL, txt, L"Lua plugin for Notepad++", MB_OK);
	}


	void ShowPopup(HWND hwndDlg)
	{
		HMENU menu = ::CreatePopupMenu();
		const bool lng = g_opt.GetLang();
		AppMenu(ID_COPY);
		AppMenu(ID_COPYSELECTED);
		AppMenu(ID_CLEARCONSOLE);
		AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
		AppMenu(ID_CLOSECONSOLE);

		POINT pt;
		GetCursorPos(&pt);
		TrackPopupMenu(menu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwndDlg, nullptr);
		PostMessage(hwndDlg, WM_NULL, 0, 0);
		DestroyMenu(menu);
	}

	bool str2Clipboard(const wchar_t* str2cpy, HWND hwnd)
	{
		const size_t len2Allocate = static_cast<size_t>(lstrlen(str2cpy)) + 1;
		HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len2Allocate * sizeof(wchar_t));
		if (!hglbCopy)
		{
			return false;
		}
		if (!::OpenClipboard(hwnd))
		{
			::GlobalFree(hglbCopy);
			::CloseClipboard();
			return false;
		}
		//if (!::EmptyClipboard())
		//{
		//	::GlobalFree(hglbCopy);
		//	::CloseClipboard();
		//	return false;
		//}
		// Lock the handle and copy the text to the buffer.
		wchar_t* pStr = reinterpret_cast<wchar_t*>(::GlobalLock(hglbCopy));
		if (!pStr)
		{
			::GlobalUnlock(hglbCopy);
			::GlobalFree(hglbCopy);
			::CloseClipboard();
			return false;
		}
		wcscpy_s(pStr, len2Allocate, str2cpy);
		::GlobalUnlock(hglbCopy);
		// Place the handle on the clipboard.
		if (!::SetClipboardData(CF_UNICODETEXT, hglbCopy))
		{
			::GlobalFree(hglbCopy);
			::CloseClipboard();
			return false;
		}
		if (!::CloseClipboard())
		{
			return false;
		}
		return true;
	}

	void OnCopy(bool selected)
	{
		HWND hRE = GetConsole();
		const size_t len = static_cast<size_t>(GetWindowTextLength(hRE)) + 1;
		std::wstring wtmp(len, 0);
		GETTEXTEX gtex{};
		gtex.cb = static_cast<DWORD>(len * sizeof(wchar_t)); // size in bytes
		gtex.codepage = 1200; // 1200 for UNICODE
		gtex.flags = selected ? GT_SELECTION : GT_DEFAULT;
		SendMessage(hRE, EM_GETTEXTEX, reinterpret_cast<WPARAM>(&gtex), reinterpret_cast<LPARAM>(wtmp.data()));
		str2Clipboard(wtmp.data(), execData.hConsole);
	}

}

void GlobalInitialize()
{
	// Fetch the menu
	execData.hMenu = GetMenu(nppData._nppHandle);

	// Create the docking dialog
	execData.hConsole = CreateDialog(execData.hNPP,
		MAKEINTRESOURCE(IDD_CONSOLE), nppData._nppHandle,
		reinterpret_cast<DLGPROC>(ConsoleProcDlg));

	dockingData.hClient = execData.hConsole;
	dockingData.pszName = execData.ConsoleCaption;
	dockingData.pszAddInfo = execData.addInfo;
	dockingData.pszModuleName = PROJECT_NAME;
	dockingData.uMask = DWS_DF_CONT_BOTTOM | DWS_ADDINFO | DWS_USEOWNDARKMODE;

	if (HICON hIcon = static_cast<HICON>(
		::LoadImage(
			GetModuleHandle(PROJECT_NAME ".dll"),
			MAKEINTRESOURCE(IDI_ICON1),
			IMAGE_ICON,
			16, 16,    // or whatever size icon you want to load
			LR_DEFAULTCOLOR))
		)
	{
		dockingData.uMask |= DWS_ICONTAB;
		dockingData.hIconTab = hIcon;
	}


	// Register the docking dialog
	SendNpp(NPPM_DMMREGASDCKDLG, 0, &dockingData);
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole, MODELESSDIALOGADD);

	int isRussian = 0;
	if (const size_t len = SendNpp(NPPM_GETNATIVELANGFILENAME, 0, NULL))
	{
		std::string tmp(len, 0);
		SendNpp(NPPM_GETNATIVELANGFILENAME, len + 1, tmp.data());
		isRussian = (tmp.find("russian") == std::string::npos) ? 1 : 0;
	}
	g_opt.ReadOptions(isRussian);
	ApplyLang();
	g_opt.m_bConsoleOpen = !g_opt.m_bConsoleOpen;
	OpenCloseConsole();

	wchar_t plugin_dir[MAX_PATH]{};
	UINT nLen = GetModuleFileName((HMODULE)execData.hNPP, plugin_dir, _countof(plugin_dir));
	while (nLen > 0 && plugin_dir[nLen] != L'\\') plugin_dir[nLen--] = 0;
	auto file_exist = [&plugin_dir](const wchar_t* dll_name)->bool
		{
			wchar_t lua_dll[MAX_PATH]{};
			lstrcpy(lua_dll, plugin_dir);
			lstrcat(lua_dll, dll_name);
			//return PathFileExists(lua_dll);
			return isFileExists(lua_dll);
		};
	auto enableMenuItem = [](int ItemID) { EnableMenuItem(execData.hMenu, funcItems[ItemID]._cmdID, MF_BYCOMMAND | MF_DISABLED); };

	const bool fne51 = !file_exist(path_lua51);
	if (fne51) enableMenuItem(LUA51);
	const bool fne52 = !file_exist(path_lua52);
	if (fne52) enableMenuItem(LUA52);
	const bool fne53 = !file_exist(path_lua53);
	if (fne53) enableMenuItem(LUA53);
	const bool fne54 = !file_exist(path_lua54);
	if (fne54) enableMenuItem(LUA54);
	const bool fnePluto = !file_exist(path_pluto);
	if (fnePluto) enableMenuItem(LUAPLUTO);
	const bool fnejit = !file_exist(path_luajit);
	if (fnejit) enableMenuItem(LUAJIT);
	if (fne51 && fne52 && fne53 && fne54 && fnePluto && fnejit)
	{
		enableMenuItem(CheckSyntax);
		enableMenuItem(RunScript);
		enableMenuItem(CheckFiles);
	}

	// disable love2d launcher if love.exe not found
	//if (!PathFileExists(g_opt.LovePath)) enableMenuItem(RunLove2D);
	if (!isFileExists(g_opt.LovePath)) enableMenuItem(RunLove2D);

	LM = new CLuaManager;
	switch (g_opt.m_uInterpType)
	{
	case LUA52:
		OnLua52();
		break;
	case LUA53:
		OnLua53();
		break;
	case LUA54:
		OnLua54();
		break;
	case LUAPLUTO:
		OnLuaPluto();
		break;
	case LUAJIT:
		OnLuaJIT();
		break;
	case LUA51:
	default:
		OnLua51();
	}
}

void GlobalDeinitialize()
{
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole, MODELESSDIALOGREMOVE);
	g_opt.SaveOptions();
	if (LM) delete LM;
}

void OpenCloseConsole()
{
	g_opt.m_bConsoleOpen = !g_opt.m_bConsoleOpen;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[ShowHideConsole]._cmdID, g_opt.m_bConsoleOpen);
	SendNpp(g_opt.m_bConsoleOpen ? NPPM_DMMSHOW : NPPM_DMMHIDE, NULL, execData.hConsole);

	// DarkMode support
	if (g_opt.m_bConsoleOpen)
		ResetConsoleColors();
}

void OnClearConsole()
{
	SetWindowText(GetConsole(), NULL);
	ResetConsoleColors();
	bNeedClear = false;
}

BOOL CALLBACK ConsoleProcDlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
	{
		NMHDR* pnmh = reinterpret_cast<NMHDR*>(lParam);
		if (pnmh->hwndFrom == nppData._nppHandle)
		{
			switch (LOWORD(pnmh->code))
			{
			case DMN_CLOSE: // closing dialog
				OpenCloseConsole();
				break;
			case DMN_FLOAT: // floating dialog
				is_floating = true;
				break;
			case DMN_DOCK: // docking dialog; HIWORD(pnmh->code) is dockstate [0..3]
				is_floating = false; // static_cast<BYTE>(HIWORD(pnmh->code));
				break;
			}
		}
		break;
	}
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case ID_COPY:
			OnCopy(false);
			break;
		case ID_COPYSELECTED:
			OnCopy(true);
			break;
		case ID_CLEARCONSOLE:
			OnClearConsole();
			break;
		case ID_CLOSECONSOLE:
			OpenCloseConsole();
			break;
		}
		break;
	}
	case WM_CONTEXTMENU:
	{
		ShowPopup(hwndDlg);
		return TRUE;
	}
	case WM_SIZE:
	{
		RECT rcConsole{};
		GetClientRect(execData.hConsole, &rcConsole);
		SetWindowPos(GetConsole(), NULL, 0, 0,
			rcConsole.right, rcConsole.bottom, SWP_NOZORDER);
		break;
	}
	default:
		break;
	}
	return FALSE;
}

void AddStr(const wchar_t* msg)
{
	if (!msg) return;
	HWND hRE = GetConsole();
	size_t ndx = GetWindowTextLength(hRE);
	const size_t line_from = SendMessage(hRE, EM_LINEFROMCHAR, ndx, 0);
	const size_t start_from = SendMessage(hRE, EM_LINEINDEX, line_from, 0);

	// clear selection
	SendMessage(hRE, EM_SETSEL, ndx, ndx);

	// set text
	SendMessage(hRE, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(msg));

	// select added text
	ndx = GetWindowTextLength(hRE);

	// set text color
	SendMessage(hRE, EM_SETSEL, start_from, ndx);
	CHARFORMAT cf{};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = current_color;
	SendMessage(hRE, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&cf));

	// move to end and clear slection
	SendMessage(hRE, EM_SETSEL, ndx, ndx);
}

///////////////////////////////////////////////////
// Main
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/)
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
	{
		execData.hNPP = (HINSTANCE)hModule;

		// init menu
		InitFuncItem(CheckSyntax, Localize(CheckSyntax, 1), OnCheckSyntax, &sk1);
		InitFuncItem(RunScript, Localize(RunScript, 1), OnRunScript, &sk2);
		InitFuncItem(CheckFiles, Localize(CheckFiles, 1), OnCheckFiles);
		InitFuncItem(AutoFormat, Localize(AutoFormat, 1), OnAutoFormat, &sk3);
		InitFuncItem(RunLove2D, Localize(RunLove2D, 1), OnLove2D);
		//InitFuncItem(Separator1);
		InitFuncItem(LUA51, Localize(LUA51, 1), OnLua51);
		InitFuncItem(LUA52, Localize(LUA52, 1), OnLua52);
		InitFuncItem(LUA53, Localize(LUA53, 1), OnLua53);
		InitFuncItem(LUA54, Localize(LUA54, 1), OnLua54);
		InitFuncItem(LUAPLUTO, Localize(LUAPLUTO, 1), OnLuaPluto);
		InitFuncItem(LUAJIT, Localize(LUAJIT, 1), OnLuaJIT);
		//InitFuncItem(Separator2);
		InitFuncItem(ShowHideConsole, Localize(ShowHideConsole, 1), OpenCloseConsole);
		InitFuncItem(Options, Localize(Options, 1), OnOptions);
		InitFuncItem(About, Localize(About, 1), OnShowAboutDlg);
#ifdef TEST_ITEM
		InitFuncItem(TestItem, Localize(About, 1), OnTestItem);
#endif

		m_hRichEditDll = LoadLibrary(L"Riched20.dll");
		break;
	}
	case DLL_PROCESS_DETACH:
		if (m_hRichEditDll)
			FreeLibrary(m_hRichEditDll);
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport)
void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
}

extern "C" __declspec(dllexport)
const wchar_t* getName()
{
	return sPluginName;
}

extern "C" __declspec(dllexport)
FuncItem* getFuncsArray(int* nbF)
{
	*nbF = nbFunc;
	return funcItems;
}

extern "C" __declspec(dllexport)
void beNotified(SCNotification* notifyCode)
{
	//if (notifyCode->nmhdr.hwndFrom == nppData._nppHandle)
	switch (notifyCode->nmhdr.code)
	{
	case NPPN_READY:
		GlobalInitialize();
		break;

	case NPPN_SHUTDOWN:
		GlobalDeinitialize();
		break;

	case NPPN_DARKMODECHANGED:
		OnClearConsole();
		break;

	case NPPN_NATIVELANGCHANGED:
		// fix reset localization to default
	case NPPN_FILEOPENED:
	case 2007:		// ??			
	case 2020:		// ??				
		ApplyLang();
		//{
		//	auto str = std::format(L"fix with event {}\n", notifyCode->nmhdr.code);
		//	AddStr(str.c_str());
		//}
		break;

	default:
		break;
	}
}

extern "C" __declspec(dllexport)
LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return TRUE;
}

extern "C" __declspec(dllexport)
BOOL isUnicode()
{
	return TRUE;
}
