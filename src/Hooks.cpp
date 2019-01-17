#include "Hooks.h"

#include "skse64_common/Relocation.h"  // RelocPtr
#include "skse64_common/SafeWrite.h"  // SafeWrite64

#include <typeinfo>  // typeid

#include "LocaleManager.h"  // LocaleManager

#include "RE/BSScaleformTranslator.h"  // BSScaleformTranslator


class BSScaleformTranslatorEx : public RE::BSScaleformTranslator
{
public:
	typedef void _Translate_t(RE::BSScaleformTranslator* a_this, TranslateInfo* a_translateInfo);
	static _Translate_t* orig_Translate;

	void Hook_Translate(TranslateInfo* a_translateInfo)
	{
		LocaleManager* locManager = LocaleManager::GetSingleton();

		static bool loaded = false;
		if (!loaded) {
			for (auto& translation : translationTable) {
				locManager->InsertLocalizationString(translation.GetKey(), translation.GetValue());
			}
			locManager->LoadLocalizationStrings();
			loaded = true;
		}

		std::wstring localization = locManager->GetLocalization(a_translateInfo->GetKey());
		a_translateInfo->SetResult(localization.data());
	}


	static void InstallHooks()
	{
		constexpr uintptr_t BS_SCALEFORM_TRANSLATOR_VTBL = 0x0017D0C30;  // 1_5_62

		RelocPtr<_Translate_t*> vtbl_Translate(BS_SCALEFORM_TRANSLATOR_VTBL + (0x2 * 0x8));
		orig_Translate = *vtbl_Translate;
		SafeWrite64(vtbl_Translate.GetUIntPtr(), GetFnAddr(&Hook_Translate));

		_DMESSAGE("[DEBUG] Installed hooks for class (%s)", typeid(BSScaleformTranslatorEx).name());
	}
};


BSScaleformTranslatorEx::_Translate_t* BSScaleformTranslatorEx::orig_Translate;


void InstallHooks()
{
	BSScaleformTranslatorEx::InstallHooks();
}
