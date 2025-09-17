#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include "render.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")


static char className[256] = "testClassName";
static char title[256] = "testTitle";
static char file[256] = "test.yuv";
static int pic_width = 176;
static int pic_height = 144;
static FILE* pFile = NULL;
D3D11YUV420PRenderer* pPlayer = NULL;


HWND InitWindow();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int DrawPic();

HWND InitWindow()
{
	WNDCLASSEXA wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = "testMenuName";
	wcex.lpszClassName = className;
	wcex.hIconSm = NULL;

	RegisterClassExA(&wcex);

	HWND hWnd = CreateWindowA(className, title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, NULL);

	if (!hWnd)
	{
		return NULL;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		if (pPlayer == NULL)
		{
			pPlayer = new D3D11YUV420PRenderer();
			pPlayer->Initialize(hWnd, pic_width, pic_height);
			SetTimer(hWnd, 1, 40, NULL);
		}
		break;
	case WM_TIMER:
		if (pPlayer != NULL)
		{
			DrawPic();
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (pPlayer != NULL)
		{
			delete pPlayer;
			pPlayer = NULL;
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	pFile = fopen("test.yuv", "rb");
	HWND hWnd = InitWindow();
	if (hWnd == NULL)
	{
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	fclose(pFile);
	return 0;
}

int DrawPic()
{
	if (pFile == NULL || pPlayer == NULL)
	{
		return -1;
	}
	int yuv_size = pic_width * pic_height * 3 / 2;
	unsigned char* data = new unsigned char[yuv_size];
	int ret = fread(data, yuv_size, 1, pFile);
	if (ret > 0)
	{
		pPlayer->Render(data);
	}
	else
	{
		fseek(pFile, 0, SEEK_SET);
	}
	delete[] data;
	return 0;
}