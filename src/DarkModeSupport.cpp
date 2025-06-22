#include "Notepad_plus_msgs.h"
#include "DarkModeSupport.hpp"
#include "PluginOptions.hpp"
#include "PluginSettings.hpp"

extern CPluginOptions g_opt;
extern NppData nppData;
#define SendNpp(msg, wParam, lParam) SendMessage(nppData._nppHandle, (msg), (WPARAM)wParam, (LPARAM)lParam)

namespace {

	struct NppDMColors
	{
		COLORREF background = 0;
		COLORREF softerBackground = 0; // ctrl background color
		COLORREF hotBackground = 0;
		COLORREF pureBackground = 0;   // dlg background color
		COLORREF errorBackground = 0;
		COLORREF text = 0;
		COLORREF darkerText = 0;
		COLORREF disabledText = 0;
		COLORREF linkText = 0;
		COLORREF edge = 0;
		COLORREF hotEdge = 0;
		COLORREF disabledEdge = 0;
	};

	inline bool isDarkModeActive() noexcept
	{
		return SendNpp(NPPM_ISDARKMODEENABLED, 0, 0);
	}

	COLORREF GetDMColor(DMColorType type) noexcept
	{
		NppDMColors colors{};
		SendNpp(NPPM_GETDARKMODECOLORS, sizeof(NppDMColors), &colors);
		switch (type)
		{
		case DMColorType::background:
			return colors.background;
		case DMColorType::softerBackground:
			return colors.softerBackground;
		case DMColorType::hotBackground:
			return colors.hotBackground;
		case DMColorType::pureBackground:
			return colors.pureBackground;
		case DMColorType::errorBackground:
			return colors.errorBackground;
		case DMColorType::text:
			return colors.text;
		case DMColorType::darkerText:
			return colors.darkerText;
		case DMColorType::disabledText:
			return colors.disabledText;
		case DMColorType::linkText:
			return colors.linkText;
		case DMColorType::edge:
			return colors.edge;
		case DMColorType::hotEdge:
			return colors.hotEdge;
		case DMColorType::disabledEdge:
			return colors.disabledEdge;
		default:
			return colors.softerBackground;
		}
	}

}

COLORREF GetNormalColor() noexcept
{
	return isDarkModeActive() ? GetDMColor(DMColorType::text) : GetSysColor(COLOR_MENUTEXT);
}

COLORREF GetOKColor() noexcept
{
	return isDarkModeActive() ? g_opt.clrOKdm : g_opt.clrOK;
}

COLORREF GetERRColor() noexcept
{
	return isDarkModeActive() ? g_opt.clrERRdm : g_opt.clrERR;
}

COLORREF GetBackGroundColor() noexcept
{
	return isDarkModeActive() ? GetDMColor(DMColorType::softerBackground) : GetSysColor(COLOR_MENU);
}