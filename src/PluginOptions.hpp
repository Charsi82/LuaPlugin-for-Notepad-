#pragma once

#include <windows.h>
#include <string>
#include <vector>

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
constexpr auto OptAFOptions = L"AutoFormatOptions";
constexpr auto OptAFColumnLimit = L"OptAFColumnLimit";
constexpr auto OptAFIndentWidth = L"OptAFIndentWidth";
constexpr auto OptAFEolType = L"OptAFEolType";
constexpr auto OptTabWidth = L"OptTabWidth";
constexpr auto OptContIndentWidth = L"OptContIndentWidth";
constexpr auto OptSpacesBeforeCall = L"OptSpacesBeforeCall";
constexpr auto OptColumnTableLimit = L"OptColumnTableLimit";
constexpr auto OptColumnTableLimitKV = L"OptColumnTableLimitKV";
constexpr auto OptLineBreaks = L"OptLineBreaks";
constexpr auto OptTabSep = L"OptTabSep";
constexpr auto OptERRVERB = L"ErrVerb";

const COLORREF DEFCOLOROK = 0x008000;
const COLORREF DEFCOLORDMOK = 0x00FA0A;
const COLORREF DEFCOLORERR = 0x00080;
const COLORREF DEFCOLORDMERR = 0x00FFFF;

enum AFOptions
{
	UseTabs,
	KeepSimpleBlockOneLine,
	KeepSimpleFunctionOneLine,
	AlighArgs,
	BreakAfterFCLP,
	BreakBeforeFCRP,
	AlignParams,
	ChopDownParams,
	BreakAfterFDLP,
	BreakBeforeFDRP,
	AlignTableFields,
	BreakAfterTLB,
	BreakBeforeTRB,
	ChopDownTable,
	ChopDownKVTable,
	ExtraSep,
	BreakAfterOperator,
	//DQtoSQ,
	TransformLiterals_Enabled,
	TransformLiterals,
	SpaceInFuncDefParens,
	SpaceInFuncCallParens,
	SpaceInsideTableBraces,
	SpaceAroundEquals,
	DumpConfig,
};

enum EOLTYPES
{
	EOL_INPUT,
	EOL_OS,
	EOL_WIN,
	EOL_Unx,
	EOL_Mac,
};

class CPluginOptions
{
public:
	CPluginOptions();

	bool m_bConsoleOpen; //открывать при запуске
	bool m_bConsoleClearOnRun;
	bool m_bConsoleAutoclear;
	bool m_bShowRunTime;
	bool m_bConEncoding;
	BYTE  m_uInterpType; // Lua interpretator type

	BYTE m_uFlags;
	wchar_t  LovePath[MAX_PATH];
	int timequote;
	COLORREF clrOKdm;
	COLORREF clrOK;
	COLORREF clrERRdm;
	COLORREF clrERR;
	UINT32 m_afopts;

	void ReadOptions(int nDefaultLang);
	void SaveOptions();
	bool MustBeSaved() const;

	void OnSwitchLang();
	BYTE GetLang() const { return m_uLang; }
	const std::vector<std::string> get_autoformat_options();
	std::string get_line_sep();
	void set_line_sep(const std::string& s);
	UINT get_column_limit();
	void set_column_limit(UINT);
	bool get_autoformat_option(AFOptions OptType) const;
	void set_autoformat_option(AFOptions OptType, bool state);
	int get_indent_width();
	void set_indent_width(int);
	int get_tab_width();
	void set_tab_width(int);
	int get_continuation_indent_width();
	void set_continuation_indent_width(int);
	int get_spaces_before_call();
	void set_spaces_before_call(int);
	std::string get_table_sep();
	bool tabsep_is_comma() const;
	void set_table_sep(bool is_comma);
	int get_column_table_limit();
	void set_column_table_limit(int);
	int get_line_breaks();
	void set_line_breaks(int);
	int get_column_table_limit_kv();
	void set_column_table_limit_kv(int);
	void reset_af_options();
	int get_err_verbose() const;
	void set_err_verbose(int);

private:
	BYTE getOptFlags() const;
	wchar_t  szIniFilePath[MAX_PATH];

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
	UINT m_column_limit;
	EOLTYPES m_eol_type;
	int m_indent_width;
	int m_tab_width;
	int m_continuation_indent_width;
	int m_spaces_before_call;
	int m_column_table_limit;
	int m_column_table_limit_kv;
	int m_line_breaks;
	bool tab_sep;
	int m_verbose;
};
