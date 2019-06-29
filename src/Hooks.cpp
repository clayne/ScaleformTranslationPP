#include "Hooks.h"

#include "skse64_common/SafeWrite.h"

#include <typeinfo>

#include "LocaleManager.h"

#include "RE/Skyrim.h"


namespace
{
	class BSScaleformTranslatorEx : public RE::BSScaleformTranslator
	{
	public:
		using func_t = function_type_t<decltype(&BSScaleformTranslator::Translate)>;
		inline static func_t* func = 0;

		void Hook_Translate(TranslateInfo* a_translateInfo)
		{
			auto locManager = LocaleManager::GetSingleton();
			if (!locManager->LocalizationsLoaded()) {
				locManager->LoadLocalizationMap(translationTable);
				locManager->LoadLocalizationStrings();
			}

			auto key = a_translateInfo->GetKey();
			if (key && key[0] == L'$') {
				std::wstring localization = locManager->GetLocalization(key);
				a_translateInfo->SetResult(localization.c_str());
			}
		}


		static void InstallHooks()
		{
			REL::Offset<func_t**> vFunc(RE::Offset::BSScaleformTranslator::Vtbl + (0x2 * 0x8));
			func = *vFunc;
			SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_Translate));

			_DMESSAGE("[DEBUG] Installed hooks for class (%s)", typeid(BSScaleformTranslatorEx).name());
		}
	};
}


void InstallHooks()
{
	BSScaleformTranslatorEx::InstallHooks();
}
