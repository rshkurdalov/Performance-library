#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"

namespace ui
{
    class UIFactory : public SharedObject
    {
    public:
		UIFactory();
		void CreateScrollBar(bool isVertical, ScrollBar **ppScrollBar);
		void CreateFlowLayout(FlowLayout **ppFlowLayout);
		void CreateTextField(TextField **ppTextField);
		void CreateEditField(TextField **ppTextField);
		void CreatePushButton(PushButton **ppPushButton);
		void CreateLayoutButton(LayoutButton **ppLayoutButton);
		void CreateCheckBox(CheckBox **ppCheckBox);
		void CreateRadioButton(
			RadioButtonGroup *group,
			RadioButton **ppRadioButton);
        /*void CreateGridLayout(GridLayout **ppGridLayout);
        void CreateFlowLayout(FlowLayout **ppFlowLayout);
        void CreateImage(Image **ppImage);
        void CreateOptionList(OptionList **ppOptionList);
        void CreateOption(Option **ppOption);
        void CreateComboBox(ComboBox **ppComboBox);
		void CreateWndDialog(
			char16 *name,
			int16 posX,
			int16 posY,
			uint16 effectiveWidth,
			uint16 effectiveHeight,
			Dialog **ppDialog);*/
    };
}
