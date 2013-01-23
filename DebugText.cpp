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
								 FLOAT colourB )
{
	dxutTextHelper->Begin();
	dxutTextHelper->SetForegroundColor( D3DXCOLOR( colourR, colourG, colourB, 1.0f ) );
	dxutTextHelper->SetInsertionPos( x, y );
	dxutTextHelper->DrawFormattedTextLine( line );
	dxutTextHelper->End();
}

void DebugText::RenderDebugTextArray(const DebugTextArray* dta, int x, int y, int spacing, int startIndex, int numToDisplay )
{
	dxutTextHelper->Begin();

	int numStrings;

	if (dta->saturated) {
		numStrings = dta->size;
	}
	else {
		numStrings = dta->curString;
	}

	y += spacing * (numToDisplay-1);

	for (int i = dta->curString - startIndex - 1; i>=0; i--) {
		dxutTextHelper->SetForegroundColor( D3DXCOLOR( dta->mDebugLineArray[i].colourR, dta->mDebugLineArray[i].colourG, dta->mDebugLineArray[i].colourB, 1.0f ) );
		dxutTextHelper->SetInsertionPos( x, y );
		dxutTextHelper->DrawFormattedTextLine( dta->mDebugLineArray[i].mLine );
		y-=spacing;
		numToDisplay--;
		if (numToDisplay == 0) {
			break;
		}
	}
	if (dta->saturated && numToDisplay > 0) {
		for (int i = dta->size-1; i>=dta->curString - startIndex; i--) {
			dxutTextHelper->SetForegroundColor( D3DXCOLOR( dta->mDebugLineArray[i].colourR, dta->mDebugLineArray[i].colourG, dta->mDebugLineArray[i].colourB, 1.0f ) );
			dxutTextHelper->SetInsertionPos( x, y );
			dxutTextHelper->DrawFormattedTextLine( dta->mDebugLineArray[i].mLine );
			y-=spacing;
			numToDisplay--;
			if (numToDisplay == 0) {
				break;
			}
		}
	}
	dxutTextHelper->End();
}

void DebugTextArray::addDebugLine(const WCHAR* line, FLOAT r, FLOAT g, FLOAT b) {

	mDebugLineArray[curString].setLine(line,r,g,b);

	curString++;
	if (curString == size) {
		saturated = true;
		curString = 0;
	}
}