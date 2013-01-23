#include "DXUT.h"
#include "DevConsole.h"
#include "DebugText.h"
#include <sstream>
#include <time.h>

#define HISTORYLENGTH 2000
#define DEFAULTCOLOUR_R 1.0f
#define DEFAULTCOLOUR_G 1.0f
#define DEFAULTCOLOUR_B 0.0f
#define LINES_TO_SCROLL LINES_TO_DISPLAY/2

DevConsole::DevConsole(DebugText* dt): mDebugText(dt), mCurrentInputCursor(0),
	mMessageProcessor(NULL), mInputColourR(1.0f), mInputColourG(1.0f), mInputColourB(1.0f),
mShowStartLine(0) {
	assert(HISTORYLENGTH > LINES_TO_DISPLAY);
	mDebugTextArray = new DebugTextArray(HISTORYLENGTH);
}

DevConsole::~DevConsole() {
	delete mDebugTextArray;
}

void DevConsole::OnD3D11FrameRender()
{
	mDebugText->RenderDebugTextArray(mDebugTextArray,0,0,DEBUG_TEXT_LINE_HEIGHT,mShowStartLine,LINES_TO_DISPLAY);
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
	log(line,DEFAULTCOLOUR_R,DEFAULTCOLOUR_G,DEFAULTCOLOUR_B);
}

void DevConsole::log(const WCHAR* line, FLOAT r, FLOAT g, FLOAT b) {
	mDebugTextArray->addDebugLine(line,r,g,b);
	if (mShowStartLine !=0) {
		onPageUp(1);
	}
}

LRESULT DevConsole::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext ) {
	if (uMsg == WM_CHAR) {
		DevConsole::OnCharacter(wParam);
		*pbNoFurtherProcessing = true;
	}
	else if (uMsg == WM_KEYDOWN) {
		//Page up
		if (wParam == 34) {
			onPageUp(LINES_TO_SCROLL);
		}
		//Page down
		else if (wParam == 33) {
			onPageDown(LINES_TO_SCROLL);
		}
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
	mShowStartLine = 0;
	log(input,mInputColourR,mInputColourG,mInputColourB);
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

void DevConsole::onPageUp( INT numLines )
{
	mShowStartLine +=numLines;
	if (mShowStartLine > HISTORYLENGTH - numLines) {
		mShowStartLine = HISTORYLENGTH - numLines;
	}
}

void DevConsole::onPageDown( INT numLines )
{
	mShowStartLine -=numLines;
	if (mShowStartLine < 0) {
		mShowStartLine = 0;
	}
}

