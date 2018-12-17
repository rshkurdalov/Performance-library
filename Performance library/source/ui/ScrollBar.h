// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "UIObject.h"

namespace ui
{
    class ScrollBar : public UIObject
    {
    protected:
        bool isVertical;
        float32 viewportSize;
        float32 contentSize;
        float32 contentOffset;
		float32 sliderSize;

		void PrepareImpl();
        void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		ScrollBar(bool isVertical);
		void SetViewportSize(float32 value);
		void SetContentSize(float32 value);
		void SetOffset(float32 value);
		float32 GetOffset();
    };
}