#pragma once
#include "kernel\kernel.h"
#include "math\VectorMath.h"
#include "graphics\Color.h"
#include "gpu\GradientCollection.h"
#include "ui\Window.h"
#include "ui\UIManager.h"
#include "ui\UIFactory.h"
#include "ui\FlowLayout.h"
#include "ui\TextField.h"
#include "ui\PushButton.h"
#include "ui\LayoutButton.h"
#include "ui\CheckBox.h"
#include "ui\RadioButton.h"
#include "gpu\GpuDevice.h"
#include "gpu\RenderTarget.h"
#include "gpu\Bitmap.h"
#include "graphics\Font.h"
#include "gpu\GpuManager.h"
#include "kernel\OperatingSystemAPI.h"

using namespace ui;

class Application
{
public:
	Window *window;
	UIObject *layout;
	Bitmap *image;

	Application();
	~Application();
	HResult Initialize();
	HResult Run();
	HResult Shutdown();
};
