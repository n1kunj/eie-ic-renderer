#pragma once
#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H
#include <sstream>
#include <string>

class Renderer;
class lua_State;

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
	virtual ~MessageProcessor() {};
};

class RendererMessageProcessor : public MessageProcessor {

public:
	RendererMessageProcessor(MessageLogger* logger, Renderer* renderer);
	~RendererMessageProcessor();

	void processMessage(WCHAR* pInput);

private:
	MessageLogger* mLogger;
	Renderer* mRenderer;
	lua_State* mLuaState;
};



#endif