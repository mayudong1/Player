#include <stdio.h>
#include <windows.h>
#include "Direct3DPlayer.h"


static char className[256] = "testClassName";
static char title[256] = "testTitle";
static char file[256] = "test.yuv";
static int pic_width = 176;
static int pic_height = 144;
static FILE* pFile = NULL;

CDirect3DPlayer* pPlayer = NULL;


HWND InitWindow();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int DrawPic();

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
			pPlayer->Open(hWnd, pic_width, pic_height);	
			SetTimer(hWnd, 1, 40, NULL);
		}
		break;
	case WM_TIMER:
		if(pPlayer != NULL)
		{			
			DrawPic();
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
	pFile = fopen("test.yuv", "rb");	
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

	fclose(pFile);
	return 0;
}

int DrawPic()
{
	if(pFile == NULL || pPlayer == NULL)
	{
		return -1;
	}
	int yuv_size = pic_width*pic_height*3/2;
	unsigned char* data = new unsigned char[yuv_size];
	unsigned char* yuv[3];
	yuv[0] = data;
	yuv[1] = data + pic_width*pic_height;
	yuv[2] = yuv[1] + pic_width*pic_height/4;

	int ret = fread(data, yuv_size, 1, pFile);
	if(ret > 0)
	{
		pPlayer->Draw(yuv);
	}
	else
	{
		fseek(pFile, 0, SEEK_SET);
	}	
	delete[] data;
	return 0;
}