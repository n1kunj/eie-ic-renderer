#include "DXUT.h"
#include "Generator.h"
#include "../Utils/ShaderTools.h"

#define ALBNORM_MAP_RESOLUTION 256
#define HEIGHT_MAP_RESOLUTION 256
#define CS_GROUP_DIM 16
#define DISPATCH_DIM ALBNORM_MAP_RESOLUTION/CS_GROUP_DIM

__declspec(align(16)) struct HeightMapCSCB {
	DirectX::XMUINT2 bufferDim;
	DirectX::XMINT2 coords;
};

DistantTextures::DistantTextures(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize) {
	mPosX = pPosX;
	mPosY = pPosY;
	mPosZ = pPosZ;
	mSize = pSize;

	D3D11_TEXTURE2D_DESC desc;
	::ZeroMemory (&desc, sizeof (desc));
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.Height = ALBNORM_MAP_RESOLUTION;
	desc.Width = ALBNORM_MAP_RESOLUTION;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	mAlbedoMap.mDesc = desc;

	desc.Format = DXGI_FORMAT_R32G32_FLOAT;
	mNormalMap.mDesc = desc;

	desc.Format = DXGI_FORMAT_R16_FLOAT;
	mHeightMap.mDesc = desc;
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
		first->mAlbedoMap.CreateTexture(pd3dDevice);
		first->mNormalMap.CreateTexture(pd3dDevice);
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
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mCSCB,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	HeightMapCSCB* cscb = (HeightMapCSCB*)MappedResource.pData;
	cscb->bufferDim = DirectX::XMUINT2(ALBNORM_MAP_RESOLUTION, ALBNORM_MAP_RESOLUTION);
	cscb->coords = DirectX::XMINT2((INT)pDT.mPosX,(INT)pDT.mPosZ);

	pd3dContext->Unmap(mCSCB,0);

	pd3dContext->CSSetConstantBuffers(0,1,&mCSCB);

	ID3D11UnorderedAccessView* uavs[3] = {pDT.mAlbedoMap.mUAV,
		pDT.mNormalMap.mUAV,pDT.mHeightMap.mUAV};

	pd3dContext->CSSetUnorderedAccessViews(0,3,uavs,0);

	pd3dContext->CSSetShader(mCS,0,0);
	pd3dContext->Dispatch(DISPATCH_DIM,DISPATCH_DIM,1);

	ID3D11UnorderedAccessView* nulluavs[3] = {NULL,NULL,NULL};
	pd3dContext->CSSetUnorderedAccessViews(0,3,nulluavs,0);
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
