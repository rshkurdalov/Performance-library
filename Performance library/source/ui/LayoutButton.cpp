#include "ui\LayoutButton.h"
#include "ui\UIManager.h"

namespace ui
{
	LayoutButton::LayoutButton()
	{
		SetPadding(Rect<UISize>(0.0f, 0.0f, 0.0f, 0.0f));
		EnableEventHook(UIHookAll, true);
	}
	void LayoutButton::MouseClick(UIMouseEvent *e)
	{
		if (e->button == MouseButtonLeft)
		{
			UIManager::SetPull(this);
			Repaint();
		}
	}
	void LayoutButton::MouseRelease(UIMouseEvent *e)
	{
		if (UIManager::IsPulled(this))
		{
			ButtonClick(e);
			onButtonClick.Notify(e);
			Repaint();
		}
	}
	void LayoutButton::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		FlowLayout::RenderImpl(rt, p);
	}
}
