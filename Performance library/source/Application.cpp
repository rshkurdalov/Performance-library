#include "Application.h"
#include "Scene.h"

Application::Application()
{

}
Application::~Application()
{

}
HResult Application::Initialize()
{
	HResult hr;

	EngineInitialize();

	LoadFont(L"C:\\Windows\\Fonts\\cambria.ttc", &font);

	OSCreateWindow(L"Test window", 50, 50, 800, 600, &window);
	VkQueryGpuDevice(&device);
	device->CreateCommandPool(&cmdPool);
	cmdPool->CreateCommandBuffer(&cmdBuffer);
	OSCreateSurface(window, &surface);
	device->CreateSwapChain(window->GetWidth(), window->GetHeight(), surface, &swapChain);

	device->CreateRenderTarget(
		swapChain,
		cmdBuffer,
		&renderTarget);

	/*IWICImagingFactory *IWICFactory;
	CoInitialize(nullptr);
	CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&IWICFactory));

	IWICBitmapDecoder *pDecoder = nullptr;

	wchar pszFileName[MAX_PATH] = L"\0";
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND)window->GetHwnd();
	ofn.lpstrFilter = L"All Image Files\0"              L"*.bmp;*.dib;*.wdp;*.mdp;*.hdp;*.gif;*.png;*.jpg;*.jpeg;*.tif;*.ico\0"
		L"Windows Bitmap\0"               L"*.bmp;*.dib\0"
		L"High Definition Photo\0"        L"*.wdp;*.mdp;*.hdp\0"
		L"Graphics Interchange Format\0"  L"*.gif\0"
		L"Portable Network Graphics\0"    L"*.png\0"
		L"JPEG File Interchange Format\0" L"*.jpg;*.jpeg\0"
		L"Tiff File\0"                    L"*.tif\0"
		L"Icon\0"                         L"*.ico\0"
		L"All Files\0"                    L"*.*\0"
		L"\0";
	ofn.lpstrFile = pszFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open Image";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	GetOpenFileName(&ofn);

	HRESULT hr1 = IWICFactory->CreateDecoderFromFilename(
		pszFileName,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&pDecoder);

	IWICBitmapFrameDecode *pFrame = nullptr;
	pDecoder->GetFrame(0, &pFrame);
	IWICFormatConverter *convertedSource;
	hr1 = IWICFactory->CreateFormatConverter(&convertedSource);
	hr1 = convertedSource->Initialize(
		pFrame,
		GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom);
	uint32 width, height, stride, bufferSize;
	hr1 = convertedSource->GetSize(&width, &height);
	stride = width * sizeof(Color);
	bufferSize = stride*height;

	void *mapped;
	device->CreateBitmap(width, height, &image);
	image->MapMemory(&mapped);
	hr1 = convertedSource->CopyPixels(
		nullptr,
		stride,
		bufferSize,
		(BYTE *)mapped);
	image->UnmapMempory();*/

	scene = new Scene(this);

	return HResultSuccess;
}
HResult Application::Run()
{
	OSEnterMsgLoop(window, scene);

	return HResultSuccess;
}
HResult Application::Shutdown()
{
	return HResultSuccess;
}
