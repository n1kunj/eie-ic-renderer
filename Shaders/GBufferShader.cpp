#include "DXUT.h"
#include "GBufferShader.h"
#include "..\DrawableMesh.h"
#include "..\DrawableState.h"
#include "..\Camera.h"
#include "..\Utils\ShaderTools.h"

using namespace DirectX;

__declspec(align(16)) struct VSConstantBuffer
{
	XMMATRIX Model;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX MV;
	XMMATRIX MVP;
};

__declspec(align(16)) struct PSConstantBuffer
{
	XMFLOAT3 CameraPos;
};

boolean GBufferShader::sCompiled = false;

ID3D11VertexShader* GBufferShader::sVertexShader = NULL;
ID3D11PixelShader* GBufferShader::sPixelShader = NULL;
ID3D11InputLayout* GBufferShader::sVertexLayout = NULL;
ID3D11Buffer* GBufferShader::sVSConstantBuffer = NULL;
ID3D11Buffer* GBufferShader::sPSConstantBuffer = NULL;

void GBufferShader::DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera)
{
	assert(pMesh!= NULL);
	assert(pMesh->mInitialised == TRUE);
	assert(pState != NULL);
	assert(pCamera != NULL);

#ifdef DEBUG
	//Allow shaders to be recompiled in debug mode
	if (!sCompiled) {
		return;
	}
#endif // DEBUG

	// Update constant buffer
	VSConstantBuffer vscb;

	XMFLOAT3 lightPos = XMFLOAT3(5.0f + pState->mCamOffset.x,
		3.0f + pState->mCamOffset.y,
		2.0f + pState->mCamOffset.z);
	XMFLOAT3 DiffuseLightColour = XMFLOAT3(0.7f,0.7f,0.7f);
	XMFLOAT3 AmbientLightColour = XMFLOAT3(0.03f,0.03f,0.03f);

	vscb.Model = XMMatrixTranspose(pState->mModelMatrix);
	vscb.View = XMMatrixTranspose(pCamera->mViewMatrix);
	vscb.Projection = XMMatrixTranspose(pCamera->mProjectionMatrix);
	vscb.MV = XMMatrixMultiplyTranspose(pState->mModelMatrix,pCamera->mViewMatrix);
	vscb.MVP = XMMatrixMultiplyTranspose(pState->mModelMatrix,pCamera->mViewProjectionMatrix);

	pd3dContext->UpdateSubresource( sVSConstantBuffer, 0, NULL, &vscb, 0, 0 );
	pd3dContext->VSSetConstantBuffers( 0, 1, &sVSConstantBuffer );

	PSConstantBuffer pscb;

	XMStoreFloat3(&pscb.CameraPos, pCamera->mEye);

	pd3dContext->UpdateSubresource( sPSConstantBuffer, 0, NULL, &pscb, 0, 0 );
	pd3dContext->PSSetConstantBuffers( 1, 1, &sPSConstantBuffer );


	//Set vertex layout and bind buffers
	pd3dContext->IASetInputLayout( sVertexLayout );

	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	pd3dContext->IASetVertexBuffers( 0, 1, &pMesh->mVertexBuffer, &stride, &offset );

	pd3dContext->IASetIndexBuffer( pMesh->mIndexBuffer, pMesh->mIndexBufferFormat, 0 );
	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//Set shaders
	pd3dContext->VSSetShader( sVertexShader, NULL, 0 );
	pd3dContext->PSSetShader( sPixelShader, NULL, 0 );

	pd3dContext->DrawIndexed( pMesh->mNumIndices, 0, 0 );
}

HRESULT GBufferShader::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
#ifndef DEBUG
	if (sCompiled) {
		return S_OK;
	}
#endif

	//Make sure everything is clean before we start
	OnD3D11DestroyDevice();

	HRESULT hr;

	//Compile VS
	ID3DBlob* pVSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "VS", "vs_4_0", &pVSBlob ));

	//Create the vertex shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &sVertexShader ));

	//Create the input layout
	V_RELEASE_AND_RETURN(pVSBlob,pd3dDevice->CreateInputLayout( vertexLayout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &sVertexLayout ));

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GBufferShader.fx", "PS", "ps_4_0", &pPSBlob ));

	// Create the pixel shader
	V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &sPixelShader ));

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = sizeof(VSConstantBuffer);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &sVSConstantBuffer ));

	bd.ByteWidth = sizeof(PSConstantBuffer);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &sPSConstantBuffer ));

	sCompiled = TRUE;
	return S_OK;
}

void GBufferShader::OnD3D11DestroyDevice() {
	SAFE_RELEASE(sVertexShader);
	SAFE_RELEASE(sPixelShader);
	SAFE_RELEASE(sVertexLayout);
	SAFE_RELEASE(sVSConstantBuffer);
	SAFE_RELEASE(sPSConstantBuffer);
	sCompiled = FALSE;
}

GBufferShader::GBufferShader() : DrawableShader(L"GBufferShader")
{

}

GBufferShader::~GBufferShader()
{

}