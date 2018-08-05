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
	typedef class SharedObject SharedObject;

	HResult EngineInitialize();
}

namespace atc
{
	using namespace kernel;
}

namespace math
{
	using namespace atc;
}

namespace algo
{
	using namespace math;

	typedef class DistanceGeometry DistanceGeometry;
}

namespace util
{
	using namespace kernel;
	using namespace atc;

	typedef class Time Time;
	typedef class AsyncTimer AsyncTimer;
	typedef class CallbackTimer CallbackTimer;
}

namespace gpu
{
	using namespace kernel;
	using namespace graphics;

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
	using namespace algo;
	using namespace gpu;
	using namespace util;

	typedef class Color Color;
	typedef class GeometryPath GeometryPath;
	typedef class Geometry Geometry;
	typedef class Font Font;
}

namespace ui
{
	using namespace kernel;
	using namespace gpu;
	using namespace util;

	typedef class Window Window;
	typedef class UIFactory UIFactory;
	typedef class UIObject UIObject;
}

