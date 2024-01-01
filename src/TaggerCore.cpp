#include "pch.h"
#include "TaggerCore.h"
#include <string>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include "nlohmann/json.hpp"

const wchar_t AdsTaggerIni[] = _T("\\adstagger.ini");
const wchar_t RecentTags[] = _T("\\recent_tags.ini");
const wchar_t RecentFiles[] = _T("\\recent_files.ini");

bool isTag(char c)
{
	return c >= '0'&&c <= '9' || c >= 'A'&&c <= 'Z' || c >= 'a'&&c <= 'z';
}

bool isVal(char c)
{
	return  c != ' ' && c != '\t' && c != '\0';
}

bool isDef(char c)
{
	return c == '=' || c == ':';
}

std::wstring GetTagsPath(LPCWSTR fpath)
{
	std::wstring adspath;
	if (_tcsstr(fpath, _T("\\\\?\\")))
		return fpath;
	if (!_tcsncmp(fpath, _T("\\\\"), 2)) {
		adspath = _T("\\\\?\\UNC\\");
		adspath += (fpath + 2);
	}
	else {
		adspath = _T("\\\\?\\");
		adspath += fpath;
	}
	adspath += _T(":Tags");
	return adspath;
}

bool GetFTime(LPCWSTR fpath, FILETIME &ftc, FILETIME &tfa, FILETIME &tfw)
{
	HANDLE h = CreateFileW(fpath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
	if (h == INVALID_HANDLE_VALUE)
		return false;
	bool r = GetFileTime(h, &ftc, &tfa, &tfw);
	CloseHandle(h);
	return r;
}

bool SetFTime(LPCWSTR fpath, FILETIME &ftc, FILETIME &tfa, FILETIME &tfw)
{
	HANDLE h = CreateFileW(fpath, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	bool r = SetFileTime(h, &ftc, &tfa, &tfw);
	CloseHandle(h);
	return r;
}

TaggerCore Core;

TagIter TaggerCore::parseTags(const char* tags, std::list<Tag*> *pFileTags)
{
	enum EState {
		S_SP1,
		S_TAG,
		S_SP2,
		S_SP3,
		S_VAL
	} s = S_SP1;
	const char* start = tags;
	TagIter it = m_Tags.end(), first = m_Tags.end();
	bool found;

	while (1) {
		switch (s) {
		case S_SP1:	// waiting for the start of the tag
			if (isTag(*tags)) {
				s = S_TAG;
				start = tags;
			}
			break;
		case S_TAG:	// waiting for the end of the tag
			if (!isTag(*tags)) {
				std::string tag(start, tags - start);
				if(addTag(tag.c_str(), 1, &it))
					if (first == m_Tags.end())
						first = it;
				if (pFileTags)
					pFileTags->push_back(&(*it));
				if (isDef(*tags))	// if there is a ':' or '=', then we switch to waiting for the beginning of the value
					s = S_SP3;
				else
					s = S_SP2;
			}
			break;
		case S_SP2:	// waiting for the ':' or '=' sign or the beginning of the next tag
			if (isDef(*tags)) {
				s = S_SP3;
			}
			else if (isTag(*tags)) {
				s = S_TAG;
				start = tags;
			}
			break;
		case S_SP3:	// waiting for the beginning of the value
			if (isVal(*tags)) {
				s = S_VAL;
				start = tags;
			}
			break;
		case S_VAL:	// waiting for the end of the value
			if (!isVal(*tags)) {
				std::string val(start, tags - start);
				it->val = val;
				s = S_SP1;
			}
			break;
		}
		// exit from the middle, we need at least one iteration, for a null character it is valid
		if (!*tags)
			break;
		tags++;
	}
	return first;
}

void TaggerCore::loadFileTags(FileTags &f)
{
	// load file tags from ADS
	std::wstring tpath = GetTagsPath(f.m_fpath.c_str());
	std::ifstream t;
	t.open(tpath);
	if (t) {
		std::string line;
		std::getline(t, line);
		parseTags(line.c_str(), &f.m_tags);
	}
	t.close();
}

void TaggerCore::saveFileTags(FileTags &f)
{
	// save file tags to ADS
	std::wstring tpath = GetTagsPath(f.m_fpath.c_str());
	std::ofstream t;
	t.open(tpath, std::ofstream::out | std::ofstream::trunc);
	if (t) {
		std::string line;
		makeTags(f.m_tags, line);
		t << line;
	}
	t.close();
	SetFTime(f.m_fpath.c_str(), f.ftc, f.tfa, f.tfw);

	addToLog(f.m_fpath);
}

void TaggerCore::addToLog(const std::wstring &fpath)
{
	std::wofstream t;
	t.open(m_AppPath + RecentFiles, std::wofstream::out | std::wofstream::app);
	if (t) {
		std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm tm;
		gmtime_s(&tm, &tt);
		t << std::put_time(&tm, _T("%Y.%m.%d %H:%M:%S"));
		t << _T(" ");
		t << fpath;
		t << std::endl;
	}
	t.close();
}

void TaggerCore::makeTags(std::list<Tag*> &fileTags, std::string &tags)
{
	// make tags string to saving into ADS
	// existing file tags
	for (auto &t : fileTags) {
		if (t->chk == 2) {
			if (!tags.empty())
				tags += " ";
			tags += t->tag;
			if (!t->val.empty()) {
				tags += ":";
				tags += t->val;
			}
		}
	}
	// new tags
	for (auto &t : m_Tags) {
		if (t.chk == 1) {
			if (!tags.empty())
				tags += " ";
			tags += t.tag;
			// val of first file (!!!)
			if (!t.val.empty()) {
				tags += ":";
				tags += t.val;
			}
		}
	}
}

void TaggerCore::parseCommandLine()
{
	// parse command line
	auto cl = GetCommandLine();
	int nArgc = 0;
	LPWSTR *pArgv = ::CommandLineToArgvW(cl, &nArgc);
	
	for (int i = 1; i < nArgc; i++) {
		if (std::filesystem::exists(pArgv[i])) {
			FileTags f;
			f.m_fpath = pArgv[i];
			GetFTime(pArgv[i], f.ftc, f.tfa, f.tfw);
			loadFileTags(f);
			m_Files.push_back(f);
		}
	}
}

void TaggerCore::buildUsedTags()
{
	// convert count to check status
	for (auto &t : m_Tags) {
		if (t.chk == Core.m_Files.size())
			t.chk = 1;
		else if (t.chk != 0)
			t.chk = -1;
	}
}

void TaggerCore::init()
{
	wchar_t path[MAX_PATH]=_T("");
	GetModuleFileName(NULL, path, MAX_PATH);
	wchar_t *ls = _tcsrchr(path, '\\');
	if (ls)
		*ls = 0;
	m_AppPath = path;

	parseCommandLine();
	buildUsedTags();
	loadRecentTags();
}

bool TaggerCore::loadIni()
{
	std::ifstream file(m_AppPath + AdsTaggerIni);
	nlohmann::json j;
	try {
		j = nlohmann::json::parse(file);
	}
	catch (...) {
		Cfg.x1 = 100;
		Cfg.y2 = 100;
		Cfg.x2 = 500;
		Cfg.y2 = 300;
		return false;
	}
	Cfg.x1 = j["x1"];
	Cfg.y1 = j["y1"];
	Cfg.x2 = j["x2"];
	Cfg.y2 = j["y2"];
	return true;
}

void TaggerCore::saveIni()
{
	nlohmann::json j;
	j["x1"] = Cfg.x1;
	j["y1"] = Cfg.y1;
	j["x2"] = Cfg.x2;
	j["y2"] = Cfg.y2;

	std::ofstream file(m_AppPath + AdsTaggerIni);
	file << j.dump(2);
}

void TaggerCore::apply()
{
	for (auto &f : m_Files) 
		saveFileTags(f);
	saveRecentTags();
}

void TaggerCore::loadRecentTags()
{
	// load recent tags from ini
	std::ifstream t;
	t.open(m_AppPath + RecentTags);
	std::string line;
	while (t) {
		std::getline(t, line);
		if(!line.empty())
			addTag(line.c_str(), 0, nullptr);
	}
	t.close();
}

void TaggerCore::saveRecentTags()
{
	// save all used tags to ini
	std::string input;
	std::ofstream out(m_AppPath + RecentTags, std::ofstream::out | std::ofstream::trunc);
	for (Tag &t : m_Tags) {
		out << t.tag << "\n";
	}
	out.close();
}

bool TaggerCore::addTag(const char* t, int chk, TagIter *pIt)
{
	// find tag in comon list
	auto it = std::find_if(m_Tags.begin(), m_Tags.end(), [&t](Tag &a) {
		return a.tag == t;
	});

	// if not found - insert at end
	bool added = false;
	if (it == m_Tags.end()) {
		it = m_Tags.insert(it, Tag{ t, "", chk });
		added = true;
	}
	else if (chk != 0) {
		it->chk++;
	}
	if (pIt)
		*pIt = it;
	return added;
}

