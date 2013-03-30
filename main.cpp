//Based off Empty Project in DX SDK Samples
#include "DXUT.h"
#include "DebugText.h"
#include "DevConsole.h"
#include "Gui.h"
#include "Renderer.h"
#include "SDKmisc.h"
#include "DXUTgui.h"

#include <stdlib.h>
#include <sstream>

#define FOCUSED_GUI 0
#define FOCUSED_CONSOLE 1
#define FOCUSED_RENDERER 2

CDXUTDialogResourceManager dxutDialogResourceManager; // manager for shared resources of dialogs

DebugText debugText = DebugText();
DevConsole devConsole = DevConsole(&debugText);
Gui gui = Gui(&debugText);
Renderer renderer = Renderer(&devConsole);
RendererMessageProcessor* rMProc;

UINT focusedUI = FOCUSED_GUI;

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									  DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	devConsole.log(L"IsD3D11DeviceAcceptable");
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	devConsole.log(L"ModifyDeviceSettings");
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									 void* pUserContext )
{
	devConsole.log(L"OnD3D11CreateDevice");

	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(dxutDialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
	V_RETURN(debugText.OnD3D11CreateDevice(pd3dDevice,&dxutDialogResourceManager));
	V_RETURN(gui.OnD3D11CreateDevice(pd3dDevice));
	V_RETURN(renderer.OnD3D11CreateDevice(pd3dDevice,pBackBufferSurfaceDesc));

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	devConsole.log(L"OnD3D11ResizedSwapChain");

	std::wstringstream wss;
	wss << L"New Window Size = " << pBackBufferSurfaceDesc->Width << L"x" << pBackBufferSurfaceDesc->Height;
	devConsole.log(&wss);

	HRESULT hr;

	V_RETURN(dxutDialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
	V_RETURN(gui.OnD3D11ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc));
	V_RETURN(renderer.OnD3D11ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc));

	return S_OK;

}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	renderer.OnFrameMove( fTime, fElapsedTime, pUserContext );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								 double fTime, float fElapsedTime, void* pUserContext )
{
	//Draw settings dialog above all else
	if (gui.IsSettingsDialogueActive()) {
		gui.RenderSettingsDialogue(fElapsedTime);
	}
	else {
		//Draw what the renderer wants to draw
		renderer.OnD3D11FrameRender(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext);

		//Draw the overlays
		if (focusedUI == FOCUSED_GUI) {
			gui.OnD3D11FrameRender(fElapsedTime);
		}
		else if (focusedUI == FOCUSED_CONSOLE) {
			devConsole.OnD3D11FrameRender();
		}

	}
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	devConsole.log(L"OnD3D11ReleasingSwapChain");

	dxutDialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	devConsole.log(L"OnD3D11DestroyDevice");

	dxutDialogResourceManager.OnD3D11DestroyDevice();
	gui.OnD3D11DestroyDevice();
	debugText.OnD3D11DestroyDevice();
	renderer.OnD3D11DestroyDevice();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						 bool* pbNoFurtherProcessing, void* pUserContext )
{

	//std::wstringstream wss;
	//wss << L"uMsg:" << uMsg << L" wParam:" << wParam << L" lParam:" << lParam;
	//devConsole.log(&wss);

	//Settings dialog takes message priority
	if (gui.IsSettingsDialogueActive()) {
		gui.SettingsDialogueMsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
		return 0;
	}

	//If tilde gets pressed, toggle between console/gui display modes
	if (uMsg == WM_CHAR && wParam == 96) {

		if (focusedUI == FOCUSED_RENDERER) {
			focusedUI = FOCUSED_GUI;
		}
		else if (focusedUI == FOCUSED_GUI) {
			focusedUI = FOCUSED_CONSOLE;
		}
		else if (focusedUI == FOCUSED_CONSOLE) {
			focusedUI = FOCUSED_RENDERER;
		}

		*pbNoFurtherProcessing = true;
	}

	if (*pbNoFurtherProcessing) {
		return 0;
	}

	if (focusedUI == FOCUSED_GUI) {
		gui.MsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
	}
	else if (focusedUI == FOCUSED_CONSOLE) {
		devConsole.MsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
	}

	if (*pbNoFurtherProcessing) {
		return 0;
	}

	if (focusedUI!= FOCUSED_CONSOLE) {
		renderer.MsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses. For raw key input, this is always called before MsgProc
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					  bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					  int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	devConsole.log(L"OnDeviceRemoved");
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	//Must be initialised here as static initialisation causes an assertion failure
	RendererMessageProcessor rMP = RendererMessageProcessor(&devConsole,&renderer);
	rMProc = &rMP;

	devConsole.setMessageProcessor(rMProc);

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here
	gui.init(&dxutDialogResourceManager);
	renderer.init();

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"Renderer" );

	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 640, 480 );
	DXUTMainLoop(); // Enter into the DXUT render loop

	// Perform any application-level cleanup here
	renderer.OnExit();

	return DXUTGetExitCode();
}