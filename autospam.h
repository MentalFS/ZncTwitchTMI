#pragma once

#include <znc/Modules.h>

#include <unordered_set>

class AutoSpam : public CModule
{
	public:
	MODCONSTRUCTOR(AutoSpam) { RegisterCommands(); lastSpam = 0; }
	virtual ~AutoSpam();

	void AddPhrase(const CString& cmd);
	void RemovePhrase(const CString& cmd);

	bool OnLoad(const CString &args, CString &message) override;
	CModule::EModRet OnChanTextMessage(CTextMessage &message) override;

	private:
	void RegisterCommands();
	void Save();

	private:
	std::unordered_set<CString> phrases;
	std::time_t lastSpam;
};

class GenericJob : public CJob
{
	public:
	GenericJob(std::function<void()> threadFunc, std::function<void()> mainFunc):threadFunc(threadFunc),mainFunc(mainFunc) {}

	void runThread() override { threadFunc(); }
	void runMain() override { mainFunc(); }

	private:
	std::function<void()> threadFunc;
	std::function<void()> mainFunc;
};

