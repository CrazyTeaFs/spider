#include <regex.h> 
#include "http_url.h"
#include "str_util.h"

using namespace std;

const string proto_regex = "^(https?:\\/\\/)?";
const string hostname_regex = "([0-9a-zA-Z-]+\\.)+[0-9a-zA-Z]{2,6}";
const string port_regex = "(:[0-9]{1,5})?";
const string port_get_regex = ":[0-9]{1,5}";
//Pattern                      /apple/banan_a/orange.html
const string path_regex = "(\\/([[:alnum:]_-]*\\/)*([[:alnum:]_-]*\\.?[[:alnum:]_-])*)?";
const string path_get_regex = "[^\\/]\\/([[:alnum:]_-]+\\/)*([[:alnum:]_-]+\\.?[[:alnum:]_-])+[\\?$]";
const string querystring_regex = "(\\?((([[:alnum:]%_-]+=[[:alnum:]%\\._-]+)&)*([[:alnum:]%_-]+=[[:alnum:]%\\._-]+)))?(#.*)?$";
const string querystring_get_regex = "(\\?((([[:alnum:]%_-]+=[[:alnum:]%\\._-]+)&)*([[:alnum:]%_-]+=[[:alnum:]%\\._-]+)))";

int HttpUrl::Parse(const string &source) {  
	int ret = 0;
	if ((ret = UrlValid(source)) != 0) {
		return ret;
	}
	
	GetProtocol(source);
	GetHostName(source);
	GetPort(source);
	GetPath(source);
	GetQueryString(source);
	ParseQueryString(querystring_);

	return ret;
}

int HttpUrl::UrlValid(const string &source) {  
	// protocol + hostname + port + path + querystring = url
	string valid_http_url = proto_regex + hostname_regex + port_regex + path_regex + querystring_regex;
	ERROR("%s\n", valid_http_url.c_str());

	regex_t reg;
	int ret = 0;
	char buf[1024] = {0};
	if ( (ret = regcomp(&reg, valid_http_url.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Valid Url Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	// Not A Http Url
	if ( (ret = regexec(&reg, source.c_str(), 0, NULL, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Valid Url Failed to Execute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}
	
	regfree(&reg);

	return ret;
}

int HttpUrl::GetProtocol(const string &source) {  
	int ret = 0;
	regmatch_t store[1];
	char buf[1024] = {0};
	regex_t reg;

	if ( (ret = regcomp(&reg, proto_regex.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Protocol Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	if ( (ret = regexec(&reg, source.c_str(), 1, store, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Protocol Failed to Execute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	memcpy(buf, source.c_str() + store[0].rm_so, store[0].rm_eo - store[0].rm_so);
	
	string protocol = buf;
	if (protocol.empty()) {
		protocol_ = "http";
		return NOTEXIST;
	}
	// Remove ://
	protocol_ = protocol.substr(0, protocol.length() - 3);

	regfree(&reg);
	return ret;
}

int HttpUrl::GetHostName(const string &source) {  
	int ret = 0;
	regmatch_t store[1];
	char buf[1024] = {0};
	regex_t reg;

	if ( (ret = regcomp(&reg, hostname_regex.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Hostname Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	if ( (ret = regexec(&reg, source.c_str(), 1, store, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Hostname Failed to Excute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	memcpy(buf, source.c_str() + store[0].rm_so, store[0].rm_eo - store[0].rm_so);
	hostname_ = buf;
	if (hostname_.empty()) {
		ret = NOTEXIST;
	}

	regfree(&reg);
	return ret;
}

int HttpUrl::GetPath(const string &source) {  
	int ret = 0;
	regmatch_t store[1];
	char buf[1024] = {0};
	regex_t reg;

	if ( (ret = regcomp(&reg, path_get_regex.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Path Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	if ( (ret = regexec(&reg, source.c_str(), 1, store, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Path Failed to Excute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	memcpy(buf, source.c_str() + store[0].rm_so, store[0].rm_eo - store[0].rm_so);
	path_ = buf;
	
	if (path_.empty()) {
		ret = NOTEXIST;
	}
	
	if (path_[path_.length() - 1] == '?') {
		path_ = path_.substr(0, path_.length() - 1);
	}

	if (path_[0] != '/') {
		path_ = path_.substr(1, path_.length() - 1);
	}

	regfree(&reg);
	return ret;
}

int HttpUrl::GetQueryString(const string &source) {  
	int ret = 0;
	regmatch_t store[1];
	char buf[1024] = {0};
	regex_t reg;

	if ( (ret = regcomp(&reg, querystring_regex.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("QureyString Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	if ( (ret = regexec(&reg, source.c_str(), 1, store, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("QueryString Failed to Execute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	memcpy(buf, source.c_str() + store[0].rm_so, store[0].rm_eo - store[0].rm_so);
	querystring_ = buf;
	if (querystring_.empty()) {
		regfree(&reg);
		return NOTEXIST;
	}
	
	querystring_ = querystring_.substr(1);

	regfree(&reg);
	return ret;
}

int HttpUrl::GetPort(const string &source) {  
	int ret = 0;
	regmatch_t store[1];
	char buf[1024] = {0};
	regex_t reg;
	string portstr;

	if ( (ret = regcomp(&reg, port_get_regex.c_str(), REG_EXTENDED)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Port Failed to Compile Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	if ( (ret = regexec(&reg, source.c_str(), 1, store, 0)) != 0) {
		regerror(ret, &reg, buf, sizeof(buf));
		ERROR("Port Failed to Execute Regular Expression, %s", buf);
		regfree(&reg);
		return ret;
	}

	memcpy(buf, source.c_str() + store[0].rm_so, store[0].rm_eo - store[0].rm_so);
	portstr = buf;
	if (portstr.empty()) {
		port_ = 80;
		ret = NOTEXIST;
	} else {
		portstr = portstr.substr(1);
		port_ = atoi(portstr.c_str());
	}

	regfree(&reg);
	return ret;
}

void HttpUrl::ParseQueryString(const string &querystring) {
	if (querystring.empty()) {
		return;
	}

	vector<string> segements = split(querystring, '&');
	
	for (size_t i = 0; i < segements.size(); i++) {
		string equation = segements[i];
		vector<string> key = split(equation, '=');
		string left = key[0];
		string right = key[1];
		query_map_.insert(make_pair(left, right));
	}

	return;
}
