#pragma once

#include <map>  // map
#include <queue>  // queue
#include <string>  // wstring, string
#include <stack>  // stack

#include "RE/BSScaleformTranslator.h"  // BSScaleformTranslator


class LocaleManager
{
public:
	static LocaleManager*	GetSingleton();
	void					Free();
	void					Dump();
	void					LoadLocalizationStrings();
	void					LoadLocalizationMap(RE::BSScaleformTranslator::TranslationTable& a_translationTable);
	bool					LocalizationsLoaded() const;
	std::wstring			GetLocalization(std::wstring a_key);
	std::string				GetLocalization(std::string a_key);

	std::wstring			ConvertStringToWstring(const std::string& a_str);
	std::string				ConvertWStringToString(const std::wstring& a_str);

private:
	struct Result
	{
		Result(bool a_good, std::wstring a_str);

		bool			good;
		std::wstring	str;
	};


	typedef std::map<std::wstring, std::wstring> LocalizationMap;
	typedef std::wstring::size_type size_type;


	LocaleManager();
	~LocaleManager();

	void				FindFiles(std::string a_path, const char* a_prefix, bool a_english);
	void				ReadFromFile(const char* a_filePath, bool a_english);
	LocalizationMap&	GetLocalizationMap();
	std::wstring		GetLocalizationInternal(const std::wstring& a_key);
	Result				GetKey(std::wstring a_key);
	bool				GetNestedLocalizations(const std::wstring& a_key, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue);
	Result				FindLocalization(const std::wstring& a_key);
	bool				InsertLocalizations(std::wstring& a_localization, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue);


	static LocaleManager*	_singleton;
	LocalizationMap			_localizations_ENG;
	LocalizationMap			_localizations_LOC;
	bool					_isLoaded;
};
