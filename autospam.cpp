#include <znc/main.h>
#include <znc/IRCNetwork.h>
#include <znc/Modules.h>
#include <znc/Chan.h>
#include <znc/version.h>

#include "autospam.h"

#if (VERSION_MAJOR < 1) || (VERSION_MAJOR == 1 && VERSION_MINOR < 7)
#error This module needs at least ZNC 1.7.0 or later.
#endif

AutoSpam::~AutoSpam()
{

}

void AutoSpam::RegisterCommands()
{
	AddHelpCommand();
	AddCommand("Spam", static_cast<CModCommand::ModCmdFunc>(&AutoSpam::AddPhrase),
                   "<phrase>",
                   "Adds phrase to be spammed in all channels");
	AddCommand("Stop", static_cast<CModCommand::ModCmdFunc>(&AutoSpam::RemovePhrase),
                   "<phrase>",
                   "Removes phrase from the to-spam list");
}

CModule::EModRet AutoSpam::OnChanTextMessage(CTextMessage &message)
{
	if(phrases.find(message.GetText()) != phrases.end() && std::time(nullptr) - lastSpam > 10)
	{
		std::stringstream ss;
		CString mynick = GetNetwork()->GetIRCNick().GetNickMask();
		CChan *chan = message.GetChan();
		CString msg = message.GetText();

		ss << "PRIVMSG " << chan->GetName() << " :" << msg;
		PutIRC(ss.str());

		CThreadPool::Get().addJob(new GenericJob([]() {}, [this, chan, mynick, msg]()
		{
			std::stringstream ss;
			ss << ":" << mynick << " PRIVMSG " << chan->GetName() << " :";
			CString s = ss.str();

			PutUser(s + msg);

			if(!chan->AutoClearChanBuffer() || !GetNetwork()->IsUserOnline() || chan->IsDetached())
			chan->AddBuffer(s + "{text}", msg);
		}));

		lastSpam = std::time(nullptr);
	}

	return CModule::CONTINUE;
}

void AutoSpam::AddPhrase(const CString& cmd)
{
	CString phrase =  cmd.Token(1, true);
	phrase.Trim();
	phrases.insert(phrase);
	Save();
	PutModule("Spamming "+phrase);
}

void AutoSpam::RemovePhrase(const CString& cmd)
{
	CString phrase =  cmd.Token(1, true);
	phrase.Trim();
	phrases.erase(phrase.Trim_n());
	Save();
	PutModule("No more "+phrase);
}

bool AutoSpam::OnLoad(const CString& args, CString& message)
{
	VCString vsPhrases;
        GetNV("Phrases").Split("\n", vsPhrases, false);
	for (const CString& phrase : vsPhrases)
	{
		phrases.insert(phrase);
	}

	return true;
}

void AutoSpam::Save()
{
	CString sPhrases;
	for (const CString& phrase : phrases)
	{
		sPhrases = sPhrases + phrase + "\n";
	}
	SetNV("Phrases", sPhrases);
}

template<> void TModInfo<AutoSpam>(CModInfo &info)
{
	info.SetWikiPage("Twitch");
	info.SetHasArgs(false);
}

NETWORKMODULEDEFS(AutoSpam, "Automatically spam phrases")
