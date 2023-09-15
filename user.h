#ifndef USER_H
#define USER_H

#include <string>
#include "message_queue.h"

struct User {
  std::string username;

  // queue of pending messages awaiting delivery
  MessageQueue mqueue;
  bool is_receiver;

  User(const std::string &username, const bool is_r) : username(username), is_receiver(is_r) { }
};

#endif // USER_H
