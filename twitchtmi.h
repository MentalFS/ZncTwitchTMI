#pragma once

#include <znc/Modules.h>
#include <znc/Threads.h>

#include <unordered_set>
#include <unordered_map>
#include <ctime>

class TwitchTMIUpdateTimer;

class TwitchTMI : public CModule
{
	friend class TwitchTMIUpdateTimer;
	friend class TwitchTMIJob;

	public:
	MODCONSTRUCTOR(TwitchTMI) { }
	virtual ~TwitchTMI();

	bool OnLoad(const CString &sArgsi, CString &sMessage) override;
	bool OnBoot() override;

	void OnIRCConnected() override;

	CModule::EModRet OnUserRaw(CString &sLine) override;
	CModule::EModRet OnRawMessage(CMessage &msg) override;
	CModule::EModRet OnUserJoin(CString &sChannel, CString &sKey) override;
	CModule::EModRet OnUserTextMessage(CTextMessage &msg) override;
	bool OnServerCapAvailable(const CString &sCap) override;

	private:
	void PutModChanNotice(CChan *chan, const CString &text);

	private:
	TwitchTMIUpdateTimer *timer;
	std::unordered_set<CString> liveChannels;
};

class TwitchTMIUpdateTimer : public CTimer
{
	friend class TwitchTMI;

	public:
	TwitchTMIUpdateTimer(TwitchTMI *mod);

	private:
	void RunJob() override;

	private:
	TwitchTMI *mod;
};

class TwitchTMIJob : public CJob
{
	public:
	TwitchTMIJob(TwitchTMI *mod);

	void runThread() override;
	void runMain() override;

	private:
	TwitchTMI *mod;
	std::unordered_set<CString> channels;
	std::unordered_map<CString,CString> titles;
	std::unordered_map<CString,bool> live;
};

