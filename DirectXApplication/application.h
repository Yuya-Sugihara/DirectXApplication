#pragma once

#include <Windows.h>
#include <cstdint>

class Application
{
public: 

	Application(uint32_t width, uint32_t height);
	~Application();

	void run();

private: 

	HINSTANCE mHInstance;
	HWND mWindow;
	uint32_t mWidth; 
	uint32_t mHeight;

	bool initApp();
	void termApp();
	bool initWindow();
	void termWindow();
	void mainLoop();

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};