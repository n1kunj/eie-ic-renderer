#include "DXUT.h"
#include "DefaultShader.h"
#include "..\Utils\ShaderTools.h"

struct Vertex
{
	XMFLOAT3 POSITION;
	XMFLOAT4 COLOR;
};

D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

UINT numLayoutElements = ARRAYSIZE(vertexLayout);

struct ConstantBuffer
{
	FLOAT Time;
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};

boolean DefaultShader::sCompiled = false;

ID3D11VertexShader* DefaultShader::sVertexShader = NULL;
ID3D11PixelShader* DefaultShader::sPixelShader = NULL;
ID3D11InputLayout* DefaultShader::sVertexLayout = NULL;
ID3D11Buffer* DefaultShader::sConstantBuffer = NULL;

void DefaultShader::DrawMesh( const BaseMesh* pMesh )
{
	assert(sCompiled == TRUE);

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