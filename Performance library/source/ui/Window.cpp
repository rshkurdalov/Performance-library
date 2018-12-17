// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "ui\Window.h"
#include "ui\UIManager.h"
#include "ui\UIObject.h"
#include "kernel\OperatingSystemAPI.h"

namespace ui
{
    Window::Window(
		uint64 hwnd,
		uint32 width,
		uint32 height,
		RenderTarget *rt,
		UIObject *layout)
    {
		this->hwnd = hwnd;
        this->width = width;
        this->height = height;
		layout->AddRef();
		this->layout = layout;
		layout->SetWindow(this);
		layout->SetPosition(Vector2f(0.0f, 0.0f));
		layout->SetWidthDesc((float32)width);
		layout->SetHeightDesc((float32)height);
		layout->Prepare(layout->GetWidthDesc().value, layout->GetHeightDesc().value);
		rt->AddRef();
		this->rt = rt;
    }
    Window::~Window()
    {
		OSReleaseWindowHandler(this);
		layout->SetWindow(nullptr);
		layout->Unref();
    }
	void Window::HitTestEvent(
		uint32 eventMask,
		UIObject **object,
		float32 *x,
		float32 *y)
	{
		struct CallbackParams
		{
			UIObject *object;
			Vector2f mousePosition;
			Vector2f objectOrigin;
			uint32 eventMask;
			bool isHooked;
		} params;
		params.object = layout;
		params.mousePosition = mousePosition;
		params.objectOrigin = layout->GetEffectivePosition();
		params.eventMask = eventMask;
		params.isHooked = false;
		static void(*callback)(UIObject *, CallbackParams *) = [](UIObject *object, CallbackParams *params) -> void
		{
			params->objectOrigin += object->GetEffectivePosition();
			if (object->HitTest(params->mousePosition - params->objectOrigin)
				&& object->visible
				&& object->enabled
				&& ((uint32)object->eventHandleMask & params->eventMask)
				&& !params->isHooked)
			{
				params->object = object;
				if ((uint32)object->eventHookMask & params->eventMask)
					params->isHooked = true;
			}
			params->objectOrigin -= object->GetEffectivePosition();
		};
		UIObject *prevObject = params.object;
		while (true)
		{
			params.object->ForEach(callback, &params);
			if (prevObject == params.object || params.isHooked) break;
			prevObject = params.object;
			params.objectOrigin += params.object->GetEffectivePosition();
		}
		if (params.isHooked)
			params.objectOrigin += params.object->GetEffectivePosition();
		*object = params.object;
		*x = mousePosition.x - params.objectOrigin.x;
		*y = mousePosition.y - params.objectOrigin.y;
	}
	uint64 Window::GetHwnd()
	{
		return hwnd;
	}
	uint32 Window::GetWidth()
	{
		return width;
	}
	uint32 Window::GetHeight()
	{
		return height;
	}
	Vector2f Window::GetMousePosition()
    {
        return mousePosition;
    }
	void Window::GetLayout(UIObject **layout)
	{
		this->layout->AddRef();
		*layout = this->layout;
	}
    void Window::Update()
    {
		rt->Begin();
		layout->Prepare(layout->GetWidthDesc().value, layout->GetHeightDesc().value);
		layout->Render(rt, layout->GetEffectivePosition());
		rt->End();
    }
	void Window::Open()
	{
		OSOpenWindow(this);
		Update();
	}
	void Window::OpenModal(Window *parent)
	{
		LeaveSharedSection();
		OSEnableWindow(parent, false);
		OSOpenWindow(this);
		OSRunModalMsgLoop(this);
		OSEnableWindow(parent, true);
		OSOpenWindow(parent);
		EnterSharedSection();
	}
	void Window::Resize(uint32 width, uint32 height)
	{
		OSResizeWindow(this, width, height);
	}
	void Window::Close()
	{
		OSCloseWindow(this);
	}
	void Window::MouseClick(UIMouseEvent *e)
	{
		UIManager::SetPull(nullptr);
		onMouseClick.Notify(e);
		UIObject *receiver;
		HitTestEvent(
			1 << e->button,
			&receiver,
			&e->x,
			&e->y);
		receiver->AddRef();
		if (e->button == MouseButtonLeft || e->button == MouseButtonRight)
			UIManager::SetFocus(receiver);
		receiver->MouseClick(e);
		receiver->onMouseClick.Notify(e);
		receiver->Unref();
	}
	void Window::MouseRelease(UIMouseEvent *e)
	{
		onMouseRelease.Notify(e);
		UIObject *receiver;
		HitTestEvent(
			1 << e->button,
			&receiver,
			&e->x,
			&e->y);
		receiver->AddRef();
		receiver->MouseRelease(e);
		receiver->onMouseRelease.Notify(e);
		receiver->Unref();
		UIManager::SetPull(nullptr);
	}
	void Window::MouseMove(UIMouseEvent *e)
	{
		onMouseMove.Notify(e);
		mousePosition.x = e->x;
		mousePosition.y = e->y;
		UIObject *receiver;
		HitTestEvent(
			(uint32)(UIHookMouseHover),
			&receiver,
			&e->x,
			&e->y);
		receiver->AddRef();
		UIObject *pulledObject;
		UIManager::GetPull(&pulledObject);
		if (pulledObject != nullptr)
		{
			if (pulledObject != receiver)
			{
				UIMouseEvent pullEvent = *e;
				pullEvent.x = mousePosition.x - pulledObject->GetAbsolutePosition().x;
				pullEvent.y = mousePosition.y - pulledObject->GetAbsolutePosition().y;
				pulledObject->MouseMove(&pullEvent);
				pulledObject->onMouseMove.Notify(&pullEvent);
			}
			pulledObject->Unref();
		}
		UIManager::SetHover(receiver);
		receiver->MouseMove(e);
		receiver->onMouseMove.Notify(e);
		receiver->Unref();
	}
	void Window::MouseWheelRotate(UIMouseWheelEvent *e)
	{
		onMouseWheelRotate.Notify(e);
		UIObject *receiver;
		HitTestEvent(
			(uint32)UIHookMouseWhellRotation,
			&receiver,
			&e->x,
			&e->y);
		receiver->AddRef();
		receiver->MouseWheelRotate(e);
		receiver->onMouseWheelRotate.Notify(e);
		receiver->Unref();
	}
	void Window::KeyPress(UIKeyboardEvent *e)
	{
		onKeyPress.Notify(e);
		if (!UIManager::IsFocused(nullptr))
		{
			UIObject *focusedObject;
			UIManager::GetFocus(&focusedObject);
			if (focusedObject->enabled
				&& focusedObject->visible)
			{
				focusedObject->KeyPress(e);
				focusedObject->onKeyPress.Notify(e);
			}
			focusedObject->Unref();
		}
	}
	void Window::KeyRelease(UIKeyboardEvent *e)
	{
		onKeyRelease.Notify(e);
		if (!UIManager::IsFocused(nullptr))
		{
			UIObject *focusedObject;
			UIManager::GetFocus(&focusedObject);
			if (focusedObject->enabled
				&& focusedObject->visible)
			{
				focusedObject->KeyRelease(e);
				focusedObject->onKeyRelease.Notify(e);
			}
			focusedObject->Unref();
		}
	}
	void Window::CharInput(UIKeyboardEvent *e)
	{
		onCharInput.Notify(e);
		if (!UIManager::IsFocused(nullptr))
		{
			UIObject *focusedObject;
			UIManager::GetFocus(&focusedObject);
			if (focusedObject->enabled
				&& focusedObject->visible)
			{
				focusedObject->CharInput(e);
				focusedObject->onCharInput.Notify(e);
			}
			focusedObject->Unref();
		}
	}
	void Window::OnResize(UIResizeEvent *e)
	{
		this->width = e->newWidth;
		this->height = e->newHeight;
		rt->Resize(e->newWidth, e->newHeight);
		layout->SetWidthDesc((float32)width);
		layout->SetHeightDesc((float32)height);
		onResize.Notify(e);
	}
	void Window::OnClose(UICloseEvent *e)
	{
		onClose.Notify(e);
		if (e->confirmClose) Close();
	}
}
