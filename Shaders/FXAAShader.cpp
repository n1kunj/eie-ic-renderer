#include "DXUT.h"
#include "FXAAShader.h"
#include "../Utils/ShaderTools.h"
#include "../DirectXMath/DirectXMath.h"

using namespace DirectX;

struct FXAAConstantBuffer
{
	XMVECTOR FXAA;
};

boolean FXAAShader::sCompiled = false;

ID3D11VertexShader*			FXAAShader::sVertexShader = NULL;
ID3D11PixelShader*			FXAAShader::sPixelShader = NULL;
ID3D11Buffer*				FXAAShader::sConstantBuffer = NULL;

void FXAAShader::DrawPost(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pInputSRV)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(sConstantBuffer,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	FXAAConstantBuffer* pFXAA = (FXAAConstantBuffer*)MappedResource.pData;
	float frameWidth = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
	float frameHeight = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height;

	pFXAA->FXAA = XMVectorSet(1.0f/frameWidth, 1.0f/frameHeight, 0.0f, 0.0f);
	pd3dContext->Unmap( sConstantBuffer, 0 );
	pd3dContext->VSSetConstantBuffers( 1, 1, &sConstantBuffer );
	pd3dContext->PSSetConstantBuffers( 1, 1, &sConstantBuffer );

	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	pd3dContext->VSSetShader( sVertexShader, NULL, 0 );
	pd3dContext->PSSetShader( sPixelShader, NULL, 0 );

	pd3dContext->PSSetShaderResources( 0, 1, &pInputSRV );
	pd3dContext->Draw( 4, 0 );
}

HRESULT FXAAShader::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	OnD3D11DestroyDevice();
	HRESULT hr;

	//Compile VS
	ID3DBlob* pVSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\FXAAShader.hlsl", "FxaaVS", "vs_5_0", &pVSBlob ));

	//Create the vertex shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &sVertexShader ));

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\FXAAShader.hlsl", "FxaaPS", "ps_5_0", &pPSBlob ));

	V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &sPixelShader ));

	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory( &cbDesc, sizeof(cbDesc) );
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof( FXAAConstantBuffer );

	V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &sConstantBuffer ) );

	sCompiled = TRUE;
	return S_OK;
}

void FXAAShader::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(sVertexShader);
	SAFE_RELEASE(sPixelShader);
	SAFE_RELEASE(sConstantBuffer);
	sCompiled = FALSE;
}
