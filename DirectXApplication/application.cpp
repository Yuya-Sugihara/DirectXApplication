#include "application.h"

Application::Application(uint32_t width, uint32_t height): 
	mHInstance(nullptr),
	mWindow(nullptr),
	mWidth(width),
	mHeight(height)
{}

Application::~Application()
{}

void Application::run()
{
	if (initApp())
		mainLoop();

	termApp();
}

bool Application::initApp()
{
	if (!initWindow())
		return false;

	return true;
}

void Application::termApp()
{
	termWindow();
}

bool Application::initWindow()
{
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
		return false;

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.hIcon = LoadIcon(hInst,IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = TEXT("DirectXApplication");
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
		return false;

	mHInstance = hInst;

	RECT rc = {};
	rc.right = static_cast<LONG>(mWidth);
	rc.bottom = static_cast<LONG>(mHeight);

	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	mWindow = CreateWindowEx(
		0,
		TEXT("DirectXApplication"),
		TEXT("Sample"),
		style, 
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		mHInstance,
		nullptr);

	if (mWindow == nullptr)
		return false;

	ShowWindow(mWindow, SW_SHOWNORMAL);

	UpdateWindow(mWindow);

	SetFocus(mWindow);

	return true;
}

void Application::termWindow()
{
	if (mHInstance != nullptr)
		UnregisterClass(TEXT("DirectXApplication"), mHInstance);

	mHInstance = nullptr;
	mWindow = nullptr;
}

void Application::mainLoop()
{
	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CALLBACK Application::wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}





