#include "ui\PushButton.h"
#include "ui\UIManager.h"

namespace ui
{
	PushButton::PushButton()
	{
		SetTextAlign(HorizontalAlignCenter);
	}
	void PushButton::MouseClick(UIMouseEvent *e)
	{
		if (e->button == MouseButtonLeft)
		{
			UIManager::SetPull(this);
			Repaint();
		}
	}
	void PushButton::MouseRelease(UIMouseEvent *e)
	{
		if (UIManager::IsPulled(this))
		{
			ButtonClick(e);
			onButtonClick.Notify(e);
			Repaint();
		}
	}
	void PushButton::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		RenderText(rt, p);
	}
}
