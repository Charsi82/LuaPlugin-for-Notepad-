///////// encoding ///////////
#include "PluginSettings.h"

int SysUniConv::str_unsafe_len(const char* str)
{
	if (!str) return 0;
	const char* str0 = str;
	while (*str)  ++str;
	return ((int)(str - str0));
}

int SysUniConv::strw_unsafe_len(const wchar_t* strw)
{
	if (!strw) return 0;
	const wchar_t* strw0 = strw;
	while (*strw)  ++strw;
	return ((int)(strw - strw0));
}

int SysUniConv::MultiByteToUnicode(wchar_t* wStr, int wMaxLen, const char* aStr,
	int aLen, UINT aCodePage)
{
	if (aStr && wStr && (wMaxLen > 0))
	{
		if (aLen < 0)
			aLen = str_unsafe_len(aStr);
		if (aLen > 0)
		{
			return a2w(wStr, wMaxLen, aStr, aLen, aCodePage);
		}
	}
	if (wStr && (wMaxLen > 0))
		wStr[0] = 0;
	return 0;
}

int SysUniConv::UnicodeToMultiByte(char* aStr, int aMaxLen, const wchar_t* wStr,
	int wLen, UINT aCodePage)
{
	if (wStr && aStr && (aMaxLen > 0))
	{
		if (wLen < 0)
			wLen = strw_unsafe_len(wStr);
		if (wLen > 0)
		{
			return w2a(aStr, aMaxLen, wStr, wLen, aCodePage);
		}
	}
	if (aStr && (aMaxLen > 0))
		aStr[0] = 0;
	return 0;
}

int SysUniConv::a2w(wchar_t* ws, int wml, const char* as, int al, UINT acp)
{
	int len = ::MultiByteToWideChar(acp, 0, as, al, ws, wml);
	ws[len] = 0;
	return len;
}

int SysUniConv::w2a(char* as, int aml, const wchar_t* ws, int wl, UINT acp)
{
	int len = ::WideCharToMultiByte(acp, 0, ws, wl, as, aml, NULL, NULL);
	as[len] = 0;
	return len;
}

int SysUniConv::u2a(char* as, int aml, const char* us, int ul, UINT acp)
{
	int      len = 0;
	wchar_t* ws = new wchar_t[ul + 1];
	if (ws)
	{
		len = ::MultiByteToWideChar(CP_UTF8, 0, us, ul, ws, ul);
		ws[len] = 0;
		len = ::WideCharToMultiByte(acp, 0, ws, len, as, aml, NULL, NULL);
		delete[] ws;
	}
	as[len] = 0;
	return len;
}

int SysUniConv::UTF8ToMultiByte(char* aStr, int aMaxLen, const char* uStr,
	int uLen, UINT aCodePage)
{
	if (uStr && aStr && (aMaxLen > 0))
	{
		if (uLen < 0)
			uLen = str_unsafe_len(uStr);
		if (uLen > 0)
		{
			return u2a(aStr, aMaxLen, uStr, uLen, aCodePage);
		}
	}
	if (aStr && (aMaxLen > 0))
		aStr[0] = 0;
	return 0;
}

