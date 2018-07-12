// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "ui\UIObject.h"
#include "atc\StaticOperators.h"
#include "ui\Window.h"
//#include "UIFactory.h"
#include "ui\ScrollBar.h"

namespace ui
{
	UIObject::UIObject(Window *window)
		: foreground(Color::Black),
		font(L"cambria"),
		fontSizeDesc(FONT_SIZE_DEFAULT),
		fontSize(FONT_SIZE_DEFAULT)
	{
		window->AddRef();
		this->window = window;
		parent = nullptr;
		minWidth = 0.0f;
		maxWidth = FLT_MAX;
		minHeight = 0.0f;
		maxHeight = FLT_MAX;
		padding = Rect<UISize>(0.0f, 0.0f, 0.0f, 0.0f);
		margin = Rect<UISize>(2.0f, 2.0f, 2.0f, 2.0f);
		vAlign = VerticalAlignTop;
		hAlign = HorizontalAlignLeft;
		foregroundInherit = true;
		fontInherit = true;
		visible = true;
		enabled = true;
		focusable = false;
		eventHandleMask = UI_EVENT_MASK_HANDLE_ALL;
		eventHookMask = UI_EVENT_MASK_HANDLE_NONE;
		updateRequired = true;

		/*onMouseWheelRotate.AddCallback([this] (UIMouseWheelEvent *e) -> void
		{
			if(IsScrollableVertically()
				&& verticalScroll->contentSize > viewport.bottom - viewport.top + 1e-2)
			{
				e->denyCallbacks = true;
				if(e->delta < 0) verticalScroll->contentOffset += verticalScroll->scrollingStep;
				else verticalScroll->contentOffset -= verticalScroll->scrollingStep;
				verticalScroll->contentOffset = max(verticalScroll->contentOffset, 0);
				verticalScroll->contentOffset = min(verticalScroll->contentOffset,
					verticalScroll->contentSize - (viewport.bottom - viewport.top));
				e->requireUpdate = true;
			}
			else if(IsScrollableHorizontally()
				&& horizontalScroll->contentSize > viewport.right - viewport.left + 1e-2)
			{
				e->denyCallbacks = true;
				if(e->delta < 0) horizontalScroll->contentOffset += horizontalScroll->scrollingStep;
				else horizontalScroll->contentOffset -= horizontalScroll->scrollingStep;
				horizontalScroll->contentOffset = max(horizontalScroll->contentOffset, 0);
				horizontalScroll->contentOffset = min(horizontalScroll->contentOffset,
					horizontalScroll->contentSize - (viewport.right - viewport.left));
				e->requireUpdate = true;
			}
		});*/
	}
	UIObject::~UIObject()
	{
		window->Release();
	}
	float32 UIObject::CalcWidth(float32 parentWidth)
	{
		float32 value = widthDesc.value;
		if (widthDesc.sizeType == UISizeTypeRelative)
			value *= parentWidth;
		if (value < minWidth) return minWidth;
		else if (value > maxWidth) return maxWidth;
		else return value;
	}
	float32 UIObject::CalcHeight(float32 parentHeight)
	{
		float32 value = heightDesc.value;
		if (heightDesc.sizeType == UISizeTypeRelative)
			value *= parentHeight;
		if (value < minHeight) return minHeight;
		else if (value > maxHeight) return maxHeight;
		else return value;
	}
	void UIObject::Update()
	{
		if (!updateRequired)
		{
			updateRequired = true;
			if (parent != nullptr)
				parent->Update();
		}
	}
	void UIObject::Prepare()
	{
		if (!updateRequired) return;
		Lock();
		float32 viewportWidth = width
			- padding.left.evaluate(width)
			- padding.right.evaluate(width),
			viewportHeight = height
			- padding.top.evaluate(height)
			- padding.bottom.evaluate(height);
		viewportWidth = Max(viewportWidth, 0.0f);
		viewportHeight = Max(viewportHeight, 0.0f);
		float32 contentWidth = viewportWidth,
			contentHeight = viewportHeight;
		PrepareImpl(
			viewportWidth,
			viewportHeight,
			contentWidth,
			contentHeight);
		if (widthDesc.sizeType == UISizeTypeAuto)
		{
			viewportWidth = contentWidth;
			float32 upperNumber = viewportWidth,
				lowerNumber = 1.0f;
			if (padding.left.sizeType == UISizeTypeRelative)
				lowerNumber -= padding.left.value;
			else upperNumber += padding.left.value;
			if (padding.right.sizeType == UISizeTypeRelative)
				lowerNumber -= padding.right.value;
			else upperNumber += padding.right.value;
			width = upperNumber / lowerNumber;
			if (width < minWidth)
			{
				width = minWidth;
				viewportWidth = width
					- padding.left.evaluate(width)
					- padding.right.evaluate(width);
				updateRequired = true;
			}
			else if (width > maxWidth)
			{
				width = maxWidth;
				viewportWidth = width
					- padding.left.evaluate(width)
					- padding.right.evaluate(width);
				updateRequired = true;
			}
		}
		if (heightDesc.sizeType == UISizeTypeAuto)
		{
			viewportHeight = contentHeight;
			float32 upperNumber = viewportHeight,
				lowerNumber = 1.0f;
			if (padding.top.sizeType == UISizeTypeRelative)
				lowerNumber -= padding.top.value;
			else upperNumber += padding.top.value;
			if (padding.bottom.sizeType == UISizeTypeRelative)
				lowerNumber -= padding.bottom.value;
			else upperNumber += padding.bottom.value;
			height = upperNumber / lowerNumber;
			if (height < minHeight)
			{
				height = minHeight;
				viewportWidth = height
					- padding.top.evaluate(height)
					- padding.bottom.evaluate(height);
				updateRequired = true;
			}
			else if (height > maxHeight)
			{
				height = maxHeight;
				viewportWidth = height
					- padding.top.evaluate(height)
					- padding.bottom.evaluate(height);
				updateRequired = true;
			}
		}
		viewport.left = x + padding.left.evaluate(width);
		viewport.right = x + width - padding.right.evaluate(width);
		viewport.top = y + padding.top.evaluate(height);
		viewport.bottom = y + height - padding.bottom.evaluate(height);
		/*if (foregroundObjects[0] != nullptr)
			dynamic_cast<ScrollBar *>(foregroundObjects[0])->contentSize = contentHeight;
		if (foregroundObjects[1] != nullptr)
			dynamic_cast<ScrollBar *>(foregroundObjects[1])->contentSize = contentWidth;*/
		updateRequired = false;
		Unlock();
	}
	void UIObject::Render()
	{
		if (!visible) return;
		Prepare();
		Lock();
		ScrollBar *verticalScroll = nullptr, *horizontalScroll = nullptr;
		/*if (foregroundObjects[0] != nullptr)
			verticalScroll = dynamic_cast<ScrollBar *>(foregroundObjects[0]);
		if (foregroundObjects[1] != nullptr)
			horizontalScroll = dynamic_cast<ScrollBar *>(foregroundObjects[1]);*/
		/*if (IsScrollableVertically()
			&& verticalScroll->contentSize > viewport.bottom - viewport.top + 1e-2)
		{
			if (IsScrollableHorizontally()
				&& horizontalScroll->contentSize > viewport.right - viewport.left + 1e-2)
				verticalScroll->SetHeight(height - horizontalScroll->GetHeightDesc().evaluate(height));
			else verticalScroll->SetHeight(height);
			verticalScroll->SetWidth(verticalScroll->GetWidthDesc().evaluate(width));
			viewport.right -= verticalScroll->GetWidth();
		}
		if (IsScrollableHorizontally()
			&& horizontalScroll->contentSize > viewport.right - viewport.left + 1e-2)
		{
			if (IsScrollableVertically()
				&& verticalScroll->contentSize > viewport.bottom - viewport.top + 1e-2)
				horizontalScroll->SetWidth(width - verticalScroll->GetWidth());
			else horizontalScroll->SetWidth(width);
			horizontalScroll->SetHeight(horizontalScroll->GetHeightDesc().evaluate(height));
			viewport.bottom -= horizontalScroll->GetHeight();
		}*/
		RenderImpl();
		/*if (IsScrollableVertically()
			&& verticalScroll->contentSize > viewport.bottom - viewport.top + 1e-2)
		{
			verticalScroll->SetX(x + width - verticalScroll->GetWidth());
			verticalScroll->SetY(y);
			verticalScroll->viewportSize = viewport.bottom - viewport.top;
			verticalScroll->contentOffset = Max(verticalScroll->contentOffset, 0);
			verticalScroll->contentOffset = Min(verticalScroll->contentOffset,
				verticalScroll->contentSize - (viewport.bottom - viewport.top));
			verticalScroll->SetVisible(true);
			verticalScroll->Prepare();
			verticalScroll->Render();
		}
		else if (verticalScroll != nullptr)
		{
			verticalScroll->contentOffset = 0;
			verticalScroll->SetVisible(false);
		}
		if (IsScrollableHorizontally()
			&& horizontalScroll->contentSize > viewport.right - viewport.left + 1e-2)
		{
			horizontalScroll->SetX(x);
			horizontalScroll->SetY(y + height - horizontalScroll->GetHeight());
			horizontalScroll->viewportSize = viewport.right - viewport.left;
			horizontalScroll->contentOffset = Max(horizontalScroll->contentOffset, 0);
			horizontalScroll->contentOffset = Min(horizontalScroll->contentOffset,
				horizontalScroll->contentSize - (viewport.right - viewport.left));
			horizontalScroll->SetVisible(true);
			horizontalScroll->Prepare();
			horizontalScroll->Render();
		}
		else if (verticalScroll != nullptr)
		{
			horizontalScroll->contentOffset = 0;
			horizontalScroll->SetVisible(false);
		}*/
		Unlock();
	}

	Window *UIObject::GetWindow()
	{
		return window;
	}
	void UIObject::SetParent(UIObject *object)
	{
		parent = object;
		if (foregroundInherit)
			SetForeground(foreground, true);
		if (fontInherit)
			SetFont(font, true);
		SetFontSizeDesc(fontSizeDesc);
	}
	void UIObject::GetParent(UIObject **parent)
	{
		if (this->parent != nullptr)
			this->parent->AddRef();
		*parent = this->parent;
	}
	void UIObject::SetX(float32 value)
	{
		x = value;
	}
	float32 UIObject::GetX()
	{
		return x;
	}
	void UIObject::SetY(float32 value)
	{
		y = value;
	}
	float32 UIObject::GetY()
	{
		return y;
	}
	void UIObject::SetWidthDesc(UISize widthDesc)
	{
		this->widthDesc = widthDesc;
	}
	UISize UIObject::GetWidthDesc()
	{
		return widthDesc;
	}
	void UIObject::SetMinWidth(float32 value)
	{
		minWidth = value;
	}
	float32 UIObject::GetMinWidth()
	{
		return minWidth;
	}
	void UIObject::SetMaxWidth(float32 value)
	{
		maxWidth = value;
	}
	float32 UIObject::GetMaxWidth()
	{
		return maxWidth;
	}
	void UIObject::SetWidth(float32 value)
	{
		if (!ScalarNearEqual(width, value, UI_EPSILON)) Update();
		width = value;
	}
	float32 UIObject::GetWidth()
	{
		return width;
	}
	void UIObject::SetHeightDesc(UISize heightDesc)
	{
		this->heightDesc = heightDesc;
	}
	UISize UIObject::GetHeightDesc()
	{
		return heightDesc;
	}
	void UIObject::SetMinHeight(float32 value)
	{
		minHeight = value;
	}
	float32 UIObject::GetMinHeight()
	{
		return minHeight;
	}
	void UIObject::SetMaxHeight(float32 value)
	{
		maxHeight = value;
	}
	float32 UIObject::GetMaxHeight()
	{
		return maxHeight;
	}
	void UIObject::SetHeight(float32 value)
	{
		if (!ScalarNearEqual(height, value, UI_EPSILON)) Update();
		height = value;
	}
	float32 UIObject::GetHeight()
	{
		return height;
	}
	void UIObject::SetMargin(Rect<UISize> &margin)
	{
		this->margin = margin;
	}
	Rect<UISize> UIObject::GetMargin()
	{
		return margin;
	}
	void UIObject::SetPadding(Rect<UISize> &padding)
	{
		this->padding = padding;
	}
	Rect<UISize> UIObject::GetPadding()
	{
		return padding;
	}
	void UIObject::SetVerticalAlign(VerticalAlign align)
	{
		vAlign = align;
	}
	VerticalAlign UIObject::GetVerticalAlign()
	{
		return vAlign;
	}
	void UIObject::SetHorizontalAlign(HorizontalAlign align)
	{
		hAlign = align;
	}
	HorizontalAlign UIObject::GetHorizontalAlign()
	{
		return hAlign;
	}
	void UIObject::SetForeground(Color color, bool inherit)
	{
		Lock();
		foregroundInherit = inherit;
		if (inherit && parent != nullptr)
			foreground = parent->foreground;
		else foreground = color;
		void(*callback)(UIObject *) = [] (UIObject *object) -> void
		{
			if (object->foregroundInherit)
				object->SetForeground(object->parent->foreground, true);
		};
		ForEachImpl(callback);
		Update();
		Unlock();
	}
	Color UIObject::GetForeground()
	{
		Lock();
		Color value = foreground;
		Unlock();
		return value;
	}
	void UIObject::SetFont(std::wstring &font, bool inherit)
	{
		Lock();
		fontInherit = inherit;
		if (inherit && parent != nullptr)
			this->font = parent->font;
		else this->font = font;
		void(*callback)(UIObject *) = [] (UIObject *object) -> void
		{
			if (object->fontInherit)
				object->SetFont(object->parent->font, true);
		};
		ForEachImpl(callback);
		Update();
		Unlock();
	}
	std::wstring UIObject::GetFont()
	{
		Lock();
		std::wstring value = font;
		Unlock();
		return value;
	}
	void UIObject::SetFontSizeDesc(UISize sizeDesc)
	{
		Lock();
		fontSizeDesc = sizeDesc;
		if (sizeDesc.sizeType == UISizeTypeRelative)
		{
			if (parent != nullptr)
				fontSize = sizeDesc.value*parent->fontSize;
			else fontSize = FONT_SIZE_DEFAULT;
		}
		else fontSize = sizeDesc.value;
		void(*callback)(UIObject *) = [] (UIObject *object) -> void
		{
			if (object->fontSizeDesc.sizeType == UISizeTypeRelative)
				object->SetFontSizeDesc(object->GetFontSizeDesc());
		};
		ForEachImpl(callback);
		Update();
		Unlock();
	}
	UISize UIObject::GetFontSizeDesc()
	{
		return fontSizeDesc;
	}
	float32 UIObject::GetFontSize()
	{
		return fontSize;
	}
	/*void UIObject::EnableVerticalScrolling(bool value)
	{
		if (value==true && foregroundObjects[0] == nullptr)
		{
			//window->GetFactory()->CreateScrollBar(true, &foregroundObjects[0]);
			foregroundObjects[0]->SetParent(this);
		}
	}
	bool UIObject::IsScrollableVertically()
	{
		return foregroundObjects[0]!=nullptr
			&& foregroundObjects[0]->IsEnabled();
	}
	void UIObject::SetVerticalScrollOffset(float32 value)
	{
		if (foregroundObjects[0] == nullptr) return;
		dynamic_cast<ScrollBar *>(foregroundObjects[0])->contentOffset = value;
	}
	float32 UIObject::GetVerticalScrollOffset()
	{
		if (foregroundObjects[0] == nullptr) return 0;
		return dynamic_cast<ScrollBar *>(foregroundObjects[0])->contentOffset;
	}
	float32 UIObject::GetVerticalContentSize()
	{
		if (foregroundObjects[0] == nullptr) return 0;
		return dynamic_cast<ScrollBar *>(foregroundObjects[0])->contentSize;
	}
	void UIObject::EnableHorizontalScrolling(bool value)
	{
		if (value == true && foregroundObjects[1] == nullptr)
		{
			//window->GetFactory()->CreateScrollBar(false, &foregroundObjects[1]);
			foregroundObjects[1]->SetParent(this);
		}
	}
	bool UIObject::IsScrollableHorizontally()
	{
		return foregroundObjects[1] != nullptr
			&& foregroundObjects[1]->IsEnabled();
	}
	void UIObject::SetHorizontalScrollOffset(float32 value)
	{
		if (foregroundObjects[1] == nullptr) return;
		dynamic_cast<ScrollBar *>(foregroundObjects[1])->contentOffset = value;
	}
	float32 UIObject::GetHorizontalScrollOffset()
	{
		if (foregroundObjects[1] == nullptr) return 0;
		return dynamic_cast<ScrollBar *>(foregroundObjects[1])->contentOffset;
	}
	float32 UIObject::GetHorizontalContentSize()
	{
		if (foregroundObjects[0] == nullptr) return 0;
		return dynamic_cast<ScrollBar *>(foregroundObjects[1])->contentSize;
	}*/
	void UIObject::SetVisible(bool value)
	{
		visible = value;
	}
	bool UIObject::IsVisible()
	{
		return visible;
	}
	void UIObject::SetEnabled(bool value)
	{
		enabled = value;
	}
	bool UIObject::IsEnabled()
	{
		return enabled;
	}
	void UIObject::SetFocusable(bool value)
	{
		focusable = value;
	}
	bool UIObject::IsFocusable()
	{
		return focusable;
	}
	void UIObject::SetMouseHoverable(bool value)
	{
		if (value) eventHandleMask |= UI_EVENT_MOUSE_HOVER_FLAG;
		else eventHandleMask &= ~UI_EVENT_MOUSE_HOVER_FLAG;
	}
	bool UIObject::IsMouseHoverable()
	{
		return eventHandleMask & UI_EVENT_MOUSE_HOVER_FLAG;
	}
	void UIObject::SetMouseClickable(MouseButton button, bool value)
	{
		if (value) eventHandleMask |= UI_EVENT_MOUSE_CLICK_FLAG(button);
		else eventHandleMask &= ~UI_EVENT_MOUSE_CLICK_FLAG(button);
	}
	bool UIObject::IsMouseClickable(MouseButton button)
	{
		return eventHandleMask & UI_EVENT_MOUSE_CLICK_FLAG(button);
	}
	void UIObject::SetMouseWheelRotatable(bool value)
	{
		if (value) eventHandleMask |= UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
		else eventHandleMask &= ~UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
	}
	bool UIObject::isMouseWheelRotatable()
	{
		return eventHandleMask & UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
	}
	void UIObject::SetMouseHoverHooking(bool value)
	{
		if (value) eventHookMask |= UI_EVENT_MOUSE_HOVER_FLAG;
		else eventHookMask &= ~UI_EVENT_MOUSE_HOVER_FLAG;
	}
	bool UIObject::IsMouseHoverHookable()
	{
		return eventHookMask & UI_EVENT_MOUSE_HOVER_FLAG;
	}
	void UIObject::SetMouseClickHooking(MouseButton button, bool value)
	{
		if (value) eventHookMask |= UI_EVENT_MOUSE_CLICK_FLAG(button);
		else eventHookMask &= ~UI_EVENT_MOUSE_CLICK_FLAG(button);
	}
	bool UIObject::IsMouseClickHookable(MouseButton button)
	{
		return eventHookMask & UI_EVENT_MOUSE_CLICK_FLAG(button);
	}
	void UIObject::SetMouseWheelRotateHooking(bool value)
	{
		if (value) eventHookMask |= UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
		else eventHookMask &= ~UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
	}
	bool UIObject::IsMouseWheelRotateHookable()
	{
		return eventHookMask & UI_EVENT_MOUSE_WHEEL_ROTATE_FLAG;
	}
	bool UIObject::ContainsPoint(Vector2f point)
	{
		return RectContainsPoint(Rectf(x, y, x + width, y + height), point);
	}
}
