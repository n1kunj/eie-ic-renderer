#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

//#include <sstream>
#include "DebugText.h"

	namespace DevConsole {
		void draw();
		void log(const WCHAR* line);
		void log(std::wstringstream* wss);
		void OnCharacter(WPARAM wParam);
		HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
		HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
		void OnD3D11ReleasingSwapChain();
		void OnD3D11DestroyDevice();
		void processConsoleInput(WCHAR* input);
	}

#endif