#include "strCoding.h"
#include <iostream>  
#include <fstream>
#include <string>
#include <sstream>
#include <memory> 

#include <cctype>
#include <algorithm>
#include <functional>

#include <shlobj.h>//选目录
#include <commdlg.h>
#include <direct.h>
#include <Shlobj.h>//高版本选目录
#include <tchar.h>

using namespace std;
#define CHUNK 16384
	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	template<typename T>
	void RTimer::setTimeout(T function, int delay) {
		this->clear = false;
		std::thread t([=]() {
			if (this->clear) return;
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			if (this->clear) return;
			function();
		});
		t.detach();
	}

	template<typename T>
	void RTimer::setInterval(T function, int interval) {
		this->clear = false;
		std::thread t([=]() {
			while (true) {
				if (this->clear) return;
				std::this_thread::sleep_for(std::chrono::milliseconds(interval));
				if (this->clear) return;
				function();
			}
		});
		t.detach();
	}

	void RTimer::stop() {
		this->clear = true;
	}
	strCoding::strCoding(void)
	{
	}
	strCoding::~strCoding(void)
	{
	}
	char* exe_file_path(void* (*allocator)(size_t)) {
		char* buf = NULL;
		size_t bufsize = 0;
		do {
			if (allocator == NULL) {
				break;
			}

#if defined(_WIN32)
			bufsize = MAX_PATH;
			buf = (char*)allocator(bufsize);
			if (buf == NULL) {
				break;
			}
			memset(buf, 0, bufsize);
			if (GetModuleFileNameA(NULL, buf, (DWORD)bufsize) == 0) {
				memset(buf, 0, bufsize);
			}
			break;

#elif TARGET_OS_IPHONE
			break;

#elif defined(__APPLE__)
			bufsize = MAXPATHLEN;
			buf = (char*)allocator(bufsize);
			if (buf == NULL) {
				break;
			}
			memset(buf, 0, bufsize);
			if (_NSGetExecutablePath(buf, (uint32_t*)&bufsize) != 0) {
				memset(buf, 0, bufsize);
			}
			break;

#elif defined(__unix__)
			bufsize = 256 * 2;
			buf = (char*)allocator(bufsize);
			if (buf == NULL) {
				break;
			}
			memset(buf, 0, bufsize);
			readlink(SELF_EXE, buf, bufsize);
			break;

#else
			break;
#endif
		} while (0);

		return buf;
	}

	void strCoding::Gb2312ToUnicode(WCHAR* pOut,const char *gbBuffer)
	{
		::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
		return;
	}
	void strCoding::UTF_8ToUnicode(WCHAR* pOut, char *pText)
	{
		char* uchar = (char *)pOut;
		uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
		uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);
		return;
	}
	void strCoding::UnicodeToUTF_8(char* pOut, WCHAR* pText)
	{
		// 注意 WCHAR高低字的顺序,低字节在前，高字节在后
		char* pchar = (char *)pText;
		pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
		pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
		pOut[2] = (0x80 | (pchar[0] & 0x3F));
		return;
	}
	void strCoding::UnicodeToGB2312(char* pOut, WCHAR uData)
	{
		WideCharToMultiByte(CP_ACP, NULL, &uData, 1, pOut, sizeof(WCHAR), NULL, NULL);
		return;

	}
	//做为解Url使用
	char strCoding::CharToInt(char ch) {
		if (ch >= '0' && ch <= '9')return (char)(ch - '0');
		if (ch >= 'a' && ch <= 'f')return (char)(ch - 'a' + 10);
		if (ch >= 'A' && ch <= 'F')return (char)(ch - 'A' + 10);
		return -1;
	}
	char strCoding::StrToBin(char *str) {
		char tempWord[2];
		char chn;
		tempWord[0] = strCoding::CharToInt(str[0]);                         //make the B to 11 -- 00001011
		tempWord[1] = strCoding::CharToInt(str[1]);                         //make the 0 to 0 -- 00000000
		chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000
		return chn;
	}
	//UTF_8 转gb2312
	void strCoding::UTF_8ToGB2312(std::string &pOut, char *pText, int pLen)
	{
		char buf[4];
		char* rst = new char[pLen + (pLen >> 2) + 2];
		memset(buf, 0, 4);
		memset(rst, 0, pLen + (pLen >> 2) + 2);
		int i = 0;
		int j = 0;
		while (i < pLen)
		{
			if (*(pText + i) >= 0)
			{
				rst[j++] = pText[i++];
			}
			else
			{
				WCHAR Wtemp;
				strCoding::UTF_8ToUnicode(&Wtemp, pText + i);
				strCoding::UnicodeToGB2312(buf, Wtemp);
				unsigned short int tmp = 0;
				tmp = rst[j] = buf[0];
				tmp = rst[j + 1] = buf[1];
				tmp = rst[j + 2] = buf[2];
				//newBuf[j] = Ctemp[0];
				//newBuf[j + 1] = Ctemp[1];
				i += 3;
				j += 2;
			}
		}
		rst[j] = '\0';
		pOut = rst;
		delete[]rst;
	}
	//GB2312 转为 UTF-8
	void strCoding::GB2312ToUTF_8(std::string& pOut, const char *pText, int pLen)
	{
		char buf[4];
		memset(buf, 0, 4);
		pOut.clear();
		int i = 0;
		while (i < pLen)
		{
			//如果是英文直接复制就可以
			if (pText[i] >= 0)
			{
				char asciistr[2] = { 0 };
				asciistr[0] = (pText[i++]);
				pOut.append(asciistr);
			}
			else
			{
				WCHAR pbuffer;
				Gb2312ToUnicode(&pbuffer, pText + i);
				UnicodeToUTF_8(buf, &pbuffer);
				pOut.append(buf);
				i += 2;
			}
		}
		return;
	}
	//把str编码为网页中的 GB2312 url encode ,英文不变，汉字双字节 如%3D%AE%88
	std::string strCoding::UrlGB2312(const char * str)
	{
		std::string dd;
		size_t len = strlen(str);
		for (size_t i = 0; i < len; i++)
		{
			if (isalnum((BYTE)str[i]))
			{
				char tempbuff[2];
				sprintf_s(tempbuff, "%c", str[i]);
				dd.append(tempbuff);
			}
			else if (isspace((BYTE)str[i]))
			{
				dd.append("+");
			}
			else
			{
				char tempbuff[4];
				sprintf_s(tempbuff, "%%%X%X", ((BYTE*)str)[i] >> 4, ((BYTE*)str)[i] % 16);
				dd.append(tempbuff);
			}
		}
		return dd;
	}
	//把str编码为网页中的 UTF-8 url encode ,英文不变，汉字三字节 如%3D%AE%88
	std::string strCoding::UrlUTF8(const char * str)
	{
		std::string tt;
		std::string dd;
		GB2312ToUTF_8(tt, str, (int)strlen(str));
		size_t len = tt.length();
		for (size_t i = 0; i < len; i++)
		{
			if (isalnum((BYTE)tt.at(i)))
			{
				char tempbuff[2] = { 0 };
				sprintf_s(tempbuff, "%c", (BYTE)tt.at(i));
				dd.append(tempbuff);
			}
			else if (isspace((BYTE)tt.at(i)))
			{
				dd.append("+");
			}
			else
			{
				char tempbuff[4];
				sprintf_s(tempbuff, "%%%X%X", ((BYTE)tt.at(i)) >> 4, ((BYTE)tt.at(i)) % 16);
				dd.append(tempbuff);
			}
		}
		return dd;
	}
	//把url GB2312解码
	std::string strCoding::UrlGB2312Decode(std::string str)
	{
		std::string output = "";
		char tmp[2];
		int i = 0, idx = 0, ndx = 0, len = str.length();
		while (i < len) {
			if (str[i] == '%') {
				tmp[0] = str[i + 1];
				tmp[1] = str[i + 2];
				output += strCoding::StrToBin(tmp);
				i = i + 3;
			}
			else if (str[i] == '+') {
				output += ' ';
				i++;
			}
			else {
				output += str[i];
				i++;
			}
		}
		return output;
	}
	//把url utf8解码
	std::string strCoding::UrlUTF8Decode(std::string str)
	{
		std::string output = "";
		std::string temp = strCoding::UrlGB2312Decode(str);//
		strCoding::UTF_8ToGB2312(output, (char *)temp.data(), strlen(temp.data()));
		return output;
	}
	char * strCoding::urlDecode(const char *str) {
		int d = 0; /* whether or not the string is decoded */
		char *dStr =(char *)malloc(strlen(str) + 1);
		char eStr[] = "00"; /* for a hex code */
		strcpy_s(dStr,strlen(str),str);
		while (!d) {
			d = 1;
			int i; /* the counter for the string */
			for (i = 0; i<strlen(dStr); ++i) {

				if (dStr[i] == '%') {
					if (dStr[i + 1] == 0)
						return dStr;
					if (isxdigit(dStr[i + 1]) && isxdigit(dStr[i + 2])) {
						d = 0;
						/* combine the next to numbers into one */
						eStr[0] = dStr[i + 1];
						eStr[1] = dStr[i + 2];
						/* convert it to decimal */
						long int x = strtol(eStr, NULL, 16);

						/* remove the hex */
						memmove(&dStr[i + 1], &dStr[i + 3], strlen(&dStr[i + 3]) + 1);
						dStr[i] = x;
					}
				}
			}
		}
		return dStr;
	}

	//gbk ת utf8
	std::string strCoding::GBKToUTF8(const std::string& strGBK)
	{
		std::string strOutUTF8 = "";
		WCHAR * str1;
		int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
		str1 = new WCHAR[n];
		MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
		n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
		char * str2 = new char[n];
		WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
		strOutUTF8 = str2;
		delete[]str1;
		str1 = NULL;
		delete[]str2;
		str2 = NULL;
		return strOutUTF8;
	}

	//utf-8 ת gbk
	std::string strCoding::UTF8ToGBK(const std::string& strUTF8)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
		unsigned short * wszGBK = new unsigned short[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, (LPSTR)szGBK, len, NULL, NULL);
		//strUTF8 = szGBK;
		std::string strTemp(szGBK);
		delete[]szGBK;
		delete[]wszGBK;
		return strTemp;
	}

	std::string strCoding::UnicodeToUTF8(const std::wstring & wstr)
	{
		std::string ret;
		try {
			std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
			ret = wcv.to_bytes(wstr);
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
		return ret;
	}

	std::wstring strCoding::UTF8ToUnicode(const std::string & str)
	{
		std::wstring ret;
		try {
			std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
			ret = wcv.from_bytes(str);
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
		return ret;
	}

	bool WideStringToString(const std::wstring& src, std::string &str)
	{
		std::locale sys_locale("");

		const wchar_t* data_from = src.c_str();
		const wchar_t* data_from_end = src.c_str() + src.size();
		const wchar_t* data_from_next = 0;

		int wchar_size = 4;
		char* data_to = new char[(src.size() + 1) * wchar_size];
		char* data_to_end = data_to + (src.size() + 1) * wchar_size;
		char* data_to_next = 0;

		memset(data_to, 0, (src.size() + 1) * wchar_size);

		typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
		mbstate_t out_state = { 0 };
		auto result = std::use_facet<convert_facet>(sys_locale).out(
			out_state, data_from, data_from_end, data_from_next,
			data_to, data_to_end, data_to_next);
		if (result == convert_facet::ok)
		{
			str = data_to;
			delete[] data_to;
			return true;
		}
		delete[] data_to;
		return false;
	}

	bool strCoding::StringToWideString(const std::string& src, std::wstring &wstr)
	{
		std::locale sys_locale("");
		const char* data_from = src.c_str();
		const char* data_from_end = src.c_str() + src.size();
		const char* data_from_next = 0;

		wchar_t* data_to = new wchar_t[src.size() + 1];
		wchar_t* data_to_end = data_to + src.size() + 1;
		wchar_t* data_to_next = 0;

		wmemset(data_to, 0, src.size() + 1);

		typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
		mbstate_t in_state = { 0 };
		auto result = std::use_facet<convert_facet>(sys_locale).in(
			in_state, data_from, data_from_end, data_from_next,
			data_to, data_to_end, data_to_next);
		if (result == convert_facet::ok)
		{
			wstr = data_to;
			delete[] data_to;
			return true;
		}
		delete[] data_to;
		return false;
	}


	bool WCharStringToUTF8String(const std::wstring &wstr, std::string &u8str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		u8str = conv.to_bytes(wstr);
		return true;
	}

	bool UTF8StringToWCharString(const std::string &u8str, std::wstring &wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
		wstr = conv.from_bytes(u8str);
		return true;
	}
	BOOL strCoding::IsUTF8(const char* str, ULONGLONG length)
	{
		DWORD nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
		UCHAR chr;
		BOOL bAllAscii = TRUE; //如果全部都是ASCII, 说明不是UTF-8
		for (int i = 0; i < length; ++i)
		{
			chr = *(str + i);
			if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
				bAllAscii = FALSE;
			if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
			{
				if (chr >= 0x80)
				{
					if (chr >= 0xFC && chr <= 0xFD)
						nBytes = 6;
					else if (chr >= 0xF8)
						nBytes = 5;
					else if (chr >= 0xF0)
						nBytes = 4;
					else if (chr >= 0xE0)
						nBytes = 3;
					else if (chr >= 0xC0)
						nBytes = 2;
					else
						return FALSE;

					nBytes--;
				}
			}
			else //多字节符的非首字节,应为 10xxxxxx
			{
				if ((chr & 0xC0) != 0x80)
					return FALSE;

				nBytes--;
			}
		}
		//if (nBytes > 0) //违返规则,不能用，有时可能混有Ascii或少取了一个字符
		//	return FALSE;
		if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
			return FALSE;

		return TRUE;
	}

	std::string strCoding::codeDecode(const char * str)
	{//&#20307;内码格式转成汉字，先把内码拆成高八位和低八位，汉字编码模式
	 //int a = 54490;//在
	 //char * str = "dfgd&#x4e2d; &#x56fd;f";//中国 十六进制unicode
	 //char * str = "asdf&#20013;sd &#22269;sdf";//中国 十进制unicode
		unsigned char buf[2] = { 0 };
		char ch[7] = { 0 };
		int len = strlen(str);
		//vector <WCHAR> wstr;
		std::wstring wstr;
		std::string val = "";
		bool ishex = false;
		for (int i = 0; i + 1 < len; i++)
		{
			if (str[i] == '&'&&str[i + 1] == '#')
			{//一个汉字编码开始
				ch[0] = '\0';
				int m = 0;
				for (i = i + 2; i + 1 < len; i++)
				{//取出内码
					if (str[i] != ';' && (str[i] >= '0'&& str[i] <= '9' || ishex&&tolower(str[i]) >= 'a'&&tolower(str[i]) <= 'f') && m < 7)
						ch[m++] = str[i];
					else
						if (str[i] == 'x')//十六进制
							ishex = true;
						else
							break;
				}
				ch[m] = '\0';
				if (m > 0)
				{
					int code = 0;
					if (ishex)
						code = std::stoi(ch, nullptr, 16);
					else
						code = std::stoi(ch, nullptr, 10);
					//wchar_t wch = code;
					//buf[0] = (code >> 8) & 0XFF;//将a左移8个单位在和0xff与，取a的前8位
					//buf[1] = code & 0XFF;//将a和0xff与，取a的后8位
					//int da = buf[0] << 8 | buf[1];//得到汉字内码
					//wstr.push_back(wch);
					wstr.push_back((wchar_t)code);
				}
			}
			else
			{
				wstr.push_back((wchar_t)str[i]);
			}
		}
		WCharStringToUTF8String(wstr, val);
		return val;
	}
	 std::string strCoding::ToLower(const std::string &s)
	{
		std::string ret = s;
		for (char &c : ret)
			c = static_cast<char>(tolower(c));
		return ret;
	}
#if _WIN32
	std::wstring strCoding::str2wstr(std::string const &str)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
		std::wstring ret(len, '\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPWSTR)ret.data(), (int)ret.size());
		return ret;
	}

	std::string strCoding::wstr2str(std::wstring const &str)
	{
		int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
		std::string ret(len, '\0');
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPSTR)ret.data(), (int)ret.size(), nullptr, nullptr);
		return ret;
	}

	bool strCoding::is_vista()
	{//vista及以上的都是true
		OSVERSIONINFOEXW osvi;
		memset(&osvi, 0, sizeof(osvi));
		DWORDLONG const mask = VerSetConditionMask(
			VerSetConditionMask(
				VerSetConditionMask(
					0, VER_MAJORVERSION, VER_GREATER_EQUAL),
				VER_MINORVERSION, VER_GREATER_EQUAL),
			VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
		osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
		osvi.wServicePackMajor = 0;

		return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, mask) != FALSE;
	}
#endif

	//写csv文件 write_csv("three_cols_copy.csv", three_cols);
	void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<int>>> dataset) {
		// Make a CSV file with one or more columns of integer values
		// Each column of data is represented by the pair <column name, column data>
		//   as std::pair<std::string, std::vector<int>>
		// The dataset is represented as a vector of these columns
		// Note that all columns should be the same size

		// Create an output filestream object
		std::ofstream myFile(strCoding::Absolutepath(filename));

		// Send column names to the stream
		for (int j = 0; j < dataset.size(); ++j)
		{
			myFile << dataset.at(j).first;
			if (j != dataset.size() - 1) myFile << ","; // No comma at end of line
		}
		myFile << "\n";

		// Send data to the stream
		for (int i = 0; i < dataset.at(0).second.size(); ++i)
		{
			for (int j = 0; j < dataset.size(); ++j)
			{
				myFile << dataset.at(j).second.at(i);
				if (j != dataset.size() - 1) myFile << ","; // No comma at end of line
			}
			myFile << "\n";
		}

		// Close the file
		myFile.close();
	}
	//读csv std::vector<std::pair<std::string, std::vector<int>>> three_cols = read_csv("three_cols.csv");
	std::vector<std::pair<std::string, std::vector<int>>> read_csv(std::string filename) {
		// Reads a CSV file into a vector of <string, vector<int>> pairs where
		// each pair represents <column name, column values>

		// Create a vector of <string, int vector> pairs to store the result
		std::vector<std::pair<std::string, std::vector<int>>> result;
		// Create an input filestream
		std::ifstream myFile(filename);
		// Make sure the file is open
		if (!myFile.is_open()) throw std::runtime_error("Could not open file");
		// Helper vars
		std::string line, colname;
		int val;
		// Read the column names
		if (myFile.good())
		{
			// Extract the first line in the file
			std::getline(myFile, line);
			// Create a stringstream from line
			std::stringstream ss(line);
			// Extract each column name
			while (std::getline(ss, colname, ',')) {
				// Initialize and add <colname, int vector> pairs to result
				result.push_back({ colname, std::vector<int> {} });
			}
		}
		// Read data, line by line
		while (std::getline(myFile, line))
		{
			// Create a stringstream of the current line
			std::stringstream ss(line);
			// Keep track of the current column index
			int colIdx = 0;
			// Extract each integer
			while (ss >> val) {

				// Add the current integer to the 'colIdx' column's values vector
				result.at(colIdx).second.push_back(val);
				// If the next token is a comma, ignore it and move on
				if (ss.peek() == ',') ss.ignore();
				// Increment the column index
				colIdx++;
			}
		}
		// Close file
		myFile.close();
		return result;
	}
	bool strCoding::StartsWith(const std::string ostr, const std::string str)
	{//判断是不是以str开头
		if (ostr.length() < str.length())
			return false;
		for (int i = 0; i < str.length(); i++)
		{
			if (ostr[i] != str[i])
				return false;
		}
		return true;
	}
	void strCoding::split(std::string line, std::vector<std::string> &words, char delimiter) {
		int index = 0;
		while ((index = line.find(delimiter)) != -1) {
			words.push_back(line.substr(0, index));
			if (index + 1 == line.length())
			{
				words.push_back("");//最后一项是空值的的时间补上
				return;
			}
			line = line.substr(index + 1);
		}
		if (!line.empty())
			words.push_back(line);
	}
	std::vector<std::string> strCoding::split(std::string string, std::string delim, int count)
	{
		std::vector<std::string> rv;

		size_t n = string.find(delim);

		while (n != std::string::npos && count != 0) {
			//Add the first part of the string to the vector
			rv.push_back(string.substr(0, n));
			//Erase the cut part (we include the delimnator this time
			string.erase(0, n + delim.length());

			//Set the variables
			n = string.find(delim);
			count--;
		}

		//Add the rest of the string to the vector, if not empty
		if (!string.empty()) rv.push_back(string);

		return rv;
	}
	 std::vector<std::string> strCoding::Split(std::string str, char delimiter) {
		std::vector<std::string> internal;
		std::stringstream ss(str); // Turn the string into a stream.
		std::string tok;
		while (getline(ss, tok, delimiter)) {
			internal.push_back(tok);
		}
		if (str.length()>0&&str[str.length()-1]== delimiter)
		{//最后一项是空值的补上
			internal.push_back("");
		}
		return internal;
	}
	 string strCoding::Tolower(const char * str)
	 {
		 string strtmp = str;
		 Tolower(strtmp);
		 return strtmp;
	  }
	 void strCoding::Tolower(std::string &str)
	 {
		  transform(str.begin(), str.end(), str.begin(), ::tolower);//全转成小写::toupper转大写
	 }
	 std::unordered_map<std::string, std::string> strCoding::GetAttributes(std::string str, std::unordered_map<std::string, std::string> &rv,char delimiter)
	 {
		 //char delimiter = ';';
		// std::unordered_map<std::string, std::string> rv;
		 std::stringstream ss(str); // Turn the string into a stream.
		 std::string tok;
		 while (getline(ss, tok, delimiter)) {
			 std::size_t found = tok.find(':');
			 if (found != std::string::npos)
			 {
				 if (tok[tok.length() - 1] == '\r' || tok[tok.length() - 1] == '\n')//\r\n连用只去掉了一个
					 tok = tok.substr(0, tok.length() - 1);
				 string name = tok.substr(0, found);
				 transform(name.begin(), name.end(), name.begin(), ::tolower);//全转成小写::toupper转大写
				 if (rv.count(name) == 0)
				 {//去掉值的前后空格 
					 string val = tok.substr(found + 1);
					 while(val.length()>0&& val[0]==' ')
						 val=val.substr(1);
					 while (val.length()>0 && val[val.length()-1] == ' ')
						 val=val.substr(0, val.length() - 1);
					 rv.insert(std::unordered_map<std::string, std::string>::value_type(name, val));
				 }
			 }
		 }

		 if (str.length() > 0 && str[str.length() - 1] == delimiter)//最后一项是空值的补上
			 if (rv.count("") == 0)
				 rv.insert(std::unordered_map<std::string, std::string>::value_type("", ""));

		 return rv;
	 }
	 std::string strCoding::concat(std::vector<std::string> args, std::string delim, int beginOffset, int endOffset)
	 {
		 std::ostringstream os;
		 //Std::copy expects a pointer to 1 past the last element of an array. So, by subtracting one here, it copies all but the last.
		 std::copy(args.begin() + beginOffset, args.end() - endOffset,
			 std::ostream_iterator<std::string>(os, delim.c_str()));
		 //args.end() returns a pointer to 1 past the last element of an array. So, we need to subtract one.
		 os << *(args.end() - 1);

		 return os.str();
	 }
	
	 void strCoding::string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
	 {//把strBig字符串的strsrc替换成strdst,如std::string strContent = "This is a Test"; string_replace(strContent, "Test", "demo");
		 std::string::size_type pos = 0;
		 std::string::size_type srclen = strsrc.size();
		 std::string::size_type dstlen = strdst.size();

		 while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
		 {
			 strBig.replace(pos, srclen, strdst);
			 pos += dstlen;
		 }
	 }
	 std::string strCoding::UrlDecode(std::string &url)
	 {//网址解码，需测试
		 return UrlUTF8Decode(url);
	 }
	 void strCoding::GetQuery(std::string result, std::unordered_map<string, string> &words)
	 {//取出链接中的所有参数
		 int index = 0;
		 std::size_t found = result.find('?');
		 if (found != std::string::npos)
		 {
			 std::string queryStr = result.substr(found + 1);
			 while ((index = queryStr.find('&')) != -1) {
				 std::string line = queryStr.substr(0, index);
				 found = line.find('=');
				 if (found != std::string::npos)
					 words.insert(std::unordered_map<std::string, std::string>::value_type(line.substr(0, found), line.substr(found + 1)));
				 queryStr = queryStr.substr(index + 1);
			 }
			 if (!queryStr.empty())
			 {
				 found = queryStr.find('=');
				 if (found != std::string::npos)
					 words.insert(std::unordered_map<std::string, std::string>::value_type(queryStr.substr(0, found), queryStr.substr(found + 1)));
			 }
		 }
	 }
	 string strCoding::Absolutepath(const char * fname)
	 {//根据文件名组合成当前执行目录下的绝对路径，只是简单相加，是子目录的需要加上子目录名。保存和打开都要用绝对路径。保存时会自动保存到上一次选文件的目录下。如导入时选了文件则会保存到导入文件的目录下，因为有绶存。
		 string filepath = fname;
		 if (strstr(fname, ":") == NULL)
		 {
			 ExeFilePath(filepath);
			 filepath.append(fname);//要用绝对路径，不然会保存到上一次选文件的目录下。如导入时选了文件则会保存到导入文件的目录下，因为有绶存。
		 }
		 return filepath;
	 }
	 string strCoding::Absolutepath(std::string fname)
	 {
		 return Absolutepath(fname.c_str());
	 }
	 void strCoding::Getvector(const char * fname,std::vector<std::string>&sp)
	 {
		 ifstream infile(fname, ios_base::in | ios_base::binary);
		// std::vector<string> sp;
		 if (!infile)
			 return;
		 std::string line;
		 while (std::getline(infile, line))
		 {
			 if (line.length() > 0 && (line[line.length() - 1] == '\n' || line[line.length() - 1] == '\r'))
				 sp.push_back(line.substr(0, line.length() - 1));
			 else
				 sp.push_back(line);
		 }
		 infile.close();
	 }
	 void strCoding::Getmap(const char * fname, std::unordered_map<std::string, bool>&sp)
	 {
		 ifstream infile(fname, ios_base::in | ios_base::binary);
		 if (!infile)
		 {
			 DWORD err = GetLastError();
			 return;
		 }
		 std::string line;
		 while (std::getline(infile, line))
		 {
			 if (line.length() > 0 && (line[line.length() - 1] == '\n' || line[line.length() - 1] == '\r'))
				 sp.insert(std::unordered_map<string, bool>::value_type(line.substr(0, line.length() - 1), true));
			 else
				 sp.insert(std::unordered_map<string, bool>::value_type(line, true));
		 }
		 infile.close();
	 }
	 bool strCoding::fileExists(const std::string& filename)
	 {
		 string path = filename;
		 if (path.find(":") == std::string::npos)
		 {
			 ExeFilePath(path);
			 path = path + filename;
		 }
		 struct stat buf;
		 if (stat(path.c_str(), &buf) != -1)
		 {
			 return true;
		 }
		 return false;
	 }

	 void strCoding::ltrim(std::string &string)
	 {
		 string.erase(string.begin(),
			 std::find_if(string.begin(), string.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	 }

	 void strCoding::rtrim(std::string &string)
	 {
		 string.erase(
			 std::find_if(string.rbegin(), string.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			 string.end());
	 }

	 void strCoding::trim(std::string &string)
	 {
		 ltrim(string);
		 rtrim(string);
	 }

	 bool strCoding::beginsWith(std::string str, std::string begin)
	 {
		 return str.substr(0, begin.length()) == begin;
	 }

	 bool strCoding::endsWith(std::string str, std::string end)
	 {
		 size_t beginIndex = str.length() - end.length();
		 return str.substr(beginIndex, end.length()) == end;
	 }
	 void strCoding::replace(std::string &src, std::string sub, std::string replace, int count)
	 {
		 while (count >0) {
			 size_t index = 0;
			 ///Find the substring
			 index = src.find(sub, index);
			 if (index == std::string::npos) break;

			 //Replace
			 src.replace(index, sub.length(), replace);

			 //Move the variables
			 index += sub.length();
			 count--;
		 }
	 }
	 void strCoding::replace(std::string &src, char ch, char newchar)
	 {//字符替换,保存数据时用逗号作分隔符，需去掉
		 for (int i = 0; i < src.length(); i++)
		 {
			 if (src[i] == ch)
				 src[i] = newchar;
		 }
	 }
	 bool strCoding::Saveline(std::vector<std::string> args, std::string filename)
	 {
		 std::ofstream os;
		 wstring wstr = strCoding::str2wstr(Absolutepath(filename.c_str()));
		 os.open(wstr, std::ofstream::binary);
		 if (os.is_open()) {
			 int totalItem = args.size();
			 for (int i = 0; i < totalItem; i++)
			 {
				 os << args[i] << "\r\n";
			 }
			 os.close();
		 }
		 else
			 return false;
		 return true;
	 }
	 bool strCoding::Getline(std::vector<std::string> &args, std::string filename)
	 {
		 std::vector<string> lines;
		 strCoding::Getvector(filename.c_str(), lines);
		 for (auto line : lines)
		 {
			 if (line.length() < 8 || line.length() >MAX_PATH)
				 continue;
			 args.push_back(line);
		 }
		 return true;
	 }
	 void strCoding::ExeFilePath(string &path)
	 {
		 static string exepath = "";
		 if (exepath != "")
		 {
			 path = exepath;
			 return;
		 }
		 char* cfg_file = exe_file_path(&malloc);
		 char* separ = NULL;
		 if (cfg_file && ((separ = strrchr(cfg_file, PATH_SEPARATOR)))) {
			 ++separ;
			 separ[0] ='\0';
		 }
		 exepath = cfg_file;
		 path = cfg_file;
		 free(cfg_file);
	 }
	 void strCoding::DefaultLang(string &lang)
	 {
		 static string clang = "";
		 if (clang != "")
		 {
			 lang = clang;
			 return;
		 }
		 LCID lcid = GetThreadLocale();
		 wchar_t name[LOCALE_NAME_MAX_LENGTH];
		 if (LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0) == 0) {}
		 std::wstring ws(name);
		 std::string langstr(ws.begin(), ws.end());
		 clang = langstr;
		 lang = clang;
	 }
	 void strCoding::DatetimeString(std::string &str)
	 {
		 time_t t= time(NULL); //获取目前秒时间    
		 char buf[128] = { 0 };
		 tm local;
		 localtime_s(&local,&t); //转为本地时间  
		 strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &local);
		 str=buf;
	 }
	 time_t strCoding::StringToDatetime(const char *str)
	 {
		 tm tm_;
		 int year, month, day, hour, minute, second;
		 //char *cha = (char*)str.data();
		 sscanf_s(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);//直接从str串按格式提出数值
		 tm_.tm_year = year - 1900;
		 tm_.tm_mon = month - 1;
		 tm_.tm_mday = day;
		 tm_.tm_hour = hour;
		 tm_.tm_min = minute;
		 tm_.tm_sec = second;
		 tm_.tm_isdst = 0;//非夏令时。
		 time_t t_ = mktime(&tm_); //已经减了8个时区  
		 return t_; //秒时间  
	 }
	 bool strCoding::isBase64(std::string &s) {
		 //因取的是innerText后面可能有'\n'，不去掉isBase64计算长度会算进去，被认为不是64位编码
		 if(s.length()<4)
			 return false;
		 while (s.length()>0&&(s[s.length() - 1] == '\n' || s[s.length() - 1] == ' '))
			 s = s.substr(0,s.length()-1);

		 int len = s.length();
		 if (len % 4 != 0) {
			 return false;
		 }
		 if(len<4)
			 return false;
		 while (s[len - 1] == '=') len--;
		 for (int i = 0; i < len; i++) {
			 if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') || s[i] == '+' || s[i] == '/') {
				 continue;
			 }
			 return false;
		 }
		 return true;
	 }
	 void strCoding::LoadFileStr(string filename,string&str)
	 {//用来加载文件
		 DWORD tickCount = GetTickCount();
		 try {
			 std::ifstream infile;
			 wstring wstr = strCoding::str2wstr(filename);
			 infile.open((wstr), ios_base::in | ios_base::binary);
			 if (infile.is_open())
			 {
				 infile.seekg(0, ios::end);
				 long length = infile.tellg();
				 infile.seekg(0, ios::beg);//移回到流开始位置
				 if (length > 6000000000)
					 length = 6000000000;
				 char * buf = new char[length + 1];
				 infile.read(buf, length);
				 buf[length] = 0;
				 str = buf;
				 infile.close();
				 delete[] buf;
			 }
			 int tm = GetTickCount() - tickCount;
			 tickCount = 0;
		 }
		 catch (...) {
		 }
	 }

	 std::string removeSubstrs(std::string &input, std::string pattern)
	 {//移除子串
		 std::string source = input;
		 std::string::size_type n = pattern.length();
		 for (std::string::size_type i = source.find(pattern); i != std::string::npos; i = source.find(pattern))
		 {
			 source.erase(i, n);
		 }
		 return source;
	 }
	 std::string reverseIpAddress(std::string ip)
	 {//倒装IP，如192.168.0.1-》1.0.168.192
		 std::vector<std::string> octeti;
		 strCoding::split(ip, octeti, ',');
		 return (octeti[3] + "." + octeti[2] + "." + octeti[1] + "." + octeti[0]);
	 }

	 unsigned long stripToDecimal(std::string &ip)
	 {//IP转整数
		 unsigned long a, b, c, d, base10IP;
		 sscanf_s(ip.c_str(), "%lu.%lu.%lu.%lu", &a, &b, &c, &d);
		 // Do calculations to convert IP to base 10
		 a *= 16777216;
		 b *= 65536;
		 c *= 256;
		 base10IP = a + b + c + d;
		 return base10IP;
	 }

	 std::string decimalToDottedIp(unsigned long ipAddr)
	 {//整数转IP
		 unsigned short a, b, c, d;
		 stringstream stream;
		 std::string ip = "";

		 a = (ipAddr & (0xff << 24)) >> 24;
		 b = (ipAddr & (0xff << 16)) >> 16;
		 c = (ipAddr & (0xff << 8)) >> 8;
		 d = ipAddr & 0xff;

		 stream << d << "." << c << "." << b << "." << a;
		 ip = stream.str();

		 return ip;
	 }
	 bool strCoding::isValidIPv4(std::string &ipv4)
	 {//判断是不是有效IPV4
		 const std::string address = ipv4;

		 std::vector<std::string> arr;
		 int k = 0;
		 arr.push_back(std::string());
		 for (std::string::const_iterator i = address.begin(); i != address.end(); ++i)
		 {
			 if (*i == '.')
			 {
				 ++k;
				 arr.push_back(std::string());
				 if (k == 4)
				 {
					 return false;
				 }
				 continue;
			 }
			 if (*i >= '0' && *i <= '9')
			 {
				 arr[k] += *i;
			 }
			 else
			 {
				 return false;
			 }
			 if (arr[k].size() > 3)
			 {
				 return false;
			 }
		 }

		 if (k != 3)
		 {
			 return false;
		 }
		 for (int i = 0; i != 4; ++i)
		 {
			 const char* nPtr = arr[i].c_str();
			 char* endPtr = 0;
			 const unsigned long a = ::strtoul(nPtr, &endPtr, 10);
			 if (nPtr == endPtr)
			 {
				 return false;
			 }
			 if (a > 255)
			 {
				 return false;
			 }
		 }
		 return true;
	 }
	 void strCoding::trimString(std::string & str)
	 {//去除前后空格
		 int s = str.find_first_not_of(" ");
		 int e = str.find_last_not_of(" ");
		 str = str.substr(s, e - s + 1);
	 }
	 void strCoding::Print(string info)
	 {//在调试区输出信息
#ifdef _DEBUG
		 OutputDebugStringA(info.c_str());
#endif
	 }
	 bool isStrUTF8(const std::string &data)
	 {
		 const char *str = data.c_str();
		 unsigned int nBytes = 0;
		 for (unsigned int i = 0; str[i] != '\0'; ++i)
		 {
			 unsigned char chr = *(str + i);
			 if (nBytes == 0)
			 {
				 if (chr >= 0x80)
				 {
					 if (chr >= 0xFC && chr <= 0xFD)
						 nBytes = 6;
					 else if (chr >= 0xF8)
						 nBytes = 5;
					 else if (chr >= 0xF0)
						 nBytes = 4;
					 else if (chr >= 0xE0)
						 nBytes = 3;
					 else if (chr >= 0xC0)
						 nBytes = 2;
					 else
						 return false;
					 nBytes--;
				 }
			 }
			 else
			 {
				 if ((chr & 0xC0) != 0x80)
					 return false;
				 nBytes--;
			 }
		 }
		 if (nBytes != 0)
			 return false;
		 return true;
	 }