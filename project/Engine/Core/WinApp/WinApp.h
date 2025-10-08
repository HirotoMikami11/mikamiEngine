#pragma once
#include <Windows.h>
#include "GraphicsConfig.h"
#include "MyFunction.h"
//ファイルに書いたり読んだりするライブラリ
#include<fstream>

class WinApp
{

public:

	WinApp();
	~WinApp();

	void Initialize(const std::wstring& title);
	void Finalize();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	bool ProsessMessege();
	//ゲッター
	HWND GetHwnd() { return hwnd; };
	HINSTANCE GetHInstance() const { return wc.hInstance; }
private:
	HWND hwnd = nullptr;
	WNDCLASS wc{};
};

