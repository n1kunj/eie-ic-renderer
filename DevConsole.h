#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "DebugText.h"
#include "SDKmisc.h"

#define DEBUG_TEXT_LINE_HEIGHT 15
#define LINES_TO_DISPLAY 15
#define CONSOLE_MAX_CHARACTERS 1024

class DevConsole {
public:
	DevConsole(DebugText* dt): debugText(dt), currentInputCursor(0){
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
	int currentInputCursor;
	void processConsoleInput(WCHAR* input);

	WCHAR currentInput[CONSOLE_MAX_CHARACTERS];
};

#endif