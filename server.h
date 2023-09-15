#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include "user.h"
#include <map>
#include <string>
#include <pthread.h>
using std::string;

class Room;

class Server {
public:
  Server(int port);
  
  ~Server();

  bool listen();

  void handle_client_requests();

  Room *find_or_create_room(const std::string &room_name);
  

private:
bool update_message_to_join_and_send(Message msg, string roomname, Connection* cnx, Room *room, User *user);
void update_message_to_invalid_and_send(Message msg, string roomname, Connection* cnx);
bool update_message_to_leave_and_send(Message msg, string roomname, Connection* cnx);
void update_message_to_not_in_room_and_send(Message msg, string roomname, Connection* cnx);
void update_message_to_sent_and_send(Message msg, string roomname, Connection* cnx);
void update_message_to_empty_and_send(Message msg, string roomname, Connection* cnx);
void update_message_to_bye_and_send(Message msg, string roomname, Connection* cnx);
void update_message_to_error_and_send(Message msg, string roomname, Connection* cnx);
void sender_chat(Connection *cnx, Server *s, User *user);
void handle_error(Message msg, Connection* cnx);
void welcome_message(Message msg, Connection* cnx);
void receiver_chat(Connection *cnx, Server *s, User *user);
void handle_login(Connection* cnx, const string& username, Message msg);
bool room_does_not_exist(const string &name_of_room);

  // prohibit value semantics
  Server(const Server &);
  Server &operator=(const Server &);


  typedef std::map<std::string, Room *> RoomMap;

  // These member variables are sufficient for implementing
  // the server operations
  int m_port;
  int m_ssock;
  RoomMap m_rooms;
  pthread_mutex_t m_lock;
  
};

#endif // SERVER_H