#pragma once
#include "kernel\kernel.h"
#include "math\VectorMath.h"
#include "graphics\Color.h"
#include "gpu\GradientCollection.h"
#include "ui\Window.h"
#include "gpu\GpuDevice.h"
#include "gpu\CommandPool.h"
#include "gpu\CommandBuffer.h"
#include "gpu\DescriptorPool.h"
#include "gpu\DescriptorSet.h"
#include "gpu\SwapChain.h"
#include "gpu\Pipeline.h"
#include "gpu\Buffer.h"
#include "gpu\RenderTarget.h"
#include "gpu\Bitmap.h"
#include "graphics\Font.h"
#include "gpu\VulkanAPI.h"
#include "kernel\OperatingSystemAPI.h"

using namespace ui;

class Application
{
	friend class Scene;
public:
	Window *window;
	Surface *surface;
	GpuDevice *device;
	CommandPool *cmdPool;
	CommandBuffer *cmdBuffer;
	SwapChain *swapChain;
	RenderTarget *renderTarget;
	Scene *scene;
	Bitmap *image;
	Font *font;

	Application();
	~Application();
	HResult Initialize();
	HResult Run();
	HResult Shutdown();
};
