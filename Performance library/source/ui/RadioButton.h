#pragma once
#include "ui\PushButton.h"

namespace ui
{
	class RadioButtonGroup : public SharedObject
	{
		friend class RadioButton;
	protected:
		RadioButton *checkedButton;
	public:
		RadioButtonGroup()
			: checkedButton(nullptr) {}
	};

	class RadioButton : public PushButton
	{
	protected:
		RadioButtonGroup *group;
		float32 boxSize;

		Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
		void PrepareImpl();
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		RadioButton(RadioButtonGroup *group);
		~RadioButton();
		void SetChecked();
		bool IsChecked();
		void ButtonClick(UIMouseEvent *e);
	};
}
