// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "kernel\OperatingSystemAPI.h"
#include "gpu\VulkanAPI.h"
#include "gpu\Surface.h"
#include "math\VectorMath.h"
#include "ui\UITypes.h"
#include "ui\UIEventArgs.h"
#include "ui\Window.h"
#include "ui\UIObject.h"

namespace kernel
{
#ifdef _WIN32
	LRESULT WINAPI WndProcStartupWin32(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	void WndProcWin32(ui::Window *window, uint32 msg, msize wParam, msize lParam)
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
		{
			math::Vector2f position(
				LOWORD(lParam),
				HIWORD(lParam));
			ui::UIMouseEvent e(
				nullptr,
				position.x,
				position.y,
				position.x - window->GetMousePosition().x,
				position.y - window->GetMousePosition().y);
			window->MouseMove(&e);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonLeft);
			window->MouseClick(&e);
			break;
		}
		case WM_LBUTTONUP:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonLeft);
			window->MouseRelease(&e);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonRight);
			window->MouseClick(&e);
			break;
		}
		case WM_RBUTTONUP:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonRight);
			window->MouseRelease(&e);
			break;
		}
		case WM_KEYDOWN:
		{
			ui::UIKeyboardEvent e(nullptr, wParam);
			window->KeyPress(&e);
			break;
		}
		case WM_KEYUP:
		{
			ui::UIKeyboardEvent e(nullptr, wParam);
			window->KeyRelease(&e);
			break;
		}
		case WM_CHAR:
		{
			ui::UIKeyboardEvent e(nullptr, wParam);
			window->CharInput(&e);
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonLeft,
				true);
			window->onMouseClick.Notify(&e);
			break;
		}
		case WM_RBUTTONDBLCLK:
		{
			ui::UIMouseEvent e(
				nullptr,
				LOWORD(lParam),
				HIWORD(lParam),
				0,
				0,
				ui::MouseButtonRight,
				true);
			window->MouseClick(&e);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			ui::UIMouseWheelEvent e(
				nullptr,
				window->GetMousePosition().x,
				window->GetMousePosition().y,
				GET_WHEEL_DELTA_WPARAM(wParam));
			window->MouseWheelRotate(&e);
			break;
		}
		case WM_SIZE:
		{
			ui::UIResizeEvent e(
				nullptr,
				window->GetWidth(),
				window->GetHeight(),
				LOWORD(lParam),
				HIWORD(lParam));
			window->OnResize(&e);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			/*targetWnd->Update();
			break;*/
		}
		case WM_CLOSE:
		{
			/*window->Close();
			break;*/
		}
		default:
			DefWindowProc((HWND)window->GetHwnd(), msg, wParam, lParam);
		}
	}
#endif

	HResult OSCreateWindow(
		const wchar *name,
		int32 x,
		int32 y,
		uint32 width,
		uint32 height,
		ui::Window **window)
	{
#ifdef _WIN32
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = WndProcStartupWin32;
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
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
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

		*window = new ui::Window(width, height, (msize)hwnd);

		return HResultSuccess;
#endif
	}
	uint32 fps = 0, s = 60;
	HResult OSEnterMsgLoop(
		ui::Window *window,
		ui::UIObject *layout)
	{
#ifdef _WIN32
		layout->AddRef();
		layout->SetParent(nullptr);
		layout->SetX(0.0f);
		layout->SetY(0.0f);
		window->PushForegroundObject(layout);
		ShowWindow((HWND)window->GetHwnd(), SW_SHOW);
		SetForegroundWindow((HWND)window->GetHwnd());
		SetFocus((HWND)window->GetHwnd());
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		window->Update();
		SYSTEMTIME t;
		
		std::vector<uint32> fpsOverTime;
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) break;
				TranslateMessage(&msg);
				WndProcWin32(window, msg.message, msg.wParam, msg.lParam);
			}
			window->Update();
			fps++;
			GetSystemTime(&t);
			if (t.wSecond != s)
			{
				fpsOverTime.push_back(fps);
				fps = 0;
				s = t.wSecond;
			}
		}
		window->RemoveForegroundObject(layout);
		layout->Release();
		return HResultSuccess;
#endif
	}

	HResult OSReleaseWindowHandler(ui::Window *window)
	{
#ifdef _WIN32
		if (DestroyWindow((HWND)window->GetHwnd()) == FALSE)
			return HResultFail;
		return HResultSuccess;
#endif
	}
	HResult OSCreateSurface(
		ui::Window *window,
		gpu::Surface **ppSurface)
	{
#ifdef _WIN32
		VkInstance vkInstance;
		CheckReturnFail(gpu::VkQueryInstance(&vkInstance));
		VkSurfaceKHR vkSurface;
		VkWin32SurfaceCreateInfoKHR vkSurfaceCreateInfo;
		vkSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		vkSurfaceCreateInfo.pNext = nullptr;
		vkSurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		vkSurfaceCreateInfo.hwnd = (HWND)window->GetHwnd();
		vkSurfaceCreateInfo.flags = 0;
		CheckReturnFail(vkCreateWin32SurfaceKHR(
			vkInstance,
			&vkSurfaceCreateInfo,
			nullptr,
			&vkSurface));

		*ppSurface = new gpu::Surface(vkSurface);

		return HResultSuccess;
#endif
	}
}
