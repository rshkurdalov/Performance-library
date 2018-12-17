#pragma once
#include "kernel\kernel.h"
#include "atc\Function.h"
#include "math\VectorMath.h"

namespace ui
{
	HResult UIInitialize();

	enum Cursor : uint32
	{
		CursorDefault,
		CursorBeam,
	};

	class UIManager
	{
	public:
		// Change global factoty for user interface components
		// so the elements that are created internally have custom style
		static void SetFactory(UIFactory *factory);
		// Returns default factory if another was not set up
		static void GetFactory(UIFactory **ppFactory);
		// Measured in nanoseconds
		// If set 0, double click will never occur
		static void SetDoubleClickThreshold(int64 value);
		static int64 GetDoubleClickThreshold();
		// Replace standard procedure for switching cursor
		static void SetCursorSwitchCallback(Function<void(uint32, Window *)> callback);
		// Standart values are defined in enum Cursor
		static void SwitchCursor(uint32 cursor, Window *window);
		static void SetHover(UIObject *object);
		static void GetHover(UIObject **object);
		static bool IsHovered(UIObject *object);
		static void SetFocus(UIObject *object);
		static void GetFocus(UIObject **object);
		static bool IsFocused(UIObject *object);
		static void SetPull(UIObject *object);
		static void GetPull(UIObject **object);
		static bool IsPulled(UIObject *object);
		// Launch timer for caret visibility
		// and synchronously repaint object
		static void LaunchCaretTimer(UIObject *object);
		static bool IsCaretVisible();
		// Stop timer and dereference object
		static void StopCaretTimer();
	};
}
