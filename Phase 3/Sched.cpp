#include "Sched.h"
#include <ncurses.h>

// We need access to safe_write() from main.cpp
extern void safe_write(WINDOW* win, const char* text);

// Constructor
// ------------------------------------------------
scheduler::scheduler() {
    process_table = nullptr;
    current = nullptr;
    next_available_task_id = 0;
    log_win = nullptr;   // ===== PHASE 3 ADDITION =====
}

// ===== PHASE 3 ADDITION =====
// Setter for log window
void scheduler::set_log_window(WINDOW* win) {
    log_win = win;
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

    // ===== PHASE 3 ADDITION =====
    new_tcb->memory_handle = -1;
    new_tcb->base = -1;
    new_tcb->limit = -1;
    new_tcb->current_location = -1;
    new_tcb->waiting_for_memory = false;

    if (log_win) {
        char buff[128];
        sprintf(buff, "Creating task #%d (%s)\n", new_tcb->task_id, name.c_str());
        safe_write(log_win, buff);
    }

    // Original task
    if (process_table == nullptr) {
        process_table = new_tcb;
        new_tcb->next = new_tcb;   // circular
        current = new_tcb;
        new_tcb->state = RUNNING;
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
        if (log_win) safe_write(log_win, "YIELD ERROR: No tasks available.\n");
        return;
    }

    if (log_win) {
        char buff[128];
        sprintf(buff, "Current Task #%d is yielding...\n", current->task_id);
        safe_write(log_win, buff);
    }

    if (current->state == RUNNING)
        current->state = READY;

    tcb *start = current;
    tcb *ptr = current->next;

    while (ptr != start && ptr->state != READY) {
        ptr = ptr->next;
    }

    if (ptr->state == READY) {
        current = ptr;
        current->state = RUNNING;
        current->start_time = clock();

        if (log_win) {
            char buff[128];
            sprintf(buff, "Switched to task #%d\n", current->task_id);
            safe_write(log_win, buff);
        }
    } else {
        if (log_win) safe_write(log_win, "No ready tasks: possible deadlock.\n");
    }
}

// Kill Task
// ------------------------------------------------
void scheduler::kill_task(int task_id) {
    if (process_table == nullptr) {
        if (log_win) safe_write(log_win, "Kill Task Error: No tasks exist.\n");
        return;
    }

    tcb *ptr = process_table;
    bool found = false;

    do {
        if (ptr->task_id == task_id) {
            found = true;
            break;
        }
        ptr = ptr->next;
    } while (ptr != process_table);

    if (!found) {
        if (log_win) {
            char buff[128];
            sprintf(buff, "KILL ERROR: Task %d not found.\n", task_id);
            safe_write(log_win, buff);
        }
        return;
    }

    if (log_win) {
        char buff[128];
        sprintf(buff, "Killing task #%d (%s)\n", task_id, ptr->task_name.c_str());
        safe_write(log_win, buff);
    }

    ptr->state = DEAD;

    if (ptr == current) {
        if (log_win) safe_write(log_win, "Killed task was CURRENT — yielding...\n");
        yield();
    }
}

// Garbage Collect
// ------------------------------------------------
void scheduler::garbage_collect() {
    if (process_table == nullptr) {
        if (log_win) safe_write(log_win, "GC: No tasks to collect.\n");
        return;
    }

    if (log_win) safe_write(log_win, "----- Running Garbage Collect -----\n");

    tcb *ptr = process_table;
    tcb *prev = nullptr;
    bool first_pass = true;

    do {
        tcb *next = ptr->next;

        if (ptr->state == DEAD) {

            if (log_win) {
                char buff[128];
                sprintf(buff, "GC: Removing task #%d\n", ptr->task_id);
                safe_write(log_win, buff);
            }

            if (ptr == ptr->next) {
                delete ptr;
                process_table = nullptr;
                current = nullptr;
                if (log_win) safe_write(log_win, "GC: All tasks removed. Scheduler empty.\n");
                return;
            }

            if (ptr == process_table) {
                tcb *last = process_table;
                while (last->next != process_table)
                    last = last->next;

                process_table = process_table->next;
                last->next = process_table;
            }

            if (ptr == current) {
                current = current->next;
                if (current->state != READY)
                    yield();
            }

            if (prev != nullptr)
                prev->next = next;

            delete ptr;
            ptr = next;

            if (ptr == process_table && !first_pass)
                break;

            continue;
        }

        prev = ptr;
        ptr = next;
        first_pass = false;

    } while (ptr != process_table);

    if (log_win) safe_write(log_win, "----- Garbage collection complete -----\n");
}

// Set state
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

// Dump to console
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
            cout << "  <-- current";

        cout << endl;

        ptr = ptr->next;
    } while (ptr != process_table);

    cout << endl;
}

// Dump to window
// ------------------------------------------------
void scheduler::dump_to_window(WINDOW* win) {
    if (win == nullptr)
        return;

    wclear(win);
    box(win, 0, 0);

    if (process_table == nullptr) {
        mvwprintw(win, 1, 1, "(Scheduler empty)");
        wrefresh(win);
        return;
    }

    mvwprintw(win, 1, 1, "PROCESS TABLE:");

    int row = 3;
    tcb* ptr = process_table;

    do {
        mvwprintw(win, row, 1,
                  "Task %d  Name: %s  State: %s%s",
                  ptr->task_id,
                  ptr->task_name.c_str(),
                  ptr->state.c_str(),
                  (ptr == current ? "  <-- current" : ""));

        row++;
        ptr = ptr->next;

    } while (ptr != process_table);

    box(win, 0, 0);
    wrefresh(win);
}

// ------------------------------------------------------------
// ⭐ MISSING FUNCTION — now added
// ------------------------------------------------------------
int scheduler::get_task_id() {
    return current ? current->task_id : -1;
}
