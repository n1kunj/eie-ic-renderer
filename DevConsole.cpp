#include "DXUT.h"
#include "DevConsole.h"
#include <sstream>
#include <time.h>

#define DEBUG_TEXT_LINE_HEIGHT 15
#define LINES_TO_DISPLAY 15
#define CONSOLE_MAX_CHARACTERS 1024

void processConsoleInput(WCHAR* input);

WCHAR currentInput[CONSOLE_MAX_CHARACTERS];
int currentInputCursor = 0;

void DevConsole::OnD3D11FrameRender()
{
	debugTextArray->render(0,0,DEBUG_TEXT_LINE_HEIGHT,0,LINES_TO_DISPLAY);
	time_t seconds = time(NULL);

	if (seconds % 2 == 0) {
		debugText->RenderSingleLine(currentInput, 0, DEBUG_TEXT_LINE_HEIGHT * LINES_TO_DISPLAY);
	}
	else {
		//Draw a blinking cursor
		size_t linelen = wcslen(currentInput);
		WCHAR* inputCopy = new WCHAR[linelen+2];
		wcscpy_s(inputCopy,linelen+1,currentInput);

		inputCopy[currentInputCursor] = L'|';
		debugText->RenderSingleLine(inputCopy, 0, DEBUG_TEXT_LINE_HEIGHT * LINES_TO_DISPLAY);
		delete[] inputCopy;
	}
}

void DevConsole::log(const WCHAR* line) {
	debugTextArray->addDebugLine(line);
}

void DevConsole::log(std::wstringstream* wss) {
	DevConsole::log(wss->str().c_str());
}

LRESULT DevConsole::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext ) {
	if (uMsg == WM_CHAR) {
		DevConsole::OnCharacter(wParam);
		*pbNoFurtherProcessing = true;
	}
	return 0;
}


void DevConsole::OnCharacter(WPARAM wParam) {
	//Percentage symbol - causes a crash!
	if (wParam == 37) {
		return;
	}

	//Enter key
	if (wParam == 13) {
		processConsoleInput(currentInput);
		SecureZeroMemory(currentInput, CONSOLE_MAX_CHARACTERS*sizeof(WCHAR));
		currentInputCursor = 0;
	}
	//Backspace key
	else if (wParam == 8) {
		if (currentInputCursor != 0) {
			currentInputCursor--;
			currentInput[currentInputCursor] = NULL;
		}
	}
	//Actual characters
	else if (wParam >= 31) {
		if (currentInputCursor < CONSOLE_MAX_CHARACTERS) {
			currentInput[currentInputCursor] = WCHAR(wParam);
			currentInputCursor++;
		}
	}
}

void DevConsole::processConsoleInput(WCHAR* input) {
	DevConsole::log(input);
	size_t linelen = wcslen(input);
	if (linelen == 0) {
		return;
	}
	std::wstringstream wss;
	wss << L"Input Error:" << input;
	DevConsole::log(&wss);
}