#include "lua_main.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <map>

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

std::wstring GetIniPath();

CPluginOptions::CPluginOptions() :
	m_bConsoleOpen(false), m_bConsoleClearOnRun(false),
	m_bConsoleAutoclear(false), m_bShowRunTime(false), m_bConEncoding(true),
	m_uLang(0), m_uInterpType(LUA51), m_uFlags(0x1B), m_timequote(3),
	clrOKdm(0), clrOK(0), clrERRdm(0), clrERR(0),
	m_afopts(0), m_column_limit(80), m_eol_type(EOL_INPUT), m_indent_width(4),
	m_tab_width(4), m_continuation_indent_width(4), m_spaces_before_call(1),
	m_column_table_limit(0), m_column_table_limit_kv(0), m_line_breaks(1), tab_sep(true), m_verbose(0),
	LovePath(L"C:\\Program Files\\LOVE\\love.exe")
{
}

void CPluginOptions::reset_colors()
{
	clrOK = 0x008000;
	clrOKdm = 0x00FA0A;
	clrERR = 0x00080;
	clrERRdm = 0x00FFFF;
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

std::map<std::wstring, std::map<std::wstring, std::wstring>> parseIniFile(const wchar_t* filename)
{
	std::map<std::wstring, std::map<std::wstring, std::wstring>> iniData;
	std::wifstream file(filename);
	if (!file.is_open())
	{
		//std::cerr << "Error: Could not open file " << filename << std::endl;
		return iniData;
	}

	std::wstring line;
	std::wstring currentSection;
	while (std::getline(file, line))
	{
		//line.erase(0, line.find_first_not_of(L" \t\r\n")); // Trim leading whitespace
		//line.erase(line.find_last_not_of(L" \t\r\n") + 1); // Trim trailing whitespace

		if (line.empty() || line[0] == ';' || line[0] == '#') { // Skip empty lines and comments
			continue;
		}

		if (line[0] == L'[' && line.back() == L']')
		{ // Section header
			currentSection = line.substr(1, line.length() - 2);
		}
		else
		{ // Key-value pair
			size_t eqPos = line.find('=');
			if (eqPos != std::wstring::npos)
			{
				std::wstring key = line.substr(0, eqPos);
				std::wstring value = line.substr(eqPos + 1);
				//key.erase(0, key.find_first_not_of(L" \t")); // Trim key
				//key.erase(key.find_last_not_of(L" \t") + 1);
				//value.erase(0, value.find_first_not_of(L" \t")); // Trim value
				//value.erase(value.find_last_not_of(L" \t") + 1);
				iniData[currentSection][key] = value;
			}
		}
	}
	file.close();
	return iniData;
}

void CPluginOptions::ReadOptions(BYTE defLang)
{
	reset_af_options();
	reset_colors();
	m_uLang = defLang;
	std::wstring ini_path = GetIniPath();
	auto mapData = parseIniFile(ini_path.c_str());
	if (!mapData.contains(OptSectName)) return;

	const auto& Section = mapData[OptSectName];
	auto ReadData = [&Section](const wchar_t* sKey, auto& val,
		std::function<ULONG(const std::wstring&)> f =
		[](const std::wstring& data) { return std::stoul(data); })
		{
			if (Section.contains(sKey)) val = static_cast<std::remove_reference_t<decltype(val)>>(f(Section.at(sKey)));
		};

	ReadData(OptFlagsKey, m_uFlags);
	{
		m_bConsoleOpen = !!(m_uFlags & OPTF_CONOPENONINIT);
		m_bConsoleClearOnRun = !!(m_uFlags & OPTF_CONCLEARONRUN);
		m_bConsoleAutoclear = !!(m_uFlags & OPTF_CONAUTOCLEAR);
		m_bShowRunTime = !!(m_uFlags & OPTF_PRINTRUNTIME);
		m_bConEncoding = !!(m_uFlags & OPTF_CONENCODING);
	}

	ReadData(OptLangKey, m_uLang);
	ReadData(OptLuaKey, m_uInterpType);
	{
		if (Section.contains(OptLovePath)) LovePath = Section.at(OptLovePath);
	}
	ReadData(OptAFColumnLimit, m_column_limit);
	ReadData(OptAFIndentWidth, m_indent_width);
	{
		int val = m_eol_type;
		ReadData(OptAFEolType, val);
		m_eol_type = static_cast<EOLTYPES>(val);
	}
	ReadData(OptTabWidth, m_tab_width);
	ReadData(OptContIndentWidth, m_continuation_indent_width);
	ReadData(OptSpacesBeforeCall, m_spaces_before_call);
	ReadData(OptColumnTableLimit, m_column_table_limit);
	ReadData(OptColumnTableLimitKV, m_column_table_limit_kv);
	ReadData(OptLineBreaks, m_line_breaks);
	{
		int val = tab_sep;
		ReadData(OptTabSep, val);
		tab_sep = val == 0;
	}
	ReadData(OptTimeQuoteKey, m_timequote);
	ReadData(OptERRVERB, m_verbose);
	auto ReadHex = [](const std::wstring& data) { return std::stoul(data, nullptr, 16); };
	ReadData(OptClrOKKey, clrOK, ReadHex);
	ReadData(OptClrOKDarkKey, clrOKdm, ReadHex);
	ReadData(OptClrERRKey, clrERR, ReadHex);
	ReadData(OptClrERRDarkKey, clrERRdm, ReadHex);
	ReadData(OptAFOptions, m_afopts, ReadHex);
}

void CPluginOptions::SaveOptions()
{
	const std::wstring ini_path = GetIniPath();
	std::wofstream outf(ini_path);
	if (!outf.is_open()) return;
	outf << std::format(L"[{}]\n", OptSectName)
		<< std::format(L"{}={}\n", OptFlagsKey, getOptFlags())
		<< std::format(L"{}={}\n", OptLangKey, m_uLang)
		<< std::format(L"{}={}\n", OptLuaKey, m_uInterpType)
		<< std::format(L"{}={}\n", OptLovePath, LovePath)
		<< std::format(L"{}=0x{:08X}\n", OptAFOptions, m_afopts)
		<< std::format(L"{}={}\n", OptAFColumnLimit, m_column_limit)
		<< std::format(L"{}={}\n", OptAFIndentWidth, m_indent_width)
		<< std::format(L"{}={}\n", OptAFEolType, static_cast<byte>(m_eol_type))
		<< std::format(L"{}={}\n", OptTabWidth, m_tab_width)
		<< std::format(L"{}={}\n", OptContIndentWidth, m_continuation_indent_width)
		<< std::format(L"{}={}\n", OptSpacesBeforeCall, m_spaces_before_call)
		<< std::format(L"{}={}\n", OptColumnTableLimit, m_column_table_limit)
		<< std::format(L"{}={}\n", OptColumnTableLimitKV, m_column_table_limit_kv)
		<< std::format(L"{}={}\n", OptLineBreaks, m_line_breaks)
		<< std::format(L"{}={}\n", OptTabSep, (tab_sep) ? 0 : 1)
		<< std::format(L"{}=0x{:06X}\n", OptClrOKKey, clrOK)
		<< std::format(L"{}=0x{:06X}\n", OptClrOKDarkKey, clrOKdm)
		<< std::format(L"{}=0x{:06X}\n", OptClrERRKey, clrERR)
		<< std::format(L"{}=0x{:06X}\n", OptClrERRDarkKey, clrERRdm)
		<< std::format(L"{}={}\n", OptTimeQuoteKey, m_timequote)
		<< std::format(L"{}={}\n", OptERRVERB, m_verbose);
	outf.close();
}

void CPluginOptions::OnSwitchLang()
{
	m_uLang = 1 - m_uLang;
}

const std::vector<std::string> CPluginOptions::get_autoformat_options()
{
	std::vector<std::string> res;
	res.reserve(32);
	res.emplace_back(get_autoformat_option(AFOptions::UseTabs) ? "--use-tab" : "--no-use-tab");
	res.emplace_back(get_autoformat_option(AFOptions::KeepSimpleBlockOneLine) ? "--keep-simple-control-block-one-line" : "--no-keep-simple-control-block-one-line");
	res.emplace_back(get_autoformat_option(AFOptions::KeepSimpleFunctionOneLine) ? "--keep-simple-function-one-line" : "--no-keep-simple-function-one-line");
	res.emplace_back(get_autoformat_option(AFOptions::AlighArgs) ? "--align-args" : "--no-align-args");
	res.emplace_back(get_autoformat_option(AFOptions::BreakAfterFCLP) ? "--break-after-functioncall-lp" : "--no-break-after-functioncall-lp");
	res.emplace_back(get_autoformat_option(AFOptions::BreakBeforeFCRP) ? "--break-before-functioncall-rp" : "--no-break-before-functioncall-rp");
	res.emplace_back(get_autoformat_option(AFOptions::AlignParams) ? "--align-parameter" : "--no-align-parameter");
	res.emplace_back(get_autoformat_option(AFOptions::ChopDownParams) ? "--chop-down-parameter" : "--no-chop-down-parameter");
	res.emplace_back(get_autoformat_option(AFOptions::BreakAfterFDLP) ? "--break-after-functiondef-lp" : "--no-break-after-functiondef-lp");
	res.emplace_back(get_autoformat_option(AFOptions::BreakBeforeFDRP) ? "--break-before-functiondef-rp" : "--no-break-before-functiondef-rp");
	res.emplace_back(get_autoformat_option(AFOptions::AlignTableFields) ? "--align-table-field" : "--no-align-table-field");
	res.emplace_back(get_autoformat_option(AFOptions::BreakAfterTLB) ? "--break-after-table-lb" : "--no-break-after-table-lb");
	res.emplace_back(get_autoformat_option(AFOptions::BreakBeforeTRB) ? "--break-before-table-rb" : "--no-break-before-table-rb");
	res.emplace_back(get_autoformat_option(AFOptions::ChopDownTable) ? "--chop-down-table" : "--no-chop-down-table");
	res.emplace_back(get_autoformat_option(AFOptions::ChopDownKVTable) ? "--chop-down-kv-table" : "--no-chop-down-kv-table");
	res.emplace_back(get_autoformat_option(AFOptions::ExtraSep) ? "--extra-sep-at-table-end" : "--no-extra-sep-at-table-end");
	res.emplace_back(get_autoformat_option(AFOptions::BreakAfterOperator) ? "--break-after-operator" : "--no-break-after-operator");
	//res.emplace_back(get_autoformat_option(AFOptions::DQtoSQ) ? "--double-quote-to-single-quote" : "--no-double-quote-to-single-quote");
	if (get_autoformat_option(AFOptions::TransformLiterals_Enabled))
		res.emplace_back(get_autoformat_option(AFOptions::TransformLiterals) ? "--double-quote-to-single-quote" : "--single-quote-to-double-quote");
	res.emplace_back(get_autoformat_option(AFOptions::SpaceInFuncDefParens) ? "--spaces-inside-functiondef-parens" : "--no-spaces-inside-functiondef-parens");
	res.emplace_back(get_autoformat_option(AFOptions::SpaceInFuncCallParens) ? "--spaces-inside-functioncall-parens" : "--no-spaces-inside-functioncall-parens");
	res.emplace_back(get_autoformat_option(AFOptions::SpaceInsideTableBraces) ? "--spaces-inside-table-braces" : "--no-spaces-inside-table-braces");
	res.emplace_back(get_autoformat_option(AFOptions::SpaceAroundEquals) ? "--spaces-around-equals-in-field" : "--no-spaces-around-equals-in-field");
	res.emplace_back(std::format("--line-separator={}", get_line_sep()));
	res.emplace_back(std::format("--column-limit={}", get_column_limit()));
	res.emplace_back(std::format("--indent-width={}", get_indent_width()));
	res.emplace_back(std::format("--tab-width={}", get_tab_width()));
	res.emplace_back(std::format("--continuation-indent-width={}", get_continuation_indent_width()));
	res.emplace_back(std::format("--spaces-before-call={}", get_spaces_before_call()));
	res.emplace_back(std::format("--column-table-limit={}", get_column_table_limit()));
	//res.emplace_back(std::format("--column-table-limit-kv={}", get_column_table_limit_kv())); // underline in original code
	res.emplace_back(std::format("--column_table_limit_kv={}", get_column_table_limit_kv()));
	res.emplace_back(std::format("--line-breaks-after-function-body={}", get_line_breaks()));
	res.emplace_back(std::format("--table-sep={}", get_table_sep())); // ',' or ';'
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

const char* CPluginOptions::get_line_sep()
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

UINT CPluginOptions::get_column_limit() const
{
	return m_column_limit;
}

void CPluginOptions::set_column_limit(UINT value)
{
	m_column_limit = value;
}

int CPluginOptions::get_indent_width() const
{
	return m_indent_width;
}

void CPluginOptions::set_indent_width(int value)
{
	m_indent_width = value;
}

int CPluginOptions::get_tab_width() const
{
	return m_tab_width;
}

void CPluginOptions::set_tab_width(int value)
{
	m_tab_width = value;
}

int CPluginOptions::get_continuation_indent_width() const
{
	return m_continuation_indent_width;
}

void CPluginOptions::set_continuation_indent_width(int value)
{
	m_continuation_indent_width = value;
}

int CPluginOptions::get_line_breaks() const
{
	return m_line_breaks;
}

void CPluginOptions::set_line_breaks(int value)
{
	m_line_breaks = value;
}

int CPluginOptions::get_spaces_before_call() const
{
	return m_spaces_before_call;
}

void CPluginOptions::set_spaces_before_call(int value)
{
	m_spaces_before_call = value;
}

int CPluginOptions::get_column_table_limit() const
{
	return m_column_table_limit;
}

void CPluginOptions::set_column_table_limit(int value)
{
	m_column_table_limit = value;
}

int CPluginOptions::get_column_table_limit_kv() const
{
	return m_column_table_limit_kv;
}

void CPluginOptions::set_column_table_limit_kv(int value)
{
	m_column_table_limit_kv = value;
}

const char* CPluginOptions::get_table_sep() const
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