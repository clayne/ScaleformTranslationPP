#include "Hooks.h"

#include <typeinfo>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"

#include "Events.h"
#include "LocaleManager.h"


namespace Hooks
{
	namespace
	{
		class BSScaleformMovieLoaderEx : public RE::BSScaleformManager
		{
		public:
			BSScaleformMovieLoaderEx* Ctor()
			{
				_ctor(this);

				auto bethTranslator = gfxLoader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator);

				auto newTranslator = new LocaleManager(bethTranslator->translationTable);
				gfxLoader->SetState(RE::GFxState::StateType::kTranslator, newTranslator);

				auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
				menuSink->SetBethTranslator(std::move(bethTranslator));

				return this;
			}


			static void InstallHooks()
			{
				// E8 ? ? ? ? 84 C0 74 1F E8 ? ? ? ?
				REL::Offset<std::uintptr_t> target(0x005AE010 + 0xA08);	// 1_5_97
				_ctor = REL::Write5Call<Ctor_t*>(target.GetAddress(), unrestricted_cast<std::uintptr_t>(&BSScaleformMovieLoaderEx::Ctor));
				_MESSAGE("Installed hooks for (%s)", typeid(BSScaleformMovieLoaderEx).name());
			}

		private:
			using Ctor_t = function_type_t<decltype(&BSScaleformMovieLoaderEx::Ctor)>;
			inline static Ctor_t* _ctor = 0;
		};
	}


	void Install()
	{
		BSScaleformMovieLoaderEx::InstallHooks();
		_MESSAGE("Installed all hooks");
	}
}
