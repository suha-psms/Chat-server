
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

using std::string;
using std::cerr;
using std::stoi;
using std::cin;

void check_send_and_receive_success(Connection &cnx, Message &message_of_send, Message &received) {
  if (!cnx.send(message_of_send)) {
    cerr << "Failed to send message";
    exit(1);
  }

  if (!cnx.receive(received)) {
    cerr << "Failed to receive message";
    exit(1);
  }

  //check if received ok
  if (strcmp(received.tag.c_str(), TAG_OK) != 0) {
      cerr << received.data;
  }
}


int main(int argc, char **argv) {
  if (argc != 4) {
    cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  string server_hostname;
  int server_port;
  string username;

  server_hostname = argv[1];
  server_port = stoi(argv[2]);
  username = argv[3];


  //connect to server
  Connection cnx;
  cnx.connect(server_hostname, server_port);

  //check if connection properly opens
  if (!(cnx.is_open())) {
    cerr << "Connection failed\n";
    return 1;
  }
  
  //send slogin message
  Message message_of_send = Message(TAG_SLOGIN, username);

  //check if send is successful
  bool successful_send = cnx.send(message_of_send);
  if (!successful_send) {
    cerr << "Invalid connection";
    return 1;
  }
  
  //receive a message from the server
  Message received_message;
  bool rec_success = cnx.receive(received_message);

  //check if received ok
  if (!rec_success) {
    cerr << "Invalid connection";
    return 1;
  }

  if (received_message.tag == TAG_ERR) {
    error_message_received(received_message.data);
    return 1;
  } else if (received_message.tag != TAG_OK){
    cerr << "Invalid message";
    return 1;
  }
 


  // Commands are /join, /leave, /quit, and /sendall
  Message received;
  string message;
  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  while (getline(cin, message)) {
    if (strcmp(message.substr(0, 5).c_str(), "/join") == 0) {
      //join
      message_of_send.tag = TAG_JOIN;
      message_of_send.data = message.substr(6);
    } else if (strcmp(message.substr(0, 6).c_str(), "/leave") == 0) {
      message_of_send.tag = TAG_LEAVE;
      message_of_send.data = "bye";
    } else if (strcmp(message.substr(0, 5).c_str(), "/quit") == 0) {
      //Send quit message to server
      message_of_send.tag = TAG_QUIT;
      message_of_send.data = "bye";
      check_send_and_receive_success(cnx, message_of_send, received);
      //close connection
      return 0;
    } else {
      //if send doesn't start with /, send as a message
      message_of_send.data = message;
      message_of_send.tag = TAG_SENDALL;
    }
    check_send_and_receive_success(cnx, message_of_send, received);
  }

  //close connection
  cnx.close();

  return 0;
}