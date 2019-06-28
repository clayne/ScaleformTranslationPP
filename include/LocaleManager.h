#pragma once

#include <filesystem>  // path
#include <optional>  // optional
#include <queue>  // queue
#include <regex>  // wregex
#include <stack>  // stack
#include <string>  // wstring, string
#include <unordered_map>  // unordered_map

#include "RE/Skyrim.h"


class LocaleManager
{
public:
	static LocaleManager*	GetSingleton();
	static std::wstring		ConvertStringToWstring(const std::string& a_str);
	static std::string		ConvertWStringToString(const std::wstring& a_str);

	void			Dump();
	void			LoadLocalizationStrings();
	void			LoadLocalizationMap(RE::BSScaleformTranslator::TranslationTable& a_translationTable);
	bool			LocalizationsLoaded() const;
	std::wstring	GetLocalization(std::wstring a_key);
	std::string		GetLocalization(std::string a_key);

private:
	using LocalizationMap = std::unordered_map<std::wstring, std::wstring>;
	using size_type = std::wstring::size_type;


	LocaleManager();
	LocaleManager(const LocaleManager&) = delete;
	LocaleManager(LocaleManager&&) = delete;
	~LocaleManager();

	LocaleManager& operator=(const LocaleManager&) = delete;
	LocaleManager& operator=(LocaleManager&&) = delete;

	void						FindFiles(const std::filesystem::path& a_path, const std::wregex& a_pattern, bool a_english);
	void						ReadFromFile(const std::filesystem::path& a_path, bool a_english);
	LocalizationMap&			GetLocalizationMap();
	std::wstring				GetLocalizationInternal(const std::wstring& a_key);
	std::optional<std::wstring>	SanitizeKey(std::wstring a_key);
	bool						GetNestedLocalizations(const std::wstring& a_key, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue);
	std::optional<std::wstring>	FindLocalization(const std::wstring& a_key);
	bool						InsertLocalizations(std::wstring& a_localization, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue);


	LocalizationMap	_localizations_ENG;
	LocalizationMap	_localizations_LOC;
	bool			_isLoaded;
};
