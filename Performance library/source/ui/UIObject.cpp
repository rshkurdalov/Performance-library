// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "ui\UIObject.h"
#include "ui\UIManager.h"
#include "atc\StaticOperators.h"
#include "ui\Window.h"
#include "ui\UIFactory.h"

namespace ui
{
	UIObject::UIObject()
	{
		window = nullptr;
		parent = nullptr;
		minWidth = 0.0f;
		maxWidth = FLT_MAX;
		minHeight = 0.0f;
		maxHeight = FLT_MAX;
		padding = Rect<UISize>(0.0f, 0.0f, 0.0f, 0.0f);
		margin = Rect<UISize>(2.0f, 2.0f, 2.0f, 2.0f);
		hAlign = HorizontalAlignLeft;
		vAlign = VerticalAlignTop;
		flowBreak = false;
		foreground = Color::Black;
		bg = BackgroundTransparent;
		borderThickness = 0.0f;
		borderRadius = Vector2f(0.0f, 0.0f);
		borderColor = Color::Black;
		opacity = 1.0f;
		visible = true;
		enabled = true;
		focusable = false;
		updateRequired = true;
		eventHandleMask = (uint32)UIHookAll;
		eventHookMask = 0;
	}
	UIObject::~UIObject()
	{
		ClearBackground();
	}
	void UIObject::SetWindow(Window *window)
	{
		this->window = window;
		static void(*callback)(UIObject *, void *) = [](UIObject *object, void *param) -> void
		{
			object->SetWindow(object->parent->window);
		};
		ForEach(callback, nullptr);
	}
	Vector2f UIObject::EvaluateContentSizeImpl(
		float32 *viewportWidth,
		float32 *viewportHeight)
	{
		return Vector2f(widthDesc.value, heightDesc.value);
	}
	void UIObject::GetWindow(Window **window)
	{
		if (this->window != nullptr)
			this->window->AddRef();
		*window = this->window;
	}
	void UIObject::SetParent(UIObject *object)
	{
		parent = object;
		if (parent != nullptr)
			SetWindow(object->window);
	}
	void UIObject::GetParent(UIObject **parent)
	{
		if (this->parent != nullptr)
			this->parent->AddRef();
		*parent = this->parent;
	}
	void UIObject::SetPosition(Vector2f value)
	{
		position = value;
		effectivePosition = position
			+ Vector2f(margin.left.evaluate(width), margin.top.evaluate(height));
	}
	Vector2f UIObject::GetPosition()
	{
		return position;
	}
	Vector2f UIObject::GetEffectivePosition()
	{
		return effectivePosition;
	}
	Vector2f UIObject::GetAbsolutePosition()
	{
		Vector2f point = effectivePosition;
		UIObject *parentObject = parent;
		while (parentObject != nullptr)
		{
			point += parentObject->effectivePosition;
			parentObject = parentObject->parent;
		}
		return point;
	}
	float32 UIObject::GetWidth()
	{
		return width;
	}
	float32 UIObject::GetEffectiveWidth()
	{
		return effectiveWidth;
	}
	float32 UIObject::GetHeight()
	{
		return height;
	}
	float32 UIObject::GetEffectiveHeight()
	{
		return effectiveHeight;
	}
	void UIObject::SetWidthDesc(UISize widthDesc)
	{
		this->widthDesc = widthDesc;
		if (parent != nullptr)
			parent->Update();
	}
	UISize UIObject::GetWidthDesc()
	{
		return widthDesc;
	}
	void UIObject::SetMinWidth(float32 value)
	{
		minWidth = value;
		if (parent != nullptr)
			parent->Update();
	}
	float32 UIObject::GetMinWidth()
	{
		return minWidth;
	}
	void UIObject::SetMaxWidth(float32 value)
	{
		maxWidth = value;
		if (parent != nullptr)
			parent->Update();
	}
	float32 UIObject::GetMaxWidth()
	{
		return maxWidth;
	}
	void UIObject::SetHeightDesc(UISize heightDesc)
	{
		this->heightDesc = heightDesc;
		if (parent != nullptr)
			parent->Update();
	}
	UISize UIObject::GetHeightDesc()
	{
		return heightDesc;
	}
	void UIObject::SetMinHeight(float32 value)
	{
		minHeight = value;
		if (parent != nullptr)
			parent->Update();
	}
	float32 UIObject::GetMinHeight()
	{
		return minHeight;
	}
	void UIObject::SetMaxHeight(float32 value)
	{
		maxHeight = value;
		if (parent != nullptr)
			parent->Update();
	}
	float32 UIObject::GetMaxHeight()
	{
		return maxHeight;
	}
	Vector2f UIObject::EvaluateSize(
		Vector2f parentSize,
		float32 *defaultWidth,
		float32 *defaultHeight,
		bool forceAutoWidth,
		bool forceAutoHeight)
	{
		Vector2f size(widthDesc.value, heightDesc.value);
		if (widthDesc.sizeType == UISizeTypeRelative)
			size.x *= parentSize.x;
		size.x = Max(minWidth, size.x);
		size.x = Min(maxWidth, size.x);
		if (heightDesc.sizeType == UISizeTypeRelative)
			size.y *= parentSize.y;
		size.y = Max(minHeight, size.y);
		size.y = Min(maxHeight, size.y);
		forceAutoWidth |= widthDesc.sizeType == UISizeTypeAuto;
		forceAutoHeight |= heightDesc.sizeType == UISizeTypeAuto || forceAutoHeight;
		if (forceAutoWidth || forceAutoHeight)
		{
			Vector2f defaultSize = size;
			if (defaultWidth == nullptr && !forceAutoWidth)
				defaultWidth = &defaultSize.x;
			if (defaultHeight == nullptr && !forceAutoHeight)
				defaultHeight = &defaultSize.y;
			if (defaultWidth != nullptr)
			{
				*defaultWidth = Min(maxWidth, *defaultWidth);
				*defaultWidth = Max(minWidth, *defaultWidth);
				*defaultWidth -= margin.left.evaluate(*defaultWidth)
					+ margin.right.evaluate(*defaultWidth)
					+ padding.left.evaluate(*defaultWidth)
					+ padding.right.evaluate(*defaultWidth);
			}
			if (defaultHeight != nullptr)
			{
				*defaultHeight = Min(maxHeight, *defaultHeight);
				*defaultHeight = Max(minHeight, *defaultHeight);
				*defaultHeight -= margin.top.evaluate(*defaultHeight)
					+ margin.bottom.evaluate(*defaultHeight)
					+ padding.top.evaluate(*defaultHeight)
					+ padding.bottom.evaluate(*defaultHeight);
			}
			Vector2f autoSize = EvaluateContentSizeImpl(defaultWidth, defaultHeight);
			float32 denominator = 1.0f;
			if (padding.left.sizeType == UISizeTypeRelative)
				denominator -= padding.left.value;
			else autoSize.x += padding.left.value;
			if (padding.right.sizeType == UISizeTypeRelative)
				denominator -= padding.right.value;
			else autoSize.x += padding.right.value;
			if (margin.left.sizeType == UISizeTypeRelative)
				denominator -= margin.left.value;
			else autoSize.x += margin.left.value;
			if (margin.right.sizeType == UISizeTypeRelative)
				denominator -= margin.right.value;
			else autoSize.x += margin.right.value;
			autoSize.x /= denominator;
			autoSize.x = Max(minWidth, autoSize.x);
			autoSize.x = Min(maxWidth, autoSize.x);
			denominator = 1.0f;
			if (padding.top.sizeType == UISizeTypeRelative)
				denominator -= padding.top.value;
			else autoSize.y += padding.top.value;
			if (padding.bottom.sizeType == UISizeTypeRelative)
				denominator -= padding.bottom.value;
			else autoSize.y += padding.bottom.value;
			if (margin.top.sizeType == UISizeTypeRelative)
				denominator -= margin.top.value;
			else autoSize.y += margin.top.value;
			if (margin.bottom.sizeType == UISizeTypeRelative)
				denominator -= margin.bottom.value;
			else autoSize.y += margin.bottom.value;
			autoSize.y /= denominator;
			autoSize.y = Max(minHeight, autoSize.y);
			autoSize.y = Min(maxHeight, autoSize.y);
			if (forceAutoWidth)
				size.x = autoSize.x;
			if (forceAutoHeight)
				size.y = autoSize.y;
		}
		return size;
	}
	void UIObject::SetMargin(Rect<UISize> &margin)
	{
		this->margin = margin;
		if (parent != nullptr)
			parent->Update();
	}
	Rect<UISize> UIObject::GetMargin()
	{
		return margin;
	}
	void UIObject::SetPadding(Rect<UISize> &padding)
	{
		this->padding = padding;
		Update();
	}
	Rect<UISize> UIObject::GetPadding()
	{
		return padding;
	}
	void UIObject::SetHorizontalAlign(HorizontalAlign align)
	{
		hAlign = align;
		if (parent != nullptr)
			parent->Update();
	}
	HorizontalAlign UIObject::GetHorizontalAlign()
	{
		return hAlign;
	}
	void UIObject::SetVerticalAlign(VerticalAlign align)
	{
		vAlign = align;
		if (parent != nullptr)
			parent->Update();
	}
	VerticalAlign UIObject::GetVerticalAlign()
	{
		return vAlign;
	}
	void UIObject::EnableFlowBreak(bool value)
	{
		flowBreak = value;
		if (parent != nullptr)
			parent->Update();
	}
	bool UIObject::IsFlowBreakEnabled()
	{
		return flowBreak;
	}
	void UIObject::SetForeground(Color value)
	{
		foreground = value;
		Repaint();
	}
	Color UIObject::GetForeground()
	{
		return foreground;
	}
	void UIObject::ClearBackground()
	{
		if (bg == BackgroundLinearGradient
			|| bg == BackgroundRadialGradient)
			bgGradient->Unref();
		bg = BackgroundTransparent;
		Repaint();
	}
	void UIObject::SetBackgroundColor(Color value)
	{
		ClearBackground();
		bg = BackgroundSolidColor;
		bgColor = value;
	}
	void UIObject::SetBackgroundLinearGradient(
		GradientCollection *collection,
		UISize x1,
		UISize y1,
		UISize x2,
		UISize y2,
		ColorInterpolationMode colorMode)
	{
		ClearBackground();
		collection->AddRef();
		bg = BackgroundLinearGradient;
		bgGradient = collection;
		bgGradientParam[0] = x1;
		bgGradientParam[1] = y1;
		bgGradientParam[2] = x2;
		bgGradientParam[3] = y2;
		bgColorMode = colorMode;
	}
	void UIObject::SetBackgroundRadialGradient(
		GradientCollection *collection,
		UISize cx,
		UISize cy,
		UISize rx,
		UISize ry,
		UISize offsetX,
		UISize offsetY,
		ColorInterpolationMode colorMode)
	{
		ClearBackground();
		collection->AddRef();
		bg = BackgroundRadialGradient;
		bgGradient = collection;
		bgGradientParam[0] = cx;
		bgGradientParam[1] = cy;
		bgGradientParam[2] = rx;
		bgGradientParam[3] = ry;
		bgGradientParam[4] = offsetX;
		bgGradientParam[5] = offsetY;
		bgColorMode = colorMode;
	}
	void UIObject::SetBorderThickness(float32 value)
	{
		borderThickness = value;
		Repaint();
	}
	float32 UIObject::GetBorderThickness()
	{
		return borderThickness;
	}
	void UIObject::SetBorderRadius(Vector2f value)
	{
		borderRadius = value;
		Repaint();
	}
	Vector2f UIObject::GetBorderRadius()
	{
		return borderRadius;
	}
	void UIObject::SetBorderColor(Color value)
	{
		borderColor = value;
		Repaint();
	}
	Color UIObject::GetBorderColor()
	{
		return borderColor;
	}
	void UIObject::SetOpacity(float32 value)
	{
		opacity = value;
		Repaint();
	}
	float32 UIObject::GetOpacity()
	{
		return opacity;
	}
	void UIObject::SetVisible(bool value)
	{
		if (visible == value) return;
		visible = value;
		Repaint();
	}
	bool UIObject::IsVisible()
	{
		return visible;
	}
	void UIObject::SetEnabled(bool value)
	{
		if (enabled == value) return;
		enabled = value;
		Repaint();
	}
	bool UIObject::IsEnabled()
	{
		return enabled;
	}
	void UIObject::SetFocusable(bool value)
	{
		focusable = value;
		if (!focusable && UIManager::IsFocused(this))
			UIManager::SetFocus(nullptr);
		Repaint();
	}
	bool UIObject::IsFocusable()
	{
		return focusable;
	}
	void UIObject::EnableEventHandle(UIHook event, bool value)
	{
		if (value) eventHandleMask |= (uint32)event;
		else eventHandleMask &= ~(uint32)event;
	}
	bool UIObject::IsEventHandled(UIHook event)
	{
		return eventHandleMask & (uint32)event;
	}
	void UIObject::EnableEventHook(UIHook event, bool value)
	{
		if (value) eventHookMask |= (uint32)event;
		else eventHookMask &= ~(uint32)event;
	}
	bool UIObject::IsEventHooked(UIHook event)
	{
		return eventHookMask & (uint32)event;
	}
	void UIObject::Update()
	{
		if (!updateRequired)
		{
			updateRequired = true;
			Repaint();
			if (parent != nullptr)
				parent->Update();
		}
	}
	void UIObject::Prepare(
		float32 width,
		float32 height)
	{
		if (!updateRequired
			&& ScalarNearEqual(this->width, width, UIEps)
			&& ScalarNearEqual(this->height, height, UIEps)) return;
		this->width = width;
		this->height = height;
		effectivePosition = position + Vector2f(margin.left.evaluate(width), margin.top.evaluate(height));
		effectiveWidth = width - margin.left.evaluate(width) - margin.right.evaluate(width);
		effectiveHeight = height - margin.top.evaluate(height) - margin.bottom.evaluate(height);
		viewport.left = padding.left.evaluate(width);
		viewport.right = effectiveWidth - padding.right.evaluate(width);
		viewport.top = padding.top.evaluate(height);
		viewport.bottom = effectiveHeight - padding.bottom.evaluate(height);
		PrepareImpl();
		updateRequired = false;
	}
	void UIObject::Repaint()
	{

	}
	void UIObject::Render(RenderTarget *rt, Vector2f p)
	{
		if (!visible) return;
		rt->SetOpacity(opacity);
		if (bg != BackgroundTransparent)
		{
			if (bg == BackgroundSolidColor)
				rt->SetSolidColorBrush(bgColor);
			else if (bg == BackgroundLinearGradient)
			{
				rt->SetColorInterpolationMode(bgColorMode);
				rt->SetLinearGradientBrush(
					bgGradient,
					p + Vector2f(bgGradientParam[0].evaluate(effectiveWidth), bgGradientParam[1].evaluate(effectiveHeight)),
					p + Vector2f(bgGradientParam[2].evaluate(effectiveWidth), bgGradientParam[3].evaluate(effectiveHeight)));
			}
			else if (bg == BackgroundRadialGradient)
			{
				rt->SetColorInterpolationMode(bgColorMode);
				rt->SetRadialGradientBrush(
					bgGradient,
					p + Vector2f(bgGradientParam[0].evaluate(effectiveWidth), bgGradientParam[1].evaluate(effectiveHeight)),
					bgGradientParam[2].evaluate(effectiveWidth),
					bgGradientParam[3].evaluate(effectiveHeight),
					Vector2f(bgGradientParam[4].evaluate(effectiveWidth), bgGradientParam[5].evaluate(effectiveHeight)));
			}
			if (borderRadius == Vector2f(0.0f, 0.0f))
				rt->FillRectangle(p.x, p.y, effectiveWidth, effectiveHeight);
			else rt->FillRoundedRectangle(p.x, p.y, effectiveWidth, effectiveHeight, borderRadius.x, borderRadius.y);
		}
		if (borderThickness >= UIEps)
		{
			rt->SetSolidColorBrush(borderColor);
			if (borderRadius == Vector2f(0.0f, 0.0f))
				rt->DrawRectangle(p.x, p.y, effectiveWidth, effectiveHeight, borderThickness);
			else rt->DrawRoundedRectangle(p.x, p.y, effectiveWidth, effectiveHeight, borderRadius.x, borderRadius.y, borderThickness);
		}
		RenderImpl(rt, p);
	}
	bool UIObject::HitTest(Vector2f point)
	{
		return point.x >= 0.0f && point.x <= effectiveWidth
			&& point.y >= 0.0f && point.y <= effectiveHeight;
	}
}
