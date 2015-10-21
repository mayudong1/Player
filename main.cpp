#include <stdio.h>
#include <windows.h>
#include "Direct3DPlayer.h"


char className[256] = "testClassName";
char title[256] = "testTitle";

CDirect3DPlayer* pPlayer = NULL;


HWND InitWindow();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND InitWindow()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= "testMenuName";
	wcex.lpszClassName	= className;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindow(className, title, WS_OVERLAPPEDWINDOW,
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
		if(pPlayer == NULL)
		{			
			pPlayer = new CDirect3DPlayer();		
			pPlayer->Open(hWnd, 100, 100);	
			SetTimer(hWnd, 1, 100, NULL);
		}
		break;
	case WM_TIMER:
		if(pPlayer != NULL)
		{
			pPlayer->Draw();
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if(pPlayer != NULL)
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


int main()
{
	printf("this is a test\n");
	HWND hWnd = InitWindow();
	if(hWnd == NULL)
	{
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{				
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}
	return 0;
}