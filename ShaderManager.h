#include "DXUT.h"
#include <vector>
#include "DrawableShader.h"
#include "MessageProcessor.h"

class ShaderManager{
private:
	std::vector<DrawableShader*> mDrawableShaders;
	MessageLogger* mLogger;
public:
	ShaderManager(MessageLogger* pLogger) : mLogger(pLogger) {
	}

	//Memory now belongs to the shader manager. It'll delete as required.
	void addShader(DrawableShader* pShader) {
		mDrawableShaders.push_back(pShader);
	}
	DrawableShader* getDrawableShader(std::string pShaderName ) {
		std::wstring ws(pShaderName.begin(),pShaderName.end());
		for (int i = 0; i < mDrawableShaders.size(); i++) {
			if (mDrawableShaders[i]->mShaderHandle == ws) {
				return mDrawableShaders[i];
			}
		}
		std::stringstream ss;
		ss << "Drawable Shader " << ws.c_str() << " Does not exist!"; 
		throw std::exception(ss.str().c_str());
	}

	void OnD3D11CreateDevice( ID3D11Device* pd3dDevice ) 
	{
		for (int i = 0; i < mDrawableShaders.size(); i++) {
			mDrawableShaders[i]->OnD3D11CreateDevice(pd3dDevice);
		}
	}

	void OnD3D11DestroyDevice() 
	{
		for (int i = 0; i < mDrawableShaders.size(); i++) {
			mDrawableShaders[i]->OnD3D11DestroyDevice();
		}
	}

	~ShaderManager() {
		for (int i = 0; i < mDrawableShaders.size(); i++) {
			delete mDrawableShaders[i];
		}
	}
};