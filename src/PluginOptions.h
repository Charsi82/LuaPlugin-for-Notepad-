#pragma once

#include "helpers.h"
constexpr auto OptSectName = L"Options";
constexpr auto OptFlagsKey = L"Flags";
constexpr auto OptLangKey = L"Language";
constexpr auto OptLuaKey = L"Lua";
constexpr auto OptLovePath = L"LovePath";
constexpr auto OptClrOKKey = L"colorOK";
constexpr auto OptClrOKDarkKey = L"colorOK_dark";
constexpr auto OptClrERRKey = L"colorERR";
constexpr auto OptClrERRDarkKey = L"colorERR_dark";
constexpr auto OptTimeQuoteKey = L"TimeQuote";

const COLORREF DEFCOLOROK = 0x008000;
const COLORREF DEFCOLORDMOK = 0x00FA0A;
const COLORREF DEFCOLORERR = 0x00080;
const COLORREF DEFCOLORDMERR = 0x00FFFF;

class CPluginOptions
{
public:
	CPluginOptions();

	bool m_bConsoleOpenOnInit; //открывать при запуске
	bool m_bConsoleClearOnRun;
	bool m_bConsoleAutoclear;
	bool m_bShowRunTime;
	bool m_bConEncoding;
	BYTE  m_uInterpType; // Lua interpretator type

	void ReadOptions();
	void SaveOptions();
	bool MustBeSaved() const;

	void OnSwitchLang();
	BYTE GetLang() const {
		return m_uLang;
	}
	void ApplyLang();
	BYTE m_uFlags;
	TCHAR LovePath[MAX_PATH];
	int timequote;
	COLORREF clrOKdm;
	COLORREF clrOK;
	COLORREF clrERRdm;
	COLORREF clrERR;

	const TCHAR* get_path() {
		return szIniFilePath;
	};
private:
	BYTE getOptFlags() const;
	TCHAR szIniFilePath[MAX_PATH];

	enum eOptConsts {
		OPTF_CONOPENONINIT = 0x01,
		OPTF_CONCLEARONRUN = 0x02,
		OPTF_CONAUTOCLEAR = 0x04,
		OPTF_PRINTRUNTIME = 0x08,
		OPTF_CONENCODING = 0x10
	};
	BYTE  m_uFlags0;
	BYTE  m_uLang;
	BYTE  m_uLang0;
	BYTE  m_uInterpType0;
};
