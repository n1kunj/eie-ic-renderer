#include "DXUT.h"
#include "DevConsole.h"
#include "DebugText.h"
#include <sstream>
#include <time.h>

DevConsole::DevConsole(DebugText* dt): mDebugText(dt), mCurrentInputCursor(0), mMessageProcessor(NULL){
	mDebugTextArray = new DebugTextArray(2000,1.0f,1.0f,0.0f,1.0f);
}

DevConsole::~DevConsole() {
	delete mDebugTextArray;
}

void DevConsole::OnD3D11FrameRender()
{
	mDebugText->RenderDebugTextArray(mDebugTextArray,0,0,DEBUG_TEXT_LINE_HEIGHT,0,LINES_TO_DISPLAY);
	time_t seconds = time(NULL);

	if (seconds % 2 == 0) {
		mDebugText->RenderSingleLine(currentInput, 0, DEBUG_TEXT_LINE_HEIGHT * LINES_TO_DISPLAY);
	}
	else {
		//Draw a blinking cursor
		size_t linelen = wcslen(currentInput);
		WCHAR* inputCopy = new WCHAR[linelen+2];
		wcscpy_s(inputCopy,linelen+1,currentInput);

		inputCopy[mCurrentInputCursor] = L'|';
		mDebugText->RenderSingleLine(inputCopy, 0, DEBUG_TEXT_LINE_HEIGHT * LINES_TO_DISPLAY);
		delete[] inputCopy;
	}
}

void DevConsole::log(const WCHAR* line) {
	mDebugTextArray->addDebugLine(line);
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
		mCurrentInputCursor = 0;
	}
	//Backspace key
	else if (wParam == 8) {
		if (mCurrentInputCursor != 0) {
			mCurrentInputCursor--;
			currentInput[mCurrentInputCursor] = NULL;
		}
	}
	//Actual characters
	else if (wParam >= 31) {
		if (mCurrentInputCursor < CONSOLE_MAX_CHARACTERS) {
			currentInput[mCurrentInputCursor] = WCHAR(wParam);
			mCurrentInputCursor++;
		}
	}
}

void DevConsole::processConsoleInput(WCHAR* input) {
	log(input);
	size_t linelen = wcslen(input);
	if (linelen == 0) {
		return;
	}
	if (mMessageProcessor == NULL) {
		std::wstringstream wss;
		wss << L"Input Error:" << input;
		log(&wss);
	}
	else {
		mMessageProcessor->processMessage(input);
	}

}