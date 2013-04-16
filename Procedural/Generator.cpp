#include "DXUT.h"
#include "Generator.h"
#include "../Utils/ShaderTools.h"

#define HEIGHT_MAP_RESOLUTION 256
#define CS_GROUP_DIM 16
#define DISPATCH_DIM HEIGHT_MAP_RESOLUTION/CS_GROUP_DIM

DistantTextures::DistantTextures(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize) {
	mPosX = pPosX;
	mPosY = pPosY;
	mPosZ = pPosZ;
	mSize = pSize;

	mColour = DirectX::XMFLOAT3(1.0f,0.0f,0.0f);

	D3D11_TEXTURE2D_DESC& desc = mHeightMap.mDesc;
	::ZeroMemory (&desc, sizeof (desc));
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.Height = HEIGHT_MAP_RESOLUTION;
	desc.Width = HEIGHT_MAP_RESOLUTION;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
}

void Generator::Generate(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, UINT pMaxRuntimeMillis) {

	if (mTextureQueue.size() == 0) {
		return;
	}

	DTPTR &first = mTextureQueue.front();

	if (first.unique()) {
		first.reset();
	}
	else {
		first->mColour = DirectX::XMFLOAT3(0.0f,1.0f,0.0f);
		first->mHeightMap.CreateTexture(pd3dDevice);
		ComputeTextures(pd3dContext, *first);
	}
	mTextureQueue.pop_front();
}

void Generator::ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTextures &pDT )
{
	if (!mCompiled) {
		return;
	}
	pd3dContext->CSSetUnorderedAccessViews(0,1,&pDT.mHeightMap.mUAV,0);
	pd3dContext->CSSetShader(mCS,0,0);
	pd3dContext->Dispatch(DISPATCH_DIM,DISPATCH_DIM,1);

	ID3D11UnorderedAccessView* uavs[1] = {NULL};
	pd3dContext->CSSetUnorderedAccessViews(0,1,uavs,0);
}

HRESULT Generator::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	OnD3D11DestroyDevice();
	HRESULT hr;

	//Compile CS
	ID3DBlob* pCSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GeneratorShader.fx", "HeightMapCS", "cs_5_0", &pCSBlob ));

	//Create the compute shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pCSBlob,pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &mCS ));

	//D3D11_BUFFER_DESC bd;
	//ZeroMemory( &bd, sizeof(bd) );
	//bd.Usage = D3D11_USAGE_DYNAMIC;
	//bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//bd.ByteWidth = sizeof(LightingCSCB);
	//V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mCSCB ));

	mCompiled = TRUE;
	return S_OK;
}
