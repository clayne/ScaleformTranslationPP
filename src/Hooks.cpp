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
				REL::Offset<std::uintptr_t> target(REL::ID(35548), 0xA08);
				auto trampoline = SKSE::GetTrampoline();
				_ctor = trampoline->Write5CallEx(target.GetAddress(), &BSScaleformMovieLoaderEx::Ctor);
				_MESSAGE("Installed hooks for (%s)", typeid(BSScaleformMovieLoaderEx).name());
			}

		private:
			using Ctor_t = decltype(&BSScaleformMovieLoaderEx::Ctor);
			static inline REL::Function<Ctor_t> _ctor;
		};
	}


	void Install()
	{
		BSScaleformMovieLoaderEx::InstallHooks();
		_MESSAGE("Installed all hooks");
	}
}
