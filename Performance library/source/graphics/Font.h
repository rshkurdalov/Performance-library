// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "math\VectorMath.h"
#include <string>
#include <map>

namespace graphics
{
	HResult FtInitialize();

	HResult LoadFont(
		std::wstring path,
		Font **ppFont);

	struct CharMetadata
	{
		char32 code;
		Vector2f advance;
		GeometryPath *outline;
	};

	class Font
	{
	public:
		std::map<wchar, CharMetadata> charData;

		Font(std::map<wchar, CharMetadata> &charData);
		~Font();
	};
}
