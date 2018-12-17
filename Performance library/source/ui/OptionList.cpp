#include "ui\OptionList.h"

namespace ui
{
	OptionList::OptionList(UIObject *contextMenu)
	{
		contextMenu->AddRef();
		this->contextMenu = contextMenu;
	}
	OptionList::~OptionList()
	{
		contextMenu->Unref();
	}
	void OptionList::SetCollapseLevel(uint32 value)
	{
		collapseLevel = value;
	}
	void OptionList::EnableCollapseEventMask(UIHook event, bool value)
	{
		if(value) collapseEventMask |= (uint32)event;
		else collapseEventMask &= ~(uint32)event;
	}
}
