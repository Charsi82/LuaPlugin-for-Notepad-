#include "lua_main.h"

CPluginOptions::CPluginOptions() :
	m_bConsoleOpenOnInit(false), m_bConsoleClearOnRun(false),
	m_bConsoleAutoclear(false), m_bShowRunTime(false), m_bConEncoding(true),
	m_uLang0(0), m_uLang(0),
	m_uInterpType0(LUA51), m_uInterpType(LUA51),
	m_uFlags0(0), m_uFlags(0), timequote(3),
	clrOKdm(0), clrOK(0), clrERRdm(0), clrERR(0)
{
	ZeroMemory(szIniFilePath, MAX_PATH);
	ZeroMemory(LovePath, MAX_PATH);
}

BYTE CPluginOptions::getOptFlags() const
{
	BYTE uFlags = 0;
	if (m_bConsoleOpenOnInit)
		uFlags |= OPTF_CONOPENONINIT;
	if (m_bConsoleClearOnRun)
		uFlags |= OPTF_CONCLEARONRUN;
	if (m_bConsoleAutoclear)
		uFlags |= OPTF_CONAUTOCLEAR;
	if (m_bShowRunTime)
		uFlags |= OPTF_PRINTRUNTIME;
	if (m_bConEncoding)
		uFlags |= OPTF_CONENCODING;
	return uFlags;
}

bool CPluginOptions::MustBeSaved() const
{
	return (getOptFlags() != m_uFlags0) || (m_uLang != m_uLang0) || (m_uInterpType != m_uInterpType0);
}

void CPluginOptions::ReadOptions()
{
	// get path to config
	TCHAR m_szDllFileName[MAX_PATH]{};
	UINT nLen = GetModuleFileName((HMODULE)execData.hNPP, szIniFilePath, MAX_PATH);
	while (nLen-- > 0)
		if ((szIniFilePath[nLen] == L'\\') || (szIniFilePath[nLen] == L'/'))
		{
			lstrcpy(m_szDllFileName, szIniFilePath + nLen + 1);
			break;
		}

	nLen = lstrlen(m_szDllFileName) - 3;
	lstrcpy(m_szDllFileName + nLen, L"ini");

	// read options
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, szIniFilePath);
	lstrcat(szIniFilePath, L"\\");
	lstrcat(szIniFilePath, m_szDllFileName);

	m_uFlags0 = (BYTE)GetPrivateProfileInt(OptSectName, OptFlagsKey, -1, szIniFilePath);
	if (m_uFlags0 != (BYTE)-1)
	{
		m_bConsoleOpenOnInit = !!(m_uFlags0 & OPTF_CONOPENONINIT);
		m_bConsoleClearOnRun = !!(m_uFlags0 & OPTF_CONCLEARONRUN);
		m_bConsoleAutoclear = !!(m_uFlags0 & OPTF_CONAUTOCLEAR);
		m_bShowRunTime = !!(m_uFlags0 & OPTF_PRINTRUNTIME);
		m_bConEncoding = !!(m_uFlags0 & OPTF_CONENCODING);
	}
	timequote = GetPrivateProfileInt(OptSectName, OptTimeQuoteKey, 3, szIniFilePath);
	GetPrivateProfileString(OptSectName, OptLovePath, L"C:\\Program Files\\LOVE\\love.exe", LovePath, MAX_PATH, szIniFilePath);
	m_uInterpType0 = (BYTE)GetPrivateProfileInt(OptSectName, OptLuaKey, LUA51, szIniFilePath);
	m_uInterpType = m_uInterpType0;
	m_uLang0 = (BYTE)GetPrivateProfileInt(OptSectName, OptLangKey, 0, szIniFilePath);
	m_uLang = m_uLang0;
	clrOK = GetPrivateProfileInt(OptSectName, OptClrOKKey, DEFCOLOROK, szIniFilePath);
	clrOKdm = GetPrivateProfileInt(OptSectName, OptClrOKDarkKey, DEFCOLORDMOK, szIniFilePath);
	clrERR = GetPrivateProfileInt(OptSectName, OptClrERRKey, DEFCOLORERR, szIniFilePath);
	clrERRdm = GetPrivateProfileInt(OptSectName, OptClrERRDarkKey, DEFCOLORDMERR, szIniFilePath);
	ApplyLang();
}

void CPluginOptions::SaveOptions()
{
	TCHAR szNum[10]{};
	if (MustBeSaved())
	{
		BYTE uFlags = getOptFlags();
		wsprintf(szNum, L"%u", uFlags);
		if (WritePrivateProfileString(OptSectName, OptFlagsKey, szNum, szIniFilePath))
			m_uFlags0 = uFlags;

		wsprintf(szNum, L"%u", m_uLang);
		if (WritePrivateProfileString(OptSectName, OptLangKey, szNum, szIniFilePath))
			m_uLang0 = m_uLang;

		wsprintf(szNum, L"%u", m_uInterpType);
		if (WritePrivateProfileString(OptSectName, OptLuaKey, szNum, szIniFilePath))
			m_uInterpType0 = m_uInterpType;
	}
	WritePrivateProfileString(OptSectName, OptLovePath, LovePath, szIniFilePath);

	const TCHAR hexfmt[] = L"0x%06X";
	wsprintf(szNum, hexfmt, clrOK);
	WritePrivateProfileString(OptSectName, OptClrOKKey, szNum, szIniFilePath);
	wsprintf(szNum, hexfmt, clrOKdm);
	WritePrivateProfileString(OptSectName, OptClrOKDarkKey, szNum, szIniFilePath);
	wsprintf(szNum, hexfmt, clrERR);
	WritePrivateProfileString(OptSectName, OptClrERRKey, szNum, szIniFilePath);
	wsprintf(szNum, hexfmt, clrERRdm);
	WritePrivateProfileString(OptSectName, OptClrERRDarkKey, szNum, szIniFilePath);
	wsprintf(szNum, L"%d", timequote);
	WritePrivateProfileString(OptSectName, OptTimeQuoteKey, szNum, szIniFilePath);
}

void CPluginOptions::OnSwitchLang()
{
	m_uLang = (m_uLang + 1) % 2;
	ApplyLang();
}

void CPluginOptions::ApplyLang()
{
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fType = MFT_STRING;
	info.fMask = MIIM_TYPE;

	TCHAR buf[MAX_PATH]{};
	TCHAR hot_key[MAX_PATH]{};
	info.dwTypeData = buf;
	for (int i = 0; i < nbFunc; i++)
		if (funcItems[i]._pFunc)
		{
			GetMenuString(execData.hMenu, funcItems[i]._cmdID, buf, MAX_PATH, FALSE);
			hot_key[0] = 0;
			TCHAR* hk = wcschr(buf, L'\t');
			if (hk) lstrcpy(hot_key, hk);
			wsprintf(buf, L"%s%s", (m_uLang ? loc_en : loc_ru)[i], hot_key);
			SetMenuItemInfo(execData.hMenu, funcItems[i]._cmdID, FALSE, &info);
		}
}
