#pragma once
#include <list>
#include <string>
#include <atlbase.h>
#include <atlstr.h>
#include <iostream>
#include <string>
#include <locale>
#include <filesystem>

struct Tag {
	std::string tag;// utf8
	std::string val;// utf8
	int chk;
};
typedef Tag* TagPtr;
typedef std::list<Tag> TagList;
typedef std::list<Tag>::iterator TagIter;


struct FileTags {
	std::wstring m_fpath;
	FILETIME ftc, tfa, tfw;
	std::list<Tag*> m_tags;	
};

class TaggerCore
{
public:
	void init();
	void apply();
	TagIter parseTags(const char* tags, std::list<Tag*> *pFileTags);
	void makeTags(std::list<Tag*> &fileTags, std::string &tags);
	void parseCommandLine();
	void loadRecentTags();
	void saveRecentTags();
	bool addTag(const char* tag, int chk, TagIter *pIt);
	void buildUsedTags();
	void loadFileTags(FileTags &f);
	void saveFileTags(FileTags &f);
	bool loadIni();
	void saveIni();
	void addToLog(const std::wstring &fpath);
public:
	std::wstring m_AppPath;
	std::list<FileTags> m_Files;
	std::list<Tag> m_Tags;
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > m_converter;
	struct {
		int x1, y1, x2, y2;
	} Cfg;
};

extern TaggerCore Core;
