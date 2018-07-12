// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "ui\Window.h"
#include "ui\UIObject.h"
/*#include "UIFactory.h"
#include "ScrollBar.h"
#include "Layout.h"
#include "CallbackTimer.h"*/

namespace ui
{
    Window::Window(
        uint32 width,
        uint32 height,
		msize hwnd)
    {
		this->hwnd = hwnd;
        this->width = width;
        this->height = height;

        /*uiFactory = new UIFactory(this);
        context = nullptr;
        closeContextCondition = [this] (UIMouseEvent *e) -> void
        {
            if(!HitTest(context)->Inside(e->x, e->y))
                CloseContext();
        };*/
        focusedObject = nullptr;
        hoveredObject = nullptr;
        draggedObject = nullptr;

        /*onMouseClick.AddCallback([this] (UIMouseEvent *e) -> void
        {
            if(draggedObject != nullptr) return;
            UIObject *receiver = HitTest();
            if((e->button == MouseButtonLeft || e->button == MouseButtonRight)
                && focusedObject != receiver)
            {
                if(focusedObject != nullptr)
                    focusedObject->onFocusLoss.Notify(e);
                if(receiver->focusable)
                {
                    focusedObject = receiver;
                    receiver->onFocusReceive.Notify(e);
                }
                else focusedObject = nullptr;
            }
            receiver->onMouseClick.Notify(e);
        });

        onMouseMove.AddCallback([this] (UIMouseEvent *e) -> void
        {
            UIObject *receiver = HitTest();
            if(hoveredObject != receiver)
            {
                hoveredObject->onEndHover.Notify(e);
                hoveredObject = receiver;
                hoveredObject->onStartHover.Notify(e);
            }
            if(draggedObject != nullptr)
            {
                draggedObject->onMouseMove.Notify(e);
                return;
            }
            receiver->onMouseMove.Notify(e);
        });

        onMouseRelease.AddCallback([this] (UIMouseEvent *e) -> void
        {
            if(draggedObject != nullptr && e->button == MouseButtonLeft)
            {
                draggedObject->onMouseRelease.Notify(e);
                draggedObject = nullptr;
                return;
            }
            UIObject *receiver = HitTest();
            receiver->onMouseRelease.Notify(e);
        });

        onMouseWheelRotate.AddCallback([this] (UIMouseWheelEvent *e) -> void
        {
            UIObject *receiver = HitTest();
            receiver->onMouseWheelRotate.Notify(e);
        });

        onKeyPress.AddCallback([this] (UIKeyboardEvent *e) -> void
        {
            if(draggedObject != nullptr) return;
            if(focusedObject != nullptr && focusedObject->enabled)
                focusedObject->onKeyPress.Notify(e);
        });

        onKeyRelease.AddCallback([this] (UIKeyboardEvent *e) -> void
        {
            if(draggedObject != nullptr) return;
            if(focusedObject != nullptr && focusedObject->enabled)
                focusedObject->onKeyRelease.Notify(e);
        });

        onCharInput.AddCallback([this] (UIKeyboardEvent *e) -> void
        {
            if(draggedObject != nullptr) return;
            if(focusedObject != nullptr && focusedObject->enabled)
                focusedObject->onCharInput.Notify(e);
        });*/
    }
    Window::~Window()
    {

    }
    UIObject *Window::HitTest()
    {
        UIObject *hitTest;
        for(size_t iter = foregroundObjects.size(); iter != 0; iter--)
        {
            if(foregroundObjects[iter - 1]->ContainsPoint(mousePosition))
            {
                hitTest = HitTest(foregroundObjects[iter - 1]);
                if(hitTest != nullptr) return hitTest;
            }
        }
        return foregroundObjects[0];
    }
    UIObject *Window::HitTest(UIObject *searchObject)
    {
        UIObject *nextObj = searchObject;
        /*searchObject->ForEachImpl([&nextObj, this] (UIObject *object) -> void
        {
            if(object->ContainsPoint(mousePosition)
                && object->visible)
                nextObj = object;
        });*/
        if(searchObject == nextObj)
        {
            if(searchObject->enabled)
                return searchObject;
            else return nullptr;
        }
        else
        {
            nextObj = HitTest(nextObj);
            if(nextObj == nullptr)
            {
                if(searchObject->enabled)
                    return searchObject;
                else return nullptr;
            }
            else return nextObj;
        }
    }

	msize Window::GetHwnd()
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
	void Window::MouseClick(UIMouseEvent *e)
	{

	}
	void Window::MouseRelease(UIMouseEvent *e)
	{

	}
	void Window::StartHover(UIMouseEvent *e)
	{

	}
	void Window::MouseMove(UIMouseEvent *e)
	{
		/*UIObject *receiver = HitTest();
		if (hoveredObject != receiver)
		{
			hoveredObject->onEndHover.Notify(e);
			hoveredObject = receiver;
			hoveredObject->onStartHover.Notify(e);
		}
		if (draggedObject != nullptr)
		{
			draggedObject->onMouseMove.Notify(e);
			return;
		}
		receiver->onMouseMove.Notify(e);
		if (e->requireUpdate) Update();*/
	}
	void Window::EndHover(UIMouseEvent *e)
	{

	}
	void Window::FocusReceive(UIMouseEvent *e)
	{

	}
	void Window::FocusLoss(UIMouseEvent *e)
	{

	}
	void Window::MouseWheelRotate(UIMouseWheelEvent *e)
	{

	}
	void Window::KeyPress(UIKeyboardEvent *e)
	{

	}
	void Window::KeyRelease(UIKeyboardEvent *e)
	{

	}
	void Window::CharInput(UIKeyboardEvent *e)
	{

	}
	void Window::OnResize(UIResizeEvent *e)
	{

	}
    UIFactory *Window::GetFactory()
    {
        return uiFactory;
    }
    void Window::SetFocus(UIObject *object)
    {
        if(focusedObject == object) return;
        UIMouseEvent focusEvent(nullptr, mousePosition.x, mousePosition.y);
        if(focusedObject != nullptr)
            focusedObject->onFocusLoss.Notify(&focusEvent);
        focusedObject = object;
        if(focusedObject != nullptr)
            focusedObject->onFocusReceive.Notify(&focusEvent);
        if(focusEvent.requireUpdate) Update();
    }
    UIObject *Window::GetFocus()
    {
        return focusedObject;
    }
    UIObject *Window::GetHover()
    {
        return hoveredObject;
    }
    void Window::SetDragging(UIObject *object)
    {
        draggedObject = object;
    }
    UIObject *Window::GetDragging()
    {
        return draggedObject;
    }
    void Window::PushForegroundObject(UIObject *object)
    {
        foregroundObjects.push_back(object);
    }
    void Window::RemoveForegroundObject(UIObject *object)
    {
        for(size_t iter = foregroundObjects.size(); iter != 0; iter--)
        {
            if(foregroundObjects[iter - 1] == object)
            {
                foregroundObjects.erase(foregroundObjects.begin() + iter - 1);
                break;
            }
        }
    }
    /*void Window::OpenContext(OptionList *object)
    {
        if(context != nullptr)
            CloseContext();
        Lock();
        context = object;
        PushForegroundObject(context);
        onMouseClick.AddCallback(closeContextCondition, 0);
        Unlock();
        Update();
    }
    void Window::CloseContext(OptionList *object)
    {
        Lock();
        if(context == nullptr
            || object != context)
        {
            Unlock();
            return;
        }
        RemoveForegroundObject(context);
        onMouseClick.RemoveCallback(closeContextCondition);
        context->onClose.Notify(&UIEvent(nullptr));
        context = nullptr;
        Unlock();
        Update();
    }*/
    void Window::CloseContext()
    {
        /*Lock();
        if(context == nullptr)
        {
            Unlock();
            return;
        }
        RemoveForegroundObject(context);
        onMouseClick.RemoveCallback(closeContextCondition);
        context->onClose.Notify(&UIEvent(nullptr));
        context = nullptr;
        Unlock();
        Update();*/
    }
    UIObject *Window::GetContext()
    {
		return 0;
        //return context;
    }
	Vector2f Window::GetMousePosition()
    {
        return mousePosition;
    }
    void Window::Update()
    {
        Lock();

        foregroundObjects[0]->width = foregroundObjects[0]->widthDesc.evaluate(width);
        foregroundObjects[0]->height = foregroundObjects[0]->heightDesc.evaluate(height);
        for(size_t iter = 0; iter < foregroundObjects.size(); iter++)
        {
            foregroundObjects[iter]->Prepare();
            foregroundObjects[iter]->Render();
        }

        Unlock();
    }
}
