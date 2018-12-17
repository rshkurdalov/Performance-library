#include "ui\TextField.h"
#include "ui\UIManager.h"
#include "ui\Window.h"
#include "ui\UIFactory.h"
#include "ui\ScrollBar.h"
#include "kernel\OperatingSystemAPI.h"

namespace ui
{
	TextField::TextField()
	{
		SetWidthDesc(autosize());
		SetHeightDesc(autosize());
		SetPadding(Rect<UISize>(2.0f, 2.0f, 2.0f, 2.0f));
		SetFocusable(true);
		EnableMultiline(true);
		EnableEdit(false);
		SetTextAlign(HorizontalAlignLeft);
		caret = 0;
		selection = 0;
		font = L"cambria";
		fontSize = 12.0f;
		UIFactory *factory;
		UIManager::GetFactory(&factory);
		factory->CreateScrollBar(true, &scroll);
		scroll->SetParent(this);
		factory->Unref();
	}
	TextField::~TextField()
	{
		scroll->Unref();
	}
	void TextField::AdjustScroll()
	{
		if (GetTextLength() == 0 || !editable)
		{
			hOffset = 0.0f;
			scroll->SetOffset(0.0f);
			return;
		}
		TextPositionMetrics tpm;
		textLayout.GetPositionMetrics(caret, &tpm);
		if (IsMultiline())
		{
			if (tpm.position.y - tpm.lineMetrics.baseline < scroll->GetOffset())
				scroll->SetOffset(tpm.position.y - tpm.lineMetrics.baseline);
			else if (tpm.position.y - tpm.lineMetrics.baseline
				+ tpm.lineMetrics.linespace > scroll->GetOffset() + (viewport.bottom - viewport.top))
				scroll->SetOffset(tpm.position.y - tpm.lineMetrics.baseline
					+ tpm.lineMetrics.linespace - (viewport.bottom - viewport.top));
		}
		else
		{
			if (tpm.position.x < hOffset)
				hOffset = tpm.position.x;
			else if (tpm.position.x > hOffset + (viewport.right - viewport.left))
				hOffset = tpm.position.x - (viewport.right - viewport.left);
		}
	}
	void TextField::PrepareText(float32 textWidth, float32 textHeight)
	{
		textLayout.SetWidth(textWidth);
		textLayout.SetHeight(textHeight);
		if (IsMultiline())
		{
			textLayout.SetVerticalAlign(VerticalAlignTop);
			if (textLayout.GetTextHeight() >= viewport.bottom - viewport.top + UIEps)
				scroll->SetVisible(true);
			else scroll->SetVisible(false);
		}
		else
		{
			textLayout.SetVerticalAlign(VerticalAlignCenter);
			scroll->SetVisible(false);
		}
		scroll->SetPosition(Vector2f(effectiveWidth - scroll->GetWidth(), 0.0f));
		scroll->SetViewportSize(viewport.bottom - viewport.top);
		scroll->SetContentSize(textLayout.GetTextHeight());
		scroll->Prepare(scroll->GetWidthDesc().value, effectiveHeight);
		AdjustScroll();
	}
	void TextField::RenderText(RenderTarget *rt, Vector2f p)
	{
		rt->PushScissor(
			p.x + viewport.left,
			p.y + viewport.top,
			viewport.right - viewport.left,
			viewport.bottom - viewport.top);
		Color color;
		if (caret != selection)
		{
			uint32 startSelection = Min(caret, selection),
				endSelection = Max(caret, selection);
			for (uint32 i = startSelection; i < endSelection; i++)
			{
				color = textLayout.GetColor(i);
				color.r = 255 - color.r;
				color.g = 255 - color.g;
				color.b = 255 - color.b;
				textLayout.SetColor(i, i + 1, color);
			}
			TextPositionMetrics tpm1, tpm2;
			textLayout.GetPositionMetrics(startSelection, &tpm1);
			textLayout.GetPositionMetrics(endSelection, &tpm2);
			rt->SetSolidColorBrush(Color(Color::DodgerBlue));
			if (tpm1.line == tpm2.line)
				rt->FillRectangle(
					p.x + viewport.left + tpm1.position.x - hOffset,
					p.y + viewport.top + tpm1.position.y - tpm1.lineMetrics.baseline - scroll->GetOffset(),
					tpm2.position.x - tpm1.position.x,
					tpm1.lineMetrics.linespace);
			else
			{
				float32 offset = p.y + viewport.top
					+ tpm1.position.y - tpm1.lineMetrics.baseline
					+ tpm1.lineMetrics.linespace - scroll->GetOffset();
				rt->FillRectangle(
					p.x + viewport.left + tpm1.position.x,
					offset - tpm1.lineMetrics.linespace,
					tpm1.lineMetrics.offset + tpm1.lineMetrics.width - tpm1.position.x + 2.0f,
					tpm1.lineMetrics.linespace);
				for (uint32 i = tpm1.line + 1; i < tpm2.line; i++)
				{
					TextLineMetrics lineMetrics;
					textLayout.GetLineMetrics(i, &lineMetrics);
					rt->FillRectangle(
						p.x + viewport.left + lineMetrics.offset,
						offset,
						lineMetrics.width + 2.0f,
						lineMetrics.linespace);
					offset += lineMetrics.linespace;
				}
				rt->FillRectangle(
					p.x + viewport.left + tpm2.lineMetrics.offset,
					offset,
					tpm2.position.x - tpm2.lineMetrics.offset,
					tpm2.lineMetrics.linespace);
			}
		}
		textLayout.Render(rt, Vector2f(p.x + viewport.left - hOffset, p.y + viewport.top - scroll->GetOffset()));
		if (editable && UIManager::IsFocused(this) && UIManager::IsCaretVisible() && caret == selection)
		{
			bool extraChar = false;
			if (textLayout.GetTextLength() == 0)
			{
				textLayout.InsertText(
					0,
					U"\n",
					1,
					(wchar *)font.c_str(),
					fontSize,
					false,
					400,
					false,
					false,
					foreground);
				extraChar = true;
			}
			if (textLayout.GetTextLength() != 0)
			{
				TextPositionMetrics tpm;
				textLayout.GetPositionMetrics(caret, &tpm);
				tpm.position += p + Vector2f(viewport.left, viewport.top);
				uint32 idx = caret;
				if (idx != 0) idx--;
				color = textLayout.GetColor(idx);
				rt->SetSolidColorBrush(color);
				rt->FillRectangle(
					tpm.position.x - hOffset,
					tpm.position.y - 1.2f*textLayout.GetFontSize(idx) - scroll->GetOffset(),
					1.0f,
					1.4f*textLayout.GetFontSize(idx));
				if (extraChar) textLayout.DeleteText(0, 1);
			}
		}
		if (caret != selection)
		{
			uint32 startSelection = Min(caret, selection),
				endSelection = Max(caret, selection);
			for (uint32 i = startSelection; i < endSelection; i++)
			{
				color = textLayout.GetColor(i);
				color.r = 255 - color.r;
				color.g = 255 - color.g;
				color.b = 255 - color.b;
				textLayout.SetColor(i, i + 1, color);
			}
		}
		rt->PopScissor();
	}
	Vector2f TextField::EvaluateContentSizeImpl(
		float32 *viewportWidth,
		float32 *viewportHeight)
	{
		float32 currentWidth = textLayout.GetWidth(),
			currentHeight = textLayout.GetHeight();
		textLayout.SetWidth(viewportWidth == nullptr ? FLT_MAX : *viewportWidth);
		textLayout.SetHeight(viewportHeight == nullptr ? FLT_MAX : *viewportHeight);
		Vector2f size = Vector2f(0.0f, textLayout.GetTextHeight());
		for (uint32 i = 0; i < textLayout.GetLineCount(); i++)
		{
			TextLineMetrics lm;
			textLayout.GetLineMetrics(i, &lm);
			size.x = Max(lm.width, size.x);
		}
		textLayout.SetWidth(currentWidth);
		textLayout.SetHeight(currentHeight);
		return size;
	}
	void TextField::PrepareImpl()
	{
		PrepareText(viewport.right - viewport.left, viewport.bottom - viewport.top);
	}
	void TextField::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		RenderText(rt, p);
		scroll->Render(rt, p + scroll->GetEffectivePosition());
	}
	void TextField::EnableEdit(bool value)
	{
		editable = value;
		Update();
	}
	bool TextField::IsEditable()
	{
		return editable;
	}
	void TextField::EnableMultiline(bool value)
	{
		textLayout.EnableMultiline(value);
		Update();
	}
	bool TextField::IsMultiline()
	{
		return textLayout.IsMultiline();
	}
	void TextField::SetTextAlign(HorizontalAlign mode)
	{
		textLayout.SetHorizontalAlign(mode);
	}
	HorizontalAlign TextField::GetTextAlign()
	{
		return textLayout.GetHorizontalAlign();
	}
	void TextField::SetDefaultFont(std::wstring &fontName)
	{
		font = fontName;
	}
	std::wstring TextField::GetDefaultFont()
	{
		return font;
	}
	void TextField::SetDefaultFontSize(float32 value)
	{
		fontSize = value;
	}
	float32 TextField::GetDefaultFontSize()
	{
		return fontSize;
	}
	void TextField::SetText(
		char32 *text,
		uint32 charCount,
		bool isItalic,
		uint32 weight,
		bool underlined,
		bool strikedthrough)
	{
		Clear();
		InsertText(
			0,
			text,
			charCount,
			isItalic,
			weight,
			underlined,
			strikedthrough);
	}
	void TextField::InsertText(
		uint32 idx,
		char32 *text,
		uint32 charCount,
		bool isItalic,
		uint32 weight,
		bool underlined,
		bool strikedthrough)
	{
		textLayout.InsertText(
			idx,
			text,
			charCount,
			font.data(),
			fontSize,
			isItalic,
			400,
			underlined,
			strikedthrough,
			foreground);
		caret = Min(idx + charCount, textLayout.GetTextLength());
		selection = caret;
		Update();
	}
	void TextField::Clear()
	{
		DeleteText(0, textLayout.GetTextLength());
	}
	void TextField::DeleteText(uint32 idxBegin, uint32 idxEnd)
	{
		textLayout.DeleteText(idxBegin, idxEnd);
		caret = Min(caret, textLayout.GetTextLength());
		selection = caret;
		Update();
	}
	void TextField::GetText(std::u32string *text)
	{
		textLayout.GetText(0, GetTextLength(), text);
	}
	uint32 TextField::GetTextLength()
	{
		return textLayout.GetTextLength();
	}
	void TextField::SelectText(uint32 caretIdx, uint32 selectIdx)
	{
		caret = caretIdx;
		selection = selectIdx;
		Repaint();
	}
	void TextField::DeselectText()
	{
		selection = caret;
		Repaint();
	}
	void TextField::SetFont(uint32 idxBegin, uint32 idxEnd, wchar *fontName)
	{
		textLayout.SetFont(idxBegin, idxEnd, fontName);
		Update();
	}
	void TextField::SetFontSize(uint32 idxBegin, uint32 idxEnd, float32 value)
	{
		textLayout.SetFontSize(idxBegin, idxEnd, value);
		Update();
	}
	float32 TextField::GetFontSize(uint32 idx)
	{
		return textLayout.GetFontSize(idx);
	}
	float32 TextField::GetLogicalFontSize(uint32 idx)
	{
		return textLayout.GetLogicalFontSize(idx);
	}
	void TextField::SetItalic(uint32 idxBegin, uint32 idxEnd, bool value)
	{
		textLayout.SetItalic(idxBegin, idxEnd, value);
		Update();
	}
	bool TextField::IsItalic(uint32 idx)
	{
		return textLayout.IsItalic(idx);
	}
	void TextField::SetFontWeight(uint32 idxBegin, uint32 idxEnd, uint32 value)
	{
		textLayout.SetFontWeight(idxBegin, idxEnd, value);
		Update();
	}
	uint32 TextField::GetFontWeight(uint32 idx)
	{
		return textLayout.GetFontWeight(idx);
	}
	void TextField::SetUnderline(uint32 idxBegin, uint32 idxEnd, bool value)
	{
		textLayout.SetUnderline(idxBegin, idxEnd, value);
		Repaint();
	}
	bool TextField::IsUnderlined(uint32 idx)
	{
		return textLayout.IsUnderlined(idx);
	}
	void TextField::SetStrikethrough(uint32 idxBegin, uint32 idxEnd, bool value)
	{
		textLayout.SetStrikethrough(idxBegin, idxEnd, value);
		Repaint();
	}
	bool TextField::IsStrikedthrough(uint32 idx)
	{
		return textLayout.IsStrikedthrough(idx);
	}
	void TextField::SetColor(uint32 idxBegin, uint32 idxEnd, Color value)
	{
		textLayout.SetColor(idxBegin, idxEnd, value);
		Repaint();
	}
	Color TextField::GetColor(uint32 idx)
	{
		return textLayout.GetColor(idx);
	}
	void TextField::ForEach(Function<void(UIObject*, void*)> callback, void *param)
	{
		callback(scroll, param);
	}
	void TextField::MouseClick(UIMouseEvent *e)
	{
		if (e->button == MouseButtonLeft)
		{
			if (e->doubleClick)
				SelectText(GetTextLength(), 0);
			else if (GetTextLength() != 0)
			{
				TextPositionMetrics tpm;
				textLayout.HitTest(
					Vector2f(
						e->x - viewport.left + hOffset,
						e->y - viewport.top + scroll->GetOffset()),
					&tpm);
				caret = tpm.hitTestIdx;
				selection = caret;
				UIManager::SetPull(this);
			}
			AdjustScroll();
			Repaint();
		}
	}
	void TextField::StartHover()
	{
		UIManager::SwitchCursor(CursorBeam, window);
	}
	void TextField::MouseMove(UIMouseEvent *e)
	{
		if (UIManager::IsPulled(this))
		{
			Vector2f point = window->GetMousePosition() - GetAbsolutePosition();
			TextPositionMetrics tpm;
			textLayout.HitTest(
				Vector2f(
					point.x - viewport.left + hOffset,
					point.y - viewport.top + scroll->GetOffset()),
				&tpm);
			caret = tpm.hitTestIdx;
			AdjustScroll();
		}
	}
	void TextField::EndHover()
	{
		UIManager::SwitchCursor(CursorDefault, window);
	}
	void TextField::FocusReceive()
	{
		UIManager::LaunchCaretTimer(this);
		Repaint();
	}
	void TextField::FocusLoss()
	{
		hOffset = 0.0f;
		DeselectText();
		UIManager::StopCaretTimer();
		Repaint();
	}
	void TextField::MouseWheelRotate(UIMouseWheelEvent *e)
	{
		scroll->SetOffset(scroll->GetOffset() - copysign(fontSize, e->delta));
		Repaint();
	}
	void TextField::KeyPress(UIKeyboardEvent *e)
	{
		if (e->ctrlModifier && !e->altModifier && !e->shiftModifier)
		{
			if (e->code == U'C' && caret != selection)
			{
				std::u32string text;
				textLayout.GetText(Min(caret, selection), Max(caret, selection), &text);
				OSCopyTextToClipboard(text);
			}
			else if (e->code == U'V')
			{
				std::u32string text;
				OSGetTextFromClipboard(&text);
				DeleteText(Min(caret, selection), Max(caret, selection));
				InsertText(caret, (char32 *)text.c_str(), text.length());
			}
			else if (e->code == U'A')
			{
				SelectText(GetTextLength(), 0);
				AdjustScroll();
				Repaint();
			}
			else if (e->code == U'X' && editable)
			{
				std::u32string text;
				textLayout.GetText(Min(caret, selection), Max(caret, selection), &text);
				OSCopyTextToClipboard(text);
				DeleteText(Min(caret, selection), Max(caret, selection));
			}
		}
		if (!e->altModifier && editable)
		{
			if (e->code == KeyCodeLeft && caret != 0)
			{
				caret--;
				selection = caret;
			}
			else if (e->code == KeyCodeRight && caret != GetTextLength())
			{
				caret++;
				selection = caret;
			}
			else if (e->code == KeyCodeDown && GetTextLength() != 0)
			{
				TextPositionMetrics tpm;
				textLayout.GetPositionMetrics(caret, &tpm);
				tpm.position.y += (tpm.lineMetrics.linespace - tpm.lineMetrics.baseline) + 0.1f;
				textLayout.HitTest(tpm.position, &tpm);
				caret = tpm.hitTestIdx;
				selection = caret;
			}
			else if (e->code == KeyCodeUp && GetTextLength() != 0)
			{
				TextPositionMetrics tpm;
				textLayout.GetPositionMetrics(caret, &tpm);
				tpm.position.y -= tpm.lineMetrics.baseline + 0.1f;
				textLayout.HitTest(tpm.position, &tpm);
				caret = tpm.hitTestIdx;
				selection = caret;
			}
			else return;
			AdjustScroll();
			Repaint();
		}
	}
	void TextField::CharInput(UIKeyboardEvent *e)
	{
		if (!editable || e->ctrlModifier || e->altModifier) return;
		if (selection != caret
			&& (e->character == KeyCodeBackspace
				|| e->character == KeyCodeTab
				|| e->character == KeyCodeEnter && textLayout.IsMultiline()
				|| e->character >= 32))
		{
			if (selection < caret) Swap(selection, caret);
			DeleteText(caret, selection);
		}
		else if (e->character == KeyCodeBackspace
			&& caret != 0
			&& GetTextLength() != 0)
		{
			caret--;
			DeleteText(caret, caret + 1);
		}
		if (e->character == KeyCodeEnter)
		{
			if (textLayout.IsMultiline())
				InsertText(caret, U"\n", 1);
			else if (UIManager::IsFocused(this))
				UIManager::SetFocus(nullptr);
		}
		else if (e->character >= 32 || e->character == KeyCodeTab)
			InsertText(caret, &e->character, 1);
	}
}
