#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <cstring>
#include <vector>
using namespace std;

enum CLIENT_REQUEST_TYPE{
  REQUEST_UNKNOWN_TYPE = 0,
  REQUEST_CONNECT,
  REQUEST_MESSAGE,
  REQUEST_JOIN,
  REQUEST_QUIT,
  REQUEST_LIST_USERS,
  REQUEST_INFO,
  REQUEST_MESSAGE_SLEEP
} ;



string eraseSpaces(string str);
vector<string> eraseSpaces(vector<string> vec);

vector <string> split (string s, char c);
bool parseIpPort(char *input, string &host, int &port);


CLIENT_REQUEST_TYPE stringToRequestType (string str);
CLIENT_REQUEST_TYPE parseClientRequest(string inputLine);


#endif
