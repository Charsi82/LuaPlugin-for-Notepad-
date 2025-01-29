#pragma once

#include "PluginInterface.h"
#include "Docking.h"
//#include <map>
constexpr auto sPluginName = L"Lua utils";

struct ExecData {
	HINSTANCE hNPP{};
	HWND hConsole{};
	HMENU hMenu{};
	wchar_t ConsoleCaption[20]{};
	//std::map<enumNFuncItems, ShortcutKey> hkeys;
};

// Extern Variables
extern NppData nppData;
extern ExecData execData;
extern FuncItem funcItems[];
extern tTbData dockingData;

void InitFuncItem(int            nItem,
	//const wchar_t *   szName, 
	PFUNCPLUGINCMD pFunc = nullptr,
	ShortcutKey*   pShortcut = nullptr);

// Plugin Functions
LRESULT SendSci(UINT iMessage, WPARAM wParam = 0, LPARAM lParam = 0);
HWND GetCurrentScintilla();
void GlobalInitialize();
void GlobalDeinitialize();
void AddStr(const wchar_t * msg);
HWND GetConsole();
void OnClearConsole();
void OpenCloseConsole();
BOOL CALLBACK ConsoleProcDlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

class SysUniConv
{
private:
	static int a2w(wchar_t* ws, int wml, const char* as, int al, UINT acp);
	static int w2a(char* as, int aml, const wchar_t* ws, int wl, UINT acp);
	static int u2a(char* as, int aml, const char* us, int ul, UINT acp);

public:
	static int str_unsafe_len( const char* str );
	static int strw_unsafe_len( const wchar_t* strw );
	static int UTF8ToMultiByte(char* aStr, int aMaxLen, const char* uStr, int uLen = -1, UINT aCodePage = CP_ACP);
	static int MultiByteToUnicode(wchar_t* wStr, int wMaxLen, const char* aStr, int aLen = -1, UINT aCodePage = CP_ACP);
	static int UnicodeToMultiByte(char* aStr, int aMaxLen, const wchar_t* wStr, int wLen = -1, UINT aCodePage = CP_ACP);
};
