#include "ui\UIFactory.h"
#include "gpu\GpuDevice.h"
#include "ui\UIManager.h"
#include "ui\Window.h"
#include "ui\ScrollBar.h"
#include "ui\FlowLayout.h"
#include "ui\TextField.h"
#include "ui\PushButton.h"
#include "ui\LayoutButton.h"
#include "ui\CheckBox.h"
#include "ui\RadioButton.h"
/*#include "GridLayout.h"
#include "Image.h"
#include "OptionList.h"
#include "Option.h"
#include "ComboBox.h"*/

namespace ui
{
	GradientCollection *gcWhiteGray,
		*gcGrayWhite;

	// Push button

	void ApplyButtonNormalStyle(UIObject *object)
	{
		object->SetBackgroundLinearGradient(
			gcWhiteGray, 0.0em, 0.0em, 0.0em, 1.0em);
	}
	void ApplyButtonHoverStyle(UIObject *object)
	{
		object->SetBackgroundLinearGradient(
			gcWhiteGray, 0.0em, 0.6em, 0.0em, 1.0em);
	}
	void ApplyButtonPullStyle(UIObject *object, float32 x, float32 y)
	{
		x = Min(object->GetEffectiveWidth(), x);
		x = Max(0.0f, x);
		y = Min(object->GetEffectiveHeight(), y);
		y = Max(0.0f, y);
		object->SetBackgroundRadialGradient(
			gcGrayWhite, x, y, 1.3f*object->GetEffectiveWidth(), 1.3f*object->GetEffectiveHeight());
	}
	void OnButtonStartHover(UIObject *object)
	{
		if (!UIManager::IsPulled(object))
			ApplyButtonHoverStyle(object);
	}
	void OnButtonMouseMove(UIMouseEvent *e, UIObject *object)
	{
		if (UIManager::IsPulled(object))
			ApplyButtonPullStyle(object, e->x, e->y);
	}
	void OnButtonEndHover(UIObject *object)
	{
		if (!UIManager::IsPulled(object))
			ApplyButtonNormalStyle(object);
	}
	void OnButtonStartPull(UIObject *object)
	{
		Window *window;
		object->GetWindow(&window);
		if (window == nullptr) return;
		Vector2f point = window->GetMousePosition() - object->GetAbsolutePosition();
		window->Unref();
		ApplyButtonPullStyle(object, point.x, point.y);
	}
	void OnButtonEndPull(UIObject *object)
	{
		if (UIManager::IsHovered(object))
			ApplyButtonHoverStyle(object);
		else ApplyButtonNormalStyle(object);
	}

	// Text field - standart editable

	void ApplyTextFieldNormalStyle(UIObject *object)
	{
		object->SetBackgroundColor(Color::GhostWhite);
	}
	void ApplyTextFieldInteractiveStyle(UIObject *object)
	{
		object->SetBackgroundColor(Color::White);
	}
	void OnTextFieldStartHover(UIObject *object)
	{
		ApplyTextFieldInteractiveStyle(object);
	}
	void OnTextFieldEndHover(UIObject *object)
	{
		if (!UIManager::IsFocused(object))
			ApplyTextFieldNormalStyle(object);
	}
	void OnTextFieldFocusReceive(UIObject *object)
	{
		ApplyTextFieldInteractiveStyle(object);
	}
	void OnTextFieldFocusLoss(UIObject *object)
	{
		if (!UIManager::IsHovered(object))
			ApplyTextFieldNormalStyle(object);
	}

	// Layout button - bordless

	void ApplyLButtonNormalStyle1(UIObject *object)
	{
		object->ClearBackground();
		object->SetBorderThickness(0.0f);
	}
	void ApplyLButtonHoverStyle1(UIObject *object)
	{
		object->SetBackgroundColor(Color::Gainsboro);
	}
	void ApplyLButtonPullStyle1(UIObject *object)
	{
		object->SetBackgroundColor(Color::Silver);
	}
	void OnLButtonStartHover1(UIObject *object)
	{
		if (!UIManager::IsPulled(object))
			ApplyLButtonHoverStyle1(object);
	}
	void OnLButtonMouseMove1(UIMouseEvent *e, UIObject *object)
	{
		if (UIManager::IsPulled(object))
			ApplyLButtonPullStyle1(object);
	}
	void OnLButtonEndHover1(UIObject *object)
	{
		if (!UIManager::IsPulled(object))
			ApplyLButtonNormalStyle1(object);
	}
	void OnLButtonStartPull1(UIObject *object)
	{
		ApplyLButtonPullStyle1(object);
	}
	void OnLButtonEndPull1(UIObject *object)
	{
		if (UIManager::IsHovered(object))
			ApplyLButtonHoverStyle1(object);
		else ApplyLButtonNormalStyle1(object);
	}

	UIFactory::UIFactory()
	{
		if (gcWhiteGray == nullptr)
		{
			GpuDevice *device;
			QueryGpuDevice(&device);
			GradientStop stops[] =
			{
				{0.0f, Color::White},
				{1.0f, Color::Gainsboro},
			};
			device->CreateGradientCollection(stops, ARRAYSIZE(stops), &gcWhiteGray);
			Swap(stops[0].color, stops[1].color);
			device->CreateGradientCollection(stops, ARRAYSIZE(stops), &gcGrayWhite);
			device->Unref();
		}
	}
	void UIFactory::CreateScrollBar(bool isVertical, ScrollBar **ppScrollBar)
	{
		*ppScrollBar = new ScrollBar(isVertical);
		(*ppScrollBar)->SetForeground(Color::Gainsboro);
	}
	void UIFactory::CreateFlowLayout(FlowLayout **ppFlowLayout)
	{
		*ppFlowLayout = new FlowLayout();
	}
	void UIFactory::CreateTextField(TextField **ppTextField)
	{
		*ppTextField = new TextField();
	}
	void UIFactory::CreateEditField(TextField **ppTextField)
	{
		*ppTextField = new TextField();
		(*ppTextField)->EnableEdit(true);
		(*ppTextField)->SetBorderThickness(1.0f);
		(*ppTextField)->SetBorderRadius(Vector2f(4.0f, 4.0f));
		(*ppTextField)->onStartHover.AddCallback(OnTextFieldStartHover, *ppTextField);
		(*ppTextField)->onEndHover.AddCallback(OnTextFieldEndHover, *ppTextField);
		(*ppTextField)->onFocusReceive.AddCallback(OnTextFieldFocusReceive, *ppTextField);
		(*ppTextField)->onFocusLoss.AddCallback(OnTextFieldFocusLoss, *ppTextField);
		ApplyTextFieldNormalStyle(*ppTextField);
	}
	void UIFactory::CreatePushButton(PushButton **ppPushButton)
	{
		*ppPushButton = new PushButton();
		(*ppPushButton)->SetBorderThickness(1.0f);
		(*ppPushButton)->SetBorderRadius(Vector2f(4.0f, 4.0f));
		(*ppPushButton)->SetBorderColor(Color::DarkGray);
		(*ppPushButton)->onStartHover.AddCallback(OnButtonStartHover, *ppPushButton);
		(*ppPushButton)->onEndHover.AddCallback(OnButtonEndHover, *ppPushButton);
		(*ppPushButton)->onStartPull.AddCallback(OnButtonStartPull, *ppPushButton);
		(*ppPushButton)->onEndPull.AddCallback(OnButtonEndPull, *ppPushButton);
		(*ppPushButton)->onMouseMove.AddCallback(OnButtonMouseMove, *ppPushButton);
		ApplyButtonNormalStyle(*ppPushButton);

	}
	void UIFactory::CreateLayoutButton(LayoutButton **ppLayoutButton)
	{
		*ppLayoutButton = new LayoutButton();
		(*ppLayoutButton)->onStartHover.AddCallback(OnLButtonStartHover1, *ppLayoutButton);
		(*ppLayoutButton)->onEndHover.AddCallback(OnLButtonEndHover1, *ppLayoutButton);
		(*ppLayoutButton)->onStartPull.AddCallback(OnLButtonStartPull1, *ppLayoutButton);
		(*ppLayoutButton)->onEndPull.AddCallback(OnLButtonEndPull1, *ppLayoutButton);
		(*ppLayoutButton)->onMouseMove.AddCallback(OnLButtonMouseMove1, *ppLayoutButton);
		ApplyLButtonNormalStyle1(*ppLayoutButton);
	}
	void UIFactory::CreateCheckBox(CheckBox **ppCheckBox)
	{
		*ppCheckBox = new CheckBox();
	}
	void UIFactory::CreateRadioButton(
		RadioButtonGroup *group,
		RadioButton **ppRadioButton)
	{
		*ppRadioButton = new RadioButton(group);
	}
    /*void UIFactory::CreateGridLayout(GridLayout **ppGridLayout)
    {
        *ppGridLayout = new GridLayout(window);
    }
    void UIFactory::CreateFlowLayout(FlowLayout **ppFlowLayout)
    {
        *ppFlowLayout = new FlowLayout(window);
    }
    void UIFactory::CreateImage(Image **ppImage)
    {
        *ppImage = new Image(window);
    }
    void UIFactory::CreateTextArea(bool isStatic, bool isMultiline, TextArea **ppTextArea)
    {
        *ppTextArea = new TextArea(isStatic, isMultiline, window);
    }
    void UIFactory::CreatePushButton(PushButton **ppPushButton)
    {
        *ppPushButton = new PushButton(window);
    }
    void UIFactory::CreateLayoutButton(LayoutButton **ppLayoutButton)
    {
        *ppLayoutButton = new LayoutButton(window);
    }
    void UIFactory::CreateCheckBox(CheckBox **ppCheckBox)
    {
        *ppCheckBox = new CheckBox(window);
    }
    void UIFactory::CreateRadioButton(
        RadioButtonGroup *group,
        RadioButton **ppRadioButton)
    {
        *ppRadioButton = new RadioButton(group, window);
    }
    void UIFactory::CreateOptionList(OptionList **ppOptionList)
    {
        *ppOptionList = new OptionList(window);
    }
    void UIFactory::CreateOption(Option **ppOption)
    {
        *ppOption = new Option(window);
    }
    void UIFactory::CreateComboBox(ComboBox **ppComboBox)
    {
        *ppComboBox = new ComboBox(window);
    }
	void UIFactory::CreateWndDialog(
		char16 *name,
		int16 posX,
		int16 posY,
		uint16 effectiveWidth,
		uint16 effectiveHeight,
		Dialog **ppDialog)
	{
		*ppDialog = new Dialog(name, posX, posY, effectiveWidth, effectiveHeight, window);
	}*/
}
