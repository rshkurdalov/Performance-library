// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include <string>

namespace kernel
{
	// Used by window class function
	HResult OSCreateSurface(
		uint64 hwnd,
		Surface **ppSurface);

	HResult OSCreateWindow(
		wchar *name,
		int32 x,
		int32 y,
		uint32 width,
		uint32 height,
		UIObject *layout,
		Window **window);

	void OSOpenWindow(Window *window);

	void OSEnableWindow(Window *window, bool value);

	void OSRunMsgLoop();

	void OSRunModalMsgLoop(Window *window);

	void OSResizeWindow(
		Window *window,
		uint32 width,
		uint32 height);

	void OSCloseWindow(Window *window);

	// Used by window class function
	void OSReleaseWindowHandler(Window *window);

	void OSCopyTextToClipboard(std::u32string &text);

	void OSGetTextFromClipboard(std::u32string *text);

	// Default callback for UI manager to set cursor
	void OSSetCursor(uint32 cursor, Window *window);

	uint32 OSGetDPI();

	HResult OSLoadFont(
		wchar *fontName,
		uint32 size,
		bool isItalic,
		uint32 weight,
		FontMetadata *font);

	HResult OSLoadGlyphMetadata(
		char32 code,
		FontMetadata *font,
		CharMetadata *charMetadata);
}
