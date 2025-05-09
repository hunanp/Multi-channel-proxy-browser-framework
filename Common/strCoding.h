#ifndef Coding_H
#define Coding_H
#include <string>
#include <windows.h>
#include <codecvt>
#include <comutil.h>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <iterator>
#include "base64.h"

#ifdef _WIN32
#include <unordered_map>
#define HASHMAP_PREFIX stdext
#else
#include <ext/hash_map>
#define HASHMAP_PREFIX __gnu_cxx
#endif

#if defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif
#include "exe_file_path.h"

#include <thread>
#include <chrono>
#include <regex.h>

	using namespace std;
	class RTimer {
		bool clear = false;

	public:
		template<typename T>
		void setTimeout(T function, int delay);
		template<typename T>
		void setInterval(T function, int interval);
		void stop();

	};

	class strCoding
	{
	public:
		strCoding(void);
		~strCoding(void);
		static void UTF_8ToGB2312(string &pOut, char *pText, int pLen);//utf_8转为gb2312
		static void GB2312ToUTF_8(string& pOut, const char *pText, int pLen); //gb2312 转utf_8
		static string UrlGB2312(const char * str);       //urlgb2312编码
		static string UrlUTF8(const char * str);                             //urlutf8 编码
		static string UrlUTF8Decode(string str);                  //urlutf8解码
		static string UrlGB2312Decode(string str);                //urlgb2312解码
		static char *urlDecode(const char *str);
		static std::wstring str2wstr(std::string const &str);
		static std::string wstr2str(std::wstring const &str);
		static std::string UTF8ToGBK(const std::string& strUTF8);
		static bool is_vista();
		static bool StringToWideString(const std::string& src, std::wstring &wstr);
		static std::string UnicodeToUTF8(const std::wstring & wstr);
		static std::string GBKToUTF8(const std::string& strGBK);
		static BOOL IsUTF8(const char* str, ULONGLONG length);
		static std::wstring UTF8ToUnicode(const std::string & str);
		static std::string codeDecode(const char * str);
		static char StrToBin(char *str);
		static char CharToInt(char ch);
		static bool StartsWith(const std::string ostr, const std::string str);
		static void split(std::string line, std::vector<std::string> &words, char delimiter);
		static std::vector<std::string> split(std::string string, std::string delim, int count = -1);
		static std::vector<std::string> Split(std::string str, char delimiter);
		static std::unordered_map<std::string, std::string> GetAttributes(std::string str, std::unordered_map<std::string, std::string> &rv,char delimiter = ';');//折分 id:124;pos:15,789;rect:124,357,697,451形式的属性成表
		static std::string concat(std::vector<std::string> args, std::string delim = "", int beginOffset = 0, int endOffset = 1);
		static void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
		static std::string UrlDecode(std::string &url);
		static void GetQuery(std::string result, std::unordered_map<std::string, std::string> &words);
		static std::string ToLower(const std::string &s);
		static void Getvector(const char * fname, std::vector<std::string>&sp);
		static void Getmap(const char * fname, std::unordered_map<std::string, bool>&sp);
		static bool fileExists(const std::string& filename);
		static void ltrim(std::string &string);
		static void rtrim(std::string &string);
		static void trim(std::string &string);
		static bool beginsWith(std::string str, std::string begin);
		static bool endsWith(std::string str, std::string end);
		static void replace(std::string &src, std::string sub, std::string replace, int count =1);
		static void replace(std::string &src, char ch, char n);
		static bool Saveline(std::vector<std::string> args, std::string filename);
		static bool Getline(std::vector<std::string> &args, std::string filename);
		static void DatetimeString(std::string &str);
		static time_t StringToDatetime(const char *str);

		static void Gb2312ToUnicode(WCHAR* pOut,const char *gbBuffer);
		static void UTF_8ToUnicode(WCHAR* pOut, char *pText);
		static void UnicodeToUTF_8(char* pOut, WCHAR* pText);
		static void UnicodeToGB2312(char* pOut, WCHAR uData);
		static void ExeFilePath(std::string &path);
		static string Absolutepath(const char * fname);
		static string Absolutepath(std::string fname);
		static void DefaultLang(std::string &lang);
		static bool isBase64(std::string &s);
		static void LoadFileStr(std::string file, std::string&str);
		static bool isValidIPv4(std::string &ipv4);
		static void trimString(std::string & str);
		static void Print(string info);
		static string Tolower(const char * str);
		static void Tolower(std::string &str);
	};

	using hash_t = uint64_t;
	constexpr hash_t prime = 0x100000001B3ull;
	constexpr hash_t basis = 0xCBF29CE484222325ull;
	inline hash_t hash_(char const* str)
	{
		hash_t ret{ basis };
		while (*str)
		{
			ret ^= *str;
			ret *= prime;
			str++;
		}
		return ret;
	}
	inline hash_t hash_(const std::string &str)
	{
		return hash_(str.data());
	}
	constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
	{
		return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
	}
	constexpr unsigned long long operator "" _hash(char const* p, size_t)
	{
		return hash_compile_time(p);
	}

	inline bool strFind(const std::string &str, const std::string &target)
	{
		return str.find(target) != str.npos;
	}

	inline bool startsWith(const std::string &hay, const std::string &needle)
	{
		return hay.substr(0, needle.length()) == needle;
	}

	inline bool endsWith(const std::string &hay, const std::string &needle)
	{
		std::string::size_type hl = hay.length(), nl = needle.length();
		return hl >= nl && hay.substr(hl - nl, nl) == needle;
	}
	inline bool isIPv4(const std::string &address)
	{
		return true;
	}

	inline bool isIPv6(const std::string &address)
	{
		return true;
	}
	inline std::string replaceAllDistinct(std::string str, const std::string &old_value, const std::string &new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
		{
			if ((pos = str.find(old_value, pos)) != std::string::npos)
				str.replace(pos, old_value.length(), new_value);
			else
				break;
		}
		return str;
	}
	inline std::string urlSafeBase64Reverse(const std::string &encoded_string)
	{
		return replaceAllDistinct(replaceAllDistinct(encoded_string, "-", "+"), "_", "/");
	}

	inline std::string urlSafeBase64Apply(const std::string &encoded_string)
	{
		return replaceAllDistinct(replaceAllDistinct(replaceAllDistinct(encoded_string, "+", "-"), "/", "_"), "=", "");
	}

	inline std::string urlSafeBase64Decode(const std::string &encoded_string)
	{
		return base64_decode(encoded_string, true);
	}

	inline std::string urlSafeBase64Encode(const std::string &string_to_encode)
	{
		return urlSafeBase64Apply(base64_encode(string_to_encode));
	}
	inline std::string trimOf(const std::string& str, char target, bool before = true, bool after = true)
	{
		if (!before && !after)
			return str;
		std::string::size_type pos = 0;
		if (before)
			pos = str.find_first_not_of(target);
		if (pos == std::string::npos)
		{
			return str;
		}
		std::string::size_type pos2 = str.size() - 1;
		if (after)
			pos2 = str.find_last_not_of(target);
		if (pos2 != std::string::npos)
		{
			return str.substr(pos, pos2 - pos + 1);
		}
		return str.substr(pos);
	}
	inline std::string trim(const std::string& str, bool before=true, bool after=true)
	{
		return trimOf(str, ' ', before, after);
	}
	inline int to_int(const std::string &str,int def_value = 0)
	{
		if (str.empty())
			return def_value;
		return std::atoi(str.data());
	}
#endif
