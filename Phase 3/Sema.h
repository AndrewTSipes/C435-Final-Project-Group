#ifndef SEMA_H
#define SEMA_H

#include <iostream>
#include <string>
#include <ncurses.h>     // Needed for dump_to_window()
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
    WINDOW* log_win;           // pointer to log window

    // Constructor
    semaphore(int starting_value, const string &name, scheduler *theScheduler);

    // Destructor
    ~semaphore();

    // Set log window
    void set_log_window(WINDOW* win);

    // Acquire or block
    void down(int taskID);

    // Release resource
    void up();

    // Debugging output (console)
    void dump(int level);

    // Dump semaphore into a WINDOW
    void dump_to_window(WINDOW* win);
};

#endif
