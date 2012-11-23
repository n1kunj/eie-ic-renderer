#include "DXUT.h"
#include "Gui.h"

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

CD3DSettingsDlg Gui::g_SettingsDlg;

void Gui::init(CDXUTDialogResourceManager* g_DialogResourceManager )
{
	g_SettingsDlg.Init( g_DialogResourceManager );
	g_HUD.Init( g_DialogResourceManager );
	g_SampleUI.Init( g_DialogResourceManager );

	g_HUD.SetCallback(&Gui::OnGUIEvent);
	int iY = 30;
	int iYo = 26;
	g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
	g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
	g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

	g_SampleUI.SetCallback(&Gui::OnGUIEvent); iY = 10;
}

HRESULT Gui::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	HRESULT hr;

	V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );

	return S_OK;
}

HRESULT Gui::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	HRESULT hr;
	V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	g_HUD.SetSize( 170, 170 );
	g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
	g_SampleUI.SetSize( 170, 300 );

	return S_OK;
}

void Gui::OnD3D11FrameRender( float fElapsedTime )
{
	g_HUD.OnRender( fElapsedTime );
	g_SampleUI.OnRender( fElapsedTime );
	debugText->RenderSingleLine(DXUTGetFrameStats( DXUTIsVsyncEnabled() ), 5, 5, 1.0f, 1.0f, 0.0f);
	debugText->RenderSingleLine(DXUTGetDeviceStats(), 5, 20, 1.0f, 1.0f, 0.0f);
}

void Gui::OnD3D11DestroyDevice()
{
	g_SettingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
}

LRESULT Gui::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	
	if( *pbNoFurtherProcessing ) {
		return 0;
	}

	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );

	return 0;
}

bool Gui::IsSettingsDialogueActive() {
	return g_SettingsDlg.IsActive();
}

void Gui::RenderSettingsDialogue(float fElapsedTime) {
	g_SettingsDlg.OnRender( fElapsedTime );
}

LRESULT Gui::SettingsDialogueMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
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
		g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
		break;
	}
}