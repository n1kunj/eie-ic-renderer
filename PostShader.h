#pragma once
#ifndef POSTSHADER_H
#define POSTSHADER_H
#include <string>

class PostShader {
public:
	std::wstring mShaderHandle;
	virtual void DrawPost() = 0;
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) = 0;
	virtual void OnD3D11DestroyDevice() = 0;

	PostShader(const WCHAR* pShaderHandle) {
		mShaderHandle = std::wstring(pShaderHandle);
	};
	virtual ~PostShader() {
	};

private:
	PostShader(const PostShader& copy);
};


#endif