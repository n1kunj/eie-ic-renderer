#ifndef DEBUG_TEXT_H
#define DEBUG_TEXT_H

#include "DXUT.h"
#include "SDKmisc.h"
#include <string>

class DebugTextArray {
public:
	DebugTextArray(int historyLength, FLOAT colourR, FLOAT colourG, FLOAT colourB, FLOAT colourA) :
		size(historyLength), curString(0), saturated(false), colourR(colourR), colourG(colourG), colourB(colourB), colourA(colourA)
	{
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
	void setTextColour(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
	FLOAT colourR;
	FLOAT colourG;
	FLOAT colourB;
	FLOAT colourA;
	int curString;
	int size;
	bool saturated;
	WCHAR** textArray;
};

class DebugText{
public:
	DebugText(): dxutTextHelper(NULL) {
	}
	~DebugText() {
		delete dxutTextHelper;
	}
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, CDXUTDialogResourceManager* dxutResourceManager);
	void OnD3D11DestroyDevice();
	void RenderDebugTextArray(const DebugTextArray* dta, int x, int y, int spacing, int startIndex, int numToDisplay);
	void RenderSingleLine(const WCHAR* line, int x, int y,
		FLOAT colourR = 1.0f, FLOAT colourG = 1.0f,
		FLOAT colourB = 1.0f, FLOAT colourA = 1.0f );
private:
	CDXUTTextHelper* dxutTextHelper;
};

#endif