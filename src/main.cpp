#include "skse64_common/skse_version.h"

#include <clocale>
#include <locale>

#include "Events.h"
#include "Hooks.h"
#include "version.h"

#include "RE/Skyrim.h"
#include "SKSE/API.h"


namespace
{
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kInputLoaded:
			{
				auto mm = RE::MenuManager::GetSingleton();
				auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
				mm->AddEventSink(menuSink);

				Hooks::Install();
			}
			break;
		}
	}
}


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		std::setlocale(LC_ALL, "");

		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\ScaleformTranslationPP.log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::UseLogStamp(true);

		_MESSAGE("ScaleformTranslationPP v%s", STPP_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "ScaleformTranslationPP";
		a_info->version = STPP_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("Loaded in editor, marking as incompatible!\n");
			return false;
		}

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_97:
			break;
		default:
			_FATALERROR("Unsupported runtime version %s!\n", a_skse->UnmangledRuntimeVersion().c_str());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("ScaleformTranslationPP loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		if (!SKSE::AllocBranchTrampoline(1024 * 1)) {
			return false;
		}

		auto messaging = SKSE::GetMessagingInterface();
		if (!messaging->RegisterListener("SKSE", MessageHandler)) {
			return false;
		}

		return true;
	}
};
