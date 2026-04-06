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
    WINDOW* log_win;           // NEW: pointer to log window

    semaphore(int starting_value, const string &name, scheduler *theScheduler);
    ~semaphore();

    // NEW: setter for log window
    void set_log_window(WINDOW* win);

    void down(int taskID);     // acquire or block
    void up();                 // release resource
    void dump(int level);      // debugging output

    // ------------------------------------------------
    // UI helper — dump semaphore into a WINDOW
    // ------------------------------------------------
    void dump_to_window(WINDOW* win);
};

#endif