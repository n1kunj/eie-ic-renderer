#ifndef RENDERER_H
#define RENDERER_H

namespace Renderer {
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnExit();
}

#endif