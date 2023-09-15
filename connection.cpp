#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

using std::string;
//using std::endl;
//using std::cerr;

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
    //call rio_readinitb to initialize the rio_t object
    rio_readinitb(&m_fdbuf, fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // call open_clientfd to connect to the server
  // call rio_readinitb to initialize the rio_t object
  // convert port to string

  string conv = std::to_string(port);
  char const *p_string = conv.c_str();
  m_fd = open_clientfd(hostname.c_str(), p_string);

  //initialize the rio_t 
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  // close the socket if it is open
 if(is_open()){
     Close(m_fd);
  }
}

bool Connection::is_open() const {
  // return true if the connection is open
  bool open = (m_fd != -1);
  return open;
}

void Connection::close() {
  // close the connection if it is open
  if(is_open()){
     Close(m_fd);
  }
}

bool Connection::send(const Message &msg) {
  // send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  //check if open
  if(!is_open()){
    return false;
  }

  string mess = msg.tag + ":" + msg.data + "\n";
  int written = rio_writen(m_fd, mess.c_str(), mess.length()); 
  
  if(written >= 0){
     m_last_result = SUCCESS;
     return true; 
  } else {
     m_last_result = EOF_OR_ERROR;
     return false;
  }
}

bool Connection::check_if_colon(string buff_string, size_t colon_pos) {
  if (colon_pos == string::npos) {
    // Handle error
    m_last_result = INVALID_MSG;
    return true;
  }
  return false;
}

// added send all to the set - forgot about it at first oops
bool Connection::valid(const std::string& tag) {
  std::set<string> valid_tags {TAG_ERR, TAG_OK , TAG_SLOGIN, TAG_RLOGIN, 
      TAG_JOIN,TAG_LEAVE,TAG_SENDUSER, TAG_QUIT, TAG_DELIVERY, TAG_EMPTY, TAG_SENDALL};
  return valid_tags.find(tag) != valid_tags.end();
}

// set the tag and data of the message
void Connection::set_message_data(Message& msg, const std::string& tg, const std::string& pay) {
  msg.tag = tg;
  msg.data = pay;
}

void Connection::handle_receive_error() {
  // Handle error
  m_last_result = EOF_OR_ERROR;
  exit(1);
}


bool Connection::receive(Message &msg) {
  // receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    return false;
  }
  
 // read from socket and account for null terminator
  char buf[msg.MAX_LEN + 1];
  ssize_t store_buff = rio_readlineb(&m_fdbuf, buf, sizeof(buf));

  if (!(store_buff > 0)) {
    handle_receive_error();
    //exit will be called in handle_receive_error()
  }

  string buff_string = buf;
  size_t colon_pos = buff_string.find(":");

  if (check_if_colon(buff_string, colon_pos)) {
      return false;
  } 
  
  string tag = buff_string.substr(0, colon_pos);
  string payload = buff_string.substr(colon_pos + 1);
  set_message_data(msg, tag, payload);

  m_last_result = SUCCESS;
  return true;

  } 
 

