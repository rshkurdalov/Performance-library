#pragma once
#include "ui\UIObject.h"
#include "graphics\TextLayout.h"

namespace ui
{
    class TextField : public UIObject
    {
    protected:
		TextLayout textLayout;
		uint32 caret;
		uint32 selection;
		bool editable;
		std::wstring font;
		float32 fontSize;
		float32 hOffset;
		ScrollBar *scroll;

		void AdjustScroll();
		void PrepareText(float32 textWidth, float32 textHeight);
		void RenderText(RenderTarget *rt, Vector2f p);
		Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
        void PrepareImpl();
        void RenderImpl(RenderTarget *rt, Vector2f p);
    public:
		TextField();
		~TextField();
		void EnableEdit(bool value);
		bool IsEditable();
		void EnableMultiline(bool value);
		bool IsMultiline();
		void SetTextAlign(HorizontalAlign mode);
		HorizontalAlign GetTextAlign();
		void SetDefaultFont(std::wstring &fontName);
		std::wstring GetDefaultFont();
		void SetDefaultFontSize(float32 value);
		float32 GetDefaultFontSize();
		void SetText(
			char32 *text,
			uint32 charCount,
			bool isItalic = false,
			uint32 weight = 400,
			bool underlined = false,
			bool strikedthrough = false);
		// idx must be valid range value
		void InsertText(
			uint32 idx,
			char32 *text,
			uint32 charCount,
			bool isItalic = false,
			uint32 weight = 400,
			bool underlined = false,
			bool strikedthrough = false);
		void Clear();
		// idxBegin, idxEnd must be valid range values
		void DeleteText(uint32 idxBegin, uint32 idxEnd);
		void GetText(std::u32string *text);
		uint32 GetTextLength();
		// caretIdx, selectIdx must be valid range values
		// selectIdx can be less than caretIdx,
		// allowing caret to be put to any place in the text
		void SelectText(uint32 caretIdx, uint32 selectIdx);
		void DeselectText();
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
		void ForEach(Function<void(UIObject *, void *)> callback, void *param);
		void MouseClick(UIMouseEvent *e);
		void StartHover();
		void MouseMove(UIMouseEvent *e);
		void EndHover();
		void FocusReceive();
		void FocusLoss();
		void MouseWheelRotate(UIMouseWheelEvent *e);
		void KeyPress(UIKeyboardEvent *e);
		void CharInput(UIKeyboardEvent *e);
    };
}
