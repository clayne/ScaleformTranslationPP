#include "Hooks.h"

#include <typeinfo>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/Trampoline.h"

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

				auto bethTranslator = loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator);

				auto newTranslator = new LocaleManager(bethTranslator->translator);
				loader->SetState(RE::GFxState::StateType::kTranslator, newTranslator);

				auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
				menuSink->SetBethTranslator(std::move(bethTranslator));

				return this;
			}


			static void InstallHooks()
			{
				// E8 ? ? ? ? 84 C0 74 1F E8 ? ? ? ?
				REL::Offset<std::uintptr_t> target(0x005AE010 + 0xA08);	// 1_5_97
				auto trampoline = SKSE::GetTrampoline();
				_ctor = trampoline->Write5CallEx<Ctor_t*>(target.GetAddress(), Ctor_f);
				_MESSAGE("Installed hooks for (%s)", typeid(BSScaleformMovieLoaderEx).name());
			}

		private:
			inline static auto Ctor_f = &BSScaleformMovieLoaderEx::Ctor;
			using Ctor_t = function_type_t<decltype(Ctor_f)>;
			inline static Ctor_t* _ctor = 0;
		};
	}


	void Install()
	{
		BSScaleformMovieLoaderEx::InstallHooks();
		_MESSAGE("Installed all hooks");
	}
}
