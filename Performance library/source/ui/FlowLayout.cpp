#include "ui\FlowLayout.h"
#include "ui\UIManager.h"
#include "ui\UIFactory.h"
#include "ui\Window.h"
#include "ui\ScrollBar.h"

namespace ui
{
	FlowLayout::FlowLayout()
	{
		widthDesc = autosize();
		heightDesc = autosize();
		padding = Rect<UISize>(4.0f, 4.0f, 4.0f, 4.0f);
		axis = FlowAxisX;
		breakLine = true;
		UIFactory *factory;
		UIManager::GetFactory(&factory);
		factory->CreateScrollBar(true, &vScroll);
		vScroll->SetParent(this);
		factory->CreateScrollBar(false, &hScroll);
		hScroll->SetParent(this);
		factory->Unref();
	}
	FlowLayout::~FlowLayout()
	{
		Clear();
		vScroll->Unref();
		hScroll->Unref();
	}
	void FlowLayout::OrganizeLine(
		uint32 idxBegin,
		uint32 idxEnd,
		float32 size1,
		float32 size2,
		float32 size3,
		float32 offset,
		float32 linespace,
		float32 viewportWidth,
		float32 viewportHeight)
	{
		float32 offset1 = 0.0f, offset2, offset3;
		Vector2f objSize, objPosition;
		if (axis == FlowAxisX)
		{
			offset2 = 0.5f*(size1 + viewportWidth - size3 - size2);
			offset2 = Max(size1, offset2);
			offset3 = Max(offset2 + size2, viewportWidth - size3);
			for (uint32 idx = idxBegin; idx < idxEnd; idx++)
			{
				objSize = Vector2f(objects[idx]->GetWidth(), objects[idx]->GetHeight());
				if (objects[idx]->GetHorizontalAlign() == HorizontalAlignLeft)
				{
					objPosition.x = offset1;
					offset1 += objSize.x;
				}
				else if (objects[idx]->GetHorizontalAlign() == HorizontalAlignCenter)
				{
					objPosition.x = offset2;
					offset2 += objSize.x;
				}
				else
				{
					objPosition.x = offset3;
					offset3 += objSize.x;
				}
				if (objects[idx]->GetVerticalAlign() == VerticalAlignTop)
					objPosition.y = offset;
				else if (objects[idx]->GetVerticalAlign() == VerticalAlignCenter)
					objPosition.y = offset + 0.5f*(linespace - objSize.y);
				else objPosition.y = offset + linespace - objSize.y;
				objects[idx]->SetPosition(objPosition);
			}
		}
		else
		{
			offset2 = 0.5f*(size1 + viewportHeight - size3 - size2);
			offset2 = Max(size1, offset2);
			offset3 = Max(offset2 + size2, viewportHeight - size3);
			for (uint32 idx = idxBegin; idx < idxEnd; idx++)
			{
				objSize = Vector2f(objects[idx]->GetWidth(), objects[idx]->GetHeight());
				if (objects[idx]->GetHorizontalAlign() == HorizontalAlignLeft)
					objPosition.x = offset;
				else if (objects[idx]->GetHorizontalAlign() == HorizontalAlignCenter)
					objPosition.x = offset + 0.5f*(linespace - objSize.x);
				else objPosition.x = offset + linespace - objSize.x;
				if (objects[idx]->GetVerticalAlign() == VerticalAlignTop)
				{
					objPosition.y = offset1;
					offset1 += objSize.y;
				}
				else if (objects[idx]->GetVerticalAlign() == VerticalAlignCenter)
				{
					objPosition.y = offset2;
					offset2 += objSize.y;
				}
				else
				{
					objPosition.y = offset3;
					offset3 += objSize.y;
				}
				objects[idx]->SetPosition(objPosition);
			}
		}
	}
	void FlowLayout::OrganizeObjects(
		float32 viewportWidth,
		float32 viewportHeight,
		float32 *contentWidth,
		float32 *contentHeight)
	{
		*contentWidth = 0.0f;
		*contentHeight = 0.0f;
		float32 size1 = 0.0f, size2 = 0.0f, size3 = 0.0f, linespace = 0.0f;
		Vector2f objSize;
		uint32 lastIdx = 0;
		if (axis == FlowAxisX)
		{
			for (uint32 idx = 0; idx < objects.size(); idx++)
			{
				objSize = objects[idx]->EvaluateSize(
					Vector2f(viewportWidth, viewportHeight),
					nullptr,
					nullptr,
					false,
					false);
				objects[idx]->Prepare(objSize.x, objSize.y);
				if (breakLine
					&& idx != lastIdx
					&& (size1 + size2 + size3 + objSize.x > viewportWidth
						|| objects[idx - 1]->IsFlowBreakEnabled()))
				{
					OrganizeLine(
						lastIdx, idx,
						size1, size2, size3,
						*contentHeight, linespace,
						viewportWidth, viewportHeight);
					*contentWidth = Max(*contentWidth, size1 + size2 + size3);
					*contentHeight += linespace;
					size1 = 0.0f;
					size2 = 0.0f;
					size3 = 0.0f;
					linespace = 0.0f;
					lastIdx = idx;
				}
				linespace = Max(linespace, objSize.y);
				if (objects[idx]->GetHorizontalAlign() == HorizontalAlignLeft)
					size1 += objSize.x;
				else if (objects[idx]->GetHorizontalAlign() == HorizontalAlignCenter)
					size2 += objSize.x;
				else size3 += objSize.x;
			}
			OrganizeLine(
				lastIdx, objects.size(),
				size1, size2, size3,
				*contentHeight, linespace,
				viewportWidth, viewportHeight);
			*contentHeight += linespace;
			*contentWidth = Max(*contentWidth, size1 + size2 + size3);
		}
		else
		{
			for (uint32 idx = 0; idx < objects.size(); idx++)
			{
				objSize = objects[idx]->EvaluateSize(
					Vector2f(viewportWidth, viewportHeight),
					nullptr,
					nullptr,
					false,
					false);
				objects[idx]->Prepare(objSize.x, objSize.y);
				if (breakLine
					&& idx != lastIdx
					&& (size1 + size2 + size3 + objSize.y > viewportHeight
						|| objects[idx - 1]->IsFlowBreakEnabled()))
				{
					OrganizeLine(
						lastIdx, idx,
						size1, size2, size3,
						*contentWidth, linespace,
						viewportWidth, viewportHeight);
					*contentWidth += linespace;
					*contentHeight = Max(*contentHeight, size1 + size2 + size3);
					size1 = 0.0f;
					size2 = 0.0f;
					size3 = 0.0f;
					linespace = 0.0f;
					lastIdx = idx;
				}
				linespace = Max(linespace, objSize.x);
				if (objects[idx]->GetVerticalAlign() == VerticalAlignTop)
					size1 += objSize.y;
				else if (objects[idx]->GetVerticalAlign() == VerticalAlignCenter)
					size2 += objSize.y;
				else size3 += objSize.y;
			}
			OrganizeLine(
				lastIdx, objects.size(),
				size1, size2, size3,
				*contentWidth, linespace,
				viewportWidth, viewportHeight);
			*contentWidth += linespace;
			*contentHeight = Max(*contentHeight, size1 + size2 + size3);
		}
	}
	Vector2f FlowLayout::EvaluateContentSizeImpl(
		float32 *viewportWidth,
		float32 *viewportHeight)
	{
		Vector2f contentSize = 0.0f, defaultSize = 0.0f;
		float32 size = 0.0f, linespace = 0.0f;
		Vector2f objSize;
		uint32 lastIdx = 0;
		if (viewportWidth != nullptr) defaultSize.x = *viewportWidth;
		if (viewportHeight != nullptr) defaultSize.y = *viewportHeight;
		if (axis == FlowAxisX)
		{
			for (uint32 idx = 0; idx < objects.size(); idx++)
			{
				bool evaluateAutoHeight =
					!breakLine
					&& viewportHeight == nullptr
					&& objects[idx]->GetHeightDesc().sizeType == UISizeTypeRelative;
				objSize = objects[idx]->EvaluateSize(
					defaultSize,
					nullptr,
					nullptr,
					false,
					evaluateAutoHeight);
				if (breakLine
					&& idx != lastIdx
					&& (size + objSize.x > defaultSize.x
						|| objects[idx - 1]->IsFlowBreakEnabled()))
				{
					contentSize.x = Max(contentSize.x, size);
					contentSize.y += linespace;
					size = 0.0f;
					linespace = 0.0f;
					lastIdx = idx;
				}
				linespace = Max(linespace, objSize.y);
				size += objSize.x;
			}
			contentSize.y += linespace;
			contentSize.x = Max(contentSize.x, size);
		}
		else
		{
			for (uint32 idx = 0; idx < objects.size(); idx++)
			{
				bool evaluateAutoWidth =
					!breakLine
					&& viewportWidth == nullptr
					&& objects[idx]->GetWidthDesc().sizeType == UISizeTypeRelative;
				objSize = objects[idx]->EvaluateSize(
					defaultSize,
					nullptr,
					nullptr,
					evaluateAutoWidth,
					false);
				if (breakLine
					&& idx != lastIdx
					&& (size + objSize.y > defaultSize.y
						|| objects[idx - 1]->IsFlowBreakEnabled()))
				{
					contentSize.x += linespace;
					contentSize.y = Max(contentSize.y, size);
					size = 0.0f;
					linespace = 0.0f;
					lastIdx = idx;
				}
				linespace = Max(linespace, objSize.x);
				size += objSize.y;
			}
			contentSize.x += linespace;
			contentSize.y = Max(contentSize.y, size);
		}
		return contentSize;
	}
	void FlowLayout::PrepareImpl()
	{
		float32 viewportWidth = viewport.right - viewport.left,
			viewportHeight = viewport.bottom - viewport.top,
			contentWidth, contentHeight;
		Vector2f vScrollSize(vScroll->GetWidthDesc().value, effectiveHeight),
			hScrollSize(effectiveWidth, hScroll->GetHeightDesc().value);
		vScroll->SetVisible(false);
		hScroll->SetVisible(false);
		OrganizeObjects(
			viewportWidth,
			viewportHeight,
			&contentWidth,
			&contentHeight);
		bool reorganize = false;
		if (contentHeight >= viewport.bottom - viewport.top + UIEps)
		{
			viewportWidth -= vScrollSize.x;
			vScroll->SetVisible(true);
			reorganize = true;
		}
		if (contentWidth >= viewport.right - viewport.left + UIEps)
		{
			viewportHeight -= hScrollSize.y;
			hScroll->SetVisible(true);
			reorganize = true;
		}
		if (reorganize)
			OrganizeObjects(
				viewportWidth,
				viewportHeight,
				&contentWidth,
				&contentHeight);
		vScroll->SetViewportSize(viewportHeight);
		vScroll->SetContentSize(contentHeight);
		hScroll->SetViewportSize(viewportWidth);
		hScroll->SetContentSize(contentWidth);
		if (vScroll->IsVisible() && hScroll->IsVisible())
		{
			vScrollSize.y -= hScrollSize.y;
			hScrollSize.x -= vScrollSize.x;
		}
		vScroll->Prepare(vScrollSize.x, vScrollSize.y);
		hScroll->Prepare(hScrollSize.x, hScrollSize.y);
		offset = 0.0f;
	}
	void FlowLayout::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		rt->PushScissor(
			p.x + viewport.left,
			p.y + viewport.top,
			viewport.right - viewport.left,
			viewport.bottom - viewport.top);
		Vector2f newOffset = Vector2f(
			viewport.left - hScroll->GetOffset(),
			viewport.top - vScroll->GetOffset());
		for (uint32 i = 0; i < objects.size(); i++)
		{
			objects[i]->SetPosition(objects[i]->GetPosition() + newOffset - offset);
			objects[i]->Render(rt, p + objects[i]->GetEffectivePosition());
		}
		offset = newOffset;
		vScroll->SetPosition(Vector2f(effectiveWidth - vScroll->GetWidth(), 0.0f));
		hScroll->SetPosition(Vector2f(0.0f, effectiveHeight - hScroll->GetHeight()));
		rt->PopScissor();
		vScroll->Render(rt, p + vScroll->GetEffectivePosition());
		hScroll->Render(rt, p + hScroll->GetEffectivePosition());
	}
	void FlowLayout::SetFlowAxis(FlowAxis value)
	{
		axis = value;
		Update();
	}
	FlowAxis FlowLayout::GetFlowAxis()
	{
		return axis;
	}
	void FlowLayout::EnableMultiline(bool value)
	{
		breakLine = value;
		Update();
	}
	bool FlowLayout::IsMultiline()
	{
		return breakLine;
	}
	void FlowLayout::Insert(uint32 idx, UIObject *object)
	{
		object->AddRef();
		object->SetParent(this);
		objects.insert(objects.begin() + idx, object);
		Update();
	}
	void FlowLayout::Append(UIObject *object)
	{
		Insert(GetSize(), object);
	}
	uint32 FlowLayout::GetSize()
	{
		return objects.size();
	}
	void FlowLayout::Remove(UIObject *object)
	{
		uint32 idx = 0;
		while (idx < GetSize() && objects[idx] != object) idx++;
		if (idx != GetSize()) RemoveAt(idx);
		Update();
	}
	void FlowLayout::RemoveAt(uint32 idx)
	{
		objects[idx]->SetParent(nullptr);
		objects[idx]->Unref();
		objects.erase(objects.begin() + idx);
		Update();
	}
	void FlowLayout::Clear()
	{
		for (uint32 i = 0; i < objects.size(); i++)
		{
			objects[i]->SetParent(nullptr);
			objects[i]->Unref();
		}
		objects.clear();
		Update();
	}
	void FlowLayout::MouseWheelRotate(UIMouseWheelEvent *e)
	{
		if (vScroll->IsVisible())
			vScroll->SetOffset(vScroll->GetOffset() - copysign(50.0f, e->delta));
		else if (hScroll->IsVisible())
			hScroll->SetOffset(hScroll->GetOffset() - copysign(50.0f, e->delta));
		Repaint();
	}
	void FlowLayout::ForEach(Function<void(UIObject *, void *)> callback, void *param)
	{
		if (GetSize() != 0)
		{
			UIObject **objectsCopy = new UIObject *[GetSize()];
			memcpy(objectsCopy, objects.data(), GetSize() * sizeof(UIObject *));
			for (uint32 i = 0; i < GetSize(); i++)
				objectsCopy[i]->AddRef();
			for (uint32 i = 0; i < GetSize(); i++)
				callback(objectsCopy[i], param);
			for (uint32 i = 0; i < GetSize(); i++)
				objectsCopy[i]->Unref();
			delete[] objectsCopy;
		}
		callback(vScroll, param);
		callback(hScroll, param);
	}
}
