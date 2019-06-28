#include "LocaleManager.h"

#include <string.h>  // _wcsicmp
#include <stringapiset.h>  // MultiByteToWideChar, WideCharToMultiByte

#include <codecvt>  // codecvt_mode, codecvt_utf16
#include <filesystem>  // path, directory_iterator
#include <fstream>  // wifstream
#include <queue>  // queue
#include <regex>  // regex, regex_match
#include <stack>  // stack
#include <string>  // string, wstring

#include "RE/Skyrim.h"


LocaleManager* LocaleManager::GetSingleton()
{
	static LocaleManager singleton;
	return &singleton;
}


std::wstring LocaleManager::ConvertStringToWstring(const std::string& a_str)
{
	if (a_str.empty()) {
		return std::wstring();
	}

	int numChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a_str.c_str(), a_str.length(), NULL, 0);
	std::wstring wstrTo;
	if (numChars) {
		wstrTo.resize(numChars);
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a_str.c_str(), a_str.length(), wstrTo.data(), numChars)) {
			return wstrTo;
		}
	}

	return std::wstring();
}


std::string LocaleManager::ConvertWStringToString(const std::wstring& a_str)
{
	if (a_str.empty()) {
		return std::string();
	}

	int numChars = WideCharToMultiByte(CP_UTF8, 0, a_str.c_str(), a_str.length(), NULL, 0, NULL, NULL);
	std::string strTo;
	if (numChars) {
		strTo.resize(numChars);
		if (WideCharToMultiByte(CP_UTF8, 0, a_str.c_str(), a_str.length(), strTo.data(), numChars, NULL, NULL)) {
			return strTo;
		}
	}

	return std::string();
}


void LocaleManager::Dump()
{
	std::string key;
	std::string value;
	for (auto& pair : GetLocalizationMap()) {
		key = ConvertWStringToString(pair.first);
		value = ConvertWStringToString(pair.second);
		_DMESSAGE("%s: %s", key.c_str(), value.c_str());
	}
}


void LocaleManager::LoadLocalizationStrings()
{
	constexpr wchar_t REGEX_PREFIX[] = L".*_";
	constexpr wchar_t ENGLISH[] = L"ENGLISH";
	constexpr wchar_t REGEX_POSTFIX[] = L"\\.txt$";
	constexpr auto REGEX_FLAGS = std::regex_constants::grep | std::regex_constants::icase;

	std::filesystem::path path("data/interface/translations");

	std::wstring pattern(REGEX_PREFIX);
	std::wstring wLanguage(ENGLISH);
	auto setting = RE::GetINISetting("sLanguage:General");
	if (setting) {
		auto u8Language = setting->GetString();
		wLanguage = ConvertStringToWstring(u8Language);
	}
	pattern += wLanguage;
	pattern += REGEX_POSTFIX;
	std::wregex regex(pattern, REGEX_FLAGS);

	bool english = _wcsicmp(ENGLISH, wLanguage.c_str()) == 0;

	FindFiles(path, regex, english);
	if (!english) {
		pattern = REGEX_PREFIX;
		pattern += ENGLISH;
		pattern += REGEX_POSTFIX;
		regex.assign(pattern, REGEX_FLAGS);
		FindFiles(path, regex, true);
	}

	_isLoaded = true;
}


void LocaleManager::LoadLocalizationMap(RE::BSScaleformTranslator::TranslationTable& a_translationTable)
{
	auto& localizations = GetLocalizationMap();
	localizations.reserve(a_translationTable.size());
	for (auto& entry : a_translationTable) {
		localizations.insert({ entry.GetKey(), entry.GetValue() });
	}

	_isLoaded = true;
}


bool LocaleManager::LocalizationsLoaded() const
{
	return _isLoaded;
}


std::wstring LocaleManager::GetLocalization(std::wstring a_key)
{
	return GetLocalizationInternal(a_key);
}


std::string LocaleManager::GetLocalization(std::string a_key)
{
	return ConvertWStringToString(GetLocalization(ConvertStringToWstring(a_key)));
}


LocaleManager::LocaleManager() :
	_isLoaded(false)
{}


LocaleManager::~LocaleManager()
{}


void LocaleManager::FindFiles(const std::filesystem::path& a_path, const std::wregex& a_pattern, bool a_english)
{
	std::filesystem::path fileName;
	for (auto& dirEntry : std::filesystem::directory_iterator(a_path)) {
		fileName = dirEntry.path().filename();
		if (std::regex_match(fileName.native(), a_pattern)) {
			ReadFromFile(dirEntry.path(), a_english);
		}
	}
}


void LocaleManager::ReadFromFile(const std::filesystem::path& a_path, bool a_english)
{
	constexpr auto CVT_MODE = std::codecvt_mode(std::little_endian | std::consume_header);
	constexpr auto NPOS = std::wstring::npos;

	auto& localizations = a_english ? _localizations_ENG : _localizations_LOC;
	std::wifstream inFile(a_path);
	inFile.imbue(std::locale(inFile.getloc(), new std::codecvt_utf16<wchar_t, 0x10FFFF, CVT_MODE>));  // UCS-2 LE w/ BOM
	std::wstring line;
	std::wstring key;
	std::wstring value;
	if (!inFile.is_open()) {
		_ERROR("[ERROR] Failed to open file \"%s\"!\n", a_path.string().c_str());
		return;
	}

	while (std::getline(inFile, line)) {
		if (!line.empty() && line.back() == L'\r') {
			line.pop_back();  // discard carriage return
		}
		if (line.empty() || line.front() != L'$') {
			continue;
		}
		key.clear();
		value.clear();

		auto pos = line.find_first_of(L'\t');
		if (pos != NPOS) {
			key = line.substr(0, pos);
			value = line.substr(pos + 1);
		}

		if (!key.empty() && !value.empty()) {
			auto sanitizedKey = SanitizeKey(key);
			if (sanitizedKey) {
				key = std::move(*sanitizedKey);
			}
			localizations.insert({ std::move(key), std::move(value) });
		}
	}
}


LocaleManager::LocalizationMap& LocaleManager::GetLocalizationMap()
{
	return _localizations_LOC.empty() ? _localizations_ENG : _localizations_LOC;
}


std::wstring LocaleManager::GetLocalizationInternal(const std::wstring& a_key)
{
	if (a_key.empty() || a_key[0] != L'$') {
		return a_key;
	}

	auto sanitizedKey = SanitizeKey(a_key);
	if (!sanitizedKey) {
		return a_key;
	}

	auto localization = FindLocalization(*sanitizedKey);
	if (!localization) {
		return a_key;
	}

	std::stack<size_type> stack;
	std::queue<std::wstring> queue;
	if (!GetNestedLocalizations(a_key, stack, queue)) {
		return *localization;
	}

	while (!stack.empty()) {
		stack.pop();
	}
	InsertLocalizations(*localization, stack, queue);
	return *localization;
}


std::optional<std::wstring> LocaleManager::SanitizeKey(std::wstring a_key)
{
	std::stack<size_type> stack;
	for (size_type pos = 0; pos < a_key.size(); ++pos) {
		switch (a_key[pos]) {
		case L'{':
			stack.push(pos);
			break;
		case L'}':
			{
				switch (stack.size()) {
				case 0:
					return std::nullopt;
				case 1:
					{
						size_type last = stack.top();
						stack.pop();
						auto off = last + 1;
						auto count = pos - last - 1;
						if (count > 0) {
							a_key.replace(off, count, L"");
						}
						pos = off;
					}
					break;
				default:
					stack.pop();
				}
				break;
			}
		}
	}

	if (!a_key.empty() && a_key.back() == L'\r') {
		a_key.pop_back();
	}

	return std::make_optional(a_key);
}


bool LocaleManager::GetNestedLocalizations(const std::wstring& a_key, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue)
{
	for (size_type pos = 0; pos < a_key.size(); ++pos) {
		switch (a_key[pos]) {
		case L'{':
			a_stack.push(pos);
			break;
		case L'}':
			{
				switch (a_stack.size()) {
				case 0:
					return false;
				case 1:
					{
						size_type last = a_stack.top();
						a_stack.pop();
						auto off = last + 1;
						auto count = pos - last - 1;
						switch (count) {
						case 0:
							return false;	// nothing to replace {} with
						case 1:
							if (std::isdigit(a_key[off])) {	// intended for skyui
								return false;
							}
							break;
						default:
							break;
						}
						auto subStr = a_key.substr(off, count);
						a_queue.push(GetLocalizationInternal(subStr));
					}
					break;
				default:
					a_stack.pop();
				}
				break;
			}
		}
	}

	return true;
}


std::optional<std::wstring> LocaleManager::FindLocalization(const std::wstring& a_key)
{
	auto& localizations = GetLocalizationMap();
	auto it = localizations.find(a_key);
	if (it == localizations.end()) {
		if (&localizations != &_localizations_ENG) {
			it = _localizations_ENG.find(a_key);
			if (it == _localizations_ENG.end()) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}
	}

	return std::make_optional(it->second);
}


bool LocaleManager::InsertLocalizations(std::wstring& a_localization, std::stack<size_type>& a_stack, std::queue<std::wstring>& a_queue)
{
	for (size_type pos = 0; pos < a_localization.size(); ++pos) {
		switch (a_localization[pos]) {
		case L'{':
			a_stack.push(pos);
			break;
		case L'}':
			{
				if (a_stack.empty() || a_queue.empty()) {
					return false;
				}

				size_type beg = a_stack.top();
				a_stack.pop();
				auto subStr = a_queue.front();
				a_queue.pop();

				a_localization.replace(beg, pos - beg + 1, subStr);
				pos = beg;
			}
			break;
		}
	}

	return true;
}
