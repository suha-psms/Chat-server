Sample README.txt

Eventually your report about how you implemented thread synchronization
in the server should go here

Since synchronization is an important part of this assignment, we’d like you to support a report on your synchronization in your README.txt. 
Please include where your critical sections are, how you determined them, and why you chose the synchronization primitives for each section. 
You should also explain how your critical sections ensure that the synchronization requirements are met without introducing synchronization 
hazards (e.g. race conditions and deadlocks).

For Milestone 1, Eric and Suha used pair programming to complete the receiver and the sender. We both also worked together to debug the program.


For Milestone 2
To prevent data loss when multiple clients connect to the server, we took synchronization into account. Specifically we incorporated it with the MessageQueue, Room, and Server. 
A quick overview is just that we used Guard objects instead of the provided pthread_mutex_lock() and pthread_mutex_unlock().

In the Server, we used a Guard object in the find_or_create_room() function to avoid searching and changing the map of room name to room object simultaneously.
For the critical sectino, we only create a new room if it doesn't already exist but if th room exists and senders are joining the room, we need a guard to prevent the stated room name/room object situation.

In the MessageQueue object, we created mutex and semaphores and used a Guard object in the critical sections of both the enqueue and dequeue methods to ensure that the MessageQueue 
is modified by one thread at a time. For the critical section, we added a message to the queue and then sent to the threads.
While dequeing, guard objects were again used to prevent modification by multiple threads.

We used Guard objects in the broadcast_message() function in Room to ensure that only one sender is sending messages at a time. Using Guard 
objects instead of directly calling pthread_mutex_lock() and pthread_mutex_unlock() prevents most deadlocks and guarantees that the lock is always released.

Overall, we chose to implement 'Guard' objects to prevent deadlock as the lock is always going to actively prevent deadlock and 
create a situation where the lock is continually released/open.