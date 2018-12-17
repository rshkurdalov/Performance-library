// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "graphics\Geometry.h"
#include <string>

namespace graphics
{
	HResult FontInitialize();

	struct FontMetadata
	{
		uint64 fontHandler;
		std::wstring fontName;
		uint32 size;
		float32 ascent;
		float32 internalLeading;
		float32 internalLeadingMultiplier;
		float32 underlineOffset;
		float32 underlineSize;
		float32 strikethroughOffset;
		float32 strikethroughSize;
	};

	struct CharMetadata
	{
		Geometry outline;
		Vector2f advance;
	};

	class FontManager
	{
		friend class TextLayout;
	protected:
		static float32 GetDPIMultiplier();
		static HResult GetFontMetadata(
			std::wstring &fontName,
			float32 size,
			bool isItalic,
			uint32 weight,
			FontMetadata **fontMetadata);
		static HResult GetCharMetadata(
			char32 code,
			FontMetadata *font,
			CharMetadata **charMetadata);
		static uint32 AdjustFontWeight(uint32 value);
	};
}
