#pragma once
#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H
#include <sstream>
#include <string>

class Renderer;
struct lua_State;

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
	//RendererMessageProcessor();
	RendererMessageProcessor(MessageLogger* logger, Renderer* renderer);
	~RendererMessageProcessor();

	void processMessage(WCHAR* pInput);

private:
	void luaLog(std::string s);
	void luaLog(float num);
	MessageLogger* mLogger;
	Renderer* mRenderer;
	lua_State* mLuaState;
};



#endif