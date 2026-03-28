#include "Sema.h"
#include <cstring>

// Constructor
// ------------------------------------------------
semaphore::semaphore(int starting_value, const string &name, scheduler *theScheduler) {
    sema_value = starting_value;
    sched_ptr = theScheduler;

    // Copy resource name into char array
    strncpy(resource_name, name.c_str(), sizeof(resource_name));
    resource_name[sizeof(resource_name) - 1] = '\0';

    cout << "Semaphore created for resource: " << resource_name << endl;
}


// Destructor
// ------------------------------------------------
semaphore::~semaphore() {
    // Nothing special yet
}


// DOWN operation
// ------------------------------------------------
void semaphore::down(int taskID) {
    cout << "Task " << taskID << " calling DOWN() on " << resource_name << endl;

    if (sema_value > 0) {
        // Resource is free — acquire it
        sema_value--;
        cout << "Resource acquired by task " << taskID << endl;
        return;
    }

    // Resource is busy: block the task
    cout << "Resource busy — task " << taskID << " BLOCKED" << endl;

    sema_queue.En_Q(taskID);
    sched_ptr->set_state(taskID, BLOCKED);

    // Switch to the next READY task
    sched_ptr->yield();
}


// UP operation
// ------------------------------------------------
void semaphore::up() {
    cout << "UP() called on " << resource_name << endl;

    if (!sema_queue.isEmpty()) {
        int next_task = sema_queue.De_Q();
        cout << "Unblocking task " << next_task << endl;

        sched_ptr->set_state(next_task, READY);
        return;
    }

    // No one waiting — release the resource
    sema_value++;
    cout << "Resource released (no waiting tasks)" << endl;
}


// Debug dump
// ------------------------------------------------
void semaphore::dump(int level) {
    cout << "\nSEMAPHORE DUMP\n";
    cout << "Resource: " << resource_name << endl;
    cout << "Sema Value: " << sema_value << endl;

    cout << "Queue: ";
    sema_queue.Print();
    cout << endl;
}