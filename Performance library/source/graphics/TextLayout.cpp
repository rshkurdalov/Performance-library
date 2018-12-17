// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "graphics\TextLayout.h"
#include "gpu\RenderTarget.h"

namespace graphics
{
	TextLayout::TextLayout()
	{
		width = FLT_MAX;
		height = FLT_MAX;
		hAlign = HorizontalAlignLeft;
		vAlign = VerticalAlignTop;
		lineBreak = true;
		spacingMode = TextLineSpacingFontSize;
		spacingArg = 0.0f;
		metricsCalculated = false;
	}
	float32 TextLayout::GetLinespace(TextObject *obj)
	{
		if (spacingMode == TextLineSpacingFontSize)
			return obj->fontSize;
		else if (spacingMode == TextLineSpacingFontSizePlusArg)
			return obj->fontSize + spacingArg;
		else if (spacingMode == TextLineSpacingFontSizeMulArg)
			return obj->fontSize*spacingArg;
		else if (spacingMode == TextLineSpacingAscentPlusLineGap)
			return (obj->font->ascent + obj->font->internalLeading);
		else return spacingArg;
	}
	void TextLayout::CalculateMetrics()
	{
		if (metricsCalculated) return;
		metricsCalculated = true;
		lineMetrics.clear();
		textHeight = 0.0f;
		if (textObjects.size() == 0) return;
		TextLineMetrics line;
		line.baseline = 0.0f;
		line.linespace = 0.0f;
		line.charStart = 0;
		line.width = 0.0f;
		lineMetrics.push_back(line);
		float32 advance = textObjects[0].charMetadata->advance.x;
		std::vector<float32> accumulatedAdvance(textObjects.size());
		accumulatedAdvance[0] = 0.0f;
		uint32 lastWhitespace = UINT32_MAX;
		for (uint32 i = 0; i < textObjects.size(); i++)
		{
			lineMetrics.back().baseline = Max(
				lineMetrics.back().baseline,
				textObjects[i].font->ascent);
			lineMetrics.back().linespace = Max(
				lineMetrics.back().linespace,
				GetLinespace(&textObjects[i]));
			lineMetrics.back().width += advance;
			if (textObjects[i].code == U' ')
				lastWhitespace = i;
			if (i == textObjects.size() - 1)
			{
				if (hAlign == HorizontalAlignLeft)
					lineMetrics.back().offset = 0.0f;
				else if (hAlign == HorizontalAlignCenter)
					lineMetrics.back().offset = 0.5f*(width - lineMetrics.back().width);
				else lineMetrics.back().offset = width - lineMetrics.back().width;
				lineMetrics.back().charEnd = textObjects.size();
				textHeight += lineMetrics.back().linespace;
				if (lineBreak && textObjects.back().code == U'\n')
				{
					line.baseline = lineMetrics.back().baseline;
					line.linespace = lineMetrics.back().linespace;
					line.charStart = textObjects.size();
					line.charEnd = line.charStart;
					if (hAlign == HorizontalAlignLeft)
						line.offset = 0.0f;
					else if (hAlign == HorizontalAlignCenter)
						line.offset = 0.5f*width;
					else line.offset = width;
					line.width = 0.0f;
					lineMetrics.push_back(line);
					textHeight += lineMetrics.back().linespace;
				}
				return;
			}
			accumulatedAdvance[i + 1] = accumulatedAdvance[i] + advance;
			advance = textObjects[i + 1].charMetadata->advance.x;
			if (lineBreak
				&& (textObjects[i].code == U'\n'
					|| lineMetrics.back().width + advance > width + 1e-2f
					&& textObjects[i + 1].code != U' '
					&& textObjects[i + 1].code != U'\n'))
			{
				if (hAlign == HorizontalAlignLeft)
					lineMetrics.back().offset = 0.0f;
				else if (hAlign == HorizontalAlignCenter)
					lineMetrics.back().offset = 0.5f*(width - lineMetrics.back().width);
				else lineMetrics.back().offset = width - lineMetrics.back().width;
				if (textObjects[i].code == U'\n')
					lastWhitespace = UINT32_MAX;
				if (lastWhitespace == UINT32_MAX)
					lineMetrics.back().charEnd = i + 1;
				else lineMetrics.back().charEnd = lastWhitespace + 1;
				lineMetrics.back().width = accumulatedAdvance[lineMetrics.back().charEnd]
					- accumulatedAdvance[lineMetrics.back().charStart];
				line.charStart = lineMetrics.back().charEnd;
				line.width = accumulatedAdvance[i + 1] - accumulatedAdvance[line.charStart];
				textHeight += lineMetrics.back().linespace;
				lineMetrics.push_back(line);
				lastWhitespace = UINT32_MAX;
			}
		}
	}
	void TextLayout::Reset()
	{
		metricsCalculated = false;
	}
	void TextLayout::SetWidth(float32 value)
	{
		Reset();
		width = value;
	}
	float32 TextLayout::GetWidth()
	{
		return width;
	}
	void TextLayout::SetHeight(float32 value)
	{
		Reset();
		height = value;
	}
	float32 TextLayout::GetHeight()
	{
		return height;
	}
	void TextLayout::SetHorizontalAlign(HorizontalAlign mode)
	{
		Reset();
		hAlign = mode;
	}
	HorizontalAlign TextLayout::GetHorizontalAlign()
	{
		return hAlign;
	}
	void TextLayout::SetVerticalAlign(VerticalAlign mode)
	{
		Reset();
		vAlign = mode;
	}
	VerticalAlign TextLayout::GetVerticalAlign()
	{
		return vAlign;
	}
	void TextLayout::EnableMultiline(bool value)
	{
		Reset();
		lineBreak = value;
	}
	bool TextLayout::IsMultiline()
	{
		return lineBreak;
	}
	void TextLayout::SetLinespacing(TextLineSpacing mode, float32 arg)
	{
		Reset();
		spacingMode = mode;
		spacingArg = arg;
	}
	void TextLayout::GetLinespacing(TextLineSpacing *mode, float32 *arg)
	{
		*mode = spacingMode;
		*arg = spacingArg;
	}
	uint32 TextLayout::GetLineCount()
	{
		CalculateMetrics();
		return lineMetrics.size();
	}
	void TextLayout::GetLineMetrics(uint32 idx, TextLineMetrics *lm)
	{
		CalculateMetrics();
		*lm = lineMetrics[idx];
	}
	float32 TextLayout::GetTextHeight()
	{
		CalculateMetrics();
		return textHeight;
	}
	void TextLayout::InsertText(
		uint32 idx,
		char32 *text,
		uint32 charCount,
		wchar *fontName,
		float32 fontSize,
		bool isItalic,
		uint32 weight,
		bool underlined,
		bool strikedthrough,
		Color color)
	{
		Reset();
		FontMetadata *font;
		if (FontManager::GetFontMetadata(
			std::wstring(fontName),
			fontSize*FontManager::GetDPIMultiplier(),
			isItalic,
			weight,
			&font) != HResultSuccess) return;
		weight = FontManager::AdjustFontWeight(weight);
		TextObject obj;
		obj.font = font;
		obj.fontPointSize = fontSize;
		fontSize *= font->internalLeadingMultiplier*FontManager::GetDPIMultiplier();
		obj.fontSize = fontSize;
		obj.isItalic = isItalic;
		obj.weight = weight;
		obj.underlined = underlined;
		obj.strikedthrough = strikedthrough;
		obj.color = color;
		for (uint32 i = 0; i < charCount; i++)
		{
			obj.code = text[i];
			if (FontManager::GetCharMetadata(obj.code, font, &obj.charMetadata) != HResultSuccess
				&& FontManager::GetCharMetadata(U'?', font, &obj.charMetadata) != HResultSuccess) continue;
			textObjects.insert(textObjects.begin() + idx, obj);
			idx++;
		}
	}
	void TextLayout::DeleteText(
		uint32 idxBegin,
		uint32 idxEnd)
	{
		Reset();
		textObjects.erase(
			textObjects.begin() + idxBegin,
			textObjects.begin() + idxEnd);
	}
	void TextLayout::GetText(
		uint32 idxBegin,
		uint32 idxEnd,
		std::u32string *text)
	{
		text->reserve(idxEnd - idxBegin);
		while (idxBegin != idxEnd)
			text->push_back(textObjects[idxBegin++].code);
	}
	uint32 TextLayout::GetTextLength()
	{
		return textObjects.size();
	}
	void TextLayout::SetFont(
		uint32 idxBegin,
		uint32 idxEnd,
		wchar *fontName)
	{
		Reset();
		while (idxBegin < idxEnd)
		{
			if (FontManager::GetFontMetadata(
				std::wstring(fontName),
				textObjects[idxBegin].fontPointSize*FontManager::GetDPIMultiplier(),
				textObjects[idxBegin].isItalic,
				textObjects[idxBegin].weight,
				&textObjects[idxBegin].font) == HResultSuccess)
			{
				FontManager::GetCharMetadata(
					textObjects[idxBegin].code,
					textObjects[idxBegin].font,
					&textObjects[idxBegin].charMetadata);
				textObjects[idxBegin].fontSize =
					textObjects[idxBegin].fontPointSize
					*textObjects[idxBegin].font->internalLeadingMultiplier
					*FontManager::GetDPIMultiplier();
			}
			idxBegin++;
		}
	}
	void TextLayout::SetFontSize(
		uint32 idxBegin,
		uint32 idxEnd,
		float32 value)
	{
		Reset();
		float32 logicalFontSize = value * FontManager::GetDPIMultiplier();
		while (idxBegin < idxEnd)
		{
			FontManager::GetFontMetadata(
				textObjects[idxBegin].font->fontName,
				value,
				textObjects[idxBegin].isItalic,
				textObjects[idxBegin].weight,
				&textObjects[idxBegin].font);
			textObjects[idxBegin].fontPointSize = value;
			textObjects[idxBegin].fontSize = textObjects[idxBegin].font->internalLeadingMultiplier*logicalFontSize;
			idxBegin++;
		}
	}
	float32 TextLayout::GetFontSize(uint32 idx)
	{
		return textObjects[idx].fontPointSize;
	}
	float32 TextLayout::GetLogicalFontSize(uint32 idx)
	{
		return textObjects[idx].fontSize;
	}
	void TextLayout::SetItalic(
		uint32 idxBegin,
		uint32 idxEnd,
		bool value)
	{
		Reset();
		while (idxBegin < idxEnd)
			textObjects[idxBegin++].isItalic = value;
	}
	bool TextLayout::IsItalic(uint32 idx)
	{
		return textObjects[idx].isItalic;
	}
	void TextLayout::SetFontWeight(
		uint32 idxBegin,
		uint32 idxEnd,
		uint32 value)
	{
		Reset();
		value = FontManager::AdjustFontWeight(value);
		while (idxBegin < idxEnd)
			textObjects[idxBegin++].weight = value;
	}
	uint32 TextLayout::GetFontWeight(uint32 idx)
	{
		return textObjects[idx].weight;
	}
	void TextLayout::SetUnderline(
		uint32 idxBegin,
		uint32 idxEnd,
		bool value)
	{
		while (idxBegin < idxEnd)
			textObjects[idxBegin++].underlined = value;
	}
	bool TextLayout::IsUnderlined(uint32 idx)
	{
		return textObjects[idx].underlined;
	}
	void TextLayout::SetStrikethrough(
		uint32 idxBegin,
		uint32 idxEnd,
		bool value)
	{
		while (idxBegin < idxEnd)
			textObjects[idxBegin++].strikedthrough = value;
	}
	bool TextLayout::IsStrikedthrough(uint32 idx)
	{
		return textObjects[idx].strikedthrough;
	}
	void TextLayout::SetColor(
		uint32 idxBegin,
		uint32 idxEnd,
		Color value)
	{
		while (idxBegin < idxEnd)
			textObjects[idxBegin++].color = value;
	}
	Color TextLayout::GetColor(uint32 idx)
	{
		return textObjects[idx].color;
	}
	void TextLayout::HitTest(
		Vector2f point,
		TextPositionMetrics *tpm)
	{
		CalculateMetrics();
		if (textObjects.size() == 0) return;
		float32 cx, cy = 0.0f;
		if (vAlign == VerticalAlignCenter)
			cy += 0.5f*(height - textHeight);
		else if (vAlign == VerticalAlignBottom)
			cy += height - textHeight;
		uint32 line = 0;
		if (point.y < cy) line = 0;
		else
		{
			while (line < lineMetrics.size() && point.y >= cy)
				cy += lineMetrics[line++].linespace;
			line--;
		}
		uint32 iter = lineMetrics[line].charStart;
		cx = lineMetrics[line].offset;
		if (point.x > cx)
		{
			while (iter < lineMetrics[line].charEnd && point.x >= cx)
			{
				cx += textObjects[iter].charMetadata->advance.x;
				iter++;
			}
			if (lineMetrics[line].charStart != lineMetrics[line].charEnd)
			{
				if (point.x < cx - 0.5f*textObjects[iter - 1].charMetadata->advance.x)
					iter--;
				if (iter != 0 && textObjects[iter - 1].code == U'\n')
					iter--;
			}
		}
		tpm->hitTestIdx = iter;
		tpm->line = line;
		tpm->lineMetrics = lineMetrics[line];
	}
	void TextLayout::GetPositionMetrics(
		uint32 idx,
		TextPositionMetrics *tpm)
	{
		CalculateMetrics();
		float32 cx, cy = 0.0f;
		if (vAlign == VerticalAlignCenter)
			cy += 0.5f*(height - textHeight);
		else if (vAlign == VerticalAlignBottom)
			cy += height - textHeight;
		uint32 line = 0;
		while (line < lineMetrics.size()
			&& lineMetrics[line].charStart <= idx)
			cy += lineMetrics[line++].linespace;
		cy -= lineMetrics[--line].linespace;
		cy += lineMetrics[line].baseline;
		cx = lineMetrics[line].offset;
		uint32 iter = lineMetrics[line].charStart;
		while (iter < idx)
		{
			cx += textObjects[iter].charMetadata->advance.x;
			iter++;
		}
		tpm->position = Vector2f(cx, cy);
		tpm->line = line;
		tpm->lineMetrics = lineMetrics[line];
	}
	void TextLayout::Render(
		RenderTarget *rt,
		Vector2f position)
	{
		if (textObjects.size() == 0) return;
		CalculateMetrics();
		float32 cx = position.x, cy = position.y,
			underlineOffset, underlineSize, underlineStart, strikethroughStart;
		bool underlinedRun, strikedthroughRun;
		if (vAlign == VerticalAlignCenter)
			cy += 0.5f*(height - textHeight);
		else if (vAlign == VerticalAlignBottom)
			cy += height - textHeight;
		rt->SetSolidColorBrush(textObjects[0].color);
		for (uint32 i = 0; i < lineMetrics.size(); i++)
		{
			underlinedRun = false;
			strikedthroughRun = false;
			cx = position.x + lineMetrics[i].offset;
			for (uint32 j = lineMetrics[i].charStart; j < lineMetrics[i].charEnd; j++)
			{
				if (j != 0 && textObjects[j].color != textObjects[j - 1].color)
					rt->SetSolidColorBrush(textObjects[j].color);
				rt->RenderGeometry(
					textObjects[j].charMetadata->outline,
					cx,
					cy + lineMetrics[i].baseline);

				if (underlinedRun && !textObjects[j].underlined)
				{
					rt->FillRectangle(
						underlineStart,
						cy + lineMetrics[i].baseline + underlineOffset,
						cx - underlineStart,
						underlineSize);
					underlinedRun = false;
				}
				if (textObjects[j].underlined)
				{
					if (!underlinedRun)
					{
						underlinedRun = true;
						underlineStart = cx;
						underlineOffset = textObjects[j].font->underlineOffset;
						underlineSize = textObjects[j].font->underlineSize;
					}
					else
					{
						underlineOffset = Max(underlineOffset, textObjects[j].font->underlineOffset);
						underlineSize = Max(underlineSize, textObjects[j].font->underlineSize);
					}
				}

				if (strikedthroughRun
					&& (!textObjects[j].strikedthrough
						|| textObjects[j - 1].font != textObjects[j].font
						|| textObjects[j - 1].fontSize != textObjects[j].fontSize))
				{
					rt->FillRectangle(
						strikethroughStart,
						cy + lineMetrics[i].baseline + textObjects[j - 1].font->strikethroughOffset,
						cx - strikethroughStart,
						textObjects[j - 1].font->strikethroughSize);
					strikedthroughRun = false;
				}
				if (textObjects[j].strikedthrough && !strikedthroughRun)
				{
					strikedthroughRun = true;
					strikethroughStart = cx;
				}

				cx += textObjects[j].charMetadata->advance.x;
			}
			if (underlinedRun)
				rt->FillRectangle(
					underlineStart,
					cy + lineMetrics[i].baseline + underlineOffset,
					cx - underlineStart,
					underlineSize);
			if (strikedthroughRun)
				rt->FillRectangle(
					strikethroughStart,
					cy + lineMetrics[i].baseline + textObjects[lineMetrics[i].charEnd - 1].font->strikethroughOffset,
					cx - strikethroughStart,
					textObjects[lineMetrics[i].charEnd - 1].font->strikethroughSize);
			cy += lineMetrics[i].linespace;
		}
	}
}
