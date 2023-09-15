#include "message_queue.h"
#include "guard.h"
#include <cassert>
#include <ctime>
#include <pthread.h>
#include <stdexcept>


MessageQueue::MessageQueue() {
// initialize the mutex and the semaphore
  pthread_mutex_init(&m_lock, NULL); //m_lock, NULL = default val
  sem_init(&m_avail, 0, 0); //m_avail, pshared = 0; value = 0
}

MessageQueue::~MessageQueue() {
  // destroy the mutex and the semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(Message *msg) {
  // put the specified message on the queue
  Guard grd(m_lock);
  m_messages.push_front(msg);
  sem_post(&m_avail);

  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
}


Message *MessageQueue::dequeue() {
  struct timespec ts;

  // get the current time using clock_gettime:
  // we don't check the return value because the only reason
  // this call would fail is if we specify a clock that doesn't
  // exist
  clock_gettime(CLOCK_REALTIME, &ts);

  // compute a time one second in the future
  ts.tv_sec += 1;

  // call sem_timedwait to wait up to 1 second for a message
  // to be available, return nullptr if no message is available

  //no message is available
  if (sem_timedwait(&m_avail, &ts) != 0) {
    return nullptr;
  }

  // remove the next message from the queue, return it
  Guard grd(m_lock);
  Message *message = nullptr;

  //Check if empty
  message = m_messages.back();
  m_messages.pop_back();
  
  return message;
}
