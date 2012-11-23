#include "DXUT.h"
#include "DebugText.h"
#include "DXUTgui.h"

CDXUTTextHelper* dxutTextHelper = NULL;

void DebugTextArray::addDebugLine(const WCHAR* line) {

	size_t linelen = wcslen(line);
	WCHAR* chars = new WCHAR[linelen+1];
	wcscpy_s(chars,linelen+1,line);

	SAFE_DELETE_ARRAY( textArray[curString] );

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

void DebugTextArray::render(int x, int y, int spacing, int startIndex, int numToDisplay) {
	dxutTextHelper->Begin();
	dxutTextHelper->SetForegroundColor( D3DXCOLOR( colourR, colourG, colourB, colourA ) );

	int numStrings;

	if (saturated) {
		numStrings = size;
	}
	else {
		numStrings = curString;
	}

	y += spacing * (numToDisplay-1);

	for (int i = curString - startIndex - 1; i>=0; i--) {
		dxutTextHelper->SetInsertionPos( x, y );
		dxutTextHelper->DrawFormattedTextLine( this->textArray[i] );
		y-=spacing;
		numToDisplay--;
		if (numToDisplay == 0) {
			break;
		}
	}
	if (saturated && numToDisplay > 0) {
		for (int i = size-1; i>=curString - startIndex; i--) {
			dxutTextHelper->SetInsertionPos( x, y );
			dxutTextHelper->DrawFormattedTextLine( this->textArray[i] );
			y-=spacing;
			numToDisplay--;
			if (numToDisplay == 0) {
				break;
			}
		}
	}
	dxutTextHelper->End();
}

void DebugTextArray::setTextColour(FLOAT r, FLOAT g, FLOAT b, FLOAT a) {
	colourR = r;
	colourG = g;
	colourB = b;
	colourA = a;
}

HRESULT DebugText::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, int lineHeight, CDXUTDialogResourceManager* dxutResourceManager )
{
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	SAFE_DELETE(dxutTextHelper);
	dxutTextHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, dxutResourceManager, lineHeight );

	return S_OK;
}

void DebugText::OnD3D11DestroyDevice() 
{
	SAFE_DELETE( dxutTextHelper );
}

void DebugText::RenderSingleLine(const WCHAR* line, int x, int y,
							   FLOAT colourR, FLOAT colourG,
							   FLOAT colourB, FLOAT colourA )
{
	dxutTextHelper->Begin();
	dxutTextHelper->SetForegroundColor( D3DXCOLOR( colourR, colourG, colourB, colourA ) );
	dxutTextHelper->SetInsertionPos( x, y );
	dxutTextHelper->DrawFormattedTextLine( line );
	dxutTextHelper->End();
}