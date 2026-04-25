#ifndef SCHED_H
#define SCHED_H

#include <iostream>
#include <string>
#include <ctime>
#include <ncurses.h>
using namespace std;

// Process states
const string READY   = "READY";
const string RUNNING = "RUNNING";
const string BLOCKED = "BLOCKED";
const string DEAD    = "DEAD";

// Task Control Block
// ------------------------------------------------
struct tcb {
    int   task_id;
    string task_name;
    string state;
    clock_t start_time;
    tcb *next;

    // ===== PHASE 3 ADDITIONS =====
    int memory_handle;       // handle returned by MMU
    int base;                // starting address of allocated block
    int limit;               // ending address of allocated block
    int current_location;    // current read/write pointer inside block
    bool waiting_for_memory; // used for shark-pond behavior
};

// Scheduler class
// ------------------------------------------------
class scheduler {

private:
    tcb *process_table;        // head of linked list
    tcb *current;              // pointer to current task
    int  next_available_task_id;

public:
    WINDOW* log_win;           // pointer to log window

    scheduler();
    ~scheduler();

    void set_log_window(WINDOW* win);

    int  create_task(const string &name);
    void kill_task(int task_id);

    void yield();              // switch
    void garbage_collect();    // remove garbage tasks

    void set_state(int task_id, const string &new_state);
    string get_state(int task_id);

    int  get_task_id();        // return current task id
    void dump(int level);      // debugging dump

    // UI helper — dump scheduler into an ncurses window
    void dump_to_window(WINDOW* win);

    // ===== PHASE 3 ADDITION: expose current task safely =====
    tcb* get_current() { return current; }
};

#endif
