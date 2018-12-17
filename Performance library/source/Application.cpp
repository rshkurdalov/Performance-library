#include "Application.h"
#include <wincodec.h>

Application::Application()
{

}
Application::~Application()
{

}
HResult Application::Initialize()
{
	LibraryInitialize();

	UIFactory *factory;
	UIManager::GetFactory(&factory);

	std::u32string str;

	FlowLayout *fl;
	factory->CreateFlowLayout(&fl);
	fl->SetBackgroundColor(Color::White);
	//fl->SetFlowAxis(FlowAxisY);
	//fl->EnableMultiline(FlowLineBreakNoBreak);
	OSCreateWindow(L"Test window", 50, 50, 800, 600, fl, &window);

	Window *dialog;
	FlowLayout *dl;
	factory->CreateFlowLayout(&dl);
	dl->SetBackgroundColor(Color::LightBlue);
	CheckBox *cb;
	factory->CreateCheckBox(&cb);
	str = U"Sample textew line";
	cb->SetText(str.data(), str.size());
	dl->Append(cb);

	OSCreateWindow(L"Dialog", 0, 0, 400, 300, dl, &dialog);

	TextField *textField;
	factory->CreateEditField(&textField);
	textField->EnableMultiline(true);
	textField->SetDefaultFont(std::wstring(L"Segoe UI"));
	textField->SetDefaultFontSize(10.0f);
	textField->SetTextAlign(HorizontalAlignCenter);
	textField->SetVerticalAlign(VerticalAlignBottom);
	//textField->SetWidthDesc(200);
	//textField->SetHeightDesc(200);
	fl->Append(textField);

	TextField *textField1;
	factory->CreateTextField(&textField1);
	str = U"LOL";
	textField1->SetText(str.data(), str.size());
	textField1->SetTextAlign(HorizontalAlignCenter);
	textField1->SetVerticalAlign(VerticalAlignBottom);
	fl->Append(textField1);

	TextField *textField2;
	factory->CreateEditField(&textField2);
	textField2->EnableMultiline(false);
	textField2->SetVerticalAlign(VerticalAlignCenter);
	fl->Append(textField2);

	TextField *textField3;
	factory->CreateTextField(&textField3);
	str = U"test str ext";
	textField3->SetText(str.data(), str.size());
	textField3->SetVerticalAlign(VerticalAlignTop);

	LayoutButton *pb;
	factory->CreateLayoutButton(&pb);
	pb->Append(textField3);
	static struct _PBCallbackParams
	{
		TextField *tf;
		Window *window;
		Window *dialog;
	} PBCallbackParams{ textField1, window, dialog };
	static void(*callback)(UIMouseEvent *, _PBCallbackParams *)
		= [](UIMouseEvent *e, _PBCallbackParams *param) -> void
	{
		std::u32string str = U"Test text";
		if (param->tf->GetTextLength() == 0)
			param->tf->SetText(str.data(), str.size());
		else param->tf->Clear();
		//param->dialog->OpenModal(param->window);
	};
	pb->onButtonClick.AddCallback(callback, &PBCallbackParams);
	fl->Append(pb);

	FlowLayout *rbLayout;
	factory->CreateFlowLayout(&rbLayout);
	rbLayout->SetFlowAxis(FlowAxisY);
	rbLayout->EnableMultiline(false);
	RadioButtonGroup *rbGroup = new RadioButtonGroup();
	RadioButton *rb1, *rb2, *rb3;
	factory->CreateRadioButton(rbGroup, &rb1);
	rb1->SetWidthDesc(1.0em);
	str = U"rb1\nnew line";
	rb1->SetText(str.data(), str.size());
	rbLayout->Append(rb1);
	factory->CreateRadioButton(rbGroup, &rb2);
	rb2->SetWidthDesc(1.0em);
	str = U"r2";
	rb2->SetText(str.data(), str.size());
	rbLayout->Append(rb2);
	factory->CreateRadioButton(rbGroup, &rb3);
	rb3->SetWidthDesc(1.0em);
	str = U"r3";
	rb3->SetText(str.data(), str.size());
	rbLayout->Append(rb3);
	fl->Append(rbLayout);

	PushButton *pb1;
	factory->CreatePushButton(&pb1);
	str = U"push button";
	pb1->SetText(str.data(), str.size());
	fl->Append(pb1);

	CheckBox *checkBox;
	factory->CreateCheckBox(&checkBox);
	//checkBox->SetDefaultFont(std::wstring(L"Segoe UI"));
	checkBox->SetDefaultFontSize(22.0f);
	str = U"Check box test";
	checkBox->SetText(str.data(), str.size());
	fl->Append(checkBox);

	factory->Unref();
	layout = fl;

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
	uint32 effectiveWidth, effectiveHeight, stride, bufferSize;
	hr1 = convertedSource->GetSize(&effectiveWidth, &effectiveHeight);
	stride = effectiveWidth * sizeof(Color);
	bufferSize = stride*effectiveHeight;

	void *mapped;
	device->CreateBitmap(effectiveWidth, effectiveHeight, &image);
	image->MapMemory(&mapped);
	hr1 = convertedSource->CopyPixels(
		nullptr,
		stride,
		bufferSize,
		(BYTE *)mapped);
	image->UnmapMempory();*/

	return HResultSuccess;
}
HResult Application::Run()
{
	window->Open();
	OSRunMsgLoop();

	return HResultSuccess;
}
HResult Application::Shutdown()
{
	return HResultSuccess;
}
