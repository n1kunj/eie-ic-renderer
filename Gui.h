#pragma once
#ifndef GUI_H
#define GUI_H

#include "SDKmisc.h"
#include "DXUTSettingsDlg.h"

class DebugText;

class Gui {
public:
	Gui(DebugText* dt): debugText(dt)  {
	}
	void init(CDXUTDialogResourceManager* g_DialogResourceManager);
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
private:
	static void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

	DebugText* debugText;

	static CD3DSettingsDlg settingsDlg; // Device settings dialog
	CDXUTDialog hud; // dialog for standard controls
	CDXUTDialog sampleUI; // dialog for sample specific controls
};

#endif