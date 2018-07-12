// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "ui\UIEventArgs.h"
#include "util\Observer.h"
#include <vector>

namespace ui
{
    class Window : public SharedObject
    {
		friend class UIFactory;
        friend class UIObject;
    protected:
		msize hwnd;
        aligned(uint32) width;
        aligned(uint32) height;
        UIFactory *uiFactory;
        std::vector<UIObject *> foregroundObjects;
        //OptionList *context;
        //function<void(UIMouseEvent *)> closeContextCondition;
        UIObject *focusedObject;
        UIObject *hoveredObject;
        UIObject *draggedObject;
        Vector2f mousePosition;
        bool isClosed;

        virtual ~Window();
        UIObject *HitTest();
        UIObject *HitTest(UIObject *searhObject);
    public:
		Window(
			uint32 width,
			uint32 height,
			msize hwnd);

		void MouseClick(UIMouseEvent *e);
        Observer<UIMouseEvent> onMouseClick;
		void MouseRelease(UIMouseEvent *e);
        Observer<UIMouseEvent> onMouseRelease;
		void StartHover(UIMouseEvent *e);
        //Observer<UIMouseEvent> onStartHover;
		void MouseMove(UIMouseEvent *e);
        Observer<UIMouseEvent> onMouseMove;
		void EndHover(UIMouseEvent *e);
        //Observer<UIMouseEvent> onEndHover;
		void FocusReceive(UIMouseEvent *e);
        //Observer<UIMouseEvent> onFocusReceive;
		void FocusLoss(UIMouseEvent *e);
        //Observer<UIMouseEvent> onFocusLoss;
		void MouseWheelRotate(UIMouseWheelEvent *e);
        Observer<UIMouseWheelEvent> onMouseWheelRotate;
		void KeyPress(UIKeyboardEvent *e);
        Observer<UIKeyboardEvent> onKeyPress;
		void KeyRelease(UIKeyboardEvent *e);
        Observer<UIKeyboardEvent> onKeyRelease;
		void CharInput(UIKeyboardEvent *e);
        Observer<UIKeyboardEvent> onCharInput;
		void OnResize(UIResizeEvent *e);
        Observer<UIResizeEvent> onResize;

		msize GetHwnd();
		uint32 GetWidth();
		uint32 GetHeight();
        UIFactory *GetFactory();
        void SetFocus(UIObject *object);
        UIObject *GetFocus();
        UIObject *GetHover();
        void SetDragging(UIObject *object);
        UIObject *GetDragging();
        void PushForegroundObject(UIObject *object);
        void RemoveForegroundObject(UIObject *object);
        //void OpenContext(OptionList *object);
        //void CloseContext(OptionList *object);
        void CloseContext();
        UIObject *GetContext();
        Vector2f GetMousePosition();
        void Update();
    };
}
