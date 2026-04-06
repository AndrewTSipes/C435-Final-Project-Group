#include "Sched.h"
#include "Sema.h"
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <vector>

// Global mutex for ncurses drawing (from Lab 7)
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// -----------------------------
// Safe window write helpers
// -----------------------------
void safe_write(WINDOW* win, const char* text) {
    pthread_mutex_lock(&myMutex);
    wprintw(win, text);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
}

void safe_write_xy(WINDOW* win, int y, int x, const char* text) {
    pthread_mutex_lock(&myMutex);
    mvwprintw(win, y, x, text);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
}

// -----------------------------
// Create window wrapper
// -----------------------------
WINDOW* create_window(int h, int w, int y, int x) {
    pthread_mutex_lock(&myMutex);
    WINDOW* win = newwin(h, w, y, x);
    scrollok(win, TRUE);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
    return win;
}

// -----------------------------
// MAIN
// -----------------------------
int main() {

    // -----------------------------
    // Initialize ncurses
    // -----------------------------
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // -----------------------------
    // CLEAN, NON-OVERLAPPING LAYOUT
    // -----------------------------
    // Heading at top
    WINDOW* Heading_Win = create_window(5, 80, 1, 2);
    safe_write_xy(Heading_Win, 1, 20, "ULTIMA 2.0 (Phase 1 Demo)");
    safe_write_xy(Heading_Win, 3, 2, "Press 'q' to quit, 'y' to yield, 'd' to dump scheduler");

    // Task windows below heading
    std::vector<WINDOW*> task_windows;
    task_windows.push_back(create_window(8, 25, 7, 2));    // Task A
    task_windows.push_back(create_window(8, 25, 7, 28));   // Task B
    task_windows.push_back(create_window(8, 25, 7, 54));   // Task C

    safe_write(task_windows[0], "Task A Window\n");
    safe_write(task_windows[1], "Task B Window\n");
    safe_write(task_windows[2], "Task C Window\n");

    // Log window below task windows
    WINDOW* Log_Win = create_window(12, 80, 16, 2);
    safe_write(Log_Win, "Log Window Initialized...\n");

    // Console window at bottom
    WINDOW* Console_Win = create_window(6, 80, 29, 2);
    keypad(Console_Win, TRUE);
    wtimeout(Console_Win, -1);
    safe_write_xy(Console_Win, 1, 1, "Console Ready");
    safe_write_xy(Console_Win, 2, 1, "Ultima # ");
    wmove(Console_Win, 2, 10);

    // -----------------------------
    // Create scheduler + tasks
    // -----------------------------
    scheduler S;
    S.set_log_window(Log_Win);

    int tA = S.create_task("A");
    int tB = S.create_task("B");
    int tC = S.create_task("C");

    // -----------------------------
    // Semaphore for testing
    // -----------------------------
    semaphore sem(1, "R1", &S);
    sem.set_log_window(Log_Win);

    // -----------------------------
    // Main input loop
    // -----------------------------
    int ch;
    while ((ch = wgetch(Console_Win)) != 'q') {

        switch (ch) {

            case 'y':   // yield
                safe_write(Log_Win, "Yield called...\n");
                S.yield();
                break;

            case 'd':   // dump scheduler
                safe_write(Log_Win, "Scheduler dump:\n");
                S.dump_to_window(Log_Win);
                break;

            case '1':   // kill task A
                safe_write(Log_Win, "Killing Task A...\n");
                S.kill_task(tA);
                break;

            case '2':   // kill task B
                safe_write(Log_Win, "Killing Task B...\n");
                S.kill_task(tB);
                break;

            case '3':   // kill task C
                safe_write(Log_Win, "Killing Task C...\n");
                S.kill_task(tC);
                break;

            case 'g':   // garbage collect
                safe_write(Log_Win, "Running garbage_collect...\n");
                S.garbage_collect();
                break;

            default:
                break;
        }

        safe_write_xy(Console_Win, 2, 1, "Ultima # ");
        wmove(Console_Win, 2, 10);
    }

    endwin();
    return 0;
}