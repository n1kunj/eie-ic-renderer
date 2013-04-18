#include "DXUT.h"
#include "MessageProcessor.h"
#include "Renderer.h"
#include "Drawable.h"
#include "Camera.h"
#include "DrawableShader.h"
#include "DrawableMesh.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include <string>
#include "DirectXMath\DirectXMath.h"
#include <boost/shared_ptr.hpp>
#include "Procedural/DistantDrawable.h"
#include "Procedural/Generator.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

using namespace luabind;
using namespace std;
using namespace DirectX;

RendererMessageProcessor::RendererMessageProcessor( MessageLogger* logger, Renderer* renderer ) :
	mLogger(logger), mRenderer(renderer),
	mLogColourR(1.0f),mLogColourG(0.0f),mLogColourB(1.0f),
	mErrorColourR(1.0f),mErrorColourG(0.0f),mErrorColourB(0.0f)
{
	//Dynamically allocates memory within this function
	mLuaState = luaL_newstate();

	open(mLuaState);

	luaL_openlibs(mLuaState);

	// Add our function to the state's global scope
	module(mLuaState) [
		class_<RendererMessageProcessor>("RendererMessageProcessor")
		.def("luaLog", (void(RendererMessageProcessor::*)(string))&RendererMessageProcessor::luaLog)
		.def("luaLog", (void(RendererMessageProcessor::*)(float))&RendererMessageProcessor::luaLog)
		.def("runScript", &RendererMessageProcessor::runScript),
		class_<Camera>("Camera")
		.def("setEye", &Camera::setEye),
		class_<DrawableShader>("DrawableShader"),
		class_<DrawableMesh>("DrawableMesh"),

		class_<XMFLOAT3>("XMFLOAT3")
		.def_readwrite("x", &XMFLOAT3::x)
		.def_readwrite("y", &XMFLOAT3::y)
		.def_readwrite("z", &XMFLOAT3::z),

		class_<RendererSettings>("RendererSettings")
		.def_readwrite("wireframe", &RendererSettings::wireframe),

		class_<DrawableState>("DrawableState")
		.def("setPosition", &DrawableState::setPosition)
		.def_readwrite("mScale", &DrawableState::mScale)
		.def_readwrite("mRotation", &DrawableState::mRotation)
		.def_readwrite("mDiffuseColour", &DrawableState::mDiffuseColour)
		.def_readwrite("mAmbientColour", &DrawableState::mAmbientColour)
		.def_readwrite("mSpecularColour", &DrawableState::mSpecularColour)
		.def_readwrite("mSpecularExponent", &DrawableState::mSpecularExponent)
		.def_readwrite("mSpecularAmount", &DrawableState::mSpecularAmount),

		class_<Drawable, boost::shared_ptr<Drawable> >("Drawable"),

		class_<BasicDrawable,bases<Drawable>,boost::shared_ptr<Drawable> >("BasicDrawable")
		.def(constructor<DrawableMesh*,DrawableShader*,Camera*>())
		.def_readwrite("mState", &BasicDrawable::mState),

		class_<DistantDrawable,bases<Drawable>,boost::shared_ptr<Drawable> > ("DistantDrawable")
		.def(constructor<Camera*,ShaderManager*,MeshManager*,Generator*,UINT,DOUBLE, DOUBLE,DOUBLE>()),

		class_<DrawableManager>("DrawableManager")
		.def("addDrawable",&DrawableManager::addDrawable)
		.def("reset",&DrawableManager::reset),

		class_<ShaderManager>("ShaderManager")
		.def("getDrawableShader",&ShaderManager::getDrawableShader),

		class_<MeshManager>("MeshManager")
		.def("getDrawableMesh",&MeshManager::getDrawableMesh),

		class_<Generator>("Generator")
	];
	runScript("setup.lua");

	call_function<void>(mLuaState,"setRMP",this);
	call_function<void>(mLuaState,"setCamera",boost::ref(mRenderer->mCamera));
	call_function<void>(mLuaState,"setShaderMan",boost::ref(mRenderer->mShaderManager));
	call_function<void>(mLuaState,"setMeshMan",boost::ref(mRenderer->mMeshManager));
	call_function<void>(mLuaState,"setDrawMan",boost::ref(mRenderer->mDrawableManager));
	call_function<void>(mLuaState,"setRendererSettings",boost::ref(mRenderer->mSettings));
	call_function<void>(mLuaState,"setGenerator",boost::ref(mRenderer->mGenerator));

	runScript("cube.lua");

}

RendererMessageProcessor::~RendererMessageProcessor()
{
	//Deallocates within the function
	lua_close(mLuaState);
}

void RendererMessageProcessor::processMessage(WCHAR* pInput)
{
	//TODO: make this not so horribly inefficient!
	wstring ws = wstring(pInput);
	string s(ws.begin(),ws.end());
	s.append("\n");

	if (luaL_dostring(mLuaState,s.c_str()) == 1) {
		luaError();	
	}
	call_function<void>(mLuaState,"collectgarbage");
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
