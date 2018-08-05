// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "graphics\TextLayout.h"
#include "gpu\RenderTarget.h"

namespace graphics
{
	void TextLayout::CalculateMetrics()
	{

	}
	TextLayout::TextLayout()
	{
		metricsCalculated = false;
	}
	TextLayout::~TextLayout()
	{

	}
	void TextLayout::SetWidth(float32 width)
	{

	}
	void TextLayout::SetHeight(float32 height)
	{

	}
	void TextLayout::SetText(
		char16 *text,
		msize charCount,
		Font *font,
		float32 fontSize)
	{
		metricsCalculated = false;
		textObjects.clear();
		TextObject obj;
		obj.attributes.font = font;
		obj.attributes.fontSize = fontSize;
		for (msize i = 0; i < charCount; i++)
		{
			obj.metadata = font->charData[text[i]];
			textObjects.push_back(obj);
		}
	}
	void TextLayout::Render(
		Vector2f position,
		gpu::RenderTarget *rt)
	{
		float32 cx = position.x,
			cy = position.y + textObjects[0].attributes.fontSize;
		Matrix3x2f transform;
		MatrixScale2d(
			textObjects[0].attributes.fontSize,
			textObjects[0].attributes.fontSize,
			0.0f,
			0.0f,
			&transform);
		transform[2][0] = cx;
		transform[2][1] = cy;
		for (msize i = 0; i < textObjects.size(); i++)
		{
			transform[2][0] = cx;
			/*rt->SetTransform(transform);
			rt->FillGeometry(textObjects[i].metadata.outline);*/
			cx += textObjects[i].metadata.advance.x*textObjects[i].attributes.fontSize;
		}
	}
}
