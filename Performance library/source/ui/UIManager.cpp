#include "ui\UIManager.h"
#include "ui\UIFactory.h"
#include "ui\UIObject.h"
#include "ui\Window.h"
#include "kernel\OperatingSystemAPI.h"
#include "util\CallbackTimer.h"
#include <chrono>

using namespace std::chrono;

namespace ui
{
	UIFactory *globalFactory;
	int64 dblClickThreshold;
	Function<void(uint32, Window*)> cursorSwitch;
	UIObject *hoveredObject;
	UIObject *focusedObject;
	UIObject *pulledObject;
	UIObject *caretObject;
	bool caretVisible;
	CallbackTimer *caretTimer;

	HResult UIInitialize()
	{
		globalFactory = new UIFactory();
		dblClickThreshold = nanoseconds(500ms).count();
		cursorSwitch = OSSetCursor;
		hoveredObject = nullptr;
		focusedObject = nullptr;
		caretObject = nullptr;
		static void(*timerCallback)(TimerEvent *) = [](TimerEvent *e) -> void
		{
			caretVisible = !caretVisible;
			caretObject->Repaint();
			Window *window;
			caretObject->GetWindow(&window);
			if (window != nullptr)
			{
				window->Update();
				window->Unref();
			}
		};
		Time::CreateCallbackTimer(
			timerCallback,
			nullptr,
			nanoseconds(500ms).count(),
			&caretTimer);
		return HResultSuccess;
	}

	void UIManager::SetFactory(UIFactory *factory)
	{
		factory->AddRef();
		globalFactory->Unref();
		globalFactory = factory;
	}
	void UIManager::GetFactory(UIFactory **ppFactory)
	{
		globalFactory->AddRef();
		*ppFactory = globalFactory;
	}
	void UIManager::SetDoubleClickThreshold(int64 value)
	{
		dblClickThreshold = value;
	}
	int64 UIManager::GetDoubleClickThreshold()
	{
		return dblClickThreshold;
	}
	void UIManager::SetCursorSwitchCallback(Function<void(uint32, Window *)> callback)
	{
		cursorSwitch = callback;
	}
	void UIManager::SwitchCursor(uint32 cursor, Window *window)
	{
		cursorSwitch(cursor, window);
	}
	void UIManager::SetHover(UIObject *object)
	{
		if (hoveredObject == object) return;
		if (hoveredObject != nullptr)
		{
			hoveredObject->EndHover();
			hoveredObject->onEndHover.Notify();
			hoveredObject->Unref();
		}
		hoveredObject = object;
		if (hoveredObject != nullptr)
		{
			hoveredObject->AddRef();
			hoveredObject->StartHover();
			hoveredObject->onStartHover.Notify();
		}
	}
	void UIManager::GetHover(UIObject **object)
	{
		if (hoveredObject != nullptr)
			hoveredObject->AddRef();
		*object = hoveredObject;
	}
	bool UIManager::IsHovered(UIObject *object)
	{
		return hoveredObject == object;
	}
	void UIManager::SetFocus(UIObject *object)
	{
		if (focusedObject == object) return;
		if (focusedObject != nullptr)
		{
			focusedObject->FocusLoss();
			focusedObject->onFocusLoss.Notify();
			focusedObject->Unref();
		}
		if (object != nullptr && object->IsFocusable())
		{
			focusedObject = object;
			focusedObject->AddRef();
			focusedObject->FocusReceive();
			focusedObject->onFocusReceive.Notify();
		}
		else focusedObject = nullptr;
	}
	void UIManager::GetFocus(UIObject **object)
	{
		if (focusedObject != nullptr)
			focusedObject->AddRef();
		*object = focusedObject;
	}
	bool UIManager::IsFocused(UIObject *object)
	{
		return focusedObject == object;
	}
	void UIManager::SetPull(UIObject *object)
	{
		if (pulledObject != nullptr)
		{
			pulledObject->EndPull();
			pulledObject->onEndPull.Notify();
			pulledObject->Unref();
		}
		pulledObject = object;
		if (pulledObject != nullptr)
		{
			pulledObject->AddRef();
			pulledObject->StartPull();
			pulledObject->onStartPull.Notify();
		}
	}
	void UIManager::GetPull(UIObject **object)
	{
		if (pulledObject != nullptr)
			pulledObject->AddRef();
		*object = pulledObject;
	}
	bool UIManager::IsPulled(UIObject *object)
	{
		return pulledObject == object;
	}
	void UIManager::LaunchCaretTimer(UIObject *object)
	{
		StopCaretTimer();
		object->AddRef();
		caretObject = object;
		caretVisible = true;
		caretTimer->Start();
	}
	bool UIManager::IsCaretVisible()
	{
		return caretVisible;
	}
	void UIManager::StopCaretTimer()
	{
		if (caretObject != nullptr)
		{
			caretObject->Unref();
			caretObject = nullptr;
		}
		caretTimer->Stop();
	}
}
