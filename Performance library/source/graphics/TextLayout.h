// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "graphics\Font.h"
#include <vector>

namespace graphics
{
	struct CharVisualData
	{
		Font *font;
		float32 fontSize;
	};

	class TextLayout
	{
	protected:
		struct TextObject
		{
			CharMetadata metadata;
			CharVisualData attributes;
		};
		std::vector<TextObject> textObjects;
		bool metricsCalculated;
		void CalculateMetrics();
	public:
		TextLayout();
		~TextLayout();
		void SetWidth(float32 width);
		void SetHeight(float32 height);
		void SetText(
			char16 *text,
			msize charCount,
			Font *font,
			float32 fontSize);
		void Render(
			Vector2f position,
			gpu::RenderTarget *rt);
	};
}
