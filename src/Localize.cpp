#include "resource.h"
#include "lua_main.hpp"

const wchar_t* Localize(unsigned long ItemID, bool lng)
{
	switch (ItemID)
	{
		// items plugin
	case CheckSyntax:
	{
		return lng ? L"Verify syntax" : L"Проверка синтаксиса";
	}
	case RunScript:
	{
		return lng ? L"Run script" : L"Запуск скрипта";
	}
	case CheckFiles:
	{
		return lng ? L"Check all files in folder" : L"Проверить все файлы в папке";
	}
	case AutoFormat:
	{
		return lng ? L"Autoformatting" : L"Автоформатирование";
	}
	case RunLove2D:
	{
		return lng ? L"Run LÖVE2D" : L"Запуск LÖVE2D";
	}
	case LUA51:
	{
		return L"Lua51";
	}
	case LUA52:
	{
		return L"Lua52";
	}
	case LUA53:
	{
		return L"Lua53";
	}
	case LUA54:
	{
		return L"Lua54";
	} 
	case LUAPLUTO:
	{
		return L"Pluto";
	}
	case LUAJIT:
	{
		return L"LuaJIT2.1";
	}
	case ShowHideConsole:
	{
		return lng ? L"Show\\hide Console" : L"Показать\\cкрыть консоль";
	}
	case Options:
	{
		return lng ? L"Options" : L"Опции";
	}
	case About:
	{
		return lng ? L"About" : L"О плагине";
	}
#ifdef TEST_ITEM
	case TestItem:
	{
		return lng ? L"Test" : L"Тест";
	}
#endif
	// items for context menu of the console
	case ID_COPY:
	{
		return lng ? L"Copy all" : L"Копировать всё";
	}
	case ID_COPYSELECTED:
	{
		return lng ? L"Copy selected" : L"Копировать выделенное";
	}
	case ID_CLEARCONSOLE:
	{
		return lng ? L"Clear" : L"Очистить";
	}
	case ID_CLOSECONSOLE:
	{
		return lng ? L"Close" : L"Закрыть";
	}

	// other string ids
	case STRINGID_FMT_CHECKALL:
	{
		return lng ? L"%s Done! Found %d files with errors out of %d." : L"%s Выполнено! Найдено %d файлов с ошибками из %d.";
	}

	// items for 'Options' Dialog
	case IDC_CHECKLNG:
	{
		return lng ? L"ENG \\ RU" : L"RU \\ ENG";
	}
	case ID_BTNRESET:
	{
		return lng ? L"Default" : L"По умолчанию";
	}
	case IDC_CHECKAUTOCLEAR:
	{
		return lng ? L"Autoclean Console" : L"Автоочистка консоли";
	}
	case IDC_CHECKRUNTIME:
	{
		return lng ? L"Print elapsed time" : L"Время выполнения";
	}
	case IDC_STATIC_RTQ:
	{
		return lng ? L"Runtime quote (in seconds):" : L"Лимит времени выполнения (в секундах):";
	}
	case IDC_GBOX_AF:
	{
		return lng ? L"Autoformat options" : L"Опции автоформатирования";
	}
	case ID_BTN_AF_APPLY:
	{
		return lng ? L"Apply" : L"Применить";
	}
	case ID_BTN_AF_RESET:
	{
		return lng ? L"Reset" : L"По умолчанию";
	}
	case IDC_TABSEP:
	{
		return lng ? L"Table element separator:" : L"Разделитель эл-тов таблицы:";
	}
	case IDC_AFUSETABS:
	{
		return lng ? L"Use tabs" : L"Использовать табуляцию";
	}
	case IDC_KeepSimpleBlockOneLine:
	{
		return lng ? L"Keep block in one line" : L"Простой блок в одной строке";
	}
	case IDC_KeepSimpleFunctionOneLine:
	{
		return lng ? L"Keep function in one line" : L"Простая ф-ция в одной строке";
	}
	case IDC_AlignArgs:
	{
		return lng ? L"Align the arguments" : L"Выравнивать аргументы";
	}
	case IDC_AlignParams:
	{
		return lng ? L"Align the parameters" : L"Выравнивать параметры";
	}
	case IDC_ChopDownParams:
	{
		return lng ? L"Chop down all parameters" : L"Разбивка параметров";
	}
	case IDC_BreakAfterFCLP:
	{
		return lng ? L"Break after '(' of function call" : L"Перенос после '(' в вызове ф-ции";
	}
	case IDC_BreakBeforeFCRP:
	{
		return lng ? L"Break before ')' of function call" : L"Перенос перед ')' в вызове ф-ции";
	}
	case IDC_AlignTableFields:
	{
		return lng ? L"Align fields of table" : L"Выровнять поля таблицы";
	}
	case IDC_BreakAfterTLB:
	{
		return lng ? L"Break after '{' of table" : L"Перенос после '{' таблицы";
	}
	case IDC_BreakBeforeTRB:
	{
		return lng ? L"Break before '}' of table" : L"Перенос перед '}' таблицы";
	}
	case IDC_ChopDownTable:
	{
		return lng ? L"Chop down any table" : L"Разбивка любой таблицы";
	}
	case IDC_ChopDownKVTable:
	{
		return lng ? L"Chop down kv table" : L"Разбивка kv таблицы";
	}
	case IDC_BreakAfterFDLP:
	{
		return lng ? L"Break after '(' of function def" : L"Перенос после '(' в опр. ф-ции";
	}
	case IDC_BreakBeforeFDRP:
	{
		return lng ? L"Break before ')' of function def" : L"Перенос перед ')' в опр. ф-ции";
	}
	case IDC_BreakAfterOperator:
	{
		return lng ? L"Put break after operators" : L"Перенос после операторов";
	}
	case IDC_ExtraSep:
	{
		return lng ? L"Add extra field separator at end of table" : L"Добавить доп. разделитель в конце таблицы";
	}
	case IDC_TransformLiterals:
	{
		return lng ? L"Transform string literals to use" : L"Изменить строковые литералы, ограничивая их";
	}
	case IDC_RADIO_SINGLEQUOTE:
	{
		return lng ? L"single quote" : L"простыми кавычками";
	}
	case IDC_RADIO_DOUBLEQOUTE:
	{
		return lng ? L"double quote" : L"двойными кавычками";
	}
	case IDC_SpaceInFuncDefParens:
	{
		return lng ? L"Put spaces on the inside of parens in function headers" : L"Добавить пробелы в скобках опр. функции";
	}
	case IDC_SpaceInFuncCallParens:
	{
		return lng ? L"Put spaces on the inside of parens in function calls" : L"Добавить пробелы в скобках вызова функции";
	}
	case IDC_SpaceInsideTableBraces:
	{
		return lng ? L"Put spaces on the inside of braces in table constructors" : L"Добавить пробелы в скобках конс-тора таблицы";
	}
	case IDC_SpaceAroundEquals:
	{
		return lng ? L"Put spaces around the equal sign in key/value fields" : L"Добавить пробелы вокруг '=' в конс-торе таблицы";
	}
	case IDC_DUMPCFG:
	{
		return lng ? L"Dump config" : L"Выводить настройки в консоль";
	}
	case IDC_EOLTYPE_STATIC:
	{
		return lng ? L"Line separator ( input, os, lf, cr, crlf )" : L"Перенос строки ( input, os, lf, cr, crlf )";
	}
	case IDC_MAXLINELEN_STATIC:
	{
		return lng ? L"Column limit of one line" : L"Макс. кол-во символов строки";
	}
	case IDC_NUMSPACES:
	{
		return lng ? L"Number of spaces used for indentation" : L"Кол-во символов для отступа";
	}
	case IDC_NUMSPACES_TAB:
	{
		return lng ? L"Number of spaces used per tab" : L"Кол-во символов для табуляции";
	}
	case IDC_NUMLINEBREAKSFB:
	{
		return lng ? L"Line breaks after function body" : L"Кол-во переводов строк после функции";
	}
	case IDC_INDENTWIDTHCONT:
	{
		return lng ? L"Indent width for continuations line" : L"Ширина отступа при переносе строки";
	}
	case IDC_SPACESBEFOREFC:
	{
		return lng ? L"Space before function calls" : L"Пробел перед вызовом функции";
	}
	case IDC_COLMNLIMITTBL:
	{
		return lng ? L"Column limit of each line of a table" : L"Макс. символов в строке таблицы";
	}
	case IDC_COLMNLIMITKVTBL:
	{
		return lng ? L"Column limit of each line of a k = v table." : L"Макс. символов в строке kv таблицы";
	}
	case IDC_GBOX_COMMON:
	{
		return lng ? L"Common" : L"Общие";
	}
	case IDC_CHECKVERBOSE:
	{
		return lng ? L"Full checklist of files" : L"Полный список проверки файлов";
	}
	}
	static std::wstring ws(L"??? Unlocalized StringID (");
	ws += std::to_wstring(ItemID) + L")";
	return ws.c_str();
}
