#pragma once

#include "PluginSettings.h"

#define INFO_WIN(text) MessageBox(nppData._nppHandle, text,	sPluginName, MB_OK | MB_ICONINFORMATION)
#define WARN_WIN(text) MessageBox(nppData._nppHandle, text, sPluginName, MB_OK | MB_ICONWARNING)
#define ERR_WIN(text) MessageBox(nppData._nppHandle, text, sPluginName, MB_OK | MB_ICONERROR)

#define SendNpp(msg, wParam, lParam) SendMessage(nppData._nppHandle, msg, (WPARAM)wParam, (LPARAM)lParam)

