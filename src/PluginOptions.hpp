#pragma once

#include <windows.h>
#include <string>
#include <vector>

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
	std::wstring LovePath;
	UINT32 m_timequote;
	COLORREF clrOKdm;
	COLORREF clrOK;
	COLORREF clrERRdm;
	COLORREF clrERR;
	UINT32 m_afopts;
	void ReadOptions(BYTE defLang);
	void SaveOptions();

	BYTE GetLuaType() const { return m_uInterpType; }
	void SetLuaType(BYTE value) { m_uInterpType = value; }
	void OnSwitchLang();
	BYTE GetLang() const { return m_uLang; }
	const std::vector<std::string> get_autoformat_options();
	const char* get_line_sep();
	void set_line_sep(const std::string& s);
	UINT get_column_limit() const;
	void set_column_limit(UINT);
	bool get_autoformat_option(AFOptions OptType) const;
	void set_autoformat_option(AFOptions OptType, bool state);
	int get_indent_width() const;
	void set_indent_width(int);
	int get_tab_width() const;
	void set_tab_width(int);
	int get_continuation_indent_width() const;
	void set_continuation_indent_width(int);
	int get_spaces_before_call() const;
	void set_spaces_before_call(int);
	const char* get_table_sep() const;
	bool tabsep_is_comma() const;
	void set_table_sep(bool is_comma);
	int get_column_table_limit() const;
	void set_column_table_limit(int);
	int get_line_breaks() const;
	void set_line_breaks(int);
	int get_column_table_limit_kv() const;
	void set_column_table_limit_kv(int);
	int get_err_verbose() const;
	void set_err_verbose(int);
	void reset_af_options();
	void reset_colors();

private:
	BYTE getOptFlags() const;

	enum eOptConsts
	{
		OPTF_CONOPENONINIT = 0x01,
		OPTF_CONCLEARONRUN = 0x02,
		OPTF_CONAUTOCLEAR = 0x04,
		OPTF_PRINTRUNTIME = 0x08,
		OPTF_CONENCODING = 0x10
	};
	BYTE  m_uLang;
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
