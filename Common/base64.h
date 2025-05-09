//
//  base64 encoding and decoding with C++.
//  Version: 2.rc.08 (release candidate)
//

#ifndef BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include <string>

#if __cplusplus >= 201703L
#include <string_view>
#endif  // __cplusplus >= 201703L

std::string base64_encode     (std::string const& s, bool url = false);
std::string base64_encode_pem (std::string const& s);
std::string base64_encode_mime(std::string const& s);

std::string base64_decode(std::string const& s, bool remove_linebreaks = false);
std::string base64_encode(unsigned char const*, size_t len, bool url = false);

//另一方式，直接转不管长度，有些不规范
static const std::string base64_char =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";
typedef std::string::size_type string_size;
std::string base64_decodeNoCheck(const std::string &encoded_string, bool accept_urlsafe = false);

#if __cplusplus >= 201703L
//
// Interface with std::string_view rather than const std::string&
// Requires C++17
// Provided by Yannic Bonenberger (https://github.com/Yannic)
//
std::string base64_encode     (std::string_view s, bool url = false);
std::string base64_encode_pem (std::string_view s);
std::string base64_encode_mime(std::string_view s);

std::string base64_decode(std::string_view s, bool remove_linebreaks = false);
#endif  // __cplusplus >= 201703L

////////法三,有些纺编码不规范，如解析ssr的password用上面两方法最后一位丢了
void base64_encode(char *data, long len, char *output, const char *table);
void base64_decode(char *str, char *data, long *len, const char *table);
void base64_encode_str(char *str, char *output, const char *table);
void base64_decode_str(char *str, char *output, const char *table);
extern const char base64_encode_table_s[];
extern const char base64_decode_table_s[];

#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */
