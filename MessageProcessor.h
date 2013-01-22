#pragma once
#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H
#include <sstream>
#include <string>

class Renderer;

class MessageLogger {
public:
	virtual void log(const WCHAR* line) = 0;

	void log(std::wstringstream* wss) {
		log(wss->str().c_str());
	}
};

class MessageProcessor {
public:
	virtual void processMessage(WCHAR* pInput) = 0;
};

class RendererMessageProcessor : public MessageProcessor {

public:
	RendererMessageProcessor(MessageLogger* logger, Renderer* renderer);
	void processMessage(WCHAR* pInput);

private:
	MessageLogger* mLogger;
	Renderer* mRenderer;
};



#endif