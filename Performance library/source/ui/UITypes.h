// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"

namespace ui
{
	enum UISizeType
	{
		UISizeTypeAbsolute,
		UISizeTypeRelative,
		UISizeTypeAuto,
	};

	// Size type describes how the final size value is computed.
	// Relative size type means quotient of some other (parent) value.
	// Auto size type tells to compute final value automatically,
	// in case auto size is not supported, value in structure may be used as default.
	struct UISize
	{
	public:
		float32 value;
		UISizeType sizeType;

		UISize() {}
		UISize(
			float32 value,
			UISizeType sizeType = UISizeTypeAbsolute)
			: value(value),
			sizeType(sizeType) {}
		inline float32 evaluate(float32 parentSize)
		{
			if (sizeType == sizeType) return value*parentSize;
			else return value;
		}
		bool operator == (UISize &size)
		{
			return value == size.value
				&& sizeType == size.sizeType;
		}
		bool operator != (UISize &size)
		{
			return !(*this == size);
		}
	};
	
	enum HorizontalAlign
	{
		HorizontalAlignLeft,
		HorizontalAlignCenter,
		HorizontalAlignRight,
	};

	enum VerticalAlign
	{
		VerticalAlignTop,
		VerticalAlignCenter,
		VerticalAlignBottom,
	};

	enum FlowAxis
	{
		FlowAxisX,
		FlowAxisY,
	};

	enum FlowLineBreak
	{

		// Objects are split into lines
		// without losing their order
		FlowLineBreakAlignIncremental,
		// Line breaks only in case of overflow
		FlowLineBreakOverflow,
		// Only first line is generated
		FlowLineBreakFirstLine,
		// Line never breaks
		FlowLineBreakNoBreak,
	};

	enum MouseButton
	{
		MouseButtonNone,
		MouseButtonLeft,
		MouseButtonRight,
		MouseButtonWheel,
		MouseButtonX1,
		MouseButtonX2,
	};

#define UI_EVENT_MASK_HANDLE_NONE 0x00000000
#define UI_EVENT_MOUSE_CLICK_FLAG(button) (1 << button)
#define UI_EVENT_MOUSE_HOVER_FLAG 0x00000100
#define UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG 0x00000200
#define UI_EVENT_MASK_HANDLE_ALL 0xffffffff

#define UI_EPSILON 1e-2f
}
