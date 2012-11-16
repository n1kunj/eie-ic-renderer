#include "DXUT.h"
#include "DevConsole.h"
#include "DebugText.h"

#define DEBUG_TEXT_LINE_HEIGHT 15

DebugTextArray dta = DebugTextArray(2000);

void DevConsole::draw() {
	DebugText::RenderSingleLine(L"SINGLE LINE", 0, 0);
	dta.render(0,15,15,0,20);
}

void DevConsole::log(const WCHAR* line) {
	dta.addDebugLine(line);
}

HRESULT DevConsole::OnD3D11CreateDevice( ID3D11Device* pd3dDevice)
{
	return DebugText::OnD3D11CreateDevice(pd3dDevice,DEBUG_TEXT_LINE_HEIGHT);
}

HRESULT DevConsole::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	return DebugText::OnD3D11ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc);
}

void DevConsole::OnD3D11ReleasingSwapChain()
{
	DebugText::OnD3D11ReleasingSwapChain();
}

void DevConsole::OnD3D11DestroyDevice()
{
	DebugText::OnD3D11DestroyDevice();
}

void DevConsole::OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext ) {
	DevConsole::log(L"On Keyboard!");
}