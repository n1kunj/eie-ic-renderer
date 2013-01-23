#pragma once
#ifndef DEBUG_TEXT_H
#define DEBUG_TEXT_H

#include "SDKmisc.h"
#include <string>

struct debugLine {
	const WCHAR* mLine;
	FLOAT colourR;
	FLOAT colourG;
	FLOAT colourB;

	debugLine() : mLine(NULL), colourR(1), colourG(1), colourB(1){

	}

	void setLine(const WCHAR* pLine, FLOAT r, FLOAT g, FLOAT b) {
		colourR = r;
		colourG = g;
		colourB = b;
		SAFE_DELETE_ARRAY(mLine);

		size_t linelen = wcslen(pLine);
		WCHAR* chars = new WCHAR[linelen+1];
		wcscpy_s(chars,linelen+1,pLine);

		mLine = chars;
	}

	~debugLine() {
		SAFE_DELETE_ARRAY(mLine);
	}
};

class DebugTextArray {
public:
	DebugTextArray(int historyLength) :
		size(historyLength), curString(0), saturated(false)
	{
		mDebugLineArray = new debugLine[historyLength]();
	}
	~DebugTextArray() {
		SAFE_DELETE_ARRAY(mDebugLineArray);
	}
	void addDebugLine(const WCHAR* line, FLOAT r, FLOAT g, FLOAT b);
	int curString;
	int size;
	bool saturated;
	debugLine* mDebugLineArray;
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
		FLOAT colourB = 1.0f);
private:
	CDXUTTextHelper* dxutTextHelper;
};

#endif