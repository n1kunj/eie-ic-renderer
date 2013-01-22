#include "DXUT.h"
#include "MessageProcessor.h"
#include "Renderer.h"

RendererMessageProcessor::RendererMessageProcessor( MessageLogger* logger, Renderer* renderer ) :
	mLogger(logger), mRenderer(renderer)
{

}

void RendererMessageProcessor::processMessage(WCHAR* pInput)
{
	std::wstringstream wss;
	wss << L"Processing!" << pInput;
	mLogger->log(&wss);
	//TODO: this!
}
