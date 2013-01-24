#include "DXUT.h"
#include "MessageProcessor.h"
#include "Renderer.h"
#include "Drawable.h"
#include "Camera.h"
#include "DrawableShader.h"
#include "DrawableMesh.h"
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
	mLogger(logger), mRenderer(renderer),
	mLogColourR(1.0f),mLogColourG(0.0f),mLogColourB(1.0f),
	mErrorColourR(1.0f),mErrorColourG(0.0f),mErrorColourB(0.0f)
{
	//Dynamically allocates memory within this function
	mLuaState = luaL_newstate();

	open(mLuaState);

	// Add our function to the state's global scope
	module(mLuaState) [
		class_<RendererMessageProcessor>("RendererMessageProcessor")
		.def("luaLog", (void(RendererMessageProcessor::*)(string))&RendererMessageProcessor::luaLog)
		.def("luaLog", (void(RendererMessageProcessor::*)(float))&RendererMessageProcessor::luaLog)
		.def("runScript", &RendererMessageProcessor::runScript),
		class_<Camera>("Camera"),
		class_<DrawableShader>("DrawableShader"),
		class_<DrawableMesh>("DrawableMesh"),
		class_<Drawable, Drawable*>("Drawable"),
		class_<BasicDrawable,bases<Drawable>,BasicDrawable*>("BasicDrawable")
		.def(constructor<DrawableMesh*,DrawableShader*,Camera*>()),
		class_<DrawableManager>("DrawableManager")
		.def("addDrawable",&DrawableManager::addDrawable)
	];
	runScript("setup.lua");
	//luaL_dofile(mLuaState,"Media/Lua/setup.lua");

	call_function<void>(mLuaState,"setRMP",this);
	call_function<void>(mLuaState,"setCamera",boost::ref(mRenderer->mCamera));
	call_function<void>(mLuaState,"setShader",boost::ref(mRenderer->mDefaultShader));
	call_function<void>(mLuaState,"setMesh",boost::ref(mRenderer->mCubeMesh));
	call_function<void>(mLuaState,"setDrawMan",boost::ref(mRenderer->mDrawableManager));

	runScript("cube.lua");

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

	if (luaL_dostring(mLuaState,s.c_str()) == 1) {
		luaError();	
	}
}

void RendererMessageProcessor::luaLog(std::string s) {
	wstring ws(s.begin(), s.end());
	wstringstream ss = wstringstream(ws);
	mLogger->log(&ss,mLogColourR,mLogColourG,mLogColourB);
}

void RendererMessageProcessor::luaLog(float num) {
	wstringstream ss = wstringstream();
	ss << num;
	mLogger->log(&ss,mLogColourR,mLogColourG,mLogColourB);
}

void RendererMessageProcessor::luaError()
{
	string s = lua_tostring(mLuaState,-1);
	wstring ws(s.begin(), s.end());
	wstringstream ss(ws);
	mLogger->log(&ss,mErrorColourR,mErrorColourG,mErrorColourB);
	lua_pop(mLuaState, 1);
}

void RendererMessageProcessor::runScript( string s )
{
	if (luaL_dofile(mLuaState,("Media/Lua/" + s).c_str()) == 1) {
		luaError();	
	}
}
