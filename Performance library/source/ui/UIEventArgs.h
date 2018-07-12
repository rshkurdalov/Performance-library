// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "ui\UITypes.h"

namespace ui
{
	class UIEvent
	{
	public:
		UIObject *sender;
		// Event requires update of the window. False by default.
		bool requireUpdate;

		UIEvent() {};
		UIEvent(UIObject *sender)
			: sender(sender),
			requireUpdate(false) {}
	};

	class UIMouseEvent : public UIEvent
	{
	public:
		float32 x;
		float32 y;
		float32 deltaX;
		float32 deltaY;
		MouseButton button;
		bool doubleClick;

		UIMouseEvent() {}
		UIMouseEvent(
			UIObject *sender,
			float32 x,
			float32 y,
			float32 deltaX = 0.0f,
			float32 deltaY = 0.0f,
			MouseButton button = MouseButton::MouseButtonNone,
			bool doubleClick = false)
			: UIEvent(sender),
			x(x),
			y(y),
			deltaX(deltaX),
			deltaY(deltaY),
			button(button),
			doubleClick(doubleClick) {}
	};

	class UIMouseWheelEvent : public UIEvent
	{
	public:
		float32 x;
		float32 y;
		float32 delta;

		UIMouseWheelEvent() {}
		UIMouseWheelEvent(
			UIObject *sender,
			float32 x,
			float32 y,
			float32 delta)
			: UIEvent(sender),
			x(x),
			y(y),
			delta(delta) {}
	};

	class UIKeyboardEvent : public UIEvent
	{
	public:
		wchar character;

		UIKeyboardEvent() {}
		UIKeyboardEvent(
			UIObject *sender,
			wchar character)
			: UIEvent(sender),
			character(character) {}
	};

	class UIResizeEvent : public UIEvent
	{
	public:
		int32 oldWidth;
		int32 oldHeight;
		int32 newWidth;
		int32 newHeight;

		UIResizeEvent() {}
		UIResizeEvent(
			UIObject *sender,
			int32 oldWidth,
			int32 oldHeight,
			int32 newWidth,
			int32 newHeight)
			: UIEvent(sender),
			oldWidth(oldWidth),
			oldHeight(oldHeight),
			newWidth(newWidth),
			newHeight(newHeight) {}
	};
}
