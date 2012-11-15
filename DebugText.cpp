#include "DXUT.h"
#include "DebugText.h"
#include "DXUTgui.h"
#include "SDKmisc.h"

CDXUTDialogResourceManager g_resourceManager;
CDXUTTextHelper*           g_pTxtHelper = NULL;

void DebugTextArray::addDebugLine(const WCHAR* line) {

	size_t linelen = wcslen(line);
	WCHAR* chars = new WCHAR[linelen+1];
	wcscpy_s(chars,linelen+1,line);

	SAFE_DELETE( textArray[this->curString] );

	textArray[curString] = chars;

	curString++;
	if (curString == size) {
		saturated = true;
		curString = 0;
	}
}

void DebugTextArray::addDebugLine( int i )
{
	this->addDebugLine(std::to_wstring(i).c_str());
}

void DebugTextArray::render(int x, int y, int spacing) {
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );

	int numStrings;

	if (saturated) {
		numStrings = size;
	}
	else {
		numStrings = curString;
	}

	for (int i = curString - 1; i>=0; i--) {
		g_pTxtHelper->SetInsertionPos( x, y );
		g_pTxtHelper->DrawFormattedTextLine( this->textArray[i] );
		y+=spacing;
	}
	if (saturated) {
		for (int i = size-1; i>=curString; i--) {
			g_pTxtHelper->SetInsertionPos( x, y );
			g_pTxtHelper->DrawFormattedTextLine( this->textArray[i] );
			y+=spacing;
		}
	}
	g_pTxtHelper->End();
}

HRESULT DebugTextOnD3D11CreateDevice(ID3D11Device* pd3dDevice, int lineHeight)
{
	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	V_RETURN (g_resourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));

	g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_resourceManager, lineHeight );

	return S_OK;
}

HRESULT DebugTextOnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	HRESULT hr;

	V_RETURN(g_resourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void DebugTextOnD3D11ReleasingSwapChain()
{
	g_resourceManager.OnD3D11ReleasingSwapChain();
}

void DebugTextOnD3D11DestroyDevice() 
{
	g_resourceManager.OnD3D11DestroyDevice();
	SAFE_DELETE( g_pTxtHelper );
}