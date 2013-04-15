#pragma once
#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "SDKmisc.h"
#include "MessageProcessor.h"

#define DEBUG_TEXT_LINE_HEIGHT 15
#define LINES_TO_DISPLAY 15
#define CONSOLE_MAX_CHARACTERS 1024

class DebugText;
class DebugTextArray;

class DevConsole : public MessageLogger {
public:
	DevConsole(DebugText* dt);
	~DevConsole();
	void OnD3D11FrameRender();
	using MessageLogger::log;
	void log(const WCHAR* line);
	void log(const WCHAR* line, FLOAT r, FLOAT g, FLOAT b);
	void OnCharacter(WPARAM wParam);
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		bool* pbNoFurtherProcessing, void* pUserContext );
	void setMessageProcessor(MessageProcessor* pMProc) {
		mMessageProcessor = pMProc;
	}
private:
	DebugText* mDebugText;
	DebugTextArray* mDebugTextArray;
	MessageProcessor* mMessageProcessor;
	INT mCurrentInputCursor;
	INT mShowStartLine;
	void processConsoleInput(WCHAR* input);
	WCHAR* currentInput;
	FLOAT mInputColourR;
	FLOAT mInputColourG;
	FLOAT mInputColourB;
	void onPageUp(INT numLines);
	void onPageDown(INT numLines);
};

#endif