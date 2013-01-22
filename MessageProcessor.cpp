#include "DXUT.h"
#include "MessageProcessor.h"
#include "Renderer.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

RendererMessageProcessor::RendererMessageProcessor( MessageLogger* logger, Renderer* renderer ) :
	mLogger(logger), mRenderer(renderer)
{
	//Allocates memory within the function!
	mLuaState = luaL_newstate();

	luabind::open(mLuaState);

	// Define a lua function that we can call
	luaL_dostring(
		mLuaState,
		"function add(first, second)\n"
		"  return first + second\n"
		"end\n"
		);
}

RendererMessageProcessor::~RendererMessageProcessor()
{
	//Deallocates within the function
	lua_close(mLuaState);
}

void RendererMessageProcessor::processMessage(WCHAR* pInput)
{
	std::wstringstream wss;
	wss << L"Processing " << pInput;
	mLogger->log(&wss);
	//TODO: this!

	std::wstringstream ss = std::wstringstream();
	ss << luabind::call_function<int>(mLuaState, "add", 2, 3) << std::endl;
	mLogger->log(&ss);
}