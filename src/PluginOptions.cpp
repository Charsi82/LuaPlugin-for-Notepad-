#include "lua_main.hpp"
#include <sstream>

extern NppData nppData;
extern ExecData execData;

CPluginOptions::CPluginOptions() :
	m_bConsoleOpen(false), m_bConsoleClearOnRun(false),
	m_bConsoleAutoclear(false), m_bShowRunTime(false), m_bConEncoding(true),
	m_uLang0(0), m_uLang(0),
	m_uInterpType0(LUA51), m_uInterpType(LUA51),
	m_uFlags0(0), m_uFlags(0), timequote(3),
	clrOKdm(0), clrOK(0), clrERRdm(0), clrERR(0), m_afopts(0),
	m_column_limit(300), m_eol_type(EOL_INPUT), m_indent_width(4),
	m_tab_width(4), m_continuation_indent_width(4), m_spaces_before_call(1),
	m_column_table_limit(0), m_column_table_limit_kv(0), m_line_breaks(1), tab_sep(true), m_verbose(0)
{
	ZeroMemory(szIniFilePath, MAX_PATH);
	ZeroMemory(LovePath, MAX_PATH);
}

void CPluginOptions::set_err_verbose(int val)
{
	m_verbose = val;
}

int CPluginOptions::get_err_verbose() const
{
	return m_verbose;
}

BYTE CPluginOptions::getOptFlags() const
{
	BYTE uFlags = 0;
	if (m_bConsoleOpen)
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

void CPluginOptions::ReadOptions(int nDefaultLang)
{
	// get path to config
	wchar_t m_szDllFileName[MAX_PATH]{};
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
	SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(szIniFilePath));

	lstrcat(szIniFilePath, L"\\");
	lstrcat(szIniFilePath, m_szDllFileName);
	const BYTE INVALID_VALUE = static_cast<BYTE>(-1);
	m_uFlags0 = static_cast<BYTE>(GetPrivateProfileInt(OptSectName, OptFlagsKey, INVALID_VALUE, szIniFilePath));
	if (m_uFlags0 != INVALID_VALUE)
	{
		m_bConsoleOpen = !!(m_uFlags0 & OPTF_CONOPENONINIT);
		m_bConsoleClearOnRun = !!(m_uFlags0 & OPTF_CONCLEARONRUN);
		m_bConsoleAutoclear = !!(m_uFlags0 & OPTF_CONAUTOCLEAR);
		m_bShowRunTime = !!(m_uFlags0 & OPTF_PRINTRUNTIME);
		m_bConEncoding = !!(m_uFlags0 & OPTF_CONENCODING);
	}

	timequote = GetPrivateProfileInt(OptSectName, OptTimeQuoteKey, 3, szIniFilePath);
	GetPrivateProfileString(OptSectName, OptLovePath, L"C:\\Program Files\\LOVE\\love.exe", LovePath, MAX_PATH, szIniFilePath);
	m_uInterpType0 = static_cast<BYTE>(GetPrivateProfileInt(OptSectName, OptLuaKey, LUA51, szIniFilePath));
	m_uInterpType = m_uInterpType0;
	m_uLang0 = static_cast<BYTE>(GetPrivateProfileInt(OptSectName, OptLangKey, nDefaultLang, szIniFilePath));
	m_uLang = m_uLang0;
	clrOK = GetPrivateProfileInt(OptSectName, OptClrOKKey, DEFCOLOROK, szIniFilePath);
	clrOKdm = GetPrivateProfileInt(OptSectName, OptClrOKDarkKey, DEFCOLORDMOK, szIniFilePath);
	clrERR = GetPrivateProfileInt(OptSectName, OptClrERRKey, DEFCOLORERR, szIniFilePath);
	clrERRdm = GetPrivateProfileInt(OptSectName, OptClrERRDarkKey, DEFCOLORDMERR, szIniFilePath);

	// af options
	reset_af_options();
	m_afopts = static_cast<UINT32>(GetPrivateProfileInt(OptSectName, OptAFOptions, m_afopts, szIniFilePath));
	m_column_limit = static_cast<UINT16>(GetPrivateProfileInt(OptSectName, OptAFColumnLimit, m_column_limit, szIniFilePath));
	m_indent_width = GetPrivateProfileInt(OptSectName, OptAFIndentWidth, m_indent_width, szIniFilePath);
	m_eol_type = static_cast<EOLTYPES>(GetPrivateProfileInt(OptSectName, OptAFEolType, m_eol_type, szIniFilePath));
	m_tab_width = GetPrivateProfileInt(OptSectName, OptTabWidth, m_tab_width, szIniFilePath);
	m_continuation_indent_width = GetPrivateProfileInt(OptSectName, OptContIndentWidth, m_continuation_indent_width, szIniFilePath);
	m_spaces_before_call = GetPrivateProfileInt(OptSectName, OptSpacesBeforeCall, m_spaces_before_call, szIniFilePath);
	m_column_table_limit = GetPrivateProfileInt(OptSectName, OptColumnTableLimit, m_column_table_limit, szIniFilePath);
	m_column_table_limit_kv = GetPrivateProfileInt(OptSectName, OptColumnTableLimitKV, m_column_table_limit_kv, szIniFilePath);
	m_line_breaks = GetPrivateProfileInt(OptSectName, OptLineBreaks, m_line_breaks, szIniFilePath);
	tab_sep = GetPrivateProfileInt(OptSectName, OptTabSep, 0, szIniFilePath) == 0;
	m_verbose = GetPrivateProfileInt(OptSectName, OptERRVERB, m_verbose, szIniFilePath);
}

void CPluginOptions::SaveOptions()
{
	wchar_t szNum[10]{};
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

	// autoformat options
	wsprintf(szNum, L"0x%08X", m_afopts);
	WritePrivateProfileString(OptSectName, OptAFOptions, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_column_limit);
	WritePrivateProfileString(OptSectName, OptAFColumnLimit, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_indent_width);
	WritePrivateProfileString(OptSectName, OptAFIndentWidth, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_eol_type);
	WritePrivateProfileString(OptSectName, OptAFEolType, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_tab_width);
	WritePrivateProfileString(OptSectName, OptTabWidth, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_continuation_indent_width);
	WritePrivateProfileString(OptSectName, OptContIndentWidth, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_spaces_before_call);
	WritePrivateProfileString(OptSectName, OptSpacesBeforeCall, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_column_table_limit);
	WritePrivateProfileString(OptSectName, OptColumnTableLimit, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_column_table_limit_kv);
	WritePrivateProfileString(OptSectName, OptColumnTableLimitKV, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", m_line_breaks);
	WritePrivateProfileString(OptSectName, OptLineBreaks, szNum, szIniFilePath);
	wsprintf(szNum, L"%u", (tab_sep) ? 0 : 1);
	WritePrivateProfileString(OptSectName, OptTabSep, szNum, szIniFilePath);

	const wchar_t* hexfmt = L"0x%06X";
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
	wsprintf(szNum, L"%d", m_verbose);
	WritePrivateProfileString(OptSectName, OptERRVERB, szNum, szIniFilePath);
}

void CPluginOptions::OnSwitchLang()
{
	m_uLang = 1 - m_uLang;
}

const std::vector<std::string> CPluginOptions::get_autoformat_options()
{
	std::vector<std::string> res;

	res.push_back(get_autoformat_option(AFOptions::UseTabs) ? "--use-tab" : "--no-use-tab");
	res.push_back(get_autoformat_option(AFOptions::KeepSimpleBlockOneLine) ? "--keep-simple-control-block-one-line" : "--no-keep-simple-control-block-one-line");
	res.push_back(get_autoformat_option(AFOptions::KeepSimpleFunctionOneLine) ? "--keep-simple-function-one-line" : "--no-keep-simple-function-one-line");
	res.push_back(get_autoformat_option(AFOptions::AlighArgs) ? "--align-args" : "--no-align-args");
	res.push_back(get_autoformat_option(AFOptions::BreakAfterFCLP) ? "--break-after-functioncall-lp" : "--no-break-after-functioncall-lp");
	res.push_back(get_autoformat_option(AFOptions::BreakBeforeFCRP) ? "--break-before-functioncall-rp" : "--no-break-before-functioncall-rp");
	res.push_back(get_autoformat_option(AFOptions::AlignParams) ? "--align-parameter" : "--no-align-parameter");
	res.push_back(get_autoformat_option(AFOptions::ChopDownParams) ? "--chop-down-parameter" : "--no-chop-down-parameter");
	res.push_back(get_autoformat_option(AFOptions::BreakAfterFDLP) ? "--break-after-functiondef-lp" : "--no-break-after-functiondef-lp");
	res.push_back(get_autoformat_option(AFOptions::BreakBeforeFDRP) ? "--break-before-functiondef-rp" : "--no-break-before-functiondef-rp");
	res.push_back(get_autoformat_option(AFOptions::AlignTableFields) ? "--align-table-field" : "--no-align-table-field");
	res.push_back(get_autoformat_option(AFOptions::BreakAfterTLB) ? "--break-after-table-lb" : "--no-break-after-table-lb");
	res.push_back(get_autoformat_option(AFOptions::BreakBeforeTRB) ? "--break-before-table-rb" : "--no-break-before-table-rb");
	res.push_back(get_autoformat_option(AFOptions::ChopDownTable) ? "--chop-down-table" : "--no-chop-down-table");
	res.push_back(get_autoformat_option(AFOptions::ChopDownKVTable) ? "--chop-down-kv-table" : "--no-chop-down-kv-table");
	res.push_back(get_autoformat_option(AFOptions::ExtraSep) ? "--extra-sep-at-table-end" : "--no-extra-sep-at-table-end");
	res.push_back(get_autoformat_option(AFOptions::BreakAfterOperator) ? "--break-after-operator" : "--no-break-after-operator");
	//res.push_back(get_autoformat_option(AFOptions::DQtoSQ) ? "--double-quote-to-single-quote" : "--no-double-quote-to-single-quote");
	if (get_autoformat_option(AFOptions::TransformLiterals_Enabled))
		res.push_back(get_autoformat_option(AFOptions::TransformLiterals) ? "--double-quote-to-single-quote" : "--single-quote-to-double-quote");
	res.push_back(get_autoformat_option(AFOptions::SpaceInFuncDefParens) ? "--spaces-inside-functiondef-parens" : "--no-spaces-inside-functiondef-parens");
	res.push_back(get_autoformat_option(AFOptions::SpaceInFuncCallParens) ? "--spaces-inside-functioncall-parens" : "--no-spaces-inside-functioncall-parens");
	res.push_back(get_autoformat_option(AFOptions::SpaceInsideTableBraces) ? "--spaces-inside-table-braces" : "--no-spaces-inside-table-braces");
	res.push_back(get_autoformat_option(AFOptions::SpaceAroundEquals) ? "--spaces-around-equals-in-field" : "--no-spaces-around-equals-in-field");
	res.push_back("--line-separator=" + get_line_sep());
	res.push_back("--column-limit=" + std::to_string(get_column_limit()));
	res.push_back("--indent-width=" + std::to_string(get_indent_width()));
	res.push_back("--tab-width=" + std::to_string(get_tab_width()));
	res.push_back("--continuation-indent-width=" + std::to_string(get_continuation_indent_width()));
	res.push_back("--spaces-before-call=" + std::to_string(get_spaces_before_call()));
	res.push_back("--column-table-limit=" + std::to_string(get_column_table_limit()));
	//res.push_back("--column-table-limit-kv=" + std::to_string(get_column_table_limit_kv())); // underline in original code
	res.push_back("--column_table_limit_kv=" + std::to_string(get_column_table_limit_kv()));
	res.push_back("--line-breaks-after-function-body=" + std::to_string(get_line_breaks()));
	res.push_back("--table-sep=" + get_table_sep()); // ',' or ';'

	if (get_autoformat_option(AFOptions::DumpConfig))
	{
		std::stringstream ss;
		for (const std::string& s : res) ss << s << "\n";
		print_from_lua(ss.str().c_str());
	}
	return res;
}

void CPluginOptions::reset_af_options()
{
	/*
	//	use_tab : false
	 set_autoformat_option(AFOptions::UseTabs, false);
	//	keep_simple_control_block_one_line : true
	 set_autoformat_option(AFOptions::KeepSimpleBlockOneLine, true);
	//	keep_simple_function_one_line : true
	 set_autoformat_option(AFOptions::KeepSimpleFunctionOneLine, true);
	//	align_args : true
	 set_autoformat_option(AFOptions::AlighArgs, true);
	//	break_after_functioncall_lp : false
	 set_autoformat_option(AFOptions::BreakAfterFCLP, false);
	//	break_before_functioncall_rp : false
	 set_autoformat_option(AFOptions::BreakBeforeFCRP, false);
	//	align_parameter : true
	 set_autoformat_option(AFOptions::AlignParams, true);
	//	chop_down_parameter : false
	 set_autoformat_option(AFOptions::ChopDownParams, false);
	//	break_after_functiondef_lp : false
	 set_autoformat_option(AFOptions::BreakAfterFDLP, false);
	//	break_before_functiondef_rp : false
	 set_autoformat_option(AFOptions::BreakBeforeFDRP, false);
	//	align_table_field : true
	 set_autoformat_option(AFOptions::AlignTableFields, true);
	//	break_after_table_lb : true
	 set_autoformat_option(AFOptions::BreakAfterTLB, true);
	//	break_before_table_rb : true
	 set_autoformat_option(AFOptions::BreakBeforeTRB, true);
	//	chop_down_table : false
	 set_autoformat_option(AFOptions::ChopDownTable, false);
	//	chop_down_kv_table : true
	 set_autoformat_option(AFOptions::ChopDownKVTable, true);
	//	extra_sep_at_table_end : false
	 set_autoformat_option(AFOptions::ExtraSep, false);
	//	break_after_operator : true
	 set_autoformat_option(AFOptions::BreakAfterOperator, true);
	//	double_quote_to_single_quote : false
	// set_autoformat_option(AFOptions::DQtoSQ, false);
	 set_autoformat_option(AFOptions::TransformLiterals_Enabled, false);
	//	single_quote_to_double_quote : false
	 set_autoformat_option(AFOptions::TransformLiterals, false);
	//	spaces_inside_functiondef_parens : false
	 set_autoformat_option(AFOptions::SpaceInFuncDefParens, false);
	//	spaces_inside_functioncall_parens : false
	 set_autoformat_option(AFOptions::SpaceInFuncCallParens, false);
	//	spaces_inside_table_braces : false
	 set_autoformat_option(AFOptions::SpaceInsideTableBraces, false);
	//	spaces_around_equals_in_field : true
	 set_autoformat_option(AFOptions::SpaceAroundEquals, true);
	// dump config to console
	 set_autoformat_option(AFOptions::DumpConfig, false);
	 */

	m_afopts = 0x00415C4E;

	//	line_separator : input	    -- os, input, cr, lf, crlf
	set_line_sep("input");
	//  column_limit: 80
	set_column_limit(80);
	//	indent_width : 4
	set_indent_width(4);
	//	tab_width : 4
	set_tab_width(4);
	//	continuation_indent_width : 4
	set_continuation_indent_width(4);
	//	spaces_before_call : 1
	set_spaces_before_call(1);
	//	column_table_limit : 0
	set_column_table_limit(0);
	//	column_table_limit_kv : 0
	set_column_table_limit_kv(0);
	//	line_breaks_after_function_body : 1
	set_line_breaks(1);
	//	table_sep : ","
	set_table_sep(true);
}

std::string CPluginOptions::get_line_sep()
{
	switch (m_eol_type)
	{
	case EOL_OS:
		return "os";
	case EOL_WIN:
		return "crlf";
	case EOL_Unx:
		return "lf";
	case EOL_Mac:
		return "cr";
	case EOL_INPUT:
	default:
		break;
	}
	return "input";
}

void CPluginOptions::set_line_sep(const std::string& s)
{
	if (s == "input")
		m_eol_type = EOL_INPUT;
	else if (s == "os")
		m_eol_type = EOL_OS;
	else if (s == "lf")
		m_eol_type = EOL_Unx;
	else if (s == "cr")
		m_eol_type = EOL_Mac;
	else if (s == "crlf")
		m_eol_type = EOL_WIN;
	else {
		m_eol_type = EOL_INPUT;
	}
}

UINT CPluginOptions::get_column_limit()
{
	return m_column_limit;
}

void CPluginOptions::set_column_limit(UINT value)
{
	m_column_limit = value;
}

int CPluginOptions::get_indent_width()
{
	return m_indent_width;
}

void CPluginOptions::set_indent_width(int value)
{
	m_indent_width = value;
}

int CPluginOptions::get_tab_width()
{
	return m_tab_width;
}

void CPluginOptions::set_tab_width(int value)
{
	m_tab_width = value;
}

int CPluginOptions::get_continuation_indent_width()
{
	return m_continuation_indent_width;
}

void CPluginOptions::set_continuation_indent_width(int value)
{
	m_continuation_indent_width = value;
}

int CPluginOptions::get_line_breaks()
{
	return m_line_breaks;
}

void CPluginOptions::set_line_breaks(int value)
{
	m_line_breaks = value;
}

int CPluginOptions::get_spaces_before_call()
{
	return m_spaces_before_call;
}

void CPluginOptions::set_spaces_before_call(int value)
{
	m_spaces_before_call = value;
}

int CPluginOptions::get_column_table_limit()
{
	return m_column_table_limit;
}

void CPluginOptions::set_column_table_limit(int value)
{
	m_column_table_limit = value;
}

int CPluginOptions::get_column_table_limit_kv()
{
	return m_column_table_limit_kv;
}

void CPluginOptions::set_column_table_limit_kv(int value)
{
	m_column_table_limit_kv = value;
}

std::string CPluginOptions::get_table_sep()
{
	return tab_sep ? "," : ";";
}

bool CPluginOptions::tabsep_is_comma() const
{
	return tab_sep;
}

void CPluginOptions::set_table_sep(bool is_comma)
{
	tab_sep = is_comma;
}

bool CPluginOptions::get_autoformat_option(AFOptions OptType) const
{
	return (m_afopts >> OptType) & 1;
}

void CPluginOptions::set_autoformat_option(AFOptions OptType, bool state)
{
	m_afopts = (m_afopts & ~((UINT32)1 << OptType)) | ((UINT32)state << OptType);
}

/*
	  --use-tab                         Use tab for indentation
	  --no-use-tab                      Do not use tab for indentation
	  --keep-simple-control-block-one-line
										keep block in one line
	  --no-keep-simple-control-block-one-line
										Do not keep block in one line
	 --keep-simple-function-one-line   keep function in one line
	  --no-keep-simple-function-one-line
										Do not keep function in one line
	  --align-args                      Align the arguments
	  --no-align-args                   Do not align the arguments
	  --break-after-functioncall-lp     Break after '(' of function call
	  --no-break-after-functioncall-lp  Do not break after '(' of function call

	  --break-before-functioncall-rp    Break before ')' of function call
	  --no-break-before-functioncall-rp Do not break before ')' of function call
	  --align-parameter                 Align the parameters
	  --no-align-parameter              Do not align the parameters
	  --chop-down-parameter             Chop down all parameters
	  --no-chop-down-parameter          Do not chop down all parameters
	  --break-after-functiondef-lp      Break after '(' of function def
	  --no-break-after-functiondef-lp   Do not break after '(' of function def
	  --break-before-functiondef-rp     Break before ')' of function def
	  --no-break-before-functiondef-rp  Do not break before ')' of function def
	  --align-table-field               Align fields of table
	  --no-align-table-field            Do not align fields of table
	  --break-after-table-lb            Break after '{' of table
	  --no-break-after-table-lb         Do not break after '{' of table
	  --break-before-table-rb           Break before '}' of table
	  --no-break-before-table-rb        Do not break before '}' of table
	  --chop-down-table                 Chop down any table
	  --no-chop-down-table              Do not chop down any table
	  --chop-down-kv-table              Chop down table if table contains key
	  --no-chop-down-kv-table           Do not chop down table if table contains
										key
	  --extra-sep-at-table-end          Add extra field separator at end of
										table
	  --no-extra-sep-at-table-end       Do not add extra field separator at end
										of table
	  --break-after-operator            Put break after operators
	  --no-break-after-operator         Do not put break after operators
	  --double-quote-to-single-quote    Transform string literals to use single quote
	  --no-double-quote-to-single-quote Do not transform string literals to use
										single quote
	  --single-quote-to-double-quote    Transform string literals to use double quote
	  --no-single-quote-to-double-quote Do not transform string literals to use
										double quote
	  --spaces-inside-functiondef-parens
										Put spaces on the inside of parens in function headers
	  --no-spaces-inside-functiondef-parens
										Do not put spaces on the inside of
										parens in function headers
	  --spaces-inside-functioncall-parens
										Put spaces on the inside of parens in function calls
	  --no-spaces-inside-functioncall-parens
										Do not put spaces on the inside of
										parens in function calls
	  --spaces-inside-table-braces      Put spaces on the inside of braces in table constructors
	  --no-spaces-inside-table-braces   Do not put spaces on the inside of
										braces in table constructors
	  --spaces-around-equals-in-field   Put spaces around the equal sign in key/value fields
	  --no-spaces-around-equals-in-field
										Do not put spaces around the equal sign
										in key/value fields

*/
/*
	  -h, --help                        Display this help menu
	  -v, --verbose                     Turn on verbose mode
	  -i                                Reformats in-place
	  --dump-config                     Dumps the default style used to stdout
	  -c[file], --config=[file]         Style config file
*/
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
OPTIONS:
	  --line-separator=[line separator] input(determined by the input content),
										os(Use line ending of the current
										Operating system), lf(Unix style "\n"),
										crlf(Windows style "\r\n"), cr(classic
										Max style "\r")
	  --column-limit=[column limit]     Column limit of one line
	  --indent-width=[indentation
	  width]                            Number of spaces used for indentation
	  --tab-width=[tab width]           Number of spaces used per tab

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	  --continuation-indent-width=[Continuation
	  indentation width]                Indent width for continuations line
	  --spaces-before-call=[spaces
	  before call]                      Space on function calls
	  --column-table-limit=[column
	  table limit]                      Column limit of each line of a table
	  --table-sep=[table separator]     Character to separate table fields


	  --line-breaks-after-function-body=[num breaks]
										Line breakes after function body
*/