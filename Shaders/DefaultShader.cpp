#include "DXUT.h"
#include "DefaultShader.h"
#include "..\Utils\ShaderTools.h"

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};

D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const UINT numLayoutElements = ARRAYSIZE(vertexLayout);

boolean DefaultShader::sCompiled = false;

ID3D11VertexShader* DefaultShader::sVertexShader = NULL;
ID3D11PixelShader* DefaultShader::sPixelShader = NULL;
ID3D11InputLayout* DefaultShader::sVertexLayout = NULL;
ID3D11Buffer* DefaultShader::sConstantBuffer = NULL;

void DefaultShader::DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh)
{
	assert(sCompiled == TRUE);

	// Update constant buffer
	ConstantBuffer cb;
	cb.World = XMMatrixTranspose( pMesh->mWorldViewMatrix );
	cb.View = XMMatrixTranspose( pMesh->mViewMatrix );
	cb.Projection = XMMatrixTranspose(pMesh->mProjectionMatrix );

	pd3dContext->UpdateSubresource( sConstantBuffer, 0, NULL, &cb, 0, 0 );
	pd3dContext->VSSetConstantBuffers( 0, 1, &sConstantBuffer );

	//Set vertex layout and bind buffers
	pd3dContext->IASetInputLayout( sVertexLayout );

	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	pd3dContext->IASetVertexBuffers( 0, 1, &pMesh->mVertexBuffer, &stride, &offset );

	pd3dContext->IASetIndexBuffer( pMesh->mIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//Set shaders

	pd3dContext->VSSetShader( sVertexShader, NULL, 0 );
	pd3dContext->PSSetShader( sPixelShader, NULL, 0 );

	//Draw
	//36 indices
	pd3dContext->DrawIndexed( 36, 0, 0 );

}

HRESULT DefaultShader::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	if (sCompiled) {
		return S_OK;
	}
	//Make sure everything is clean before we start
	OnD3D11DestroyDevice();

	HRESULT hr;

	//Compile VS
	ID3DBlob* pVSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DefaultShader.fx", "VS", "vs_4_0", &pVSBlob ));

	//Create the vertex shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &sVertexShader ));

	//Create the input layout
	V_RELEASE_AND_RETURN(pVSBlob,pd3dDevice->CreateInputLayout( vertexLayout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &sVertexLayout ));

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DefaultShader.fx", "PS", "ps_4_0", &pPSBlob ));

	// Create the pixel shader
	V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &sPixelShader ));

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	V_RETURN(hr = pd3dDevice->CreateBuffer( &bd, NULL, &sConstantBuffer ));

	sCompiled = TRUE;
	return S_OK;
}

void DefaultShader::OnD3D11DestroyDevice() {
	SAFE_RELEASE(sVertexShader);
	SAFE_RELEASE(sPixelShader);
	SAFE_RELEASE(sVertexLayout);
	SAFE_RELEASE(sConstantBuffer);
	sCompiled = FALSE;
}

DefaultShader::DefaultShader()
{

}

DefaultShader::~DefaultShader()
{

}