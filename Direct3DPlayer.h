#ifndef __DIRECT3DPLAYER_H_
#define __DIRECT3DPLAYER_H_

#include <d3d9.h>

class CDirect3DPlayer
{
public:
	CDirect3DPlayer(void);
	~CDirect3DPlayer(void);

public:
	int Open(HWND hWnd, int nWidth, int nHeight);
	int Close();
	int Draw();
private:
	IDirect3D9* m_pD3D9;
	IDirect3DDevice9* m_pD3d9Device;
	IDirect3DSurface9* m_pD3d9Surface;
	HWND m_hWnd;
	RECT m_rcWindow;

	int m_nWidth;
	int m_nHeight;
};

#endif