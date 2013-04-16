#include "DXUT.h"
#include "Generator.h"
#include "../Utils/ShaderTools.h"

#define HEIGHT_MAP_RESOLUTION 512
#define CS_GROUP_DIM 16
#define DISPATCH_DIM HEIGHT_MAP_RESOLUTION/CS_GROUP_DIM

__declspec(align(16)) struct HeightMapCSCB {
	DirectX::XMUINT2 bufferDim;
	DirectX::XMINT2 coords;
};

DistantTextures::DistantTextures(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize) {
	mPosX = pPosX;
	mPosY = pPosY;
	mPosZ = pPosZ;
	mSize = pSize;

	mColour = DirectX::XMFLOAT3(1.0f,0.0f,0.0f);

	D3D11_TEXTURE2D_DESC& desc = mAlbedoMap.mDesc;
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
		first->mAlbedoMap.CreateTexture(pd3dDevice);
		ComputeTextures(pd3dContext, *first);
	}
	mTextureQueue.pop_front();
}

void Generator::ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTextures &pDT )
{
	if (!mCompiled) {
		return;
	}
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mCSCB,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	HeightMapCSCB* cscb = (HeightMapCSCB*)MappedResource.pData;
	cscb->bufferDim = DirectX::XMUINT2(HEIGHT_MAP_RESOLUTION, HEIGHT_MAP_RESOLUTION);
	cscb->coords = DirectX::XMINT2((INT)pDT.mPosX,(INT)pDT.mPosZ);

	pd3dContext->Unmap(mCSCB,0);

	pd3dContext->CSSetConstantBuffers(0,1,&mCSCB);


	pd3dContext->CSSetUnorderedAccessViews(0,1,&pDT.mAlbedoMap.mUAV,0);
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
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GeneratorShader.fx", "CSPass1", "cs_5_0", &pCSBlob ));

	//Create the compute shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pCSBlob,pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &mCS ));

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(HeightMapCSCB);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mCSCB ));

	mCompiled = TRUE;
	return S_OK;
}
