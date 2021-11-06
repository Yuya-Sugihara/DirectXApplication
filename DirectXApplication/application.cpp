#include "application.h"

template<typename T>
void safeRelease(T*& ptr)
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

Application::Application(uint32_t width, uint32_t height): 
	mHInstance(nullptr),
	mWindow(nullptr),
	mWidth(width),
	mHeight(height),
	mpDevice(nullptr),
	mpCommandQueue(nullptr),
	mpSwapChain(nullptr),
	mpCommandList(nullptr),
	mpHeapRTV(nullptr),
	mpFence(nullptr),
	mFrameIndex(0)
{}

Application::~Application()
{
	safeRelease(mpDevice);
	safeRelease(mpCommandQueue);
	safeRelease(mpSwapChain);
	safeRelease(mpCommandList);
	safeRelease(mpHeapRTV);
	safeRelease(mpFence);
}

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

bool Application::initD3D()
{
	//	デバイスの作成
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mpDevice));
	if (FAILED(hr))
		return false;

	//	コマンドキューの作成
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = mpDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mpCommandQueue));
		if (FAILED(hr))
			return false;
	}

	//	スワップチェインの作成
	{
		//	DXGIFactoryの作成
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
			return false;

		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = mWidth;
		desc.BufferDesc.Height = mHeight;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = mFrameCount;
		desc.OutputWindow = mWindow;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(mpCommandQueue, &desc, &pSwapChain);
		if (FAILED(hr))
		{
			safeRelease(pFactory);
			return false;
		}

		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&mpSwapChain));
		if (FAILED(hr))
		{
			safeRelease(pFactory);
			safeRelease(pSwapChain);
			return false;
		}

		mFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();

		safeRelease(pFactory);
		safeRelease(pSwapChain);
	}

	//	コマンドアロケータの作成
	{
		for (auto i = 0u; i < mFrameCount; i++)
		{
			hr = mpDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&mpCommandAllocator[i]));

			if (FAILED(hr))
				return false;
		}
	}

	//	コマンドリストの作成
	{
		hr = mpDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mpCommandAllocator[mFrameIndex],
			nullptr,
			IID_PPV_ARGS(&mpCommandList));

		if (FAILED(hr))
			return false;
	}

	//	レンダーターゲットビューの作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = mFrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		hr = mpDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mpHeapRTV));
		if (FAILED(hr))
			return false;

		auto handle = mpHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0u; i < mFrameCount; i++)
		{
			hr = mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mpColorBuffer[i]));
			if (FAILED(hr))
				return false;

			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			mpDevice->CreateRenderTargetView(mpColorBuffer[i], &viewDesc, handle);

			mHandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	//	フェンスの作成
	{
		for (auto i = 0u; i < mFrameCount; i++)
		{
			mFenceCounter[i] = 0;
		}

		hr = mpDevice->CreateFence(
			mFenceCounter[mFrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&mpFence));
		if (FAILED(hr))
			return false;

		mFenceCounter[mFrameIndex]++;

		mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (mFenceEvent == nullptr)
			return false;

		mpCommandList->Close();
		
		return true;
	}
}

void Application::termD3D()
{

}

void Application::render()
{

}

void Application::waitGpu()
{

}

void Application::present(uint32_t interval)
{

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





