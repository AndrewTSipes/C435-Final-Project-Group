#include "Sched.h"


// Constructor
// ------------------------------------------------
scheduler::scheduler() {
    process_table = nullptr;
    current = nullptr;
    next_available_task_id = 0;
}


// Destructor: free all TCB
// ------------------------------------------------
scheduler::~scheduler() {
    if (process_table == nullptr)
        return;

    tcb *start = process_table;
    tcb *ptr = process_table->next;

    while (ptr != start) {
        tcb *next = ptr->next;
        delete ptr;
        ptr = next;
    }

    delete start;
}


// Create a new task + insert
// ------------------------------------------------
int scheduler::create_task(const string &name) {
    tcb *new_tcb = new tcb;

    new_tcb->task_id = next_available_task_id++;
    new_tcb->task_name = name;
    new_tcb->state = READY;
    new_tcb->start_time = clock();
    new_tcb->next = nullptr;

    cout << "Creating task #" << new_tcb->task_id
         << " (" << name << ")" << endl;

    // Original task
    if (process_table == nullptr) {
        process_table = new_tcb;
        new_tcb->next = new_tcb;   // circular
        current = new_tcb;
        new_tcb->state = RUNNING;  // first task starts running
        return new_tcb->task_id;
    }

    // Insert task at end
    tcb *ptr = process_table;
    while (ptr->next != process_table)
        ptr = ptr->next;

    ptr->next = new_tcb;
    new_tcb->next = process_table;

    return new_tcb->task_id;
}


// Yield: strict round robin scheduling
// ------------------------------------------------
void scheduler::yield() {
    if (current == nullptr || process_table == nullptr) {
        cout << "YIELD ERROR: No tasks available." << endl;
        return;
    }

    cout << "Current Task #" << current->task_id << " is yielding..." << endl;

    // Mark current task REDY if it was RUNNING
    if (current->state == RUNNING)
        current->state = READY;

    // Find next READY task
    tcb *start = current;
    tcb *ptr = current->next;

    while (ptr != start && ptr->state != READY) {
        ptr = ptr->next;
    }

    if (ptr->state == READY) {
        current = ptr;
        current->state = RUNNING;
        current->start_time = clock();

        cout << "Switched to task #" << current->task_id << endl;
    } else {
        cout << "No ready taks: possible deadlock." << endl;
    }
}


// Kill Task 
// ------------------------------------------------
void scheduler::kill_task(int task_id) {
    if (process_table == nullptr) {
        cout << "Kill Task Error: No tasks exist." << endl;
        return;
    }

    tcb *ptr = process_table;
    bool found = false;

    // Search for the task in the ring
    do {
        if (ptr->task_id == task_id) {
            found = true;
            break;
        }
        ptr = ptr->next;
    } while (ptr != process_table);

    if (!found) {
        cout << "KILL ERROR: Task " << task_id << " not found." << endl;
        return;
    }

    cout << "Killing task #" << task_id << " (" << ptr->task_name << ")" << endl;

    // Mark the task as DEAD
    ptr->state = DEAD;

    // If the current task was killed, switch to next READY task
    if (ptr == current) {
        cout << "Killed task was CURRENT — yielding..." << endl;
        yield();
    }
}


// Garbage Collect
// ------------------------------------------------
void scheduler::garbage_collect() {
    if (process_table == nullptr) {
        cout << "GC: No tasks to collect." << endl;
        return;
    }

    cout << "\n----- Running Garbage Collect -----\n";

    tcb *ptr = process_table;
    tcb *prev = nullptr;
    bool first_pass = true;

    do {
        tcb *next = ptr->next;

        if (ptr->state == DEAD) {
            cout << "GC: Removing task #" << ptr->task_id << endl;


            // Case 1: Only one node in the ring
            if (ptr == ptr->next) {
                delete ptr;
                process_table = nullptr;
                current = nullptr;
                cout << "GC: All tasks removed. Scheduler empty.\n";
                return;
            }


            // Case 2: Removing the head of the ring
            if (ptr == process_table) {
                // Find the last node to fix the ring
                tcb *last = process_table;
                while (last->next != process_table)
                    last = last->next;

                process_table = process_table->next;
                last->next = process_table;
            }


            // Case 3: Removing the current task
            if (ptr == current) {
                current = current->next;   // move forward
                if (current->state != READY)
                    yield();               // ensure we land on a READY task
            }


            // Case 4: Normal removal
            if (prev != nullptr)
                prev->next = next;

            delete ptr;
            ptr = next;

            // Continue scanning without advancing prev
            if (ptr == process_table && !first_pass)
                break;

            continue;
        }

        // Move forward
        prev = ptr;
        ptr = next;
        first_pass = false;

    } while (ptr != process_table);

    cout << "\n----- Garbage collection complete -----\n";
}


// Set State and Get State
// ------------------------------------------------
void scheduler::set_state(int task_id, const string &new_state) {
    if (process_table == nullptr)
        return;

    tcb *ptr = process_table;
    do {
        if (ptr->task_id == task_id) {
            ptr->state = new_state;
            return;
        }
        ptr = ptr->next;
    } while (ptr != process_table);
}

string scheduler::get_state(int task_id) {
    if (process_table == nullptr)
        return "";

    tcb *ptr = process_table;
    do {
        if (ptr->task_id == task_id)
            return ptr->state;
        ptr = ptr->next;
    } while (ptr != process_table);

    return "";
}


// Get Task ID: return current running task
// ------------------------------------------------
int scheduler::get_task_id() {
    if (current == nullptr)
        return -1;
    return current->task_id;
}


// Dump: print the process table
// ------------------------------------------------
void scheduler::dump(int level) {
    cout << "\n-----PROCESS TABLE-----\n";

    if (process_table == nullptr) {
        cout << "(empty)\n";
        return;
    }

    tcb *ptr = process_table;
    do {
        cout << "Task " << ptr->task_id
             << "  Name: " << ptr->task_name
             << "  State: " << ptr->state;

        if (ptr == current)
            cout << "  <-- curent";

        cout << endl;

        ptr = ptr->next;
    } while (ptr != process_table);

    cout << endl;
}