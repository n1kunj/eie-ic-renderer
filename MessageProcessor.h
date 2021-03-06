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
	virtual void log(const WCHAR* line, FLOAT r, FLOAT g, FLOAT b) = 0;

	void log(std::wstringstream* wss) {
		log(wss->str().c_str());
	}

	void log(std::wstringstream* wss, FLOAT r, FLOAT g, FLOAT b) {
		log(wss->str().c_str(),r,g,b);
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
	void luaLog(std::string s);
	void luaLog(float num);
	void runScript(std::string s);
	void luaError();
	MessageLogger* mLogger;
	Renderer* mRenderer;
	lua_State* mLuaState;
	FLOAT mLogColourR;
	FLOAT mLogColourG;
	FLOAT mLogColourB;
	FLOAT mErrorColourR;
	FLOAT mErrorColourG;
	FLOAT mErrorColourB;
};



#endif