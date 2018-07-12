// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "graphics\Color.h"
#include "ui\UITypes.h"
#include "ui\UIEventArgs.h"
#include "util\Observer.h"
#include <string>

#define FONT_SIZE_DEFAULT 14.0f

namespace ui
{
    class UIObject : public SharedObject
    {
        friend class Window;
    protected:
        Window *window;
        aligned(UIObject *) parent;
		aligned(float32) x;
		aligned(float32) y;
		aligned(float32) width;
		aligned(float32) height;
        UISize widthDesc;
        float32 minWidth;
        float32 maxWidth;
        UISize heightDesc;
        float32 minHeight;
        float32 maxHeight;
        Rect<UISize> margin;
		Rect<UISize> padding;
        Rectf viewport;
		aligned(VerticalAlign) vAlign;
		aligned(HorizontalAlign) hAlign;
        Color foreground;
        bool foregroundInherit;
        std::wstring font;
        bool fontInherit;
        float32 fontSize;
        UISize fontSizeDesc;
		aligned(bool) visible;
		aligned(bool) enabled;
		aligned(bool) focusable;
		aligned(bool) updateRequired;
		aligned(uint32) eventHandleMask;
		aligned(uint32) eventHookMask;

        UIObject(Window *window);
        ~UIObject();
		float32 CalcWidth(float32 parentWidth);
		float32 CalcHeight(float32 parentHeight);
        void Update();
		void Prepare();
		void Render();
        virtual void ForEachImpl(
			Function<void(UIObject *)> callback) {}
        virtual void PrepareImpl(
            float32 viewportWidth,
            float32 viewportHeight,
            float32 &contentWidth,
            float32 &contentHeight) {}
        virtual void RenderImpl() {}
    public:
        Observer<UIMouseEvent> onMouseClick;
        Observer<UIMouseEvent> onMouseRelease;
        Observer<UIMouseEvent> onStartHover;
        Observer<UIMouseEvent> onMouseMove;
        Observer<UIMouseEvent> onEndHover;
        Observer<UIMouseEvent> onFocusReceive;
        Observer<UIMouseEvent> onFocusLoss;
        Observer<UIMouseWheelEvent> onMouseWheelRotate;
        Observer<UIKeyboardEvent> onKeyPress;
        Observer<UIKeyboardEvent> onKeyRelease;
        Observer<UIKeyboardEvent> onCharInput;

        Window *GetWindow();
        void SetParent(UIObject *object);
		// Returns nullptr if has no parent.
        void GetParent(UIObject **parent);
        void SetX(float32 value);
        float32 GetX();
        void SetY(float32 value);
        float32 GetY();
        void SetWidthDesc(UISize widthDesc);
        UISize GetWidthDesc();
        void SetMinWidth(float32 value);
        float32 GetMinWidth();
		void SetMaxWidth(float32 value);
		float32 GetMaxWidth();
        void SetWidth(float32 value);
        float32 GetWidth();
        void SetHeightDesc(UISize heightDesc);
        UISize GetHeightDesc();
        void SetMinHeight(float32 value);
        float32 GetMinHeight();
		void SetMaxHeight(float32 value);
		float32 GetMaxHeight();
        void SetHeight(float32 value);
        float32 GetHeight();
        void SetMargin(Rect<UISize> &margin);
		Rect<UISize> GetMargin();
        void SetPadding(Rect<UISize> &padding);
		Rect<UISize> GetPadding();
		void SetVerticalAlign(VerticalAlign align);
		VerticalAlign GetVerticalAlign();
		void SetHorizontalAlign(HorizontalAlign align);
		HorizontalAlign GetHorizontalAlign();
        void SetForeground(Color color, bool inherit = false);
        Color GetForeground();
        void SetFont(std::wstring &font, bool inherit = false);
		std::wstring GetFont();
        void SetFontSizeDesc(UISize sizeDesc);
        UISize GetFontSizeDesc();
        float32 GetFontSize();
        void EnableVerticalScrolling(bool value);
        bool IsScrollableVertically();
        void SetVerticalScrollOffset(float32 value);
        float32 GetVerticalScrollOffset();
        float32 GetVerticalContentSize();
        void EnableHorizontalScrolling(bool value);
        bool IsScrollableHorizontally();
        void SetHorizontalScrollOffset(float32 value);
        float32 GetHorizontalScrollOffset();
        float32 GetHorizontalContentSize();
        void SetVisible(bool value);
        bool IsVisible();
        void SetEnabled(bool value);
        bool IsEnabled();
        void SetFocusable(bool value);
        bool IsFocusable();
		void SetMouseHoverable(bool value);
		bool IsMouseHoverable();
		void SetMouseClickable(MouseButton button, bool value);
		bool IsMouseClickable(MouseButton button);
		void SetMouseWheelRotatable(bool value);
		bool isMouseWheelRotatable();
		void SetMouseHoverHooking(bool value);
		bool IsMouseHoverHookable();
		void SetMouseClickHooking(MouseButton button, bool value);
		bool IsMouseClickHookable(MouseButton button);
		void SetMouseWheelRotateHooking(bool value);
		bool IsMouseWheelRotateHookable();
        virtual bool ContainsPoint(Vector2f point);
    };
}
