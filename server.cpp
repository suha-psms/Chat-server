#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

using std::string;
using std::cerr;
using std::endl;
using std::to_string;

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions
struct Data {
  public:
    Connection *cnx;
    Server *s;

    Data(Connection *cnx, Server *s)
    : cnx(cnx), s(s) { }
};


////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////
namespace {

// ARE YOU ACTUALLY BROADCASTING A MESSAGE AND CAN RECEIVER THREAD DEQUEUE
// IS ENDQUEUE BEInG CALLED - THATS ENCOURAGING A THEORY IS GOING INTO A MESSAGE QUEUE
//CLEARLY DEQUEUE IS BEING CALLED, IT STARTS ITS LOOP - DOES DEQUEUE EVER RETURN A POINTER TO A MESSAGE THAT HAS BEEN SUCCESFFULY


// OPEN 3 SEPARATE TERMINALS, add debug print statements handling sendall and other messages - message object getting enqueued, receiver is successfuly dequeing
// send puts in and sengs message and receiver propogates it to client
// other tow terminals are netcat local host port number, every number that is received will get printed - except in case of receiver thread waiting in loop to dequeue messages, only delivery emssages should be receiverd
// get to point where you can do a sendall in message to sender and received in receiver


bool update_message_to_join_and_send(Message msg, string roomname, Connection* cnx, Room *room, User *user) {
  room->add_member(user);
  cnx->send(Message(TAG_OK, "joined room " + roomname));
  return true;
}

void update_message_to_invalid_and_send(Message msg, string roomname, Connection* cnx) {
  cnx->send(Message(TAG_ERR, "invalid room name (empty)"));    
}

bool update_message_to_leave_and_send(Message msg, string roomname, Connection* cnx){
  cnx->send(Message(TAG_OK, "left room" + roomname)); 
  return false;
}

void update_message_to_not_in_room_and_send(Message msg, string roomname, Connection* cnx) {     
  cnx->send(Message(TAG_ERR, "invalid room name (empty)"));
}

void update_message_to_sent_and_send(Message msg, string roomname, Connection* cnx) {
  cnx->send(Message(TAG_OK, "message sent"));
}

void update_message_to_empty_and_send(Message msg, string roomname, Connection* cnx) {
  cnx->send(Message(TAG_ERR, "Invalid Message"));
}

void update_message_to_bye_and_send(Message msg, string roomname, Connection* cnx) {
  cnx->send(Message(TAG_OK, "bye"));
}

void update_message_to_error_and_send(Message msg, string roomname, Connection* cnx){
  cnx->send(Message(TAG_OK, "bye"));
}

void sender_chat(Connection *cnx, Server *s, User *user) {
  //Initialize a room
  Room *room = nullptr;
  string roomname;
  Message msg;

  //Boolean value indicating whether we are in a room or not
  bool user_in_room = false;
  bool const tr = true;
  
  //Keep open
  while(tr){ 
    cnx->receive(msg);
    // Join the room
    int join = (msg.tag == TAG_JOIN);
    bool leave = (msg.tag == TAG_LEAVE);
    bool send_all = (msg.tag == TAG_SENDALL);
    bool quit = (msg.tag == TAG_QUIT);

    if (join) {
      int length = msg.data.length() - 1;
      if (length == 0) {
        update_message_to_invalid_and_send(msg, roomname, cnx);       
      } else {
        roomname = msg.data.substr(0,msg.data.length() - 1); // roomname to join
        room = s->find_or_create_room(roomname);
        user_in_room = update_message_to_join_and_send(msg,roomname, cnx, room, user);
      }
    //Leave the room
    } else if (leave) {
      //check
      if(!user_in_room){
        update_message_to_not_in_room_and_send(msg, roomname, cnx);
      } else {
        //Actually remove from the room
        room->remove_member(user);
        user_in_room = update_message_to_leave_and_send(msg, roomname, cnx);
      } 
    // Sendall
    } else if(send_all) {
      if (!user_in_room){
        update_message_to_invalid_and_send(msg, roomname, cnx);
      } else {
        // send message to receiver in the room
        int length = msg.data.length() - 1;
        if (length == 0) {
          update_message_to_empty_and_send(msg, roomname, cnx);
        } else {
          room->broadcast_message(user->username,msg.data);
          update_message_to_sent_and_send(msg, roomname, cnx);
        }
      } 
    // Quit
    } else if (quit){ 
      update_message_to_bye_and_send(msg, roomname, cnx);
      break;
    // Error occured becauase wasn't any of these
    } else {
      update_message_to_error_and_send(msg, roomname, cnx);
    }
  }
}

void handle_error(Message msg, Connection* cnx){
  // ERROR handling
  cnx->send(Message(TAG_ERR, "invalid join"));

}

void welcome_message(Message msg, Connection* cnx) {
  cnx->send(Message(TAG_OK, "welcome"));
}


void receiver_chat(Connection *cnx, Server *s, User *user) {

  //Message for joining the room
  string roomname;
  bool tr = true;

  //This is the receiver
  Message msg;
  while(tr){
    cnx->receive(msg);
    roomname = msg.data.substr(0,msg.data.length()-1); // roomname to join
    if (msg.tag == TAG_JOIN) {
      welcome_message(msg, cnx);
      break;
    } else {
      //THEY HAD TO SEND A JOIN REQUEST
      handle_error(msg, cnx);
    }
  }
  

  // Join the room
  Room *room = s->find_or_create_room(roomname);
  room->add_member(user);

  bool cond = true;
  // Receive the message
  while(cond){
  Message *received_message;
  received_message = user->mqueue.dequeue();
  if (received_message == nullptr){
    continue;
  }
  bool successful_send = cnx->send(*received_message);
  if(!successful_send){
    // Remove from the room
    delete received_message;
    break; // avoid an error
  }
    delete received_message;
  }
  room->remove_member(user);
  cnx->close();
}


void handle_login(Connection* cnx, const string& username, Message msg) {
  cnx->send(Message(TAG_OK, "logged in as " + username));
}

int handle_receive_error(Message message, Data* dat) {
  if (message.tag == TAG_ERR) { //Check if the receive was successful
    cerr << message.data;
    delete dat;
    return 7; //lol idk
  }
  return 0; 
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // use a static cast to convert arg from a void* to
  // whatever pointer type describes the object(s) needed
  // to communicate with a client (sender or receiver)
  Data *dat = (Data*)arg;
  Connection *cnx = dat->cnx;
  Server *s = dat->s;
  Message msg;

  cnx->receive(msg);
  
  int num = handle_receive_error(msg, dat); 
  if (num == 7) {
    exit(1);
  }

  bool send = true;
  if(msg.tag == TAG_RLOGIN){ // if it's receiver
    send = false;
  }

  // process received message
  int ind_colon = msg.data.find(":");
  string username = msg.data.substr(ind_colon+1,msg.data.length()-1);
  handle_login(cnx, username, msg); 

  // depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  bool curr;

  // Sender
  if(send){ 
  User *user;
    curr = false;
    user = new User(username,curr);
    sender_chat(cnx, s, user);
  } else {
    //receiver
    curr = true;
    User* user = new User(username,curr);
    receiver_chat(cnx, s, user);
  }

  // delete user;     
  delete dat;
  return nullptr;
}
}



////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port) 
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  string sock = to_string(m_port);
  m_ssock = open_listenfd(sock.c_str());

  if (m_ssock >= 0) {
    return true;
  } else {
    return false;
  }
}

void Server::handle_client_requests() {
  Connection *cnx;
  Data *data; 
  bool ans = true; 
  
  while(ans){
    int socket = Accept(m_ssock, NULL, NULL);
    // connection using rc
    cnx = new Connection(socket);
    // constructor
    data = new Data(cnx, this);
    
    pthread_t thread;
    int err = pthread_create(&thread, NULL, worker, data); 
    if(err >= 0){
      continue;
    } else {
      cerr << "Failed to create a thread" << endl;
      delete data;
    }
  }
}

bool Server::room_does_not_exist(const string &name_of_room) {
  if (m_rooms.find(name_of_room) == m_rooms.end()) {
  return true;
  }
  return false;
}

Room *Server::find_or_create_room(const string &name_of_room) {
 
    Guard guard(m_lock);
    Room *room; 
    // return a pointer to the unique Room object representing
    //       the named chat room, creating a new one if necessary
    if (room_does_not_exist(name_of_room)) { // if the room exist
        room = new Room(name_of_room);
        m_rooms.insert({name_of_room, room});
    } else { 
      return m_rooms[name_of_room];
    }
    return room;
} 