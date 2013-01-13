#include "DXUT.h"
#include "DrawableShader.h"

DrawableShader::DrawableShader(const WCHAR* pShaderHandle) {
	size_t linelen = wcslen(pShaderHandle);
	mShaderHandle = new WCHAR[linelen+1];
	wcscpy_s(mShaderHandle,linelen+1,pShaderHandle);
};
DrawableShader::~DrawableShader() {
	SAFE_DELETE_ARRAY(mShaderHandle);
};