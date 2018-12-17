// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\OperatingSystemAPI.h"
#include "gpu\GpuDevice.h"
#include "gpu\Surface.h"
#include "math\VectorMath.h"
#include "graphics\GeometryPath.h"
#include "graphics\Geometry.h"
#include "graphics\Font.h"
#include "ui\UIManager.h"
#include "ui\UITypes.h"
#include "ui\UIEventArgs.h"
#include "ui\Window.h"
#include "ui\UIObject.h"
#include "util\Time.h"
#include <vector>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace kernel
{
#ifdef _WIN32
	std::map<HWND, Window *> hwndMap;
	LRESULT WINAPI WndProcWin32(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static int64 time = 0;
		static int64 lastTimeCliked = 0;
		static MouseButton lastButton = MouseButtonNone;
		static uint32 lastClickX = UINT32_MAX;
		static uint32 lastClickY = UINT32_MAX;
		Window *window = hwndMap[hwnd];
		switch (msg)
		{
		case WM_MOUSEMOVE:
		{
			Vector2f position(
				LOWORD(lParam),
				HIWORD(lParam));
			UIMouseEvent e(
				position.x,
				position.y,
				position.x - window->GetMousePosition().x,
				position.y - window->GetMousePosition().y);
			window->MouseMove(&e);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			time = Time::Now();
			UIMouseEvent e(
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				MouseButtonLeft,
				lastClickX == LOWORD(lParam) && lastClickY == HIWORD(lParam)
				&& lastButton == MouseButtonLeft
				&& time < lastTimeCliked + UIManager::GetDoubleClickThreshold());
			if (e.doubleClick) lastButton = MouseButtonNone;
			else lastButton = MouseButtonLeft;
			lastTimeCliked = time;
			lastClickX = LOWORD(lParam);
			lastClickY = HIWORD(lParam);
			window->MouseClick(&e);
			break;
		}
		case WM_LBUTTONUP:
		{
			UIMouseEvent e(
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				MouseButtonLeft);
			window->MouseRelease(&e);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			time = Time::Now();
			UIMouseEvent e(
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				MouseButtonRight,
				lastClickX == LOWORD(lParam) && lastClickY == HIWORD(lParam)
				&& lastButton == MouseButtonRight
				&& time < lastTimeCliked + UIManager::GetDoubleClickThreshold());
			if (e.doubleClick) lastButton = MouseButtonNone;
			else lastButton = MouseButtonRight;
			lastTimeCliked = time;
			lastClickX = LOWORD(lParam);
			lastClickY = HIWORD(lParam);
			window->MouseClick(&e);
			break;
		}
		case WM_RBUTTONUP:
		{
			UIMouseEvent e(
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				MouseButtonRight);
			window->MouseRelease(&e);
			break;
		}
		case WM_KEYDOWN:
		{
			UIKeyboardEvent e(
				(KeyCode)wParam,
				(GetAsyncKeyState(VK_SHIFT) < 0 ? true : false),
				(GetAsyncKeyState(VK_CONTROL) < 0 ? true : false),
				(GetAsyncKeyState(VK_MENU) < 0 ? true : false));
			window->KeyPress(&e);
			break;
		}
		case WM_KEYUP:
		{
			UIKeyboardEvent e(
				(KeyCode)wParam,
				(GetAsyncKeyState(VK_SHIFT) < 0 ? true : false),
				(GetAsyncKeyState(VK_CONTROL) < 0 ? true : false),
				(GetAsyncKeyState(VK_MENU) < 0 ? true : false));
			window->KeyRelease(&e);
			break;
		}
		case WM_CHAR:
		{ 
			UIKeyboardEvent e(
				(KeyCode)0,
				(GetAsyncKeyState(VK_SHIFT) < 0 ? true : false),
				(GetAsyncKeyState(VK_CONTROL) < 0 ? true : false),
				(GetAsyncKeyState(VK_MENU) < 0 ? true : false),
				(char32)wParam);
			window->CharInput(&e);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			UIMouseWheelEvent e(
				window->GetMousePosition().x,
				window->GetMousePosition().y,
				GET_WHEEL_DELTA_WPARAM(wParam));
			window->MouseWheelRotate(&e);
			break;
		}
		case WM_SIZE:
		{
			UIResizeEvent e(
				window->GetWidth(),
				window->GetHeight(),
				LOWORD(lParam),
				HIWORD(lParam));
			window->OnResize(&e);
			break;
		}
		case WM_CLOSE:
		{
			UICloseEvent e;
			window->OnClose(&e);
			if (e.confirmClose)
			{
				window->Close();
				PostMessage(hwnd, WM_APP, 0, 0);
			}
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		window->Update();
		return 0;
	}
#endif

	HResult OSCreateSurface(
		uint64 hwnd,
		Surface **ppSurface)
	{
#ifdef _WIN32
		GpuDevice *device;
		QueryGpuDevice(&device);
		VkInstance vkInstance = device->GetVkInstance();
		device->Unref();
		VkSurfaceKHR vkSurface;
		VkWin32SurfaceCreateInfoKHR vkSurfaceCreateInfo;
		vkSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		vkSurfaceCreateInfo.pNext = nullptr;
		vkSurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		vkSurfaceCreateInfo.hwnd = (HWND)hwnd;
		vkSurfaceCreateInfo.flags = 0;
		CheckReturnFail(vkCreateWin32SurfaceKHR(
			vkInstance,
			&vkSurfaceCreateInfo,
			nullptr,
			&vkSurface));

		*ppSurface = new Surface(vkSurface);

		return HResultSuccess;
#endif
	}

	HResult OSCreateWindow(
		wchar *name,
		int32 x,
		int32 y,
		uint32 width,
		uint32 height,
		UIObject *layout,
		Window **window)
	{
#ifdef _WIN32
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = WndProcWin32;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = LoadIcon(GetModuleHandle(nullptr), IDI_WINLOGO);
		wc.hIconSm = LoadIcon(GetModuleHandle(nullptr), IDI_WINLOGO);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = L"Performance library";
		ATOM atom = RegisterClassEx(&wc);
		RECT windowRect;
		windowRect.left = 0;
		windowRect.right = width;
		windowRect.top = 0;
		windowRect.bottom = height;
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
		HWND hwnd = CreateWindowEx(
			0,
			L"Performance library",
			name,
			WS_OVERLAPPEDWINDOW,
			x,
			y,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,
			nullptr,
			GetModuleHandle(nullptr),
			nullptr);
		if (hwnd == nullptr) return HResultFail;

		Surface *surface;
		CheckReturn(OSCreateSurface((uint64)hwnd, &surface));

		GpuDevice *device;
		QueryGpuDevice(&device);
		RenderTarget *rt;
		HResult hr = device->CreateRenderTarget(width, height, surface, &rt);
		device->Unref();
		if (hr != HResultSuccess) return hr;

		*window = new Window((uint64)hwnd, width, height, rt, layout);
		hwndMap[hwnd] = *window;

		return HResultSuccess;
#endif
	}

	void OSOpenWindow(Window *window)
	{
#ifdef _WIN32
		ShowWindow((HWND)window->GetHwnd(), SW_SHOW);
		SetForegroundWindow((HWND)window->GetHwnd());
		SetFocus((HWND)window->GetHwnd());
#endif
	}

	void OSEnableWindow(Window *window, bool value)
	{
#ifdef _WIN32

#endif
		EnableWindow((HWND)window->GetHwnd(), (value ? TRUE : FALSE));
	}

	void OSRunMsgLoop()
	{
#ifdef _WIN32
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			EnterSharedSection();
			WndProcWin32(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			LeaveSharedSection();
		}
#endif
	}

	void OSRunModalMsgLoop(Window *window)
	{
#ifdef _WIN32
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			if (msg.message == WM_APP
				&& msg.hwnd == (HWND)window->GetHwnd()) break;
			TranslateMessage(&msg);
			EnterSharedSection();
			WndProcWin32(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			LeaveSharedSection();
		}
#endif
	}

	void OSResizeWindow(
		Window *window,
		uint32 width,
		uint32 height)
	{
#ifdef _WIN32
		SetWindowPos(
			(HWND)window->GetHwnd(),
			nullptr,
			0,
			0,
			width,
			height,
			SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
#endif
	}

	void OSCloseWindow(Window *window)
	{
#ifdef _WIN32
		ShowWindow((HWND)window->GetHwnd(), SW_HIDE);
#endif
	}

	void OSReleaseWindowHandler(Window *window)
	{
#ifdef _WIN32
		DestroyWindow((HWND)window->GetHwnd());
		hwndMap.erase((HWND)window->GetHwnd());
#endif
	}

	void OSCopyTextToClipboard(std::u32string &text)
	{
#ifdef _WIN32
		if (OpenClipboard(nullptr) == 0) return;
		HGLOBAL hglbCopy = GlobalAlloc(
			GMEM_MOVEABLE,
			(text.size() + 1) * sizeof(wchar));
		wchar *pStrCopy = (wchar *)GlobalLock(hglbCopy);
		std::wstring wtext(text.begin(), text.end());
		memcpy(pStrCopy, wtext.c_str(), (text.size() + 1) * sizeof(wchar));
		GlobalUnlock(hglbCopy);
		SetClipboardData(CF_UNICODETEXT, hglbCopy);
		CloseClipboard();
#endif
	}

	void OSGetTextFromClipboard(std::u32string *text)
	{
#ifdef _WIN32
		if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE
			|| OpenClipboard(nullptr) == 0) return;
		HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
		if (hglb != nullptr)
		{
			wchar *pBuffer = (wchar *)GlobalLock(hglb);
			if (pBuffer != nullptr)
			{
				std::wstring wtext(pBuffer);
				text->assign(wtext.begin(), wtext.end());
			}
			GlobalUnlock(hglb);
		}
		CloseClipboard();
#endif
	}

	void OSSetCursor(uint32 cursor, Window *window)
	{
#ifdef _WIN32
		wchar *cursorId;
		if (cursor == CursorDefault)
			cursorId = IDC_ARROW;
		else if (cursor == CursorBeam)
			cursorId = IDC_IBEAM;
		else return;
		SetClassLong(
			(HWND)window->GetHwnd(),
			GCL_HCURSOR,
			(LONG)LoadCursor(nullptr, cursorId));
#endif
	}

	uint32 OSGetDPI()
	{
#ifdef _WIN32
		HDC hdc = CreateCompatibleDC(nullptr);
		uint32 dpi = (uint32)GetDeviceCaps(hdc, LOGPIXELSY);
		DeleteDC(hdc);
		return dpi;
#endif
	}

	HResult OSLoadFont(
		wchar *fontName,
		uint32 size,
		bool isItalic,
		uint32 weight,
		FontMetadata *font)
	{
#ifdef _WIN32
		HFONT hFont = CreateFont(
			-(int32)size,
			0,
			0,
			0,
			weight,
			(isItalic ? TRUE : FALSE),
			FALSE,
			FALSE,
			DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			DEFAULT_PITCH,
			fontName);
		if (hFont == nullptr) return HResultFail;
		HDC hdc = CreateCompatibleDC(nullptr);
		SelectObject(hdc, hFont);
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		LPOUTLINETEXTMETRICW otm;
		uint32 bufferSize = GetOutlineTextMetrics(hdc, 0, nullptr);
		otm = (LPOUTLINETEXTMETRIC)new uint8[bufferSize];
		GetOutlineTextMetrics(hdc, bufferSize, otm);
		font->fontHandler = (uint64)hFont;
		font->fontName = fontName;
		font->size = size;
		font->ascent = (float32)tm.tmAscent;
		font->internalLeading = (float32)tm.tmInternalLeading;
		font->internalLeadingMultiplier = (float32)size / ((float32)size - font->internalLeading);
		font->underlineOffset = -(float32)otm->otmsUnderscorePosition;
		font->underlineSize = (float32)otm->otmsUnderscoreSize;
		font->strikethroughOffset = -(float32)otm->otmsStrikeoutPosition;
		font->strikethroughSize = (float32)otm->otmsStrikeoutSize;
		delete[] otm;
		DeleteDC(hdc);
		return HResultSuccess;
#endif
	}

	HResult OSLoadGlyphMetadata(
		char32 code,
		FontMetadata *font,
		CharMetadata *charMetadata)
	{
#ifdef _WIN32
		if (code == L'\t')
		{
			charMetadata->advance = Vector2f(2.0f*(float32)font->size, 0.0f);
			return HResultSuccess;
		}
		if (code == L'\n')
		{
			charMetadata->advance = Vector2f(0.0f, 0.0f);
			return HResultSuccess;
		}
		GLYPHMETRICS glyphMetrics;
		auto FixedToFloat32 = [](FIXED value) -> float32
		{
			constexpr float32 fractMultiplier = 1.0f / UINT16_MAX;
			return (float32)value.value + (float32)value.fract*fractMultiplier;
		};
		HFONT hFont = (HFONT)font->fontHandler;
		HDC hdc = CreateCompatibleDC(nullptr);
		SelectObject(hdc, hFont);
		MAT2 transform = { 0, 1, 0, 0, 0, 0, 0, 1 };
		DWORD size = GetGlyphOutline(
			hdc,
			code,
			GGO_NATIVE,
			&glyphMetrics,
			0,
			nullptr,
			&transform);
		std::vector<uint8> outline(size);
		if (GetGlyphOutline(
			hdc,
			code,
			GGO_NATIVE,
			&glyphMetrics,
			size,
			outline.data(),
			&transform) == GDI_ERROR)
		{
			DeleteDC(hdc);
			return HResultFail;
		}
		GeometryPath path;
		TTPOLYGONHEADER *data = (TTPOLYGONHEADER *)outline.data();
		TTPOLYCURVE *curve;
		uint8 *contourEnd;
		while ((uint8 *)data < outline.data() + outline.size())
		{
			contourEnd = (uint8 *)data + data->cb;
			path.Move(Vector2f(
				FixedToFloat32(data->pfxStart.x),
				-FixedToFloat32(data->pfxStart.y)));
			data++;
			curve = (TTPOLYCURVE *)data;
			while ((uint8 *)curve < contourEnd)
			{
				if (curve->wType == TT_PRIM_LINE)
				{
					for (uint32 iter = 0; iter < curve->cpfx; iter++)
						path.PushLine(Vector2f(
							FixedToFloat32(curve->apfx[iter].x),
							-FixedToFloat32(curve->apfx[iter].y)));
				}
				else
				{
					for (uint32 iter = 0; iter < curve->cpfx - 2; iter++)
					{
						path.PushQuadraticBezier(
							Vector2f(
								FixedToFloat32(curve->apfx[iter].x),
								-FixedToFloat32(curve->apfx[iter].y)),
							Vector2f(
								0.5f*(FixedToFloat32(curve->apfx[iter].x) + FixedToFloat32(curve->apfx[iter + 1].x)),
								-0.5f*(FixedToFloat32(curve->apfx[iter].y) + FixedToFloat32(curve->apfx[iter + 1].y))));
					}
					path.PushQuadraticBezier(
						Vector2f(
							FixedToFloat32(curve->apfx[curve->cpfx - 2].x),
							-FixedToFloat32(curve->apfx[curve->cpfx - 2].y)),
						Vector2f(
							FixedToFloat32(curve->apfx[curve->cpfx - 1].x),
							-FixedToFloat32(curve->apfx[curve->cpfx - 1].y)));
				}
				curve = (TTPOLYCURVE *)(&curve->apfx[curve->cpfx]);
			}
			data = (TTPOLYGONHEADER *)curve;
		}
		charMetadata->outline.SetFaceOrientation(false);
		charMetadata->outline.FillGeometry(path);
		charMetadata->advance.x = (float32)glyphMetrics.gmCellIncX;
		charMetadata->advance.y = (float32)glyphMetrics.gmCellIncY;
		DeleteDC(hdc);
		return HResultSuccess;
#endif
	}
}
