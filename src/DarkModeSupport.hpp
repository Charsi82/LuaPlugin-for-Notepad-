#pragma once

enum class DMColorType
{
	background,
	softerBackground, // ctrl background color
	hotBackground,
	pureBackground,  // dlg background color
	errorBackground,
	text,
	darkerText,
	disabledText,
	linkText,
	edge,
	hotEdge,
	disabledEdge,
};

COLORREF GetOKColor() noexcept;
COLORREF GetERRColor() noexcept;
COLORREF GetNormalColor() noexcept;
COLORREF GetBackGroundColor() noexcept;
