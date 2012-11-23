#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "DebugText.h"
#include "SDKmisc.h"

class DevConsole {
public:
	DevConsole(DebugText* dt): debugText(dt){
		debugTextArray = new DebugTextArray(2000,1.0f,1.0f,0.0f,1.0f);
	}
	~DevConsole() {
		delete debugTextArray;
	}
	void OnD3D11FrameRender();
	void log(const WCHAR* line);
	void log(std::wstringstream* wss);
	void OnCharacter(WPARAM wParam);
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext );
private:
	DebugText* debugText;
	DebugTextArray* debugTextArray;
	void processConsoleInput(WCHAR* input);
};

#endif