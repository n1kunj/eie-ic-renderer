#include "DXUT.h"
#include "DebugText.h"

#define DXUTLINEHEIGHT 15

HRESULT DebugText::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, CDXUTDialogResourceManager* dxutResourceManager )
{
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	dxutTextHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, dxutResourceManager, DXUTLINEHEIGHT );

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

void DebugText::RenderDebugTextArray(const DebugTextArray* dta, int x, int y, int spacing, int startIndex, int numToDisplay )
{
	dxutTextHelper->Begin();
	dxutTextHelper->SetForegroundColor( D3DXCOLOR( dta->colourR, dta->colourG, dta->colourB, dta->colourA ) );

	int numStrings;

	if (dta->saturated) {
		numStrings = dta->size;
	}
	else {
		numStrings = dta->curString;
	}

	y += spacing * (numToDisplay-1);

	for (int i = dta->curString - startIndex - 1; i>=0; i--) {
		dxutTextHelper->SetInsertionPos( x, y );
		dxutTextHelper->DrawFormattedTextLine( dta->textArray[i] );
		y-=spacing;
		numToDisplay--;
		if (numToDisplay == 0) {
			break;
		}
	}
	if (dta->saturated && numToDisplay > 0) {
		for (int i = dta->size-1; i>=dta->curString - startIndex; i--) {
			dxutTextHelper->SetInsertionPos( x, y );
			dxutTextHelper->DrawFormattedTextLine( dta->textArray[i] );
			y-=spacing;
			numToDisplay--;
			if (numToDisplay == 0) {
				break;
			}
		}
	}
	dxutTextHelper->End();
}

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

void DebugTextArray::setTextColour(FLOAT r, FLOAT g, FLOAT b, FLOAT a) {
	colourR = r;
	colourG = g;
	colourB = b;
	colourA = a;
}
