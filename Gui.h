#ifndef GUI_H
#define GUI_H

#include "SDKmisc.h"

namespace Gui {
	void guiInit(CDXUTDialogResourceManager* g_DialogResourceManager);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	void OnD3D11FrameRender(float fElapsedTime);
	void OnD3D11DestroyDevice();

	bool IsSettingsDialogueActive();
	void RenderSettingsDialogue(float fElapsedTime);

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext );

	LRESULT SettingsDialogueMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext );
}

#endif