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
#include "gpu\RenderTarget.h"

namespace ui
{
    class UIObject : public SharedObject
    {
        friend class Window;
    protected:
		enum Background
		{
			BackgroundTransparent,
			BackgroundSolidColor,
			BackgroundLinearGradient,
			BackgroundRadialGradient,
		};

        Window *window;
        UIObject *parent;
		Vector2f position;
		Vector2f effectivePosition;
		float32 width;
		float32 height;
		float32 effectiveWidth;
		float32 effectiveHeight;
        UISize widthDesc;
        float32 minWidth;
        float32 maxWidth;
        UISize heightDesc;
        float32 minHeight;
        float32 maxHeight;
        Rect<UISize> margin;
		Rect<UISize> padding;
        Rect<float32> viewport;
		HorizontalAlign hAlign;
		VerticalAlign vAlign;
		bool flowBreak;
        Color foreground;
		Background bg;
		Color bgColor;
		GradientCollection *bgGradient;
		UISize bgGradientParam[6];
		ColorInterpolationMode bgColorMode;
		float32 borderThickness;
		Vector2f borderRadius;
		Color borderColor;
		float32 opacity;
		bool visible;
		bool enabled;
		bool focusable;
		bool updateRequired;
		uint32 eventHandleMask;
		uint32 eventHookMask;

        UIObject();
        ~UIObject();
		void SetWindow(Window *window);
		virtual Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
        virtual void PrepareImpl() {}
        virtual void RenderImpl(RenderTarget *rt, Vector2f p) {}
    public:
		// Returns nullptr if is not contained by any window
        void GetWindow(Window **window);
		// Can be nullptr
		// Used by containing elements
        void SetParent(UIObject *object);
		// Returns nullptr if has no parent
        void GetParent(UIObject **parent);
		void SetPosition(Vector2f value);
		// Value is relative to parent position
		Vector2f GetPosition();
		Vector2f GetEffectivePosition();
		Vector2f GetAbsolutePosition();
		float32 GetWidth();
		float32 GetEffectiveWidth();
		float32 GetHeight();
		// Get actual effectiveHeight
		float32 GetEffectiveHeight();
		// Set up effectiveWidth description for layouts to calculate element effectiveWidth,
		// does not initialize actual value
        void SetWidthDesc(UISize widthDesc);
        UISize GetWidthDesc();
		// Has no effect if effectiveWidth is auto
        void SetMinWidth(float32 value);
        float32 GetMinWidth();
		// Has no effect if effectiveWidth is auto
		void SetMaxWidth(float32 value);
		float32 GetMaxWidth();
		// Set up effectiveHeight description for layouts to calculate element effectiveHeight,
		// does not initialize actual value
        void SetHeightDesc(UISize heightDesc);
        UISize GetHeightDesc();
		// Has no effect if effectiveHeight is auto
        void SetMinHeight(float32 value);
        float32 GetMinHeight();
		// Has no effect if effectiveHeight is auto
		void SetMaxHeight(float32 value);
		float32 GetMaxHeight();
		Vector2f EvaluateSize(
			Vector2f parentSize,
			float32 *defaultWidth = nullptr,
			float32 *defaultHeight = nullptr,
			bool forceAutoWidth = false,
			bool forceAutoHeight = false);
		// Set up margin description for layouts to calculate element margin
        void SetMargin(Rect<UISize> &margin);
		Rect<UISize> GetMargin();
		// Set up padding description for layouts to calculate element padding
        void SetPadding(Rect<UISize> &padding);
		Rect<UISize> GetPadding();
		void SetHorizontalAlign(HorizontalAlign align);
		HorizontalAlign GetHorizontalAlign();
		void SetVerticalAlign(VerticalAlign align);
		VerticalAlign GetVerticalAlign();
		void EnableFlowBreak(bool value);
		bool IsFlowBreakEnabled();
		// Reassigned automatically when parent changes
        void SetForeground(Color value);
        Color GetForeground();
		void ClearBackground();
		void SetBackgroundColor(Color value);
		void SetBackgroundLinearGradient(
			GradientCollection *collection,
			UISize x1,
			UISize y1,
			UISize x2,
			UISize y2,
			ColorInterpolationMode colorMode = ColorInterpolationModeSmooth);
		void SetBackgroundRadialGradient(
			GradientCollection *collection,
			UISize cx,
			UISize cy,
			UISize rx,
			UISize ry,
			UISize offsetX = 0.0f,
			UISize offsetY = 0.0f,
			ColorInterpolationMode colorMode = ColorInterpolationModeSmooth);
		void SetBorderThickness(float32 value);
		float32 GetBorderThickness();
		void SetBorderRadius(Vector2f value);
		Vector2f GetBorderRadius();
		void SetBorderColor(Color value);
		Color GetBorderColor();
		void SetOpacity(float32 value);
		float32 GetOpacity();
		// Invisible elements do not handle and hook any events
        void SetVisible(bool value);
        bool IsVisible();
		// Disabled elements do not handle and hook any events
        void SetEnabled(bool value);
        bool IsEnabled();
		// If element is focusable it will receive focus when
		// hooks left or right mouse button click event,
		// Focus can be still set up manually regardless
        void SetFocusable(bool value);
        bool IsFocusable();
		// Set up which events in specified mask are handled
		// Unhandled events are pushed futher to contained elements by window controller
		void EnableEventHandle(UIHook event, bool value);
		// Check if all events in specified mask are handled
		bool IsEventHandled(UIHook event);
		// Set up which events in specified mask are hooked
		// and not pushed further to contained elements by window controller
		void EnableEventHook(UIHook event, bool value);
		// Check if all events in specified mask are hooked
		bool IsEventHooked(UIHook event);
		// States that element requires preparing
		void Update();
		// Used by containing elements
		void Prepare(
			float32 width,
			float32 height);
		void Repaint();
		// Used by containing elements
		void Render(RenderTarget *rt, Vector2f p);
		// Override to specify hit test for element
		// Default function threats area as rectangle
        virtual bool HitTest(Vector2f point);
		// Override to implement child elements access
		// Must be synchronized internally
		// Additional parameter must be pushed to function call
		virtual void ForEach(Function<void(UIObject *, void *)> callback, void *param) {}
		virtual void MouseClick(UIMouseEvent *e) {}
		Observer<UIMouseEvent *> onMouseClick;
		virtual void MouseRelease(UIMouseEvent *e) {}
		Observer<UIMouseEvent *> onMouseRelease;
		virtual void StartHover() {}
		Observer<> onStartHover;
		virtual void MouseMove(UIMouseEvent *e) {}
		Observer<UIMouseEvent *> onMouseMove;
		virtual void EndHover() {}
		Observer<> onEndHover;
		virtual void FocusReceive() {}
		Observer<> onFocusReceive;
		virtual void FocusLoss() {}
		Observer<> onFocusLoss;
		virtual void StartPull() {}
		Observer<> onStartPull;
		virtual void EndPull() {}
		Observer<> onEndPull;
		virtual void MouseWheelRotate(UIMouseWheelEvent *e) {}
		Observer<UIMouseWheelEvent *> onMouseWheelRotate;
		virtual void KeyPress(UIKeyboardEvent *e) {}
		Observer<UIKeyboardEvent *> onKeyPress;
		virtual void KeyRelease(UIKeyboardEvent *e) {}
		Observer<UIKeyboardEvent *> onKeyRelease;
		virtual void CharInput(UIKeyboardEvent *e) {}
		Observer<UIKeyboardEvent *> onCharInput;
    };
}
