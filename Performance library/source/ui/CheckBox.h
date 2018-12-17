#pragma once
#include "ui\PushButton.h"

namespace ui
{
	class CheckBox : public PushButton
	{
		friend class UIFactory;
	protected:
		bool isChecked;
		float32 boxSize;

		Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
		void PrepareImpl();
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		CheckBox();
		void SetChecked(bool value);
		bool IsChecked();
		void ButtonClick(UIMouseEvent *e);
	};
}
