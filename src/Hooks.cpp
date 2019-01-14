#include "Hooks.h"

#include "skse64_common/Relocation.h"  // RelocPtr
#include "skse64_common/SafeWrite.h"  // SafeWrite64

#include <typeinfo>  // typeid

#include "LocaleManager.h"  // LocaleManager

#include "RE/BSScaleformTranslator.h"  // BSScaleformTranslator


class BSScaleformTranslatorEx : public RE::BSScaleformTranslator
{
public:
	typedef void _GetTranslation_t(RE::BSScaleformTranslator* a_this, wchar_t*& a_key);
	static _GetTranslation_t* orig_GetTranslation;

	void Hook_GetTranslation(wchar_t*& a_key)
	{
		constexpr uintptr_t PUSH_TRANSLATION_ADDR = 0x00F4B4E0;  // 1_5_62
		typedef void _PushTranslation_t(wchar_t*& a_key, wchar_t* a_translation, UInt64 a_arg3);
		static RelocAddr<_PushTranslation_t*> _PushTranslation(PUSH_TRANSLATION_ADDR);

		LocaleManager* locManager = LocaleManager::GetSingleton();

		static bool loaded = false;
		if (!loaded) {
			for (auto& entry : translationTable) {
				locManager->InsertLocalizationString(entry.GetKey(), entry.GetValue());
			}
			loaded = true;
		}

		std::wstring localization = locManager->GetLocalization(a_key);
		_PushTranslation(a_key, localization.data(), -1);
	}


	static void InstallHooks()
	{
		constexpr uintptr_t BS_SCALEFORM_TRANSLATOR_VTBL = 0x0017D0C30;  // 1_5_62

		RelocPtr<_GetTranslation_t*> vtbl_GetTranslation(BS_SCALEFORM_TRANSLATOR_VTBL + (0x2 * 0x8));
		orig_GetTranslation = *vtbl_GetTranslation;
		SafeWrite64(vtbl_GetTranslation.GetUIntPtr(), GetFnAddr(&Hook_GetTranslation));

		_DMESSAGE("[DEBUG] Installed hooks for class (%s)", typeid(BSScaleformTranslatorEx).name());
	}
};


BSScaleformTranslatorEx::_GetTranslation_t* BSScaleformTranslatorEx::orig_GetTranslation;


void InstallHooks()
{
	BSScaleformTranslatorEx::InstallHooks();
}
