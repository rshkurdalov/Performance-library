#include "ui\CheckBox.h"
#include "ui\UIManager.h"

namespace ui
{
	CheckBox::CheckBox()
	{
		isChecked = false;
	}
	Vector2f CheckBox::EvaluateContentSizeImpl(
		float32 *viewportWidth,
		float32 *viewportHeight)
	{
		if (GetTextLength() != 0)
		{
			TextLineMetrics lm;
			textLayout.GetLineMetrics(0, &lm);
			boxSize = 0.66f*lm.linespace;
		}
		else boxSize = GetDefaultFontSize();
		if (viewportWidth != nullptr) *viewportWidth -= 1.5f*boxSize;
		Vector2f size = TextField::EvaluateContentSizeImpl(viewportWidth, viewportHeight);
		if (GetTextLength() != 0) size.x += 1.5f*boxSize;
		else size = boxSize;
		return size;
	}
	void CheckBox::PrepareImpl()
	{
		if (GetTextLength() != 0)
		{
			TextLineMetrics lm;
			textLayout.GetLineMetrics(0, &lm);
			boxSize = 0.66f*lm.linespace;
		}
		else boxSize = GetDefaultFontSize();
		PrepareText(
			viewport.right - viewport.left - boxSize,
			viewport.bottom - viewport.top);
	}
	void CheckBox::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		Vector2f boxPosition(
			p.x + viewport.left + 0.25f*boxSize,
			p.y + viewport.top + 0.5f*(viewport.bottom - viewport.top - boxSize));
		if (UIManager::IsPulled(this) || UIManager::IsHovered(this))
		{
			Color fillColor = foreground;
			if (UIManager::IsPulled(this))
			{
				fillColor.r += (255 - foreground.r) >> 1;
				fillColor.g += (255 - foreground.g) >> 1;
				fillColor.b += (255 - foreground.b) >> 1;
			}
			else
			{
				fillColor.r += ((255 - foreground.r) << 1) / 3;
				fillColor.g += ((255 - foreground.g) << 1) / 3;
				fillColor.b += ((255 - foreground.b) << 1) / 3;
			}
			rt->SetSolidColorBrush(fillColor);
			rt->FillRectangle(boxPosition.x, boxPosition.y, boxSize, boxSize);
		}
		rt->SetSolidColorBrush(foreground);
		rt->DrawRectangle(boxPosition.x, boxPosition.y, boxSize, boxSize, 1.0f);
		if (isChecked)
		{
			rt->SetSolidColorBrush(foreground);
			rt->DrawLine(
				boxPosition + Vector2f(0.15f, 0.6f)*boxSize,
				boxPosition + Vector2f(0.45f, 0.8f)*boxSize,
				0.06f*boxSize);
			rt->DrawLine(
				boxPosition + Vector2f(0.45f, 0.8f)*boxSize,
				boxPosition + Vector2f(0.85f, 0.2f)*boxSize,
				0.06f*boxSize);
		}
		RenderText(rt, Vector2f(p.x + 1.5f*boxSize, p.y));
	}
	void CheckBox::SetChecked(bool value)
	{
		isChecked = value;
		Repaint();
	}
	bool CheckBox::IsChecked()
	{
		return isChecked;
	}
	void CheckBox::ButtonClick(UIMouseEvent *e)
	{
		isChecked = !isChecked;
		Repaint();
	}
}
