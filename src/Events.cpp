#include "Events.h"


namespace Events
{
	MenuOpenCloseEventHandler* MenuOpenCloseEventHandler::GetSingleton()
	{
		static MenuOpenCloseEventHandler singleton;
		return &singleton;
	}


	auto MenuOpenCloseEventHandler::ReceiveEvent(RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
		-> EventResult
	{
		auto uiStr = RE::InterfaceStrings::GetSingleton();
		if (a_event && a_event->isOpening && a_event->menuName == uiStr->journalMenu) {
			auto mm = RE::MenuManager::GetSingleton();
			auto journal = mm->GetMenu(uiStr->journalMenu);
			if (journal && journal->view) {
				journal->view->SetState(RE::GFxState::StateType::kTranslator, _bethImpl.get());
			}
		}

		return EventResult::kContinue;
	}


	void MenuOpenCloseEventHandler::SetBethTranslator(RE::GPtr<RE::GFxTranslator>&& a_translator)
	{
		_bethImpl = std::move(a_translator);
	}
}
