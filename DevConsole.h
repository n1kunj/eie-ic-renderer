#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "DebugText.h"

	namespace DevConsole {
		void draw();
		void log(const WCHAR* line);
		void OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
		HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
		HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
		void OnD3D11ReleasingSwapChain();
		void OnD3D11DestroyDevice();
	}

#endif