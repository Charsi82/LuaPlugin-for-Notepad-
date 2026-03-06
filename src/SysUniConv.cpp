///////// encoding ///////////
#include "PluginSettings.hpp"

std::wstring SysUniConv::MultibyteToWideChar(const char* ansiString, UINT codePage)
{
	if (!strlen(ansiString))
	{
		return {};
	}

	// 1. Determine the required buffer size
	int wideCharLen = MultiByteToWideChar(
		codePage,       // Code page for the conversion (e.g., CP_ACP for ANSI)
		0,              // Conversion flags
		ansiString,		// Source string
		-1,             // -1 indicates null-terminated string, calculate length automatically
		NULL,           // Destination buffer (NULL to get required size)
		0               // Destination buffer size (0 to get required size)
	);

	if (wideCharLen == 0) {
		// Handle error, e.g., GetLastError()
		return {};
	}

	// 2. Allocate the destination buffer
	std::wstring wideCharBuffer(wideCharLen, L'\0');

	// 3. Perform the conversion
	int result = MultiByteToWideChar(
		codePage,
		0,
		ansiString,
		-1,
		wideCharBuffer.data(), // Pointer to the beginning of the buffer
		wideCharLen
	);

	if (result == 0) {
		// Handle error
		return {};
	}

	return wideCharBuffer;
}

std::string SysUniConv::WideCharToMultibyte(const wchar_t* wideStr, UINT codePage)
{
	if (wideStr == nullptr || lstrlen(wideStr) == 0) return {};	

	// Determine the required buffer size for the ANSI string
	int ansiSize = WideCharToMultiByte(
		codePage,         // Code page to use for the conversion
		0,                // No special flags
		wideStr,          // Pointer to the wide-character string
		-1,               // Input string is null-terminated
		nullptr,          // Buffer for the converted string (get size)
		0,                // Size of the buffer (get size)
		nullptr,          // Default character for unmappable characters
		nullptr           // Pointer to a boolean indicating if default char was used
	);

	if (ansiSize == 0) {
		// Handle error, e.g., GetLastError()
		return {};
	}

	// Allocate buffer for the ANSI string
	std::string ansiBuffer(ansiSize, '0');

	// Perform the actual conversion
	WideCharToMultiByte(
		codePage,
		0,
		wideStr,
		-1,
		ansiBuffer.data(), // Pointer to the buffer
		ansiSize,          // Size of the buffer
		nullptr,
		nullptr
	);

	return ansiBuffer;
}

std::string SysUniConv::AnsiToUtf8(const char* ansi_string)
{
	const std::wstring ws = SysUniConv::MultibyteToWideChar(ansi_string, CP_ACP);
	return SysUniConv::WideCharToMultibyte(ws.c_str(), CP_UTF8);
}

std::string SysUniConv::Utf8ToAnsi(const char* utf8_string)
{
	const std::wstring ws = SysUniConv::MultibyteToWideChar(utf8_string, CP_UTF8);
	return SysUniConv::WideCharToMultibyte(ws.c_str(), CP_ACP);
}

