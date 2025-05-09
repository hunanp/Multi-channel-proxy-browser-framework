/*
   base64.cpp and base64.h

   base64 encoding and decoding with C++.
   More information at
     https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp

   Version: 2.rc.08 (release candidate)

   Copyright (C) 2004-2017, 2020, 2021 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include "base64.h"

#include <algorithm>
#include <stdexcept>

#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */
 //
 // Depending on the url parameter in base64_chars, one of
 // two sets of base64 characters needs to be chosen.
 // They differ in their last two characters.
 //
static const char* base64_chars[2] = {
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "+/",

             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "-_"};

static unsigned int pos_of_char(const unsigned char chr) {
 //
 // Return the position of chr within base64_encode()
 //

    if      (chr >= 'A' && chr <= 'Z') return chr - 'A';
    else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A')               + 1;
    else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
    else if (chr == '+' || chr == '-') return 62; // Be liberal with input and accept both url ('-') and non-url ('+') base 64 characters (
    else if (chr == '/' || chr == '_') return 63; // Ditto for '/' and '_'
    else
 //
 // 2020-10-23: Throw std::exception rather than const char*
 //(Pablo Martin-Gomez, https://github.com/Bouska)
 //
    throw std::runtime_error("Input is not valid base64-encoded data.");
}

static std::string insert_linebreaks(std::string str, size_t distance) {
 //
 // Provided by https://github.com/JomaCorpFX, adapted by me.
 //
    if (!str.length()) {
        return "";
    }

    size_t pos = distance;

    while (pos < str.size()) {
        str.insert(pos, "\n");
        pos += distance + 1;
    }

    return str;
}

template <typename String, unsigned int line_length>
static std::string encode_with_line_breaks(String s) {
  return insert_linebreaks(base64_encode(s, false), line_length);
}

template <typename String>
static std::string encode_pem(String s) {
  return encode_with_line_breaks<String, 64>(s);
}

template <typename String>
static std::string encode_mime(String s) {
  return encode_with_line_breaks<String, 76>(s);
}

template <typename String>
static std::string encode(String s, bool url) {
  return base64_encode(reinterpret_cast<const unsigned char*>(s.data()), s.length(), url);
}

std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url) {

    size_t len_encoded = (in_len +2) / 3 * 4;

    unsigned char trailing_char = url ? '.' : '=';

 //
 // Choose set of base64 characters. They differ
 // for the last two positions, depending on the url
 // parameter.
 // A bool (as is the parameter url) is guaranteed
 // to evaluate to either 0 or 1 in C++ therefore,
 // the correct character set is chosen by subscripting
 // base64_chars with url.
 //
    const char* base64_chars_ = base64_chars[url];

    std::string ret;
    ret.reserve(len_encoded);

    unsigned int pos = 0;

    while (pos < in_len) {
        ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

        if (pos+1 < in_len) {
           ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

           if (pos+2 < in_len) {
              ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
              ret.push_back(base64_chars_[  bytes_to_encode[pos + 2] & 0x3f]);
           }
           else {
              ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
              ret.push_back(trailing_char);
           }
        }
        else {

            ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
            ret.push_back(trailing_char);
            ret.push_back(trailing_char);
        }

        pos += 3;
    }


    return ret;
}

template <typename String>
static std::string decode(String encoded_string, bool remove_linebreaks) {
 //
 // decode(…) is templated so that it can be used with String = const std::string&
 // or std::string_view (requires at least C++17)
 //

    if (encoded_string.empty()) return std::string();

    if (remove_linebreaks) {

       std::string copy(encoded_string);

       copy.erase(std::remove(copy.begin(), copy.end(), '\n'), copy.end());

       return base64_decode(copy, false);
    }

    size_t length_of_string = encoded_string.length();
    size_t pos = 0;

 //
 // The approximate length (bytes) of the decoded string might be one or
 // two bytes smaller, depending on the amount of trailing equal signs
 // in the encoded string. This approximation is needed to reserve
 // enough space in the string to be returned.
 //
    size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
    std::string ret;
    ret.reserve(approx_length_of_decoded_string);

    while (pos+4 <= length_of_string) {//经常有不是4的倍数的，订阅链接少几个说明字符不影响
    //
    // Iterate over encoded input string in chunks. The size of all
    // chunks except the last one is 4 bytes.
    //
    // The last chunk might be padded with equal signs or dots
    // in order to make it 4 bytes in size as well, but this
    // is not required as per RFC 2045.
    //
    // All chunks except the last one produce three output bytes.
    //
    // The last chunk produces at least one and up to three bytes.
    //

       size_t pos_of_char_1 = pos_of_char(encoded_string[pos+1] );

    //
    // Emit the first output byte that is produced in each chunk:
    //
       ret.push_back(static_cast<std::string::value_type>( ( (pos_of_char(encoded_string[pos+0]) ) << 2 ) + ( (pos_of_char_1 & 0x30 ) >> 4)));

       if ( ( pos + 2 < length_of_string  )       &&  // Check for data that is not padded with equal signs (which is allowed by RFC 2045)
              encoded_string[pos+2] != '='        &&
              encoded_string[pos+2] != '.'            // accept URL-safe base 64 strings, too, so check for '.' also.
          )
       {
       //
       // Emit a chunk's second byte (which might not be produced in the last chunk).
       //
          unsigned int pos_of_char_2 = pos_of_char(encoded_string[pos+2] );
          ret.push_back(static_cast<std::string::value_type>( (( pos_of_char_1 & 0x0f) << 4) + (( pos_of_char_2 & 0x3c) >> 2)));

          if ( ( pos + 3 < length_of_string )     &&
                 encoded_string[pos+3] != '='     &&
                 encoded_string[pos+3] != '.'
             )
          {
          //
          // Emit a chunk's third byte (which might not be produced in the last chunk).
          //
             ret.push_back(static_cast<std::string::value_type>( ( (pos_of_char_2 & 0x03 ) << 6 ) + pos_of_char(encoded_string[pos+3])   ));
          }
       }

       pos += 4;
    }

    return ret;
}
bool isBase64(std::string s) {
	int len = s.length();
	if (len % 4 != 0) {
		return false;
	}
	while (s[len - 1] == '=') len--;
	for (int i = 0; i < len; i++) {
		if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') || s[i] == '+' || s[i] == '/') {
			continue;
		}
		return false;
	}
	return true;
}
std::string base64_decode(std::string const& s, bool remove_linebreaks) {
	if(s.length()<4)
		return "";
	
	if (s.length() % 4 != 0) {//不是4的倍数情况20210823改成去尾，保前成一部分,需测试
		std::string tmp = s.substr(0,s.length()- s.length()%4);
		if (isBase64(tmp))
			return decode(tmp, remove_linebreaks);//强制解，有的不规范，起码保证绝大部发是正确的
		else
			return  base64_decodeNoCheck(s);
	}
	if (isBase64(s))
		return decode(s, remove_linebreaks);
	else
		return base64_decodeNoCheck(s);
}

std::string base64_encode(std::string const& s, bool url) {
		return encode(s, url);
}

std::string base64_encode_pem (std::string const& s) {
   return encode_pem(s);
}

std::string base64_encode_mime(std::string const& s) {
   return encode_mime(s);
}
std::string base64_decodeNoCheck(const std::string &encoded_string, bool accept_urlsafe)
{
	string_size in_len = encoded_string.size();
	string_size i = 0;
	string_size in_ = 0;
	unsigned char char_array_4[4], char_array_3[3], uchar;
	static unsigned char dtable[256], itable[256], table_ready = 0;
	std::string ret;

	// Should not need thread_local with the flag...
	if (!table_ready)
	{
		// No memset needed for static/TLS
		for (string_size k = 0; k < base64_char.length(); k++)
		{
			uchar = base64_char[k]; // make compiler happy
			dtable[uchar] = k;  // decode (find)
			itable[uchar] = 1;  // is_base64
		}
		const unsigned char dash = '-', add = '+', under = '_', slash = '/';
		// Add urlsafe table
		dtable[dash] = dtable[add]; itable[dash] = 2;
		dtable[under] = dtable[slash]; itable[under] = 2;
		table_ready = 1;
	}

	while (in_len-- && (encoded_string[in_] != '='))
	{
		uchar = encoded_string[in_]; // make compiler happy
		if (!(accept_urlsafe ? itable[uchar] : (itable[uchar] == 1))) // break away from the while condition
		{
			ret += uchar; // not base64 encoded data, copy to result
			in_++;
			i = 0;
			continue;
		}
		char_array_4[i++] = uchar;
		in_++;
		if (i == 4)
		{
			for (string_size j = 0; j < 4; j++)
				char_array_4[j] = dtable[char_array_4[j]];

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i)
	{
		for (string_size j = i; j <4; j++)
			char_array_4[j] = 0;

		for (string_size j = 0; j <4; j++)
			char_array_4[j] = dtable[char_array_4[j]];

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (string_size j = 0; (j < i - 1); j++)
			ret += char_array_3[j];
	}

	return ret;
}
#if __cplusplus >= 201703L
//
// Interface with std::string_view rather than const std::string&
// Requires C++17
// Provided by Yannic Bonenberger (https://github.com/Yannic)
//

std::string base64_encode(std::string_view s, bool url) {
   return encode(s, url);
}

std::string base64_encode_pem(std::string_view s) {
   return encode_pem(s);
}

std::string base64_encode_mime(std::string_view s) {
   return encode_mime(s);
}

std::string base64_decode(std::string_view s, bool remove_linebreaks) {
   return decode(s, remove_linebreaks);
}

#endif  // __cplusplus >= 201703L

//////////////解决丢最后一位的问题
const char base64_encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64_decode_table[] =
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\x3E\0\0\0\x3F"
"\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\0\0\0\0\0\0"
"\0\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9\xA\xB\xC\xD\xE"
"\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\0\0\0\0\0"
"\0\x1A\x1B\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28"
"\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
;

const char base64_encode_table_s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
const char base64_decode_table_s[] =
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\x3E\0\0"
"\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\0\0\0\0\0\0"
"\0\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9\xA\xB\xC\xD\xE"
"\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\0\0\0\0\x3F"
"\0\x1A\x1B\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28"
"\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
;

void base64_encode(char *data, long len, char *output, const char *table) {
	char *d = output;
	if (table == nullptr) table = base64_encode_table;
	long end = len - 2;
	int i;
	for (i = 0; i < end; i += 3) {
		char *p = data + i;
		*(d++) = table[p[0] >> 2];
		*(d++) = table[((p[0] << 4) & 0x3F) | (p[1] >> 4)];
		*(d++) = table[((p[1] << 2) & 0x3F) | (p[2] >> 6)];
		*(d++) = table[(p[2] & 0x3F)];
	}
	char *p = data + i;
	switch (len - i) {
	case 1:
		*(d++) = table[p[0] >> 2];
		*(d++) = table[((p[0] << 4) & 0x3F)];
		*(d++) = '=';
		*(d++) = '=';
		break;
	case 2:
		*(d++) = table[p[0] >> 2];
		*(d++) = table[((p[0] << 4) & 0x3F) | (p[1] >> 4)];
		*(d++) = table[((p[1] << 2) & 0x3F)];
		*(d++) = '=';
		break;
	}
	*(d++) = '\0';
}

void base64_decode(char *str, char *data, long *len, const char *table) {
	char *d = data;
	if (table == nullptr) table = base64_decode_table;
	for (char *p = str; *p; p += 4) {
		char b = p[1];
		*(d++) = (table[p[0]] << 2) | (table[b] >> 4);
		if (!b) break;
		b = p[2];
		*(d++) = (table[p[1]] << 4) | (table[b] >> 2);
		if (!b) break;
		b = p[3];
		*(d++) = (table[p[2]] << 6) | (table[b]);
		if (!b) break;
	}
	*len = d - data;
}

void base64_encode_str(char *str, char *output, const char *table) {
	base64_encode(str, (long)strlen(str), output, table);
}

void base64_decode_str(char *str, char *output, const char *table) {
	long len;
	base64_decode(str, output, &len, table);
	output[len] = '\0';
}
