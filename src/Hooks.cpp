#include "Hooks.h"

#include "Events.h"
#include "LocaleManager.h"

namespace Hooks
{
	namespace
	{
		class BSScaleformMovieLoaderEx :
			public RE::BSScaleformManager
		{
		public:
			BSScaleformMovieLoaderEx* Ctor()
			{
				_ctor(this);

				const auto bethTranslator = loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator);

				const auto newTranslator = new LocaleManager(bethTranslator->translator);
				loader->SetState(RE::GFxState::StateType::kTranslator, newTranslator);

				const auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
				menuSink->SetBethTranslator(std::move(bethTranslator));

				return this;
			}

			static void InstallHooks()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(36547), 0xE91 };
				REL::make_pattern<"E8 ?? ?? ?? ?? 48 89 05 ?? ?? ?? ??">().match_or_fail(target.address());
				auto& trampoline = SKSE::GetTrampoline();
				_ctor = trampoline.write_call<5>(target.address(), &BSScaleformMovieLoaderEx::Ctor);
				logger::debug("Installed hooks for ({})", typeid(BSScaleformMovieLoaderEx).name());
			}

		private:
			using Ctor_t = decltype(&BSScaleformMovieLoaderEx::Ctor);
			static inline REL::Relocation<Ctor_t> _ctor;
		};
	}

	void Install()
	{
		BSScaleformMovieLoaderEx::InstallHooks();
		logger::debug("Installed all hooks");
	}
}
