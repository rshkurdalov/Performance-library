// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#pragma warning(disable: 4455)
#include "kernel\kernel.h"
#include <float.h>

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
			if (sizeType == UISizeTypeRelative)
				return value*parentSize;
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
	inline UISize operator""em(long double value)
	{
		return UISize((float32)value, UISizeTypeRelative);
	}
	inline UISize autosize(float32 defaultSize = FLT_MAX)
	{
		return UISize(defaultSize, UISizeTypeAuto);
	}
	
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

	enum MouseButton
	{
		MouseButtonNone,
		MouseButtonLeft,
		MouseButtonRight,
		MouseButtonWheel,
		MouseButtonX1,
		MouseButtonX2,
	};

	enum KeyCode
	{
		KeyCodeBackspace = 8,
		KeyCodeTab = 9,
		KeyCodeEnter = 13,
		KeyCodeLeft = 37,
		KeyCodeUp = 38,
		KeyCodeRight = 39,
		KeyCodeDown = 40,
	};

	enum UIHook
	{
		UIHookMouseButtonLeft = 1 << MouseButtonLeft,
		UIHookMouseButtonRight = 1 << MouseButtonRight,
		UIHookMouseButtonWheel = 1 << MouseButtonWheel,
		UIHookMouseButtonX1 = 1 << MouseButtonX1,
		UIHookMouseButtonX2 = 1 << MouseButtonX2,
		UIHookMouseButtonAll = 63,
		UIHookMouseHover = 1 << 6,
		UIHookMouseWhellRotation = 1 << 7,
		UIHookAll = 255,
	};

	const float32 UIEps = 1e-2f;
}
