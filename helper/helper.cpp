#include "helper.h"

string eraseSpaces(string str) {
	string ret = "";
	for (char c : str) {
		if (c != ' ') {
			ret += c;
		}
	}
	return ret;
}

vector<string> eraseSpaces(vector<string> vec) {
	for (string & str :vec) {
		str = eraseSpaces(str);
	}
	return vec;
}

vector <string> split (string s, char c) {
	vector <string> ret;
	string str = "";
	for (char el : s) {
			if (el == c) {
				if (str.length() > 0) {
					ret.push_back(str);
				}
				str = "";
			} else {
				str += el;
			}
	}
	if (str.length() > 0) {
		ret.push_back(str);
	}
	return ret;
}
/////////////////////////////////////////////////////////////

bool parseIpPort(char *input, string &host, int &port) {
	vector <string> splitted = split ((string) input, ':');
	if (splitted.size() < 2)
		return false;
	host = splitted[0];
	port = stoi(splitted[1]);
	return true;
}

//////////////////////////////////////////////////////////////////////

CLIENT_REQUEST_TYPE stringToRequestType (string str) {
	if (str == "/connect") {
		return REQUEST_CONNECT;
	}else if (str == "/join") {
		return REQUEST_JOIN;
	} else if (str == "/quit") {
		return REQUEST_QUIT;
	} else if (str == "/list") {
		return REQUEST_LIST_USERS;
	} else if (str == "/info") {
		return REQUEST_INFO;
	}
	return REQUEST_MESSAGE;
}

CLIENT_REQUEST_TYPE
parseClientRequest(string inputLine) {
	return stringToRequestType(eraseSpaces(split(inputLine, ' '))[0]);
}
