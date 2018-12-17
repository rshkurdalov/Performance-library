// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "ui\UIEventArgs.h"
#include "util\Observer.h"

namespace ui
{
    class Window : public SharedObject
    {
    protected:
		uint64 hwnd;
        uint32 width;
        uint32 height;
		UIObject *layout;
        Vector2f mousePosition;
		RenderTarget *rt;

		void HitTestEvent(
			uint32 eventMask,
			UIObject **object,
			float32 *x,
			float32 *y);
    public:
		Window(
			uint64 hwnd,
			uint32 width,
			uint32 height,
			RenderTarget *rt,
			UIObject *layout);
		~Window();
		uint64 GetHwnd();
		uint32 GetWidth();
		uint32 GetHeight();
        Vector2f GetMousePosition();
		void GetLayout(UIObject **layout);
        void Update();
		void Open();
		void OpenModal(Window *parent);
		void Resize(uint32 width, uint32 height);
		void Close();
		void MouseClick(UIMouseEvent *e);
		Observer<UIMouseEvent *> onMouseClick;
		void MouseRelease(UIMouseEvent *e);
		Observer<UIMouseEvent *> onMouseRelease;
		void MouseMove(UIMouseEvent *e);
		Observer<UIMouseEvent *> onMouseMove;
		void MouseWheelRotate(UIMouseWheelEvent *e);
		Observer<UIMouseWheelEvent *> onMouseWheelRotate;
		void KeyPress(UIKeyboardEvent *e);
		Observer<UIKeyboardEvent *> onKeyPress;
		void KeyRelease(UIKeyboardEvent *e);
		Observer<UIKeyboardEvent *> onKeyRelease;
		void CharInput(UIKeyboardEvent *e);
		Observer<UIKeyboardEvent *> onCharInput;
		void OnResize(UIResizeEvent *e);
		Observer<UIResizeEvent *> onResize;
		void OnClose(UICloseEvent *e);
		Observer<UICloseEvent *> onClose;
    };
}
