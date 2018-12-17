// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once

#include "atc\TypeBase.h"
#include "kernel\ErrorCodes.h"

namespace kernel {}
namespace atc {}
namespace math {}
namespace algo {}
namespace util {}
namespace gpu {}
namespace graphics {}
namespace ui {}

namespace kernel
{
	using namespace atc;
	using namespace math;
	using namespace algo;
	using namespace util;
	using namespace gpu;
	using namespace graphics;
	using namespace ui;

	typedef class SharedObject SharedObject;

	HResult LibraryInitialize();
	void EnterSharedSection();
	void LeaveSharedSection();
}

namespace atc
{
	using namespace kernel;
	using namespace math;
	using namespace algo;
	using namespace util;
	using namespace gpu;
	using namespace graphics;
	using namespace ui;
}

namespace math
{
	using namespace kernel;
	using namespace atc;
	using namespace algo;
	using namespace util;
	using namespace gpu;
	using namespace graphics;
	using namespace ui;
}

namespace algo
{
	using namespace kernel;
	using namespace atc;
	using namespace math;
	using namespace util;
	using namespace gpu;
	using namespace graphics;
	using namespace ui;

	typedef class DistanceGeometry DistanceGeometry;
}

namespace util
{
	using namespace kernel;
	using namespace atc;
	using namespace math;
	using namespace algo;
	using namespace gpu;
	using namespace graphics;
	using namespace ui;

	typedef class Time Time;
	typedef class AsyncTimer AsyncTimer;
	typedef class CallbackTimer CallbackTimer;
}

namespace gpu
{
	using namespace kernel;
	using namespace atc;
	using namespace math;
	using namespace algo;
	using namespace util;
	using namespace graphics;
	using namespace ui;

	typedef class GpuDevice GpuDevice;
	typedef class GpuMemoryManager GpuMemoryManager;
	typedef class CommandPool CommandPool;
	typedef class CommandBuffer CommandBuffer;
	typedef class DescriptorPool DescriptorPool;
	typedef class DescriptorSet DescriptorSet;
	typedef class Shader Shader;
	typedef class Pipeline Pipeline;
	typedef class Surface Surface;
	typedef class SwapChain SwapChain;
	typedef class Buffer Buffer;
	typedef class RenderTarget RenderTarget;
	typedef class GradientCollection GradientCollection;
	typedef class Bitmap Bitmap;
}

namespace graphics
{
	using namespace kernel;
	using namespace atc;
	using namespace math;
	using namespace algo;
	using namespace util;
	using namespace gpu;
	using namespace ui;

	typedef class Color Color;
	typedef class GeometryPath GeometryPath;
	typedef class Geometry Geometry;
	typedef struct FontMetadata FontMetadata;
	typedef struct CharMetadata CharMetadata;
	typedef class FontManager FontManager;
}

namespace ui
{
	using namespace kernel;
	using namespace atc;
	using namespace math;
	using namespace algo;
	using namespace util;
	using namespace gpu;
	using namespace graphics;

	typedef class UIManager UIManager;
	typedef class Window Window;
	typedef class UIFactory UIFactory;
	typedef class UIObject UIObject;
	typedef class ScrollBar ScrollBar;
	typedef class FlowLayout FlowLayout;
	typedef class TextField TextField;
	typedef class PushButton PushButton;
	typedef class LayoutButton LayoutButton;
	typedef class CheckBox CheckBox;
	typedef class RadioButtonGroup RadioButtonGroup;
	typedef class RadioButton RadioButton;
}

