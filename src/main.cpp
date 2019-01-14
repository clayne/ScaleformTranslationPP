#include "skse64/PluginAPI.h"  // PluginHandle, SKSEMessagingInterface, SKSEInterface, PluginInfo, SKSETaskInterface, SKSESerializationInterface
#include "skse64_common/skse_version.h"  // RUNTIME_VERSION

#include <exception> // exception
#include <locale>  // locale

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS

#include <clocale>  // setlocale

#include "Hooks.h"  // InstallHooks
#include "LocaleManager.h"  // LocaleManager
#include "version.h"  // STPP_VERSION_MAJOR


static PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
static SKSEMessagingInterface*		g_messaging = 0;
static SKSESerializationInterface*	g_serialization = 0;
static SKSETaskInterface*			g_task = 0;

constexpr UInt32 SERIALIZATION_VERSION = 2;


void MessageHandler(SKSEMessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSEMessagingInterface::kMessage_DataLoaded:
		{
			LocaleManager* locManager = LocaleManager::GetSingleton();
			locManager->LoadLocalizationStrings();
			locManager->Dump();
		}
		break;
	}
}


extern "C" {
	bool SKSEPlugin_Query(const SKSEInterface* a_skse, PluginInfo* a_info)
	{
		std::setlocale(LC_ALL, "");

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\ScaleformTranslationPP.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("ScaleformTranslationPP v%s", STPP_VERSION_VERSTRING);

		a_info->infoVersion = PluginInfo::kInfoVersion;
		a_info->name = "ScaleformTranslationPP";
		a_info->version = STPP_VERSION_MAJOR;

		g_pluginHandle = a_skse->GetPluginHandle();

		if (a_skse->isEditor) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		if (a_skse->runtimeVersion != RUNTIME_VERSION_1_5_62) {
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", a_skse->runtimeVersion);
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSEInterface* a_skse)
	{
		_MESSAGE("[MESSAGE] ScaleformTranslationPP loaded");

		g_task = (SKSETaskInterface*)a_skse->QueryInterface(kInterface_Task);
		if (g_task) {
			_MESSAGE("[MESSAGE] Task interface query successful");
		} else {
			_FATALERROR("[FATAL ERROR] Task interface query failed!\n");
			return false;
		}

		g_messaging = (SKSEMessagingInterface*)a_skse->QueryInterface(kInterface_Messaging);
		if (g_messaging->RegisterListener(g_pluginHandle, "SKSE", MessageHandler)) {
			_MESSAGE("[MESSAGE] Messaging interface registration successful");
		} else {
			_FATALERROR("[FATAL ERROR] Messaging interface registration failed!\n");
			return false;
		}

		InstallHooks();
		_MESSAGE("[MESSAGE] Hooks installed");

		return true;
	}
};
