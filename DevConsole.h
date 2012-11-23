#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

#include "DebugText.h"
#include "SDKmisc.h"

	namespace DevConsole {
		void OnD3D11FrameRender();
		void log(const WCHAR* line);
		void log(std::wstringstream* wss);
		void OnCharacter(WPARAM wParam);
		LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
			bool* pbNoFurtherProcessing, void* pUserContext );
	}

#endif