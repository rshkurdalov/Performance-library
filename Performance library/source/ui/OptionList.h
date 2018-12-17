#pragma once
#include "ui\FlowLayout.h"

namespace ui
{
	class OptionList : public FlowLayout
	{
	protected:
		UIObject *contextMenu;
		uint32 collapseLevel;
		uint32 collapseEventMask;
	public:
		OptionList(UIObject *contextMenu);
		~OptionList();
		void SetCollapseLevel(uint32 value);
		void EnableCollapseEventMask(UIHook event, bool value);
	};
}
