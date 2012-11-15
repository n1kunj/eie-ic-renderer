#include <string>

HRESULT DebugTextOnD3D11CreateDevice(ID3D11Device* pd3dDevice, int lineHeight);
HRESULT DebugTextOnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
void DebugTextOnD3D11ReleasingSwapChain();
void DebugTextOnD3D11DestroyDevice();

class DebugTextArray {
public:
	DebugTextArray(int size) : size(size), curString(0), saturated(false){
		textArray = new WCHAR*[size]();
	}
	~DebugTextArray() {
		delete[] textArray;
	}
	void addDebugLine(const WCHAR* line);
	void addDebugLine(int i);
	void render(int x, int y, int spacing);
private:
	int size;
	int curString;
	bool saturated;
	WCHAR** textArray;
};