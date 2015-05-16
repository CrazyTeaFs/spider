#ifndef _STR_UTILS_H_
#define _STR_UTILS_H_

// Spilt String By A Delimter
std::vector<std::string> split(const std::string &str, char pattern);

// BKDR Hash 
unsigned int BKDRHash(const std::string& str);

// Replace All Substr
std::string replace_all(const std::string &source, const std::string& sub, const std::string& target);

// Ip To Dotted Decimal String
char* iptostr(unsigned host_ip);

#endif
