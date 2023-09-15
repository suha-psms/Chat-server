
#include <iostream>
#include <string>
#include "connection.h"
#include "message.h"
#include "client_util.h"
#include <string>

using std::string;
using std::endl;
using std::cerr;
using std::cout;

// string trim functions shamelessly stolen from
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/

const string WHITESPACE = " \n\r\t\f\v";

string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
 
string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
string trim(const std::string &s) {
  return rtrim(ltrim(s));
}
 
//NOT used, caused error-handling gradescope error
void error_message_received(const string pl){
  cerr << pl;
}

unsigned long roomUsernameMessage(const Message message, string payload) {
  size_t colon_pos = payload.find(":");
  return colon_pos;
}

unsigned long roomUsername(string str2) {
  size_t colon_pos = str2.find(":");
  return colon_pos;
}

void printOut(string username, string str) {
  cout << username << ": " << trim(str) << endl;
}


void deliv_message_printed(const Message msg){
  int orig_position = 0;

  //room:username:message
  string payload = msg.data;
  unsigned long colon = roomUsernameMessage(msg, payload); 

  //username:message
  string temp = payload.substr(colon + 1);
  colon = roomUsername(temp);

  string str =  temp.substr(colon + 1);
  string username =  temp.substr(orig_position,colon);
  

  printOut(username, str);
}

