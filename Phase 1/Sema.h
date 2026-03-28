#ifndef SEMA_H
#define SEMA_H

#include <iostream>
#include <string>
#include "Queue.h"
#include "Sched.h"

using namespace std;

class semaphore {

private:
    char resource_name[64];
    int sema_value;            // 0 or 1 for binary semaphore
    Queue<int> sema_queue;     // queue of blocked task IDs
    scheduler *sched_ptr;      // pointer to scheduler

public:
    semaphore(int starting_value, const string &name, scheduler *theScheduler);
    ~semaphore();

    void down(int taskID);     // acquire or block
    void up();                 // release resource
    void dump(int level);      // debugging output
};

#endif