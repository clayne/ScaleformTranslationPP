#include "skse64/PluginAPI.h"  // PluginHandle, SKSEMessagingInterface, SKSEInterface, PluginInfo, SKSETaskInterface, SKSESerializationInterface
#include "skse64_common/skse_version.h"  // RUNTIME_VERSION

#include <exception> // exception
#include <locale>  // locale

#include <clocale>  // setlocale

#include "Hooks.h"  // InstallHooks
#include "LocaleManager.h"  // LocaleManager
#include "version.h"  // STPP_VERSION_MAJOR

#include "RE/Skyrim.h"
#include "SKSE/API.h"


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		std::setlocale(LC_ALL, "");

		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\ScaleformTranslationPP.log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);

		_MESSAGE("ScaleformTranslationPP v%s", STPP_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "ScaleformTranslationPP";
		a_info->version = STPP_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_73:
		case RUNTIME_VERSION_1_5_80:
			break;
		default:
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("[MESSAGE] ScaleformTranslationPP loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		InstallHooks();
		_MESSAGE("[MESSAGE] Hooks installed");

		return true;
	}
};
