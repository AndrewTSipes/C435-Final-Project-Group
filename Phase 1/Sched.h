#ifndef SCHED_H
#define SCHED_H

#include <iostream>
#include <string>
#include <ctime>
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
};


// Scheduler class
// ------------------------------------------------
class scheduler {

private:
    tcb *process_table; 	// head of linked list
    tcb *current;             	// pointer to current task
    int  next_available_task_id;


public:
    scheduler();
    ~scheduler();

    int  create_task(const string &name);
    void kill_task(int task_id);

    void yield();              // switch
    void garbage_collect();    // remove garbage tasks

    void set_state(int task_id, const string &new_state);
    string get_state(int task_id);

    int  get_task_id();        // return current task id
    void dump(int level);      // debugging dump
};

#endif