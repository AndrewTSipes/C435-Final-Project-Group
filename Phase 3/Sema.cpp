#include "Sema.h"
#include <cstring>
#include <ncurses.h>

// We need access to safe_write() from main.cpp
extern void safe_write(WINDOW* win, const char* text);

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
semaphore::semaphore(int starting_value, const string &name, scheduler *theScheduler) {
    sema_value = starting_value;
    sched_ptr = theScheduler;
    log_win = nullptr;

    // Copy resource name into char array
    strncpy(resource_name, name.c_str(), sizeof(resource_name));
    resource_name[sizeof(resource_name) - 1] = '\0';

    // ============================================================
    // ===== PHASE 3 ADDITION (NO LOGIC CHANGE REQUIRED) ==========
    //
    // If this semaphore is created as:
    //
    //     semaphore CoreMemory(1, "Core Memory", &S);
    //
    // then the MMU will use this semaphore to protect the
    // core memory array and the memory block linked list.
    //
    // No additional logic is required here.
    // ============================================================
}

// ------------------------------------------------------------
// Set log window
// ------------------------------------------------------------
void semaphore::set_log_window(WINDOW* win) {
    log_win = win;
}

// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------
semaphore::~semaphore() {
    // Nothing special needed
}

// ------------------------------------------------------------
// DOWN operation (acquire or block)
// ------------------------------------------------------------
void semaphore::down(int taskID) {

    if (log_win) {
        char buff[128];
        sprintf(buff, "Task %d calling DOWN() on %s\n", taskID, resource_name);
        safe_write(log_win, buff);
    }

    // If resource is free, take it
    if (sema_value > 0) {
        sema_value--;

        if (log_win) {
            char buff[128];
            sprintf(buff, "Resource acquired by task %d\n", taskID);
            safe_write(log_win, buff);
        }

        return;
    }

    // Otherwise block the task
    if (log_win) {
        char buff[128];
        sprintf(buff, "Resource busy — task %d BLOCKED\n", taskID);
        safe_write(log_win, buff);
    }

    sema_queue.En_Q(taskID);
    sched_ptr->set_state(taskID, BLOCKED);

    // Switch to next READY task
    sched_ptr->yield();
}

// ------------------------------------------------------------
// UP operation (release resource)
// ------------------------------------------------------------
void semaphore::up() {

    if (log_win) {
        char buff[128];
        sprintf(buff, "UP() called on %s\n", resource_name);
        safe_write(log_win, buff);
    }

    // If tasks are waiting, unblock the first one
    if (!sema_queue.isEmpty()) {
        int next_task = sema_queue.De_Q();

        if (log_win) {
            char buff[128];
            sprintf(buff, "Unblocking task %d\n", next_task);
            safe_write(log_win, buff);
        }

        sched_ptr->set_state(next_task, READY);
        return;
    }

    // Otherwise just release the resource
    sema_value++;

    if (log_win) {
        safe_write(log_win, "Resource released (no waiting tasks)\n");
    }
}

// ------------------------------------------------------------
// Debug dump (console)
// ------------------------------------------------------------
void semaphore::dump(int level) {
    cout << "\nSEMAPHORE DUMP\n";
    cout << "Resource: " << resource_name << endl;
    cout << "Sema Value: " << sema_value << endl;

    cout << "Queue: ";
    sema_queue.Print();
    cout << endl;
}

// ------------------------------------------------------------
// Dump semaphore into an ncurses window
// ------------------------------------------------------------
void semaphore::dump_to_window(WINDOW* win) {
    if (win == nullptr)
        return;

    wclear(win);
    box(win, 0, 0);

    mvwprintw(win, 1, 1, "SEMAPHORE DUMP");
    mvwprintw(win, 3, 1, "Resource: %s", resource_name);
    mvwprintw(win, 4, 1, "Value: %d", sema_value);

    mvwprintw(win, 6, 1, "Blocked Queue:");

    int row = 7;
    Queue<int> temp = sema_queue;

    while (!temp.isEmpty()) {
        int tid = temp.De_Q();
        mvwprintw(win, row++, 3, "Task %d", tid);
    }

    if (row == 7) {
        mvwprintw(win, row, 3, "(empty)");
    }

    box(win, 0, 0);
    wrefresh(win);
}
