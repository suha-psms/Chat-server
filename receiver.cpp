#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

using std::string;
using std::cerr;
//using std::endl;
using std::stoi;
using std::cout;


void signal_handler(int signal_num)
{
    // It terminates the  program
    exit(signal_num);
}

void deliv_message_printed(Message msg, string room_name){
  string data = msg.data;
  string room = data.substr(0, data.find(":"));
  string nameAndMsg = data.substr(data.find(":") + 1);//, data.length()


  if (room == room_name) { // in the same room
    string senderName = nameAndMsg.substr(0, nameAndMsg.find(":"));
    string sendMsg = nameAndMsg.substr(nameAndMsg.find(":") + 1);//, nameAndMsg.length()
  
    cout << senderName << ": " << sendMsg;
  }

}

void receive_and_print(Connection &cnx, Message &message, string room_name) {
    //receive a message from the server
    cnx.receive(message);

    //if the message is a delivery message, print it
    if(message.tag == TAG_DELIVERY) {
      //print the message
      deliv_message_printed(message, room_name);
    } else {
      exit(2);
    }
}

int check_tag(Message message) {
  if (message.tag == TAG_ERR) {
    cerr << message.data;
    return 1;
  } else if (message.tag != TAG_OK) {
    cerr << "Invalid message\n";
    return 1;
  }
  return 0;
}

int cnx_not_open(Connection* cnx){
  if(!cnx->is_open()) {
    cerr << "Connection failed\n";
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc != 5) {
    cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }
 
  signal(SIGINT, signal_handler);

  string hostname = argv[1];
  int port = stoi(argv[2]);
  string username = argv[3];
  string room_name = argv[4];

  Connection cnx;

  //connect to server
  cnx.connect(hostname, port);
  
  // if it fails, print error message and exit with non-zero exit code
  if(cnx_not_open(&cnx)) {
    return 1;
  }
  

  //send rlogin and join messages (expect a response from the server for each one)
  Message message(TAG_RLOGIN, username);

  cnx.send(message);
  cnx.receive(message);

  if (check_tag(message) == 1) {
    return 1;
  }

  //send join message
  message.data = room_name;
  message.tag = TAG_JOIN;

  cnx.send(message);
  cnx.receive(message);
  
  //incorrect tag, fail
  if (check_tag(message) == 1) {
    return 1;
  }
  
  bool wait_for_messages = true;

  //loop waiting for messages from server
  while (wait_for_messages) {
    receive_and_print(cnx, message, room_name);
  }
  
  return 0;
}