#pragma once

#include "PluginInterface.h"
#include "Docking.h"
#include <string>

constexpr auto sPluginName = L"Lua utils";

struct ExecData
{
	HINSTANCE hNPP{};
	HWND hConsole{};
	HMENU hMenu{};
	wchar_t ConsoleCaption[12]{ L"Lua Console" };
	wchar_t addInfo[8]{};
};

void InitFuncItem(int	nItem,
	const wchar_t *		szName = nullptr, 
	PFUNCPLUGINCMD		pFunc = nullptr,
	ShortcutKey*		pShortcut = nullptr);

// Plugin Functions
LRESULT SendSci(UINT iMessage, WPARAM wParam = 0, LPARAM lParam = 0) noexcept;
HWND GetCurrentScintilla();
void GlobalInitialize();
void GlobalDeinitialize();
void AddStr(const wchar_t * msg);
void OnClearConsole();

BOOL CALLBACK ConsoleProcDlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

namespace SysUniConv
{
	std::wstring MultibyteToWideChar(const char* ansiString, UINT codePage = CP_ACP);
	std::string WideCharToMultibyte(const wchar_t* wideStr, UINT codePage = CP_ACP);
	std::string AnsiToUtf8(const char* ansi_string);
	std::string Utf8ToAnsi(const char* utf8_string);
};
