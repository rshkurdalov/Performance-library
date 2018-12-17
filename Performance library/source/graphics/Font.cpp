// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "Font.h"
#include "kernel\OperatingSystemAPI.h"
#include <tuple>
#include <map>

namespace graphics
{
	std::map<std::tuple<std::wstring, uint32, bool, uint32>, FontMetadata *> fontTable;
	std::map<std::tuple<char32, FontMetadata *>, CharMetadata *> charTable;
	float32 dpiMultiplier;

	HResult FontInitialize()
	{
		dpiMultiplier = (float32)OSGetDPI() / 72.0f;
		return HResultSuccess;
	}

	float32 FontManager::GetDPIMultiplier()
	{
		return dpiMultiplier;
	}
	HResult FontManager::GetFontMetadata(
		std::wstring &fontName,
		float32 size,
		bool isItalic,
		uint32 weight,
		FontMetadata **fontMetadata)
	{
		size = round(size);
		weight = Max(400, weight);
		weight = Min(1000, weight);
		decltype(fontTable)::key_type key(fontName, (uint32)size, isItalic, weight);
		decltype(fontTable)::iterator iter = fontTable.find(key);
		if (iter == fontTable.end())
		{
			FontMetadata *font = new FontMetadata();
			if (OSLoadFont((wchar *)fontName.c_str(), (uint32)size, isItalic, weight, font) == HResultSuccess)
			{
				fontTable[key] = font;
				*fontMetadata = font;
				return HResultSuccess;
			}
			else return HResultFail;
		}
		*fontMetadata = iter->second;
		return HResultSuccess;
	}
	HResult FontManager::GetCharMetadata(
		char32 code,
		FontMetadata *font,
		CharMetadata **charMetadata)
	{
		decltype(charTable)::key_type key(code, font);
		decltype(charTable)::iterator iter = charTable.find(key);
		if (iter == charTable.end())
		{
			CharMetadata *fontCharMetadata = new CharMetadata();
			if (OSLoadGlyphMetadata(code, font, fontCharMetadata) == HResultSuccess)
			{
				charTable[key] = fontCharMetadata;
				*charMetadata = fontCharMetadata;
				return HResultSuccess;
			}
			else return HResultFail;
		}
		*charMetadata = iter->second;
		return HResultSuccess;
	}
	uint32 FontManager::AdjustFontWeight(uint32 value)
	{
		if (value <= 400) value = 400;
		else if (value >= 1000) value = 1000;
		else
		{
			uint32 remainder = value % 100;
			if (remainder <= 50) value -= remainder;
			else value += (100 - remainder);
		}
		return value;
	}
}
