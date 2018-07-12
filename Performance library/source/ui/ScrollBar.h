// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "UIObject.h"

#define SCROLL_BAR_DEFAULT_SIZE 8
#define SCROLL_BAR_DEFAULT_STEP 20

namespace ui
{
    class ScrollBar : public UIObject
    {
        friend class UIFactory;
        friend class UIObject;
    protected:
        bool isVertical;
        float32 viewportSize;
        float32 contentSize;
        float32 contentOffset;
        float32 scrollingStep;
        float32 sliderSize;
        Rectf sliderRect;

        ScrollBar(bool isVertical, Window *window);
        void RenderImpl();
    };
}