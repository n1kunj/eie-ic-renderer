#pragma once
#ifndef SHADERS_GBUFFERSHADER_H
#define SHADERS_GBUFFERSHADER_H
#include "DXUT.h"
#include "../DrawableShader.h"
#include "../DirectXMath/DirectXMath.h"
#include "../DrawableMesh.h"
#include "../DrawableState.h"
#include "../Camera.h"
#include "../Utils/ShaderTools.h"
#include "../Procedural/Generator.h"

__declspec(align(16)) struct GBufferVSCB
{
	DirectX::XMMATRIX Model;
	DirectX::XMMATRIX MV;
	DirectX::XMMATRIX VP;
	DirectX::XMINT3 Offset;
};

__declspec(align(16)) struct GBufferPSCB
{
	DirectX::XMFLOAT3 Albedo;
	FLOAT SpecPower;
	FLOAT SpecAmount;
	DirectX::XMFLOAT3 padding0;
};

class GBufferShader : public DrawableShader {
private:
	boolean mCompiled;
	ID3D11VertexShader* mVertexShader;
	ID3D11InputLayout* mVertexLayout;
	ID3D11VertexShader* mInstancedVS;
	ID3D11InputLayout* mInstancedVL;
	ID3D11PixelShader* mPixelShader;
	ID3D11PixelShader* mInstancedPS;
	ID3D11Buffer* mVSConstantBuffer;
	ID3D11Buffer* mPSConstantBuffer;

public:
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mVertexShader);
		SAFE_RELEASE(mInstancedVS);
		SAFE_RELEASE(mInstancedVL);
		SAFE_RELEASE(mPixelShader);
		SAFE_RELEASE(mInstancedPS);
		SAFE_RELEASE(mVertexLayout);
		SAFE_RELEASE(mVSConstantBuffer);
		SAFE_RELEASE(mPSConstantBuffer);
		mCompiled = FALSE;
	}

	GBufferShader() : DrawableShader(L"GBufferShader"),mCompiled(FALSE),mVertexLayout(NULL),mInstancedVS(NULL),mInstancedVL(NULL),
		mVertexShader(NULL),mPixelShader(NULL),mVSConstantBuffer(NULL),mPSConstantBuffer(NULL),mInstancedPS(NULL) {}

	GBufferShader::~GBufferShader()
	{
		OnD3D11DestroyDevice();
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) {
		//Make sure everything is clean before we start
		OnD3D11DestroyDevice();

		HRESULT hr;

		//Compile VS
		{
			ID3DBlob* pVSBlob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "VS", "vs_5_0", &pVSBlob ));

			//Create the vertex shader
			//If fails, releases pVSBlob.
			V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mVertexShader ));

			//Create the input layout
			V_RELEASE_AND_RETURN(pVSBlob,pd3dDevice->CreateInputLayout( VertexLayout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &mVertexLayout ));
		}

		//Compile Instanced VS
		{
			ID3DBlob* pVSBlob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "VS_INSTANCED", "vs_5_0", &pVSBlob ));

			//Create the vertex shader
			//If fails, releases pVSBlob.
			V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mInstancedVS ));

			//Create the input layout
			V_RELEASE_AND_RETURN(pVSBlob,pd3dDevice->CreateInputLayout( VertexLayout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &mInstancedVL ));
		}

		{
			// Compile the pixel shader
			ID3DBlob* pPSBlob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "PS", "ps_5_0", &pPSBlob ));

			// Create the pixel shader
			V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &mPixelShader ));
		}

		{
			// Compile the pixel shader
			ID3DBlob* pPSBlob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "PS_INSTANCED", "ps_5_0", &pPSBlob ));

			// Create the pixel shader
			V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &mInstancedPS ));
		}

		{
			// Create the constant buffer
			D3D11_BUFFER_DESC bd;
			ZeroMemory( &bd, sizeof(bd) );
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.ByteWidth = sizeof(GBufferVSCB);
			V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mVSConstantBuffer ));

			bd.ByteWidth = sizeof(GBufferPSCB);
			V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mPSConstantBuffer ));
		}


		mCompiled = TRUE;
		return S_OK;
	}

	void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) {
		assert(pMesh!= NULL);
		assert(pMesh->mInitialised == TRUE);
		assert(pState != NULL);
		assert(pCamera != NULL);

		if (!mCompiled) {
			return;
		}

		setupShader(pd3dContext,pMesh,pState,pCamera);
		pd3dContext->PSSetShader( mPixelShader, NULL, 0 );
		pd3dContext->IASetInputLayout( mVertexLayout );
		pd3dContext->VSSetShader( mVertexShader, NULL, 0 );

		pd3dContext->DrawIndexed( pMesh->mNumIndices, 0, 0 );

		cleanupShader(pd3dContext);
	}

	void DrawInstancedIndirect(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) {

		assert(pState->mCityTile != NULL);

		setupShader(pd3dContext,pMesh,pState,pCamera);

		pd3dContext->IASetInputLayout( mInstancedVL );
		pd3dContext->VSSetShader( mInstancedVS, NULL, 0 );
		pd3dContext->PSSetShader(mInstancedPS,NULL,0);
		CityTile& ct = *pState->mCityTile;
		pd3dContext->VSSetShaderResources(0,1,&ct.mInstanceBuffer.mSRV);

		pd3dContext->DrawIndexedInstancedIndirect(ct.mIndirectBuffer.mBuffer,0);

	}
private:
	void setupShader(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) {

		using namespace DirectX;

		// Update constant buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dContext->Map(mVSConstantBuffer,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		GBufferVSCB* vscb = (GBufferVSCB*)MappedResource.pData;
		vscb->Model = XMMatrixTranspose(pState->mModelMatrix);
		vscb->MV = XMMatrixMultiplyTranspose(pState->mModelMatrix,pCamera->mViewMatrix);
		vscb->VP = XMMatrixTranspose(pCamera->mViewProjectionMatrix);
		const XMINT3& mCrds = pState->mCoords;
		const XMINT3& cCrds = pCamera->mCoords;
		vscb->Offset = XMINT3(mCrds.x - cCrds.x,
			mCrds.y - cCrds.y,
			mCrds.z - cCrds.z);
		pd3dContext->Unmap(mVSConstantBuffer,0);

		pd3dContext->VSSetConstantBuffers( 0, 1, &mVSConstantBuffer );

		pd3dContext->Map(mPSConstantBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&MappedResource);
		GBufferPSCB* pscb = (GBufferPSCB*)MappedResource.pData;
		pscb->Albedo = XMFLOAT3(pState->mDiffuseColour);
		pscb->SpecPower = pState->mSpecularExponent;
		pscb->SpecAmount = pState->mSpecularAmount;
		pd3dContext->Unmap(mPSConstantBuffer,0);

		pd3dContext->PSSetConstantBuffers( 0, 1, &mPSConstantBuffer );

		//Set vertex layout and bind buffers
		UINT offset = 0;
		pd3dContext->IASetVertexBuffers( 0, 1, &pMesh->mVertexBuffer, &VertexDataStride, &offset );

		pd3dContext->IASetIndexBuffer( pMesh->mIndexBuffer, pMesh->mIndexBufferFormat, 0 );
		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}

	void cleanupShader(ID3D11DeviceContext* pd3dContext) {
		pd3dContext->VSSetShader(NULL,NULL,0);
		pd3dContext->PSSetShader(NULL,NULL,0);
	}
};

#endif