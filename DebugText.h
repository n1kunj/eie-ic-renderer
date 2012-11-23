#ifndef DEBUG_TEXT_H
#define DEBUG_TEXT_H

#include <DXUT.h>
#include <string>

namespace DebugText{
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, int lineHeight);
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
	void OnD3D11DestroyDevice();

	void RenderSingleLine(const WCHAR* line, int x, int y,
		FLOAT colourR = 1.0f, FLOAT colourG = 1.0f,
		FLOAT colourB = 1.0f, FLOAT colourA = 1.0f );
}

class DebugTextArray {
public:
	DebugTextArray(int historyLength, FLOAT colourR = 1.0f, FLOAT colourG = 1.0f,
		FLOAT colourB = 1.0f, FLOAT colourA = 1.0f)
		: size(historyLength), curString(0), saturated(false), colourR(colourR),
		colourG(colourG), colourB(colourB), colourA(colourA){
		textArray = new WCHAR*[historyLength]();
	}
	~DebugTextArray() {
		for (int i = 0; i < size; i++) {
			delete[] textArray[i];
		} 
		delete[] textArray;
	}
	void addDebugLine(const WCHAR* line);
	void addDebugLine(int i);
	void render(int x, int y, int spacing, int startIndex, int numToDisplay);
	void setTextColour(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
private:
	int size;
	int curString;
	bool saturated;
	WCHAR** textArray;

	FLOAT colourR;
	FLOAT colourG;
	FLOAT colourB;
	FLOAT colourA;
};

#endif