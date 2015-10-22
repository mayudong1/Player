#include "Direct3DPlayer.h"

CDirect3DPlayer::CDirect3DPlayer(void)
{
	m_pD3D9 = NULL;
	m_pD3d9Device = NULL;
	m_pD3d9Surface = NULL;
	m_hWnd = NULL;

	m_nWidth = 0;
	m_nHeight = 0;
}


CDirect3DPlayer::~CDirect3DPlayer(void)
{
	Close();
}

int CDirect3DPlayer::Open(HWND hWnd, int nWidth, int nHeight)
{
	m_hWnd = hWnd;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(m_pD3D9 == NULL)
	{
		return -1;
	}

	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	if( FAILED( m_pD3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &m_pD3d9Device ) ) )
	{
		return -1;
	}

	if(FAILED(m_pD3d9Device->CreateOffscreenPlainSurface(nWidth, 
		nHeight, 
		(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), 
		D3DPOOL_DEFAULT, 
		&m_pD3d9Surface,
		NULL)))
	{
		return -1;
	}

	GetClientRect(hWnd, &m_rcWindow);
	return 0;
}

int CDirect3DPlayer::Close()
{
	if(m_pD3d9Device)
	{
		//m_pD3d9Device->Clear();
	}

	if(m_pD3d9Surface)
	{
		m_pD3d9Surface->Release();
		m_pD3d9Surface = NULL;
	}
	if(m_pD3d9Device)
	{
		m_pD3d9Device->Release();
		m_pD3d9Device = NULL;
	}
	return 0;
}

int CDirect3DPlayer::Draw(unsigned char* yuv[3])
{
	if(m_pD3d9Device == NULL)
	{
		return -1;
	}

	HRESULT hr_lost = m_pD3d9Device->TestCooperativeLevel();
	if(FAILED(hr_lost))
	{
		OutputDebugString("ddddd\n");
		return -1;
	}

	D3DLOCKED_RECT d3d_rect;
	m_pD3d9Surface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
	unsigned char* dest[3];
	dest[0] = (unsigned char*)d3d_rect.pBits;
	dest[1] = (unsigned char*)d3d_rect.pBits + m_nHeight * d3d_rect.Pitch;
	dest[2] = (unsigned char*)dest[1] + m_nHeight * d3d_rect.Pitch / 4;

	for(int i=0;i<m_nHeight;i++)
	{
		memcpy(dest[0] + (i*d3d_rect.Pitch), yuv[0] + (i*m_nWidth), m_nWidth);
	}	
	for(int i=0;i<m_nHeight/2;i++)
	{
		memcpy(dest[2] + (i*d3d_rect.Pitch/2), yuv[1] + (i*m_nWidth/2), m_nWidth/2);
		memcpy(dest[1] + (i*d3d_rect.Pitch/2), yuv[2] + (i*m_nWidth/2), m_nWidth/2);
	}		
	m_pD3d9Surface->UnlockRect();


	m_pD3d9Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	m_pD3d9Device->BeginScene();
	IDirect3DSurface9* pBackSurface = NULL;
	m_pD3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
	if(pBackSurface != NULL)
	{
		m_pD3d9Device->StretchRect(m_pD3d9Surface, NULL, pBackSurface, NULL, D3DTEXF_LINEAR);
	}
	pBackSurface->Release();
	m_pD3d9Device->EndScene();
	m_pD3d9Device->Present(NULL, NULL, NULL, NULL);
	return 0;
}