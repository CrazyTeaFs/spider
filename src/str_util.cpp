#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <arpa/inet.h>

using namespace std;

int const APLPHABET = 62;

char letter[62] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'
, 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

// Spilt String By A Delimter
vector<string> split(const string &str, char pattern) {  
    string::size_type i = 0;  
    string::size_type j = str.find(pattern);  
    vector<string> result;
	if (j == string::npos)
    {
        result.push_back(str);
        return result;
    }

	while (j != string::npos) {
		result.push_back(str.substr(i, j - i));
		i = ++j;
		j = str.find(pattern, i);
	
		if (j == string::npos) {
			result.push_back(str.substr(i));
		}
	}
	
    return result;  
}  

// BKDR Hash 
unsigned int BKDRHash(const string& str) {
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    for(size_t i = 0; i < str.length(); i++)
    {
        hash = (hash * seed) + str[i];
    }

    return hash;
}

// Ip To Dotted Decimal String
char* iptostr(unsigned host_ip) {
    struct in_addr  addr;
    memcpy(&addr, &host_ip, 4);
    return inet_ntoa(addr);
}

// Replace All Substr
string replace_all(const string &source, const string& sub, const string& target) {
	const size_t len = target.length();
	const size_t sub_len = sub.length();
	string result = source;

    string::size_type i = 0;  
    string::size_type j = 0;  

	while ( (j = result.find(sub, i)) != string::npos ) {
		if (len > sub_len) { 
			size_t offset = len - sub_len;
			result.append(string(offset, '0'));
			string tmp = result;
			for (size_t index = j; index < result.length(); index++) {
				result[index + len - sub_len] = tmp[index];
			}			
			result.replace(j, len, target);
			i = j + len;
		} else if (len == sub_len) {
			result.replace(j, len, target);
		} else {
			string tmp = result;
			size_t offset = sub_len - len;
			result.replace(j, len, target);
			for (size_t index = j + len; index < result.length(); index++) {
				result[index] = tmp[index + offset];
			}			
			result = result.substr(0, result.length() - offset);
		}
	}
	return result;
}

static int int_rand() {
    return rand() % APLPHABET;
}

// Generate A Random String
string generate_key(int digit) {
    srand(time(NULL));
    
    string result(digit, '0');
    for (int i = 0; i < digit; i++) {
        char flow_char = letter[int_rand()];
        result[i] = flow_char;
    }
    return result;
}

