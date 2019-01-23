#include "LocaleManager.h"

#include "skse64/GameSettings.h"  // Setting, GetINISetting

#include <string>  // string, wstring
#include <fstream>  // wifstream
#include <map>  // map
#include <queue>  // queue
#include <stack>  // stack
#include <utility>  // make_pair
#include <codecvt>  // codecvt_mode, codecvt_utf16

#include <wchar.h>  // _wcsicmp
#include <stringapiset.h>  // MultiByteToWideChar, WideCharToMultiByte

#include "RE/BSScaleformTranslator.h"  // BSScaleformTranslator


LocaleManager* LocaleManager::GetSingleton()
{
	if (!_singleton) {
		_singleton = new LocaleManager();
	}
	return _singleton;
}


void LocaleManager::Free()
{
	delete _singleton;
	_singleton = 0;
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
	constexpr char* PREFIX = "Data\\interface\\translations\\";
	constexpr char* ENGLISH = "ENGLISH";
	constexpr char* FILE_EXT = ".txt";

	Setting* language_general = GetINISetting("sLanguage:General");
	std::string path = PREFIX;
	path += "*_";
	std::string language = (language_general && (language_general->GetType() == Setting::kType_String)) ? language_general->data.s : ENGLISH;
	bool english = language == ENGLISH;
	path += language;
	path += FILE_EXT;

	FindFiles(path, PREFIX, english);
	if (!english) {
		path = PREFIX;
		path += "*_";
		path += ENGLISH;
		path += FILE_EXT;
		FindFiles(path, PREFIX, true);
	}

	_isLoaded = true;
}


void LocaleManager::LoadLocalizationMap(RE::BSScaleformTranslator::TranslationTable& a_translationTable)
{
	LocalizationMap& localizations = GetLocalizationMap();
	for (auto& entry : a_translationTable) {
		localizations.insert(std::make_pair(entry.GetKey(), entry.GetValue()));
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


LocaleManager::Result::Result(bool a_good, std::wstring a_str) :
	good(a_good),
	str(a_str)
{}


LocaleManager::LocaleManager() :
	_isLoaded(false)
{}


LocaleManager::~LocaleManager()
{}


void LocaleManager::FindFiles(std::string a_path, const char* a_prefix, bool a_english)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;

	hFind = FindFirstFile(a_path.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			a_path = a_prefix;
			a_path += findData.cFileName;
			ReadFromFile(a_path.c_str(), a_english);
		} while (FindNextFile(hFind, &findData));
	}
}


#pragma warning(push)
#pragma warning(disable : 4996)  // codecvt deprecated in c++17
void LocaleManager::ReadFromFile(const char* a_filePath, bool a_english)
{
	constexpr std::codecvt_mode cvtMode = std::codecvt_mode(std::little_endian | std::consume_header);
	constexpr size_type NPOS = std::wstring::npos;

	LocalizationMap& localizations = a_english ? _localizations_ENG : _localizations_LOC;
	std::wifstream inFile(a_filePath);
	inFile.imbue(std::locale(inFile.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, cvtMode>));  // UCS-2 LE w/ BOM
	std::wstring line;
	std::wstring key;
	std::wstring value;
	if (!inFile.is_open()) {
		_ERROR("[ERROR] Failed to open file \"%s\"", a_filePath);
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

		size_type pos = line.find_first_of(L'\t');
		if (pos != NPOS) {
			key = std::wstring(line, 0, pos);
			value = std::wstring(line, pos + 1);
		}

		if (!key.empty() && !value.empty()) {
			localizations.insert(std::make_pair(key, value));
		}
	}
}
#pragma warning(pop)


LocaleManager::LocalizationMap& LocaleManager::GetLocalizationMap()
{
	return _localizations_LOC.empty() ? _localizations_ENG : _localizations_LOC;
}


std::wstring LocaleManager::GetLocalizationInternal(const std::wstring& a_key)
{
	if (a_key.empty() || a_key[0] != L'$') {
		return a_key;
	}

	Result result = GetKey(a_key);
	if (!result.good) {
		return a_key;
	}
	std::wstring key(result.str);

	std::stack<size_t> stack;
	std::queue<std::wstring> queue;
	if (!GetNestedLocalizations(a_key, stack, queue)) {
		return a_key;
	}

	result = FindLocalization(key);
	if (!result.good) {
		return a_key;
	}
	std::wstring localization(result.str);

	while (!stack.empty()) {
		stack.pop();
	}
	InsertLocalizations(localization, stack, queue);
	return localization;
}


LocaleManager::Result LocaleManager::GetKey(std::wstring a_key)
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
					return { false, L"" };
				case 1:
					{
						size_type last = stack.top();
						stack.pop();
						size_type off = last + 1;
						size_type count = pos - last - 1;
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

	return { true, a_key };
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
						size_type off = last + 1;
						size_type count = pos - last - 1;
						std::wstring subStr = (count > 0) ? std::wstring(a_key, off, count) : L"";
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


LocaleManager::Result LocaleManager::FindLocalization(const std::wstring& a_key)
{
	LocalizationMap& localizations = GetLocalizationMap();
	LocalizationMap::const_iterator it = localizations.find(a_key);
	if (it == localizations.end()) {
		if (&localizations != &_localizations_ENG) {
			it = _localizations_ENG.find(a_key);
			if (it == _localizations_ENG.end()) {
				return { false, L"" };
			}
		} else {
			return { false, L"" };
		}
	}

	return { true, it->second };
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
				std::wstring subStr = a_queue.front();
				a_queue.pop();

				a_localization.replace(beg, pos - beg + 1, subStr);
				pos = beg;
			}
			break;
		}
	}

	return true;
}


LocaleManager* LocaleManager::_singleton = 0;
