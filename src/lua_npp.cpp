#include "resource.h"
#include "lua_main.h"
#include "ctime"
#include <Richedit.h>
#include <shlwapi.h>
#include "PluginDarkMode.h"
///////////////////
// Plugin Variables
///////////////////
NppData nppData;
ExecData execData;
CPluginOptions g_opt;
bool bNeedClear = false;
tTbData dockingData;
SCNotification* scNotification;
HMODULE m_hRichEditDll;
CLuaManager* LM;
const TCHAR path_lua51[] = L"lua51.dll";
const TCHAR path_lua52[] = L"lua52.dll";
const TCHAR path_lua53[] = L"lua53.dll";
const TCHAR path_lua54[] = L"lua54.dll";
const TCHAR path_luajit[] = L"luajit.dll";
static TCHAR ConsoleCaption[20]{};
static BYTE is_floating = 8;
static bool clr_choose_shown = false;

enum class ColorMode {
	NORMAL,
	OK,
	ERR
};

static ColorMode ColorModeState = ColorMode::NORMAL;

FuncItem funcItems[nbFunc];

bool file_exist(const TCHAR* dll_name)
{
	TCHAR lua_dll[MAX_PATH]{};
	UINT nLen = GetModuleFileName((HMODULE)execData.hNPP, lua_dll, MAX_PATH);
	while (nLen > 0 && lua_dll[nLen] != L'\\') lua_dll[nLen--] = 0;
	//lstrcpy(lua_dll, dll_name);
	lstrcat(lua_dll, dll_name);
	return PathFileExists(lua_dll);
}

void InitFuncItem(int nItem, const TCHAR* szName, PFUNCPLUGINCMD pFunc, ShortcutKey* pShortcut)
{
	lstrcpy(funcItems[nItem]._itemName, szName);
	funcItems[nItem]._pFunc = pFunc;
	funcItems[nItem]._init2Check = false; //bCheck;
	funcItems[nItem]._pShKey = pShortcut;
}

HWND GetCurrentScintilla()
{
	int currentView = 0;
	SendNpp(NPPM_GETCURRENTSCINTILLA, 0, &currentView);
	return (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

LRESULT SendSci(UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(GetCurrentScintilla(), iMessage, wParam, lParam);
}

void SetCharFormat(COLORREF color = RGB(0, 0, 0), DWORD dwMask = CFM_COLOR, DWORD dwEffects = 0, DWORD dwOptions = SCF_ALL)
{
	CHARFORMAT cf{};
	cf.cbSize = sizeof(cf);
	cf.dwMask = dwMask;
	cf.dwEffects = dwEffects;
	cf.crTextColor = color;
	SendMessage(GetConsole(), EM_SETCHARFORMAT, dwOptions, (LPARAM)&cf);
}

void SetConsoleTextColor()
{
	const bool isDM = SendNpp(NPPM_ISDARKMODEENABLED, 0, 0);
	switch (ColorModeState)
	{
	case ColorMode::OK:
		SetCharFormat(isDM ? g_opt.clrOKdm : g_opt.clrOK);
		break;
	case ColorMode::ERR:
		SetCharFormat(isDM ? g_opt.clrERRdm : g_opt.clrERR);
		break;
	default:
		SetCharFormat(isDM ? PluginDarkMode::getTextColor() : GetSysColor(COLOR_MENUTEXT));
	}
}

void OnLove2D()
{
	// check love2d path
	if (!PathFileExists(g_opt.LovePath))
	{
		TCHAR tmp[MAX_PATH]{};
		wsprintf(tmp, L"Can't find file '%s.'\n", g_opt.LovePath);
		AddStr(tmp);
		EnableMenuItem(execData.hMenu, funcItems[RunLove2D]._cmdID, MF_BYCOMMAND | MF_DISABLED);
		return;
	}
	TCHAR ws_full_path[MAX_PATH]{};
	SendNpp(NPPM_GETCURRENTDIRECTORY, MAX_PATH, ws_full_path);

	// check main.lua in current directory
	TCHAR main_path[MAX_PATH]{};
	wsprintf(main_path, L"%s%s", ws_full_path, L"\\main.lua");
	if (!PathFileExists(main_path))
		return AddStr(L"File 'main.lua' not found in current directory!\n");

	// run script
	TCHAR param_dir[MAX_PATH];
	wsprintf(param_dir, L"\"%s\"", ws_full_path);
	ShellExecute(NULL, L"open", g_opt.LovePath, param_dir, NULL, SW_NORMAL);
}

void ResetConsoleColors()
{
	SendMessage(GetConsole(), EM_SETBKGNDCOLOR, 0,
		SendNpp(NPPM_ISDARKMODEENABLED, 0, 0) ?
		PluginDarkMode::getSofterBackgroundColor() :
		GetSysColor(COLOR_MENU));
	SetConsoleTextColor();
}

void ResetItemCheck()
{
	for (auto i = (UINT8)LUA51; i <= (UINT8)LUAJIT; ++i)
		SendNpp(NPPM_SETMENUITEMCHECK, funcItems[i]._cmdID, g_opt.m_uInterpType == i);
	switch (g_opt.m_uInterpType)
	{
	case LUA54:
		wcscpy_s(ConsoleCaption, L" Console Lua54 ");
		break;

	case LUA53:
		wcscpy_s(ConsoleCaption, L" Console Lua53 ");
		break;

	case LUA52:
		wcscpy_s(ConsoleCaption, L" Console Lua52 ");
		break;

	case LUAJIT:
		wcscpy_s(ConsoleCaption, L" Console LuaJIT ");
		break;

	case LUA51:
	default:
		wcscpy_s(ConsoleCaption, L" Console Lua51 ");
	}

	SetWindowText(execData.hConsole, ConsoleCaption);
	if (g_opt.m_bConsoleOpenOnInit)
		if (is_floating < 4)
		{
			SetFocus(execData.hConsole);
			SetFocus(GetCurrentScintilla());
		}
		else
		{
			SendNpp(NPPM_DMMHIDE, 0, execData.hConsole);
			SendNpp(NPPM_DMMSHOW, 0, execData.hConsole);
		}
}

void OnLua51()
{
	g_opt.m_uInterpType = LUA51;
	ResetItemCheck();
	LM->reset_lib(path_lua51, LUA51);
}

void OnLua52()
{
	g_opt.m_uInterpType = LUA52;
	ResetItemCheck();
	LM->reset_lib(path_lua52, LUA52);
}

void OnLua53()
{
	g_opt.m_uInterpType = LUA53;
	ResetItemCheck();
	LM->reset_lib(path_lua53, LUA53);
}

void OnLua54()
{
	g_opt.m_uInterpType = LUA54;
	ResetItemCheck();
	LM->reset_lib(path_lua54, LUA54);
}

void OnLuaJIT()
{
	g_opt.m_uInterpType = LUAJIT;
	ResetItemCheck();
	LM->reset_lib(path_luajit, LUA51); // luajit based on Lua5.1
}

// Properly detects if dark mode is enabled (Notepad++ 8.3.4 and above)
void RefreshDarkMode(bool ForceUseDark = false, bool UseDark = false)
{
	// Initialization
	if (!PluginDarkMode::isInitialized())
		PluginDarkMode::initDarkMode();

	bool isDarkModeEnabled = false;

	// Legacy support
	if (ForceUseDark)
		isDarkModeEnabled = UseDark;

	if (!ForceUseDark)
	{
		isDarkModeEnabled = SendNpp(NPPM_ISDARKMODEENABLED, 0, 0);
		PluginDarkMode::Colors newColors;
		bool bSuccess = SendNpp(NPPM_GETDARKMODECOLORS, sizeof(newColors), &newColors);
		if (bSuccess)
		{
			PluginDarkMode::setDarkTone(PluginDarkMode::ColorTone::customizedTone);
			PluginDarkMode::changeCustomTheme(newColors);
		}
	}

	// Set Dark Mode for window/application
	PluginDarkMode::setDarkMode(isDarkModeEnabled, true);
	PluginDarkMode::autoSetupWindowAndChildren(execData.hConsole);
	PluginDarkMode::autoThemeChildControls(GetConsole());
}

void GlobalInitialize()
{
	// Fetch the menu
	execData.hMenu = GetMenu(nppData._nppHandle);

	// Create the docking dialog
	execData.hConsole = CreateDialog(execData.hNPP,
		MAKEINTRESOURCE(IDD_CONSOLE), nppData._nppHandle,
		(DLGPROC)ConsoleProcDlg);

	dockingData.hClient = execData.hConsole;
	dockingData.pszName = ConsoleCaption;
	dockingData.dlgID = 0;
	dockingData.uMask = DWS_DF_CONT_BOTTOM;
	dockingData.pszModuleName = L"";

	// Register the docking dialog
	SendNpp(NPPM_DMMREGASDCKDLG, 0, &dockingData);
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole, MODELESSDIALOGADD);

	SendSci(SCI_SETMOUSEDWELLTIME, 500);

	g_opt.ReadOptions();

	g_opt.m_bConsoleOpenOnInit = !g_opt.m_bConsoleOpenOnInit;
	OpenCloseConsole();

	//	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[AutoClear]._cmdID, g_opt.m_bConsoleAutoclear);
	//	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[PrintTime]._cmdID, g_opt.m_bShowRunTime);

	bool fe51 = !file_exist(path_lua51);
	if (fe51) EnableMenuItem(execData.hMenu, funcItems[LUA51]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	bool fe52 = !file_exist(path_lua52);
	if (fe52) EnableMenuItem(execData.hMenu, funcItems[LUA52]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	bool fe53 = !file_exist(path_lua53);
	if (fe53) EnableMenuItem(execData.hMenu, funcItems[LUA53]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	bool fe54 = !file_exist(path_lua54);
	if (fe54) EnableMenuItem(execData.hMenu, funcItems[LUA54]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	bool fejit = !file_exist(path_luajit);
	if (fejit) EnableMenuItem(execData.hMenu, funcItems[LUAJIT]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	if (fe51 && fe52 && fe53 && fe54 && fejit)
	{
		EnableMenuItem(execData.hMenu, funcItems[CheckSyntax]._cmdID, MF_BYCOMMAND | MF_DISABLED);
		EnableMenuItem(execData.hMenu, funcItems[RunScript]._cmdID, MF_BYCOMMAND | MF_DISABLED);
		EnableMenuItem(execData.hMenu, funcItems[CheckFiles]._cmdID, MF_BYCOMMAND | MF_DISABLED);
	}

	// disable love2d launcher if love.exe not found
	if (!PathFileExists(g_opt.LovePath))
		EnableMenuItem(execData.hMenu, funcItems[RunLove2D]._cmdID, MF_BYCOMMAND | MF_DISABLED);

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
	case LUAJIT:
		OnLuaJIT();
		break;
	case LUA51:
	default:
		OnLua51();
	}

	//OriginalRichEditProc = (WNDPROC)SetWindowLongPtr(GetConsole(), GWLP_WNDPROC, (LONG_PTR)RichEditWndProc);
}

void GlobalDeinitialize()
{
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole, MODELESSDIALOGREMOVE);
	g_opt.SaveOptions();
	//HWND hDlgItem = GetConsole();
	//if (hDlgItem) SetWindowLongPtr(hDlgItem, GWLP_WNDPROC, (LONG_PTR)OriginalRichEditProc);
	if (LM) delete LM;
}

COLORREF start_colorgialog(COLORREF clr)
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
		return m_choose_color.rgbResult;
	}
	return clr;
}

void SetLangOptDialog(HWND hw, BYTE lng)
{
	SendMessage(hw, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? loc_en[14] : loc_ru[14]));
	SendDlgItemMessage(hw, IDC_CHECKLNG, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? L"RU \\ ENG" : L"ENG \\ RU"));
	SendDlgItemMessage(hw, ID_BTNRESET, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? L"Default" : L"По умолчанию"));
	SendDlgItemMessage(hw, IDC_CHECKAUTOCLEAR, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? L" Autoclean Console" : L" Автоочистка консоли"));
	SendDlgItemMessage(hw, IDC_CHECKRUNTIME, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? L" Print run time" : L" Время выполнения"));
	SendDlgItemMessage(hw, IDC_STATIC_RTQ, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lng ? L"Runtime quote (in seconds):" : L"Лимит времени выполнения (в секундах):"));
}

INT_PTR CALLBACK OptionDlgProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		SendDlgItemMessage(hw, IDC_CHECKAUTOCLEAR, BM_SETCHECK, g_opt.m_bConsoleAutoclear, 0);
		SendDlgItemMessage(hw, IDC_CHECKRUNTIME, BM_SETCHECK, g_opt.m_bShowRunTime, 0);
		SendDlgItemMessage(hw, IDC_CHECKCONENC, BM_SETCHECK, g_opt.m_bConEncoding, 0);
		SendDlgItemMessage(hw, IDC_CHECKCONENC, WM_SETTEXT, 0, (LPARAM)(g_opt.m_bConEncoding ? L"UTF8" : L"ANSI"));
		SendDlgItemMessage(hw, IDC_EDITLOVEPATH, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(g_opt.LovePath));
		SendDlgItemMessage(hw, IDC_SPIN1, UDM_SETRANGE32, 0, 15);
		SendDlgItemMessage(hw, IDC_SPIN1, UDM_SETPOS32, 0, g_opt.timequote);

		BYTE lng = g_opt.GetLang();
		SendDlgItemMessage(hw, IDC_CHECKLNG, BM_SETCHECK, lng, 0);
		SetLangOptDialog(hw, lng);
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
			g_opt.clrOK = start_colorgialog(g_opt.clrOK);
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
			start_colorgialog(g_opt.clrOKdm);
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
			g_opt.clrERR = start_colorgialog(g_opt.clrERR);
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
			g_opt.clrERRdm = start_colorgialog(g_opt.clrERRdm);
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
		case IDCANCEL:
		case IDC_BUTTONOK:
			EndDialog(hw, 0);
			break;
		case IDC_CHECKAUTOCLEAR:
		{
			g_opt.m_bConsoleAutoclear = !!SendDlgItemMessage(hw, IDC_CHECKAUTOCLEAR, BM_GETCHECK, 0, 0);
			break;
		}
		case IDC_CHECKCONENC:
		{
			g_opt.m_bConEncoding = !!SendDlgItemMessage(hw, IDC_CHECKCONENC, BM_GETCHECK, 0, 0);
			SendDlgItemMessage(hw, IDC_CHECKCONENC, WM_SETTEXT, 0, (LPARAM)(g_opt.m_bConEncoding ? L"UTF8" : L"ANSI"));
			break;
		}
		case IDC_CHECKRUNTIME:
		{
			g_opt.m_bShowRunTime = !!SendDlgItemMessage(hw, IDC_CHECKRUNTIME, BM_GETCHECK, 0, 0);
			break;
		}
		case IDC_OFDLOVEPATH:
		{
			TCHAR Love2D_path[MAX_PATH]{};
			OPENFILENAME ofn{};
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.lpstrFilter = L"love.exe\0love.exe\0\0"; // filter
			ofn.lpstrTitle = L"Path to 'love.exe'"; // title
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrFile = Love2D_path; // buffer for result
			if (!GetOpenFileName(&ofn)) return TRUE;
			const TCHAR pt[] = L"\\love.exe";
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
		}
	}
	return FALSE;
}

void OnOptions()
{
	DialogBox(execData.hNPP, MAKEINTRESOURCE(IDD_OPTDIALOG), nppData._nppHandle, OptionDlgProc);
}

//void OnSwitchLang()
//{
//	g_opt.OnSwitchLang();
//}

void OnShowAboutDlg()
{
	TCHAR txt[] =
		L" Lua plugin v2.3.4 "
#ifdef _WIN64
		L"(64-bit)"
#else
		L"(32-bit)"
#endif  
		L"\n\n"\
		L" Author: Charsi <charsi2011@gmail.com>\n\n"\
		L" Syntax checking of Lua scripts.\n"\
		L" Run the script from current file.\n"\
		L" Run Love2D game from current directory.\n";
	MessageBox(NULL, txt, L"Lua plugin for Notepad++", MB_OK);
}

void OpenCloseConsole()
{
	g_opt.m_bConsoleOpenOnInit = !g_opt.m_bConsoleOpenOnInit;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[ShowHideConsole]._cmdID, g_opt.m_bConsoleOpenOnInit);
	SendNpp(g_opt.m_bConsoleOpenOnInit ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, execData.hConsole);

	// DarkMode support
	if (g_opt.m_bConsoleOpenOnInit)
		ResetConsoleColors();
}

HWND GetConsole()
{
	return GetDlgItem(execData.hConsole, IDC_RICHEDIT21);
}

void OnClearConsole()
{
	HWND hRE = GetConsole();
	size_t ndx = GetWindowTextLength(hRE);
	SendMessage(hRE, EM_SETSEL, 0, ndx);
	SendMessage(hRE, EM_REPLACESEL, 0, (LPARAM)L"");
	bNeedClear = false;
}

void OnSize()
{
	HWND hRE = GetConsole();
	if (hRE)
	{
		GetClientRect(execData.hConsole, &execData.rcConsole);
		SetWindowPos(hRE, NULL, 0, 0,
			execData.rcConsole.right, execData.rcConsole.bottom, SWP_NOZORDER);
	}
}

void ShowPopup(HWND hwndDlg)
{
	HMENU menu = ::CreatePopupMenu();
	AppendMenu(menu, MF_ENABLED, ID_COPY, g_opt.GetLang() ? L"Copy all" : L"Копировать всё");
	AppendMenu(menu, MF_ENABLED, ID_COPYSELECTED, g_opt.GetLang() ? L"Copy selected" : L"Копировать выделенное");
	AppendMenu(menu, MF_ENABLED, ID_CLEARCONSOLE, g_opt.GetLang() ? L"Clear" : L"Очистить");
	AppendMenu(menu, MF_SEPARATOR, 0, L"");
	AppendMenu(menu, MF_ENABLED, ID_CLOSECONSOLE, g_opt.GetLang() ? L"Close" : L"Закрыть");

	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(menu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwndDlg, NULL);
	PostMessage(hwndDlg, WM_NULL, 0, 0);
	DestroyMenu(menu);
}

bool str2Clipboard(const TCHAR* str2cpy, HWND hwnd)
{
	size_t len2Allocate = (lstrlen(str2cpy) + 1) * sizeof(TCHAR);
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len2Allocate);
	if (hglbCopy == NULL)
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
	TCHAR* pStr = (TCHAR*)::GlobalLock(hglbCopy);
	if (pStr == NULL)
	{
		::GlobalUnlock(hglbCopy);
		::GlobalFree(hglbCopy);
		::CloseClipboard();
		return false;
	}
	wcscpy_s(pStr, len2Allocate / sizeof(TCHAR), str2cpy);
	::GlobalUnlock(hglbCopy);
	// Place the handle on the clipboard.
	if (::SetClipboardData(CF_UNICODETEXT, hglbCopy) == NULL)
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
	int len = GetWindowTextLength(hRE) + 1;
	TCHAR* tmp = new TCHAR[len]{};
	GETTEXTEX gtex{};
	gtex.cb = static_cast<DWORD>(len * sizeof(TCHAR));// size in bytes
	gtex.codepage = 1200; // 1200 for UNICODE
	gtex.flags = selected ? GT_SELECTION : GT_DEFAULT;
	SendMessage(hRE, EM_GETTEXTEX, (WPARAM)&gtex, (LPARAM)tmp);
	str2Clipboard(tmp, execData.hConsole);
	delete[] tmp;
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
				is_floating = 8;
				break;
			case DMN_DOCK:  // docking dialog; HIWORD(pnmh->code) is dockstate [0..3]
				is_floating = (BYTE)HIWORD(pnmh->code);
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
		ShowPopup(hwndDlg);
		return TRUE;
	case WM_SIZE:
		OnSize();
		break;
	}
	return FALSE;
}

void AddStr(TCHAR* msg)
{
	if (!msg) return;
	HWND hRE = GetConsole();
	int ndx = GetWindowTextLength(hRE);
	SendMessage(hRE, EM_SETSEL, ndx, ndx);
	SendMessage(hRE, EM_REPLACESEL, 0, (LPARAM)msg);
}

//void OnPrintTime()
//{
//	g_opt.m_bShowRunTime = !g_opt.m_bShowRunTime;
//	//SendNpp(NPPM_SETMENUITEMCHECK, funcItems[PrintTime]._cmdID, g_opt.m_bShowRunTime);
//}

//void OnAutoClear()
//{
//	g_opt.m_bConsoleAutoclear = !g_opt.m_bConsoleAutoclear;
//	//SendNpp(NPPM_SETMENUITEMCHECK, funcItems[AutoClear]._cmdID, g_opt.m_bConsoleAutoclear);
//}

bool not_valid_document(const TCHAR* ws_full_path)
{
	// save file
	SendNpp(NPPM_SAVECURRENTFILE, 0, 0);
	if (ws_full_path[1] != ':' && ws_full_path[2] != '\\') return true;
	// get language type
	LangType docType = LangType::L_EXTERNAL;
	SendNpp(NPPM_GETCURRENTLANGTYPE, 0, &docType);
	if (docType != LangType::L_LUA) return true; // не Lua скрипт!
	return false;
}

void AddMarker(int line)
{
	const bool stat = line == 0;
	bNeedClear = true;
	if (stat) return;
	ColorModeState = ColorMode::ERR;
	line--;
	SendSci(SCI_GOTOLINE, line);
	//SendSci(SCI_SETEMPTYSELECTION);
	SendSci(SCI_MARKERDELETE, line, SC_MARK_ARROWS); // remove mark if added
	SendSci(SCI_MARKERADD, line, SC_MARK_ARROWS);
	SetFocus(GetCurrentScintilla());
}

void OnCheckSyntax()
{
	TCHAR ws_full_path[MAX_PATH]{};
	SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path);
	if (not_valid_document(ws_full_path)) return;

	OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	char full_path[MAX_PATH];
	SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);
	int line = LM->process(full_path);
	ColorModeState = ColorMode::OK;
	AddMarker(line);
	SetConsoleTextColor();
}

void OnRunScript()
{
	TCHAR ws_full_path[MAX_PATH]{};
	SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path);
	if (not_valid_document(ws_full_path)) return;

	if (g_opt.m_bConsoleAutoclear || bNeedClear) OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	char full_path[MAX_PATH]{};
	SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);

	clock_t run_time = clock();
	int rslt = LM->process(full_path, true);
	run_time = clock() - run_time;

	ColorModeState = ColorMode::NORMAL;
	AddMarker(rslt);
	if (!rslt)
	{
		TCHAR str[MAX_PATH]{};
		SYSTEMTIME sTime;
		GetLocalTime(&sTime);

		wsprintf(str, L"Success: %02d:%02d:%02d", sTime.wHour, sTime.wMinute, sTime.wSecond);
		AddStr(str);

		if (g_opt.m_bShowRunTime)
		{
			wsprintf(str, L"\r\nRuntime: %d ms", run_time);
			AddStr(str);
		}

		bNeedClear = false;
	}
	SetConsoleTextColor();
}

void OnCheckFiles()
{
	TCHAR ws_full_path[MAX_PATH]{};
	SendNpp(NPPM_GETFULLCURRENTPATH, MAX_PATH, ws_full_path); // full file path
	if (not_valid_document(ws_full_path)) return;

	//if (g_opt.m_bConsoleAutoclear)
	OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	TCHAR f_ext[MAX_PATH]{};
	SendNpp(NPPM_GETEXTPART, MAX_PATH, f_ext);
	TCHAR ws_pattern[MAX_PATH]{};
	SendNpp(NPPM_GETCURRENTDIRECTORY, MAX_PATH, ws_full_path); // current dir

	wsprintf(ws_pattern, L"%s%s%s", ws_full_path, L"\\*", f_ext);
	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(ws_pattern, &f);
	if (h != INVALID_HANDLE_VALUE)
	{
		int _cnt = 0;
		bool stat = true;
		TCHAR ws_full_filepath[MAX_PATH]{};
		do
		{
			wsprintf(ws_full_filepath, L"%s%s%s", ws_full_path, L"\\", f.cFileName);
			char full_filepath[MAX_PATH]{};
			SysUniConv::UnicodeToMultiByte(full_filepath, MAX_PATH, ws_full_filepath);
			bool isOK = !LM->process(full_filepath);
			stat = stat && isOK;
			if (!isOK) _cnt++;
		} while (FindNextFile(h, &f));

		wsprintf(ws_full_filepath, L"\r\nDone! Found %d file(s) with errors.", _cnt);
		AddStr(ws_full_filepath);
		ColorModeState = stat ? ColorMode::OK : ColorMode::ERR;
		SetConsoleTextColor();
	}
}
#ifdef TEST_ITEM
void OnTestItem()
{
	OnClearConsole();
	AddStr(L"on test item clicked");
}
#endif

///////////////////////////////////////////////////
// Main
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved)
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		execData.hNPP = (HINSTANCE)hModule;
		// init menu
		InitFuncItem(CheckSyntax, loc_ru[CheckSyntax], OnCheckSyntax);
		InitFuncItem(RunScript, loc_ru[RunScript], OnRunScript);
		InitFuncItem(CheckFiles, loc_ru[CheckFiles], OnCheckFiles);
		InitFuncItem(RunLove2D, loc_ru[RunLove2D], OnLove2D);
		//InitFuncItem(Separator1, L"");
		InitFuncItem(LUA51, loc_ru[LUA51], OnLua51);
		InitFuncItem(LUA52, loc_ru[LUA52], OnLua52);
		InitFuncItem(LUA53, loc_ru[LUA53], OnLua53);
		InitFuncItem(LUA54, loc_ru[LUA54], OnLua54);
		InitFuncItem(LUAJIT, loc_ru[LUAJIT], OnLuaJIT);
		//InitFuncItem(Separator2, L"");
		InitFuncItem(ShowHideConsole, loc_ru[ShowHideConsole], OpenCloseConsole);
		//InitFuncItem(ClearConsole, loc_ru[ClearConsole], OnClearConsole);
		//InitFuncItem(AutoClear, loc_ru[AutoClear], OnAutoClear);
		//InitFuncItem(PrintTime, loc_ru[PrintTime], OnPrintTime);
		//InitFuncItem(Separator3, L"");
		//InitFuncItem(SwitchLang, loc_ru[SwitchLang], OnSwitchLang);
		InitFuncItem(Options, loc_ru[Options], OnOptions);
		InitFuncItem(About, loc_ru[About], OnShowAboutDlg);
#ifdef TEST_ITEM
		InitFuncItem(TestItem, loc_ru[TestItem], OnTestItem);
#endif
		m_hRichEditDll = LoadLibrary(L"Riched20.dll");
		break;

	case DLL_PROCESS_DETACH:
		if (m_hRichEditDll)
			FreeLibrary(m_hRichEditDll);
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return sPluginName;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int* nbF)
{
	*nbF = nbFunc;
	return funcItems;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification * notifyCode)
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
		RefreshDarkMode();
		ResetConsoleColors();
		break;

	default:
		break;
	}
}

extern "C" __declspec(dllexport)
LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}