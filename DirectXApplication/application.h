#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Application
{
public: 

	Application(uint32_t width, uint32_t height);
	~Application();

	void run();

private: 

	static const uint32_t mFrameCount = 2;

	HINSTANCE mHInstance;
	HWND mWindow;
	uint32_t mWidth; 
	uint32_t mHeight;

	ID3D12Device* mpDevice;
	ID3D12CommandQueue* mpCommandQueue;
	IDXGISwapChain3* mpSwapChain;
	ID3D12Resource* mpColorBuffer[mFrameCount];
	ID3D12CommandAllocator* mpCommandAllocator[mFrameCount];
	ID3D12GraphicsCommandList* mpCommandList;
	ID3D12DescriptorHeap* mpHeapRTV;
	ID3D12Fence* mpFence;
	HANDLE mFenceEvent;
	uint64_t mFenceCounter[mFrameCount];
	uint32_t mFrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE mHandleRTV[mFrameCount];

	bool initApp();
	void termApp();
	bool initWindow();
	void termWindow();
	void mainLoop();

	bool initD3D();
	void termD3D();
	void render();
	void waitGpu();
	void present(uint32_t interval);

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

};