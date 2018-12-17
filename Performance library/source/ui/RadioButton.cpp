#include "ui\RadioButton.h"
#include "ui\UIManager.h"

namespace ui
{
	RadioButton::RadioButton(RadioButtonGroup *group)
	{
		group->AddRef();
		this->group = group;
	}
	RadioButton::~RadioButton()
	{
		if (group->checkedButton == this)
			group->checkedButton = nullptr;
		group->Unref();
	}
	Vector2f RadioButton::EvaluateContentSizeImpl(
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
	void RadioButton::PrepareImpl()
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
	void RadioButton::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		Vector2f boxCenter(
			p.x + viewport.left + 0.25f*boxSize + 0.5f*boxSize,
			p.y + viewport.top + 0.5f*(viewport.bottom - viewport.top - boxSize) + 0.5f*boxSize);
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
			rt->FillEllipse(Vector2f(boxCenter.x, boxCenter.y), 0.5f*boxSize, 0.5f*boxSize);
		}
		rt->SetSolidColorBrush(foreground);
		rt->DrawEllipse(Vector2f(boxCenter.x, boxCenter.y), 0.5f*boxSize, 0.5f*boxSize, 1.0f);
		if (IsChecked())
		{
			rt->SetSolidColorBrush(foreground);
			rt->FillEllipse(Vector2f(boxCenter.x, boxCenter.y), 0.26f*boxSize, 0.26f*boxSize);
		}
		RenderText(rt, Vector2f(p.x + 1.5f*boxSize, p.y));
	}
	void RadioButton::SetChecked()
	{
		if (group->checkedButton != nullptr)
			group->checkedButton->Repaint();
		group->checkedButton = this;
		Repaint();
	}
	bool RadioButton::IsChecked()
	{
		return group->checkedButton == this;
	}
	void RadioButton::ButtonClick(UIMouseEvent *e)
	{
		SetChecked();
	}
}
