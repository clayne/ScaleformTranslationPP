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
		if (!locManager->LocalizationsLoaded()) {
			locManager->LoadLocalizationMap(translationTable);
			locManager->LoadLocalizationStrings();
		}

		const wchar_t* key = a_translateInfo->GetKey();
		if (key && key[0] == L'$') {
			std::wstring localization = locManager->GetLocalization(key);
			a_translateInfo->SetResult(localization.c_str());
		}
	}


	static void InstallHooks()
	{
		RelocPtr<_Translate_t*> vtbl_Translate(RE::Offset::BSScaleformTranslator::Vtbl + (0x2 * 0x8));
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
