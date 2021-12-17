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
				REL::Relocation<Ctor_t> func{ _ctor };
				func(this);
				auto bethTranslator = loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator);

				auto newTranslator = new LocaleManager(bethTranslator->translator);
				loader->SetState(RE::GFxState::StateType::kTranslator, newTranslator);

				auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
				menuSink->SetBethTranslator(std::move(bethTranslator));

				return this;
			}


			static void InstallHooks()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(35548),
#ifndef SKYRIMVR
					0xA08
#else
					0xACE
#endif
				};
				auto& trampoline = SKSE::GetTrampoline();
				_ctor = trampoline.write_call<5>(target.address(), &BSScaleformMovieLoaderEx::Ctor);
				logger::info("Installed hooks for ({})", typeid(BSScaleformMovieLoaderEx).name());
			}

		private:
			using Ctor_t = decltype(&BSScaleformMovieLoaderEx::Ctor);
			static inline std::uintptr_t _ctor;
		};
	}


	void Install()
	{
		BSScaleformMovieLoaderEx::InstallHooks();
		logger::info("Installed all hooks");
	}
}
