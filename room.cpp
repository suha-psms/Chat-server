#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"
#include <mutex>

Room::Room(const std::string &room_name)
  : room_name(room_name) {
    // initialize the mutex
    // default value is null
  pthread_mutex_init(&lock, NULL);
}

Room::~Room() {
  // destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  // add User to the room
  Guard guard(lock); 
  members.insert(user); 
}

void Room::remove_member(User *user) {
  // remove User from the room
  Guard guard(lock); 
  members.erase(user);

}

void Room::enqueue_message_to_receiver(const std::string& sender_username, const std::string& message_text, User *user) {
  if(user->is_receiver){
    Message *msg = new Message(TAG_DELIVERY, this->room_name + ":" + sender_username + ":" + message_text.substr(0,message_text.length()-1));
    user->mqueue.enqueue(msg);
  }
}

void Room::broadcast_message(const std::string& sender_username, const std::string& message_text) {
  // send a message to every (receiver) User in the room
  Guard g(lock);
  for(User *user : members){
    enqueue_message_to_receiver(sender_username, message_text, user);
  }
}