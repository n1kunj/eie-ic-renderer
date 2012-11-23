#include "DXUT.h"
#include "Gui.h"

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

CD3DSettingsDlg Gui::settingsDlg;

void Gui::init(CDXUTDialogResourceManager* g_DialogResourceManager )
{
	settingsDlg.Init( g_DialogResourceManager );
	hud.Init( g_DialogResourceManager );
	sampleUI.Init( g_DialogResourceManager );

	hud.SetCallback(&Gui::OnGUIEvent);
	int iY = 30;
	int iYo = 26;
	hud.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
	hud.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
	hud.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

	sampleUI.SetCallback(&Gui::OnGUIEvent); iY = 10;
}

HRESULT Gui::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	HRESULT hr;

	V_RETURN( settingsDlg.OnD3D11CreateDevice( pd3dDevice ) );

	return S_OK;
}

HRESULT Gui::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	HRESULT hr;
	V_RETURN( settingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	hud.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	hud.SetSize( 170, 170 );
	sampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
	sampleUI.SetSize( 170, 300 );

	return S_OK;
}

void Gui::OnD3D11FrameRender( float fElapsedTime )
{
	hud.OnRender( fElapsedTime );
	sampleUI.OnRender( fElapsedTime );
	debugText->RenderSingleLine(DXUTGetFrameStats( DXUTIsVsyncEnabled() ), 5, 5, 1.0f, 1.0f, 0.0f);
	debugText->RenderSingleLine(DXUTGetDeviceStats(), 5, 20, 1.0f, 1.0f, 0.0f);
}

void Gui::OnD3D11DestroyDevice()
{
	settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
}

LRESULT Gui::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	*pbNoFurtherProcessing = hud.MsgProc( hWnd, uMsg, wParam, lParam );
	
	if( *pbNoFurtherProcessing ) {
		return 0;
	}

	*pbNoFurtherProcessing = sampleUI.MsgProc( hWnd, uMsg, wParam, lParam );

	return 0;
}

bool Gui::IsSettingsDialogueActive() {
	return settingsDlg.IsActive();
}

void Gui::RenderSettingsDialogue(float fElapsedTime) {
	settingsDlg.OnRender( fElapsedTime );
}

LRESULT Gui::SettingsDialogueMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	settingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
	return 0;
}

void CALLBACK Gui::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;
	case IDC_TOGGLEREF:
		DXUTToggleREF();
		break;
	case IDC_CHANGEDEVICE:
		settingsDlg.SetActive( !settingsDlg.IsActive() );
		break;
	}
}