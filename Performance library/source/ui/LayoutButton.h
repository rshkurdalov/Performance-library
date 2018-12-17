#pragma once
#include "ui\FlowLayout.h"

namespace ui
{
	class LayoutButton : public FlowLayout
	{
	protected:
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		LayoutButton();
		void MouseClick(UIMouseEvent *e);
		void MouseRelease(UIMouseEvent *e);
		virtual void ButtonClick(UIMouseEvent *e) {}
		Observer<UIMouseEvent *> onButtonClick;
	};
}
