#include "ui\ScrollBar.h"
#include "ui\UIManager.h"
#include "ui\Window.h"

namespace ui
{
	ScrollBar::ScrollBar(bool isVertical)
	{
		this->isVertical = isVertical;
		SetWidthDesc(12.0f);
		SetHeightDesc(1.0em);
		if (!isVertical) Swap(widthDesc, heightDesc);
		SetMargin(Rect<UISize>(0.0f, 0.0f, 0.0f, 0.0f));
		SetPadding(Rect<UISize>(2.0f, 2.0f, 2.0f, 2.0f));
		EnableEventHandle(UIHookAll, false);
		EnableEventHandle((UIHook)(UIHookMouseButtonLeft | UIHookMouseButtonRight | UIHookMouseHover), true);
		viewportSize = 0.0f;
		contentSize = 0.0f;
		contentOffset = 0.0f;
	}
	void ScrollBar::PrepareImpl()
	{
		if (viewportSize > contentSize)
		{
			contentOffset = 0.0f;
			return;
		}
		float32 viewportWidth = viewport.right - viewport.left,
			viewportHeight = viewport.bottom - viewport.top;
		sliderSize = (isVertical ? viewportHeight : viewportWidth) * viewportSize / contentSize;
		sliderSize = Min((isVertical ? viewportHeight : viewportWidth), Max(6.0f, sliderSize));
		SetOffset(contentOffset);
	}
	void ScrollBar::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		if (viewportSize > contentSize) return;
		float32 sliderOffset = ((isVertical ? (viewport.bottom - viewport.top) : (viewport.right - viewport.left)) - sliderSize)
			*(contentOffset / (contentSize - viewportSize));
		Color sliderColor = foreground;
		if (UIManager::IsPulled(this))
		{
			sliderColor.r >>= 2;
			sliderColor.g >>= 2;
			sliderColor.b >>= 2;
		}
		else if (UIManager::IsHovered(this))
		{
			Vector2f position = window->GetMousePosition() - p;
			if (isVertical
				&& position.x >= viewport.left
				&& position.x <= viewport.right
				&& position.y >= viewport.top + sliderOffset
				&& position.y <= viewport.top + sliderOffset + sliderSize
				|| !isVertical
				&& position.x >= viewport.left + sliderOffset
				&& position.x <= viewport.right + sliderOffset + sliderSize
				&& position.y >= viewport.top
				&& position.y <= viewport.bottom)
			{
				sliderColor.r >>= 1;
				sliderColor.g >>= 1;
				sliderColor.b >>= 1;
			}
		}
		rt->SetSolidColorBrush(sliderColor);
		if (isVertical)
			rt->FillRectangle(
				p.x + viewport.left,
				p.y + viewport.top + sliderOffset,
				viewport.right - viewport.left,
				sliderSize);
		else rt->FillRectangle(
			p.x + viewport.left + sliderOffset,
			p.y + viewport.top,
			sliderSize,
			viewport.bottom - viewport.top);
	}
	void ScrollBar::SetViewportSize(float32 value)
	{
		viewportSize = value;
		Update();
	}
	void ScrollBar::SetContentSize(float32 value)
	{
		contentSize = value;
		Update();
	}
	void ScrollBar::SetOffset(float32 value)
	{
		contentOffset = value;
		if (value > contentSize - viewportSize)
			contentOffset = contentSize - viewportSize;
		if (contentOffset < 0.0f)
			contentOffset = 0.0f;
		Repaint();
	}
	float32 ScrollBar::GetOffset()
	{
		return contentOffset;
	}
}
