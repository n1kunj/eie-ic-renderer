#pragma once
#ifndef SHADERS_FXAASHADER_H
#define SHADERS_FXAASHADER_H

class FXAAShader{
public:
	void FXAAShader::DrawPost(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pInputSRV, ID3D11RenderTargetView* pOutputRTV);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();

	FXAAShader(){}
	~FXAAShader(){
		OnD3D11DestroyDevice();
	}

private:
	static boolean sCompiled;
	//static ID3D11Texture2D*				mProxyTexture;
	//static ID3D11ShaderResourceView*	mProxyTextureSRV;
	//static ID3D11RenderTargetView*		mProxyTextureRTV;
	static ID3D11VertexShader* sVertexShader;
	static ID3D11PixelShader* sPixelShader;
	static ID3D11Buffer* sConstantBuffer;
};


#endif