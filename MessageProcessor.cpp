#include "DXUT.h"
#include "MessageProcessor.h"
#include "Renderer.h"
#include <string>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

using namespace luabind;
using namespace std;

RendererMessageProcessor::RendererMessageProcessor( MessageLogger* logger, Renderer* renderer ) :
	mLogger(logger), mRenderer(renderer)
{
	//Dynamically allocates within this function
	mLuaState = luaL_newstate();

	open(mLuaState);

	luaL_dostring(
		mLuaState,
		"RMP = 0 \n"
		"function setRMP(pRMP)\n"
		"RMP = pRMP\n"
		"end\n"
		);

	luaL_dostring(
		mLuaState,
		"function print(X)\n"
		"RMP:luaLog(X)\n"
		"end\n"
		);

	// Add our function to the state's global scope
	module(mLuaState) [
		class_<RendererMessageProcessor>("RendererMessageProcessor")
		.def("luaLog", (void(RendererMessageProcessor::*)(string))&RendererMessageProcessor::luaLog)
		.def("luaLog", (void(RendererMessageProcessor::*)(float))&RendererMessageProcessor::luaLog)
	];

	call_function<void>(mLuaState,"setRMP",this);

}

RendererMessageProcessor::~RendererMessageProcessor()
{
	//Deallocates within the function
	lua_close(mLuaState);
}

void RendererMessageProcessor::processMessage(WCHAR* pInput)
{
	//TODO: this!

	//TODO: make this not so horribly inefficient!
	wstring ws = wstring(pInput);
	string s(ws.begin(),ws.end());
	s.append("\n");

	luaL_dostring(mLuaState,s.c_str());
}

void RendererMessageProcessor::luaLog(std::string s) {
	wstring ws(s.begin(), s.end());
	wstringstream ss = wstringstream(ws);
	mLogger->log(&ss);
}

void RendererMessageProcessor::luaLog(float num) {
	wstringstream ss = wstringstream();
	ss << num;
	mLogger->log(&ss);
}