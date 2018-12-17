// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "graphics\Font.h"
#include "ui\UITypes.h"
#include "graphics\Color.h"
#include <vector>

namespace graphics
{
	enum TextLineSpacing
	{
		TextLineSpacingFontSize,
		TextLineSpacingFontSizePlusArg,
		TextLineSpacingFontSizeMulArg,
		TextLineSpacingAscentPlusLineGap,
		TextLineSpacingConstArg,
	};

	struct TextLineMetrics
	{
		float32 baseline;
		float32 linespace;
		uint32 charStart;
		uint32 charEnd;
		float32 offset;
		float32 width;
	};

	struct TextPositionMetrics
	{
		uint32 hitTestIdx;
		uint32 line;
		TextLineMetrics lineMetrics;
		Vector2f position;
	};

	class TextLayout
	{
	protected:
		struct TextObject
		{
			char32 code;
			FontMetadata *font;
			CharMetadata *charMetadata;
			float32 fontPointSize;
			float32 fontSize;
			bool isItalic;
			uint32 weight;
			bool underlined;
			bool strikedthrough;
			Color color;
		};
		std::vector<TextObject> textObjects;
		float32 width;
		float32 height;
		HorizontalAlign hAlign;
		VerticalAlign vAlign;
		bool lineBreak;
		TextLineSpacing spacingMode;
		float32 spacingArg;
		std::vector<TextLineMetrics> lineMetrics;
		float32 textHeight;
		bool metricsCalculated;

		float32 GetLinespace(TextObject *obj);
		void CalculateMetrics();
		void Reset();
	public:
		TextLayout();
		void SetWidth(float32 value);
		float32 GetWidth();
		void SetHeight(float32 value);
		float32 GetHeight();
		void SetHorizontalAlign(HorizontalAlign mode);
		HorizontalAlign GetHorizontalAlign();
		void SetVerticalAlign(VerticalAlign mode);
		VerticalAlign GetVerticalAlign();
		void EnableMultiline(bool value);
		bool IsMultiline();
		void SetLinespacing(TextLineSpacing mode, float32 arg);
		void GetLinespacing(TextLineSpacing *mode, float32 *arg);
		uint32 GetLineCount();
		void GetLineMetrics(uint32 idx, TextLineMetrics *lm);
		float32 GetTextHeight();
		void InsertText(
			uint32 idx,
			char32 *text,
			uint32 charCount,
			wchar *fontName,
			float32 fontSize,
			bool isItalic = false,
			uint32 weight = 400,
			bool underlined = false,
			bool strikedthrough = false,
			Color color = Color(Color::Black));
		void DeleteText(
			uint32 idxBegin,
			uint32 idxEnd);
		void GetText(
			uint32 idxBegin,
			uint32 idxEnd,
			std::u32string *text);
		uint32 GetTextLength();
		void SetFont(
			uint32 idxBegin,
			uint32 idxEnd,
			wchar *fontName);
		void SetFontSize(
			uint32 idxBegin,
			uint32 idxEnd,
			float32 value);
		float32 GetFontSize(uint32 idx);
		float32 GetLogicalFontSize(uint32 idx);
		void SetItalic(
			uint32 idxBegin,
			uint32 idxEnd,
			bool value);
		bool IsItalic(uint32 idx);
		void SetFontWeight(
			uint32 idxBegin,
			uint32 idxEnd,
			uint32 value);
		uint32 GetFontWeight(uint32 idx);
		void SetUnderline(
			uint32 idxBegin,
			uint32 idxEnd,
			bool value);
		bool IsUnderlined(uint32 idx);
		void SetStrikethrough(
			uint32 idxBegin,
			uint32 idxEnd,
			bool value);
		bool IsStrikedthrough(uint32 idx);
		void SetColor(
			uint32 idxBegin,
			uint32 idxEnd,
			Color value);
		Color GetColor(uint32 idx);
		void HitTest(
			Vector2f point,
			TextPositionMetrics *tpm);
		void GetPositionMetrics(
			uint32 idx,
			TextPositionMetrics *tpm);
		void Render(
			RenderTarget *rt,
			Vector2f position);
	};
}
