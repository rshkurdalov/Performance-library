// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "ui\UITypes.h"

namespace ui
{
	struct UIMouseEvent
	{
		float32 x;
		float32 y;
		float32 deltaX;
		float32 deltaY;
		MouseButton button;
		bool doubleClick;

		UIMouseEvent() {}
		UIMouseEvent(
			float32 x,
			float32 y,
			float32 deltaX = 0.0f,
			float32 deltaY = 0.0f,
			MouseButton button = MouseButton::MouseButtonNone,
			bool doubleClick = false)
			: x(x),
			y(y),
			deltaX(deltaX),
			deltaY(deltaY),
			button(button),
			doubleClick(doubleClick) {}
	};

	struct UIMouseWheelEvent
	{
		float32 x;
		float32 y;
		float32 delta;

		UIMouseWheelEvent() {}
		UIMouseWheelEvent(
			float32 x,
			float32 y,
			float32 delta)
			: x(x),
			y(y),
			delta(delta) {}
	};

	struct UIKeyboardEvent
	{
		uint32 code;
		bool shiftModifier;
		bool ctrlModifier;
		bool altModifier;
		char32 character;

		UIKeyboardEvent() {}
		UIKeyboardEvent(
			uint32 code,
			bool shiftModifier,
			bool ctrlModifier,
			bool altModifier,
			char32 character = U'\0')
			: code(code),
			shiftModifier(shiftModifier),
			ctrlModifier(ctrlModifier),
			altModifier(altModifier),
			character(character) {}
	};

	struct UIResizeEvent
	{
		int32 oldWidth;
		int32 oldHeight;
		int32 newWidth;
		int32 newHeight;

		UIResizeEvent() {}
		UIResizeEvent(
			int32 oldWidth,
			int32 oldHeight,
			int32 newWidth,
			int32 newHeight)
			: oldWidth(oldWidth),
			oldHeight(oldHeight),
			newWidth(newWidth),
			newHeight(newHeight) {}
	};

	struct UICloseEvent
	{
		bool confirmClose;

		UICloseEvent() : confirmClose(true) {}
	};
}
