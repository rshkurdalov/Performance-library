#pragma once
#include "kernel\kernel.h"
#include "atc\Function.h"
#include "math\VectorMath.h"
#include "ui\UIObject.h"
#include <vector>

namespace ui
{
	class FlowLayout : public UIObject
	{
	protected:
		std::vector<UIObject *> objects;
		FlowAxis axis;
		bool breakLine;
		ScrollBar *vScroll;
		ScrollBar *hScroll;
		Vector2f offset;

		void OrganizeLine(
			uint32 idxBegin,
			uint32 idxEnd,
			float32 size1,
			float32 size2,
			float32 size3,
			float32 offset,
			float32 linespace,
			float32 viewportWidth,
			float32 viewportHeight);
		void OrganizeObjects(
			float32 viewportWidth,
			float32 viewportHeight,
			float32 *contentWidth,
			float32 *contentHeight);
		Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
		void PrepareImpl();
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		FlowLayout();
		~FlowLayout();
		void SetFlowAxis(FlowAxis value);
		FlowAxis GetFlowAxis();
		void EnableMultiline(bool value);
		bool IsMultiline();
		void Insert(uint32 idx, UIObject *object);
		void Append(UIObject *object);
		uint32 GetSize();
		void Remove(UIObject *object);
		void RemoveAt(uint32 idx);
		void Clear();
		void MouseWheelRotate(UIMouseWheelEvent *e);
		void ForEach(Function<void(UIObject *, void *)> callback, void *param);
	};
}
