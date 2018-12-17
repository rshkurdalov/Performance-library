#pragma once
#include "ui\TextField.h"

namespace ui
{
	class PushButton : public TextField
	{
	protected:
		using TextField::EnableEdit;
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		PushButton();
		void MouseClick(UIMouseEvent *e);
		void MouseRelease(UIMouseEvent *e);
		void StartHover() {}
		void MouseMove(UIMouseEvent *e) {}
		void EndHover() {}
		void FocusReceive() {}
		void FocusLoss() {}
		void MouseWheelRotate(UIMouseWheelEvent *e) {}
		void KeyPress(UIKeyboardEvent *e) {}
		void CharInput(UIKeyboardEvent *e) {}
		virtual void ButtonClick(UIMouseEvent *e) {}
		Observer<UIMouseEvent *> onButtonClick;
	};
}
